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

//! \file glc_meshdata.h Interface for the GLC_MeshData class.

#ifndef GLC_MESHDATA_H_
#define GLC_MESHDATA_H_

#include <QVector>
#include <QGLBuffer>

#include "glc_lod.h"
#include "../glc_global.h"

#include "../glc_config.h"


//////////////////////////////////////////////////////////////////////
//! \class GLC_MeshData
/*! \brief GLC_MeshData : Contains all data of the mesh
 */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_MeshData
{
	friend GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_MeshData &);
	friend GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_MeshData &);

public:

	//! Enum of VBO TYPE
	enum VboType
	{
		GLC_Vertex= 30,
		GLC_Normal,
		GLC_Texel,
		GLC_Color
	};

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_MeshData();

	//! Copy constructor
	GLC_MeshData(const GLC_MeshData&);

	//! Overload "=" operator
	GLC_MeshData& operator=(const GLC_MeshData&);

	//! Destructor
	virtual ~GLC_MeshData();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the class Chunk ID
	static quint32 chunckID();

	//! Return the number of lod
	inline int lodCount() const
	{return m_LodList.size();}

	//! Return the Position Vector
	GLfloatVector positionVector() const;

	//! Return the normal Vector
	GLfloatVector normalVector() const;

	//! Return the texel Vector
	GLfloatVector texelVector() const;

	//! Return the color Vector
	GLfloatVector colorVector() const;

	//! Return the Position Vector handle
	inline GLfloatVector* positionVectorHandle()
	{ return &m_Positions;}

	//! Return the Normal Vector handle
	inline GLfloatVector* normalVectorHandle()
	{ return &m_Normals;}

	//! Return the Texel Vector handle
	inline GLfloatVector* texelVectorHandle()
	{ return &m_Texels;}

	//! Return the Color Vector handle
	inline GLfloatVector* colorVectorHandle()
	{ return &m_Colors;}

	//! Return the Index Vector of the specified LOD
	inline GLuintVector indexVector(const int i= 0) const
	{
		Q_ASSERT(i < m_LodList.size());
		return m_LodList.at(i)->indexVector();
	}

	//! Return the Index Vector handle of the specified LOD
	inline GLuintVector* indexVectorHandle(const int i= 0) const
	{
		Q_ASSERT(i < m_LodList.size());
		return m_LodList.at(i)->indexVectorHandle();
	}

	//! Return the size of the triangles index Vector of the specified LOD
	inline int indexVectorSize(const int i= 0) const
	{
		Q_ASSERT(i < m_LodList.size());
		return m_LodList.at(i)->indexVectorSize();
	}
	//! Return the specified LOD if the LOD doesn't exists, return NULL
	inline GLC_Lod* getLod(int index) const
	{
		return m_LodList.value(index);
	}

	//! Return true if the mesh data doesn't contains vertice
	inline bool isEmpty() const
	{return (1 > m_PositionSize) && (0 == m_Positions.size());}

	//! Return the number of triangle from the given lod index
	inline unsigned int trianglesCount(int lod) const
	{
		Q_ASSERT(lod < m_LodList.size());
		return m_LodList.at(lod)->trianglesCount();
	}

	//! Return true if the position size is set
	inline bool positionSizeIsSet() const
	{return m_PositionSize != -1;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Add a empty Lod to the engine
	inline void appendLod(double accuracy= 0.0)
	{m_LodList.append(new GLC_Lod(accuracy));}

	//! If the there is more than 2 LOD Swap the first and last
	void finishLod();

	//! Clear the content of the meshData and makes it empty
	void clear();

	//! Copy VBO to the Client Side
	void copyVboToClientSide();

	//! Release client VBO
	void releaseVboClientSide(bool update= false);

	//! Given number of triangles added to the given lod index
	inline void trianglesAdded(int lod, int number)
	{
		if (lod != 0) lod-= 1;
		else lod= m_LodList.size() - 1;
		m_LodList.at(lod)->trianglesAdded(number);
	}

	//! Set VBO usage
	void setVboUsage(bool usage);

	//! Init the position size
	inline void initPositionSize()
	{m_PositionSize= m_Positions.size();}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Vbo creation
	void createVBOs();

	//! Ibo Usage
	bool useVBO(bool, GLC_MeshData::VboType);

	//! Ibo Usage
	inline void useIBO(bool use, const int currentLod= 0)
	{
		if (use) m_LodList.at(currentLod)->useIBO();
		else QGLBuffer::release(QGLBuffer::IndexBuffer);
	}

	//! Fill all LOD IBO
	void fillLodIbo();

	//! Fill the VBO of the given type
	void fillVbo(GLC_MeshData::VboType vboType);

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:

	//! The vertex Buffer
	QGLBuffer m_VertexBuffer;

	//! Vertex Position Vector
	GLfloatVector m_Positions;

	//! Vertex Normal Vector
	GLfloatVector m_Normals;

	//! Vertex Texture coordinate
	GLfloatVector m_Texels;

	//! Color index
	GLfloatVector m_Colors;

	//! Normals Buffer
	QGLBuffer m_NormalBuffer;

	//! Texture Buffer
	QGLBuffer m_TexelBuffer;

	//! Color Buffer
	QGLBuffer m_ColorBuffer;

	//! The list of LOD
	QList<GLC_Lod*> m_LodList;

	//! The size of Position and normal VBO
	int m_PositionSize;

	//! The size of texel VBO
	int m_TexelsSize;

	//! The size of Color VBO
	int m_ColorSize;

	//! Use VBO
	bool m_UseVbo;

	//! Class chunk id
	static quint32 m_ChunkId;
};

//! Non-member stream operator
GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_MeshData &);
GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_MeshData &);

#endif /* GLC_MESHDATA_H_ */
