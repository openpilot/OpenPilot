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

//! \file glc_point.cpp implementation of the GLC_Point class.

#include "glc_point.h"
#include "../glc_openglexception.h"

using namespace glc;
//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////


GLC_Point::GLC_Point(const GLC_Point3d &setCoord)
:GLC_PointCloud()
, m_Coordinate(setCoord)
, m_Size(1.0f)
{
	setCoordinate(m_Coordinate);
}

GLC_Point::GLC_Point(double x, double y, double z)
:GLC_PointCloud()
, m_Coordinate(x, y, z)
, m_Size(1.0f)
{
	setCoordinate(m_Coordinate);
}

GLC_Point::GLC_Point(const GLC_Point& point)
:GLC_PointCloud(point)
, m_Coordinate(point.m_Coordinate)
, m_Size(point.m_Size)
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Get a 4D point represent point coordinate
GLC_Point3d GLC_Point::coordinate(void) const
{
	return m_Coordinate;
}

// Return a copy of the current geometry
GLC_Geometry* GLC_Point::clone() const
{
	return new GLC_Point(*this);
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set Point coordinate by 4D Vector
void GLC_Point::setCoordinate(const GLC_Point3d &point)
{
	m_Coordinate= point;
	GLC_PointCloud::clear();
	QList<GLC_Point3d> points;
	points.append(m_Coordinate);
	GLC_PointCloud::addPoint(points);

}
// Set Point coordinate by 3 double
void GLC_Point::setCoordinate(double x, double y, double z)
{
	setCoordinate(GLC_Point3d(x, y, z));
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_Point::glDraw(const GLC_RenderProperties& renderProperties)
{
	glPointSize(m_Size);
	// Point Display
	GLC_PointCloud::glDraw(renderProperties);
}

