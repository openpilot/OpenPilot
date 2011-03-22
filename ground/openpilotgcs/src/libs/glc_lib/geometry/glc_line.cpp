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

#include "glc_line.h"
#include "../glc_openglexception.h"

//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////

GLC_Line::GLC_Line(const GLC_Point3d & point1, const GLC_Point3d & point2)
: GLC_Geometry("Line", true)
, m_Point1(point1)
, m_Point2(point2)
{

}

GLC_Line::GLC_Line(const GLC_Line& line)
: GLC_Geometry(line)
, m_Point1(line.m_Point1)
, m_Point2(line.m_Point2)
{

}

GLC_Line::~GLC_Line()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

const GLC_BoundingBox& GLC_Line::boundingBox(void)
{

	if (NULL == m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox();

		m_pBoundingBox->combine(m_Point1);
		m_pBoundingBox->combine(m_Point2);
	}
	return *m_pBoundingBox;
}

GLC_Geometry* GLC_Line::clone() const
{
	return new GLC_Line(*this);
}


//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
void GLC_Line::setColor(const QColor& color)
{
	m_WireColor= color;
	if (GLC_Geometry::hasMaterial())
	{
		GLC_Geometry::firstMaterial()->setDiffuseColor(color);
	}
}
//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_Line::glDraw(const GLC_RenderProperties&)
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

