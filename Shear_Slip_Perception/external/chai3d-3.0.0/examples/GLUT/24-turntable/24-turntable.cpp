//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2014, CHAI3D.
    (www.chai3d.org)

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

    * Neither the name of CHAI3D nor the names of its contributors may
    be used to endorse or promote products derived from this software
    without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE. 

    \author    <http://www.chai3d.org>
    \author    Federico Barbagli
    \author    Chris Sewell
    \author    Francois Conti
    \version   3.0.0 $Rev: 1292 $
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "bass.h"
//------------------------------------------------------------------------------
#include "chai3d.h"
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------
#ifndef MACOSX
#include "GL/glut.h"
#else
#include "GLUT/glut.h"
#endif
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// GENERAL SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
    C_STEREO_DISABLED:            Stereo is disabled 
    C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
    C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
    C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/
cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;


//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in a window display
cCamera* camera;

// a light source to illuminate the objects in the virtual scene
cSpotLight *light;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// a virtual tool representing the haptic device in the scene
cToolCursor* tool;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelHapticRate;

// a scope to monitor the sound signal
cScope* scope;

// a virtual turntable object
cMultiMesh* turntable;

// a virtual vinyl
cMesh* record;


// BASS library
const int MAX_VAL_SLIDERS_P = 100000;
const int MAX_VAL_SLIDERS_I = 10000000;
const int FREQ = 1000;
const double chartClock = 50;
const double MESH_SCALE_SIZE = 0.35;

// Global variables for the audio stream
HSTREAM stream;
BASS_CHANNELINFO infoBass;
QWORD stream_length;
char *data;
int record_direction = 1;
unsigned int pos = 0;

// angular velocity of the record
double angVel = 0;

// angular position of the record
double angPos = 0;

// indicates if the haptic simulation currently running
bool simulationRunning = false;

// indicates if the haptic simulation has terminated
bool simulationFinished = true;

// frequency counter to measure the simulation haptic rate
cFrequencyCounter frequencyCounter;

// information about computer screen and GLUT display window
int screenW;
int screenH;
int windowW;
int windowH;
int windowPosX;
int windowPosY;


//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------
// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void resizeWindow(int w, int h);

// callback when a key is pressed
void keySelect(unsigned char key, int x, int y);

// callback to render graphic scene
void updateGraphics(void);

// callback of GLUT timer
void graphicsTimer(int data);

// function that closes the application
void close(void);

// main haptics simulation loop
void updateHaptics(void);

// write the requested data from the loaded buffer to the sound card
DWORD CALLBACK MyStreamWriter(HSTREAM handle, void *buf, DWORD len, void *user);


//==============================================================================
/*
    DEMO:    turntable.cpp

    This example demonstrates the use of friction, animation, and
    sound effects.  With the haptic device you can spin the record
    back and forth and at different speeds.
*/
//==============================================================================

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "Demo: 24-turntable" << endl;
    cout << "Copyright 2003-2014" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[f] - Enable/Disable full screen mode" << endl;
    cout << "[m] - Enable/Disable vertical mirroring" << endl;
    cout << "[x] - Exit application" << endl;
    cout << endl << endl;

    // parse first arg to try and locate resources
    string resourceRoot = string(argv[0]).substr(0,string(argv[0]).find_last_of("/\\")+1);


    //--------------------------------------------------------------------------
    // OPEN GL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLUT
    glutInit(&argc, argv);

    // retrieve  resolution of computer display and position window accordingly
    screenW = glutGet(GLUT_SCREEN_WIDTH);
    screenH = glutGet(GLUT_SCREEN_HEIGHT);
    windowW = 0.8 * screenH;
    windowH = 0.5 * screenH;
    windowPosY = (screenH - windowH) / 2;
    windowPosX = windowPosY; 

    // initialize the OpenGL GLUT window
    glutInitWindowPosition(windowPosX, windowPosY);
    glutInitWindowSize(windowW, windowH);
    if (stereoMode == C_STEREO_ACTIVE)
    {
        glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STEREO);
    }
    else
    {
        glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    }

    // create display context and initialize GLEW library
    glutCreateWindow(argv[0]);
    glewInit();

    // setup GLUT options
    glutDisplayFunc(updateGraphics);
    glutKeyboardFunc(keySelect);
    glutReshapeFunc(resizeWindow);
    glutSetWindowTitle("CHAI3D");

    // set fullscreen mode
    if (fullscreen)
    {
        glutFullScreen();
    }


    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    world->m_backgroundColor.setBlack();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set( cVector3d (2.2, 0.14, 1.4),    // camera position (eye)
                 cVector3d (0.2, 0.14,-0.1),    // lookat position (target)
                 cVector3d (0.0, 0.00, 1.0));   // direction of the (up) vector

    // set the near and far clipping planes of the camera
    // anything in front/behind these clipping planes will not be rendered
    camera->setClippingPlanes(0.01, 10.0);

    // set stereo mode
    camera->setStereoMode(stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.03);
    camera->setStereoFocalLength(3.0);

    // set vertical mirrored display mode
    camera->setMirrorVertical(mirroredDisplay);

    // Enable shadow casting
    camera->setUseShadowCasting(true);

    // create a light source
    light = new cSpotLight(world);

    // attach light to camera
    world->addChild(light);

    // enable light source
    light->setEnabled(true);

    // position the light source
    light->setLocalPos( 1.5, 0.40, 1.5);

    // define the direction of the light beam
    light->setDir(-2.0,-0.5,-2.0);

    // set lighting conditions
    light->m_ambient.set(0.4, 0.4, 0.4);
    light->m_diffuse.set(0.8, 0.8, 0.8);
    light->m_specular.set(1.0, 1.0, 1.0);

    // enable this light source to generate shadows
    light->setShadowMapEnabled(true);

    // set the resolution of the shadow map
    light->m_shadowMap->setResolutionLow();
    //light->m_shadowMap->setResolutionMedium();

    // set light cone half angle
    light->setCutOffAngleDeg(30);


    //--------------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get access to the first available haptic device found
    handler->getDevice(hapticDevice, 0);

    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = hapticDevice->getSpecifications();

    // create a tool (cursor) and insert into the world
    tool = new cToolCursor(world);
    world->addChild(tool);

    // connect the haptic device to the virtual tool
    tool->setHapticDevice(hapticDevice);

    // define the radius of the tool (sphere)
    double toolRadius = 0.01;

    // define a radius for the tool
    tool->setRadius(toolRadius);

    // hide the device sphere. only show proxy.
    tool->setShowContactPoints(true, false);

    // create a white cursor
    tool->m_hapticPoint->m_sphereProxy->m_material->setWhite();

    // enable if objects in the scene are going to rotate of translate
    // or possibly collide against the tool. If the environment
    // is entirely static, you can set this parameter to "false"
    tool->enableDynamicObjects(true);

    // map the physical workspace of the haptic device to a larger virtual workspace.
    tool->setWorkspaceRadius(1.0);

    // start the haptic tool
    tool->start();


    //--------------------------------------------------------------------------
    // CREATE OBJECTS
    //--------------------------------------------------------------------------

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    double workspaceScaleFactor = tool->getWorkspaceScaleFactor();

    // stiffness properties
    double maxStiffness	= hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;


    /////////////////////////////////////////////////////////////////////////
    // TURNTABLE OBJECT
    /////////////////////////////////////////////////////////////////////////

    // create a virtual mesh
    turntable = new cMultiMesh();

    // add object to world
    world->addChild(turntable);

    // set the position of turntable object at the center of the world
    turntable->setLocalPos(0.0, 0.0, 0.0);
    turntable->rotateAboutGlobalAxisDeg(cVector3d(0,0,1), 90);

    // load an object file
    bool fileload;
    fileload = turntable->loadFromFile(RESOURCE_PATH("resources/models/turntable/turntable.obj"));
    if (!fileload)
    {
        #if defined(_MSVC)
        fileload = turntable->loadFromFile("../../../bin/resources/models/turntable/turntable.obj");
        #endif
    }
    if (!fileload)
    {
        printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }

    // compute a boundary box
    turntable->computeBoundaryBox(true);

    // get dimensions of object
    double size = cSub(turntable->getBoundaryMax(), turntable->getBoundaryMin()).length();

    // resize object to screen
    if (size > 0)
    {
        turntable->scale( 2.0 * tool->getWorkspaceRadius() / size);
    }

    // setup collision detection algorithm
    turntable->createAABBCollisionDetector(toolRadius);

    // define a default stiffness for the object
    turntable->setStiffness(0.8 * maxStiffness, true);


    /////////////////////////////////////////////////////////////////////////
    // VINYL OBJECT
    /////////////////////////////////////////////////////////////////////////

    // create new record disc
    record = new cMesh();
    cCreateCylinder(record, 0.04, 0.48, 36, 1, false, true);
    cCreateDisk(record, 0.49, 0.49, 36, cVector3d(0,0,0.04));

    cTexture2dPtr recordImage = cTexture2d::create();

    fileload = recordImage->loadFromFile(RESOURCE_PATH("resources/models/turntable/record.jpg"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = recordImage->loadFromFile("../../../bin/resources/models/turntable/record.jpg");
#endif
    }
    if (!fileload)
    {
        cout << "Error - Texture image failed to load correctly." << endl;
        close();
        return (-1);
    }

    record->m_material->setWhite();
    record->setTexture(recordImage);
    record->setUseTexture(true, true);
    record->setUseMaterial(true, true);
    record->createAABBCollisionDetector(toolRadius);

    // add object to world and translate
    world->addChild(record);
    record->translate(-0.0, 0, 0.02);

    // set stiffness properties of record
    record->setStiffness(maxStiffness, true);

    // set static and dynamic friction
    double staticFriction = (double)100 / 100.0;
    double dynamicFriction = (double)100 / 100.0;
    record->setFriction(staticFriction, dynamicFriction, true);


    //--------------------------------------------------------------------------
    // SETUP AUDIO
    //--------------------------------------------------------------------------

    // Initialize sound device and create audio stream
    BASS_Init(1,44100,0,0,NULL);

    // Load the data from the specified file
    HSTREAM file_stream = 1;
    file_stream = BASS_StreamCreateFile(FALSE,RESOURCE_PATH("resources/sounds/classic.mp3"),0,0,BASS_STREAM_DECODE);
    if (file_stream == 0)
    {
        #if defined(_MSVC)
        file_stream = BASS_StreamCreateFile(FALSE,"../../../bin/resources/sounds/classic.mp3",0,0,BASS_STREAM_DECODE);
        #endif
    }
    if (!fileload)
    {
        printf("Error - MP3 audio file failed to load correctly.\n");
        close();
        return (-1);
    }

    // Get the length and header info from the loaded file
    //stream_length = BASS_StreamGetLength(file_stream);
    stream_length = BASS_ChannelGetLength(file_stream, 0);
    BASS_ChannelGetInfo(file_stream, &infoBass);

    // Get the audio samples from the loaded file
    data = new char[(unsigned int)stream_length];
    BASS_ChannelGetData(file_stream, data, (unsigned int)stream_length);

    // Set playing to begin at the beginning of the loaded data
    stream = BASS_StreamCreate(infoBass.freq, infoBass.chans, 0, &MyStreamWriter, 0);


    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    cFont *font = NEW_CFONTCALIBRI20();
    
    // create a label to display the haptic rate of the simulation
    labelHapticRate = new cLabel(font);
    labelHapticRate->m_fontColor.setBlack();
    camera->m_frontLayer->addChild(labelHapticRate);

    // create a background
    cBackground* background = new cBackground();
    camera->m_backLayer->addChild(background);

    // set background properties
    background->setCornerColors(cColorf(1.00, 1.00, 1.00),
                                     cColorf(1.00, 1.00, 1.00),
                                     cColorf(0.90, 0.90, 0.90),
                                     cColorf(0.90, 0.90, 0.90));

    // create a scope to plot haptic device position data
    scope = new cScope();
    camera->m_frontLayer->addChild(scope);
    scope->setSize(512, 100);
    scope->setLocalPos(100,80);
    scope->setRange(0.0, 0.2);
    scope->setSignalEnabled(true, false, false, false);
    scope->m_colorSignal0.setRed();

    cColorf color;
    color.setBlueCornflower();
    scope->setColor(color);
    scope->setTransparencyLevel(0.2);


    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    cThread* hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // start the main graphics rendering loop
    glutTimerFunc(50, graphicsTimer, 0);
    glutMainLoop();

    // close everything
    close();

    // exit
    return (0);
}

//------------------------------------------------------------------------------

void resizeWindow(int w, int h)
{
    windowW = w;
    windowH = h;
}

//------------------------------------------------------------------------------

void keySelect(unsigned char key, int x, int y)
{
    // escape key
    if ((key == 27) || (key == 'x'))
    {
        // close everything
        close();

        // exit application
        exit(0);
    }

    // option f: toggle fullscreen
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

    // option m: toggle vertical mirroring
    if (key == 'm')
    {
        mirroredDisplay = !mirroredDisplay;
        camera->setMirrorVertical(mirroredDisplay);
    }
}

//------------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close haptic device
    tool->stop();
}

//------------------------------------------------------------------------------

void graphicsTimer(int data)
{
    if (simulationRunning)
    {
        glutPostRedisplay();
    }

    glutTimerFunc(50, graphicsTimer, 0);
}

//------------------------------------------------------------------------------

void updateGraphics(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // update haptic rate label
    labelHapticRate->setString ("haptic rate: "+cStr(frequencyCounter.getFrequency(), 0) + " [Hz]");

    // update position of label
    labelHapticRate->setLocalPos((int)(0.5 * (windowW - labelHapticRate->getWidth())), 15);

    // update scope data
    scope->clearSignals();
    float fft[512]; // fft data buffer
    int result = BASS_ChannelGetData(stream, fft, BASS_DATA_FFT1024);
    if (result != -1)
    {
        for (int a=0; a<512; a++)
        {
            double value = cAbs(angVel) * fft[a];
            scope->setSignalValues(value);
        }
    }

    // update position of scope
    scope->setLocalPos((0.5 * (windowW - scope->getWidth())), 80);


    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // render world
    camera->renderView(windowW, windowH);

    // swap buffers
    glutSwapBuffers();

    // check for any OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error: " << gluErrorString(err) << endl;
}

//------------------------------------------------------------------------------

void updateHaptics(void)
{
    // reset clock
    cPrecisionClock clock;
    clock.reset();

    // simulation in now running
    simulationRunning  = true;
    simulationFinished = false;

    // main haptic simulation loop
    while(simulationRunning)
    {
        /////////////////////////////////////////////////////////////////////
        // SIMULATION TIME    
        /////////////////////////////////////////////////////////////////////

        // stop the simulation clock
        clock.stop();

        // read the time increment in seconds
        double timeInterval = clock.getCurrentTimeSeconds();

        // restart the simulation clock
        clock.reset();
        clock.start();

        // update frequency counter
        frequencyCounter.signal(1);


        /////////////////////////////////////////////////////////////////////
        // HAPTIC FORCE COMPUTATION
        /////////////////////////////////////////////////////////////////////

        // compute global reference frames for each object
        world->computeGlobalPositions(true);

        // update position and orientation of tool
        tool->updatePose();

        // compute interaction forces
        tool->computeInteractionForces();

        // send forces to haptic device
        tool->applyForces();


        /////////////////////////////////////////////////////////////////////
        // ANIMATION
        /////////////////////////////////////////////////////////////////////

        // init
        cVector3d torque(0,0,0);

        // figure out if we're touching the record
        if (tool->isInContact(record))
        {
            // get position of cursor in global coordinates
            cVector3d toolPos = tool->getDeviceGlobalPos();

            // get position of object in global coordinates
            cVector3d objectPos = record->getGlobalPos();

            // compute a vector from the center of mass of the object (point of rotation) to the tool
            cVector3d vObjectCMToTool = cSub(toolPos, objectPos);

            if (vObjectCMToTool.length() > 0)
            {
                // get the last force applied to the cursor in global coordinates
                // we negate the result to obtain the opposite force that is applied on the
                // object
                cVector3d toolForce = cNegate(tool->m_lastComputedGlobalForce);

                // compute effective force to take into account the fact the object
                // can only rotate arround a its center mass and not translate
                cVector3d effectiveForce = toolForce - cProject(toolForce, vObjectCMToTool);

                // compute the resulting torque
                torque = cMul(vObjectCMToTool.length(), cCross( cNormalize(vObjectCMToTool), effectiveForce));
            }
        }

        // update rotational acceleration
        const double OBJECT_INERTIA = 0.02;
        double angAcc = (1.0 / OBJECT_INERTIA) * torque.z();

        // update rotational velocity
        angVel = angVel + timeInterval * angAcc;

        // set a threshold on the rotational velocity term
        const double ANG_VEL_MAX = 20.0;

        if (angVel > ANG_VEL_MAX)
        {
            angVel = ANG_VEL_MAX;
        }
        else if (angVel < -ANG_VEL_MAX)
        {
            angVel = -ANG_VEL_MAX;
        }

        // compute the next rotation of the torus
        angPos = angPos + timeInterval * angVel;

        // update position of record
        cMatrix3d rot;
        rot.identity();
        rot.rotateAboutGlobalAxisRad(cVector3d(0,0,1), angPos);
        record->setLocalRot(rot);

        // update audio
        // set audio direction and frequency based on rotational velocity
        if ((fabs(angVel)) > 0.0)
        {
            if (angVel < 0.0) record_direction = 1;
            else record_direction = -1;

            BASS_ChannelSetAttribute(stream, BASS_ATTRIB_FREQ, (int)(infoBass.freq*(0.05+fabs(angVel))/6.5));

            if (!(BASS_ChannelPlay(stream,FALSE)))
            {
            }
        }
        else
        {
            BASS_ChannelStop(stream);
        }
    }

    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------

DWORD CALLBACK MyStreamWriter(HSTREAM handle, void *buf, DWORD len, void *user)
{
    // Cast the buffer to a character array
    char *d=(char*)buf;

    // Loop the file when it reaches the beginning or end
    if ((pos >= stream_length) && (record_direction == 1))
            pos = 0;
    if ((pos <= 0) && (record_direction == -1))
            pos = (unsigned int)stream_length;

    // If record is spinning in positive direction, write requested
    // amount of data from current position forwards
    if (record_direction == 1)
    {
        int up = len + pos;
        if (up > stream_length)
            up = (unsigned int)stream_length;

        for (int i=pos; i<up; i+=1)
        {
            d[(i-pos)] = data[i];
        }

        int amt = (up-pos);
        pos += amt;
        return amt;
     }

    // If record is spinning in negative direction, write requested
    // amount of data from current position backwards
    if (record_direction == -1)
    {
        int up = pos - len;

        if (up < 0)
            up = 0;

        int cnt = 0;
        for (int i=pos; i>up; i-=1)
                d[cnt++] = data[i];

        int amt = cnt;
        pos -= amt;

        return amt;
     }

     return 0;
}

//------------------------------------------------------------------------------
