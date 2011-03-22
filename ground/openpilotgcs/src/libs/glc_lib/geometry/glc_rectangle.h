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

//! \file glc_rectangle.h interface for the GLC_Rectangle class.

#ifndef GLC_RECTANGLE_H_
#define GLC_RECTANGLE_H_

#include "glc_mesh.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Rectangle
/*! \brief GLC_Rectangle : OpenGL flat 3D Rectangle*/

/*! An GLC_Rectangle is just a simple 3D Rectangle*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Rectangle : public GLC_Mesh
{

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_Rectangle();

	//! Construct a rectangle with the given lenght
	GLC_Rectangle(double l1, double l2);

	//! Construct a restangle with the given rectangle
	GLC_Rectangle(const GLC_Rectangle&);

	//! Destructor
	virtual ~GLC_Rectangle();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return length 1 of this rectangle
	inline double length1() const
	{ return m_L1;}

	//! Return length 2 of this rectangle
	inline double length2() const
	{ return m_L2;}

	//! Clone this rectangle
	virtual GLC_Geometry* clone() const;

	//! Return this rectangle bounding box
	virtual const GLC_BoundingBox& boundingBox(void);

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set this rectangle with the given lenght
	GLC_Rectangle& setRectangle(double l1, double l2);

	//! Set this rectangle length 1
	void setLength1(double l1);

	//! Set this rectangle length 2
	void setLength2(double l2);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:

	//! Virtual interface for OpenGL Geometry set up.
	/*! This Virtual function is implemented here.\n
	 *  Throw GLC_OpenGlException*/
	virtual void glDraw(const GLC_RenderProperties&);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Private services Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Create this rectangle mesh and wire
	void createMeshAndWire();
//@}


//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! The Rectangle length 1
	double m_L1;

	//! The Rectangle length 2
	double m_L2;

};

#endif /* GLC_RECTANGLE_H_ */
