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

//! \file glc_boundingbox.h interface for the GLC_BoundingBox class.

#ifndef GLC_BOUNDINGBOX_
#define GLC_BOUNDINGBOX_

#include "maths/glc_vector4d.h"
#include "maths/glc_utils_maths.h"
#include "maths/glc_matrix4x4.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_BoundingBox
/*! \brief GLC_BoundingBox : Geometry bounding box*/

/*! An GLC_BoundingBox is a non oriented bounding box
*/

//////////////////////////////////////////////////////////////////////

class GLC_BoundingBox
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_BoundingBox();

	//! Copy constructor
	GLC_BoundingBox(const GLC_BoundingBox& boundingBox);

	//! Constructor with 2 points.
	GLC_BoundingBox(const GLC_Point4d& lower, const GLC_Point4d& upper);

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////

	//! Get the empty state of the bounding Box
	bool isEmpty(void) const
	{
		return m_IsEmpty;
	}

	//! Test if a point is in the bounding Box
	bool intersect(const GLC_Point4d& point) const;

	//! Test if a point is in the bounding Sphere
	bool intersectBoundingSphere(const GLC_Point4d&) const;

	//! Return the max distance between a point and a corner of the bounding box
	//double maxDistance(const GLC_Vector4d& point) const;

	//! Get the lower corner of the bounding box
	GLC_Point4d lowerCorner(void) const;

	//! Get the upper corner of the bounding box
	GLC_Point4d upperCorner(void) const;

	//! Get the center of the bounding box
	GLC_Point4d center(void) const;

	//! Return the boundingSphere Radius
	inline double boundingSphereRadius() const
	{return GLC_Vector4d(m_Lower - m_Upper).norm() / 2.0;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Combine the bounding Box with new geometry point
	GLC_BoundingBox& combine(const GLC_Point4d& point);

	//! Combine the bounding Box with new geometry point
	GLC_BoundingBox& combine(const GLC_Point3d& point);

	//! Combine the bounding Box with new geometry point
	GLC_BoundingBox& combine(const GLC_Point3df& point);

	//! Combine the bounding Box with another bounding box
	GLC_BoundingBox& combine(const GLC_BoundingBox& box);

	//! Transform the bounding Box
	GLC_BoundingBox& transform(const GLC_Matrix4x4& matrix);


//@}


//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	GLC_Point4d m_Lower;
	GLC_Point4d m_Upper;
	bool m_IsEmpty;

};
#endif /*GLC_BOUNDINGBOX_*/
