/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 1.2.0, packaged on September 2009.

 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 *****************************************************************************/

//! \file glc_rectangle.cpp Implementation for the GLC_Rectangle class.

#include "glc_rectangle.h"
#include "../glc_state.h"

// Default constructor
GLC_Rectangle::GLC_Rectangle()
:GLC_VboGeom("Rectangle", false)
, m_PrimitiveGroup(0)
, m_IsSelected(false)
, m_ExtendedGeomEngine()
, m_Normal(0.0, 1.0, 0.0)
, m_L1(1.0)
, m_L2(1.0)
{
	createRectangleMesh();
}

// Complete constructor
GLC_Rectangle::GLC_Rectangle(const GLC_Vector4d& normal, double l1, double l2)
:GLC_VboGeom("Rectangle", false)
, m_PrimitiveGroup(0)
, m_IsSelected(false)
, m_ExtendedGeomEngine()
, m_Normal(normal)
, m_L1(l1)
, m_L2(l2)
{
	createRectangleMesh();
}

// Copy constructor
GLC_Rectangle::GLC_Rectangle(const GLC_Rectangle& rect)
:GLC_VboGeom(rect)
, m_PrimitiveGroup(rect.m_PrimitiveGroup, 0)
, m_IsSelected(rect.m_IsSelected)
, m_ExtendedGeomEngine(rect.m_ExtendedGeomEngine)
, m_Normal(rect.m_Normal)
, m_L1(rect.m_L1)
, m_L2(rect.m_L2)
{
	createRectangleMesh();
}


GLC_Rectangle::~GLC_Rectangle()
{

}
//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// clone the rectangle
GLC_VboGeom* GLC_Rectangle::clone() const
{
	return new GLC_Rectangle(*this);
}

// return the rectangle bounding box
GLC_BoundingBox& GLC_Rectangle::boundingBox()
{
	if (NULL == m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox();

		if (m_ExtendedGeomEngine.positionVectorHandle()->isEmpty())
		{
			qDebug() << "GLC_Rectangle::getBoundingBox empty m_Positions";
			Q_ASSERT(false);
		}
		else
		{
			qDebug() << "Create Bounding box";
			GLfloatVector* pVertexVector= m_ExtendedGeomEngine.positionVectorHandle();
			const int max= pVertexVector->size();
			for (int i= 0; i < max; i= i + 3)
			{
				GLC_Vector3d vector((*pVertexVector)[i], (*pVertexVector)[i + 1], -2.0 * glc::EPSILON);
				m_pBoundingBox->combine(vector);
			}
			for (int i= 0; i < max; i= i + 3)
			{
				GLC_Vector3d vector((*pVertexVector)[i], (*pVertexVector)[i + 1], 2.0 * glc::EPSILON);
				m_pBoundingBox->combine(vector);
			}

		}

	}
	return *m_pBoundingBox;
}

// Get number of faces
unsigned int GLC_Rectangle::numberOfFaces() const
{
	return 2;
}

// Get number of vertex
unsigned int GLC_Rectangle::numberOfVertex() const
{
	return 4;
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set the rectangle
GLC_Rectangle& GLC_Rectangle::setRectangle(const GLC_Vector4d& normal, double l1, double l2)
{
	m_Normal= normal;
	m_L1= l1;
	m_L2= l2;

	// Invalid the geometry
	m_GeometryIsValid = false;

	return *this;
}

// Copy vertex list in a vector list for Vertex Array Use
void GLC_Rectangle::finished()
{
	qDebug() << "GLC_Rectangle::finished()";
	if (GLC_State::vboUsed())
	{
		finishVbo();
	}
	else
	{
		finishNonVbo();
	}
}

//////////////////////////////////////////////////////////////////////
// Private OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Virtual interface for OpenGL Geometry set up.
void GLC_Rectangle::glDraw(bool)
{
	const bool vboIsUsed= GLC_State::vboUsed();

	if (not m_GeometryIsValid)
	{
		createRectangleMesh();
		if (vboIsUsed)
		{
			m_ExtendedGeomEngine.createVBOs();
			createVbos();
		}
	}
	if (vboIsUsed)
	{
		// Activate Vertices VBO
		m_ExtendedGeomEngine.useVBO(true, GLC_ExtendedGeomEngine::GLC_Vertex);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		glEnableClientState(GL_VERTEX_ARRAY);

		// Activate Normals VBO
		m_ExtendedGeomEngine.useVBO(true, GLC_ExtendedGeomEngine::GLC_Normal);
		glNormalPointer(GL_FLOAT, 0, 0);
		glEnableClientState(GL_NORMAL_ARRAY);

		// Activate texel VBO if needed
		m_ExtendedGeomEngine.useVBO(true, GLC_ExtendedGeomEngine::GLC_Texel);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		m_ExtendedGeomEngine.useIBO(true);
	}
	else
	{
		// Use Vertex Array
		glVertexPointer(3, GL_FLOAT, 0, m_ExtendedGeomEngine.positionVectorHandle()->data());
		glEnableClientState(GL_VERTEX_ARRAY);

		glNormalPointer(GL_FLOAT, 0, m_ExtendedGeomEngine.normalVectorHandle()->data());
		glEnableClientState(GL_NORMAL_ARRAY);

		glTexCoordPointer(2, GL_FLOAT, 0, m_ExtendedGeomEngine.texelVectorHandle()->data());
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	if (vboIsUsed)
	{

		// Draw Triangles strip
		const GLsizei stripsCount= static_cast<GLsizei>(m_PrimitiveGroup.stripsOffset().size());
		for (GLint i= 0; i < stripsCount; ++i)
		{
			glDrawElements(GL_TRIANGLE_STRIP, m_PrimitiveGroup.stripsSizes().at(i), GL_UNSIGNED_INT, m_PrimitiveGroup.stripsOffset().at(i));
		}

	}
	else
	{
		const GLsizei stripsCount= static_cast<GLsizei>(m_PrimitiveGroup.stripsOffseti().size());
		for (GLint i= 0; i < stripsCount; ++i)
		{
			GLvoid* pOffset= &m_ExtendedGeomEngine.indexVectorHandle()->data()[m_PrimitiveGroup.stripsOffseti().at(i)];
			glDrawElements(GL_TRIANGLE_STRIP, m_PrimitiveGroup.stripsSizes().at(i), GL_UNSIGNED_INT, pOffset);
		}

	}

	if (vboIsUsed)
	{
		m_ExtendedGeomEngine.useIBO(false);
		m_ExtendedGeomEngine.useVBO(false, GLC_ExtendedGeomEngine::GLC_Normal);
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
// Create VBO and IBO
void GLC_Rectangle::createVbos()
{
	qDebug() << "GLC_Rectangle::createVbos";
	// Create VBO of vertices
	{
		m_ExtendedGeomEngine.useVBO(true, GLC_ExtendedGeomEngine::GLC_Vertex);

		GLfloatVector* pPositionVector= m_ExtendedGeomEngine.positionVectorHandle();
		const GLsizei dataNbr= static_cast<GLsizei>(pPositionVector->size());
		const GLsizeiptr dataSize= dataNbr * sizeof(GLfloat);
		glBufferData(GL_ARRAY_BUFFER, dataSize, pPositionVector->data(), GL_STATIC_DRAW);
	}

	// Create VBO of normals
	{
		m_ExtendedGeomEngine.useVBO(true, GLC_ExtendedGeomEngine::GLC_Normal);

		GLfloatVector* pNormalVector= m_ExtendedGeomEngine.normalVectorHandle();
		const GLsizei dataNbr= static_cast<GLsizei>(pNormalVector->size());
		const GLsizeiptr dataSize= dataNbr * sizeof(GLfloat);
		glBufferData(GL_ARRAY_BUFFER, dataSize, pNormalVector->data(), GL_STATIC_DRAW);
	}

	// Create VBO of texel if needed
	if (m_ExtendedGeomEngine.useVBO(true, GLC_ExtendedGeomEngine::GLC_Texel))
	{
		GLfloatVector* pTexelVector= m_ExtendedGeomEngine.texelVectorHandle();
		const GLsizei dataNbr= static_cast<GLsizei>(pTexelVector->size());
		const GLsizeiptr dataSize= dataNbr * sizeof(GLfloat);
		glBufferData(GL_ARRAY_BUFFER, dataSize, pTexelVector->data(), GL_STATIC_DRAW);
	}

	//Create IBO
	if (not m_ExtendedGeomEngine.indexVectorHandle()->isEmpty())
	{
		QVector<GLuint>* pIndexVector= m_ExtendedGeomEngine.indexVectorHandle();
		m_ExtendedGeomEngine.useIBO(true);
		const GLsizei indexNbr= static_cast<GLsizei>(pIndexVector->size());
		const GLsizeiptr indexSize = indexNbr * sizeof(GLuint);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, pIndexVector->data(), GL_STATIC_DRAW);
	}

	//Clear engine
	m_ExtendedGeomEngine.finished();

}

// Finish VBO mesh
void GLC_Rectangle::finishVbo()
{

	m_PrimitiveGroup.setBaseTrianglesStripOffset(BUFFER_OFFSET(m_ExtendedGeomEngine.indexVectorSize() * sizeof(GLvoid*)));
	(*m_ExtendedGeomEngine.indexVectorHandle())+= m_PrimitiveGroup.stripsIndex().toVector();

	m_PrimitiveGroup.finished();
}

// Finish non Vbo mesh
void GLC_Rectangle::finishNonVbo()
{
	m_PrimitiveGroup.setBaseTrianglesStripOffseti(m_ExtendedGeomEngine.indexVectorSize());
	(*m_ExtendedGeomEngine.indexVectorHandle())+= m_PrimitiveGroup.stripsIndex().toVector();

	m_PrimitiveGroup.finished();
}

// Create rectangle mesh
void GLC_Rectangle::createRectangleMesh()
{
	m_PrimitiveGroup.clear();
	m_ExtendedGeomEngine.clear();
	m_ExtendedGeomEngine.appendLod();

	const double halfL1= m_L1 / 2.0;
	const double halfL2= m_L2 / 2.0;

	// points vector
	GLC_Point4d p0(-halfL1, halfL2, 0.0);
	GLC_Point4d p1(halfL1, halfL2, 0.0);
	GLC_Point4d p2(-halfL1, -halfL2, 0.0);
	GLC_Point4d p3(halfL1, -halfL2, 0.0);

	QVector<GLC_Point4d> points;
	points << p0 << p1 << p2 << p3;

	// Normals vector
	GLC_Vector4d normal(glc::Y_AXIS);
	QVector<GLC_Vector4d> normals;
	for (int i= 0; i < 4; ++i)
	{
		normals << normal;
	}

	// Rotate the rectangle
	if ((m_Normal != glc::Y_AXIS) and (m_Normal != -glc::Y_AXIS))
	{
		const GLC_Vector4d axis= normal ^ glc::Y_AXIS;
		const double angle= normal.getAngleWithVect(glc::Y_AXIS);
		const GLC_Matrix4x4 matRot(axis, angle);
		for (int i= 0; i < 4; ++i)
		{
			points[i]= matRot * points.at(i);
			normals[i]= matRot * normals.at(i);
		}
	}
	else if (m_Normal == -glc::Y_AXIS)
	{
		const GLC_Matrix4x4 matRot(glc::X_AXIS, glc::PI);
		for (int i= 0; i < 4; ++i)
		{
			points[i]= matRot * points.at(i);
			normals[i]= matRot * normals.at(i);
		}
	}

	GLfloatVector verticeFloat;
	GLfloatVector normalsFloat;
	GLfloatVector texels;
	for (int i= 0; i < 4; ++i)
	{
		verticeFloat << points[i].toFloat3dQVector();
		normalsFloat << normals[i].toFloat3dQVector();
	}
	texels << 0.0 << 1.0;
	texels << 1.0 << 1.0;
	texels << 0.0 << 0.0;
	texels << 1.0 << 0.0;

	*(m_ExtendedGeomEngine.texelVectorHandle())+= texels;
	*(m_ExtendedGeomEngine.positionVectorHandle())+= verticeFloat;
	*(m_ExtendedGeomEngine.normalVectorHandle())+= normalsFloat;

	IndexList index;
	index << 0 << 2 << 1 << 3;
	m_PrimitiveGroup.addTrianglesStrip(index);
	finished();
}


