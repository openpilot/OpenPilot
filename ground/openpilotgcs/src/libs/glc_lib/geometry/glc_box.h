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

//! \file glc_box.h interface for the GLC_Box class.

#ifndef GLC_BOX_H_
#define GLC_BOX_H_


#include "glc_mesh.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Box
/*! \brief GLC_Box : OpenGL Box*/

/*! An GLC_Box is a polygonal geometry\n
 *  It's a rectangular parallelepiped box centred at (0, 0, 0)*/


//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_Box : public GLC_Mesh
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct an GLC_Box
	/*! By default, discretion is set to #GLC_POLYDISCRET*/
	GLC_Box(double, double, double);

	//! Copy constructor
	GLC_Box(const GLC_Box&);

	//! Destructor
	virtual ~GLC_Box();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Get X length
	inline double getLgX(void) const
	{return m_dLgX;}

	//! Get Y length
	inline double getLgY(void) const
	{return m_dLgY;}

	//! Get Z length
	inline double getLgZ(void) const
	{return m_dLgZ;}

	//! return the box bounding box
	virtual const GLC_BoundingBox& boundingBox(void);

	//! Return a copy of the geometry
	virtual GLC_Geometry* clone() const;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Set X length
	/*! This Function invalid OpenGL display list
	 * LgX must be > 0*/
	void setLgX(double LgX);

	//! Set Y length
	/*! This Function invalid OpenGL display list
	 * LgY must be > 0*/
	void setLgY(double LgY);

	//! Set Z length
	/*! This Function invalid OpenGL display list
	 * LgZ must be > 0*/
	void setLgZ(double LgZ);
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
	//! Create the box mesh
	void createMeshAndWire();

	//! Create the wire of the mesh
	void createWire();

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:

	//! X Length
	double m_dLgX;

	//! Y Length
	double m_dLgY;

	//! Z Length
	double m_dLgZ;
};
#endif //GLC_BOX_H_
