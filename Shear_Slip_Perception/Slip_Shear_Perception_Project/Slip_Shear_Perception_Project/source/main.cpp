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


#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "UDP_BG.h"
#include <assert.h>
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "Phantom.h"
#include "cForceSensor.h"
#include "cATIForceSensor.h"
#include "cDaqHardwareInterface.h"
#include "../../../external/chai3d-3.0.0/src/chai3d.h"
#include "experiment.h"
#include "graphics.h"
#include "shared_data.h"
#include "data.h"
#include "UdpSocket.h"
#include "PacketListener.h"

// library inclusion to work with oscpack library
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
/*
// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
// library inclusion to work with oscpack library
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
*/
using namespace chai3d;
using namespace std;

// Main or Test Harness Selection
#define MAIN
//#define TEST_NIDAQ_FT
//#define TEST_UDP


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
	linkSharedDataToUDP_BG(*sharedData);

	// call setup functions
	setup();

	// call setup of perception force profiles

	//seed rand() function
	srand(sharedData->rand_seed);

	// create threads
	cThread* phantomThread = new cThread();
	cThread* experimentThread = new cThread();
	cThread* joystickThread = new cThread();
	cThread* UDP_thread = new cThread();

	// initialize devices
	initUDP_BG();
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



#ifdef TEST_UDP

int main(int argc, char *argv[])
{

	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != 0) {
		std::cout << "WSAStartup failed: " << nResult << std::endl;
		return 1;
	}
	struct sockaddr_in  serverAddress;   // declared as global
	struct sockaddr_in  clientAddress; // declared as global
	int len = sizeof(struct sockaddr); // declared as global
	SOCKET s = NULL; // declared as global

	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &serverAddress.sin_addr);
	//serverAddress.sin_addr.s_addr = inet_addr(IP);// INADDR_ANY;
//	serverAddress.sin_addr =  inet_addr(IP);//INADDR_ANY;
	serverAddress.sin_port = htons(PORT);
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//int c = connect(s, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (s == INVALID_SOCKET)
	{

		printf(" Unable to create a socket \n");
		printf(" Failed with error : %d\n", WSAGetLastError());
		while (1) {}

	}
	else
	{
		std::cout << "CREATED" << std::endl;
	}

	if (bind(s, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	{
		printf(" Unable to bind socket \n");
		printf(" Failed with error : %d\n", WSAGetLastError());
		while (1) {}
	}
	else
	{
		struct sockaddr_in sin;
		socklen_t len = sizeof(sin);
		if (getsockname(s, (struct sockaddr *)&sin, &len) == -1)
			perror("getsockname");
		else {
			//printf("port number %d\n", ntohs(sin.sin_port));
			//printf("IP number %d\n", inet_ntoa(sin.sin_addr));
		}
		printf(" Bound to socket .\n");
		while(1){

			//cout << "Received message from the Server: " << recvudp(s, MAX_MSG, serverAddress, len) << endl;
			int i = recvudp(s, MAX_MSG, serverAddress, len);
			if (getsockname(s, (struct sockaddr *)&sin, &len) == -1)
				perror("getsockname");
			else {
			//	printf("port number %d\n", ntohs(serverAddress.sin_port));
			//	printf("IP number %d\n", inet_ntoa(serverAddress.sin_addr));
			}
		}
	}

	WSACleanup();
	return 0;

	/**/
}


#endif