//*
// * File:   Main.cpp
// * Author: Darrel R. Deo
// *
// * 
// * Created on October 15, 2017, 9:53 PM
// */
//
//
//*******************************************************************************
// *                                 INCLUDES                                    *
// ******************************************************************************/
#include <Windows.h>
#include <assert.h>
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include "Phantom.h"
#include "cForceSensor.h"
#include "cATIForceSensor.h"
#include "cDaqHardwareInterface.h"
#include "../../../external/chai3d-3.0.0/src/chai3d.h"
#include "experiment.h"
#include "graphics.h"
#include "shared_data.h"
#include "data.h"

using namespace chai3d;
using namespace std;

// Main or Test Harness Selection
#define MAIN
//#define TEST_NIDAQ_FT

//------------------------
// Variables & Structures
//------------------------
// threads and the data shared between them (NOTE: graphics is threaded separate from the CThread architecture)

cThread* phantomThread;
cThread* experimentThread;
shared_data *sharedData;


//---------------
// Main Function
//---------------
#ifdef MAIN
int main(int argc, char* argv[]) {

	cGenericHapticDevicePtr hapticDevice[2];
	// create a haptic device handler
	cHapticDeviceHandler* handler = new cHapticDeviceHandler();

	// get number of haptic devices
	int numHapticDevices = handler->getNumDevices();
	//numHapticDevices = 2;

	// setup each haptic device
	for (int i = 0; i<numHapticDevices; i++)
	{
		// get a handle to the first haptic device
		handler->getDevice(hapticDevice[i], i);

		// open a connection to haptic device
		hapticDevice[i]->open();

		// calibrate device (if necessary)
		hapticDevice[i]->calibrate();

		// retrieve information about the current haptic device
		cHapticDeviceInfo info = hapticDevice[i]->getSpecifications();

		// if the device has a gripper, enable the gripper to simulate a user switch
		hapticDevice[i]->setEnableGripperUserSwitch(true);
	}


	sharedData = new shared_data();

	// link shared data structure
	linkSharedData(*sharedData);

	// give each thread access to shared data
	linkSharedDataToPhantom(*sharedData);
	linkSharedDataToExperiment(*sharedData);
	linkSharedDataToGraphics(*sharedData);

	// call setup functions
	setup();

	// call setup of perception force profiles

	//seed rand() function
	srand(sharedData->rand_seed);

	// create threads
	cThread* phantomThread = new cThread();
	cThread* experimentThread = new cThread();
	cThread* joystickThread = new cThread();


	// initialize devices
	if ((sharedData->input_device == INPUT_PHANTOM) || ((sharedData->output_device == OUTPUT_PHANTOM)))     initPhantom();


	// initialize experiment(default) or demo 
	if (sharedData->opMode == EXPERIMENT) initExperiment();
	if (sharedData->opMode == DEMO) initDemo();

	// initialize graphics
	initGraphics(argc, argv);

	

	// start simulation 
	sharedData->simulationRunning = true;
	// start threads
	if ((sharedData->input_device == INPUT_PHANTOM) || ((sharedData->output_device == OUTPUT_PHANTOM))) phantomThread->start(updatePhantom, CTHREAD_PRIORITY_HAPTICS);
	experimentThread->start(updateExperiment, CTHREAD_PRIORITY_GRAPHICS);
	glutTimerFunc(50, graphicsTimer, 0);
	glutMainLoop();


	// close everything
	close();

	delete sharedData;

	// exit
	return 0;
}

#endif



//#define TEST_NIDAQ_FT
#ifdef TEST_NIDAQ_FT

int main(int argc, char* argv[]) {
	// set up simulation with user input
	linkSharedData(sharedData);
	setup();

	// create threads
	cThread* phantomThread = new cThread();
	cThread* experimentThread = new cThread();

	// give each thread access to shared data

	linkSharedDataToPhantom(sharedData);
	linkSharedDataToExperiment(sharedData);
	linkSharedDataToGraphics(sharedData);

	// initialize devices
	if (sharedData->input == PHANTOM) initPhantom();
	initNeuroTouch();

	// initialize force sensor
	sharedData->g_ForceSensor.Set_Calibration_File_Loc(FS_CALIB);
	sharedData->g_ForceSensor.Initialize_Force_Sensor(FS_INIT);
	cSleepMs(1000);
	sharedData->g_ForceSensor.Zero_Force_Sensor();

	// initialize experiment or demo (default)
	if (sharedData->opMode == EXPERIMENT) initExperiment();
	else if (sharedData->opMode == DEMO) initDemo();


	// initialize graphics
	initGraphics(argc, argv);

	// display keyboard control options
	printf("\n\n*********************\n");
	printf("M = operating mode toggle (experiment vs. demo)\n");
	printf("I = input device toggle for demo mode (Emotiv vs. PHANTOM vs. auto)\n");
	printf("S = force sensing toggle (ON/OFF)");
	printf("C = controller toggle\n");
	printf("O = increase speed of autonomous cursor\n");
	printf("L = decrease speed of autonomous cursor\n");
	printf("F = fullscreen toggle\n");
	printf("Q/ESC = quit\n");
	printf("*********************\n\n");


	double measuredForce[3];
	double forceData[3] = { 0,0,0 };
	for (;;) {
		// display the forces
		// get force sensor data 
		int forceSensorData = sharedData->g_ForceSensor.AcquireFTData();
		sharedData->g_ForceSensor.GetForceReading(forceData);

		measuredForce[0] = forceData[0];
		measuredForce[1] = forceData[1];
		measuredForce[2] = forceData[2];

		printf("\nX:    %f\nY:    %f\nZ:    %f\n", measuredForce[0], measuredForce[1], measuredForce[2]);
		Sleep(1000);


	}

	// start threads
	neurotouchThread->start(updateNeuroTouch, CTHREAD_PRIORITY_HAPTICS);  // highest priority
	bciThread->start(updateBCI, CTHREAD_PRIORITY_HAPTICS);
	phantomThread->start(updatePhantom, CTHREAD_PRIORITY_GRAPHICS);
	experimentThread->start(updateExperiment, CTHREAD_PRIORITY_GRAPHICS);
	glutTimerFunc(50, graphicsTimer, 0);
	glutMainLoop();



	// close everything
	close();

	// exit
	return 0;




}





#endif // TEST_NIDAQ_FT
