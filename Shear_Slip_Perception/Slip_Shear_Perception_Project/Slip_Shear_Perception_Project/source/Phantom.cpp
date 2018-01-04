
#include "Phantom.h"
#include <cmath>
#include <limits>

#include <stdint.h>

using namespace chai3d;
using namespace std;

#define DEBUG_FLAG


// Data variables we need to record during the record step
cVector3d cursor_pos; //temporary variable for haptic tool cursor position
cVector3d input_pos;  // temporary variable for input PHANTOM position 
cVector3d input_vel;  // temporary variable for input PHANTOM velocity 
cVector3d output_pos;  // temporary variable for output PHANTOM position 
cMatrix3d output_rotation;
cVector3d output_vel;  // temporary variable for output PHANTOM velocity 
cVector3d output_force; // temp var for output PHANTOM currently output force


static const double phantomScalar = 2;  // to scale PHANTOM workspace to graphics workspace
										
// private vector to hold phantom GetForce (should move to p_Shared)
static cVector3d phantomForce(0, 0, 0);

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

			//calculate the cursorVel
			if (delta != 0)
			{
				p_sharedData->cursorVelX = (p_sharedData->cursorPosX - p_sharedData->cursorPosX_OneAgo) / delta;
				p_sharedData->cursorVelY = (p_sharedData->cursorPosY - p_sharedData->cursorPosY_OneAgo) / delta;
				p_sharedData->cursorVelZ = (p_sharedData->cursorPosZ - p_sharedData->cursorPosZ_OneAgo) / delta;
			}
			else
			{
				p_sharedData->cursorVelX = p_sharedData->cursorPosX - p_sharedData->cursorPosX_OneAgo;
				p_sharedData->cursorVelY = p_sharedData->cursorPosY - p_sharedData->cursorPosY_OneAgo;
				p_sharedData->cursorVelZ = p_sharedData->cursorPosZ - p_sharedData->cursorPosZ_OneAgo;
			}

			//store current position as old position
			p_sharedData->cursorPosX_OneAgo = p_sharedData->cursorPosX;
			p_sharedData->cursorPosY_OneAgo = p_sharedData->cursorPosY;
			p_sharedData->cursorPosZ_OneAgo = p_sharedData->cursorPosZ;

			// poll the input and output devices and record their states.

			// get INPUT PHANTOM position and velocity vectors
			p_sharedData->p_input_Phantom->getPosition(input_pos);
			p_sharedData->p_input_Phantom->getLinearVelocity(input_vel);

			//store position values into respective variable in sharedData structure
			p_sharedData->inputPhantomPosX = input_pos.x();
			p_sharedData->inputPhantomPosY = input_pos.y();
			p_sharedData->inputPhantomPosZ = input_pos.z();

			// store velocity values into respective vars in sharedData structure
			p_sharedData->inputPhantomVelX = input_vel.x();
			p_sharedData->inputPhantomVelY = input_vel.y();
			p_sharedData->inputPhantomVelZ = input_vel.z();

			// get current forces output by OUTPUT PHANTOM device
			p_sharedData->p_output_Phantom->getForce(output_force);
			p_sharedData->outputPhantomForce_X = output_force.x();
			p_sharedData->outputPhantomForce_Y = output_force.y();
			p_sharedData->outputPhantomForce_Z = output_force.z();

			// get OUTPUT PHANTOM position and velocity vectors and rotation matrix
			p_sharedData->p_output_Phantom->getPosition(output_pos);
			p_sharedData->p_output_Phantom->getLinearVelocity(output_vel);
			p_sharedData->p_output_Phantom->getRotation(output_rotation);

			//store position values into respective variable in sharedData structure
			p_sharedData->outputPhantomPosX = output_pos.x();
			p_sharedData->outputPhantomPosY = output_pos.y();
			p_sharedData->outputPhantomPosZ = output_pos.z();

			//store rotation matrix of output phantom tool
			p_sharedData->outputPhantomRotation = output_rotation;

			// store velocity values into respective vars in sharedData structure
			p_sharedData->outputPhantomVelX = output_vel.x();
			p_sharedData->outputPhantomVelY = output_vel.y();
			p_sharedData->outputPhantomVelZ = output_vel.z();



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

				// obtain force sensor reading
				// if sensing, measure XYZ finger force
				if (p_sharedData->sensing) {
					int n = p_sharedData->g_ForceSensor.AcquireFTData();  // integer output indicates success/failure
					p_sharedData->g_ForceSensor.GetForceReading(p_sharedData->force);
				}

				
				// check Zero force flag, if true then force to zero
			
					// render the appropriate forces through interaction with virtual environment (these desired forces should be computed in experiment thread)
					cVector3d desiredForce = cVector3d(p_sharedData->outputPhantomForce_Desired_X, p_sharedData->outputPhantomForce_Desired_Y, p_sharedData->outputPhantomForce_Desired_Z);
				
					if (p_sharedData->ZeroPhantomForce_FLAG) {
						cVector3d desiredForce(0, 0, 0);
					}
				p_sharedData->p_output_Phantom->setForce(desiredForce);

				// update Phantom Getforce
				p_sharedData->p_output_Phantom->getForce(phantomForce);
				p_sharedData->outputPhantomForce_X = phantomForce.x();
				p_sharedData->outputPhantomForce_Y = phantomForce.y();
				p_sharedData->outputPhantomForce_Z = phantomForce.z();
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
	p_sharedData->cursorPosX = phantomScalar*p_sharedData->inputPhantomPosX;
	p_sharedData->cursorPosY = phantomScalar*p_sharedData->inputPhantomPosY;
	p_sharedData->cursorPosZ = phantomScalar*p_sharedData->inputPhantomPosZ;

	// update cursor position
	p_sharedData->vCursor->setLocalPos( cVector3d(VIRTUAL_CURSOR_VPOS,p_sharedData->cursorPosY,p_sharedData->cursorPosZ) );
}



// safely close the PHANTOM devices
void closePhantom(void) {
    if (p_sharedData->input_device == INPUT_PHANTOM) p_sharedData->p_input_Phantom->close();
	if (p_sharedData->output_device == OUTPUT_PHANTOM) p_sharedData->p_output_Phantom->close();
}



// transformation matrix function for contact point (tool tip) to base frame
// input parameters are tool_force_desired (in tool frame) and tool orientation (rotation matrix between current tool frame and 
cVector3d Rotate_Tool_to_Base_Frame(cVector3d tool_force_desired, cMatrix3d tool_orientation) {

	cVector3d force_desired = cVector3d(0,0,0);
	force_desired = tool_orientation*tool_force_desired;


	return force_desired;
}



// transformation matrix function for tool tip to premium base frame



