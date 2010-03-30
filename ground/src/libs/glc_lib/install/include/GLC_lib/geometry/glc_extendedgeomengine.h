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

//! \file glc_extendedgeomengine.h Interface for the GLC_ExtendedGeomEngine class.

#ifndef GLC_EXTENDEDGEOMENGINE_H_
#define GLC_EXTENDEDGEOMENGINE_H_

#include <QVector>
#include "glc_geomengine.h"
#include "glc_enginelod.h"

//! QVector of GLfloat
typedef QVector<GLfloat> GLfloatVector;

//! QVector of GLuint
typedef QVector<GLuint> GLuintVector;

//////////////////////////////////////////////////////////////////////
//! \class GLC_ExtendedGeomEngine
/*! \brief GLC_ExtendedGeomEngine : Specialized engine
 */

/*! GLC_ExtendedGeomEngine can handle mesh with Triangles, strips and fans
 */
//////////////////////////////////////////////////////////////////////
class GLC_ExtendedGeomEngine : public GLC_GeomEngine
{
public:

	//! Enum of VBO TYPE
	enum VboType
	{
		GLC_Vertex= 30,
		GLC_Normal,
		GLC_Texel,
		GLC_Color
	};

public:
	//! Default constructor
	GLC_ExtendedGeomEngine();

	//! Copy constructor
	GLC_ExtendedGeomEngine(const GLC_ExtendedGeomEngine&);

	virtual ~GLC_ExtendedGeomEngine();


//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the number of lod
	inline int numberOfLod()
	{return m_EngineLodList.size();}

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

	//! Return the Index Vector
	inline GLuintVector indexVector(const int i= 0) const
	{
		Q_ASSERT(i < m_EngineLodList.size());
		return m_EngineLodList.at(i)->indexVector();
	}

	//! Return the Index Vector handle
	inline GLuintVector* indexVectorHandle(const int i= 0) const
	{
		Q_ASSERT(i < m_EngineLodList.size());
		return m_EngineLodList.at(i)->indexVectorHandle();
	}

	//! Return the size of the triangles index Vector
	inline int indexVectorSize(const int i= 0) const
	{
		Q_ASSERT(i < m_EngineLodList.size());
		return m_EngineLodList.at(i)->indexVectorSize();
	}
	//! Return the specified LOD
	inline GLC_EngineLod* getLod(int index) const
	{
		return m_EngineLodList.value(index);
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Add a Lod to the engine
	inline void appendLod(double accuracy= 0.0)
	{m_EngineLodList.append(new GLC_EngineLod(accuracy));}

	//! The mesh wich use this engine is finished
	void finished();

	//! If the there is more than 2 LOD Swap the first and last
	void finishedLod();

	//! Clear the engine
	void clear();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Vbo creation
	void createVBOs();

	//! Ibo Usage
	bool useVBO(bool, GLC_ExtendedGeomEngine::VboType);

	//! Ibo Usage
	inline void useIBO(bool use, const int currentLod= 0)
	{
		if (use) m_EngineLodList.at(currentLod)->useIBO();
		else glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	//!
//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Vertex Position Vector
	GLfloatVector m_Positions;

	//! Vertex Normal Vector
	GLfloatVector m_Normals;

	//! Vertex Texture coordinate
	GLfloatVector m_Texels;

	//! Color index
	GLfloatVector m_Colors;

	//! Normals VBO ID
	GLuint m_NormalVboId;

	//! Texture VBO ID
	GLuint m_TexelVboId;

	//! Color VBO ID
	GLuint m_ColorVboId;

	//! The list of LOD
	QList<GLC_EngineLod*> m_EngineLodList;

	//! The size of Position and normal VBO
	int m_PositionSize;

	//! The size of texel VBO
	int m_TexelsSize;

	//! The size of Color VBO
	int m_ColorSize;

};

#endif /* GLC_EXTENDEDGEOMENGINE_H_ */
