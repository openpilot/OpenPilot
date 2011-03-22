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

//! \file glc_wiredata.cpp Implementation for the GLC_WireData class.

#include "glc_wiredata.h"
#include "../glc_ext.h"
#include "../glc_state.h"

// Class chunk id
quint32 GLC_WireData::m_ChunkId= 0xA706;


GLC_WireData::GLC_WireData()
: m_VboId(0)
, m_NextPrimitiveLocalId(1)
, m_Positions()
, m_PositionSize(0)
, m_pBoundingBox(NULL)
, m_VerticeGrouprSizes()
, m_VerticeGroupOffset()
, m_VerticeGroupId()
, m_VerticeGroupCount(0)
{

}


GLC_WireData::GLC_WireData(const GLC_WireData& data)
: m_VboId(0)
, m_NextPrimitiveLocalId(data.m_NextPrimitiveLocalId)
, m_Positions(data.positionVector())
, m_PositionSize(data.m_PositionSize)
, m_pBoundingBox(NULL)
, m_VerticeGrouprSizes(data.m_VerticeGrouprSizes)
, m_VerticeGroupOffset(data.m_VerticeGroupOffset)
, m_VerticeGroupId(data.m_VerticeGroupId)
, m_VerticeGroupCount(data.m_VerticeGroupCount)
{
	if (NULL != data.m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox(*(data.m_pBoundingBox));
	}
}


GLC_WireData& GLC_WireData::operator=(const GLC_WireData& data)
{
	if (this != &data)
	{
		clear();
		m_NextPrimitiveLocalId= data.m_NextPrimitiveLocalId;
		m_Positions= data.positionVector();
		m_PositionSize= data.m_PositionSize;
		if (NULL != data.m_pBoundingBox)
		{
			m_pBoundingBox= new GLC_BoundingBox(*(data.m_pBoundingBox));
		}
		m_VerticeGrouprSizes= data.m_VerticeGrouprSizes;
		m_VerticeGroupOffset= data.m_VerticeGroupOffset;
		m_VerticeGroupId= data.m_VerticeGroupId;
		m_VerticeGroupCount= data.m_VerticeGroupCount;
	}
	return *this;
}

GLC_WireData::~GLC_WireData()
{
	clear();

	// Delete Main Vbo ID
	if (0 != m_VboId)
	{
		glDeleteBuffers(1, &m_VboId);
		m_VboId= 0;
	}
}
//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////


quint32 GLC_WireData::chunckID()
{
	return m_ChunkId;
}


GLfloatVector GLC_WireData::positionVector() const
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


GLC_BoundingBox& GLC_WireData::boundingBox()
{
	if (NULL == m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox();

		if (m_Positions.isEmpty())
		{
			//qDebug() << "GLC_WireData::getBoundingBox empty m_Positions";
		}
		else
		{
			const int max= m_Positions.size();
			for (int i= 0; i < max; i= i + 3)
			{
				GLC_Point3d point(m_Positions[i], m_Positions[i + 1], m_Positions[i + 2]);
				m_pBoundingBox->combine(point);
			}
		}

	}
	return *m_pBoundingBox;
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////


GLC_uint GLC_WireData::addVerticeGroup(const GLfloatVector& floatVector)
{
	Q_ASSERT((floatVector.size() % 3) == 0);

	++m_VerticeGroupCount;
	m_Positions+= floatVector;

	m_VerticeGrouprSizes.append(static_cast<GLsizei>(floatVector.size() / 3));

	if (m_VerticeGroupOffset.isEmpty())
	{
		m_VerticeGroupOffset.append(0);
	}
	int offset= m_VerticeGroupOffset.last() + m_VerticeGrouprSizes.last();
	m_VerticeGroupOffset.append(offset);

	// The Polyline id
	m_VerticeGroupId.append(m_NextPrimitiveLocalId);
	return m_NextPrimitiveLocalId++;
}

void GLC_WireData::clear()
{
	m_NextPrimitiveLocalId= 1;
	m_Positions.clear();
	m_PositionSize= 0;
	delete m_pBoundingBox;
	m_pBoundingBox= NULL;

	m_VerticeGrouprSizes.clear();
	m_VerticeGroupOffset.clear();
	m_VerticeGroupId.clear();
	m_VerticeGroupCount= 0;
}

void GLC_WireData::copyVboToClientSide()
{
	if ((0 != m_VboId) && m_Positions.isEmpty())
	{
		m_Positions= positionVector();
	}
}

void GLC_WireData::releaseVboClientSide(bool update)
{
	if ((0 != m_VboId) && !m_Positions.isEmpty())
	{
		if (update) finishVbo();
	}
}


//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_WireData::finishVbo()
{
	createVBOs();
	useVBO(true);
	fillVBOs();
	useVBO(false);

	m_PositionSize= m_Positions.size();
	m_Positions.clear();
}

void GLC_WireData::useVBO(bool use)
{
	if (use)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_VboId);	}
	else
	{
		// Unbind VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void GLC_WireData::glDraw(const GLC_RenderProperties&, GLenum mode)
{
	Q_ASSERT(!isEmpty());
	const bool vboIsUsed= GLC_State::vboUsed();

	if (vboIsUsed && ((m_PositionSize == 0) || (0 == m_VboId)))
	{
		finishVbo();
	}
	else if (m_PositionSize == 0)
	{
		m_PositionSize= m_Positions.size();
	}

	// Activate VBO or Vertex Array
	if (vboIsUsed)
	{
		useVBO(true);
		glVertexPointer(3, GL_FLOAT, 0, 0);
	}
	else
	{
		glVertexPointer(3, GL_FLOAT, 0, m_Positions.data());
	}
	glEnableClientState(GL_VERTEX_ARRAY);

	// Render polylines
	for (int i= 0; i < m_VerticeGroupCount; ++i)
	{
		glDrawArrays(mode, m_VerticeGroupOffset.at(i), m_VerticeGrouprSizes.at(i));
	}

	// Desactivate VBO or Vertex Array
	if (vboIsUsed)
	{
		useVBO(false);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
}

void GLC_WireData::createVBOs()
{
	// Create position VBO
	if (0 == m_VboId)
	{
		glGenBuffers(1, &m_VboId);
	}
}

void GLC_WireData::fillVBOs()
{
	const GLsizei dataNbr= static_cast<GLsizei>(m_Positions.size());
	const GLsizeiptr dataSize= dataNbr * sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, dataSize, m_Positions.data(), GL_STATIC_DRAW);
}

QDataStream &operator<<(QDataStream &stream, const GLC_WireData &wireData)
{
	quint32 chunckId= GLC_WireData::m_ChunkId;
	stream << chunckId;

	stream << wireData.m_NextPrimitiveLocalId;
	stream << wireData.positionVector();
	stream << wireData.m_PositionSize;

	stream << wireData.m_VerticeGrouprSizes;
	stream << wireData.m_VerticeGroupOffset;
	stream << wireData.m_VerticeGroupId;
	stream << wireData.m_VerticeGroupCount;

	return stream;
}

QDataStream &operator>>(QDataStream &stream, GLC_WireData &wireData)
{
	quint32 chunckId;
	stream >> chunckId;
	Q_ASSERT(chunckId == GLC_WireData::m_ChunkId);

	wireData.clear();
	stream >> wireData.m_NextPrimitiveLocalId;
	stream >> wireData.m_Positions;
	stream >> wireData.m_PositionSize;

	stream >> wireData.m_VerticeGrouprSizes;
	stream >> wireData.m_VerticeGroupOffset;
	stream >> wireData.m_VerticeGroupId;
	stream >> wireData.m_VerticeGroupCount;

	return stream;
}
