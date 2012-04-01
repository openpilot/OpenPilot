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

//! \file glc_lod.h interface for the GLC_Lod class.

#ifndef GLC_LOD_H_
#define GLC_LOD_H_

#include <QVector>
#include <QGLBuffer>

#include "../glc_ext.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Lod
/*! \brief GLC_Lod is a Level of detail index and accuracy*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Lod
{
	friend GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_Lod &);
	friend GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_Lod &);

public:
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default Constructor
	GLC_Lod();

	//! Construct a Lod with the specified accuracy
	GLC_Lod(double accuracy);

	//! Copy constructor
	GLC_Lod(const GLC_Lod&);

	//! Overload "=" operator
	GLC_Lod& operator=(const GLC_Lod&);

	//!Destructor
	virtual ~GLC_Lod();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the class Chunk ID
	static quint32 chunckID();

	//! Return the accuracy of the LOD
	inline double accuracy() const
	{return m_Accuracy;}

	//! Return The unique index Vector which contains :
	/*!
	 * - Triangles index
	 * - Triangles Strips index
	 * - Triangles Fans index
	 */
	QVector<GLuint> indexVector() const;

	//! Return The unique index Vector handle which contains :
	/*!
	 * - Triangles index
	 * - Triangles Strips index
	 * - Triangles Fans index
	 */
	inline QVector<GLuint>* indexVectorHandle()
	{return &m_IndexVector;}

	//! Return the size of the index Vector
	inline int indexVectorSize() const
	{return m_IndexVector.size();}

	//! Return this lod triangle count
	inline unsigned int trianglesCount() const
	{return m_TrianglesCount;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Copy IBO to the Client Side
	void copyIboToClientSide();

	//! Release client IBO
	void releaseIboClientSide(bool update= false);

	//! Set accuracy of the LOD
	inline void setAccuracy(const double& accuracy)
	{m_Accuracy= accuracy;}

	//! Given number of triangles added
	inline void trianglesAdded(unsigned int count)
	{
		m_TrianglesCount+= count;
	}

	//! Set IBO usage
	void setIboUsage(bool usage);


//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! IBO creation
	inline void createIBO()
	{
		if (!m_IndexBuffer.isCreated() && !m_IndexVector.isEmpty())
		{
			m_IndexBuffer.create();
		}
	}

	//! Ibo Usage
	void useIBO() const;

	//! Fill IBO
	inline void fillIbo()
	{releaseIboClientSide(true);}

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:

	//! The accuracy of the LOD
	double m_Accuracy;

	//! The Index Buffer
	QGLBuffer m_IndexBuffer;

	//! The Index Vector
	QVector<GLuint> m_IndexVector;

	//! The Index vector size
	int m_IndexSize;

	//! Lod number of faces
	unsigned int m_TrianglesCount;

	//! Class chunk id
	static quint32 m_ChunkId;

};

//! Non-member stream operator
GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_Lod &);
GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_Lod &);

#endif /* GLC_LOD_H_ */
