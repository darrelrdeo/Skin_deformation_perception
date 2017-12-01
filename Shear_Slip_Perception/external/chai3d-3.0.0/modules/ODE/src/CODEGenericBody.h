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
#ifndef CODEGenericBodyH
#define CODEGenericBodyH
//---------------------------------------------------------------------------
#ifndef dDOUBLE
#define dDOUBLE
#endif
#include "ode/ode.h"
#include "chai3d.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
    \file       CODEGenericBody.h

    \brief 
    <b> ODE Module </b> \n 
    ODE Generic Object.
*/
//===========================================================================

//---------------------------------------------------------------------------
class cODEWorld;
//---------------------------------------------------------------------------
//! ODE geometry used for dynamic collision computation.
enum cODEDynamicModelType
{
    ODE_MODEL_BOX,
    ODE_MODEL_SPHERE,
    ODE_MODEL_CYLINDER,
    ODE_MODEL_PLANE,
    ODE_MODEL_TRIMESH
};
//---------------------------------------------------------------------------

//===========================================================================
/*!
    \class      cODEGenericBody
    \ingroup    ODE

    \brief
    ODE dynamic object

    \details
    cODEGenericBody is a base class for modeling any ODE dynamic body.
*/
//===========================================================================
class cODEGenericBody : public chai3d::cGenericObject, public dBody
{
    //-----------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //-----------------------------------------------------------------------

public:

    //! Constructor of cODEGenericBody.
	cODEGenericBody(cODEWorld* a_world) { initialize(a_world); }

    //! Destructor of cODEGenericBody.
    virtual ~cODEGenericBody() {};


    //-----------------------------------------------------------------------
    // PUBLIC METHODS - TRANSLATION AND ORIENTATION:
    //-----------------------------------------------------------------------

public:

    //! Set the position of object.
    virtual void setLocalPos(const chai3d::cVector3d &a_position);

#ifdef C_USE_EIGEN
    //! Set the local position of this object.
    void setLocalPos(const Eigen::Vector3d& a_localPos)
    {
        setLocalPos(chai3d::cVector3d(a_localPos[0], a_localPos[1], a_localPos[2]));
    }
#endif

    //! Set the local position of this object.
    void setLocalPos(const double a_x = 0.0, 
        const double a_y = 0.0, 
        const double a_z = 0.0)
    {
        setLocalPos(chai3d::cVector3d(a_x, a_y, a_z));
    }

#ifdef C_USE_EIGEN
    //! Set the local rotation matrix for this object.
    inline void setLocalRot(const Eigen::Matrix3d a_localRot)
    {
        chai3d::cMatrix3d localRot;
        localRot.copyfrom(a_localRot);
        setLocalRot(localRot);
    }
#endif

    //! Set the orientation of object.
    virtual void setLocalRot(const chai3d::cMatrix3d &a_rotation);

    //! Update position and orientation from ode model to chai model.
    void updateBodyPosition(void);


    //-----------------------------------------------------------------------
    // PUBLIC METHODS - EXTERNAL FORCES:
    //-----------------------------------------------------------------------

public:

	//! Apply an external force.
    void addExternalForce(const chai3d::cVector3d& a_force);

	//! Apply an external torque.
    void addExternalTorque(const chai3d::cVector3d& a_torque);

    //! Apply an external force at a given position decribed in global coordinates.
    void addExternalForceAtPoint(const chai3d::cVector3d& a_force,
                                 const chai3d::cVector3d& a_pos);

    //-----------------------------------------------------------------------
    // PUBLIC METHODS - DYNAMIC PROPERTIES:
    //-----------------------------------------------------------------------

public:

    //! Set object mass.
    void setMass(const double a_mass);

    //! Read object mass.
    double getMass() const { return (m_mass); }

    //! Enable object from dynamic update.
    void enableDynamics();

    //! Disable object from dynamic update.
    void disableDynamics();

    //! Is the current object static? (cannot move).
    bool isStatic() const { return (m_static); } 

    //! Create a dynamic model of the object.
    virtual void buildDynamicModel() {};

    //! Create a dynamic box representation.
    void createDynamicBoundingBox(const bool a_staticObject = false);

    //! Create a dynamic sphere representation.
    void createDynamicSphere(const double a_radius, 
        const bool a_staticObject = false,
        const chai3d::cVector3d& a_offsetPos = chai3d::cVector3d(0.0, 0.0, 0.0),
        const chai3d::cMatrix3d& a_offsetRot = chai3d::cIdentity3d());

    //! Create a dynamic box representation.
    void createDynamicBox(const double a_lengthX, 
        const double a_lengthY, 
        const double a_lengthZ,
        const  bool a_staticObject = false,
        const chai3d::cVector3d& a_offsetPos = chai3d::cVector3d(0.0, 0.0, 0.0),
        const chai3d::cMatrix3d& a_offsetRot = chai3d::cIdentity3d());

    //! Create a dynamic capsule representation.
    void createDynamicCapsule(const double a_radius, 
        const double a_length,
        const bool a_staticObject = false,
        const chai3d::cVector3d& a_offsetPos = chai3d::cVector3d(0.0, 0.0, 0.0),
        const chai3d::cMatrix3d& a_offsetRot = chai3d::cIdentity3d());

    //! Create a dynamic caped cylinder representation.
    void createDynamicCylinder(const double a_radius,
        const double a_length,
        const bool a_staticObject = false,
        const chai3d::cVector3d& a_offsetPos = chai3d::cVector3d(0.0, 0.0, 0.0),
        const chai3d::cMatrix3d& a_offsetRot = chai3d::cIdentity3d());

    //! Create a static plane representation.
    void createStaticPlane(const chai3d::cVector3d& a_position,
        const chai3d::cVector3d& a_normal);

    //! Create a triangle mesh representation.
    void createDynamicMesh(const bool a_staticObject = false,
        const chai3d::cVector3d& a_offsetPos = chai3d::cVector3d(0.0, 0.0, 0.0),
        const chai3d::cMatrix3d& a_offsetRot = chai3d::cIdentity3d());
    

    //-----------------------------------------------------------------------
    // PUBLIC METHODS - BODY IMAGE AND DISPLAY PROPERTIES:
    //-----------------------------------------------------------------------

public:

    //! Set a CHAI3D body image for the object.
    void setImageModel(chai3d::cGenericObject* a_imageModel);

    //! Get CHAI3D body image.
    chai3d::cGenericObject* getImageModel() { return (m_imageModel); }

    //! Show collision model.
    void setShowDynamicCollisionModel(const bool a_show) { m_showDynamicCollisionModel = a_show; }

    //! Read show collision model status.
    bool getShowDynamicCollisionModel() { return (m_showDynamicCollisionModel); }

    //! Color use to render collision model.
    chai3d::cColorf m_colorDynamicCollisionModel;

    //-----------------------------------------------------------------------
    // PUBLIC MEMBERS:
    //-----------------------------------------------------------------------

public:

    //! Parent world.
    cODEWorld* m_ODEWorld;

    //! Object used to represent the geometry and graphical properties of the object.
    chai3d::cGenericObject* m_imageModel;

    //! ODE body.
    dBodyID m_ode_body;

    //! ODE body geometry.
    dGeomID m_ode_geom;


    //-----------------------------------------------------------------------
    // PROTECTED METHODS:
    //-----------------------------------------------------------------------

protected:

    //! Update global position frames.
    virtual void updateGlobalPositions(const bool a_frameOnly);

    //! Render object in OpenGL.
    virtual void render(chai3d::cRenderOptions& a_options);

    //! Compute collision with object geometry.
    virtual bool computeOtherCollisionDetection(chai3d::cVector3d& a_segmentPointA,
        chai3d::cVector3d& a_segmentPointB,
        chai3d::cCollisionRecorder& a_recorder,
        chai3d::cCollisionSettings& a_settings);
   
    //! Build table of triangles and vertices for ODE mesh representation.
    int buildMeshTable(chai3d::cMultiMesh* a_mesh,
        int& a_indexOffset, 
        int& a_verticesCount, 
        int& a_indexCount);

    //! Initialize ODE body.
    void initialize(cODEWorld* a_world);


	//-----------------------------------------------------------------------
    // PROTECTED MEMBERS:
    //-----------------------------------------------------------------------

protected:

    //! if \b true then ODE object is static and does not move. 
    bool m_static;

    //! ODE body mass matrix.
    dMass m_ode_mass;

    //! Mass of object units: [kg].
    double m_mass;

    //! ODE vertices (for triangle mesh models) - Do not use doubles since not supported under ODE! 
    float* m_vertices;

    //! ODE indices (for triangle mesh models).
    int* m_vertexIndices;

    //! ODE previous tri mesh position and orientation.
    double m_prevTransform[16];

    //! ODE tri mesh ID.
    dTriMeshDataID m_ode_triMeshDataID;

    //! Enable/Disable graphical representation of collision model.
    bool m_showDynamicCollisionModel;

    //! Dynamic model type.
    cODEDynamicModelType m_typeDynamicCollisionModel;

    /*! 
		Variables which store the parameters of the collision model.
        Depending of the dynamic model type, these value correspond to
        lengths or radius.
	*/
    double m_paramDynColModel0;
    double m_paramDynColModel1;
    double m_paramDynColModel2;
    chai3d::cVector3d m_posOffsetDynColModel;
    chai3d::cMatrix3d m_rotOffsetDynColModel;
};


//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
