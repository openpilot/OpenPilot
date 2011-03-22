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
//! \file glc_arrow.cpp implementation of the GLC_Arrow class.

#include <QtGlobal>
#include "../maths/glc_utils_maths.h"
#include "glc_arrow.h"


GLC_Arrow::GLC_Arrow(const GLC_Point3d& startPoint, const GLC_Point3d& endPoint, const GLC_Vector3d& viewDir)
: GLC_Geometry("Arrow", true)
, m_StartPoint(startPoint)
, m_EndPoint(endPoint)
, m_HeadLenght((m_EndPoint - m_StartPoint).length() / 10.0)
, m_HeadAngle(glc::toRadian(30.0))
, m_ViewDir(GLC_Vector3d(viewDir).normalize())
{

}

GLC_Arrow::GLC_Arrow(const GLC_Arrow& arrow)
: GLC_Geometry(arrow)
, m_StartPoint(arrow.m_StartPoint)
, m_EndPoint(arrow.m_EndPoint)
, m_HeadLenght(arrow.m_HeadLenght)
, m_HeadAngle(arrow.m_HeadAngle)
, m_ViewDir(arrow.m_ViewDir)
{

}

GLC_Arrow::~GLC_Arrow()
{

}
//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////
const GLC_BoundingBox& GLC_Arrow::boundingBox()
{
	if (NULL == m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox();
		if (m_WireData.isEmpty()) createWire();
		m_pBoundingBox->combine(m_WireData.boundingBox());
	}
	return *m_pBoundingBox;
}

GLC_Geometry* GLC_Arrow::clone() const
{
	return new GLC_Arrow(*this);
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
GLC_Arrow& GLC_Arrow::operator=(const GLC_Arrow& arrow)
{
	if (this != &arrow)
	{
		GLC_Geometry::operator=(arrow);
		m_StartPoint= arrow.m_StartPoint;
		m_EndPoint= arrow.m_EndPoint;
		m_HeadLenght= arrow.m_HeadLenght;
		m_HeadAngle= arrow.m_HeadAngle;
		m_ViewDir= arrow.m_ViewDir;
	}
	return *this;
}

void GLC_Arrow::setStartPoint(const GLC_Point3d& startPoint)
{
	if (startPoint != m_StartPoint)
	{
		m_StartPoint= startPoint;
		GLC_Geometry::clearWireAndBoundingBox();
	}
}

void GLC_Arrow::setEndPoint(const GLC_Point3d& endPoint)
{
	if (endPoint != m_EndPoint)
	{
		m_EndPoint= endPoint;
		GLC_Geometry::clearWireAndBoundingBox();
	}
}

void GLC_Arrow::setHeadLength(double headLenght)
{
	if (!qFuzzyCompare(m_HeadLenght, headLenght))
	{
		m_HeadLenght= headLenght;
		GLC_Geometry::clearWireAndBoundingBox();
	}
}

void GLC_Arrow::setHeadAngle(double headAngle)
{
	if (!qFuzzyCompare(m_HeadAngle, headAngle))
	{
		m_HeadAngle= headAngle;
		GLC_Geometry::clearWireAndBoundingBox();
	}
}

void GLC_Arrow::setViewDir(const GLC_Vector3d& viewDir)
{

	if (viewDir != m_ViewDir)
	{
		m_ViewDir= GLC_Vector3d(viewDir).normalize();
		GLC_Geometry::clearWireAndBoundingBox();
	}
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////
void GLC_Arrow::glDraw(const GLC_RenderProperties& renderProperties)
{
	if (m_WireData.isEmpty())
	{
		createWire();
	}

	m_WireData.glDraw(renderProperties, GL_LINE_STRIP);
}

void GLC_Arrow::createWire()
{
	Q_ASSERT(m_WireData.isEmpty());
	GLfloatVector floatVector;
	floatVector.append(static_cast<float>(m_StartPoint.x()));
	floatVector.append(static_cast<float>(m_StartPoint.y()));
	floatVector.append(static_cast<float>(m_StartPoint.z()));
	floatVector.append(static_cast<float>(m_EndPoint.x()));
	floatVector.append(static_cast<float>(m_EndPoint.y()));
	floatVector.append(static_cast<float>(m_EndPoint.z()));

	GLC_Geometry::addVerticeGroup(floatVector);

	// Arrow Head
	GLC_Point3d headPoint1(-m_HeadLenght, m_HeadLenght * tan(m_HeadAngle / 2.0), 0.0);
	GLC_Point3d headPoint2(headPoint1.x(), -(headPoint1.y()), headPoint1.z());

	// Arrow frame
	GLC_Vector3d xArrow= (m_EndPoint - m_StartPoint).normalize();
	GLC_Vector3d yArrow= ((-m_ViewDir) ^ xArrow).normalize();
	GLC_Vector3d zArrow= (xArrow ^ yArrow).normalize();

	GLC_Matrix4x4 headMatrix;
	headMatrix.setColumn(0, xArrow);
	headMatrix.setColumn(1, yArrow);
	headMatrix.setColumn(2, zArrow);
	GLC_Matrix4x4 translate(m_EndPoint);
	headPoint1= translate * headMatrix * headPoint1;
	headPoint2= translate * headMatrix * headPoint2;

	// add head data
	floatVector.clear();
	floatVector.append(static_cast<float>(headPoint1.x()));
	floatVector.append(static_cast<float>(headPoint1.y()));
	floatVector.append(static_cast<float>(headPoint1.z()));

	floatVector.append(static_cast<float>(m_EndPoint.x()));
	floatVector.append(static_cast<float>(m_EndPoint.y()));
	floatVector.append(static_cast<float>(m_EndPoint.z()));

	floatVector.append(static_cast<float>(headPoint2.x()));
	floatVector.append(static_cast<float>(headPoint2.y()));
	floatVector.append(static_cast<float>(headPoint2.z()));

	GLC_Geometry::addVerticeGroup(floatVector);

}
