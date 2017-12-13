
#include "experiment.h"
#include <math.h>
#include <cmath>
#include <random>
#include <ctime>
#include <vector>
#include <algorithm>

using namespace chai3d;
using namespace std;

//#define DEBUG 

/*
// Data variables we need to record during the record step
cVector3d cursor_pos; //temporary variable for haptic tool cursor position
cVector3d input_pos;  // temporary variable for input PHANTOM position 
cVector3d input_vel;  // temporary variable for input PHANTOM velocity 
cVector3d output_pos;  // temporary variable for output PHANTOM position 
cVector3d output_vel;  // temporary variable for output PHANTOM velocity 
cVector3d output_force; // temp var for output PHANTOM currently output force
*/

// Experiment State Machine params
#define TRIALS_TRG 8
#define TRIALS_EXP 20

int trialsPerExperimentBlock = TRIALS_TRG;	  // trials per experiment block, initialize as training first.
static const int trialsBeforeBreak = 5;
static const int trialTime = 30;                  // max time per trial or washout [sec]
static const int breakTime = 15;                 // break time [sec] between blocks
static const int preblockTime = 10;               // time to display message [sec]
static const int trainingTime = 60;
static const int recordTime = 1;                  // time to record data [sec]
static const int relaxTime = 10;				// time to relax between large strings of trials within same block
static const int relax_to_trial_time = 2;       // Message prompt in seconds before starting trial after relaxation, requires button press to continue
static const int numberOfBlocks = 2;




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

    // get subject number, control paradigm, and session number
    printf("\nEnter subject number: ");
    cin >> subjectNum;
    printf("\nEnter session number: ");
    cin >> session;
    printf("\n");
       
    // generate filename and open file for writing
    sprintf(filename, "Subj_%d_Session_%d.dat", subjectNum, session);
    p_sharedData->outputFile = fopen(filename,"w");
    fprintf(p_sharedData->outputFile, "blockNum, trialNum, cursorPosX, cursorPosY, cursorPosZ, cursorPosX_OneAgo, cursorPosY_OneAgo, cursorPosZ_OneAgo, cursorVelX, cursorVelY, cursorVelZ, inputPhantomPosX, inputPhantomPosY, inputPhantomPosZ, inputPhantomVelX, inputPhantomVelY, inputPhantomVelZ, inputPhantomSwitch, outputPhantomPosX, outputPhantomPosY, outputPhantomPosZ, outputPhantomVelX, outputPhantomVelY, outputPhantomVelZ, outputPhantomSwitch, outputPhantomForce_X, outputPhantomForce_Y, outputPhantomForce_Z, joystickPosX, joystickPosY, joystickSwitch, phantomLoopTimeStamp, joystickLoopTimeStamp, experimentLoopTimeStamp,recordTimeStamp, phantomLoopDelta, joystickLoopDelta, experimentLoopDelta, phantomFreq, joystickFreq, experimentFreq, timeElapsed\n");
    
    // enter start-up mode, with force feedback off for safety
   	p_sharedData->opMode = EXPERIMENT;
    p_sharedData->experimentStateNumber = START_UP;
    p_sharedData->message = "Welcome. Press any key to proceed to training.";
    
	// initialize experiment loop timer
	p_sharedData->m_expLoopTimer.setTimeoutPeriodSeconds(LOOP_TIME);
	p_sharedData->m_expLoopTimer.start(true);

	//initialize random seed for randomization of trials
	srand(timeGetTime());

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

			/*
			//calculate the cursorVel
			if (delta!=0)
				{
					p_sharedData->cursorVelX = (p_sharedData->cursorPosX-p_sharedData->cursorPosX_OneAgo)/delta;
					p_sharedData->cursorVelY = (p_sharedData->cursorPosY-p_sharedData->cursorPosY_OneAgo)/delta;
					p_sharedData->cursorVelZ = (p_sharedData->cursorPosZ-p_sharedData->cursorPosZ_OneAgo)/delta;
				}
			else
				{
					p_sharedData->cursorVelX = p_sharedData->cursorPosX-p_sharedData->cursorPosX_OneAgo;
					p_sharedData->cursorVelY = p_sharedData->cursorPosY-p_sharedData->cursorPosY_OneAgo;
					p_sharedData->cursorVelZ = p_sharedData->cursorPosZ-p_sharedData->cursorPosZ_OneAgo;
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

			// get OUTPUT PHANTOM position and velocity vectors
            p_sharedData->p_output_Phantom->getPosition(output_pos);
            p_sharedData->p_output_Phantom->getLinearVelocity(output_vel);

			//store position values into respective variable in sharedData structure
			p_sharedData->outputPhantomPosX = output_pos.x();
			p_sharedData->outputPhantomPosY = output_pos.y();
			p_sharedData->outputPhantomPosZ = output_pos.z();

			// store velocity values into respective vars in sharedData structure
            p_sharedData->outputPhantomVelX = output_vel.x();
			p_sharedData->outputPhantomVelY = output_vel.y();
			p_sharedData->outputPhantomVelZ = output_vel.z();

*/


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


						// display All measured forces from Nano17


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

						

						

							//initialize noise standard deviation
							if(blockNumberIndex==1){/*do nothing to current sigma*/}
		

							// Initialize cursor state
							//initializeCursorState();		

							// set/start timer (from zero) and begin block of trials
							p_sharedData->timer->setTimeoutPeriodSeconds(trialTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentStateNumber = EXPERIMENT;

						}
						break; // END: PRE BLOCK STATE

/**********************************************************************************/

						// START RECORD STATE
					case RECORD:
						// update experiment state name 
						p_sharedData->experimentStateName = "RECORDING";
						
						// wait for time to expire (more than enough to record data)
						if (p_sharedData->timer->timeoutOccurred()) {
							
							

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

							// prep for next trial
							(p_sharedData->trialNum)++;

							//only iterate to the next hole if we haven't reach the end of the experiment, otherwise we will get an error.
							if (p_sharedData->trialNum<trialsPerExperimentBlock)
							{
								



							}
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




