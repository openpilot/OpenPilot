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
//! \file glc_polyline.h interface for the GLC_Polylines class.

#ifndef GLC_POLYLINES_H_
#define GLC_POLYLINES_H_

#include "glc_geometry.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Polylines
/*! \brief GLC_Polylines : OpenGL 3D Polylines*/

/*! An GLC_Polylines is a group of wire polyline
 * All polylines of this class have the same color*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Polylines : public GLC_Geometry
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct an empty polylines
	GLC_Polylines();

	//! Copy constructor
	GLC_Polylines(const GLC_Polylines& polyline);

	//! Destructor
	virtual ~GLC_Polylines();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the polylines bounding box
	const GLC_BoundingBox& boundingBox();

	//! Return a copy of the geometry
	virtual GLC_Geometry* clone() const;

	//! Return true if this polylines is empty
	inline bool isEmpty() const
	{return GLC_Geometry::m_WireData.isEmpty();}


//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Add a Polyline to this polylines and returns its id if id are managed
	inline GLC_uint addPolyline(const GLfloatVector& data)
	{return GLC_Geometry::m_WireData.addVerticeGroup(data);}

	//! Add polyline with the given list of points to this polylines and returns its id if id are managed
	GLC_uint addPolyline(const QList<GLC_Point3d>& pointsList);

	//! Add polyline with the given list of points to this polylines and returns its id if id are managed
	GLC_uint addPolyline(const QList<GLC_Point3df>& pointsList);

	//! Set this polylines from the given polylines and return a reference of this polylines
	GLC_Polylines& operator=(const GLC_Polylines& polyline);

	//! Clear the content of this polylines Data and makes it empty
	inline void clear()
	{GLC_Geometry::m_WireData.clear();}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
protected:

	//! Virtual interface for OpenGL Geometry set up.
	/*! This Virtual function is implemented here.\n
	 *  Throw GLC_OpenGlException*/
	virtual void glDraw(const GLC_RenderProperties&);

//@}

};

#endif /* GLC_POLYLINES_H_ */
