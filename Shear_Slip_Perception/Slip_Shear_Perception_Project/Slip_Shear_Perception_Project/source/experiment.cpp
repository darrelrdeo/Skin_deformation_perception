
#include "experiment.h"
#include "Phantom.h"
#include <math.h>
#include <cmath>
#include <random>
#include <ctime>
#include <vector>
#include <algorithm>

using namespace chai3d;
using namespace std;

//#define DEBUG 



// Experiment State Machine params
#define TRIALS_TRG 8
#define TRIALS_EXP 20

int trialsPerExperimentBlock = TRIALS_TRG;	  // trials per experiment block, initialize as training first.
static const int trialsBeforeBreak = 5;
static const int trialTime = 30;                  // max time per trial or washout [sec]
static const int stretchTime_Validation = 5;	 // 5 seconds
static const int breakTime = 15;                 // break time [sec] between blocks
static const int preblockTime = 10;               // time to display message [sec]
static const int trainingTime = 60;
static const int recordTime = 5;                  // time to record data [sec]
static const int relaxTime = 10;				// time to relax between large strings of trials within same block
static const int relax_to_trial_time = 2;       // Message prompt in seconds before starting trial after relaxation, requires button press to continue
static const int force_sense_trial_time = 5;
static const int numberOfBlocks = 1;






// Test force defines
static const double degree_increment = 10; // degrees increment 
cMatrix3d R_Z;
										  
// set force vector magnitude 
double force_vector_magnitude = 1; // 0.4 N
cVector3d force_vector_rotated;
cVector3d force_vector;

// Demo Loop Params
bool demo_start = true;
DWORD dbouncer = 0;

// zero holding
double KP = 10;
cVector3d position(0, 0, 0);
cVector3d force(0, 0, 0);

// File params
static int subjectNum;           // subject number
static int session;              // session number for subject
static char filename[100];       // output filename
static int nextExperimentState;  // so state machine knows where to go next

// BCI File params
static char posx_filename[100];
static char posy_filename[100];
static char velx_filename[100];
static char vely_filename[100];

// BCI command arrays
std::vector<float>posx_BCI;
std::vector<float>posy_BCI;
std::vector<float>velx_BCI;
std::vector<float>vely_BCI;
static int num_command_elements = 0;
static int BMI_command_itorator = 0;


// Perception Force Profiles params
const int num_single_stimulus_application = 3; // number of times a unique stimulus is applied
const int delta_angle_deg = 30; // degrees of perception angle wedges
const int num_wedges = 360/delta_angle_deg;
const int num_perception_trials = num_wedges*num_single_stimulus_application; // number of total perception trials
int Randomized_stimulus_angle_array[num_perception_trials]; // this array will hold the randomized trial progression
int Randomized_stimulus_angleIndex_array[num_perception_trials]; // this will be the target index 
const int ramp_duration_ms = 1000; // 1 second ramp
const double ramp_duration_s = ramp_duration_ms*0.001;
const int delta_time_ms = 1; // 1 ms
const int num_force_ind = ramp_duration_ms / delta_time_ms;
int curr_force_ind = 0;
int perception_angles_tf[num_wedges];
float force_profiles[num_wedges][num_force_ind][2]; // [force vector angle][index of force profile in tool frame][forces in tool plane XY]
cMatrix3d R_z_perception;											// Perception Study Defines
cVector3d unitVect_zero_deg(1, 0, 0);


int blockNumberIndex = 0;

int userReady = 0;

static shared_data* p_sharedData;  // structure for sharing data between threads

// thread timestamp vars
static DWORD currTime = 0;
static DWORD lastTime = 0;

// point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedDataToExperiment(shared_data& sharedData) {
    
    p_sharedData = &sharedData;
    
}


// set-up for experiment
void initExperiment(void) {
	// BCI filename read params
	sprintf(posx_filename, "cursorPosx.csv");
	sprintf(posy_filename, "cursorPosy.csv");
	sprintf(velx_filename, "cursorVelx.csv");
	sprintf(vely_filename, "cursorVely.csv");
	float in_number;
	ifstream File;
	
	File.open(posx_filename);
	while (File >> in_number) {
		posx_BCI.push_back(in_number);
		
		// add 1 to the count of command elements in array
		num_command_elements = num_command_elements + 1;
	}
	printf("\n\n num_elements = %d\n\n", num_command_elements);
	File.close();

	File.open(posy_filename);
	while (File >> in_number) {
		posy_BCI.push_back(in_number);
	}
	File.close();

	File.open(velx_filename);
	while (File >> in_number) {
		velx_BCI.push_back(in_number);
	}
	File.close();

	File.open(vely_filename);
	while (File >> in_number) {
		vely_BCI.push_back(in_number);
	}
 	File.close();


    // get subject number, control paradigm, and session number
    printf("\nEnter subject number: ");
    cin >> subjectNum;
    printf("\nEnter session number: ");
    cin >> session;
    printf("\n");
       
    // generate filename and open file for writing
    sprintf(filename, "Subj_%d_Session_%d.dat", subjectNum, session);
    p_sharedData->outputFile = fopen(filename,"w");
    fprintf(p_sharedData->outputFile, "blockNum, trialNum, cursorPosX, cursorPosY, cursorPosZ, cursorPosX_OneAgo, cursorPosY_OneAgo, cursorPosZ_OneAgo, cursorVelX, cursorVelY, cursorVelZ, inputPhantomPosX, inputPhantomPosY, inputPhantomPosZ, inputPhantomVelX, inputPhantomVelY, inputPhantomVelZ, inputPhantomSwitch, outputPhantomPosX, outputPhantomPosY, outputPhantomPosZ, outputPhantomVelX, outputPhantomVelY, outputPhantomVelZ, outputPhantomSwitch, outputPhantomForce_X, outputPhantomForce_Y, outputPhantomForce_Z, outputPhantomForce_Desired_X, outputPhantomForce_Desired_Y, outputPhantomForce_Desired_Z, outputPhantomForce_Desired_Tool_X, outputPhantomForce_Desired_Tool_Y, outputPhantomForce_Desired_Tool_Z, outputPhantomForce_Desired_Tool_angle_deg, Measured_Force_X, Measured_Force_Y, Measured_Force_Z, joystickPosX, joystickPosY, joystickSwitch, phantomLoopTimeStamp, joystickLoopTimeStamp, experimentLoopTimeStamp,recordTimeStamp, phantomLoopDelta, joystickLoopDelta, experimentLoopDelta, velocity_force_scalar, position_force_scalar, Fmax, velocity_MaxForce_scalar, BMI_command_update_time_MS, scaled_posX_command, scaled_posY_command, scaled_velX_command, scaled_velY_command, phantomFreq, joystickFreq, experimentFreq, timeElapsed\n");
    
    // enter start-up mode, with force feedback off for safety
   	p_sharedData->opMode = EXPERIMENT;
    p_sharedData->experimentStateNumber = START_UP;
    p_sharedData->message = "Welcome. Press any key to proceed to training.";
    
	// initialize experiment loop timer
	p_sharedData->m_expLoopTimer.setTimeoutPeriodSeconds(LOOP_TIME);
	p_sharedData->m_expLoopTimer.start(true);

	//initialize random seed for randomization of trials
	srand(timeGetTime());

	// load perception study array
	setup_Perception_Force_Profiles();


}

void initDemo(void){


	// initialize experiment loop timer
	p_sharedData->opMode = DEMO;
	p_sharedData->m_expLoopTimer.setTimeoutPeriodSeconds(LOOP_TIME);
	p_sharedData->m_expLoopTimer.start(true);
	p_sharedData->message = "Welcome to DEMO MODE.";

	p_sharedData->demoStateNumber = START_UP_DEMO;

}


// experiment state machine (only entered if in experiment mode)
void updateExperiment(void) {

	// initialize frequency counter for experiment thread
    p_sharedData->experimentFreqCounter.reset();

    while (p_sharedData->simulationRunning) {
		
		// only update experiment if timer has expired
	    if (p_sharedData->m_expLoopTimer.timeoutOccurred()) {
			
			// Get timestamp and compute the delta
			currTime = timeGetTime();
			DWORD delta = currTime - lastTime;
			p_sharedData->experimentLoopTimeStamp = currTime;
			p_sharedData->experimentLoopDelta = delta;

			// stop timer for experiment loop
			p_sharedData->m_expLoopTimer.stop();



//*********************EXPERIMENT STATE MACHINE************************************		
			if (p_sharedData->opMode == EXPERIMENT) {
						
				// Begin State Machine
				switch (p_sharedData->experimentStateNumber) {

/**********************************************************************************/                    
					// START UP STATE 
					case START_UP:
						p_sharedData->experimentStateName = "START_UP";
						// wait for a keypress
						while(true){
							if (_kbhit()) {
                        
								// when keypress occurs then send to ZERO Protocol
								p_sharedData->message = "Please bias the Nano17 (press n). \nPlease Zero the tactor against the Participant's neck. Press z when complete.";
								p_sharedData->experimentStateNumber = ZERO_TACTOR;
								
								break;
							}
						}
						break; // END : START UP STATE
                    

/**********************************************************************************/

						// ZERO Tactor State
					case ZERO_TACTOR : 
						p_sharedData->experimentStateName = "ZERO_TACTOR";
						// button press in graphic window z and n in graphics.cpp

						// display All measured forces from Nano17
						p_sharedData->timer->setTimeoutPeriodSeconds(preblockTime);


						break;


					case TEST_FORCE:
						p_sharedData->experimentStateName = "Test Force";



						// construct initial force vector to be rotated (in tool frame)
						force_vector = cVector3d(1, 0, 0);
						force_vector = force_vector*force_vector_magnitude;


						// rotate vector:

						R_Z.set(cos(p_sharedData->outputPhantomForce_Desired_Tool_angle_deg*PI/180), -sin(p_sharedData->outputPhantomForce_Desired_Tool_angle_deg*PI / 180), 0, sin(p_sharedData->outputPhantomForce_Desired_Tool_angle_deg*PI / 180), cos(p_sharedData->outputPhantomForce_Desired_Tool_angle_deg*PI / 180), 0, 0, 0, 1);
						force_vector_rotated = R_Z*force_vector;

						// add z force into neck
						force_vector.z(p_sharedData->outputNormalForce_Set);
						force_vector_rotated = Rotate_Tool_to_Base_Frame(force_vector_rotated, p_sharedData->outputPhantomRotation);

						// command desired forces to phantom
						//send force to hapic device
						p_sharedData->outputPhantomForce_Desired_X = force_vector_rotated.x();
						p_sharedData->outputPhantomForce_Desired_Y = force_vector_rotated.y();
						p_sharedData->outputPhantomForce_Desired_Z = force_vector_rotated.z();

						// save one timestep
						saveOneTimeStep();

						// wait for time to expire and increment angle to display and send to small down
						//and turn off all forces and send to record. 
						if (p_sharedData->timer->timeoutOccurred()) {
							// turn off all forces
							setOutputForceToZero();

							// increment angle of desired force vector
							p_sharedData->outputPhantomForce_Desired_Tool_angle_deg = p_sharedData->outputPhantomForce_Desired_Tool_angle_deg + degree_increment;

							recordTrial();
							// set/start timer (from zero) and begin block of trials
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentStateNumber = RECORD;


						}

						break;






					
					case TRACK_BMI:
						p_sharedData->experimentStateName = "Track BMI";

						if (BMI_command_itorator < num_command_elements/2) {
							//update current force to be output by BMI command updated at 1KHz
							p_sharedData->scaled_posX_command = p_sharedData->position_force_scalar*posx_BCI[BMI_command_itorator];
							p_sharedData->scaled_posY_command = p_sharedData->position_force_scalar*posy_BCI[BMI_command_itorator];
							p_sharedData->scaled_velX_command = p_sharedData->velocity_force_scalar*velx_BCI[BMI_command_itorator];
							p_sharedData->scaled_velY_command = p_sharedData->velocity_force_scalar*vely_BCI[BMI_command_itorator];

							// increment itorator
							BMI_command_itorator = BMI_command_itorator + 1;

							// construct initial force vector to be rotated (in tool frame)
							force_vector = cVector3d(p_sharedData->scaled_velX_command, p_sharedData->scaled_velY_command, p_sharedData->outputNormalForce_Set);

							// update desired tool forces
							p_sharedData->outputPhantomForce_Desired_Tool_X = force_vector.x();
							p_sharedData->outputPhantomForce_Desired_Tool_Y = force_vector.y();
							p_sharedData->outputPhantomForce_Desired_Tool_Z = force_vector.z();

							// rotate tool to base frame
							force_vector = Rotate_Tool_to_Base_Frame(force_vector, p_sharedData->outputPhantomRotation);

							

							// command desired forces to phantom
							//send force to hapic device
							p_sharedData->outputPhantomForce_Desired_X = force_vector.x();
							p_sharedData->outputPhantomForce_Desired_Y = force_vector.y();
							p_sharedData->outputPhantomForce_Desired_Z = force_vector.z();

							// save one timestep
							saveOneTimeStep();
						}
						else // end of tracking
						{
							// set all forces to zero
							setOutputForceToZero();
							
							// send to record
							recordTrial();
							// set/start timer (from zero) and begin block of trials
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentStateNumber = RECORD;


						}


						break;






					case TRACK_BMI_NONLINEAR:
						p_sharedData->experimentStateName = "Track BMI Nonlinear";

						if (BMI_command_itorator < num_command_elements / 2) {
							//update current force to be output by BMI command updated at 1KHz
							p_sharedData->scaled_posX_command = p_sharedData->position_force_scalar*posx_BCI[BMI_command_itorator];
							p_sharedData->scaled_posY_command = p_sharedData->position_force_scalar*posy_BCI[BMI_command_itorator];
							p_sharedData->scaled_velX_command = p_sharedData->velocity_force_scalar*velx_BCI[BMI_command_itorator];
							p_sharedData->scaled_velY_command = p_sharedData->velocity_force_scalar*vely_BCI[BMI_command_itorator];

							// check if scaled commands exceed the maximums defined 
							if (p_sharedData->scaled_posX_command > p_sharedData->Fmax) p_sharedData->scaled_posX_command = p_sharedData->Fmax;
							if (p_sharedData->scaled_posX_command < -1*p_sharedData->Fmax) p_sharedData->scaled_posX_command = -1*p_sharedData->Fmax;

							if (p_sharedData->scaled_posY_command > p_sharedData->Fmax) p_sharedData->scaled_posY_command = p_sharedData->Fmax;
							if (p_sharedData->scaled_posY_command < -1*p_sharedData->Fmax) p_sharedData->scaled_posY_command = -1*p_sharedData->Fmax;

							if (p_sharedData->scaled_velX_command > p_sharedData->Fmax) p_sharedData->scaled_velX_command = p_sharedData->Fmax;
							if (p_sharedData->scaled_velX_command < -1*p_sharedData->Fmax) p_sharedData->scaled_velX_command = -1*p_sharedData->Fmax;

							if (p_sharedData->scaled_velY_command > p_sharedData->Fmax) p_sharedData->scaled_velY_command = p_sharedData->Fmax;
							if (p_sharedData->scaled_velY_command < -1 * p_sharedData->Fmax) p_sharedData->scaled_velY_command = -1 * p_sharedData->Fmax;

							// increment itorator
							BMI_command_itorator = BMI_command_itorator + 1;

							// construct initial force vector to be rotated (in tool frame)
							force_vector = cVector3d(p_sharedData->scaled_velX_command, p_sharedData->scaled_velY_command, p_sharedData->outputNormalForce_Set);

							// update desired tool forces
							p_sharedData->outputPhantomForce_Desired_Tool_X = force_vector.x();
							p_sharedData->outputPhantomForce_Desired_Tool_Y = force_vector.y();
							p_sharedData->outputPhantomForce_Desired_Tool_Z = force_vector.z();

							// rotate tool to base frame
							force_vector = Rotate_Tool_to_Base_Frame(force_vector, p_sharedData->outputPhantomRotation);



							// command desired forces to phantom
							//send force to hapic device
							p_sharedData->outputPhantomForce_Desired_X = force_vector.x();
							p_sharedData->outputPhantomForce_Desired_Y = force_vector.y();
							p_sharedData->outputPhantomForce_Desired_Z = force_vector.z();

							// save one timestep
							saveOneTimeStep();
						}
						else // end of tracking
						{
							// set all forces to zero
							setOutputForceToZero();

							// send to record
							recordTrial();
							// set/start timer (from zero) and begin block of trials
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentStateNumber = RECORD;


						}


						break;








					case TRACK_BMI_DIRECTION:
						p_sharedData->experimentStateName = "Track BMI Direction";

						if (BMI_command_itorator < num_command_elements / 2) {
							//update current force to be output by BMI command updated at 1KHz

							// save one timestep
							saveOneTimeStep();

							float V_mag = sqrt(((velx_BCI[BMI_command_itorator]) *(velx_BCI[BMI_command_itorator])) + ((vely_BCI[BMI_command_itorator])*(vely_BCI[BMI_command_itorator])));
							if (V_mag != 0) {
								p_sharedData->velocity_MaxForce_scalar = p_sharedData->Fmax / V_mag;
							}
							else {
								p_sharedData->velocity_MaxForce_scalar = 0;
							}
							p_sharedData->scaled_velX_command = velx_BCI[BMI_command_itorator] * p_sharedData->velocity_MaxForce_scalar;
							p_sharedData->scaled_velY_command = vely_BCI[BMI_command_itorator] * p_sharedData->velocity_MaxForce_scalar;

							// increment itorator
							BMI_command_itorator = BMI_command_itorator + 1;

							// construct initial force vector to be rotated (in tool frame)
							force_vector = cVector3d(p_sharedData->scaled_velX_command, p_sharedData->scaled_velY_command, p_sharedData->outputNormalForce_Set);

							// update desired tool forces
							p_sharedData->outputPhantomForce_Desired_Tool_X = force_vector.x();
							p_sharedData->outputPhantomForce_Desired_Tool_Y = force_vector.y();
							p_sharedData->outputPhantomForce_Desired_Tool_Z = force_vector.z();

							// rotate tool to base frame
							force_vector = Rotate_Tool_to_Base_Frame(force_vector, p_sharedData->outputPhantomRotation);



							// command desired forces to phantom
							//send force to hapic device
							p_sharedData->outputPhantomForce_Desired_X = force_vector.x();
							p_sharedData->outputPhantomForce_Desired_Y = force_vector.y();
							p_sharedData->outputPhantomForce_Desired_Z = force_vector.z();


						}
						else // end of tracking
						{
							// set all forces to zero
							setOutputForceToZero();

							// send to record
							recordTrial();
							// set/start timer (from zero) and begin block of trials
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentStateNumber = RECORD;


						}


						break;


					case IDLE :
						p_sharedData->experimentStateName = "IDLE";

						//hold position (desired - actual)
						//read position of haptic device
						p_sharedData->p_output_Phantom->getPosition(position);

						// obtain desired force for spring control
						force = 1 * KP*(cVector3d(p_sharedData->outputZeroPosX, p_sharedData->outputZeroPosY, p_sharedData->outputZeroPosZ) - position);

						//send force to hapic device
						p_sharedData->outputPhantomForce_Desired_X = force.x();
						p_sharedData->outputPhantomForce_Desired_Y = force.y();
						p_sharedData->outputPhantomForce_Desired_Z = force.z();


						break;










/**********************************************************************************/
					// TRAINING STATE
					case TRAINING :
						p_sharedData->experimentStateName = "TRAINING";

						if (p_sharedData->timer->timeoutOccurred()) {

							//move to the preblock sequence which for block 1 is the "mock trials"
							p_sharedData->experimentStateNumber= PRE_BLOCK;
							p_sharedData->message = "Beginning Mock Experiment Block " + to_string(static_cast<long long>(p_sharedData->blockNum)) + " : " + p_sharedData->blockName + " in " + to_string(static_cast<long long>(preblockTime)) + " seconds.";
							p_sharedData->timer->setTimeoutPeriodSeconds(preblockTime);



						}
						break; // END: TRAINING STATE
/**********************************************************************************/
						// PRE BLOCK STATE
					case PRE_BLOCK :
						// set experiment state name
						p_sharedData->experimentStateName = "PRE_BLOCK";
						
						// wait for PRE_BLOCK timer to expire
						if (p_sharedData->timer->timeoutOccurred()) {
							
							// prepare for 1st trial
							p_sharedData->trialNum = 1;


							// set desired angle 
							p_sharedData->outputPhantomForce_Desired_Tool_angle_deg = 0;
						
							//

							// switch on what the next state is and setup
							switch (p_sharedData->nextExperimentStateNumber) {
								case PERCEPTION_EXPERIMENT_TRIAL :
									// reset curr_force_ind to iterate through 
									curr_force_ind = 0;

									// obtain current trials target number (index) 
									p_sharedData->outputPhantomForce_Desired_Tool_angle_deg = Randomized_stimulus_angle_array[p_sharedData->trialNum - 1];

									// set first force before jumping to next state
									// construct initial force vector to be rotated (in tool frame)
									force_vector = cVector3d(force_profiles[Randomized_stimulus_angleIndex_array[p_sharedData->trialNum - 1]][curr_force_ind][0], force_profiles[Randomized_stimulus_angleIndex_array[p_sharedData->trialNum - 1]][curr_force_ind][1], p_sharedData->outputNormalForce_Set);

									// update desired tool forces
									p_sharedData->outputPhantomForce_Desired_Tool_X = force_vector.x();
									p_sharedData->outputPhantomForce_Desired_Tool_Y = force_vector.y();
									p_sharedData->outputPhantomForce_Desired_Tool_Z = force_vector.z();

									// rotate tool to base frame
									force_vector = Rotate_Tool_to_Base_Frame(force_vector, p_sharedData->outputPhantomRotation);



									// command desired forces to phantom
									//send force to hapic device
									p_sharedData->outputPhantomForce_Desired_X = force_vector.x();
									p_sharedData->outputPhantomForce_Desired_Y = force_vector.y();
									p_sharedData->outputPhantomForce_Desired_Z = force_vector.z();

								break;




							}
					
		

							// Initialize cursor state
							//initializeCursorState();		

							// set/start timer (from zero) and begin block of trials, uncomment for Stretch validation trials
						/*	p_sharedData->timer->setTimeoutPeriodSeconds(stretchTime_Validation);
							p_sharedData->timer->start(true);
							p_sharedData->experimentStateNumber = TEST_FORCE;
							*/

							//p_sharedData->experimentStateNumber = TRACK_BMI;
							//p_sharedData->experimentStateNumber = TRACK_BMI_DIRECTION;

							p_sharedData->experimentStateNumber = p_sharedData->nextExperimentStateNumber;

						}
						break; // END: PRE BLOCK STATE

/**********************************************************************************/

						// Perception Experiment
					case PERCEPTION_EXPERIMENT_TRIAL:
						// update experiment name
						p_sharedData->experimentStateName = "PERCEPTION EXP. STATE";


						// save data from time step
						p_sharedData->timeElapsed = p_sharedData->timer->getCurrentTimeSeconds();


						saveOneTimeStep();
						if (p_sharedData->trialNum <= num_perception_trials) {
							if (curr_force_ind < num_force_ind) {
								force_vector = cVector3d(force_profiles[Randomized_stimulus_angleIndex_array[p_sharedData->trialNum - 1]][curr_force_ind][0], force_profiles[Randomized_stimulus_angleIndex_array[p_sharedData->trialNum - 1]][curr_force_ind][1], p_sharedData->outputNormalForce_Set);

								// update desired tool forces
								p_sharedData->outputPhantomForce_Desired_Tool_X = force_vector.x();
								p_sharedData->outputPhantomForce_Desired_Tool_Y = force_vector.y();
								p_sharedData->outputPhantomForce_Desired_Tool_Z = force_vector.z();

								// rotate tool to base frame
								force_vector = Rotate_Tool_to_Base_Frame(force_vector, p_sharedData->outputPhantomRotation);



								// command desired forces to phantom
								//send force to hapic device
								p_sharedData->outputPhantomForce_Desired_X = force_vector.x();
								p_sharedData->outputPhantomForce_Desired_Y = force_vector.y();
								p_sharedData->outputPhantomForce_Desired_Z = force_vector.z();

								curr_force_ind = curr_force_ind + 1;
							}
							else {
								recordTrial();

								// set/start timer (from zero) and send to record state
								p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
								p_sharedData->timer->start(true);
								p_sharedData->experimentStateNumber = RECORD;
								p_sharedData->experimentStateName = "RECORD";
								p_sharedData->nextExperimentStateNumber = PERCEPTION_EXPERIMENT_TRIAL;


							}
						}
						else {

							// thank subject and terminate experiment
							p_sharedData->message = "Thank you.";
							p_sharedData->experimentStateNumber = THANKS;
							p_sharedData->experimentStateName = "THANKS";
							closeExperiment();
							}

						break;



/**********************************************************************************/
						// START RECORD STATE
					case RECORD:
						// update experiment state name 
						p_sharedData->experimentStateName = "RECORDING";
						
						// wait for time to expire (more than enough to record data)
						if (p_sharedData->timer->timeoutOccurred()) {
							
							
							/*
							// If it is time for a relax
							if((p_sharedData->trialNum % trialsBeforeBreak) == 0){
								p_sharedData->experimentStateNumber = RELAX;
								p_sharedData->message =  "Please Take a break";

								// set/start timer (from zero) and return to block
								p_sharedData->timer->setTimeoutPeriodSeconds(relaxTime);
								p_sharedData->timer->start(true);
								
							}
							

							else{ // it is time to go to next trial
							//initializeCursorState();

							// set/start timer (from zero) and return to block
							p_sharedData->timer->setTimeoutPeriodSeconds(trialTime);
							p_sharedData->timer->start(true);

							p_sharedData->experimentStateNumber = EXPERIMENT;

							}
							*/

							// set/start timer (from zero) and return to block


							switch (p_sharedData->nextExperimentStateNumber) {
								case PERCEPTION_EXPERIMENT_TRIAL :

									// prep for next trial
									(p_sharedData->trialNum)++;

									// reset current force index and increment the trial number index
									curr_force_ind = 0;

									// obtain current trials target number (index) 
									p_sharedData->outputPhantomForce_Desired_Tool_angle_deg = Randomized_stimulus_angle_array[p_sharedData->trialNum - 1];
								break;

							}

							// if finished, send to thank you state, uncomment for Test_Force State Machine
/*							if (p_sharedData->outputPhantomForce_Desired_Tool_angle_deg >= 360) {
								p_sharedData->experimentStateNumber = THANKS;

							}
							else {
								p_sharedData->timer->setTimeoutPeriodSeconds(stretchTime_Validation);
								p_sharedData->timer->start(true);
								p_sharedData->experimentStateNumber = TEST_FORCE;
							}
							*/


							p_sharedData->experimentStateNumber = THANKS;
						}
						break; // END RECORD STATE
					
					
/**********************************************************************************/	
					// START RELAX STATE

					case RELAX:

						// update experiment state name
						p_sharedData->experimentStateName = "RELAX";

						// check that timeout has occured for relaxation and return to next trial after relax_to_trial_time
						if (p_sharedData->timer->timeoutOccurred()){
							p_sharedData->message =  "Press button when ready. Trial will start 5 seconds afterwards.";
							if (p_sharedData->inputPhantomSwitch == 1) {
								userReady = 1;
								p_sharedData->inputPhantomSwitch = 0;
								p_sharedData->timer->setTimeoutPeriodSeconds(relax_to_trial_time);
								p_sharedData->timer->start(true);
							}
						}

							if ((p_sharedData->timer->timeoutOccurred()) && (userReady)){
								userReady = 0;
								//initializeCursorState();
								
								// set/start timer (from zero) and return to block
								p_sharedData->timer->setTimeoutPeriodSeconds(trialTime);
								p_sharedData->timer->start(true);

								p_sharedData->experimentStateNumber = EXPERIMENT;
							}

					break;

/**********************************************************************************/	
					// START BREAK STATE
					case BREAK:
						// update experiment state name
						p_sharedData->experimentStateName = "BREAK";
						
						// check if break is over
						if (p_sharedData->timer->timeoutOccurred()) {

							// set/start timer (from zero) and send directly to preBlock
							p_sharedData->timer->setTimeoutPeriodSeconds(preblockTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentStateNumber = PRE_BLOCK;
						}
						break; // END BREAK STATE
					
					
/**********************************************************************************/
						// START EXPERIMENT STATE
						case EXPERIMENT:
							// update experiment name
							p_sharedData->experimentStateName = "EXPERIMENT";


							// save data from time step
							p_sharedData->timeElapsed = p_sharedData->timer->getCurrentTimeSeconds();

							if (p_sharedData->trialNum <= trialsPerExperimentBlock) // if the trial is within the block save data
							{
								saveOneTimeStep();
							}

						// check if the Experiment block is complete (all trials completed)
						if (p_sharedData->trialNum > trialsPerExperimentBlock) {
                       
							// give subject a break before Experiment block
							p_sharedData->message = "Break: " + to_string(static_cast<long long>(breakTime)) + " seconds until next experiment block.";
                        
							// set/start timer (from zero) send to break with break time
							p_sharedData->timer->setTimeoutPeriodSeconds(breakTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentStateNumber = BREAK;



							// Iterate to next block type and parameters
							blockNumberIndex++;

							if(blockNumberIndex > numberOfBlocks){
								
								// thank subject and terminate experiment
								p_sharedData->message = "Thank you.";
								p_sharedData->experimentStateNumber = THANKS;
								p_sharedData->experimentStateName = "THANKS";
								closeExperiment();
								
							}


						} else {
							// if switch is pressed, denote as end of trial
							if((p_sharedData->inputPhantomSwitch == 1) || (p_sharedData->joystickSwitch == 1)) {
							
								// turn off switch
								p_sharedData->inputPhantomSwitch = 0;
								p_sharedData->message = "You pressed a button!";

								// start recording trial data
								saveOneTimeStep();
								recordTrial();

								// set/start timer (from zero) and send to record state
								p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
								p_sharedData->timer->start(true);
								p_sharedData->experimentStateNumber = RECORD;



							}

							// unsuccessful trial (i.e., time expired)
							if (p_sharedData->timer->timeoutOccurred()) {
							
								p_sharedData->message = "Time expired!";
                        
								// start recording trial data
								saveOneTimeStep();
								recordTrial();
							
								// set/start timer (from zero) and send to record state
								p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
								p_sharedData->timer->start(true);
								p_sharedData->experimentStateNumber = RECORD;
							}
						}
						break; // END EXPERIMENT STATE


/**********************************************************************************/

						case THANKS :
							p_sharedData->message = "Thank You!";
						break;


/**********************************************************************************/
					
				}

				
			} 
//*********************END OF EXPERIMENT STATE MACHINE ****************************

//******************************DEMO LOOP******************************************
			if (p_sharedData->opMode==DEMO){
				// Begin State Machine
				switch (p_sharedData->demoStateNumber) {

					/**********************************************************************************/
				case START_UP_DEMO:
					// prompt user to define zero location
					p_sharedData->message = "Waiting on Keypress to select next State ";
					p_sharedData->demoStateName = "START UP DEMO";
					// key press in graphics thread will send state machine to next state once zero location set

					break;
					/**********************************************************************************/


				case FORCE_SENSOR_TESTING_DEMO:

					// set demoState number and name
					p_sharedData->demoStateName = "Force Sensor Testing";

					if (!p_sharedData->ZeroPhantomForce_FLAG) {

						p_sharedData->outputPhantomForce_Desired_X = 10 * p_sharedData->cursorPosZ;
						p_sharedData->outputPhantomForce_Desired_Y = 10 * p_sharedData->cursorPosY;
						p_sharedData->outputPhantomForce_Desired_Z = 0; // 10 * p_sharedData->cursorPosZ;
				}

					break;
/**********************************************************************************/



				case INPUT_PROMPT_DEMO : 

					p_sharedData->demoStateName = "INPUT PROMPT DEMO";
					//  scale force command of input phantom to 2d shear force
					// define vector to obtain current input phantom position
					// Desired force output

					p_sharedData->outputPhantomForce_Desired_X = -10*p_sharedData->cursorPosX;
					p_sharedData->outputPhantomForce_Desired_Y = 10*p_sharedData->cursorPosY;
					p_sharedData->outputPhantomForce_Desired_Z = 10*p_sharedData->cursorPosZ;


				break;




/**********************************************************************************/
				case HOLD_ZERO_POINT_DEMO :
					
					//read position of haptic device
					p_sharedData->p_output_Phantom->getPosition(position);

					// obtain desired force for spring control
					force = -1*KP*position;

					//send force to hapic device
					p_sharedData->p_output_Phantom->setForce(force);

					

					
				break;





				}



			}
//****************************** END DEMO LOOP***************************************

			// restart experiment loop timer            
			p_sharedData->m_expLoopTimer.start(true);
			lastTime = currTime;
		}
    }
     
}



// terminate the experiment
void closeExperiment(void) {
    
    p_sharedData->experimentStateNumber = THANKS;
    if (p_sharedData->outputFile != NULL) fclose(p_sharedData->outputFile);
    
}

// Set all velocities and positions to zero (for cursor)
void initializeCursorState(void){
	p_sharedData->cursorPosX = 0;
	p_sharedData->cursorPosY = 0;
	p_sharedData->cursorPosZ = 0;
	
	p_sharedData->cursorPosX_OneAgo = 0;
	p_sharedData->cursorPosY_OneAgo = 0;
	p_sharedData->cursorPosZ_OneAgo = 0;

	p_sharedData->cursorVelX = 0;
	p_sharedData->cursorVelY = 0;
	p_sharedData->cursorVelZ = 0;
}




void setOutputForceToZero(void) {
	p_sharedData->outputPhantomForce_Desired_X = 0;
	p_sharedData->outputPhantomForce_Desired_Y = 0;
	p_sharedData->outputPhantomForce_Desired_Z = 0;

}





void setup_Perception_Force_Profiles(void) {

	/*const int delta_angle_deg = 30; // degrees of perception angle wedges
	const int num_single_stimulus_application = 3; // number of times a unique stimulus is applied
	const int delta_angle_deg = 30; // degrees of perception angle wedges
	const int num_wedges = 360/delta_angle_deg;
	const int num_perception_trials = num_wedges*num_single_stimulus_application; // number of total perception trials
	const int Randomized_stimulus_angle_array[num_perception_trials]; // this array will hold the randomized trial progression
	const int num_wedges = 360 / delta_angle_deg;
	const int ramp_duration_ms = 1000; // 1 second ramp
	const int delta_time_ms = 1; // 1 ms
	const int num_force_ind = ramp_duration_ms / delta_time_ms;
	float perception_angles_tf[num_wedges];
	float force_profiles[num_wedges][num_force_ind][2];
	*/

	// fill in perception angles array
	perception_angles_tf[0] = 0; // initialize first angle to zero degrees
	for (int i = 1; i < num_wedges; i++) {
		perception_angles_tf[i] = perception_angles_tf[i - 1] + delta_angle_deg;
	}

	float delta_F = p_sharedData->Fmax / num_force_ind;

	for (int i = 0; i < num_wedges; i++) {
		double theta_deg = perception_angles_tf[i];
		// rotate unit vector by theta
		R_z_perception.set(cos(theta_deg*PI / 180), -sin(theta_deg*PI / 180), 0, sin(theta_deg*PI / 180), cos(theta_deg*PI / 180), 0, 0, 0, 1);
		for (int j = 0; j < num_force_ind; j++) {
			double currForceMag = j*delta_F;
			cVector3d temp;
			temp = R_z_perception*(currForceMag*unitVect_zero_deg);
			force_profiles[i][j][0] = temp.x();
			force_profiles[i][j][1] = temp.y();
		}
	}

	// randomization of perception cueues 
	randomizeTargets();



}









void randomizeTargets(void) {
	for (int j = 0; j < num_single_stimulus_application; j++) {
		// Temporary randomized array with every target number
		int tempRandTargetArray[num_wedges];
		int k = 0;
		for (k = 0; k < num_wedges; k++) {
			tempRandTargetArray[k] = perception_angles_tf[k];
		}
		srand(time(NULL));
		randomize(tempRandTargetArray, num_wedges);

		// now fill the corresponding Randomized_stimulus_angle_array
		for (k = 0; k < num_wedges; k++) {
			Randomized_stimulus_angle_array[j*num_wedges+k] = tempRandTargetArray[k];
			Randomized_stimulus_angleIndex_array[j*num_wedges + k] = k;
		}

	}


	printf("RandTargets: ");
	int i = 0;
	for (i = 0; i < num_perception_trials; i++) printf(" %d ", Randomized_stimulus_angle_array[i]);

}




// A utility function to swap to integers
void swap(int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}



// A function to generate a random permutation of arr[]
void randomize(int arr[], int n)
{
	// Use a different seed value so that we don't get same
	// result each time we run this program
	srand(time(NULL));

	// Start from the last element and swap one by one. We don't
	// need to run for the first element that's why i > 0
	for (int i = n - 1; i > 0; i--)
	{
		// Pick a random index from 0 to i
		int j = rand() % (i + 1);

		// Swap arr[i] with the element at random index
		swap(&arr[i], &arr[j]);
	}
}