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

//! \file glc_primitivegroup.h interface for the GLC_PrimitiveGroup class.

#ifndef GLC_PRIMITIVEGROUP_H_
#define GLC_PRIMITIVEGROUP_H_

#include "../glc_ext.h"
#include "../glc_global.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_PrimitiveGroup
/*! \brief GLC_PrimitiveGroup : Triangles, Strip and fan index*/

/*! An GLC_PrimitiveGroup is used to stored Triangles, strips and fans index
 * Grouped by material*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_PrimitiveGroup
{
	friend GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_PrimitiveGroup &);
	friend GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_PrimitiveGroup &);

public:
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

	//! Default constructor
	GLC_PrimitiveGroup(GLC_uint id= 0);

	//! Copy constructor
	GLC_PrimitiveGroup(const GLC_PrimitiveGroup&);

	//! Copy constructor
	GLC_PrimitiveGroup(const GLC_PrimitiveGroup&, GLC_uint);

	//! = operator
	GLC_PrimitiveGroup& operator=(const GLC_PrimitiveGroup&);

	~GLC_PrimitiveGroup();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the class Chunk ID
	static quint32 chunckID();

	//! Return true if the group is finished
	inline bool isFinished() const
	{return m_IsFinished;}

	//! Return the group id
	inline GLC_uint id() const
	{return m_Id;}

	//! Return true if the group contains triangles
	inline bool containsTriangles() const
	{return m_TrianglesIndexSize > 0;}

	//! Return true if the group contains triangles group id
	inline bool containsTrianglesGroupId() const
	{return !m_TrianglesId.isEmpty();}

	//! Return the Triangle group ID
	inline GLC_uint triangleGroupId(int index)
	{return m_TrianglesId.at(index);}

	//! Return the size of list of triangles index of the group
	inline int trianglesIndexSize() const
	{return m_TrianglesIndexSize;}

	//! Return the size of list of triangles index of the group
	inline const IndexSizes& trianglesIndexSizes() const
	{return m_TrianglesGroupsSizes;}

	//! Return the list of triangles index of the group
	inline const IndexList& trianglesIndex() const
	{
		Q_ASSERT(!m_IsFinished);
		return m_TrianglesIndex;
	}

	//! Return the offset of triangles index
	inline const GLvoid* trianglesIndexOffset() const
	{return m_TrianglesGroupOffset.first();}

	//! Return the offset of triangles index
	inline int trianglesIndexOffseti() const
	{return m_TrianglesGroupOffseti.first();}

	//! Return the offset of triangles index
	inline const OffsetVector& trianglesGroupOffset() const
	{return m_TrianglesGroupOffset;}

	//! Return the offset of triangles index
	inline const OffsetVectori& trianglesGroupOffseti() const
	{return m_TrianglesGroupOffseti;}

	//! Return true if the group contains strips
	inline bool containsStrip() const
	{return m_TrianglesStripSize > 0;}

	//! Return true if the group contains strips group id
	inline bool containsStripGroupId() const
	{return !m_StripsId.isEmpty();}

	//! Return the strip ID
	inline GLC_uint stripGroupId(int index)
	{return m_StripsId.at(index);}

	//! Return the size of index of strips
	inline int stripsIndexSize() const
	{return m_TrianglesStripSize;}

	//! Return the list of index of strips
	inline const IndexList& stripsIndex() const
	{
		Q_ASSERT(!m_IsFinished);
		return m_StripsIndex;
	}

	//! Return the vector of strips sizes
	inline const IndexSizes& stripsSizes() const
	{return m_StripIndexSizes;}

	//! Return the vector of strip offset
	inline const OffsetVector& stripsOffset() const
	{return m_StripIndexOffset;}

	//! Return the vector of strip offset
	inline const OffsetVectori& stripsOffseti() const
	{return m_StripIndexOffseti;}

	//! Return true if the group contains fans
	inline bool containsFan() const
	{return m_TrianglesFanSize > 0;}

	//! Return true if the group contains fans group id
	inline bool containsFanGroupId() const
	{return !m_FansId.isEmpty();}

	//! Return the fan ID
	inline GLC_uint fanGroupId(int index)
	{return m_FansId.at(index);}

	//! Return the size of index of fans
	inline int fansIndexSize() const
	{return m_TrianglesFanSize;}

	//! Return the list of index of fans
	inline const IndexList& fansIndex() const
	{
		Q_ASSERT(!m_IsFinished);
		return m_FansIndex;
	}

	//! Return the vector of fans sizes
	inline const IndexSizes& fansSizes() const
	{return m_FansIndexSizes;}

	//! Return the vector of strip offset
	inline const OffsetVector& fansOffset() const
	{return m_FanIndexOffset;}

	//! Return the vector of strip offset
	inline const OffsetVectori& fansOffseti() const
	{return m_FanIndexOffseti;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set the group id
	inline void setId(GLC_uint id)
	{m_Id= id;}

	//! Add triangles to the group
	void addTriangles(const IndexList& input, GLC_uint id= 0);

	//! Set the triangle index offset
	void setTrianglesOffset(GLvoid* pOffset);

	//! Set the triangle index offset
	void setTrianglesOffseti(int offset);

	//! Add triangle strip to the group
	void addTrianglesStrip(const IndexList&, GLC_uint id= 0);

	//! Set base triangle strip offset
	void setBaseTrianglesStripOffset(GLvoid*);

	//! Set base triangle strip offset
	void setBaseTrianglesStripOffseti(int);

	//! Add triangle fan to the group
	void addTrianglesFan(const IndexList&, GLC_uint id= 0);

	//! Set base triangle fan offset
	void setBaseTrianglesFanOffset(GLvoid*);

	//! Set base triangle fan offset
	void setBaseTrianglesFanOffseti(int);

	//! Compute VBO offset
	void computeVboOffset();

	//! The mesh wich use this group is finished
	inline void finish()
	{
		m_TrianglesIndex.clear();
		m_StripsIndex.clear();
		m_FansIndex.clear();
		m_IsFinished= true;
	}

	//! Clear the group
	void clear();

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Grouped material id
	GLC_uint m_Id;

	//! Triangles index list
	IndexList m_TrianglesIndex;

	//! Triangles groups index size
	IndexSizes m_TrianglesGroupsSizes;

	//! Vector of triangles group offset
	OffsetVector m_TrianglesGroupOffset;
	OffsetVectori m_TrianglesGroupOffseti;

	//! Triangles groups id
	QList<GLC_uint> m_TrianglesId;

	//! Strips index list
	IndexList m_StripsIndex;

	//! Strips index size
	IndexSizes m_StripIndexSizes;

	//! Vector of strips offset
	OffsetVector m_StripIndexOffset;
	OffsetVectori m_StripIndexOffseti;

	//! Strips id
	QList<GLC_uint> m_StripsId;

	//! Fans index list
	IndexList m_FansIndex;

	//! Fans index size
	IndexSizes m_FansIndexSizes;

	//! Vector of fan Offset
	OffsetVector m_FanIndexOffset;
	OffsetVectori m_FanIndexOffseti;

	//! Fans id
	QList<GLC_uint> m_FansId;

	//! Flag to know if the group is finish
	int m_IsFinished;

	//! Flag to know if there is triangles
	int m_TrianglesIndexSize;

	//! Flag to know if there is triangles strip
	int m_TrianglesStripSize;

	//! Flag to know if there is triangles fan
	int m_TrianglesFanSize;

	//! Class chunk id
	static quint32 m_ChunkId;

};

//! Non-member stream operator
GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_PrimitiveGroup &);
GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_PrimitiveGroup &);

#endif /* GLC_PRIMITIVEGROUP_H_ */
