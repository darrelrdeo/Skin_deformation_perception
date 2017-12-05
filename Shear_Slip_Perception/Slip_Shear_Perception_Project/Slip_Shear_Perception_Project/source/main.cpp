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

#include "../../../external/chai3d-3.0.0/src/chai3d.h"
#include "experiment.h"
#include "graphics.h"
#include "shared_data.h"
#include "data.h"

using namespace chai3d;
using namespace std;

// Main or Test Harness Selection
#define MAIN

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

	// call setup function
	setup();

	//seed rand() function
	srand(sharedData->rand_seed);

	// create threads
	cThread* phantomThread = new cThread();
	cThread* experimentThread = new cThread();
	cThread* joystickThread = new cThread();



	// initialize devices
	if ((sharedData->input_device == INPUT_PHANTOM) || ((sharedData->output_device == OUTPUT_PHANTOM)))     initPhantom();
	//if (sharedData->input_device == INPUT_JOYSTICK) initJoystick();

	// initialize experiment(default) or demo 
	if (sharedData->opMode == EXPERIMENT) initExperiment();
	if (sharedData->opMode == DEMO) initDemo();

	// initialize graphics
	initGraphics(argc, argv);

	// display keyboard control options
	printf("\n\n*********************\n");
	printf("F = fullscreen toggle\n");
	printf("Q/ESC = quit\n");
	printf("*********************\n\n");

	// start simulation 
	sharedData->simulationRunning = true;
	// start threads
	if ((sharedData->input_device == INPUT_PHANTOM) || ((sharedData->output_device == OUTPUT_PHANTOM))) phantomThread->start(updatePhantom, CTHREAD_PRIORITY_HAPTICS);
	//if (sharedData->input_device == INPUT_JOYSTICK) joystickThread->start(updateJoystick, CTHREAD_PRIORITY_GRAPHICS);
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



