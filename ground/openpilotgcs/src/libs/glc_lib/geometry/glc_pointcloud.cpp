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
//! \file glc_pointcloud.cpp implementation of the GLC_PointCloud class.

#include "glc_pointcloud.h"

GLC_PointCloud::GLC_PointCloud()
: GLC_Geometry("Point Cloud", true)
{

}

GLC_PointCloud::GLC_PointCloud(const GLC_PointCloud& pointCloud)
: GLC_Geometry(pointCloud)
{


}

GLC_PointCloud::~GLC_PointCloud()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////
const GLC_BoundingBox& GLC_PointCloud::boundingBox()
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

GLC_Geometry* GLC_PointCloud::clone() const
{
	return new GLC_PointCloud(*this);
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

GLC_uint GLC_PointCloud::addPoint(const QList<GLC_Point3d>& pointsList)
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

GLC_uint GLC_PointCloud::addPoint(const QList<GLC_Point3df>& pointsList)
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

void GLC_PointCloud::addColors(const QList<QColor>& colors)
{
	const int colorCount= colors.count();
	const int size= colorCount * 4;
	GLfloatVector data(size);
	for (int i= 0; i < colorCount; ++i)
	{
		QColor color= colors.at(i);
		data[i * 4]= static_cast<GLfloat>(color.redF());
		data[i * 4 + 1]= static_cast<GLfloat>(color.greenF());
		data[i * 4 + 2]= static_cast<GLfloat>(color.blueF());
		data[i * 4 + 3]= static_cast<GLfloat>(color.alphaF());
	}

	GLC_Geometry::m_WireData.addColors(data);
}

GLC_PointCloud& GLC_PointCloud::operator=(const GLC_PointCloud& pointCloud)
{
	if (this != &pointCloud)
	{
		GLC_Geometry::operator=(pointCloud);
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////
void GLC_PointCloud::glDraw(const GLC_RenderProperties& renderProperties)
{
	if (!GLC_Geometry::m_WireData.isEmpty())
	{
		GLC_Geometry::m_WireData.glDraw(renderProperties, GL_POINTS);
	}
}

