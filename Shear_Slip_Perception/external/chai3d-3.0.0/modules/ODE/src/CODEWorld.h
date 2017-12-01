//===========================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2014, CHAI3D
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
    \author    Francois Conti
    \version   3.0.0 $Rev: 1242 $
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CODEWorldH
#define CODEWorldH
//---------------------------------------------------------------------------
#include "chai3d.h"
#include "CODEGenericBody.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
    \file       CODEWorld.h

    \brief 
    <b> ODE Module </b> \n 
    ODE World.
*/
//===========================================================================


//===========================================================================
/*!
    \class      cODEWorld
    \ingroup    ODE

    \brief
    ODE dynamic world

    \details
    cODEWorld implements a virtual world to handle ODE based objects 
    (cODEGenericBody).
*/
//===========================================================================
class cODEWorld : public chai3d::cGenericObject
{
    //-----------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //-----------------------------------------------------------------------

public:

    //! Constructor of cODEWorld.
    cODEWorld(chai3d::cWorld* a_parentWorld);

    //! Destructor of cODEWorld.
    virtual ~cODEWorld();


    //-----------------------------------------------------------------------
    // PUBLIC MEMBERS:
    //-----------------------------------------------------------------------

public:

    //! List of ODE dynamic bodies.
    std::list<cODEGenericBody*> m_bodies;


	//-----------------------------------------------------------------------
    // PUBLIC METHODS:
    //-----------------------------------------------------------------------

public:

    //! Set gravity field.
    void setGravity(chai3d::cVector3d a_gravity);

    //! Read gravity field.
    chai3d::cVector3d getGravity();

    //! Set linear damping.
    void setLinearDamping(double a_value) { dWorldSetLinearDamping(m_ode_world, a_value); }

    //! Set angular damping.
    void setAngularDamping(double a_value) { dWorldSetAngularDamping(m_ode_world, a_value); } 

    //! Set max angular speed.
    void setMaxAngularSpeed(double a_value) { dWorldSetMaxAngularSpeed(m_ode_world, a_value); }

    //! compute dynamic simulation.
    void updateDynamics(double a_interval);

    //! update position and orientation from ode models to chai models.
    void updateBodyPositions(void);

    // update global position frames.
    void updateGlobalPositions(const bool a_frameOnly);


	//-----------------------------------------------------------------------
    // PUBLIC MEMBERS:
    //-----------------------------------------------------------------------

public:

    //! ODE dynamics world.
    dWorldID m_ode_world;

    //! ODE collision space.
    dSpaceID m_ode_space;

	//! ODE contact group.
	dJointGroupID m_ode_contactgroup;


	//-----------------------------------------------------------------------
    // PROTECTED MEMBERS:
    //-----------------------------------------------------------------------

protected:

    //! current time of simulation.
    double m_simulationTime;

    //! Parent chai3d world.
    chai3d::cWorld* m_parentWorld;


	//-----------------------------------------------------------------------
    // PROTECTED METHODS:
    //-----------------------------------------------------------------------
    
protected:

	//! ODE collision callback.
	static void nearCallback (void *data, dGeomID o1, dGeomID o2);

    //! Render deformable mesh (OpenGL).
    virtual void render(chai3d::cRenderOptions& a_options);

    //! Return the nearest triangle intersected by the given segment, if any.
    virtual bool computeOtherCollisionDetection(chai3d::cVector3d& a_segmentPointA,
        chai3d::cVector3d& a_segmentPointB,
        chai3d::cCollisionRecorder& a_recorder,
        chai3d::cCollisionSettings& a_settings);
};


//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
