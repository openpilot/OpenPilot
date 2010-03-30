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

//! \file glc_enginelod.h interface for the GLC_EngineLod class.

#ifndef GLC_ENGINELOD_H_
#define GLC_ENGINELOD_H_

#include <QVector>
#include "../glc_ext.h"

class GLC_EngineLod
{
public:

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_EngineLod(double accuracy);

	//! Copy constructor
	GLC_EngineLod(const GLC_EngineLod&);

	//!Destructor
	virtual ~GLC_EngineLod();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the accuracy of the LOD
	inline double accuracy() const
	{return m_Accuracy;}

	//! Return the Triangle Index Vector
	QVector<GLuint> indexVector() const;

	//! Return the Triangle Index Vector handle
	inline QVector<GLuint>* indexVectorHandle()
	{ return &m_IboVector;}

	//! Return the size of the triangles index Vector
	inline int indexVectorSize() const
	{return m_IboVector.size();}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! The mesh wich use this lod is finished
	inline void finished()
	{
		m_IndexSize= m_IboVector.size();
		m_IboVector.clear();
	}
	//! Set accuracy of the LOD
	inline void setAccuracy(const double& accuracy)
	{m_Accuracy= accuracy;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! IBO creation
	inline void createIBO()
	{
		if (0 == m_IboId and not m_IboVector.isEmpty())
		{
			glGenBuffers(1, &m_IboId);
		}
	}

	//! Ibo Usage
	inline void useIBO()
	{glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IboId);}

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:

	//! The accuracy of the load
	double m_Accuracy;

	//! The IBO ID
	GLuint m_IboId;

	//! The IBO Vector
	QVector<GLuint> m_IboVector;

	//! The Index vector size
	int m_IndexSize;

};

#endif /* GLC_ENGINELOD_H_ */
