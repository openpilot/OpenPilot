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

//! \file glc_lod.cpp implementation of the GLC_Lod class.

#include "../glc_exception.h"
#include "glc_lod.h"

// Class chunk id
quint32 GLC_Lod::m_ChunkId= 0xA708;


GLC_Lod::GLC_Lod()
: m_Accuracy(0.0)
, m_IndexBuffer(QGLBuffer::IndexBuffer)
, m_IndexVector()
, m_IndexSize(0)
, m_TrianglesCount(0)
{

}


GLC_Lod::GLC_Lod(double accuracy)
: m_Accuracy(accuracy)
, m_IndexBuffer(QGLBuffer::IndexBuffer)
, m_IndexVector()
, m_IndexSize(0)
, m_TrianglesCount(0)
{

}


GLC_Lod::GLC_Lod(const GLC_Lod& lod)
: m_Accuracy(lod.m_Accuracy)
, m_IndexBuffer(QGLBuffer::IndexBuffer)
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
		m_IndexBuffer.destroy();
		m_IndexVector= lod.indexVector();
		m_IndexSize= lod.m_IndexSize;
		m_TrianglesCount= lod.m_TrianglesCount;
	}

	return *this;
}

GLC_Lod::~GLC_Lod()
{

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
	if (m_IndexBuffer.isCreated())
	{
		// VBO created get data from VBO
		const int sizeOfIbo= m_IndexSize;
		const GLsizeiptr dataSize= sizeOfIbo * sizeof(GLuint);
		QVector<GLuint> indexVector(sizeOfIbo);

		const_cast<QGLBuffer&>(m_IndexBuffer).bind();
		GLvoid* pIbo = const_cast<QGLBuffer&>(m_IndexBuffer).map(QGLBuffer::ReadOnly);
		memcpy(indexVector.data(), pIbo, dataSize);
		const_cast<QGLBuffer&>(m_IndexBuffer).unmap();
		const_cast<QGLBuffer&>(m_IndexBuffer).release();
		return indexVector;
	}
	else
	{
		return m_IndexVector;
	}
}


void GLC_Lod::copyIboToClientSide()
{
	if (m_IndexBuffer.isCreated() && (m_IndexVector.isEmpty()))
	{
		m_IndexVector= indexVector();
	}
}


void GLC_Lod::releaseIboClientSide(bool update)
{
	if(m_IndexBuffer.isCreated() && !m_IndexVector.isEmpty())
	{
		if (update)
		{
			// Copy index from client side to serveur
			m_IndexBuffer.bind();

			const GLsizei indexNbr= static_cast<GLsizei>(m_IndexVector.size());
			const GLsizeiptr indexSize = indexNbr * sizeof(GLuint);
			m_IndexBuffer.allocate(m_IndexVector.data(), indexSize);
			m_IndexBuffer.release();
		}
		m_IndexSize= m_IndexVector.size();
		m_IndexVector.clear();
	}
}

void GLC_Lod::setIboUsage(bool usage)
{
	if (usage && !m_IndexVector.isEmpty())
	{
		createIBO();
		// Copy index from client side to serveur
		m_IndexBuffer.bind();

		const GLsizei indexNbr= static_cast<GLsizei>(m_IndexVector.size());
		const GLsizeiptr indexSize = indexNbr * sizeof(GLuint);
		m_IndexBuffer.allocate(m_IndexVector.data(), indexSize);
		m_IndexBuffer.release();

		m_IndexSize= m_IndexVector.size();
		m_IndexVector.clear();

	}
	else if (!usage && m_IndexBuffer.isCreated())
	{
		m_IndexVector= indexVector();
		m_IndexBuffer.destroy();
	}
}

void GLC_Lod::useIBO() const
{
	Q_ASSERT(m_IndexBuffer.isCreated());
	if (!const_cast<QGLBuffer&>(m_IndexBuffer).bind())
	{
		GLC_Exception exception("GLC_Lod::useIBO  Failed to bind index buffer");
		throw(exception);
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
	stream >> lod.m_IndexVector;
	stream >> lod.m_TrianglesCount;

	return stream;
}

