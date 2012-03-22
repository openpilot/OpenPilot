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

#ifndef GLC_LINE_H_
#define GLC_LINE_H_

#include "glc_polylines.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Line
/*! \brief GLC_Line : OpenGL 3D Line*/

/*! An GLC_Line is just a simple renderable 3D Line*/
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_Line : public GLC_Polylines
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct an GLC_Line by to point
	GLC_Line(const GLC_Point3d &, const GLC_Point3d &);

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
	inline GLC_Point3d point1(void) const
	{return m_Point1;}

	//! Return the point2 coordinate
	inline GLC_Point3d point2(void) const
	{return m_Point2;}

	//! Return the point bounding box
	virtual const GLC_BoundingBox& boundingBox(void);

	//! Return a copy of the geometry
	virtual GLC_Geometry* clone() const;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set Line coordinate by 3D point
	void setCoordinate(const GLC_Point3d &point1, const GLC_Point3d &point2);

	//! Clear the content of this line Data and makes it empty
	inline void clear()
	{GLC_Polylines::clear();}

	//! Set this line from the given line and return a reference of this line
	GLC_Line& operator=(const GLC_Line& line);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////

private:
	//! Virtual interface for OpenGL Geometry set up.
	/*! This Virtual function is implemented here.\n*/
	virtual void glDraw(const GLC_RenderProperties&);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Private services Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Create the wire
	void createWire();

//@}

//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////

private:
	//! First point of the line
	GLC_Point3d m_Point1;

	//! First point of the line
	GLC_Point3d m_Point2;

};

#endif /* GLC_LINE_H_ */
