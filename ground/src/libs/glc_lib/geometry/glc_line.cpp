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

#include "glc_line.h"
#include "../glc_openglexception.h"

//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////

// Construct an GLC_Line by to point
GLC_Line::GLC_Line(const GLC_Point4d & point1, const GLC_Point4d & point2)
: GLC_VboGeom("Line", true)
, m_Point1(point1)
, m_Point2(point2)
{

}

// Copy constructor
GLC_Line::GLC_Line(const GLC_Line& line)
: GLC_VboGeom(line)
, m_Point1(line.m_Point1)
, m_Point2(line.m_Point2)
{

}

//! Default destructor
GLC_Line::~GLC_Line()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// return the line bounding box
GLC_BoundingBox& GLC_Line::boundingBox(void)
{

	if (NULL == m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox();

		m_pBoundingBox->combine(m_Point1);
		m_pBoundingBox->combine(m_Point2);
	}
	return *m_pBoundingBox;
}

// Return a copy of the current geometry
GLC_VboGeom* GLC_Line::clone() const
{
	return new GLC_Line(*this);
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_Line::glDraw(bool)
{
	// Point Display
	glBegin(GL_LINES);
		glVertex3dv(m_Point1.data());
		glVertex3dv(m_Point2.data());
	glEnd();

	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_Line::GlDraw ", error);
		throw(OpenGlException);
	}
}

