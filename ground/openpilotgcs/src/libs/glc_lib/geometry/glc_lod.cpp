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

//! \file glc_lod.cpp implementation of the GLC_Lod class.


#include "glc_lod.h"

// Class chunk id
quint32 GLC_Lod::m_ChunkId= 0xA708;


GLC_Lod::GLC_Lod()
: m_Accuracy(0.0)
, m_IboId(0)
, m_IndexVector()
, m_IndexSize(0)
, m_TrianglesCount(0)
{

}


GLC_Lod::GLC_Lod(double accuracy)
: m_Accuracy(accuracy)
, m_IboId(0)
, m_IndexVector()
, m_IndexSize(0)
, m_TrianglesCount(0)
{

}


GLC_Lod::GLC_Lod(const GLC_Lod& lod)
: m_Accuracy(lod.m_Accuracy)
, m_IboId(0)
, m_IndexVector(lod.indexVector())
, m_IndexSize(lod.m_IndexSize)
, m_TrianglesCount(lod.m_TrianglesCount)
{


}


GLC_Lod& GLC_Lod::operator=(const GLC_Lod& lod)
{
	if (this != &lod)
	{
		m_Accuracy= lod.m_Accuracy;
		m_IboId= 0;
		m_IndexVector= lod.indexVector();
		m_IndexSize= lod.m_IndexSize;
		m_TrianglesCount= lod.m_TrianglesCount;
	}

	return *this;
}

GLC_Lod::~GLC_Lod()
{
	// Delete IBO
	if (0 != m_IboId)
	{
		glDeleteBuffers(1, &m_IboId);
	}
}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

quint32 GLC_Lod::chunckID()
{
	return m_ChunkId;
}


QVector<GLuint> GLC_Lod::indexVector() const
{
	if (0 != m_IboId)
	{
		// VBO created get data from VBO
		const int sizeOfIbo= m_IndexSize;
		const GLsizeiptr dataSize= sizeOfIbo * sizeof(GLuint);
		QVector<GLuint> indexVector(sizeOfIbo);

		useIBO();
		GLvoid* pIbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
		memcpy(indexVector.data(), pIbo, dataSize);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		return indexVector;
	}
	else
	{
		return m_IndexVector;
	}
}


void GLC_Lod::copyIboToClientSide()
{
	if ((0 != m_IboId) && (m_IndexVector.isEmpty()))
	{
		m_IndexVector= indexVector();
	}
}


void GLC_Lod::releaseIboClientSide(bool update)
{
	if((0 != m_IboId) && !m_IndexVector.isEmpty())
	{
		if (update)
		{
			// Copy index from client side to serveur
			useIBO();

			const GLsizei indexNbr= static_cast<GLsizei>(m_IndexVector.size());
			const GLsizeiptr indexSize = indexNbr * sizeof(GLuint);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, m_IndexVector.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		m_IndexVector.clear();
	}
}


QDataStream &operator<<(QDataStream &stream, const GLC_Lod &lod)
{
	quint32 chunckId= GLC_Lod::m_ChunkId;
	stream << chunckId;

	stream << lod.m_Accuracy;
	stream << lod.indexVector();
	stream << lod.m_TrianglesCount;

	return stream;
}
QDataStream &operator>>(QDataStream &stream, GLC_Lod &lod)
{
	quint32 chunckId;
	stream >> chunckId;
	Q_ASSERT(chunckId == GLC_Lod::m_ChunkId);

	stream >> lod.m_Accuracy;

	QVector<GLuint> indexVector;
	stream >> lod.m_IndexVector;
	stream >> lod.m_TrianglesCount;

	return stream;
}

