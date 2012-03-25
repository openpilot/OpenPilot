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
//! \file glc_polyline.cpp implementation of the GLC_Polylines class.

#include "glc_polylines.h"

GLC_Polylines::GLC_Polylines()
: GLC_Geometry("Polyline", true)
{

}

GLC_Polylines::GLC_Polylines(const GLC_Polylines& polyline)
: GLC_Geometry(polyline)
{

}

GLC_Polylines::~GLC_Polylines()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////
const GLC_BoundingBox& GLC_Polylines::boundingBox()
{
	if (NULL == GLC_Geometry::m_pBoundingBox)
	{
		GLC_Geometry::m_pBoundingBox= new GLC_BoundingBox();
		if (! m_WireData.isEmpty())
		{
			GLC_Geometry::m_pBoundingBox->combine(m_WireData.boundingBox());
		}
	}
	return *GLC_Geometry::m_pBoundingBox;
}

GLC_Geometry* GLC_Polylines::clone() const
{
	return new GLC_Polylines(*this);
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////


GLC_uint GLC_Polylines::addPolyline(const QList<GLC_Point3d>& pointsList)
{
	const int pointCount= pointsList.size();
	const int size= pointCount * 3;
	GLfloatVector data(size);
	for (int i= 0; i < pointCount; ++i)
	{
		const GLC_Point3d currentPoint(pointsList.at(i));
		data[i * 3]= static_cast<float>(currentPoint.x());
		data[i * 3 + 1]= static_cast<float>(currentPoint.y());
		data[i * 3 + 2]= static_cast<float>(currentPoint.z());
	}
	return GLC_Geometry::m_WireData.addVerticeGroup(data);
}

GLC_uint GLC_Polylines::addPolyline(const QList<GLC_Point3df>& pointsList)
{
	const int pointCount= pointsList.size();
	const int size= pointCount * 3;
	GLfloatVector data(size);
	for (int i= 0; i < pointCount; ++i)
	{
		const GLC_Point3df currentPoint(pointsList.at(i));
		data[i * 3]= currentPoint.x();
		data[i * 3 + 1]= currentPoint.y();
		data[i * 3 + 2]= currentPoint.z();
	}
	return GLC_Geometry::m_WireData.addVerticeGroup(data);
}

GLC_Polylines& GLC_Polylines::operator=(const GLC_Polylines& polyline)
{
	if (this != &polyline)
	{
		GLC_Geometry::operator=(polyline);
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////
void GLC_Polylines::glDraw(const GLC_RenderProperties& renderProperties)
{
	if (!GLC_Geometry::m_WireData.isEmpty())
	{
		GLC_Geometry::m_WireData.glDraw(renderProperties, GL_LINE_STRIP);
	}
}

