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
//! \file glc_pullmanipulator.cpp Implementation of the GLC_PullManipulator class.

#include "glc_pullmanipulator.h"
#include "../maths/glc_line3d.h"
#include "../maths/glc_geomtools.h"
#include "../viewport/glc_viewport.h"

#include <QtGlobal>

GLC_PullManipulator::GLC_PullManipulator(GLC_Viewport* pViewport, const GLC_Vector3d& pullDirection)
: GLC_AbstractManipulator(pViewport)
, m_PullDirection(pullDirection)
{

}

GLC_PullManipulator::GLC_PullManipulator(const GLC_PullManipulator& pullManipulator)
: GLC_AbstractManipulator(pullManipulator)
, m_PullDirection(pullManipulator.m_PullDirection)
{

}

GLC_PullManipulator::~GLC_PullManipulator()
{

}

GLC_AbstractManipulator* GLC_PullManipulator::clone() const
{
	return new GLC_PullManipulator(*this);
}

void GLC_PullManipulator::setPullingDirection(const GLC_Vector3d& pullingDirection)
{
	Q_ASSERT(!GLC_AbstractManipulator::isInManipulateState());
	m_PullDirection= pullingDirection;
}

GLC_Matrix4x4 GLC_PullManipulator::doManipulate(const GLC_Point3d& newPoint, const GLC_Vector3d& projectionDirection)
{
	// Project the given point on the sliding plane with the given direction
	GLC_Point3d projectedPoint;
	GLC_Line3d projectionLine(newPoint, projectionDirection);
	glc::lineIntersectPlane(projectionLine, GLC_AbstractManipulator::m_SliddingPlane, &projectedPoint);

	// Project the point on the pulling direction
	projectedPoint= glc::project(projectedPoint, GLC_Line3d(GLC_AbstractManipulator::previousPosition(), m_PullDirection));

	// Compute the translation matrix
	GLC_Matrix4x4 translationMatrix(projectedPoint - GLC_AbstractManipulator::m_PreviousPosition);

	// Update previous position to this position
	GLC_AbstractManipulator::m_PreviousPosition= projectedPoint;
	return translationMatrix;
}
