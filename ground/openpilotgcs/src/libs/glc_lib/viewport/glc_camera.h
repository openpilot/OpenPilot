/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Copyright (C) 2009 Laurent Bauer
 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

//! \file glc_camera.h Interface for the GLC_Camera class.

#ifndef GLC_CAMERA_H_
#define GLC_CAMERA_H_

#include "../glc_object.h"
#include "../maths/glc_vector3d.h"
#include "../maths/glc_matrix4x4.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Camera
/*! \brief GLC_Camera : OpenGL perpective viewpoint */

/*! An GLC_Camera define Viewpoint and orientation
 * of an OpenGL perpective camera*/
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_Camera :
	public GLC_Object
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
   //! Default constructor
   /*! Point of view (0, 0, 1) Up Vector (0, 1, 0)*/
   GLC_Camera();

   //! Explicit constructor
	/* VectUp and VectCam could not be parallel
	 * VectUp could not be NULL
	 * VectCam could not be NULL */
   GLC_Camera(const GLC_Point3d &, const GLC_Point3d &, const GLC_Vector3d &);

   //! Copy constructor
   GLC_Camera(const GLC_Camera&);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Get the distance between the eye and the target of camera
	inline double distEyeTarget(void) const
	{return (m_Eye - m_Target).length();}

	//! Get camera's eye coordinate point
	inline GLC_Point3d eye(void) const
	{return m_Eye;}

	//! Get camera's target coordinate point
	inline GLC_Point3d target(void) const
	{return m_Target;}

	//! Get camera's Up vector
	inline GLC_Vector3d upVector(void) const
	{return m_VectUp;}

	//! Get camera's forward vector (from eye to target)
	inline GLC_Vector3d forward(void) const
	{return m_Target - m_Eye;}

	//! Side camera vector
	inline GLC_Vector3d sideVector() const
	{return ((m_Target - m_Eye).normalize() ^ m_VectUp).normalize();}

	//! Get camera's orbit composition matrix
	inline GLC_Matrix4x4 viewMatrix(void) const
	{return m_ModelViewMatrix;}

	//! equality operator
	bool operator==(const GLC_Camera&) const;

    //! almost equality (Bauer Laurent)
    bool isAlmostEqualTo(const GLC_Camera&, const double distanceAccuracy=0.05) const;

	//! Return the default up vector
	inline GLC_Vector3d defaultUpVector() const
	{return m_DefaultVectUp;}

	//! Return the standard front view form this camera
	GLC_Camera frontView() const;

	//! Return the standard rear view form this camera
	GLC_Camera rearView() const;

	//! Return the standard right view form this camera
	GLC_Camera rightView() const;

	//! Return the standard left view form this camera
	GLC_Camera leftView() const;

	//! Return the standard top view form this camera
	GLC_Camera topView() const;

	//! Return the standard bottom view form this camera
	GLC_Camera bottomView() const;

	//! Return the standard isoview from his camera
	/*! Iso View is at the front top left*/
	GLC_Camera isoView() const;

	//! Return the model view matrix of the camera
	inline GLC_Matrix4x4 modelViewMatrix() const
	{
		GLC_Matrix4x4 translate(-m_Eye);
		GLC_Matrix4x4 modelView= GLC_Matrix4x4(m_ModelViewMatrix * translate);
		return modelView;
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Camera orbiting
	GLC_Camera& orbit(GLC_Vector3d VectOldPoss, GLC_Vector3d VectCurPoss);

	//! panoramic movement
	GLC_Camera& pan(GLC_Vector3d VectDep);

	//! move camera's eye along camera vector (eye -> target)
	/*! Factor must be > 0*/
	GLC_Camera& zoom(double factor);

	//! Move camera
	GLC_Camera& move(const GLC_Matrix4x4 &MatMove);

	//! Rotate around an axis
	GLC_Camera& rotateAround(const GLC_Vector3d&, const double&, const GLC_Point3d&);

	//! Rotate around camera target
	GLC_Camera& rotateAroundTarget(const GLC_Vector3d&, const double&);

 	//! Camera translation
	GLC_Camera& translate(const GLC_Vector3d &VectTrans);

	//! Set the camera
	/* VectUp and VectCam could not be parallel
	 * VectUp could not be NULL
	 * VectCam could not be NULL */
	GLC_Camera& setCam(GLC_Point3d Eye, GLC_Point3d Target, GLC_Vector3d Up);

	//! Set the camera by copying another camera
	GLC_Camera& setCam(const GLC_Camera&);

   //! Set camera's eye coordinate vector
	GLC_Camera& setEyeCam(const GLC_Point3d &Eye);

	//! Set camera's target coordinate vector
	GLC_Camera& setTargetCam(const GLC_Point3d &Target);

	//! Set camera's Up vector
	GLC_Camera& setUpCam(const GLC_Vector3d &Up);

	//! Set the distance between eye and target (move eye)
	GLC_Camera& setDistEyeTarget(double Longueur);

	//! Set the distance between target and eye (move target)
	GLC_Camera& setDistTargetEye(double Longueur);

	//! Assignement operator
	GLC_Camera& operator=(const GLC_Camera&);

	//! Set the default Up vector
	/*! Must Be X, Y or Z Axis*/
	inline GLC_Camera& setDefaultUpVector(const GLC_Vector3d& up)
	{
		Q_ASSERT((up == glc::X_AXIS) || (up == glc::Y_AXIS) || (up == glc::Z_AXIS));
		m_DefaultVectUp= up;
		return *this;
	}

	//! Set the standard front view form this camera
	inline void setFrontView()
	{setCam(frontView());}

	//! Set the standard rear view form this camera
	inline void setRearView()
	{setCam(rearView());}

	//! Set the standard right view form this camera
	inline void setRightView()
	{setCam(rightView());}

	//! Set the standard left view form this camera
	inline void setLeftView()
	{setCam(leftView());}

	//! Set the standard top view form this camera
	inline void setTopView()
	{setCam(topView());}

	//! Set the standard bottom view form this camera
	inline void setBottomView()
	{setCam(bottomView());}

	//! Set the standard isoview from his camera
	/*! Iso View is at the front top left*/
	inline void setIsoView()
	{setCam(isoView());}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
   //! Execute OpenGL Camera
   void glExecute();

//@}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
private:
	//! compute composition matrix
 	void createMatComp(void);


//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////
private:
	//! Camera's eye point
	GLC_Point3d m_Eye;

	//! Camera's target point
	GLC_Point3d m_Target;

	//! Camera's Up vector
	GLC_Vector3d m_VectUp;

	//! Camera model view matrix
	GLC_Matrix4x4 m_ModelViewMatrix;

	//! The default Up axis
	GLC_Vector3d m_DefaultVectUp;
};
#endif //GLC_CAMERA_H_
