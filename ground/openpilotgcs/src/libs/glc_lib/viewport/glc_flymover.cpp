/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
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
//! \file glc_flymover.cpp Implementation of the GLC_FlyMover class.

#include "glc_flymover.h"
#include "glc_viewport.h"
#include "../maths/glc_utils_maths.h"

GLC_FlyMover::GLC_FlyMover(GLC_Viewport* pViewport, const QList<GLC_RepMover*>& repsList)
: GLC_Mover(pViewport, repsList)
, m_TurnRate(glc::toRadian(6))
, m_TimerId(0)
, m_TimerInterval(66)
, m_Velocity(1.0)
{
	GLC_Mover::m_MoverInfo.m_VectorInfo.append(GLC_Vector3d());
	GLC_Mover::m_MoverInfo.m_DoubleInfo.append(m_Velocity);
}

GLC_FlyMover::GLC_FlyMover(const GLC_FlyMover& flyMover)
: GLC_Mover(flyMover)
, m_TurnRate(flyMover.m_TurnRate)
, m_TimerId(0)
, m_TimerInterval(flyMover.m_TimerInterval)
, m_Velocity(flyMover.m_Velocity)
{

}

GLC_FlyMover::~GLC_FlyMover()
{
	if (0 != m_TimerId)
	{
		QObject::killTimer(m_TimerId);
	}
}

//////////////////////////////////////////////////////////////////////
//Get Functions
//////////////////////////////////////////////////////////////////////

GLC_Mover* GLC_FlyMover::clone() const
{
	return new GLC_FlyMover(*this);
}


//////////////////////////////////////////////////////////////////////
//Set Functions
//////////////////////////////////////////////////////////////////////

void GLC_FlyMover::init(const GLC_UserInput& userInput)
{
	m_PreviousVector= mapForFlying(static_cast<double>(userInput.x()), static_cast<double>(userInput.y()));
	GLC_Point3d point= m_pViewport->unProject(userInput.x(), userInput.y());
	const double distance= (point - m_pViewport->cameraHandle()->eye()).length();
	m_pViewport->cameraHandle()->setDistTargetEye(distance);
	// 5 secondes to travel
	m_Velocity= distance / 5000;

	GLC_Mover::m_MoverInfo.m_DoubleInfo.first()= m_Velocity;

	// Start the timer
	m_TimerId= QObject::startTimer(m_TimerInterval);
}

bool GLC_FlyMover::move(const GLC_UserInput& userInput)
{
	m_PreviousVector= mapForFlying(static_cast<double>(userInput.x()), static_cast<double>(userInput.y()));
	GLC_Point3d point= m_pViewport->unProject(userInput.x(), userInput.y());
	const double distance= (point - m_pViewport->cameraHandle()->eye()).length();
	m_pViewport->cameraHandle()->setDistTargetEye(distance);

	return false;
}
void GLC_FlyMover::ends()
{
	QObject::killTimer(m_TimerId);
	m_TimerId= 0;
}

void GLC_FlyMover::setFlyingVelocity(double velocity)
{
	m_Velocity= velocity;
	GLC_Mover::m_MoverInfo.m_DoubleInfo.first()= m_Velocity;
}

void GLC_FlyMover::increaseVelocity(double factor)
{
	m_Velocity*= factor;
	GLC_Mover::m_MoverInfo.m_DoubleInfo.first()= m_Velocity;
}

void GLC_FlyMover::timerEvent(QTimerEvent*)
{
	fly();
	GLC_Vector3d direction(m_pViewport->cameraHandle()->forward());
	direction.normalize();
	direction= direction * m_Velocity * m_TimerInterval;
	const GLC_Matrix4x4 translation(direction);
	m_pViewport->cameraHandle()->move(translation);

	emit updated();
}

GLC_Vector3d GLC_FlyMover::mapForFlying(double x, double y)
{
	double AspectRatio;
	const double winHSize= static_cast<double>(GLC_Mover::m_pViewport->viewHSize());
	const double winVSize= static_cast<double>(GLC_Mover::m_pViewport->viewVSize());


	// Change origin and cover
	if (winHSize < winVSize)
	{
		AspectRatio= winVSize / winHSize;
		x= ( (x - winHSize  / 2.0 ) / ( winHSize / 2.0) ) / AspectRatio;
		y= ( ( winVSize / 2.0 - y) / ( winVSize / 2.0 ) );
	}
	else
	{
		AspectRatio= winHSize / winVSize;
		x= ( (x - winHSize  / 2.0 ) / ( winHSize / 2.0) );
		y= ( (winVSize / 2.0 - y) / (winVSize / 2.0 ) ) / AspectRatio;
	}

	GLC_Vector3d pos(x, y, 0.0);

	if (pos.length() > 1.0)
	{
		pos.normalize();
	}
	GLC_Mover::m_MoverInfo.m_VectorInfo.first()= pos;
	GLC_Mover::updateRepresentation();

	double z= -cos(m_TurnRate) / sin(m_TurnRate);
	pos.setZ(z);
	pos.normalize();
	return pos;
}
void GLC_FlyMover::fly()
{
	const GLC_Matrix4x4 viewMatrix(m_pViewport->cameraHandle()->viewMatrix());
	const GLC_Vector3d newPos= viewMatrix.inverted() * m_PreviousVector;
	const GLC_Vector3d forward(GLC_Vector3d(m_pViewport->cameraHandle()->forward()).normalize());

	// Compute rotation matrix
	const GLC_Vector3d axe(forward ^ newPos);
	if (!axe.isNull())
	{
		const double angle= acos(forward * newPos);
		const GLC_Matrix4x4 rotation(axe, angle);
		const GLC_Matrix4x4 trans1(-m_pViewport->cameraHandle()->eye());
		const GLC_Matrix4x4 trans2(m_pViewport->cameraHandle()->eye());
		const GLC_Matrix4x4 composition(trans2 * rotation * trans1);
		m_pViewport->cameraHandle()->move(composition);
	}
}
