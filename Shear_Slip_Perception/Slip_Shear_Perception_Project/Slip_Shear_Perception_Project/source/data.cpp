
#include "data.h"
using namespace std;

static char response;   // Y/N response to set-up questions
static save_data temp;  // for temporarily holding one time step of data (that is to be saved)

static shared_data* p_sharedData;  // structure for sharing data between threads


// point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedData(shared_data& sharedData) {
    
    p_sharedData = &sharedData;
    
}

// set-up for simulation
void setup(void) {
	// initialize all elements of p_sharedData structure


	// Simulation State
	p_sharedData->simulationRunning = false; // selected during setup/initialization
	p_sharedData->simulationFinished = false;// executed by experiment

	// input devices 
	p_sharedData->input_device = 0;

	// output devices
	p_sharedData->output_device = 0;

	// graphics
	p_sharedData->message = " ";


	// state initializations
	p_sharedData->experimentStateName = " ";
	p_sharedData->experimentStateNumber = 0;
	p_sharedData->nextExperimentStateNumber = 0;
	p_sharedData->blockNum = 0;			// number of current block
	p_sharedData->blockName = "init";		// name of the current block (i.e. Haptics_Block, Vision_Block)
	p_sharedData->trialNum = 0;			// current trial number

										// BrainGate data params

	p_sharedData->Fmax = DIRECTION_SHEAR_MAX;			// shear force maximum
	p_sharedData->velocity_force_scalar = p_sharedData->Fmax / BMI_Max_Vel;//0.000542236026376;
	p_sharedData->position_force_scalar = p_sharedData->Fmax / BMI_Max_Pos; //0.000417101138737;
	//p_sharedData->UDP_BG_velocity_to_force_scalar = 

	//BG Params
	p_sharedData->UDP_BG_VelX = 0;
	p_sharedData->UDP_BG_VelY = 0;
	p_sharedData->UDP_TStamp = 0;
	p_sharedData->UDP_BG_Gain = 0;
	p_sharedData->velocity_MaxForce_scalar = 0; // computed scalar by dividing max shear force (Fmax) by velocity Magnitude (Vmag)
	p_sharedData->BMI_command_update_time = 1; //ms

	// cursor parameters
	p_sharedData->cursorPosX = 0;	// current cursor x position NOTE: This should be set to our desired z position in the space
	p_sharedData->cursorPosY = 0;	// current cursor y position
	p_sharedData->cursorPosZ = 0;	// current cursor z position 

	p_sharedData->cursorPosX_OneAgo = 0;
	p_sharedData->cursorPosY_OneAgo = 0;
	p_sharedData->cursorPosZ_OneAgo = 0;

	p_sharedData->cursorVelX = 0;
	p_sharedData->cursorVelY = 0;
	p_sharedData->cursorVelZ = 0;

	// Perception Study (need to save)
	p_sharedData->currPerceptionAngle = 0;
	p_sharedData->nextPerceptionStim_flag = false;

	// Input Phantom state
	p_sharedData->inputPhantomPosX = 0;
	p_sharedData->inputPhantomPosY = 0;
	p_sharedData->inputPhantomPosZ = 0;

	p_sharedData->inputPhantomVelX = 0;
	p_sharedData->inputPhantomVelY = 0;
	p_sharedData->inputPhantomVelZ = 0;

	p_sharedData->inputPhantomSwitch = 0;

	// Output Phantom state
	p_sharedData->outputPhantomPosX = 0;
	p_sharedData->outputPhantomPosY = 0;
	p_sharedData->outputPhantomPosZ = 0;

	p_sharedData->outputPhantomVelX = 0;
	p_sharedData->outputPhantomVelY = 0;
	p_sharedData->outputPhantomVelZ = 0;

	// initialize output phantom zero positions and rotation
	p_sharedData->outputZeroPosX = 0;
	p_sharedData->outputZeroPosY = 0;
	p_sharedData->outputZeroPosZ = 0;
	p_sharedData->output_ZeroRotation = cMatrix3d(0, 0, 0, 0, 0, 0, 0, 0, 0);
	p_sharedData->outputNormalForce_Set = 0;

	p_sharedData->outputPhantomSwitch = 0;

	// Output Phantom desired force output
	p_sharedData->outputPhantomForce_Desired_X = 0;
	p_sharedData->outputPhantomForce_Desired_Y = 0;
	p_sharedData->outputPhantomForce_Desired_Z = 0;
	p_sharedData->outputPhantomForce_Desired_Tool_X = 0;
	p_sharedData->outputPhantomForce_Desired_Tool_Y = 0;
	p_sharedData->outputPhantomForce_Desired_Tool_Z = 0;

	// output phantom desired force vector (tool) end
	p_sharedData->outputPhantomForce_Desired_Tool_angle_deg = 0;


	// Output Phantom current force output
	p_sharedData->outputPhantomForce_X = 0;
	p_sharedData->outputPhantomForce_Y = 0;
	p_sharedData->outputPhantomForce_Z = 0;

	p_sharedData->ZeroPhantomForce_FLAG = false;

	// Data Saving flag
	p_sharedData->saveData_FLAG = true;

	// Joystick State
	p_sharedData->joystickPosX = 0;
	p_sharedData->joystickPosY = 0;
	p_sharedData->joystickSwitch = 0;



	// Turn force sensing on
	p_sharedData->sensing = true;
	for (int i = 0; i < 3; i++) p_sharedData->force[i] = 0;


	// Initialize Force sensing
	p_sharedData->g_ForceSensor.Set_Calibration_File_Loc(FS_CALIB);
	p_sharedData->g_ForceSensor.Initialize_Force_Sensor(FS_INIT);
	cSleepMs(1000);
	p_sharedData->g_ForceSensor.Zero_Force_Sensor();
	cSleepMs(1000);
	printf("\n\nForce Sensor Initialized\n\n");



	// Time Stamps
	p_sharedData->phantomLoopTimeStamp = 0;
	p_sharedData->joystickLoopTimeStamp = 0;
	p_sharedData->experimentLoopTimeStamp = 0;
	p_sharedData->recordTimeStamp = 0;

	// Loop Rate Stamps (delta Time)
	p_sharedData->phantomLoopDelta = 0;
	p_sharedData->joystickLoopDelta = 0;
	p_sharedData->experimentLoopDelta = 0;

	// Frequency Counter reported loop frequency in Hz
	p_sharedData->phantomFreq = 0;
	p_sharedData->joystickFreq = 0;
	p_sharedData->experimentFreq = 0;

	// elapsed time 
	p_sharedData->timeElapsed = 0;

    // create timers, PHANTOM device handler
    // NOTE: only use these constructors once (at beginning of main) to avoid pointer issues
    p_sharedData->timer = new cPrecisionClock();

	// New handler instances for phantom devices
    p_sharedData->p_Phantom_Handler = new cHapticDeviceHandler();
	int numHapticDevices = p_sharedData->p_Phantom_Handler->getNumDevices();

	// Assign haptic devices to handler for easy reference via pointers
	p_sharedData->p_Phantom_Handler->getDevice(p_sharedData->p_input_Phantom, 0);   // 1st available haptic device
	p_sharedData->p_Phantom_Handler->getDevice(p_sharedData->p_output_Phantom, 1); // 2nd available haptic device

	//get device specifications
	p_sharedData->inputPhantom_spec = p_sharedData->p_input_Phantom->getSpecifications();
	p_sharedData->outputPhantom_spec = p_sharedData->p_output_Phantom->getSpecifications();




	// ask for operating mode (defaults to demo)
    printf("\nIs this going to be an Experiment(E) or Demo(D)?\n");
    cin >> response;
    if (response == 'e' || response == 'E') {

		// Ask for type of input 
		printf("\nWhat is your input device?");
		printf("\n(1) PHANTOM, (2) JOYSTICK\n");
        cin >> response;
		if (response == '1') p_sharedData->input_device = INPUT_PHANTOM;
		else if (response == '2') p_sharedData->input_device = INPUT_JOYSTICK;
	

		// ask for type of output device
		printf("\nWhat is your output device?");
		printf("\n(1) PHANTOM, (2) DELTA\n");
		cin >> response;
		if (response == '1') p_sharedData->output_device = OUTPUT_PHANTOM;
		else if (response == '2') p_sharedData->output_device = OUTPUT_DELTA;

		// ask for type of output device
		printf("\nWhat is your Control Type?");
		printf("\n(L) LINEAR, (N) NONLINEAR, (D) DIRECTION\n");
		cin >> response;
		if (response == 'l') p_sharedData->BG_Vel_Control_Mapping = UDP_BG_CONTROL_LINEAR;
		else if (response == 'n') p_sharedData->BG_Vel_Control_Mapping = UDP_BG_CONTROL_NONLINEAR;
		else if (response == 'd') p_sharedData->BG_Vel_Control_Mapping = UDP_BG_CONTROL_DIRECTION;

		// set operation mode to experiment
        p_sharedData->opMode = EXPERIMENT;
    }
    // if demo, ask for input device 
    else if (response == 'd' || response == 'D'){
		p_sharedData->opMode = DEMO;
        printf("\nWhat is your input device?");
        printf("\n(1) PHANTOM, (2) JOYSTICK\n");
        cin >> response;
        if (response == '1') p_sharedData->input_device = INPUT_PHANTOM;
        else if (response == '2') p_sharedData->input_device = INPUT_JOYSTICK;
		
		//hard code the output device to be a phantom
		p_sharedData->output_device = OUTPUT_PHANTOM;

		// Set first state of demo state machine
		p_sharedData->opMode = DEMO;

	}	



}

// save one time step of data to vector for current trial
void saveOneTimeStep(void) {
    // Time Stamp
	p_sharedData->recordTimeStamp = timeGetTime();
	
	// create temporary save_data structure
	save_data temp;
    
    // record individual parameters
	temp.d_blockNum = p_sharedData->blockNum;			// number of current block
	temp.d_blockName = p_sharedData->blockName;		// name of the current block (i.e. Haptics_Block, Vision_Block)
	temp.d_trialNum = p_sharedData->trialNum;			// current trial number
	

	// BG
	temp.d_UDP_TStamp = p_sharedData->UDP_TStamp;
	temp.d_UDP_BG_Gain = p_sharedData->UDP_BG_Gain;
	temp.d_UDP_BG_VelX = p_sharedData->UDP_BG_VelX;
	temp.d_UDP_BG_VelY = p_sharedData->UDP_BG_VelY;
	temp.d_velocity_force_scalar = p_sharedData->velocity_force_scalar;
	temp.d_position_force_scalar = p_sharedData->position_force_scalar;
	temp.d_Fmax = p_sharedData->Fmax;
	temp.d_velocity_MaxForce_scalar = p_sharedData->velocity_MaxForce_scalar;
	temp.d_BMI_command_update_time = p_sharedData->BMI_command_update_time; //ms
	temp.d_scaled_posX_command = p_sharedData->scaled_posX_command;
	temp.d_scaled_posY_command = p_sharedData->scaled_posY_command;
	temp.d_scaled_velX_command = p_sharedData->scaled_velX_command;
	temp.d_scaled_velY_command = p_sharedData->scaled_velY_command;

	// cursor parameters
	temp.d_cursorPosX = p_sharedData->cursorPosX;	// current cursor x position
	temp.d_cursorPosY = p_sharedData->cursorPosY;	// current cursor y position
	temp.d_cursorPosZ = p_sharedData->cursorPosZ;	// current curosr z position

	temp.d_cursorPosX_OneAgo = p_sharedData->cursorPosX_OneAgo;
	temp.d_cursorPosY_OneAgo = p_sharedData->cursorPosY_OneAgo;
	temp.d_cursorPosZ_OneAgo = p_sharedData->cursorPosZ_OneAgo;

	temp.d_cursorVelX = p_sharedData->cursorVelX;
	temp.d_cursorVelY = p_sharedData->cursorVelY;
	temp.d_cursorVelZ = p_sharedData->cursorVelZ;

	// perception
	temp.d_currPerceptionAngle = p_sharedData->currPerceptionAngle;

	// Input Phantom state
	temp.d_inputPhantomPosX = p_sharedData->inputPhantomPosX;
	temp.d_inputPhantomPosY = p_sharedData->inputPhantomPosY;
	temp.d_inputPhantomPosZ = p_sharedData->inputPhantomPosZ;

	temp.d_inputPhantomVelX = p_sharedData->inputPhantomVelX;
	temp.d_inputPhantomVelY = p_sharedData->inputPhantomVelY;
	temp.d_inputPhantomVelZ = p_sharedData->inputPhantomVelZ;

	temp.d_inputPhantomSwitch = p_sharedData->inputPhantomSwitch;

	// Output Phantom state
	temp.d_outputPhantomPosX = p_sharedData->outputPhantomPosX;
	temp.d_outputPhantomPosY = p_sharedData->outputPhantomPosY;
	temp.d_outputPhantomPosZ = p_sharedData->outputPhantomPosZ;

	temp.d_outputPhantomVelX = p_sharedData->outputPhantomVelX;
	temp.d_outputPhantomVelY = p_sharedData->outputPhantomVelY;
	temp.d_outputPhantomVelZ = p_sharedData->outputPhantomVelZ;

	temp.d_outputPhantomSwitch = p_sharedData->outputPhantomSwitch;

	// Output Phantom current force output
	temp.d_outputPhantomForce_X = p_sharedData->outputPhantomForce_X;
	temp.d_outputPhantomForce_Y = p_sharedData->outputPhantomForce_Y;
	temp.d_outputPhantomForce_Z = p_sharedData->outputPhantomForce_Z;



	// Output Phantom desired force output
	temp.d_outputPhantomForce_Desired_X = p_sharedData->outputPhantomForce_Desired_X;
	temp.d_outputPhantomForce_Desired_Y = p_sharedData->outputPhantomForce_Desired_Y;
	temp.d_outputPhantomForce_Desired_Z = p_sharedData->outputPhantomForce_Desired_Z;
	temp.d_outputPhantomForce_Desired_Tool_X = p_sharedData->outputPhantomForce_Desired_Tool_X;
	temp.d_outputPhantomForce_Desired_Tool_Y = p_sharedData->outputPhantomForce_Desired_Tool_Y;
	temp.d_outputPhantomForce_Desired_Tool_Z = p_sharedData->outputPhantomForce_Desired_Tool_Z;

	// output phantom desired force vector angle (deg)
	temp.d_outputPhantomForce_Desired_Tool_angle_deg = p_sharedData->outputPhantomForce_Desired_Tool_angle_deg;

	// force sensing
	for (int i = 0; i<3; i++) temp.d_force[i] = p_sharedData->force[i];

	// Joystick State
	temp.d_joystickPosX = p_sharedData->joystickPosX;
	temp.d_joystickPosY = p_sharedData->joystickPosY;
	temp.d_joystickSwitch = p_sharedData->joystickSwitch;


	// Time Stamps
	temp.d_phantomLoopTimeStamp = p_sharedData->phantomLoopTimeStamp;
	temp.d_joystickLoopTimeStamp = p_sharedData->joystickLoopTimeStamp;
	temp.d_experimentLoopTimeStamp = p_sharedData->experimentLoopTimeStamp;
	temp.d_recordTimeStamp = p_sharedData->recordTimeStamp;

	// Loop Rate Stamps (delta Time)
	temp.d_phantomLoopDelta = p_sharedData->phantomLoopDelta;
	temp.d_joystickLoopDelta = p_sharedData->joystickLoopDelta;
	temp.d_experimentLoopDelta = p_sharedData->experimentLoopDelta;

	// Frequency Counter reported loop frequency in Hz
	temp.d_phantomFreq = p_sharedData->phantomFreq;
	temp.d_joystickFreq = p_sharedData->joystickFreq;
	temp.d_experimentFreq = p_sharedData->experimentFreq;

	// trial elapsed time
	temp.d_timeElapsed = p_sharedData->timeElapsed;
   
    // push into vector for current trial
	p_sharedData->trialData.push_back(temp);
    
}

// write data to file from current trial
void recordTrial(void) {
	//DWORD start_t = timeGetTime();

    // iterate over vector, writing one time step at a time
    for (vector<save_data>::iterator it = p_sharedData->trialData.begin() ; it != p_sharedData->trialData.end(); ++it) {
        fprintf(p_sharedData->outputFile,"%d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %lu %lu %lu %lu %lu %lu %lu %f %f %f %f %lu %f %f %f %f %f %f %f %f %f %f %f %u",
                	it->d_blockNum,
					it->d_trialNum,
					it->d_cursorPosX,
					it->d_cursorPosY,
					it->d_cursorPosZ,
					it->d_cursorPosX_OneAgo,
					it->d_cursorPosY_OneAgo,
					it->d_cursorPosZ_OneAgo,
					it->d_cursorVelX,
					it->d_cursorVelY,
					it->d_cursorVelZ,
					it->d_inputPhantomPosX,
					it->d_inputPhantomPosY,
					it->d_inputPhantomPosZ,
					it->d_inputPhantomVelX,
					it->d_inputPhantomVelY,
					it->d_inputPhantomVelZ,
					it->d_inputPhantomSwitch,
					it->d_outputPhantomPosX,
					it->d_outputPhantomPosY,
					it->d_outputPhantomPosZ,
					it->d_outputPhantomVelX,
					it->d_outputPhantomVelY,
					it->d_outputPhantomVelZ,
					it->d_outputPhantomSwitch,
					it->d_outputPhantomForce_X,
					it->d_outputPhantomForce_Y,
					it->d_outputPhantomForce_Z,
					it->d_outputPhantomForce_Desired_X,
					it->d_outputPhantomForce_Desired_Y,
					it->d_outputPhantomForce_Desired_Z,
					it->d_outputPhantomForce_Desired_Tool_X,
					it->d_outputPhantomForce_Desired_Tool_Y,
					it->d_outputPhantomForce_Desired_Tool_Z,
					it->d_outputPhantomForce_Desired_Tool_angle_deg,
					it->d_force[0],
					it->d_force[1],
					it->d_force[2],
					it->d_joystickPosX,
					it->d_joystickPosY,
					it->d_joystickSwitch,
					it->d_phantomLoopTimeStamp,
					it->d_joystickLoopTimeStamp,
					it->d_experimentLoopTimeStamp,
					it->d_recordTimeStamp,
					it->d_phantomLoopDelta,
					it->d_joystickLoopDelta,
					it->d_experimentLoopDelta,
					it->d_velocity_force_scalar,
					it->d_position_force_scalar,
					it->d_Fmax,
				    it->d_velocity_MaxForce_scalar,
					it->d_BMI_command_update_time,
					it->d_scaled_posX_command,
					it->d_scaled_posY_command,
					it->d_scaled_velX_command,
					it->d_scaled_velY_command,
					it->d_phantomFreq,
					it->d_joystickFreq,
					it->d_experimentFreq,
					it->d_timeElapsed,
					it->d_UDP_BG_VelX,
					it->d_UDP_BG_VelY,
					it->d_UDP_BG_Gain,
					it->d_UDP_TStamp
				);

		// print to file
		fprintf(p_sharedData->outputFile, "\n");

	}
    
    // clear vector for next segment and signal that recording is done
    p_sharedData->trialData.clear();
	//DWORD end_t = timeGetTime();
	//DWORD Total_t = end_t - start_t;

	//printf("\n\nFINISHED RECORDING in %ul ms\n\n", Total_t);
    
}
