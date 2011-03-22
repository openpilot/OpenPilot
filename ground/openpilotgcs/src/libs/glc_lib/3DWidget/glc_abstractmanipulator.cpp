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

#include "glc_abstractmanipulator.h"
#include "../viewport/glc_viewport.h"
#include "../maths/glc_geomtools.h"

#include <QtGlobal>

GLC_AbstractManipulator::GLC_AbstractManipulator(GLC_Viewport* pViewport)
: m_pViewport(pViewport)
, m_SliddingPlane()
, m_PreviousPosition()
, m_IsInManipulateState(false)
{
	Q_ASSERT(NULL != m_pViewport);
}

GLC_AbstractManipulator::GLC_AbstractManipulator(const GLC_AbstractManipulator& abstractManipulator)
: m_pViewport(abstractManipulator.m_pViewport)
, m_SliddingPlane(abstractManipulator.m_SliddingPlane)
, m_PreviousPosition(abstractManipulator.m_PreviousPosition)
, m_IsInManipulateState(abstractManipulator.m_IsInManipulateState)
{

}

GLC_AbstractManipulator::~GLC_AbstractManipulator()
{
}

void GLC_AbstractManipulator::enterManipulateState(const GLC_Point3d& startPoint)
{
	m_SliddingPlane= GLC_Plane(m_pViewport->cameraHandle()->forward().normalize(), startPoint);

	m_PreviousPosition = startPoint;
	m_IsInManipulateState= true;
}

GLC_Matrix4x4 GLC_AbstractManipulator::manipulate(const GLC_Point3d& newPoint)
{
	Q_ASSERT(m_IsInManipulateState);

	// Select the projection direction
	GLC_Vector3d projectionDirection;
	if (m_pViewport->useOrtho())
	{
		projectionDirection= m_pViewport->cameraHandle()->forward().normalize();
	}
	else
	{
		projectionDirection= (newPoint - m_pViewport->cameraHandle()->eye());
	}

	// Use concrete class to compute matrix
	GLC_Matrix4x4 transformation(doManipulate(newPoint, projectionDirection));

	return transformation;
}
