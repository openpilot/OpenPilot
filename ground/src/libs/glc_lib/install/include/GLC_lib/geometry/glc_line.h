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

#ifndef GLC_LINE_H_
#define GLC_LINE_H_

#include "glc_vbogeom.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Line
/*! \brief GLC_Line : OpenGL 3D Line*/

/*! An GLC_Line is just a simple 3D Line*/
//////////////////////////////////////////////////////////////////////

class GLC_Line : public GLC_VboGeom
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct an GLC_Line by to point
	GLC_Line(const GLC_Point4d &, const GLC_Point4d &);

	//! Copy constructor
	GLC_Line(const GLC_Line&);

	//!Default dstructor
	virtual ~GLC_Line();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Return the point1 coordinate
	inline GLC_Point4d point1(void) const
	{return m_Point1;}

	//! Return the point2 coordinate
	inline GLC_Point4d point2(void) const
	{return m_Point2;}

	//! Return the point bounding box
	virtual GLC_BoundingBox& boundingBox(void);

	//! Return a copy of the geometry
	virtual GLC_VboGeom* clone() const;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set Line coordinate by 4D point
	void setCoordinate(const GLC_Point4d &, const GLC_Point4d &);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////

private:
	//! Virtual interface for OpenGL Geometry set up.
	/*! This Virtual function is implemented here.\n*/
	virtual void glDraw(bool transparent= false);

//@}

//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////

private:
	//! First point of the line
	GLC_Point4d m_Point1;

	//! First point of the line
	GLC_Point4d m_Point2;

};

#endif /* GLC_LINE_H_ */
