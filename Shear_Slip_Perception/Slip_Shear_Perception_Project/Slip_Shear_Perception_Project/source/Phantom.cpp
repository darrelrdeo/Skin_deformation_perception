
#include "Phantom.h"
#include <cmath>
#include <limits>

#include <stdint.h>

using namespace chai3d;
using namespace std;

#define DEBUG_FLAG

static const double phantomScalar = 2;  // to scale PHANTOM workspace to graphics workspace

static shared_data* p_sharedData;  // structure for sharing data between threads


// thread timestamp vars
static DWORD currTime = 0;
static DWORD lastTime = 0;


// initialize the PHANTOM device
void initPhantom(void) {

    // open and calibrate the input PHANTOM
    bool inPhantomOpened = p_sharedData->p_input_Phantom->open();
    p_sharedData->p_input_Phantom->calibrate();

	// open and calibrate the output PHANTOM
    bool outPhantomOpened = p_sharedData->p_output_Phantom->open();
    p_sharedData->p_output_Phantom->calibrate();

	p_sharedData->p_input_Phantom->setEnableGripperUserSwitch(true);
	p_sharedData->p_output_Phantom->setEnableGripperUserSwitch(true);

    // initialize device loop timer
	p_sharedData->m_phantomLoopTimer.setTimeoutPeriodSeconds(LOOP_TIME);
	p_sharedData->m_phantomLoopTimer.start();
	p_sharedData->m_noiseLoopTimer.start();




#ifdef DEBUG_FLAG
    printf("\n Phantoms initialized!\n");
#endif
}

// point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedDataToPhantom(shared_data& sharedData) {
    
    p_sharedData = &sharedData;
    
}

// get PHANTOM state and update shared data
void updatePhantom(void) {

    // initialize frequency counter
    p_sharedData->phantomFreqCounter.reset();

	// check whether the simulation is running
    while(p_sharedData->simulationRunning) {

		// run loop only if phantomLoopTimer timeout has occurred
        if (p_sharedData->m_phantomLoopTimer.timeoutOccurred()) {

			// ensure the timer has stopped for this loop
			p_sharedData->m_phantomLoopTimer.stop();

			// Get timestamp and compute the delta for looprate
			currTime = timeGetTime();
			DWORD delta = currTime - lastTime;

			// store time stamps for book-keeping
			p_sharedData->phantomLoopDelta = delta;
			p_sharedData->phantomLoopTimeStamp = currTime;

			// if the input device is a phantom then perform updates for input, otherwise skip
            if (p_sharedData->input_device == INPUT_PHANTOM) {

				updateCursor();


				// Checking for switch press
				bool stat1 = false;
				bool stat2 = false;
				p_sharedData->p_input_Phantom->getUserSwitch(1,stat1);
				p_sharedData->p_input_Phantom->getUserSwitch(2,stat2);
				if(stat1 || stat2) p_sharedData->inputPhantomSwitch = 1;
				else p_sharedData->inputPhantomSwitch = 0;
   
            }

			// if the output device is a phantom then perform updates for output, otherwise skip
			if (p_sharedData->output_device == OUTPUT_PHANTOM) {

				// render the appropriate forces through interaction with virtual environment (these desired forces should be computed in experiment thread)
				cVector3d desiredForce = cVector3d(p_sharedData->outputPhantomForce_Desired_X, p_sharedData->outputPhantomForce_Desired_Y, p_sharedData->outputPhantomForce_Desired_Z);
				p_sharedData->p_output_Phantom->setForce(desiredForce);

			}
            // update frequency counter
            p_sharedData->phantomFreqCounter.signal(1);
			p_sharedData->phantomFreq = p_sharedData->phantomFreqCounter.getFrequency();

			// restart loop timer after update completion
            p_sharedData->m_phantomLoopTimer.start(true);

			// update timestamp var last
			lastTime = currTime;
        }
    }

}

void updateCursor(void) {
	// position-position mapping between input phantom and virtual cursor

	/*
	//This code segment maps cursor position to non-noisy device position
	p_sharedData->cursorPosY = p_sharedData->tool->getDeviceLocalPos().y();
	p_sharedData->cursorPosZ = p_sharedData->tool->getDeviceLocalPos().z();
	p_sharedData->cursorPosX = p_sharedData->tool->getDeviceLocalPos().x();
	*/
	p_sharedData->cursorPosX = phantomScalar*p_sharedData->inputPhantomPosX;
	p_sharedData->cursorPosY = phantomScalar*p_sharedData->inputPhantomPosY;
	p_sharedData->cursorPosZ = phantomScalar*p_sharedData->inputPhantomPosZ;
	

	

	/*
	//This code segment maps cursor position to the "proxy sphere"
	p_sharedData->cursorPosX = p_sharedData->tool->m_hapticPoint->m_sphereProxy->getLocalPos().x();
	p_sharedData->cursorPosY = p_sharedData->tool->m_hapticPoint->m_sphereProxy->getLocalPos().y();
	p_sharedData->cursorPosZ = p_sharedData->tool->m_hapticPoint->m_sphereProxy->getLocalPos().z();
	*/

	// update cursor position
	p_sharedData->vCursor->setLocalPos( cVector3d(VIRTUAL_CURSOR_VPOS,p_sharedData->cursorPosY,p_sharedData->cursorPosZ) );
	//p_sharedData->vCursor->setLocalPos( cVector3d(p_sharedData->cursorPosX,p_sharedData->cursorPosY,p_sharedData->cursorPosZ) );
}



// safely close the PHANTOM devices
void closePhantom(void) {
    if (p_sharedData->input_device == INPUT_PHANTOM) p_sharedData->p_input_Phantom->close();
	if (p_sharedData->output_device == OUTPUT_PHANTOM) p_sharedData->p_output_Phantom->close();
}


