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
//! \file glc_cone.cpp implementation of the GLC_Cone class.

#include "glc_cone.h"

// Class chunk id
quint32 GLC_Cone::m_ChunkId= 0xA709;

GLC_Cone::GLC_Cone(double dRadius, double dLength)
:GLC_Mesh()
, m_Radius(dRadius)
, m_Length(dLength)
, m_Discret(glc::GLC_POLYDISCRET)	// Default discretion
{
	Q_ASSERT((m_Radius > 0.0) && (m_Length > 0.0));
	createMeshAndWire();
}

GLC_Cone::GLC_Cone(const GLC_Cone& sourceCone)
:GLC_Mesh(sourceCone)
, m_Radius(sourceCone.m_Radius)
, m_Length(sourceCone.m_Length)
, m_Discret(sourceCone.m_Discret)
{
	createMeshAndWire();
}

GLC_Cone::~GLC_Cone()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

quint32 GLC_Cone::chunckID()
{
	return m_ChunkId;
}


GLC_Geometry* GLC_Cone::clone() const
{
	return new GLC_Cone(*this);
}


const GLC_BoundingBox& GLC_Cone::boundingBox()
{
	if (GLC_Mesh::isEmpty())
	{
		createMeshAndWire();
	}
	return GLC_Mesh::boundingBox();
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

void GLC_Cone::setLength(double Length)
{
	Q_ASSERT(Length > 0.0);
	m_Length= Length;

	GLC_Mesh::clearMeshWireAndBoundingBox();
}


void GLC_Cone::setRadius(double Radius)
{
	Q_ASSERT(Radius > 0.0);
	m_Radius= Radius;

	GLC_Mesh::clearMeshWireAndBoundingBox();
}


void GLC_Cone::setDiscretion(int TargetDiscret)
{
	Q_ASSERT(TargetDiscret > 0);
	if (TargetDiscret != m_Discret)
	{
		m_Discret= TargetDiscret;
		if (m_Discret < 6) m_Discret= 6;

		GLC_Mesh::clearMeshWireAndBoundingBox();
	}
}

//////////////////////////////////////////////////////////////////////
// Private Opengl functions
//////////////////////////////////////////////////////////////////////

void GLC_Cone::glDraw(const GLC_RenderProperties& renderProperties)
{

	if (GLC_Mesh::isEmpty())
	{
		createMeshAndWire();
	}

	GLC_Mesh::glDraw(renderProperties);
}


void GLC_Cone::createMeshAndWire()
{
	Q_ASSERT(GLC_Mesh::isEmpty());
	Q_ASSERT(m_WireData.isEmpty());

	// Create cosinus and sinus array according to the discretion and radius
	const int vertexNumber= m_Discret + 1;
	// Normals values
	QVector<float> cosNormalArray(vertexNumber);
	QVector<float> sinNormalArray(vertexNumber);

	QVector<float> cosArray(vertexNumber);
	QVector<float> sinArray(vertexNumber);

	const double angle= (2.0 * glc::PI) / static_cast<double>(m_Discret);

	// Normal Z value
	GLC_Vector3d normalVector(1.0, 0.0, 0.0);
	GLC_Matrix4x4 rotation(glc::Y_AXIS, -atan(m_Radius / m_Length));
	normalVector= rotation * normalVector;
	const float normalZ= static_cast<float>(normalVector.z());
	const double factor= normalVector.x(); // Normailsation factor

	for (int i= 0; i < vertexNumber; ++i)
	{
		const double cosValue= cos(static_cast<double>(i) * angle);
		const double sinValue= sin(static_cast<double>(i) * angle);

		cosNormalArray[i]= static_cast<GLfloat>(factor * cosValue);
		sinNormalArray[i]= static_cast<GLfloat>(factor * sinValue);

		cosArray[i]= static_cast<GLfloat>(m_Radius * cosValue);
		sinArray[i]= static_cast<GLfloat>(m_Radius * sinValue);
	}


	// Mesh Data
	GLfloatVector verticeVector;
	GLfloatVector normalsVector;
	GLfloatVector texelVector;

	// Wire Data
	GLfloatVector bottomWireData(vertexNumber * 3);

	const int size= vertexNumber * 3;
	verticeVector.resize(3 * size);
	normalsVector.resize(3 * size);
	texelVector.resize(2 * size);

	for (int i= 0; i < vertexNumber; ++i)
	{
		// Bottom Mesh
		verticeVector[3 * i]= cosArray[i];
		verticeVector[3 * i + 1]= sinArray[i];
		verticeVector[3 * i + 2]= 0.0f;

		normalsVector[3 * i]= cosNormalArray[i];
		normalsVector[3 * i + 1]= sinNormalArray[i];
		normalsVector[3 * i + 2]= normalZ;

		texelVector[2 * i]= static_cast<float>(i) / static_cast<float>(m_Discret);
		texelVector[2 * i + 1]= 0.0f;

		// Bottom Wire
		bottomWireData[3 * i]= cosArray[i];
		bottomWireData[3 * i + 1]= sinArray[i];
		bottomWireData[3 * i + 2]= 0.0f;

		// Top
		verticeVector[3 * i + 3 * vertexNumber]= 0.0f;
		verticeVector[3 * i + 1 + 3 * vertexNumber]= 0.0f;
		verticeVector[3 * i + 2 + 3 * vertexNumber]= static_cast<float>(m_Length);

		normalsVector[3 * i + 3 * vertexNumber]= cosNormalArray[i];
		normalsVector[3 * i + 1 + 3 * vertexNumber]= sinNormalArray[i];
		normalsVector[3 * i + 2 + 3 * vertexNumber]= normalZ;

		texelVector[2 * i + 2 * vertexNumber]= texelVector[i];
		texelVector[2 * i + 1 + 2 * vertexNumber]= 1.0f;

		// Bottom Cap ends
		verticeVector[3 * i + 2 * 3 * vertexNumber]= cosArray[i];
		verticeVector[3 * i + 1 + 2 * 3 * vertexNumber]= sinArray[i];
		verticeVector[3 * i + 2 + 2 * 3 * vertexNumber]= 0.0f;

		normalsVector[3 * i + 2 * 3 * vertexNumber]= 0.0f;
		normalsVector[3 * i + 1 + 2 * 3 * vertexNumber]= 0.0f;
		normalsVector[3 * i + 2 + 2 * 3 * vertexNumber]= -1.0f;

		texelVector[2 * i + 2 * 2 * vertexNumber]= texelVector[i];
		texelVector[2 * i + 1 + 2 * 2 * vertexNumber]= 0.0f;

	}

	// Add bulk data in to the mesh
	GLC_Mesh::addVertice(verticeVector);
	GLC_Mesh::addNormals(normalsVector);
	GLC_Mesh::addTexels(texelVector);

	// Add polyline to wire data
	GLC_Geometry::addVerticeGroup(bottomWireData);

	// Set the material to use
	GLC_Material* pCylinderMaterial;
	if (hasMaterial())
	{
		pCylinderMaterial= this->firstMaterial();
	}
	else
	{
		pCylinderMaterial= new GLC_Material();
	}

	IndexList circumferenceStrips;
	// Create the index
	for (int i= 0; i < vertexNumber; ++i)
	{
		circumferenceStrips.append(i + vertexNumber);
		circumferenceStrips.append(i);
	}
	addTrianglesStrip(pCylinderMaterial, circumferenceStrips);

	{
		IndexList bottomCap;
		IndexList topCap;
		int id1= 0;
		int id2= m_Discret - 1;
		const int size= m_Discret / 2 + (m_Discret % 2);
		for (int i= 0; i < size; ++i)
		{
			bottomCap.append(id1 + 2 * vertexNumber);
			bottomCap.append(id2 + 2 * vertexNumber);

			id1+= 1;
			id2-= 1;
		}
		addTrianglesStrip(pCylinderMaterial, bottomCap);
	}

	finish();
}

