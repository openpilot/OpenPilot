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

//! \file glc_rectangle.cpp Implementation for the GLC_Rectangle class.

#include "glc_rectangle.h"
#include "../glc_state.h"

GLC_Rectangle::GLC_Rectangle()
:GLC_Mesh()
, m_L1(1.0)
, m_L2(1.0)
{

}

GLC_Rectangle::GLC_Rectangle(double l1, double l2)
:GLC_Mesh()
, m_L1(l1)
, m_L2(l2)
{

}

GLC_Rectangle::GLC_Rectangle(const GLC_Rectangle& rect)
:GLC_Mesh(rect)
, m_L1(rect.m_L1)
, m_L2(rect.m_L2)
{

}


GLC_Rectangle::~GLC_Rectangle()
{

}
//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

GLC_Geometry* GLC_Rectangle::clone() const
{
	return new GLC_Rectangle(*this);
}

// return the rectangle bounding box
const GLC_BoundingBox& GLC_Rectangle::boundingBox()
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

GLC_Rectangle& GLC_Rectangle::setRectangle(double l1, double l2)
{
	m_L1= l1;
	m_L2= l2;

	GLC_Mesh::clearMeshWireAndBoundingBox();

	return *this;
}

void GLC_Rectangle::setLength1(double value)
{
	m_L1= value;

	GLC_Mesh::clearMeshWireAndBoundingBox();
}

void GLC_Rectangle::setLength2(double value)
{
	m_L2= value;

	GLC_Mesh::clearMeshWireAndBoundingBox();
}

//////////////////////////////////////////////////////////////////////
// Private OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_Rectangle::glDraw(const GLC_RenderProperties& renderProperties)
{
	if (GLC_Mesh::isEmpty())
	{
		createMeshAndWire();
	}

	GLC_Mesh::glDraw(renderProperties);
}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

void GLC_Rectangle::createMeshAndWire()
{
	Q_ASSERT(GLC_Mesh::isEmpty());

	const GLfloat lgX= static_cast<const GLfloat>(m_L1 / 2.0);
	const GLfloat lgY= static_cast<const GLfloat>(m_L2 / 2.0);

	GLfloatVector verticeVector;
	GLfloatVector normalsVector;
	GLfloatVector texelVector;

	// Wire data
	GLfloatVector wireData;

	// the unique face of this rectangle
	verticeVector << -lgX; verticeVector << -lgY; verticeVector << 0.0f;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << 1.0f;
	texelVector << 0.0f; texelVector << 0.0f;

	verticeVector << lgX; verticeVector << -lgY; verticeVector << 0.0f;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << 1.0f;
	texelVector << 1.0f; texelVector << 0.0f;

	verticeVector << lgX; verticeVector << lgY; verticeVector << 0.0f;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << 1.0f;
	texelVector << 1.0f; texelVector << 1.0f;

	verticeVector << -lgX; verticeVector << lgY; verticeVector << 0.0f;
	normalsVector << 0.0f; normalsVector << 0.0f; normalsVector << 1.0f;
	texelVector << 0.0f; texelVector << 1.0f;

	// Add bulk data in to the mesh
	GLC_Mesh::addVertice(verticeVector);
	GLC_Mesh::addNormals(normalsVector);
	GLC_Mesh::addTexels(texelVector);

	// Add the first point of the rectangle for wire
	verticeVector << -lgX; verticeVector << -lgY; verticeVector << 0.0f;
	GLC_Geometry::addVerticeGroup(verticeVector);

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

	GLC_Mesh::finish();
}


