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
//! \file glc_disc.cpp implementation of the GLC_Disc class.

#include "glc_disc.h"

GLC_Disc::GLC_Disc(double radius, double angle)
: GLC_Mesh()
, m_Radius(radius)
, m_Discret(glc::GLC_POLYDISCRET)
, m_Angle(angle)
, m_Step(0)
{
	createMeshAndWire();
}

GLC_Disc::GLC_Disc(const GLC_Disc& disc)
: GLC_Mesh(disc)
, m_Radius(disc.m_Radius)
, m_Discret(disc.m_Discret)
, m_Angle(disc.m_Angle)
, m_Step(disc.m_Step)
{
	createMeshAndWire();
}

GLC_Disc::~GLC_Disc()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

const GLC_BoundingBox& GLC_Disc::boundingBox()
{
	if (GLC_Mesh::isEmpty())
	{
		createMeshAndWire();
	}
	return GLC_Mesh::boundingBox();
}

GLC_Geometry* GLC_Disc::clone() const
{
	return new GLC_Disc(*this);
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
GLC_Disc& GLC_Disc::operator=(const GLC_Disc& disc)
{
	if (this != &disc)
	{
		// Call the operator of the super class
		GLC_Mesh::operator=(disc);

		m_Radius= disc.m_Radius;
		m_Discret= disc.m_Discret;
		m_Angle= disc.m_Angle;
		m_Step= disc.m_Step;
	}
	return *this;
}

void GLC_Disc::setRadius(double radius)
{
	Q_ASSERT(radius > 0.0);
	m_Radius= radius;

	GLC_Mesh::clearMeshWireAndBoundingBox();
}

void GLC_Disc::setDiscretion(int targetDiscret)
{
	Q_ASSERT(targetDiscret > 0);
	if (targetDiscret != m_Discret)
	{
		m_Discret= targetDiscret;
		if (m_Discret < 6) m_Discret= 6;

		GLC_Mesh::clearMeshWireAndBoundingBox();
	}
}

void GLC_Disc::setAngle(double angle)
{
	Q_ASSERT(angle > 0.0);
	m_Angle= angle;

	GLC_Mesh::clearMeshWireAndBoundingBox();
}
//////////////////////////////////////////////////////////////////////
// Private Opengl functions
//////////////////////////////////////////////////////////////////////
void GLC_Disc::glDraw(const GLC_RenderProperties& renderProperties)
{

	if (GLC_Mesh::isEmpty())
	{
		createMeshAndWire();
	}
	GLC_Mesh::glDraw(renderProperties);
}

// Create the cylinder mesh
void GLC_Disc::createMeshAndWire()
{
	Q_ASSERT(GLC_Mesh::isEmpty());
	Q_ASSERT(m_WireData.isEmpty());

	m_Step= static_cast<GLuint>(static_cast<double>(m_Discret) * (m_Angle / (2 * glc::PI)));
	if (m_Step < 2) m_Step= 2;

	// Create cosinus and sinus array according to the discretion and radius
	const int vertexNumber= m_Step + 1;

	QVector<float> cosArray(vertexNumber);
	QVector<float> sinArray(vertexNumber);

	const double angle= m_Angle / static_cast<double>(m_Step);
	for (int i= 0; i < vertexNumber; ++i)
	{
		const double cosValue= cos(static_cast<double>(i) * angle);
		const double sinValue= sin(static_cast<double>(i) * angle);

		cosArray[i]= static_cast<GLfloat>(m_Radius * cosValue);
		sinArray[i]= static_cast<GLfloat>(m_Radius * sinValue);
	}

	// Mesh Data
	GLfloatVector verticeVector(vertexNumber * 3);
	GLfloatVector normalsVector(vertexNumber * 3);
	GLfloatVector texelVector(vertexNumber * 2);

	// Wire Data
	GLfloatVector wireData(vertexNumber * 3);

	for (int i= 0; i < vertexNumber; ++i)
	{
		verticeVector[3 * i]= cosArray[i];
		verticeVector[3 * i + 1]= sinArray[i];
		verticeVector[3 * i + 2]= 0.0f;

		normalsVector[3 * i]= 0.0f;
		normalsVector[3 * i + 1]= 0.0f;
		normalsVector[3 * i + 2]= 1.0f;

		texelVector[2 * i]= texelVector[i];
		texelVector[2 * i + 1]= 0.0f;

		wireData[3 * i]= cosArray[i];
		wireData[3 * i + 1]= sinArray[i];
		wireData[3 * i + 2]= 0.0f;
	}
	// Center Point
	verticeVector << 0.0f << 0.0f << 0.0f;
	normalsVector << 0.0f << 0.0f << 1.0f;
	texelVector << 0.5f << 0.5f;

	if (!qFuzzyCompare(m_Angle, (2.0 * glc::PI)))
	{
		wireData << 0.0f << 0.0f << 0.0f;
		wireData << wireData[0] << wireData[1] << wireData[2];
	}

	// Add bulk data in to the mesh
	GLC_Mesh::addVertice(verticeVector);
	GLC_Mesh::addNormals(normalsVector);
	GLC_Mesh::addTexels(texelVector);

	// Add polyline to wire data
	GLC_Geometry::addVerticeGroup(wireData);

	// Set the material to use
	GLC_Material* pDiscMaterial;
	if (hasMaterial())
	{
		pDiscMaterial= this->firstMaterial();
	}
	else
	{
		pDiscMaterial= new GLC_Material();
	}

	IndexList discIndex;
	discIndex << vertexNumber;
	for (int i= 0; i < vertexNumber; ++i)
	{
		discIndex << i;
	}

	addTrianglesFan(pDiscMaterial, discIndex);

	finish();
}
