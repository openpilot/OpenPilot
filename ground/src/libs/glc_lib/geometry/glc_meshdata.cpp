/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 2.0.0, packaged on July 2010.

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

//! \file glc_meshdata.cpp Implementation for the GLC_MeshData class.

#include "glc_meshdata.h"
#include "../glc_state.h"

// Class chunk id
quint32 GLC_MeshData::m_ChunkId= 0xA704;

// Default constructor
GLC_MeshData::GLC_MeshData()
: m_VboId(0)
, m_Positions()
, m_Normals()
, m_Texels()
, m_Colors()
, m_NormalVboId(0)
, m_TexelVboId(0)
, m_ColorVboId(0)
, m_LodList()
, m_PositionSize(0)
, m_TexelsSize(0)
, m_ColorSize(0)
{

}

// Copy constructor
GLC_MeshData::GLC_MeshData(const GLC_MeshData& meshData)
: m_VboId(0)
, m_Positions(meshData.positionVector())
, m_Normals(meshData.normalVector())
, m_Texels(meshData.texelVector())
, m_Colors(meshData.colorVector())
, m_NormalVboId(0)
, m_TexelVboId(0)
, m_ColorVboId(0)
, m_LodList()
, m_PositionSize(meshData.m_PositionSize)
, m_TexelsSize(meshData.m_TexelsSize)
, m_ColorSize(meshData.m_ColorSize)
{
	// Copy meshData LOD list
	const int size= meshData.m_LodList.size();
	for (int i= 0; i < size; ++i)
	{
		m_LodList.append(new GLC_Lod(*meshData.m_LodList.at(i)));
	}
}

// Overload "=" operator
GLC_MeshData& GLC_MeshData::operator=(const GLC_MeshData& meshData)
{
	if (this != &meshData)
	{
		// Clear the content of the mesh Data
		clear();

		// Copy mesh Data members
		m_Positions= meshData.positionVector();
		m_Normals= meshData.normalVector();
		m_Texels= meshData.texelVector();
		m_Colors= meshData.colorVector();
		m_PositionSize= meshData.m_PositionSize;
		m_TexelsSize= meshData.m_TexelsSize;
		m_ColorSize= meshData.m_ColorSize;

		// Copy meshData LOD list
		const int size= meshData.m_LodList.size();
		for (int i= 0; i < size; ++i)
		{
			m_LodList.append(new GLC_Lod(*meshData.m_LodList.at(i)));
		}
	}
	return *this;
}

GLC_MeshData::~GLC_MeshData()
{
	clear();
}
//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////
// Return the class Chunk ID
quint32 GLC_MeshData::chunckID()
{
	return m_ChunkId;
}

// Return the Position Vector
GLfloatVector GLC_MeshData::positionVector() const
{
	if (0 != m_VboId)
	{
		// VBO created get data from VBO
		const int sizeOfVbo= m_PositionSize;
		const GLsizeiptr dataSize= sizeOfVbo * sizeof(float);
		GLfloatVector positionVector(sizeOfVbo);

		glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
		GLvoid* pVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		memcpy(positionVector.data(), pVbo, dataSize);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return positionVector;
	}
	else
	{
		return m_Positions;
	}
}

// Return the normal Vector
GLfloatVector GLC_MeshData::normalVector() const
{
	if (0 != m_NormalVboId)
	{
		// VBO created get data from VBO
		const int sizeOfVbo= m_PositionSize;
		const GLsizeiptr dataSize= sizeOfVbo * sizeof(GLfloat);
		GLfloatVector normalVector(sizeOfVbo);

		glBindBuffer(GL_ARRAY_BUFFER, m_NormalVboId);
		GLvoid* pVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		memcpy(normalVector.data(), pVbo, dataSize);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return normalVector;
	}
	else
	{
		return m_Normals;
	}
}

// Return the texel Vector
GLfloatVector GLC_MeshData::texelVector() const
{
	if (0 != m_TexelVboId)
	{
		// VBO created get data from VBO
		const int sizeOfVbo= m_TexelsSize;
		const GLsizeiptr dataSize= sizeOfVbo * sizeof(GLfloat);
		GLfloatVector texelVector(sizeOfVbo);

		glBindBuffer(GL_ARRAY_BUFFER, m_TexelVboId);
		GLvoid* pVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		memcpy(texelVector.data(), pVbo, dataSize);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return texelVector;
	}
	else
	{
		return m_Texels;
	}
}

// Return the color Vector
GLfloatVector GLC_MeshData::colorVector() const
{
	if (0 != m_ColorVboId)
	{
		// VBO created get data from VBO
		const int sizeOfVbo= m_ColorSize;
		const GLsizeiptr dataSize= sizeOfVbo * sizeof(GLfloat);
		GLfloatVector normalVector(sizeOfVbo);

		glBindBuffer(GL_ARRAY_BUFFER, m_ColorVboId);
		GLvoid* pVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		memcpy(normalVector.data(), pVbo, dataSize);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return normalVector;
	}
	else
	{
		return m_Colors;
	}
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// The mesh wich use the data is finished
void GLC_MeshData::finishVbo()
{
	m_PositionSize= m_Positions.size();
	m_Positions.clear();
	m_Normals.clear();
	m_TexelsSize= m_Texels.size();
	m_Texels.clear();
	m_ColorSize= m_Colors.size();
	m_Colors.clear();

	// Finish the LOD
	const int size= m_LodList.size();
	for (int i= 0; i < size; ++i)
	{
		m_LodList[i]->finishVbo();
	}
}

// If the there is more than 2 LOD Swap the first and last
void GLC_MeshData::finishLod()
{
	// PLace the master LOD at the beginning of the list
	const int size= m_LodList.size();
	if (size > 1)
	{
		GLC_Lod* PMasterLod= m_LodList.at(size - 1);
		m_LodList.removeAt(size - 1);
		m_LodList.prepend(PMasterLod);
	}
}

// Clear the content of the meshData and makes it empty
void GLC_MeshData::clear()
{
	m_Positions.clear();
	m_Normals.clear();
	m_Texels.clear();
	m_Colors.clear();
	m_PositionSize= 0;
	m_TexelsSize= 0;
	m_ColorSize= 0;

	// Delete Main Vbo ID
	if (0 != m_VboId)
	{
		glDeleteBuffers(1, &m_VboId);
		m_VboId= 0;
	}

	// Delete Normal VBO
	if (0 != m_NormalVboId)
	{
		glDeleteBuffers(1, &m_NormalVboId);
		m_NormalVboId= 0;
	}

	// Delete Texel VBO
	if (0 != m_TexelVboId)
	{
		glDeleteBuffers(1, &m_TexelVboId);
		m_TexelVboId= 0;
	}
	// Delete color index
	if (0 != m_ColorVboId)
	{
		glDeleteBuffers(1, &m_ColorVboId);
		m_ColorVboId= 0;
	}

	const int size= m_LodList.size();
	for (int i= 0; i < size; ++i)
	{
		delete m_LodList.at(i);
	}
	m_LodList.clear();
}

void GLC_MeshData::copyVboToClientSide()
{

	if ((0 != m_VboId) && m_Positions.isEmpty())
	{
		Q_ASSERT(0 != m_NormalVboId);
		m_Positions= positionVector();
		m_Normals= normalVector();
		if (0 != m_TexelVboId)
		{
			m_Texels= texelVector();
		}
		if (0 != m_ColorVboId)
		{
			m_Colors= colorVector();
		}
	}
}

void GLC_MeshData::releaseVboClientSide(bool update)
{
	if ((0 != m_VboId) && !m_Positions.isEmpty())
	{
		if (update)
		{
			fillVbo(GLC_MeshData::GLC_Vertex);
			fillVbo(GLC_MeshData::GLC_Normal);
			fillVbo(GLC_MeshData::GLC_Texel);
			fillVbo(GLC_MeshData::GLC_Color);
			useVBO(false, GLC_MeshData::GLC_Color);
		}
		m_PositionSize= m_Positions.size();
		m_Positions.clear();
		m_Normals.clear();
		m_TexelsSize= m_Texels.size();
		m_Texels.clear();
		m_ColorSize= m_Colors.size();
		m_Colors.clear();
	}
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////
// Vbo creation
void GLC_MeshData::createVBOs()
{
	// Create position VBO
	if (0 == m_VboId)
	{
		glGenBuffers(1, &m_VboId);
		glGenBuffers(1, &m_NormalVboId);

		// Create Texel VBO
		if (0 == m_TexelVboId && !m_Texels.isEmpty())
		{
			glGenBuffers(1, &m_TexelVboId);
		}

		// Create Color VBO
		if (0 == m_ColorVboId && !m_Colors.isEmpty())
		{
			glGenBuffers(1, &m_ColorVboId);
		}

		const int size= m_LodList.size();
		for (int i= 0; i < size; ++i)
		{
			m_LodList.at(i)->createIBO();
		}
	}
}

// Ibo Usage
bool GLC_MeshData::useVBO(bool use, GLC_MeshData::VboType type) const
{
	bool result= true;
	if (use)
	{
		// Chose the right VBO
		if (type == GLC_MeshData::GLC_Vertex)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
		}
		else if (type == GLC_MeshData::GLC_Normal)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_NormalVboId);
		}
		else if ((type == GLC_MeshData::GLC_Texel) && (0 != m_TexelVboId))
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_TexelVboId);
		}
		else if ((type == GLC_MeshData::GLC_Color) && (0 != m_ColorVboId))
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_ColorVboId);
		}

		else result= false;
	}
	else
	{
		// Unbind VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	return result;
}

void GLC_MeshData::fillVbo(GLC_MeshData::VboType type)
{
	// Chose the right VBO
	if (type == GLC_MeshData::GLC_Vertex)
	{
		useVBO(true, type);
		const GLsizei dataNbr= static_cast<GLsizei>(m_Positions.size());
		const GLsizeiptr dataSize= dataNbr * sizeof(GLfloat);
		glBufferData(GL_ARRAY_BUFFER, dataSize, m_Positions.data(), GL_STATIC_DRAW);
	}
	else if (type == GLC_MeshData::GLC_Normal)
	{
		useVBO(true, type);
		const GLsizei dataNbr= static_cast<GLsizei>(m_Normals.size());
		const GLsizeiptr dataSize= dataNbr * sizeof(GLfloat);
		glBufferData(GL_ARRAY_BUFFER, dataSize, m_Normals.data(), GL_STATIC_DRAW);
	}
	else if ((type == GLC_MeshData::GLC_Texel) && (0 != m_TexelVboId))
	{
		useVBO(true, type);
		const GLsizei dataNbr= static_cast<GLsizei>(m_Texels.size());
		const GLsizeiptr dataSize= dataNbr * sizeof(GLfloat);
		glBufferData(GL_ARRAY_BUFFER, dataSize, m_Texels.data(), GL_STATIC_DRAW);
	}
	else if ((type == GLC_MeshData::GLC_Color) && (0 != m_ColorVboId))
	{
		useVBO(true, type);
		const GLsizei dataNbr= static_cast<GLsizei>(m_Colors.size());
		const GLsizeiptr dataSize= dataNbr * sizeof(GLfloat);
		glBufferData(GL_ARRAY_BUFFER, dataSize, m_Colors.data(), GL_STATIC_DRAW);
	}
}
// Non Member methods
// Non-member stream operator
QDataStream &operator<<(QDataStream &stream, const GLC_MeshData &meshData)
{
	quint32 chunckId= GLC_MeshData::m_ChunkId;
	stream << chunckId;

	stream << meshData.positionVector();
	stream << meshData.normalVector();
	stream << meshData.texelVector();
	stream << meshData.colorVector();

	// List of lod serialisation
	const int lodCount= meshData.m_LodList.size();
	QList<GLC_Lod> lodsList;
	for (int i= 0; i < lodCount; ++i)
	{
		lodsList.append(*(meshData.m_LodList[i]));
	}
	stream << lodsList;

	return stream;
}

QDataStream &operator>>(QDataStream &stream, GLC_MeshData &meshData)
{
	quint32 chunckId;
	stream >> chunckId;
	Q_ASSERT(chunckId == GLC_MeshData::m_ChunkId);

	meshData.clear();

	GLfloatVector position, normal, texel, color;

	stream >> *(meshData.positionVectorHandle());
	stream >> *(meshData.normalVectorHandle());
	stream >> *(meshData.texelVectorHandle());
	stream >> *(meshData.colorVectorHandle());

	// List of lod serialisation
	QList<GLC_Lod> lodsList;
	stream >> lodsList;
	const int lodCount= lodsList.size();
	for (int i= 0; i < lodCount; ++i)
	{
		meshData.m_LodList.append(new GLC_Lod(lodsList.at(i)));
	}

	return stream;
}

