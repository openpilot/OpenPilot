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
: GLC_Polylines()
, m_Point1(point1)
, m_Point2(point2)
{
	createWire();
}

GLC_Line::GLC_Line(const GLC_Line& line)
: GLC_Polylines(line)
, m_Point1(line.m_Point1)
, m_Point2(line.m_Point2)
{
	createWire();
}

GLC_Line::~GLC_Line()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

const GLC_BoundingBox& GLC_Line::boundingBox(void)
{
	return GLC_Polylines::boundingBox();
}

GLC_Geometry* GLC_Line::clone() const
{
	return new GLC_Line(*this);
}


//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
void GLC_Line::setCoordinate(const GLC_Point3d &point1, const GLC_Point3d &point2)
{
	m_Point1= point1;
	m_Point2= point2;
	clear();
	createWire();
}

GLC_Line& GLC_Line::operator=(const GLC_Line& line)
{
	if (this != &line)
	{
		m_Point1= line.m_Point1;
		m_Point2= line.m_Point2;
		GLC_Polylines::operator=(line);
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_Line::glDraw(const GLC_RenderProperties& renderProperties)
{
	GLC_Polylines::glDraw(renderProperties);
}


//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
void  GLC_Line::createWire()
{
	QList<GLC_Point3d> points;
	points.append(m_Point1);
	points.append(m_Point2);
	GLC_Polylines::addPolyline(points);
}


