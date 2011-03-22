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
//! \file glc_rotationmanipulator.cpp Implementation of the GLC_RotationManipulator class.

#include "glc_rotationmanipulator.h"
#include "../maths/glc_geomtools.h"

GLC_RotationManipulator::GLC_RotationManipulator(GLC_Viewport* pViewport, const GLC_Line3d& rotationLine)
: GLC_AbstractManipulator(pViewport)
, m_RotationLine(rotationLine)
{

}

GLC_RotationManipulator::GLC_RotationManipulator(const GLC_RotationManipulator& rotationmanipulator)
: GLC_AbstractManipulator(rotationmanipulator)
, m_RotationLine(rotationmanipulator.m_RotationLine)
{

}

GLC_RotationManipulator::~GLC_RotationManipulator()
{

}

GLC_AbstractManipulator* GLC_RotationManipulator::clone() const
{
	return new GLC_RotationManipulator(*this);
}

GLC_Matrix4x4 GLC_RotationManipulator::doManipulate(const GLC_Point3d& newPoint, const GLC_Vector3d& projectionDirection)
{
	// Project the given point on the sliding plane with the given direction
	GLC_Point3d projectedPoint;
	GLC_Line3d projectionLine1(GLC_AbstractManipulator::m_PreviousPosition, projectionDirection);
	GLC_Line3d projectionLine(newPoint, projectionDirection);

	// create the rotation plane
	const GLC_Point3d origine(m_RotationLine.startingPoint());
	GLC_Plane rotationPlane(m_RotationLine.direction(), origine);
	// Project the point on the previous computed plane
	glc::lineIntersectPlane(projectionLine1, rotationPlane, &(GLC_AbstractManipulator::m_PreviousPosition));
	glc::lineIntersectPlane(projectionLine, rotationPlane, &projectedPoint);

	// Compute the the to vector
	GLC_Vector3d vector1((GLC_AbstractManipulator::m_PreviousPosition - origine).normalize());
	GLC_Vector3d vector2((projectedPoint - origine).normalize());

	// Update previous position to this position
	GLC_AbstractManipulator::m_PreviousPosition= projectedPoint;

	// Return the rotation matrix
	const GLC_Matrix4x4 trans1(-origine);
	const GLC_Matrix4x4 trans2(origine);
	const GLC_Matrix4x4 rotation(vector1, vector2);

	return (trans2 * rotation * trans1);

}
