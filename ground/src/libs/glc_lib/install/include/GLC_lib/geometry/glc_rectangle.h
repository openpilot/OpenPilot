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

//! \file glc_rectangle.h interface for the GLC_Rectangle class.

#ifndef GLC_RECTANGLE_H_
#define GLC_RECTANGLE_H_

#include "glc_extendedgeomengine.h"
#include "glc_vbogeom.h"
#include "glc_primitivegroup.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Rectangle
/*! \brief GLC_Rectangle : OpenGL flat 3D Rectangle*/

/*! An GLC_Rectangle is just a simple 3D Rectangle which use VBO*/
//////////////////////////////////////////////////////////////////////
class GLC_Rectangle : public GLC_VboGeom
{

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_Rectangle();

	//! Complete constructor
	GLC_Rectangle(const GLC_Vector4d&, double, double);

	//! Copy constructor
	GLC_Rectangle(const GLC_Rectangle&);

	//! Destructor
	virtual ~GLC_Rectangle();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return length 1
	inline double length1() const
	{ return m_L1;}

	//! Return length 2
	inline double length2() const
	{ return m_L2;}

	//! Return the rectangle normal
	inline GLC_Vector4d normal() const
	{ return m_Normal;}

	//! clone the rectangle
	virtual GLC_VboGeom* clone() const;

	//! return the rectangle bounding box
	virtual GLC_BoundingBox& boundingBox(void);

	//! Get number of faces
	virtual unsigned int numberOfFaces() const;

	//! Get number of vertex
	virtual unsigned int numberOfVertex() const;

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set the rectangle
	GLC_Rectangle& setRectangle(const GLC_Vector4d&, double, double);

	//! Set the rectangle normal
	inline void setNormal(const GLC_Vector4d& normal)
	{
		m_Normal= normal;
		// Invalid the geometry
		m_GeometryIsValid = false;
	}
	//! Set rectangle length 1
	inline void setLength1(double value)
	{
		m_L1= value;
		// Invalid the geometry
		m_GeometryIsValid = false;
	}

	//! Set rectangle length 2
	inline void setLength2(double value)
	{
		m_L2= value;
		// Invalid the geometry
		m_GeometryIsValid = false;
	}

	//! Copy vertex list in a vector list for Vertex Array Use
	void finished();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:

	//! Virtual interface for OpenGL Geometry set up.
	/*! This Virtual function is implemented here.\n
	 *  Throw GLC_OpenGlException*/
	virtual void glDraw(bool transparent= false);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Private services Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Create VBO and IBO
	void createVbos();

	//! Finish VBO mesh
	void finishVbo();

	//! Finish non Vbo mesh
	void finishNonVbo();

	//! Create rectangle mesh
	void createRectangleMesh();
//@}


//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! the list of Hash table of primitive group
	GLC_PrimitiveGroup m_PrimitiveGroup;

	//! Selection state
	bool m_IsSelected;

	//! Geom engine
	GLC_ExtendedGeomEngine m_ExtendedGeomEngine;

	//! Normal Vector of the rectangle
	GLC_Vector4d m_Normal;

	//! The Rectangle length 1
	double m_L1;

	//! The Rectangle length 2
	double m_L2;

};

#endif /* GLC_RECTANGLE_H_ */
