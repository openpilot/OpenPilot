/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
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

//! \file glc_boundingbox.cpp implementation of the GLC_BoundingBox class.

#include "glc_boundingbox.h"
#include "maths/glc_matrix4x4.h"
//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////

// Default constructor
GLC_BoundingBox::GLC_BoundingBox()
: m_Lower(0, 0, 0)
, m_Upper(0, 0, 0)
, m_IsEmpty(true)
{

}

// Copy constructor
GLC_BoundingBox::GLC_BoundingBox(const GLC_BoundingBox& boundingBox)
: m_Lower(boundingBox.m_Lower)
, m_Upper(boundingBox.m_Upper)
, m_IsEmpty(boundingBox.m_IsEmpty)
{
}

// Constructor with 2 points.
GLC_BoundingBox::GLC_BoundingBox(const GLC_Point4d& lower, const GLC_Point4d& upper)
: m_Lower(lower)
, m_Upper(upper)
, m_IsEmpty(false)
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Test if a point is in the bounding Box
bool GLC_BoundingBox::intersect(const GLC_Point4d& point) const
{
	if (!m_IsEmpty)
	{
		bool result= (point.X() < m_Upper.X()) && (point.Y() < m_Upper.Y())
		&& (point.Z() < m_Upper.Z()) && (point.X() > m_Lower.X())
		&& (point.Y() > m_Lower.Y()) && (point.Z() > m_Lower.Z());

		return result;
	}
	else
	{
		return false;
	}
}

// Test if a point is in the bounding Sphere
bool GLC_BoundingBox::intersectBoundingSphere(const GLC_Point4d& point) const
{
	const double distance= (center() - point).norm();
	return distance < boundingSphereRadius();
}

// Get the lower corner of the bounding box
GLC_Point4d GLC_BoundingBox::lowerCorner(void) const
{
	return m_Lower;
}

// Get the upper corner of the bounding box
GLC_Point4d GLC_BoundingBox::upperCorner(void) const
{
	return m_Upper;
}

// Get the center of the bounding box
GLC_Point4d GLC_BoundingBox::center(void) const
{
	GLC_Vector4d vectResult = (m_Lower + m_Upper) * (1.0 / 2.0);
	return vectResult;

}


//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Combine the bounding Box with new geometry point
GLC_BoundingBox& GLC_BoundingBox::combine(const GLC_Point4d& point)
{
	if (m_IsEmpty)
	{
		m_Lower= point;
		m_Upper= point;
		m_IsEmpty= false;
	}
	else
	{
		double lowerX= qMin(point.X(), m_Lower.X());
		double lowerY= qMin(point.Y(), m_Lower.Y());
		double lowerZ= qMin(point.Z(), m_Lower.Z());
		m_Lower.setVect(lowerX, lowerY, lowerZ);

		double upperX= qMax(point.X(), m_Upper.X());
		double upperY= qMax(point.Y(), m_Upper.Y());
		double upperZ= qMax(point.Z(), m_Upper.Z());
		m_Upper.setVect(upperX, upperY, upperZ);
	}
	return *this;
}

// Combine the bounding Box with new geometry point
GLC_BoundingBox& GLC_BoundingBox::combine(const GLC_Point3d& point)
{
	if (m_IsEmpty)
	{
		m_Lower= point;
		m_Upper= point;
		m_IsEmpty= false;
	}
	else
	{
		double lowerX= qMin(point.X(), m_Lower.X());
		double lowerY= qMin(point.Y(), m_Lower.Y());
		double lowerZ= qMin(point.Z(), m_Lower.Z());
		m_Lower.setVect(lowerX, lowerY, lowerZ);

		double upperX= qMax(point.X(), m_Upper.X());
		double upperY= qMax(point.Y(), m_Upper.Y());
		double upperZ= qMax(point.Z(), m_Upper.Z());
		m_Upper.setVect(upperX, upperY, upperZ);
	}
	return *this;
}

// Combine the bounding Box with new geometry point
GLC_BoundingBox& GLC_BoundingBox::combine(const GLC_Point3df& pointf)
{
	GLC_Point3d point(pointf);
	if (m_IsEmpty)
	{
		m_Lower= point;
		m_Upper= point;
		m_IsEmpty= false;
	}
	else
	{
		double lowerX= qMin(point.X(), m_Lower.X());
		double lowerY= qMin(point.Y(), m_Lower.Y());
		double lowerZ= qMin(point.Z(), m_Lower.Z());
		m_Lower.setVect(lowerX, lowerY, lowerZ);

		double upperX= qMax(point.X(), m_Upper.X());
		double upperY= qMax(point.Y(), m_Upper.Y());
		double upperZ= qMax(point.Z(), m_Upper.Z());
		m_Upper.setVect(upperX, upperY, upperZ);
	}
	return *this;
}

// Combine the bounding Box with another bounding box
GLC_BoundingBox& GLC_BoundingBox::combine(const GLC_BoundingBox& box)
{
	if (m_IsEmpty)
	{
		m_Lower= box.m_Lower;
		m_Upper= box.m_Upper;
		m_IsEmpty= box.m_IsEmpty;
	}
	else
	{
		double lowerX= qMin(box.m_Lower.X(), m_Lower.X());
		double lowerY= qMin(box.m_Lower.Y(), m_Lower.Y());
		double lowerZ= qMin(box.m_Lower.Z(), m_Lower.Z());
		m_Lower.setVect(lowerX, lowerY, lowerZ);

		double upperX= qMax(box.m_Upper.X(), m_Upper.X());
		double upperY= qMax(box.m_Upper.Y(), m_Upper.Y());
		double upperZ= qMax(box.m_Upper.Z(), m_Upper.Z());
		m_Upper.setVect(upperX, upperY, upperZ);
	}

	return *this;
}

// Transform the bounding Box
GLC_BoundingBox& GLC_BoundingBox::transform(const GLC_Matrix4x4& matrix)
{
	// Compute Transformed BoundingBox Corner
	GLC_Point4d corner1(m_Lower);
	GLC_Point4d corner7(m_Upper);
	GLC_Point4d corner2(corner7.X(), corner1.Y(), corner1.Z());
	GLC_Point4d corner3(corner7.X(), corner7.Y(), corner1.Z());
	GLC_Point4d corner4(corner1.X(), corner7.Y(), corner1.Z());
	GLC_Point4d corner5(corner1.X(), corner1.Y(), corner7.Z());
	GLC_Point4d corner6(corner7.X(), corner1.Y(), corner7.Z());
	GLC_Point4d corner8(corner1.X(), corner7.Y(), corner7.Z());

	corner1 = (matrix * corner1);
	corner2 = (matrix * corner2);
	corner3 = (matrix * corner3);
	corner4 = (matrix * corner4);
	corner5 = (matrix * corner5);
	corner6 = (matrix * corner6);
	corner7 = (matrix * corner7);
	corner8 = (matrix * corner8);

	// Compute the new BoundingBox
	GLC_BoundingBox boundingBox;
	boundingBox.combine(corner1);
	boundingBox.combine(corner2);
	boundingBox.combine(corner3);
	boundingBox.combine(corner4);
	boundingBox.combine(corner5);
	boundingBox.combine(corner6);
	boundingBox.combine(corner7);
	boundingBox.combine(corner8);

	m_Lower= boundingBox.m_Lower;
	m_Upper= boundingBox.m_Upper;
	return *this;
}

