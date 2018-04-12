
#include "graphics.h"
#include <math.h>
#include "shared_data.h"
#include "Phantom.h"
using namespace chai3d;
using namespace std;

static bool DEBUG_DISPLAYS = true; // if true then all debug labels will be displayed

static const int Tgraphics = 20;  // minimum time between graphic updates [msec] = 50 Hz

static int screenW;              // screen width (for relative window sizing/positioning)
static int screenH;              // screen height
static int windowW;              // graphics window width
static int windowH;              // graphics window height
static int windowPosX;           // graphics window X-coordinate
static int windowPosY;           // graphics window Y-coordinate
static bool fullscreen = false;  // for toggling in/out of fullscreen

static cWorld* world;             // CHAI world
static cCamera* camera;           // camera to render the world
static cDirectionalLight* light;  // light to illuminate the world

// Labels for display 
static cLabel* opMode;            // label to display simulation operating mode
static cLabel* input_device;      // label to display input device
static cLabel* output_device;	  // label to display output device
static cLabel* phantomRate;       // label to display PHANTOM rate

static cLabel* experimentState;
static cLabel* blockType;
static cLabel* trial;             // label to display trial count
static cLabel* message;           // label to display message to user

// output Phantom forces via GetForce()
static cLabel* XForce;			  // label to display the x force
static cLabel* YForce;			  // label to display the y force
static cLabel* ZForce;			  // label to display the z force

// desired output force for output Phantom
static cLabel* XForce_Desired;
static cLabel* YForce_Desired;
static cLabel* ZForce_Desired;

// desired output force in tool frame
static cLabel* XForce_Desired_Tool;
static cLabel* YForce_Desired_Tool;
static cLabel* ZForce_Desired_Tool;

// measured force sensor readings
static cLabel* XForce_Measured;
static cLabel* YForce_Measured;
static cLabel* ZForce_Measured;

static cLabel* cursorPosX;		// label to display cursor x position
static cLabel* cursorPosY;		// label to display cursor y position
static cLabel* cursorPosZ;		// label to display cursor z position

static cLabel* cursorVelX;
static cLabel* cursorVelY;
static cLabel* cursorVelZ;

// phantom positions
static cLabel* input_phantomPosX;
static cLabel* input_phantomPosY;
static cLabel* input_phantomPosZ;

static cLabel* output_phantomPosX;
static cLabel* output_phantomPosY;
static cLabel* output_phantomPosZ;

static cLabel* outputPhantomForce_Desired_Tool_angle_deg;



static shared_data* p_sharedData;  // structure for sharing data between threads (NOTE: for graphics,

// point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedDataToGraphics(shared_data& sharedData) {
    
    p_sharedData = &sharedData;
    
}

// initialize graphics before starting the main graphics rendering loop
void initGraphics(int argc, char* argv[]) {
    
    // initialize GLUT & GLEW
    glutInit(&argc, argv);
    
    // size/position graphics window
    screenW = glutGet(GLUT_SCREEN_WIDTH);
    screenH = glutGet(GLUT_SCREEN_HEIGHT);
    windowW = (int)(0.8 * screenH);
    windowH = (int)(0.5 * screenH);
    windowPosY = (screenH - windowH) / 2;
    windowPosX = windowPosY;
    glutInitWindowPosition(windowPosX, windowPosY);
    glutInitWindowSize(windowW, windowH);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow(argv[0]);
    glutSetWindowTitle("Slip/Shear Perception");
    if (fullscreen) glutFullScreen();
    
    // initialize GLEW library (must happen after window creation)
    glewInit();
    
    // set up GLUT event handlers
    glutDisplayFunc(updateGraphics);
    glutKeyboardFunc(respToKey);
    glutReshapeFunc(resizeWindow);
    
    // create a black world
    world = new cWorld();
    world->m_backgroundColor.setBlack();
    
    // position/orient the camera in the world
    camera = new cCamera(world);
    world->addChild(camera);

    camera->set(cVector3d (1.0, 0.0, -0.003),   // camera position
                cVector3d (0.0, 0.0, 0.0),   // look at center
                cVector3d (0.0, 0.0, 1.0));  // "up" vector
				

    camera->setClippingPlanes(0.01, 100.0);

    // enable multi-pass rendering to handle transparent objects
    camera->setUseMultipassTransparency(true);

    // add the light source
    light = new cDirectionalLight(world);
    world->addChild(light);
    light->setEnabled(true);
    light->setDir(-3.0, -0.5, 0.0);  

	// create new haptic tool cursor
	p_sharedData->tool = new cToolCursor(world);
	world->addChild(p_sharedData->tool);
	p_sharedData->tool->setHapticDevice(p_sharedData->p_input_Phantom);
	p_sharedData->tool->setWorkspaceRadius(WORKSPACE_RADIUS);
	p_sharedData->tool->setRadius(CURSOR_SIZE);
	//p_sharedData->tool->m_material->setGreenChartreuse();
	p_sharedData->tool->setShowContactPoints(false,false); //show the actual position and not the god particle
	p_sharedData->tool->m_hapticPoint->m_sphereProxy->m_material->setGreenChartreuse();

	
	//create virtual cursor that shadows the haptic tool
	p_sharedData->vCursor= new cShapeSphere(CURSOR_SIZE);
	world->addChild(p_sharedData->vCursor);
	p_sharedData->vCursor->m_material->setRedCrimson();

	//initialize the tool once the phantom has been initialized
	p_sharedData->tool->start();


    // create labels
    cFont* font = NEW_CFONTCALIBRI20();
    opMode = new cLabel(font);
    input_device = new cLabel(font);
	output_device = new cLabel(font);
    phantomRate = new cLabel(font);
    trial = new cLabel(font);
    message = new cLabel(font);
	experimentState = new cLabel(font);
	blockType = new cLabel(font);

	input_phantomPosX = new cLabel(font);
	input_phantomPosY = new cLabel(font);
	input_phantomPosZ = new cLabel(font);

	// Output Phantom forces from GetForce()
	XForce = new cLabel(font);
	YForce = new cLabel(font);
	ZForce = new cLabel(font);

	// Output Phantom desired forces
	XForce_Desired = new cLabel(font);
	YForce_Desired = new cLabel(font);
	ZForce_Desired = new cLabel(font);

	// output phantom desired forces in tool frame
	XForce_Desired_Tool = new cLabel(font);
	YForce_Desired_Tool = new cLabel(font);
	ZForce_Desired_Tool = new cLabel(font);

	XForce_Measured = new cLabel(font);
	YForce_Measured = new cLabel(font);
	ZForce_Measured = new cLabel(font);

	cursorPosX = new cLabel(font);
	cursorPosY = new cLabel(font);
	cursorPosZ = new cLabel(font);

	output_phantomPosX = new cLabel(font);
	output_phantomPosY = new cLabel(font);
	output_phantomPosZ = new cLabel(font);

	outputPhantomForce_Desired_Tool_angle_deg = new cLabel(font);

	// add labels to frontLayer
	//-----------------------------------------------
    camera->m_frontLayer->addChild(opMode);
	camera->m_frontLayer->addChild(experimentState);
    camera->m_frontLayer->addChild(input_device);
	camera->m_frontLayer->addChild(output_device);
    camera->m_frontLayer->addChild(phantomRate);
    camera->m_frontLayer->addChild(trial);
    camera->m_frontLayer->addChild(message);
	camera->m_frontLayer->addChild(XForce);
	camera->m_frontLayer->addChild(YForce);
	camera->m_frontLayer->addChild(ZForce);
	camera->m_frontLayer->addChild(XForce_Desired);
	camera->m_frontLayer->addChild(YForce_Desired);
	camera->m_frontLayer->addChild(ZForce_Desired);
	camera->m_frontLayer->addChild(XForce_Desired_Tool);
	camera->m_frontLayer->addChild(YForce_Desired_Tool);
	camera->m_frontLayer->addChild(ZForce_Desired_Tool);
	camera->m_frontLayer->addChild(XForce_Measured);
	camera->m_frontLayer->addChild(YForce_Measured);
	camera->m_frontLayer->addChild(ZForce_Measured);
	camera->m_frontLayer->addChild(cursorPosX);
	camera->m_frontLayer->addChild(cursorPosY);
	camera->m_frontLayer->addChild(cursorPosZ);
	camera->m_frontLayer->addChild(input_phantomPosX);
	camera->m_frontLayer->addChild(input_phantomPosY);
	camera->m_frontLayer->addChild(input_phantomPosZ);



	camera->m_frontLayer->addChild(output_phantomPosX);
	camera->m_frontLayer->addChild(output_phantomPosY);
	camera->m_frontLayer->addChild(output_phantomPosZ);

	camera->m_frontLayer->addChild(outputPhantomForce_Desired_Tool_angle_deg);



	// display keyboard control options
	printf("\n\n*****************************************\n");
	printf("f = fullscreen toggle\n");
	printf("x = (Toggle) Zero Phantom Output Forces\n");
	printf("7 =  Decrement Tool Y force\n");
	printf("9 =  Increment Tool Y force\n");
	printf("8 =  Increment Tool Z\n");
	printf("2 =  Decrement Tool Z\n");
	printf("4 =  Decrement Tool X\n");
	printf("2 =  Increment Tool X\n");
	printf("Q/ESC = quit\n");
	printf("*****************************************\n\n");
}

// update and re-render the graphics
void updateGraphics(void) {
    
	//compute global reference frames for each object
	world->computeGlobalPositions(true);


    // update labels
	// operation mode
	switch (p_sharedData->opMode){
		case DEMO : 
			opMode->setString("Mode: DEMO");
		break;

		case EXPERIMENT :
			opMode->setString("Mode: EXPERIMENT");
		break;
	}

	// input device
	switch (p_sharedData->input_device){
		case INPUT_PHANTOM :
			input_device->setString("Input: PHANTOM");
		break;

		case INPUT_JOYSTICK : 
			input_device->setString("Input: JOYSTICK");
		break;
	}

	// output device
	switch (p_sharedData->output_device){
		case OUTPUT_PHANTOM :
			output_device->setString("Output: PHANTOM");
		break;

		case OUTPUT_DELTA :
			output_device->setString("Output: DELTA");
		break;

	}

    phantomRate->setString("PHANTOM: " + to_string(static_cast<long long>(p_sharedData->phantomFreqCounter.getFrequency())));

    trial->setString("Trial: " + to_string(static_cast<long long>(p_sharedData->trialNum)));
	if (p_sharedData->opMode == EXPERIMENT) {
		experimentState->setString("Experiment State: " + p_sharedData->experimentStateName);
	}
	else {
		experimentState->setString("Demo State: " + p_sharedData->demoStateName);
	}
	blockType->setString("Block Name: " + p_sharedData->blockName);


	XForce->setString("XForce: " + to_string(static_cast<long double>(p_sharedData->outputPhantomForce_X)));
	YForce->setString("YForce: " + to_string(static_cast<long double>(p_sharedData->outputPhantomForce_Y)));
	ZForce->setString("ZForce: " + to_string(static_cast<long double>(p_sharedData->outputPhantomForce_Z)));

	XForce_Desired->setString("XForce Des: " + to_string(static_cast<long double>(p_sharedData->outputPhantomForce_Desired_X)));
	YForce_Desired->setString("YForce Des: " + to_string(static_cast<long double>(p_sharedData->outputPhantomForce_Desired_Y)));
	ZForce_Desired->setString("ZForce Des: " + to_string(static_cast<long double>(p_sharedData->outputPhantomForce_Desired_Z)));

	XForce_Desired_Tool->setString("XForce Des_Tool: " + to_string(static_cast<long double>(p_sharedData->outputPhantomForce_Desired_Tool_X)));
	YForce_Desired_Tool->setString("YForce Des_Tool: " + to_string(static_cast<long double>(p_sharedData->outputPhantomForce_Desired_Tool_Y)));
	ZForce_Desired_Tool->setString("ZForce Des_Tool: " + to_string(static_cast<long double>(p_sharedData->outputPhantomForce_Desired_Tool_Z)));

	XForce_Measured->setString("XForce Measured: " + to_string(static_cast<long double>(p_sharedData->force[0])));
	YForce_Measured->setString("YForce Measured: " + to_string(static_cast<long double>(p_sharedData->force[1])));
	ZForce_Measured->setString("ZForce Measured: " + to_string(static_cast<long double>(p_sharedData->force[2])));



    cursorPosX->setString("CursorPos X: " + to_string(static_cast<long double>(p_sharedData->cursorPosX)));
	cursorPosY->setString("CursorPos Y: " + to_string(static_cast<long double>(p_sharedData->cursorPosY)));
	cursorPosZ->setString("CursorPos Z: " + to_string(static_cast<long double>(p_sharedData->cursorPosZ)));

	input_phantomPosX->setString("Input Phantom Pos X: " + to_string(static_cast<long double>(p_sharedData->inputPhantomPosX)));
	input_phantomPosY->setString("Input Phantom Pos Y: " + to_string(static_cast<long double>(p_sharedData->inputPhantomPosY)));
	input_phantomPosZ->setString("Input Phantom Pos Z: " + to_string(static_cast<long double>(p_sharedData->inputPhantomPosZ)));
	
	outputPhantomForce_Desired_Tool_angle_deg->setString("Desired Tool Angle: " + to_string(static_cast<long double>(p_sharedData->outputPhantomForce_Desired_Tool_angle_deg)));

	int i = 1;
	opMode->setLocalPos(10, (int)(windowH - i * opMode->getHeight()), 0); i++;
    input_device->setLocalPos(10, (int) (windowH - i * input_device->getHeight()), 0); i++;
	output_device->setLocalPos(10, (int) (windowH - i * output_device->getHeight()), 0); i++;
    phantomRate->setLocalPos(10, (int) (windowH - i * phantomRate->getHeight()), 0); i++;
   	
	XForce->setLocalPos(10, (int) (windowH - i * XForce->getHeight()), 0); i++;
	YForce->setLocalPos(10, (int) (windowH - i * YForce->getHeight()), 0); i++;
	ZForce->setLocalPos(10, (int) (windowH - i * ZForce->getHeight()), 0); i++;

	XForce_Desired->setLocalPos(10, (int)(windowH - i * XForce_Desired->getHeight()), 0); i++;
	YForce_Desired->setLocalPos(10, (int)(windowH - i * YForce_Desired->getHeight()), 0); i++;
	ZForce_Desired->setLocalPos(10, (int)(windowH - i * ZForce_Desired->getHeight()), 0); i++;

	XForce_Desired_Tool->setLocalPos(10, (int)(windowH - i * XForce_Desired_Tool->getHeight()), 0); i++;
	YForce_Desired_Tool->setLocalPos(10, (int)(windowH - i * YForce_Desired_Tool->getHeight()), 0); i++;
	ZForce_Desired_Tool->setLocalPos(10, (int)(windowH - i * ZForce_Desired_Tool->getHeight()), 0); i++;

	XForce_Measured->setLocalPos(10, (int)(windowH - i * XForce_Measured->getHeight()), 0); i++;
	YForce_Measured->setLocalPos(10, (int)(windowH - i * YForce_Measured->getHeight()), 0); i++;
	ZForce_Measured->setLocalPos(10, (int)(windowH - i * ZForce_Measured->getHeight()), 0); i++;

	cursorPosX->setLocalPos(10, (int) (windowH - i * cursorPosX->getHeight()),0); i++;
	cursorPosY->setLocalPos(10, (int) (windowH - i * cursorPosY->getHeight()),0); i++;
	cursorPosZ->setLocalPos(10, (int) (windowH - i * cursorPosZ->getHeight()),0); i++;

	input_phantomPosX->setLocalPos(10, (int) (windowH - i * input_phantomPosX->getHeight()),0); i++;
	input_phantomPosY->setLocalPos(10, (int) (windowH - i * input_phantomPosY->getHeight()),0); i++;
	input_phantomPosZ->setLocalPos(10, (int) (windowH - i * input_phantomPosZ->getHeight()),0); i++;

	outputPhantomForce_Desired_Tool_angle_deg->setLocalPos(10, (int)(windowH - i * input_phantomPosX->getHeight()), 0); i++;


	trial->setLocalPos((int) (windowW - trial->getWidth() - 10), (int) (windowH - 2.0 * trial->getHeight()), 0);
	
	blockType->setLocalPos((int) (windowW - blockType->getWidth() - 10), (int) (0), 0); i++;
        
	// update message to user
    message->setString(p_sharedData->message);
    message->setLocalPos((int) (0.5 * (windowW - message->getWidth())), (int) (0.95 * (windowH - message->getHeight())), 0);  // center of window
    
	opMode->setShowEnabled(DEBUG_DISPLAYS);
    input_device->setShowEnabled(DEBUG_DISPLAYS);
	phantomRate->setShowEnabled(DEBUG_DISPLAYS);
	trial->setShowEnabled(DEBUG_DISPLAYS);
	input_phantomPosX->setShowEnabled(DEBUG_DISPLAYS);
	input_phantomPosY->setShowEnabled(DEBUG_DISPLAYS);
	input_phantomPosZ->setShowEnabled(DEBUG_DISPLAYS);
	XForce->setShowEnabled(DEBUG_DISPLAYS);
	YForce->setShowEnabled(DEBUG_DISPLAYS);
	ZForce->setShowEnabled(DEBUG_DISPLAYS);

	XForce_Desired->setShowEnabled(DEBUG_DISPLAYS);
	YForce_Desired->setShowEnabled(DEBUG_DISPLAYS);
	ZForce_Desired->setShowEnabled(DEBUG_DISPLAYS);

	XForce_Desired_Tool->setShowEnabled(DEBUG_DISPLAYS);
	YForce_Desired_Tool->setShowEnabled(DEBUG_DISPLAYS);
	ZForce_Desired_Tool->setShowEnabled(DEBUG_DISPLAYS);
	
	XForce_Measured->setShowEnabled(DEBUG_DISPLAYS);
	YForce_Measured->setShowEnabled(DEBUG_DISPLAYS);
	ZForce_Measured->setShowEnabled(DEBUG_DISPLAYS);

	cursorPosX->setShowEnabled(DEBUG_DISPLAYS);
	cursorPosY->setShowEnabled(DEBUG_DISPLAYS);
	cursorPosZ->setShowEnabled(DEBUG_DISPLAYS);
	message->setShowEnabled(DEBUG_DISPLAYS);

	outputPhantomForce_Desired_Tool_angle_deg->setShowEnabled(DEBUG_DISPLAYS);

	if (p_sharedData->opMode==DEMO)
		{	
			experimentState->setShowEnabled(true);
			trial->setShowEnabled(false);

		}

	/*
	input_phantomVelX->setShowEnabled(DEBUG_DISPLAYS);
	input_phantomVelY->setShowEnabled(DEBUG_DISPLAYS);
	input_phantomVelZ->setShowEnabled(DEBUG_DISPLAYS);
    output_phantomPosX->setShowEnabled(DEBUG_DISPLAYS);
	output_phantomPosY->setShowEnabled(DEBUG_DISPLAYS);
	output_phantomPosZ->setShowEnabled(DEBUG_DISPLAYS);
    output_phantomVelX->setShowEnabled(DEBUG_DISPLAYS);
	output_phantomVelY->setShowEnabled(DEBUG_DISPLAYS);
	output_phantomVelZ->setShowEnabled(DEBUG_DISPLAYS);
	cursorVelX->setShowEnabled(DEBUG_DISPLAYS);
	cursorVelY->setShowEnabled(DEBUG_DISPLAYS);
	cursorVelZ->setShowEnabled(DEBUG_DISPLAYS);
	*/

	if(p_sharedData->experimentStateNumber == EXPERIMENT) message->setShowEnabled(false);

    // render and (smoothly, via buffer swapping) display world
    camera->renderView(windowW, windowH);
    glutSwapBuffers();
    
    // check for OpenGL errors
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error: %s\n" << gluErrorString(err);
    
}

// dictates update frequency for the graphics
void graphicsTimer(int data) {
    
    if (p_sharedData->simulationRunning) {
        glutPostRedisplay();  // flag to update CHAI world
    }
    glutTimerFunc(Tgraphics, graphicsTimer, 0);
    
}

// dictates response to resizing the graphics window
void resizeWindow(int W, int H) {
    
    windowW = W;
    windowH = H;
    
}

// dictates response to a keypress within the graphics window
void respToKey(unsigned char key, int x, int y) {
	
	printf("\n\nkeypressed: %d\n\n", key);
	// Zero force
	if (key == 'x')
	{

		p_sharedData->outputPhantomForce_Desired_X = 0;
		p_sharedData->outputPhantomForce_Desired_Y = 0;
		p_sharedData->outputPhantomForce_Desired_Z = 0;

		// toggle flag
		if (p_sharedData->ZeroPhantomForce_FLAG) {
			// flip flag to zero force
			p_sharedData->ZeroPhantomForce_FLAG = false;
		}
		else {
			p_sharedData->ZeroPhantomForce_FLAG = true;
		}
	}


	// Force Sensor Testing t
	if (key == 't')
	{
		// log the desired z normal force
		p_sharedData->outputNormalForce_Set = p_sharedData->outputPhantomForce_Desired_Tool_Z;

		// start timer for next state of test force and increment 
		
		p_sharedData->timer->start(true);
		// set demoState number and name
		p_sharedData->experimentStateNumber = PRE_BLOCK;
		p_sharedData->experimentStateName = "Pre Block";



	}


	// BMI Tracking Non-linear sub-state machine
	if (key == 'k')
	{
		// update the velocity scalar 
		p_sharedData->velocity_force_scalar = p_sharedData->Fmax / BMI_Max_Vel_Nonlinear;
		p_sharedData->position_force_scalar = p_sharedData->Fmax / BMI_Max_Pos_Nonlinear;

		// log the desired z normal force
		p_sharedData->outputNormalForce_Set = p_sharedData->outputPhantomForce_Desired_Tool_Z;

		// start timer for next state of BMI tracking 

		p_sharedData->timer->start(true);
		// set demoState number and name
		p_sharedData->experimentStateNumber = PRE_BLOCK;
		p_sharedData->experimentStateName = "Pre Block";

		// set next state to go to after Pre_Block
		p_sharedData->nextExperimentStateNumber = TRACK_BMI_NONLINEAR;
	}

	// BMI Tracking Direction sub-state machine
	if (key == 'c')
	{
		// log the desired z normal force
		p_sharedData->outputNormalForce_Set = p_sharedData->outputPhantomForce_Desired_Tool_Z;

		// start timer for next state of BMI tracking 

		p_sharedData->timer->start(true);
		// set demoState number and name
		p_sharedData->experimentStateNumber = PRE_BLOCK;
		p_sharedData->experimentStateName = "Pre Block";

		// set next state to go to after Pre_Block
		p_sharedData->nextExperimentStateNumber = TRACK_BMI_DIRECTION;
	}

	// BMI Tracking sub-state machine
	if (key == 'b')
	{
		// update the velocity scalar 
		p_sharedData->velocity_force_scalar = p_sharedData->Fmax / BMI_Max_Vel;
		p_sharedData->position_force_scalar = p_sharedData->Fmax / BMI_Max_Pos;

		// log the desired z normal force
		p_sharedData->outputNormalForce_Set = p_sharedData->outputPhantomForce_Desired_Tool_Z;

		// start timer for next state of BMI tracking 

		p_sharedData->timer->start(true);
		// set demoState number and name
		p_sharedData->experimentStateNumber = PRE_BLOCK;
		p_sharedData->experimentStateName = "Pre Block";
	
		// set next state to go to after Pre_Block
		p_sharedData->nextExperimentStateNumber = TRACK_BMI;
	
	}




	// nano 17 zero
	if (key == 'n'){
	p_sharedData->g_ForceSensor.Zero_Force_Sensor();
	p_sharedData->message = "Nano17 biased. \nPlease Zero the tactor against the Participant's neck. Press Z when complete.";
}

	// zero position
	if (key == 'z') {
		p_sharedData->outputZeroPosX = p_sharedData->outputPhantomPosX;
		p_sharedData->outputZeroPosY = p_sharedData->outputPhantomPosY;
		p_sharedData->outputZeroPosZ = p_sharedData->outputPhantomPosZ;
		p_sharedData->output_ZeroRotation = p_sharedData->outputPhantomRotation;
		//p_sharedData->experimentStateNumber = IDLE;
		p_sharedData->message = "Adjust forces in tool frame: Z (7,9), Y (4,6), X (2,8) by 0.1N. \nPress (T) for Test Force \n(B) for BMI Velocity tracking \n(C) for BMI Velocity Direction tracking \n(K) for BMI Nonlinear VelocityTracking \n(D) for Direction Perception user study";



	}

	// Direction Perception Study (d keypress)
	if (key == 'd') {
		// log the desired z normal force
		p_sharedData->outputNormalForce_Set = p_sharedData->outputPhantomForce_Desired_Tool_Z;
		// start timer for next state of BMI tracking 

		p_sharedData->timer->start(true);
		// set demoState number and name
		p_sharedData->experimentStateNumber = PRE_BLOCK;
		p_sharedData->experimentStateName = "Pre Block";

		// set next state to go to after Pre_Block
		p_sharedData->nextExperimentStateNumber = PERCEPTION_EXPERIMENT_TRIAL;
	}





	// 9 to adjust normal force into neck (tool z direction)
	double force_increment = 0.1; // 0.1N
	if (key == 57) {
		p_sharedData->outputPhantomForce_Desired_Tool_Z = p_sharedData->outputPhantomForce_Desired_Tool_Z + force_increment;
		//transform desired tool force vector to Phantom Premium world frame
		cVector3d tool_force = cVector3d(p_sharedData->outputPhantomForce_Desired_Tool_X, p_sharedData->outputPhantomForce_Desired_Tool_Y, p_sharedData->outputPhantomForce_Desired_Tool_Z);

		cVector3d force_des = Rotate_Tool_to_Base_Frame(tool_force, p_sharedData->outputPhantomRotation);
		p_sharedData->outputPhantomForce_Desired_X = force_des.x();
		p_sharedData->outputPhantomForce_Desired_Y = force_des.y();
		p_sharedData->outputPhantomForce_Desired_Z = force_des.z();

	}

	// 7 to decrement normal force into neck (tool z direction)
	if (key == 55) {
		p_sharedData->outputPhantomForce_Desired_Tool_Z = p_sharedData->outputPhantomForce_Desired_Tool_Z - force_increment;
		//transform desired tool force vector to Phantom Premium world frame
		cVector3d tool_force = cVector3d(p_sharedData->outputPhantomForce_Desired_Tool_X, p_sharedData->outputPhantomForce_Desired_Tool_Y, p_sharedData->outputPhantomForce_Desired_Tool_Z);

		cVector3d force_des = Rotate_Tool_to_Base_Frame(tool_force, p_sharedData->outputPhantomRotation);
		p_sharedData->outputPhantomForce_Desired_X = force_des.x();
		p_sharedData->outputPhantomForce_Desired_Y = force_des.y();
		p_sharedData->outputPhantomForce_Desired_Z = force_des.z();

	}

	// 4 to decrement force in tool Y direction
	if (key == 52) {
		p_sharedData->outputPhantomForce_Desired_Tool_Y = p_sharedData->outputPhantomForce_Desired_Tool_Y - force_increment;
		//transform desired tool force vector to Phantom Premium world frame
		cVector3d tool_force = cVector3d(p_sharedData->outputPhantomForce_Desired_Tool_X, p_sharedData->outputPhantomForce_Desired_Tool_Y, p_sharedData->outputPhantomForce_Desired_Tool_Z);

		cVector3d force_des = Rotate_Tool_to_Base_Frame(tool_force, p_sharedData->outputPhantomRotation);
		p_sharedData->outputPhantomForce_Desired_X = force_des.x();
		p_sharedData->outputPhantomForce_Desired_Y = force_des.y();
		p_sharedData->outputPhantomForce_Desired_Z = force_des.z();

	}


	// 6 to increment force in tool Y direction
	if (key == 54) {
		p_sharedData->outputPhantomForce_Desired_Tool_Y = p_sharedData->outputPhantomForce_Desired_Tool_Y + force_increment;
		//transform desired tool force vector to Phantom Premium world frame
		cVector3d tool_force = cVector3d(p_sharedData->outputPhantomForce_Desired_Tool_X, p_sharedData->outputPhantomForce_Desired_Tool_Y, p_sharedData->outputPhantomForce_Desired_Tool_Z);

		cVector3d force_des = Rotate_Tool_to_Base_Frame(tool_force, p_sharedData->outputPhantomRotation);
		p_sharedData->outputPhantomForce_Desired_X = force_des.x();
		p_sharedData->outputPhantomForce_Desired_Y = force_des.y();
		p_sharedData->outputPhantomForce_Desired_Z = force_des.z();

	}

	// 8 to increment force in tool X direction
	if (key == 56) {
		p_sharedData->outputPhantomForce_Desired_Tool_X = p_sharedData->outputPhantomForce_Desired_Tool_X + force_increment;
		//transform desired tool force vector to Phantom Premium world frame
		cVector3d tool_force = cVector3d(p_sharedData->outputPhantomForce_Desired_Tool_X, p_sharedData->outputPhantomForce_Desired_Tool_Y, p_sharedData->outputPhantomForce_Desired_Tool_Z);

		cVector3d force_des = Rotate_Tool_to_Base_Frame(tool_force, p_sharedData->outputPhantomRotation);
		p_sharedData->outputPhantomForce_Desired_X = force_des.x();
		p_sharedData->outputPhantomForce_Desired_Y = force_des.y();
		p_sharedData->outputPhantomForce_Desired_Z = force_des.z();

	}


	// 6 to increment force in tool X direction
	if (key == 50) {
		p_sharedData->outputPhantomForce_Desired_Tool_X = p_sharedData->outputPhantomForce_Desired_Tool_X - force_increment;
		//transform desired tool force vector to Phantom Premium world frame
		cVector3d tool_force = cVector3d(p_sharedData->outputPhantomForce_Desired_Tool_X, p_sharedData->outputPhantomForce_Desired_Tool_Y, p_sharedData->outputPhantomForce_Desired_Tool_Z);

		cVector3d force_des = Rotate_Tool_to_Base_Frame(tool_force, p_sharedData->outputPhantomRotation);
		p_sharedData->outputPhantomForce_Desired_X = force_des.x();
		p_sharedData->outputPhantomForce_Desired_Y = force_des.y();
		p_sharedData->outputPhantomForce_Desired_Z = force_des.z();

	}




	// Enable data saving
	if (key == 'r')
	{
		/*
		// set recording flag
		if (p_sharedData->saveData_FLAG) {
			// flip flag to zero force
			p_sharedData->saveData_FLAG = false;
		}
		else {
			p_sharedData->saveData_FLAG = true;
		}

		p_sharedData->demoStateNumber = FORCE_SENSOR_TESTING_DEMO;
		p_sharedData->demoStateName = "Force Sensor Testing";
		*/


	}


	// option ESC: exit
    if ((key == 27) || (key == 'x'))
    {
        // close everything
        close();

        // exit application
        exit(0);
    }

	if (key == 'f')
    {
        if (fullscreen)
        {
            windowPosX = glutGet(GLUT_INIT_WINDOW_X);
            windowPosY = glutGet(GLUT_INIT_WINDOW_Y);
            windowW = glutGet(GLUT_INIT_WINDOW_WIDTH);
            windowH = glutGet(GLUT_INIT_WINDOW_HEIGHT);
            glutPositionWindow(windowPosX, windowPosY);
            glutReshapeWindow(windowW, windowH);
            fullscreen = false;
        }
        else
        {
            glutFullScreen();
            fullscreen = true;
        }
	}

	if (p_sharedData->opMode == DEMO) {



	}
}


// shut down the simulation in response to quitting/ESCaping from within graphics window
void close(void) {
    
    
    // terminate the experiment
    //if (p_sharedData->opMode == EXPERIMENT) closeExperiment();
    
    // stop the simulation
    p_sharedData->simulationRunning = false;
    
    // wait for NeuroTouch thread to terminate
    while (!p_sharedData->simulationFinished) cSleepMs(100);
    
    // close all devices
    if ((p_sharedData->input_device == INPUT_PHANTOM) || (p_sharedData->output_device == OUTPUT_PHANTOM))	closePhantom();
 
	// clean up memory
    

}

