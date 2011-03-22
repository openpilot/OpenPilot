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

//! \file glc_circle.cpp implementation of the GLC_Circle class.

#include "glc_circle.h"
#include "../glc_openglexception.h"
#include "../glc_state.h"

using namespace glc;
//////////////////////////////////////////////////////////////////////
// Constructor destructor
//////////////////////////////////////////////////////////////////////

GLC_Circle::GLC_Circle(const double &dRadius, double Angle)
:GLC_Geometry("Circle", true)
, m_Radius(dRadius)
, m_Discret(GLC_DISCRET)
, m_Angle(Angle)
, m_Step(0)
{

}

GLC_Circle::GLC_Circle(const GLC_Circle& sourceCircle)
:GLC_Geometry(sourceCircle)
, m_Radius(sourceCircle.m_Radius)
, m_Discret(sourceCircle.m_Discret)
, m_Angle(sourceCircle.m_Angle)
, m_Step(sourceCircle.m_Step)
{

}
GLC_Circle::~GLC_Circle()
{

}
//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// return the circle bounding box
const GLC_BoundingBox& GLC_Circle::boundingBox(void)
{
	if (NULL == m_pBoundingBox)
	{
		//qDebug() << "GLC_Mesh2::boundingBox create boundingBox";
		m_pBoundingBox= new GLC_BoundingBox();
		if (m_WireData.isEmpty()) createWire();
		m_pBoundingBox->combine(m_WireData.boundingBox());
		m_pBoundingBox->combine(GLC_Vector3d(0.0, 0.0, -2 * glc::EPSILON));
	}
	return *m_pBoundingBox;
}

// Return a copy of the current geometry
GLC_Geometry* GLC_Circle::clone() const
{
	return new GLC_Circle(*this);
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set Circle diameter
void GLC_Circle::setDiameter(double D)
{
	Q_ASSERT(!qFuzzyCompare(D, 0.0));
	setRadius(D / 2);
}

// Set Circle Radius
void GLC_Circle::setRadius(double R)
{
	Q_ASSERT(!qFuzzyCompare(R, 0.0));
	if (!qFuzzyCompare(R - m_Radius, 0.0))
	{	// Radius is changing
		m_Radius= R;

		GLC_Geometry::clearWireAndBoundingBox();
	}
}

// Set Circle discret
void GLC_Circle::setDiscretion(int TargetDiscret)
{
	Q_ASSERT(TargetDiscret > 0);
	if (TargetDiscret != m_Discret)
	{
		m_Discret= TargetDiscret;
		if (m_Discret < 6) m_Discret= 6;

		GLC_Geometry::clearWireAndBoundingBox();
	}
}

// Set Circle Angle
void GLC_Circle::setAngle(double AngleRadians)	// Angle in Radians
{
	Q_ASSERT((!qFuzzyCompare(AngleRadians, 0.0)) && (AngleRadians < 2 * PI));
	if (!qFuzzyCompare(AngleRadians - m_Angle, 0.0))
	{	// Angle is changing
			m_Angle= AngleRadians;

			GLC_Geometry::clearWireAndBoundingBox();
	}
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Circle drawing
void GLC_Circle::glDraw(const GLC_RenderProperties& renderProperties)
{
	if (m_WireData.isEmpty())
	{
		createWire();
	}

	m_WireData.glDraw(renderProperties, GL_LINE_STRIP);
}

// Create the wire
void GLC_Circle::createWire()
{
	Q_ASSERT(m_WireData.isEmpty());

	m_Step= static_cast<GLuint>(static_cast<double>(m_Discret) * (m_Angle / (2 * glc::PI)));
	if (m_Step < 2) m_Step= 2;

	// Float vector
	GLfloatVector floatVector;

	// Resize the Vertex vector
	const int size= (m_Step + 1) * 3;
	floatVector.resize(size);
	// Fill Vertex Vector
	const double angleOnStep= m_Angle / static_cast<double>(m_Step);
	for (GLuint i= 0; i <= m_Step; ++i)
	{
		floatVector[(i * 3)]= static_cast<float>(m_Radius * cos(static_cast<double>(i) * angleOnStep));
		floatVector[(i * 3) + 1]= static_cast<float>(m_Radius * sin(static_cast<double>(i) * angleOnStep));
		floatVector[(i * 3) + 2]= 0.0f;
	}
	GLC_Geometry::addVerticeGroup(floatVector);
}
