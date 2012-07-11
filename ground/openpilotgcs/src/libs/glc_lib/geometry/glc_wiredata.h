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
//! \file glc_wiredata.h Interface for the GLC_WireData class.

#ifndef GLC_WIREDATA_H_
#define GLC_WIREDATA_H_

#include <QColor>
#include <QGLBuffer>

#include "../glc_global.h"
#include "../glc_boundingbox.h"
#include "../shading/glc_renderproperties.h"

#include "../glc_config.h"
//////////////////////////////////////////////////////////////////////
//! \class GLC_WireData
/*! \brief GLC_WireData : Contains geometries's wire data
 */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_WireData
{
	friend GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_WireData &);
	friend GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_WireData &);

	//! Enum of VBO TYPE
	enum VboType
	{
		GLC_Vertex= 30,
		GLC_Color,
		GLC_Index
	};

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct a empty wire data
	GLC_WireData();

	//! Construct wire data from the given wire data
	GLC_WireData(const GLC_WireData&);

	//! Copy the given wire data in this wire data
	GLC_WireData& operator=(const GLC_WireData&);

	//! Destructor
	virtual ~GLC_WireData();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return this wire data class Chunk ID
	static quint32 chunckID();

	//! Return this wire data Position Vector
	GLfloatVector positionVector() const;

	//! Return the color Vector
	GLfloatVector colorVector() const;

	//! Return the unique index vector
	QVector<GLuint> indexVector() const;

	//! Return true if this wire data is empty
	inline bool isEmpty() const
	{return ((m_PositionSize == 0) && m_Positions.isEmpty());}

	//! Return this wire data bounding box
	GLC_BoundingBox& boundingBox();

	//! Return the number of vertice group
	inline int verticeGroupCount() const
	{return m_VerticeGroupCount;}

	//! Return the vertice group offset from the given index
	inline GLuint verticeGroupOffset(int index) const
	{return m_VerticeGroupOffseti.at(index);}

	//! Return the vertice group size from the given index
	inline GLsizei verticeGroupSize(int index) const
	{return m_VerticeGrouprSizes.at(index);}

	//! Return true if this wire data use indexed colors
	inline bool useIndexdColors() const
	{return (m_ColorSize > 0) || (m_Colors.size() > 0);}
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Add a Polyline to this wire and returns its id if id are managed
	GLC_uint addVerticeGroup(const GLfloatVector&);

	//! Add Colors
	inline void addColors(const GLfloatVector& colors)
	{m_Colors+= colors;}

	//! Clear the content of this wire Data and makes it empty
	void clear();

	//! Copy VBO to the Client Side
	void copyVboToClientSide();

	//! Release client VBO
	void releaseVboClientSide(bool update= false);

	//! Set VBO usage
	void setVboUsage(bool usage);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Make this wire data a VBO
	void finishVbo();

	//! Set vbo usage of this wire data
	void useVBO(GLC_WireData::VboType type, bool usage);

	//! Render this wire data using Opengl
	/*! The mode can be : GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP GL_LINES*/
	void glDraw(const GLC_RenderProperties&, GLenum mode);

private:

	//! Fill this wire data VBO from memmory
	void fillVBOs();

	//! Built index
	void buidIndex();

	//! Activate VBO and IBO
	void activateVboAndIbo();

	//! Finish offset
	void finishOffset();
//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! VBO ID
	QGLBuffer m_VerticeBuffer;

	//! The next primitive local id
	GLC_uint m_NextPrimitiveLocalId;

	//! Vertex Position Vector
	GLfloatVector m_Positions;

	//! Color Buffer
	QGLBuffer m_ColorBuffer;

	//! Color index
	GLfloatVector m_Colors;

	//! The Index Buffer
	QGLBuffer m_IndexBuffer;

	//! The Index Vector
	QVector<GLuint> m_IndexVector;

	//! The size of the VBO
	int m_PositionSize;

	//! The size of Color VBO
	int m_ColorSize;

	//! Wire data bounding box
	GLC_BoundingBox* m_pBoundingBox;

	//! Vector of Vertice group size
	IndexSizes m_VerticeGrouprSizes;

	//! Vector of vertice group offset
	OffsetVectori m_VerticeGroupOffseti;

	//! VBO Vector of vertice group offset
	OffsetVector m_VerticeGroupOffset;

	//! Vertice groups id
	QList<GLC_uint> m_VerticeGroupId;

	//! The number of vertice group
	int m_VerticeGroupCount;

	//! VBO usage
	bool m_UseVbo;

	//! Class chunk id
	static quint32 m_ChunkId;
};

//! Non-member stream operator
GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_WireData &);
GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_WireData &);

#endif /* GLC_WIREDATA_H_ */
