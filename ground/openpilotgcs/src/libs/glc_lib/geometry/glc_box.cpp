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

//! \file glc_box.cpp implementation of the GLC_Box class.

#include "glc_box.h"
#include "../glc_openglexception.h"
#include "../glc_state.h"

//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////

GLC_Box::GLC_Box(double dLx, double dLy, double dlz)
:GLC_Mesh()
, m_dLgX(dLx)
, m_dLgY(dLy)
, m_dLgZ(dlz)
{
	createMeshAndWire();
}
// Copy constructor
GLC_Box::GLC_Box(const GLC_Box& box)
:GLC_Mesh(box)
, m_dLgX(box.m_dLgX)
, m_dLgY(box.m_dLgY)
, m_dLgZ(box.m_dLgZ)
{
	createMeshAndWire();
}
GLC_Box::~GLC_Box()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// return the box bounding box
const GLC_BoundingBox& GLC_Box::boundingBox(void)
{
	if (GLC_Mesh::isEmpty())
	{
		createMeshAndWire();
	}
	return GLC_Mesh::boundingBox();
}

// Return a copy of the current geometry
GLC_Geometry* GLC_Box::clone() const
{
	return new GLC_Box(*this);
}


//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set X length
void GLC_Box::setLgX(double LgX)
{
	Q_ASSERT(LgX > 0);
	m_dLgX= LgX;

	GLC_Mesh::clearMeshWireAndBoundingBox();
}

// Set Y length
void GLC_Box::setLgY(double LgY)
{
	Q_ASSERT(LgY > 0);
	m_dLgY= LgY;

	GLC_Mesh::clearMeshWireAndBoundingBox();
}

// Set Z length
void GLC_Box::setLgZ(double LgZ)
{
	Q_ASSERT(LgZ > 0);
	m_dLgZ= LgZ;

	GLC_Mesh::clearMeshWireAndBoundingBox();
}

//////////////////////////////////////////////////////////////////////
// Private OpenGL functions
//////////////////////////////////////////////////////////////////////

// Box Set Up
void GLC_Box::glDraw(const GLC_RenderProperties& renderProperties)
{
	if (GLC_Mesh::isEmpty())
	{
		createMeshAndWire();
	}

	GLC_Mesh::glDraw(renderProperties);
}
// Create the box mesh
void GLC_Box::createMeshAndWire()
{
	Q_ASSERT(GLC_Mesh::isEmpty());
	createWire();

	const GLfloat lgX= static_cast<const GLfloat>(m_dLgX / 2.0);
	const GLfloat lgY= static_cast<const GLfloat>(m_dLgY / 2.0);
	const GLfloat lgZ= static_cast<const GLfloat>(m_dLgZ / 2.0);

	GLfloatVector verticeVector;
	GLfloatVector normalsVector;
	GLfloatVector texelVector;

	// Face 1
	verticeVector << -lgX; verticeVector << -lgY; verticeVector << lgZ;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << 1.0f;
	texelVector << 0.0f; texelVector << 0.0f;

	verticeVector << lgX; verticeVector << -lgY; verticeVector << lgZ;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << 1.0f;
	texelVector << 1.0f; texelVector << 0.0f;

	verticeVector << lgX; verticeVector << lgY; verticeVector << lgZ;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << 1.0f;
	texelVector << 1.0f; texelVector << 1.0f;

	verticeVector << -lgX; verticeVector << lgY; verticeVector << lgZ;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << 1.0f;
	texelVector << 0.0f; texelVector << 1.0f;

	// Face 2
	verticeVector << lgX; verticeVector << -lgY; verticeVector << lgZ;
	normalsVector << 1.0f; normalsVector << 0.0f; normalsVector << 0.0f;
	texelVector << 0.0f; texelVector << 0.0f;

	verticeVector << lgX; verticeVector << -lgY; verticeVector << -lgZ;
	normalsVector << 1.0f; normalsVector << 0.0f; normalsVector << 0.0f;
	texelVector << 1.0f; texelVector << 0.0f;

	verticeVector << lgX; verticeVector << lgY; verticeVector << -lgZ;
	normalsVector << 1.0f; normalsVector << 0.0f; normalsVector << 0.0f;
	texelVector << 1.0f; texelVector << 1.0f;

	verticeVector << lgX; verticeVector << lgY; verticeVector << lgZ;
	normalsVector << 1.0f; normalsVector << 0.0f; normalsVector << 0.0f;
	texelVector << 0.0f; texelVector << 1.0f;

	// Face 3
	verticeVector << -lgX; verticeVector << -lgY; verticeVector << -lgZ;
	normalsVector << -1.0f; normalsVector << 0.0f; normalsVector << 0.0f;
	texelVector << 0.0f; texelVector << 0.0f;

	verticeVector << -lgX; verticeVector << -lgY; verticeVector << lgZ;
	normalsVector << -1.0f; normalsVector << 0.0f; normalsVector << 0.0f;
	texelVector << 1.0f; texelVector << 0.0f;

	verticeVector << -lgX; verticeVector << lgY; verticeVector << lgZ;
	normalsVector << -1.0f; normalsVector << 0.0f; normalsVector << 0.0f;
	texelVector << 1.0f; texelVector << 1.0f;

	verticeVector << -lgX; verticeVector << lgY; verticeVector << -lgZ;
	normalsVector << -1.0f; normalsVector << 0.0f; normalsVector << 0.0f;
	texelVector << 0.0f; texelVector << 1.0f;

	// Face 4
	verticeVector << lgX; verticeVector << -lgY; verticeVector << -lgZ;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << -1.0f;
	texelVector << 0.0f; texelVector << 0.0f;

	verticeVector << -lgX; verticeVector << -lgY; verticeVector << -lgZ;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << -1.0f;
	texelVector << 1.0f; texelVector << 0.0f;

	verticeVector << -lgX; verticeVector << lgY; verticeVector << -lgZ;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << -1.0f;
	texelVector << 1.0f; texelVector << 1.0f;

	verticeVector << lgX; verticeVector << lgY; verticeVector << -lgZ;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << -1.0f;
	texelVector << 0.0f; texelVector << 1.0f;

	// Face 5
	verticeVector << -lgX; verticeVector << lgY; verticeVector << lgZ;
	normalsVector << 0.0f; normalsVector << 1.0f; normalsVector << 0.0f;
	texelVector << 0.0f; texelVector << 0.0f;

	verticeVector << lgX; verticeVector << lgY; verticeVector << lgZ;
	normalsVector << 0.0f; normalsVector << 1.0f; normalsVector << 0.0f;
	texelVector << 1.0f; texelVector << 0.0f;

	verticeVector << lgX; verticeVector << lgY; verticeVector << -lgZ;
	normalsVector << 0.0f; normalsVector << 1.0f; normalsVector << 0.0f;
	texelVector << 1.0f; texelVector << 1.0f;

	verticeVector << -lgX; verticeVector << lgY; verticeVector << -lgZ;
	normalsVector << 0.0f; normalsVector << 1.0f; normalsVector << 0.0f;
	texelVector << 0.0f; texelVector << 1.0f;

	// Face 6
	verticeVector << -lgX; verticeVector << -lgY; verticeVector << -lgZ;
	normalsVector << 0.0f; normalsVector << -1.0f; normalsVector << 0.0f;
	texelVector << 0.0f; texelVector << 0.0f;

	verticeVector << lgX; verticeVector << -lgY; verticeVector << -lgZ;
	normalsVector << 0.0f; normalsVector << -1.0f; normalsVector << 0.0f;
	texelVector << 1.0f; texelVector << 0.0f;

	verticeVector << lgX; verticeVector << -lgY; verticeVector << lgZ;
	normalsVector << 0.0f; normalsVector << -1.0f; normalsVector << 0.0f;
	texelVector << 1.0f; texelVector << 1.0f;

	verticeVector << -lgX; verticeVector << -lgY; verticeVector << lgZ;
	normalsVector << 0.0f; normalsVector << -1.0f; normalsVector << 0.0f;
	texelVector << 0.0f; texelVector << 1.0f;

	// Add bulk data in to the mesh
	GLC_Mesh::addVertice(verticeVector);
	GLC_Mesh::addNormals(normalsVector);
	GLC_Mesh::addTexels(texelVector);

	// Set the material to use
	GLC_Material* pMaterial;
	if (hasMaterial())
	{
		pMaterial= this->firstMaterial();
	}
	else
	{
		pMaterial= new GLC_Material();
	}

	IndexList index;
	// Face 1
	index << 0 << 1 << 3 << 2;
	GLC_Mesh::addTrianglesStrip(pMaterial, index);
	index.clear();
	// Face 2
	index << 4 << 5 << 7 << 6;
	GLC_Mesh::addTrianglesStrip(pMaterial, index);
	index.clear();
	// Face 3
	index << 8 << 9 << 11 << 10;
	GLC_Mesh::addTrianglesStrip(pMaterial, index);
	index.clear();
	// Face 4
	index << 12 << 13 << 15 << 14;
	GLC_Mesh::addTrianglesStrip(pMaterial, index);
	index.clear();
	// Face 5
	index << 16 << 17 << 19 << 18;
	GLC_Mesh::addTrianglesStrip(pMaterial, index);
	index.clear();
	// Face 6
	index << 20 << 21 << 23 << 22;
	GLC_Mesh::addTrianglesStrip(pMaterial, index);
	index.clear();

	GLC_Mesh::finish();
}

// Create the wire of the mesh
void GLC_Box::createWire()
{
	Q_ASSERT(m_WireData.isEmpty());

	const GLfloat lgX= static_cast<const GLfloat>(m_dLgX / 2.0);
	const GLfloat lgY= static_cast<const GLfloat>(m_dLgY / 2.0);
	const GLfloat lgZ= static_cast<const GLfloat>(m_dLgZ / 2.0);

	// Float vector
	GLfloatVector floatVector;
	floatVector << lgX << lgY << lgZ;
	floatVector << lgX << lgY << -lgZ;
	floatVector << -lgX << lgY << -lgZ;
	floatVector << -lgX << lgY << lgZ;
	floatVector << lgX << lgY << lgZ;
	GLC_Geometry::addVerticeGroup(floatVector);
	floatVector.clear();

	floatVector << lgX << -lgY << lgZ;
	floatVector << lgX << -lgY << -lgZ;
	floatVector << -lgX << -lgY << -lgZ;
	floatVector << -lgX << -lgY << lgZ;
	floatVector << lgX << -lgY << lgZ;
	GLC_Geometry::addVerticeGroup(floatVector);
	floatVector.clear();

	floatVector << lgX << lgY << lgZ;
	floatVector << lgX << -lgY << lgZ;
	GLC_Geometry::addVerticeGroup(floatVector);
	floatVector.clear();

	floatVector << lgX << lgY << -lgZ;
	floatVector << lgX << -lgY << -lgZ;
	GLC_Geometry::addVerticeGroup(floatVector);
	floatVector.clear();

	floatVector << -lgX << lgY << -lgZ;
	floatVector << -lgX << -lgY << -lgZ;
	GLC_Geometry::addVerticeGroup(floatVector);
	floatVector.clear();

	floatVector << -lgX << lgY << lgZ;
	floatVector << -lgX << -lgY << lgZ;
	GLC_Geometry::addVerticeGroup(floatVector);
	floatVector.clear();
}

