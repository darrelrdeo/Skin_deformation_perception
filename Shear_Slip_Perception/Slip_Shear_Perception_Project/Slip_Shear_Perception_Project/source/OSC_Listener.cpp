#include "OSC_Listener.h"
#include <stdlib.h>
#include <stdio.h>

using namespace std;
// Required Library inclusion to work with oscpack library
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <cstring>

#include <cstdio>
#include <cassert>
#include <windows.h>
#include <stdio.h>
#include <queue>
#include "OscReceivedElements.h"
#include "OscPacketListener.h"
#include "UdpSocket.h"
#include "shared_Data.h"
#include <conio.h>


#define MAIN
//#define TEST_SET_FORCE



// DEBUG FLAG
//#define DEBUG_PRINTF
//#define TEST_EPOC_FORCE

// Module Level Defines
static float PosX = 0; // initialize BG Data Pos X
static float PosY = 0; // initialize BG Data Pos Y
static float CurrGain = 0;
static int Rx_FLAG = 0;
static float Pos;

OSC_Listener::OSC_Listener(void)
{

}


void OSC_Listener::queryState(float& Pos_X, float& Pos_Y, float& Gain){
	Pos_X = PosX;
	Pos_Y = PosY;
	Gain = CurrGain;
}




OSC_Listener::~OSC_Listener(void)
{
}


// Function to process the messages recieved on the desired port
    void OSC_Listener::ProcessMessage( const osc::ReceivedMessage& m, 
				const IpEndpointName& remoteEndpoint )
    {
		//printf("PROCESS\n\n");
        (void) remoteEndpoint; // suppress unused parameter warning

        try{
            
            // If a message is recieved, decode it
            if( std::strcmp( m.AddressPattern(), "/POS/UPDATE" ) == 0 ){ // if the adress matches Pos Update data
                
				// Recieve the argument of the stream and store it into args, should be two arguments
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream(); 
            
				// store the recieved message (cognitive magnitude) into a variable

				args >> Pos >> osc::EndMessage;//osc::EndMessage;
                
	
                std::cout << "Recieved /POS/UPDATE pattern with contents:     "
                    << Pos <<"\n\n";
	
            
			}else if( std::strcmp( m.AddressPattern(), "/COG/RIGHT" ) == 0 ){ // else if the address matches COG RIGHT
				
				// Recieve the argument of the stream and store it into args
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream(); 
            
				// store the recieved message (cognitive magnitude) into a variable
               /* args >> CogRight >>  osc::EndMessage;
				CogLeft = 0;
				CogNeutral = 0;
                */
				#ifdef DEBUG_PRINTF
                std::cout << "Recieved /COG/RIGHT pattern with contents:     "
                    << CogRight <<"\n\n";
				#endif

			}else if( std::strcmp( m.AddressPattern(), "/COG/NEUTRAL" ) == 0 ){ // else if the address matches COG Neutral
				
				// Recieve the argument of the stream and store it into args
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream(); 
            
				// store the recieved message (cognitive magnitude) into a variable
             /*   args >> CogNeutral >>  osc::EndMessage;
				CogRight = 0;
				CogLeft = 0;
               */ 
				#ifdef DEBUG_PRINTF
                std::cout << "Recieved /COG/NEUTRAL pattern with contents:     "
                    << CogNeutral <<"\n\n";
				#endif
            
            }



        }catch( osc::Exception& e ){ // catch any errors
            // any parsing errors such as unexpected argument types, or 
            // missing arguments get thrown as exceptions.
            std::cout << "error while parsing message: "
                << m.AddressPattern() << ": " << e.what() << "\n";
        }


		
		// Test The Set Force
#ifdef TEST_EPOC_FORCE
		// Set forces to the NeutoTouch, make instance of neurotouch
		
		static float Force_desired = 0;
		if( abs(CogRight) > abs(CogLeft)) Force_desired = CogRight;
		else Force_desired = -1*CogLeft;
		device.setForce(-1*Force_desired,0);
		printf("Force Desired: %f\n\n", Force_desired);

#endif


    }