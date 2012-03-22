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

//! \file glc_primitivegroup.cpp implementation of the GLC_PrimitiveGroup class.

#include "glc_primitivegroup.h"
#include "../glc_state.h"

// Class chunk id
quint32 GLC_PrimitiveGroup::m_ChunkId= 0xA700;

// Default constructor
GLC_PrimitiveGroup::GLC_PrimitiveGroup(GLC_uint materialId)
: m_Id(materialId)
, m_TrianglesIndex()
, m_TrianglesGroupsSizes()
, m_TrianglesGroupOffset()
, m_TrianglesGroupOffseti()
, m_TrianglesId()
, m_StripsIndex()
, m_StripIndexSizes()
, m_StripIndexOffset()
, m_StripIndexOffseti()
, m_StripsId()
, m_FansIndex()
, m_FansIndexSizes()
, m_FanIndexOffset()
, m_FanIndexOffseti()
, m_FansId()
, m_IsFinished(false)
, m_TrianglesIndexSize(0)
, m_TrianglesStripSize(0)
, m_TrianglesFanSize(0)
{


}
//! Copy constructor
GLC_PrimitiveGroup::GLC_PrimitiveGroup(const GLC_PrimitiveGroup& group)
: m_Id(group.m_Id)
, m_TrianglesIndex(group.m_TrianglesIndex)
, m_TrianglesGroupsSizes(group.m_TrianglesGroupsSizes)
, m_TrianglesGroupOffset(group.m_TrianglesGroupOffset)
, m_TrianglesGroupOffseti(group.m_TrianglesGroupOffseti)
, m_TrianglesId(group.m_TrianglesId)
, m_StripsIndex(group.m_StripsIndex)
, m_StripIndexSizes(group.m_StripIndexSizes)
, m_StripIndexOffset(group.m_StripIndexOffset)
, m_StripIndexOffseti(group.m_StripIndexOffseti)
, m_StripsId(group.m_StripsId)
, m_FansIndex(group.m_FansIndex)
, m_FansIndexSizes(group.m_FansIndexSizes)
, m_FanIndexOffset(group.m_FanIndexOffset)
, m_FanIndexOffseti(group.m_FanIndexOffseti)
, m_FansId(group.m_FansId)
, m_IsFinished(group.m_IsFinished)
, m_TrianglesIndexSize(group.m_TrianglesIndexSize)
, m_TrianglesStripSize(group.m_TrianglesStripSize)
, m_TrianglesFanSize(group.m_TrianglesFanSize)
{


}

//! Copy constructor
GLC_PrimitiveGroup::GLC_PrimitiveGroup(const GLC_PrimitiveGroup& group, GLC_uint id)
: m_Id(id)
, m_TrianglesIndex(group.m_TrianglesIndex)
, m_TrianglesGroupsSizes(group.m_TrianglesGroupsSizes)
, m_TrianglesGroupOffset(group.m_TrianglesGroupOffset)
, m_TrianglesGroupOffseti(group.m_TrianglesGroupOffseti)
, m_TrianglesId(group.m_TrianglesId)
, m_StripsIndex(group.m_StripsIndex)
, m_StripIndexSizes(group.m_StripIndexSizes)
, m_StripIndexOffset(group.m_StripIndexOffset)
, m_StripIndexOffseti(group.m_StripIndexOffseti)
, m_StripsId(group.m_StripsId)
, m_FansIndex(group.m_FansIndex)
, m_FansIndexSizes(group.m_FansIndexSizes)
, m_FanIndexOffset(group.m_FanIndexOffset)
, m_FanIndexOffseti(group.m_FanIndexOffseti)
, m_FansId(group.m_FansId)
, m_IsFinished(group.m_IsFinished)
, m_TrianglesIndexSize(group.m_TrianglesIndexSize)
, m_TrianglesStripSize(group.m_TrianglesStripSize)
, m_TrianglesFanSize(group.m_TrianglesFanSize)
{


}

// = operator
GLC_PrimitiveGroup& GLC_PrimitiveGroup::operator=(const GLC_PrimitiveGroup& group)
{
	if (this != &group)
	{
		m_Id= group.m_Id;
		m_TrianglesIndex= group.m_TrianglesIndex;
		m_TrianglesGroupsSizes= group.m_TrianglesGroupsSizes;
		m_TrianglesGroupOffset= group.m_TrianglesGroupOffset;
		m_TrianglesGroupOffseti= group.m_TrianglesGroupOffseti;
		m_TrianglesId= group.m_TrianglesId;
		m_StripsIndex= group.m_StripsIndex;
		m_StripIndexSizes= group.m_StripIndexSizes;
		m_StripIndexOffset= group.m_StripIndexOffset;
		m_StripIndexOffseti= group.m_StripIndexOffseti;
		m_StripsId= group.m_StripsId;
		m_FansIndex= group.m_FansIndex;
		m_FansIndexSizes= group.m_FansIndexSizes;
		m_FanIndexOffset= group.m_FanIndexOffset;
		m_FanIndexOffseti= group.m_FanIndexOffseti;
		m_FansId= group.m_FansId;
		m_IsFinished= group.m_IsFinished;
		m_TrianglesIndexSize= group.m_TrianglesIndexSize;
		m_TrianglesStripSize= group.m_TrianglesStripSize;
		m_TrianglesFanSize= group.m_TrianglesFanSize;
	}
	return *this;
}

GLC_PrimitiveGroup::~GLC_PrimitiveGroup()
{

}
// Return the class Chunk ID
quint32 GLC_PrimitiveGroup::chunckID()
{
	return m_ChunkId;
}

// Add triangles to the group
void GLC_PrimitiveGroup::addTriangles(const IndexList& input, GLC_uint id)
{
	m_TrianglesIndex+= input;
	m_TrianglesIndexSize= m_TrianglesIndex.size();

	m_TrianglesGroupsSizes.append(static_cast<GLsizei>(input.size()));

	if (m_TrianglesGroupOffseti.isEmpty())
	{
		m_TrianglesGroupOffseti.append(0);
	}
	int offset= m_TrianglesGroupOffseti.last() + m_TrianglesGroupsSizes.last();
	m_TrianglesGroupOffseti.append(offset);

	// The Triangles group id
	if (0 != id) m_TrianglesId.append(id);
	else Q_ASSERT(m_TrianglesId.isEmpty());
}

// Add triangle strip to the group
void GLC_PrimitiveGroup::addTrianglesStrip(const IndexList& input, GLC_uint id)
{
	m_StripsIndex+= input;
	m_TrianglesStripSize= m_StripsIndex.size();

	m_StripIndexSizes.append(static_cast<GLsizei>(input.size()));

	if (m_StripIndexOffseti.isEmpty())
	{
		m_StripIndexOffseti.append(0);
	}
	int offset= m_StripIndexOffseti.last() + m_StripIndexSizes.last();
	m_StripIndexOffseti.append(offset);

	// The strip id
	if (0 != id) m_StripsId.append(id);
	else Q_ASSERT(m_StripsId.isEmpty());
}
// Set the triangle index offset
void GLC_PrimitiveGroup::setTrianglesOffset(GLvoid* pOffset)
{
	//m_TrianglesGroupOffseti.pop_back();
	const int size= m_TrianglesGroupOffseti.size();
	for (int i= 0; i < size; ++i)
	{
		m_TrianglesGroupOffset.append(BUFFER_OFFSET(static_cast<GLsizei>(m_TrianglesGroupOffseti[i]) * sizeof(GLuint) + reinterpret_cast<GLsizeiptr>(pOffset)));
	}
}

// Set the triangle index offset
void GLC_PrimitiveGroup::setTrianglesOffseti(int offset)
{
	m_TrianglesGroupOffseti.pop_back();
	const int size= m_TrianglesGroupOffseti.size();
	for (int i= 0; i < size; ++i)
	{
		m_TrianglesGroupOffseti[i]= m_TrianglesGroupOffseti[i] + offset;
	}
}

// Set base triangle strip offset
void GLC_PrimitiveGroup::setBaseTrianglesStripOffset(GLvoid* pOffset)
{
	//m_StripIndexOffseti.pop_back();
	const int size= m_StripIndexOffseti.size();
	for (int i= 0; i < size; ++i)
	{
		m_StripIndexOffset.append(BUFFER_OFFSET(static_cast<GLsizei>(m_StripIndexOffseti[i]) * sizeof(GLuint) + reinterpret_cast<GLsizeiptr>(pOffset)));
	}
}

// Set base triangle strip offset
void GLC_PrimitiveGroup::setBaseTrianglesStripOffseti(int offset)
{
	m_StripIndexOffseti.pop_back();
	const int size= m_StripIndexOffseti.size();
	for (int i= 0; i < size; ++i)
	{
		m_StripIndexOffseti[i]= m_StripIndexOffseti[i] + offset;
	}
}

//! Add triangle fan to the group
void GLC_PrimitiveGroup::addTrianglesFan(const IndexList& input, GLC_uint id)
{
	m_FansIndex+= input;
	m_TrianglesFanSize= m_FansIndex.size();

	m_FansIndexSizes.append(static_cast<GLsizei>(input.size()));

	if (m_FanIndexOffseti.isEmpty())
	{
		m_FanIndexOffseti.append(0);
	}
	int offset= m_FanIndexOffseti.last() + m_FansIndexSizes.last();
	m_FanIndexOffseti.append(offset);

	// The fan id
	if (0 != id) m_FansId.append(id);
	else Q_ASSERT(m_FansId.isEmpty());


}

// Set base triangle fan offset
void GLC_PrimitiveGroup::setBaseTrianglesFanOffset(GLvoid* pOffset)
{
	//m_FanIndexOffseti.pop_back();
	const int size= m_FanIndexOffseti.size();
	for (int i= 0; i < size; ++i)
	{
		m_FanIndexOffset.append(BUFFER_OFFSET(static_cast<GLsizei>(m_FanIndexOffseti[i]) * sizeof(GLuint) + reinterpret_cast<GLsizeiptr>(pOffset)));
	}
}

// Set base triangle fan offset
void GLC_PrimitiveGroup::setBaseTrianglesFanOffseti(int offset)
{
	m_FanIndexOffseti.pop_back();
	const int size= m_FanIndexOffseti.size();
	for (int i= 0; i < size; ++i)
	{
		m_FanIndexOffseti[i]= m_FanIndexOffseti[i] + offset;
	}
}

// Change index to VBO mode
void GLC_PrimitiveGroup::computeVboOffset()
{
	m_TrianglesGroupOffset.clear();
	const int triangleOffsetSize= m_TrianglesGroupOffseti.size();
	for (int i= 0; i < triangleOffsetSize; ++i)
	{
		m_TrianglesGroupOffset.append(BUFFER_OFFSET(static_cast<GLsizei>(m_TrianglesGroupOffseti.at(i)) * sizeof(GLuint)));
	}

	m_StripIndexOffset.clear();
	const int stripOffsetSize= m_StripIndexOffseti.size();
	for (int i= 0; i < stripOffsetSize; ++i)
	{
		m_StripIndexOffset.append(BUFFER_OFFSET(static_cast<GLsizei>(m_StripIndexOffseti.at(i)) * sizeof(GLuint)));
	}

	m_FanIndexOffset.clear();
	const int fanOffsetSize= m_FanIndexOffseti.size();
	for (int i= 0; i < fanOffsetSize; ++i)
	{
		m_FanIndexOffset.append(BUFFER_OFFSET(static_cast<GLsizei>(m_FanIndexOffseti.at(i)) * sizeof(GLuint)));
	}
}

// Clear the group
void GLC_PrimitiveGroup::clear()
{
	m_TrianglesIndex.clear();
	m_TrianglesGroupsSizes.clear();
	m_TrianglesGroupOffset.clear();
	m_TrianglesGroupOffseti.clear();
	m_StripsIndex.clear();
	m_StripIndexSizes.clear();
	m_StripIndexOffset.clear();
	m_StripIndexOffseti.clear();
	m_FansIndex.clear();
	m_FansIndexSizes.clear();
	m_FanIndexOffset.clear();
	m_FanIndexOffseti.clear();
	m_IsFinished= false;
	m_TrianglesIndexSize= 0;
	m_TrianglesStripSize= 0;
	m_TrianglesFanSize= 0;
}


// Non Member methods
// Non-member stream operator
QDataStream &operator<<(QDataStream &stream, const GLC_PrimitiveGroup &primitiveGroup)
{
	Q_ASSERT(primitiveGroup.isFinished());
	quint32 chunckId= GLC_PrimitiveGroup::m_ChunkId;
	stream << chunckId;

	// Primitive group id
	stream << primitiveGroup.m_Id;

	// Triangles, strips and fan offset index
	OffsetVectori trianglesGroupOffseti;
	OffsetVectori stripIndexOffseti;
	OffsetVectori fanIndexOffseti;

	// Get triangles, strips and fans offset
	trianglesGroupOffseti= primitiveGroup.m_TrianglesGroupOffseti;
	stripIndexOffseti= primitiveGroup.m_StripIndexOffseti;
	fanIndexOffseti= primitiveGroup.m_FanIndexOffseti;

	// Triangles index
	stream << primitiveGroup.m_TrianglesIndexSize;
	stream << trianglesGroupOffseti;
	stream << primitiveGroup.m_TrianglesGroupsSizes;
	stream << primitiveGroup.m_TrianglesId;

	// Triangles strips index
	stream << primitiveGroup.m_TrianglesStripSize;
	stream << stripIndexOffseti;
	stream << primitiveGroup.m_StripIndexSizes;
	stream << primitiveGroup.m_StripsId;

	// Triangles fans index
	stream << primitiveGroup.m_TrianglesFanSize;
	stream << fanIndexOffseti;
	stream << primitiveGroup.m_FansIndexSizes;
	stream << primitiveGroup.m_FansId;

	return stream;
}
QDataStream &operator>>(QDataStream &stream, GLC_PrimitiveGroup &primitiveGroup)
{
	quint32 chunckId;
	stream >> chunckId;
	Q_ASSERT(chunckId == GLC_PrimitiveGroup::m_ChunkId);
	stream >> primitiveGroup.m_Id;

	// Triangles index
	stream >> primitiveGroup.m_TrianglesIndexSize;
	stream >> primitiveGroup.m_TrianglesGroupOffseti;
	stream >> primitiveGroup.m_TrianglesGroupsSizes;
	stream >> primitiveGroup.m_TrianglesId;

	// Triangles strips index
	stream >> primitiveGroup.m_TrianglesStripSize;
	stream >> primitiveGroup.m_StripIndexOffseti;
	stream >> primitiveGroup.m_StripIndexSizes;
	stream >> primitiveGroup.m_StripsId;

	// Triangles fans index
	stream >> primitiveGroup.m_TrianglesFanSize;
	stream >> primitiveGroup.m_FanIndexOffseti;
	stream >> primitiveGroup.m_FansIndexSizes;
	stream >> primitiveGroup.m_FansId;


	primitiveGroup.finish();

	return stream;
}

