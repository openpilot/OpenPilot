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

//! \file glc_simplegeomengine.cpp Implementation for the GLC_SimpleGeomEngine class.

#include "glc_simplegeomengine.h"
#include "../glc_state.h"

// Default constructor
GLC_SimpleGeomEngine::GLC_SimpleGeomEngine()
: GLC_GeomEngine()
, m_VertexVector()
, m_IndexVector()
, m_IboId(0)
, m_NumberOfVertex(0)
{


}
// Copy constructor
GLC_SimpleGeomEngine::GLC_SimpleGeomEngine(const GLC_SimpleGeomEngine& engine)
: GLC_GeomEngine(engine)
, m_VertexVector(engine.vertexVector())
, m_IndexVector(engine.indexVector())
, m_IboId(0)
, m_NumberOfVertex(engine.m_NumberOfVertex)
{

}

GLC_SimpleGeomEngine::~GLC_SimpleGeomEngine()
{
	if (GLC_State::vboUsed())
	{
		// IBO
		if (0 != m_IboId)
			glDeleteBuffers(1, &m_IboId);
	}
}


//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Return the Vertex Vector
VertexVector GLC_SimpleGeomEngine::vertexVector() const
{

	if (0 != m_VboId)
	{
		// VBO created get data from VBO
		const int sizeOfVbo= m_NumberOfVertex;
		const GLsizeiptr dataSize= sizeOfVbo * sizeof(GLC_Vertex);
		VertexVector vertexVector(sizeOfVbo);

		glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
		GLvoid* pVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		memcpy(vertexVector.data(), pVbo, dataSize);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return vertexVector;
	}
	else
	{
		return m_VertexVector;
	}
}

// Return the Index Vector
QVector<GLuint> GLC_SimpleGeomEngine::indexVector() const
{
	if (0 != m_IboId)
	{
		// IBO created get data from IBO
		const int sizeOfVbo= m_NumberOfVertex;
		const GLsizeiptr indexSize = sizeOfVbo * sizeof(GLuint);
		QVector<GLuint> indexVector(sizeOfVbo);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IboId);
		GLvoid* pIbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
		memcpy(indexVector.data(), pIbo, indexSize);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		return indexVector;
	}
	else
	{
		return m_IndexVector;
	}

}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Vbo creation
void GLC_SimpleGeomEngine::createVBOs()
{
	if (0 == m_VboId)
	{
		glGenBuffers(1, &m_VboId);
		glGenBuffers(1, &m_IboId);
	}
}

// Vbo Usage
void GLC_SimpleGeomEngine::useVBOs(bool use)
{
	if (use)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IboId);
	}
	else
	{
		// Unbind VBOs
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}
