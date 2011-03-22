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
:GLC_Geometry("Point", true)
, m_Coordinate(setCoord)
, m_Size(1.0f)
{

}
//! Construct an GLC_Point
GLC_Point::GLC_Point(double x, double y, double z)
:GLC_Geometry("Point", true)
, m_Coordinate(x, y, z)
, m_Size(1.0f)
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

// return the point bounding box
const GLC_BoundingBox& GLC_Point::boundingBox(void)
{

	if (NULL == m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox();
		const double delta= 1e-2;
		GLC_Point3d lower(m_Coordinate.x() - delta,
				m_Coordinate.y() - delta,
				m_Coordinate.z() - delta);
		GLC_Point3d upper(m_Coordinate.x() + delta,
				m_Coordinate.y() + delta,
				m_Coordinate.z() + delta);
		m_pBoundingBox->combine(lower);
		m_pBoundingBox->combine(upper);
	}
	return *m_pBoundingBox;
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
}
// Set Point coordinate by 3 double
void GLC_Point::setCoordinate(double x, double y, double z)
{
	m_Coordinate.setVect(x, y, z);
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_Point::glDraw(const GLC_RenderProperties&)
{
	glPointSize(m_Size);
	// Point Display
	glBegin(GL_POINTS);
		glVertex3dv(m_Coordinate.data());
	glEnd();
	glPointSize(1.0f);

	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_Point::GlDraw ", error);
		throw(OpenGlException);
	}
}

