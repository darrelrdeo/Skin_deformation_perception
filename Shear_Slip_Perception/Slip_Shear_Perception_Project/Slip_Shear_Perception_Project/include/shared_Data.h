#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "../../../external/chai3d-3.0.0/src/chai3d.h"
#include "NIDAQcommands.h"
#include "NIDAQmx.h"
#include "cForceSensor.h"
#include "cATIForceSensor.h"
#include <stdint.h>
#include "UdpSocket.h"


using namespace chai3d;
using namespace std;

// UDP Setup
#define PORT 50114 // 
//#define IP "192.168.30.4"
//#define IP "192.168.30.197" // Always want to use my computer's IP address for IPV4 Newest
#define IP "192.168.30.18"
#define UDP_Packet_Length  403
#define MAX_MSG 381

// Operation Mode
#define DEMO		0
#define EXPERIMENT	1

// Input devices
#define INPUT_PHANTOM		1
#define INPUT_JOYSTICK		2

// Output devcies
#define OUTPUT_PHANTOM		1
#define OUTPUT_DELTA		2

// Force Sensing
#define FS_CALIB "C:\\CalibrationFiles\\FT11289.cal"
#define FS_INIT  "Dev1/ai0:5"

// BMI Max Velocity to expect for nonlinear saturation or continuous 
#define BMI_Max_Vel 737 //for continuous
#define BMI_Max_Pos 959 //for continuous
#define BMI_Max_Vel_Nonlinear 212
#define BMI_Max_Pos_Nonlinear 225

// UDP BMI Defines
#define UDP_BG_CONTROL_LINEAR 1
#define UDP_BG_CONTROL_NONLINEAR 2
#define UDP_BG_CONTROL_DIRECTION 3

// UDP BMI Control Params
#define BETA_PARAM 0.0001
#define BETA_SAT 0.5
#define BETA_DEAD_SCALAR 0.2
#define BETA_PARAM_DIVIDER 2
# define DIRECTION_SHEAR_MAX 0.5



// Experiment States
#define START_UP		0
#define ZERO_TACTOR		14
#define TEST_FORCE		17
#define TRACK_BMI		18
#define TRACK_BMI_DIRECTION 19
#define TRACK_BMI_NONLINEAR 21
#define TRACK_T5_DIR			25
#define TRACK_T5_NONLINEAR      26
#define PERCEPTION_EXPERIMENT_TRIAL 20
#define RETURN_SHEAR_TO_ZERO 22
#define RETURN_NORMAL_TO_ZERO 23
#define SET_NORMAL	24
#define IDLE			16
#define PRE_TRAINING	15
#define PRE_BLOCK		5
#define TRAINING		7
#define RECORD			2
#define RELAX			3
#define BREAK			4
#define THANKS			6

// Demo States
#define START_UP_DEMO			 7

#define FORCE_SENSOR_TESTING_DEMO	13
#define SET_ZERO_POINT_DEMO		 8
#define HOLD_ZERO_POINT_DEMO	 12
#define INPUT_PROMPT_DEMO		 9
#define DISPLAYING_FORCE_DEMO	 10
#define RAMP_DOWN_FORCE_DEMO	 11

// Loop rate parameters
#define LOOP_TIME 0.001  // for regulating thread loop rates (sec) (1Khz)


// Graphics
#define CURSOR_SIZE 0.006 // cursor radius
#define OBJECT_SCALE_FACTOR 3
#define WORKSPACE_RADIUS 0.5
#define VIRTUAL_CURSOR_VPOS 0.11



////////// SAVE DATA STRUCTURE 
typedef struct {
	int d_blockNum;			// number of current block
	string d_blockName;		// name of the current block (i.e. Haptics_Block, Vision_Block)
	int d_trialNum;			// current trial number

	// BrainGate data params
	int d_BG_Vel_Control_Mapping;
	UINT32 d_UDP_TStamp;
	float d_UDP_BG_VelX;
	float d_UDP_BG_VelY;
	float d_UDP_BG_Gain;
	double d_velocity_force_scalar;
	double d_position_force_scalar;
	double d_Fmax;
	double d_velocity_MaxForce_scalar;
	DWORD d_BMI_command_update_time;
	double d_scaled_velX_command;
	double d_scaled_velY_command;
	double d_scaled_posX_command;
	double d_scaled_posY_command;

	// cursor parameters
	double d_cursorPosX;	// current cursor x position
	double d_cursorPosY;	// current cursor y position
	double d_cursorPosZ;	// current curosr z position

	double d_cursorPosX_OneAgo;
	double d_cursorPosY_OneAgo;
	double d_cursorPosZ_OneAgo;

	double d_cursorVelX;
	double d_cursorVelY;
	double d_cursorVelZ;

	// perception study
	int d_currPerceptionAngle;

	// Input Phantom state
	double d_inputPhantomPosX;
	double d_inputPhantomPosY;
	double d_inputPhantomPosZ;

	double d_inputPhantomVelX;
	double d_inputPhantomVelY;
	double d_inputPhantomVelZ;

	double d_inputPhantomSwitch;

	// Output Phantom state
	double d_outputPhantomPosX;
	double d_outputPhantomPosY;
	double d_outputPhantomPosZ;

	double d_outputPhantomVelX;
	double d_outputPhantomVelY;
	double d_outputPhantomVelZ;

	double d_outputPhantomSwitch;

	// Output Phantom desired force output
	double d_outputPhantomForce_Desired_X;
	double d_outputPhantomForce_Desired_Y;
	double d_outputPhantomForce_Desired_Z;

	double d_outputPhantomForce_Desired_Tool_X;
	double d_outputPhantomForce_Desired_Tool_Y;
	double d_outputPhantomForce_Desired_Tool_Z;

	// Output phantom desired force vector Tool angle
	double d_outputPhantomForce_Desired_Tool_angle_deg;

	// Output Phantom current force output
	double d_outputPhantomForce_X;
	double d_outputPhantomForce_Y;
	double d_outputPhantomForce_Z;



	// Force Sensor
	double d_force[3];

	// Joystick State
	double d_joystickPosX;
	double d_joystickPosY;
	double d_joystickSwitch;

	// Time Stamps
	DWORD d_phantomLoopTimeStamp;
	DWORD d_joystickLoopTimeStamp;
	DWORD d_experimentLoopTimeStamp;
	DWORD d_recordTimeStamp;

	// Loop Rate Stamps (delta Time)
	DWORD d_phantomLoopDelta;
	DWORD d_joystickLoopDelta;
	DWORD d_experimentLoopDelta;

	// Frequency Counter reported loop frequency in Hz
	double d_phantomFreq;
	double d_joystickFreq;
	double d_experimentFreq;

	double d_timeElapsed;

} save_data; 







///////////// shared data structure
// data to share between all the threads
typedef struct {
	// operation mode
	int opMode;

	// Simulation State
	bool simulationRunning; // selected during setup/initialization
	bool simulationFinished;// executed by experiment

	// input devices 
	int input_device;

	// output devices
	int output_device;

	// graphics
	string message;

	// UDP
	
	
	// state machine params
	int experimentStateNumber;
	int nextExperimentStateNumber;
	int demoStateNumber;
	string experimentStateName;
	string demoStateName;
	int blockNum;			// number of current block
	string blockName;		// name of the current block (i.e. Haptics_Block, Vision_Block)
	int trialNum;			// current trial number

	// perception study information
	int currPerceptionAngle;
	bool nextPerceptionStim_flag;

	// cursor parameters
	double cursorPosX;	// current cursor x position
	double cursorPosY;	// current cursor y position
	double cursorPosZ;	// current curosr z position

	// zero position variables
	double outputZeroPosX;
	double outputZeroPosY;
	double outputZeroPosZ;
	cMatrix3d output_ZeroRotation;

	// variable to hold the z force into the neck upon completion of initialization and setup of end effector, in tool frame
	double outputNormalForce_Set;

	double cursorPosX_OneAgo;
	double cursorPosY_OneAgo;
	double cursorPosZ_OneAgo;

	double cursorVelX;
	double cursorVelY;
	double cursorVelZ;


	// BrainGate data params
	int BG_Vel_Control_Mapping;
	UINT32 UDP_TStamp;
	float UDP_BG_VelX;
	float UDP_BG_VelY;
	float UDP_BG_Gain;
	float UDP_BG_velocity_to_force_scalar;
	double velocity_force_scalar;
	double position_force_scalar;
	double Fmax;
	double velocity_MaxForce_scalar;
	DWORD BMI_command_update_time;
	double scaled_velX_command;
	double scaled_velY_command;
	double scaled_posX_command;
	double scaled_posY_command;




	// sensing
	bool sensing;
	cForceSensor g_ForceSensor;
	double force[3];


	// Timers for simulation/experiment
	cPrecisionClock* timer;

	// device handlers and pointers
	cHapticDeviceHandler* p_Phantom_Handler; // Handler for input Phantom device
	cGenericHapticDevicePtr p_input_Phantom;	// ptr to input phantom device
	cGenericHapticDevicePtr p_output_Phantom;	// ptr to output phantom device


	//haptic device specification data (e.g max linear stiffness and etc.)
	cHapticDeviceInfo inputPhantom_spec;
	cHapticDeviceInfo outputPhantom_spec;

	// haptic tool pointers
	cToolCursor* tool;

	// virtual cursor
	cShapeSphere* vCursor;

	// thread frequency counter
	cFrequencyCounter phantomFreqCounter;
	cFrequencyCounter joystickFreqCounter;
	cFrequencyCounter experimentFreqCounter;
	cFrequencyCounter udpFreqCounter;

	// Input Phantom state
	double inputPhantomPosX;
	double inputPhantomPosY;
	double inputPhantomPosZ;

	double inputPhantomVelX;
	double inputPhantomVelY;
	double inputPhantomVelZ;

	double inputPhantomSwitch;

	// Output Phantom state
	double outputPhantomPosX;
	double outputPhantomPosY;
	double outputPhantomPosZ;
	cMatrix3d outputPhantomRotation;

	double outputPhantomVelX;
	double outputPhantomVelY;
	double outputPhantomVelZ;

	double outputPhantomSwitch;

	// Output Phantom desired force output
	double outputPhantomForce_Desired_X;
	double outputPhantomForce_Desired_Y;
	double outputPhantomForce_Desired_Z;
	double outputPhantomForce_Desired_Tool_X;
	double outputPhantomForce_Desired_Tool_Y;
	double outputPhantomForce_Desired_Tool_Z;

	// output phantom desired force vector angle (deg)
	double outputPhantomForce_Desired_Tool_angle_deg;


	// Output Phantom current force output
	double outputPhantomForce_X;
	double outputPhantomForce_Y;
	double outputPhantomForce_Z;

	//  Output Phantom zero force flag for safety
	bool ZeroPhantomForce_FLAG;

	// Joystick State
	double joystickPosX;
	double joystickPosY;
	double joystickSwitch;

	// data storage
	bool saveData_FLAG;// flag to enable saveOneTimeStep
	vector<save_data> trialData;  // for one trial of experiment
    FILE* outputFile;              // output file for entire experiment (all blocks/trials)

	// timers to regulate thread loop rates
	cPrecisionClock m_phantomLoopTimer;
	cPrecisionClock m_joystickLoopTimer;
	cPrecisionClock m_expLoopTimer;
	cPrecisionClock m_noiseLoopTimer;
	cPrecisionClock m_UDP_BG_LoopTimer;
	
	// Time Stamps
	DWORD phantomLoopTimeStamp;
	DWORD joystickLoopTimeStamp;
	DWORD experimentLoopTimeStamp;
	DWORD recordTimeStamp;
	DWORD udp_BG_TimeStamp;

	// Loop Rate Stamps (delta Time)
	DWORD phantomLoopDelta;
	DWORD joystickLoopDelta;
	DWORD experimentLoopDelta;
	DWORD udp_BG_LoopDelta;

	// Frequency Counter reported loop frequency in Hz
	double phantomFreq;
	double joystickFreq;
	double experimentFreq;
	double udp_BG_Freq;

	
	// experiment trial elapsed time
	double timeElapsed;

	//random number generator seed
	uint32_t rand_seed;

} shared_data;



#endif