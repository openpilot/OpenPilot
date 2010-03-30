/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Copyright (C) 2009 Laurent Bauer
 Version 1.2.0, packaged on September 2009.

 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

//! \file glc_camera.h Interface for the GLC_Camera class.

#ifndef GLC_CAMERA_H_
#define GLC_CAMERA_H_

#include "../glc_object.h"
#include "../maths/glc_vector4d.h"
#include "../maths/glc_matrix4x4.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Camera
/*! \brief GLC_Camera : OpenGL perpective viewpoint */

/*! An GLC_Camera define Viewpoint and orientation
 * of an OpenGL perpective camera*/
//////////////////////////////////////////////////////////////////////

class GLC_Camera :
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
   GLC_Camera(const GLC_Point4d &, const GLC_Point4d &, const GLC_Vector4d &);

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
	{return (m_Eye - m_Target).norm();}

	//! Get camera's eye coordinate point
	inline GLC_Point4d eye(void) const
	{return m_Eye;}

	//! Get camera's target coordinate point
	inline GLC_Point4d target(void) const
	{return m_Target;}

	//! Get camera's Up vector
	inline GLC_Vector4d upVector(void) const
	{return m_VectUp;}

	//! Get camera's Vector (from eye to target)
	inline GLC_Vector4d camVector(void) const
	{return m_Eye - m_Target;}

	//! Get camera's orbit composition matrix
	inline GLC_Matrix4x4 viewMatrix(void) const
	{return m_MatCompOrbit;}

	//! equality operator
	bool operator==(const GLC_Camera&) const;

    //! almost equality (Bauer Laurent)
    bool isAlmostEqualTo(const GLC_Camera&, const double distanceAccuracy=0.05) const;

	//! Return the default up vector
	inline GLC_Vector4d defaultUpVector() const
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

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Camera orbiting
	GLC_Camera& orbit(GLC_Vector4d VectOldPoss, GLC_Vector4d VectCurPoss);

	//! panoramic movement
	GLC_Camera& pan(GLC_Vector4d VectDep);

	//! move camera's eye along camera vector (eye -> target)
	/*! Factor must be > 0*/
	GLC_Camera& zoom(double factor);

	//! Move camera
	GLC_Camera& move(const GLC_Matrix4x4 &MatMove);

	//! Rotate around an axis
	GLC_Camera& rotateAround(const GLC_Vector4d&, const double&, const GLC_Point4d&);

	//! Rotate around camera target
	GLC_Camera& rotateAroundTarget(const GLC_Vector4d&, const double&);

 	//! Camera translation
	GLC_Camera& translate(const GLC_Vector4d &VectTrans);

	//! Set the camera
	/* VectUp and VectCam could not be parallel
	 * VectUp could not be NULL
	 * VectCam could not be NULL */
	GLC_Camera& setCam(GLC_Point4d Eye, GLC_Point4d Target, GLC_Vector4d Up);

	//! Set the camera by copying another camera
	GLC_Camera& setCam(const GLC_Camera&);

   //! Set camera's eye coordinate vector
	GLC_Camera& setEyeCam(const GLC_Point4d &Eye);

	//! Set camera's target coordinate vector
	GLC_Camera& setTargetCam(const GLC_Point4d &Target);

	//! Set camera's Up vector
	GLC_Camera& setUpCam(const GLC_Vector4d &Up);

	//! Set the distance between eye and target (move eye)
	GLC_Camera& setDistEyeTarget(double Longueur);

	//! Assignement operator
	GLC_Camera& operator=(const GLC_Camera&);

	//! Set the default Up vector
	/*! Must Be X, Y or Z Axis*/
	inline GLC_Camera& setDefaultUpVector(const GLC_Vector4d& up)
	{
		Q_ASSERT((up == glc::X_AXIS) or (up == glc::Y_AXIS) or (up == glc::Z_AXIS));
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
	GLC_Point4d m_Eye;

	//! Camera's target point
	GLC_Point4d m_Target;

	//! Camera's Up vector
	GLC_Vector4d m_VectUp;

	//! Camera orbit composition matrix
	GLC_Matrix4x4 m_MatCompOrbit;

	//! The default Up axis
	GLC_Vector4d m_DefaultVectUp;
};
#endif //GLC_CAMERA_H_
