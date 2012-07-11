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

//! \file glc_camera.cpp Implementation of the GLC_Camera class.

#include "glc_camera.h"
#include "../glc_context.h"

#include <QtDebug>

using namespace glc;
//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////
GLC_Camera::GLC_Camera()
: GLC_Object("Camera")
, m_Eye(0,0,1)
, m_Target()
, m_VectUp(Y_AXIS)
, m_ModelViewMatrix()
, m_DefaultVectUp(Y_AXIS)
{

}

GLC_Camera::GLC_Camera(const GLC_Point3d &Eye, const GLC_Point3d &Target, const GLC_Vector3d &Up)
: GLC_Object("Camera")
, m_Eye()
, m_Target()
, m_VectUp()
, m_ModelViewMatrix()
, m_DefaultVectUp(Y_AXIS)
{
	setCam(Eye, Target, Up);
	createMatComp();
}

// Copy constructor
GLC_Camera::GLC_Camera(const GLC_Camera& cam)
: GLC_Object(cam)
, m_Eye(cam.m_Eye)
, m_Target(cam.m_Target)
, m_VectUp(cam.m_VectUp)
, m_ModelViewMatrix(cam.m_ModelViewMatrix)
, m_DefaultVectUp(cam.m_DefaultVectUp)
{

}

/////////////////////////////////////////////////////////////////////
// Get Functions
/////////////////////////////////////////////////////////////////////

// equality operator
bool GLC_Camera::operator==(const GLC_Camera& cam) const
{
	return (m_Eye == cam.m_Eye) && (m_Target == cam.m_Target)
			&& (m_VectUp == cam.m_VectUp) && (m_DefaultVectUp == cam.m_DefaultVectUp);
}


/////////////////////////////////////////////////////////////////////
// Set Functions
/////////////////////////////////////////////////////////////////////
GLC_Camera& GLC_Camera::orbit(GLC_Vector3d VectOldPoss, GLC_Vector3d VectCurPoss)
{
	// Map Vectors
	GLC_Matrix4x4 invMat(m_ModelViewMatrix);
	invMat.invert();
	VectOldPoss= invMat * VectOldPoss;
	VectCurPoss= invMat * VectCurPoss;

	// Compute rotation matrix
	const GLC_Vector3d VectAxeRot(VectCurPoss ^ VectOldPoss);
	// Check if rotation vector is not null
	if (!VectAxeRot.isNull())
	{  // Ok, is not null
		const double Angle= acos(VectCurPoss * VectOldPoss);
		const GLC_Matrix4x4 MatOrbit(VectAxeRot, Angle);

		// Camera transformation
		m_Eye= (MatOrbit * (m_Eye - m_Target)) + m_Target;
		m_VectUp= MatOrbit * m_VectUp;
		createMatComp();
	}

	return *this;
}

GLC_Camera& GLC_Camera::pan(GLC_Vector3d VectDep)
{
	// Vector mapping
	GLC_Matrix4x4 invMat(m_ModelViewMatrix);
	invMat.invert();
	VectDep= invMat * VectDep;

	// Camera transformation
	m_Eye= m_Eye + VectDep;
	m_Target= m_Target + VectDep;

	return *this;
}

GLC_Camera& GLC_Camera::zoom(double factor)
{
	Q_ASSERT(factor > 0);
	// Eye->target vector
	GLC_Vector3d VectCam(m_Eye - m_Target);

	// Compute new vector length
	const double Norme= VectCam.length() * 1 / factor;
	VectCam.setLength(Norme);

	m_Eye= VectCam + m_Target;

	return *this;
}

// Move camera
GLC_Camera& GLC_Camera::move(const GLC_Matrix4x4 &MatMove)
{
	m_Eye= MatMove * m_Eye;
	m_Target= MatMove * m_Target;
	m_VectUp= MatMove.rotationMatrix() * m_VectUp;
	createMatComp();

	return *this;
}

// Rotate around an axis
GLC_Camera& GLC_Camera::rotateAround(const GLC_Vector3d& axis, const double& angle, const GLC_Point3d& point)
{
	const GLC_Matrix4x4 rotationMatrix(axis, angle);
	translate(-point);
	move(rotationMatrix);
	translate(point);

	return *this;
}

// Rotate around camera target
GLC_Camera& GLC_Camera::rotateAroundTarget(const GLC_Vector3d& axis, const double& angle)
{
	GLC_Point3d target(m_Target);
	rotateAround(axis, angle, target);

	return *this;
}

GLC_Camera& GLC_Camera::translate(const GLC_Vector3d &VectTrans)
{
	m_Eye= m_Eye + VectTrans;
	m_Target= m_Target + VectTrans;

	return *this;
}

GLC_Camera& GLC_Camera::setEyeCam(const GLC_Point3d &Eye)
{
	// Old camera's vector
	GLC_Vector3d VectOldCam(m_Eye - m_Target);
	// New camera's vector
	GLC_Vector3d VectCam(Eye - m_Target);
	if ( !(VectOldCam - VectCam).isNull() )
	{
		VectOldCam.setLength(1);
		VectCam.setLength(1);
		const double Angle= acos(VectOldCam * VectCam);
		if ( !qFuzzyCompare(Angle, 0.0) && !qFuzzyCompare(PI - Angle, 0.0))
		{
			const GLC_Vector3d VectAxeRot(VectOldCam ^ VectCam);
			const GLC_Matrix4x4 MatRot(VectAxeRot, Angle);
			m_VectUp= MatRot * m_VectUp;
		}
		else
		{
			if ( qFuzzyCompare(PI - Angle, 0.0))
			{	// Angle de 180%
				m_VectUp.invert();
			}
		}

		setCam(Eye, m_Target, m_VectUp);
	}

	return *this;
}

GLC_Camera& GLC_Camera::setTargetCam(const GLC_Point3d &Target)
{
	// Old camera's vector
	GLC_Vector3d VectOldCam(m_Eye - m_Target);
	// New camera's vector
	GLC_Vector3d VectCam(m_Eye - Target);
	if ( !(VectOldCam - VectCam).isNull() )
	{
		VectOldCam.setLength(1);
		VectCam.setLength(1);
		const double Angle= acos(VectOldCam * VectCam);
		if ( !qFuzzyCompare(Angle, 0.0) && !qFuzzyCompare(PI - Angle, 0.0))
		{
			const GLC_Vector3d VectAxeRot(VectOldCam ^ VectCam);
			const GLC_Matrix4x4 MatRot(VectAxeRot, Angle);
			m_VectUp= MatRot * m_VectUp;
		}
		else
		{
			if ( qFuzzyCompare(PI - Angle, 0.0))
			{	// Angle of 180%
				m_VectUp.invert();
			}
		}

		setCam(m_Eye, Target, m_VectUp);
	}

	return *this;
}

GLC_Camera& GLC_Camera::setUpCam(const GLC_Vector3d &Up)
{
	if ( !(m_VectUp - Up).isNull() )
	{
		if (!qFuzzyCompare(forward().angleWithVect(Up), 0.0))
		{
			setCam(m_Eye, m_Target, Up);
		}
	}

	return *this;
}

GLC_Camera& GLC_Camera::setCam(GLC_Point3d Eye, GLC_Point3d Target, GLC_Vector3d Up)
{
	Up.setLength(1);

	const GLC_Vector3d VectCam((Eye - Target).setLength(1));
	const double Angle= acos(VectCam * Up);

	/* m_VectUp and VectCam could not be parallel
	 * m_VectUp could not be NULL
	 * VectCam could not be NULL */
	//Q_ASSERT((Angle > EPSILON) && ((PI - Angle) > EPSILON));

	if ( !qFuzzyCompare(Angle - (PI / 2), 0.0))
	{	// Angle not equal to 90
		const GLC_Vector3d AxeRot(VectCam ^ Up);
		GLC_Matrix4x4 MatRot(AxeRot, PI / 2);
		Up= MatRot * VectCam;
	}

	m_Eye= Eye;
	m_Target= Target;
	m_VectUp= Up;
	createMatComp();

	return *this;
}

//! Set the camera by copying another camera
GLC_Camera& GLC_Camera::setCam(const GLC_Camera& cam)
{
	m_Eye= cam.m_Eye;
	m_Target= cam.m_Target;
	m_VectUp= cam.m_VectUp;
	m_ModelViewMatrix= cam.m_ModelViewMatrix;

	return *this;
}


GLC_Camera& GLC_Camera::setDistEyeTarget(double Longueur)
{
    GLC_Vector3d VectCam(forward());
    VectCam.setLength(Longueur);
    m_Eye= m_Target - VectCam;

    return *this;
}
GLC_Camera& GLC_Camera::setDistTargetEye(double Longueur)
{
    GLC_Vector3d VectCam(forward());
    VectCam.setLength(Longueur);
    m_Target= m_Eye + VectCam;

    return *this;
}

// Assignment operator
GLC_Camera &GLC_Camera::operator=(const GLC_Camera& cam)
{
	GLC_Object::operator=(cam);
	m_Eye= cam.m_Eye;
	m_Target= cam.m_Target;
	m_VectUp= cam.m_VectUp;
	m_ModelViewMatrix= cam.m_ModelViewMatrix;
	m_DefaultVectUp= cam.m_DefaultVectUp;

	return *this;
}
// almost equality (Bauer Laurent)
bool GLC_Camera::isAlmostEqualTo(const GLC_Camera& cam, const double distanceAccuracy) const
{
      GLC_Vector3d incident1 = m_Target - m_Eye;
      GLC_Vector3d incident2 = cam.m_Target - cam.m_Eye;

      double allowedGap =  incident1.length() * distanceAccuracy;
      GLC_Point3d left1 = incident1 ^ m_VectUp;
      GLC_Point3d left2 = incident2 ^ cam.m_VectUp;

      return ((m_Eye - cam.m_Eye).length() < allowedGap ) && ( (m_Target - cam.m_Target).length() < allowedGap)
                  && ((left1 - left2).length() < allowedGap) ;
}

// Return the standard front view form this camera
GLC_Camera GLC_Camera::frontView() const
{
	GLC_Vector3d eye;

	if (m_DefaultVectUp == glc::Z_AXIS)
	{
		eye.setVect(0.0, -1.0, 0.0);
	}
	else // Y_AXIS or X_AXIS
	{
		eye.setVect(0.0, 0.0, 1.0);
	}
	eye= eye + m_Target;

	GLC_Camera newCam(eye, m_Target, m_DefaultVectUp);
	newCam.setDistEyeTarget(distEyeTarget());
	newCam.setDefaultUpVector(m_DefaultVectUp);
	return newCam;
}

// Return the standard rear view form this camera
GLC_Camera GLC_Camera::rearView() const
{
	return frontView().rotateAroundTarget(m_DefaultVectUp, glc::PI);
}

// Return the standard right view form this camera
GLC_Camera GLC_Camera::rightView() const
{
	return frontView().rotateAroundTarget(m_DefaultVectUp, glc::PI / 2.0);}

// Return the standard left view form this camera
GLC_Camera GLC_Camera::leftView() const
{
	return frontView().rotateAroundTarget(m_DefaultVectUp, - glc::PI / 2.0);
}

// Return the standard top view form this camera
GLC_Camera GLC_Camera::topView() const
{
	GLC_Vector3d eye= m_DefaultVectUp;
	eye= eye + m_Target;
	GLC_Vector3d up;

	if (m_DefaultVectUp == glc::Y_AXIS)
	{
		up.setVect(0.0, 0.0, -1.0);
	}
	else // Z_AXIS or X_AXIS
	{
		up.setVect(0.0, 1.0, 0.0);
	}

	GLC_Camera newCam(eye, m_Target, up);
	newCam.setDistEyeTarget(distEyeTarget());
	newCam.setDefaultUpVector(m_DefaultVectUp);

	return newCam;
}

// Return the standard bottom view form this camera
GLC_Camera GLC_Camera::bottomView() const
{
	GLC_Camera newCam(topView());
	newCam.rotateAroundTarget(newCam.upVector(), glc::PI);

	return newCam;
}

// Return the standard isoview from his camera
GLC_Camera GLC_Camera::isoView() const
{
	GLC_Vector3d eye;
	if (m_DefaultVectUp == glc::Z_AXIS)
	{
		eye.setVect(-1.0, -1.0, 1.0);
	}
	else if (m_DefaultVectUp == glc::Y_AXIS)
	{
		eye.setVect(-1.0, 1.0, 1.0);
	}
	else
	{
		eye.setVect(1.0, 1.0, 1.0);
	}

	eye= eye + m_Target;

	GLC_Camera newCam(eye, m_Target, m_DefaultVectUp);
	newCam.setDistEyeTarget(distEyeTarget());
	newCam.setDefaultUpVector(m_DefaultVectUp);
	return newCam;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////
void GLC_Camera::glExecute()
{
	GLC_Context::current()->glcMultMatrix(modelViewMatrix());
}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

void GLC_Camera::createMatComp(void)
{
	const GLC_Vector3d forward((m_Target - m_Eye).normalize());
	const GLC_Vector3d side((forward ^ m_VectUp).normalize());

	// Update camera matrix
	m_ModelViewMatrix.setData()[0]= side.x();
	m_ModelViewMatrix.setData()[4]= side.y();
	m_ModelViewMatrix.setData()[8]= side.z();
	m_ModelViewMatrix.setData()[12]= 0.0;

	// Vector Up is Y Axis
	m_ModelViewMatrix.setData()[1]= m_VectUp.x();
	m_ModelViewMatrix.setData()[5]= m_VectUp.y();
	m_ModelViewMatrix.setData()[9]= m_VectUp.z();
	m_ModelViewMatrix.setData()[13]= 0.0;

	// Vector Cam is Z axis
	m_ModelViewMatrix.setData()[2]= - forward.x();
	m_ModelViewMatrix.setData()[6]= - forward.y();
	m_ModelViewMatrix.setData()[10]= - forward.z();
	m_ModelViewMatrix.setData()[14]= 0.0;

	m_ModelViewMatrix.setData()[3]= 0.0;
	m_ModelViewMatrix.setData()[7]= 0.0;
	m_ModelViewMatrix.setData()[11]= 0.0;
	m_ModelViewMatrix.setData()[15]= 1.0;

}
