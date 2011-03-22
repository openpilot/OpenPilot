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

//! \file glc_boundingbox.h interface for the GLC_BoundingBox class.

#ifndef GLC_BOUNDINGBOX_
#define GLC_BOUNDINGBOX_

#include "maths/glc_vector3d.h"
#include "maths/glc_utils_maths.h"
#include "maths/glc_matrix4x4.h"
#include <QtDebug>
#include "glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_BoundingBox
/*! \brief GLC_BoundingBox : Geometry bounding box*/

/*! An GLC_BoundingBox is a non oriented bounding box
*/

//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_BoundingBox
{
	friend GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_BoundingBox &);
	friend GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_BoundingBox &);

//////////////////////////////////////////////////////////////////////
/*! @name Constructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct an empty bounding box
	GLC_BoundingBox();

	//! Construct a bounding box from  the given bounding box
	GLC_BoundingBox(const GLC_BoundingBox& boundingBox);

	//! Construct a bounding box from the given 3d point
	GLC_BoundingBox(const GLC_Point3d& lower, const GLC_Point3d& upper);

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return this class Chunk ID
	static quint32 chunckID();

	//! Return true if this bounding box is empty
	bool isEmpty(void) const
	{
		return m_IsEmpty;
	}

	//! Return true if the given 3d point intersect this bounding box
	bool intersect(const GLC_Point3d& point) const;

	//! Return true if the given bounding box intersect this bounding box
	inline bool intersect(const GLC_BoundingBox& boundingBox) const;

	//! Return true if the given 3d point intersect this bounding sphere of bounding box
	bool intersectBoundingSphere(const GLC_Point3d&) const;

	//! Return true if the given bounding sphere of bounding box intersect the bounding sphere of box bounding box
	bool intersectBoundingSphere(const GLC_BoundingBox&) const;

	//! Return the lower corner of this bounding box
	inline const GLC_Point3d& lowerCorner() const
	{return m_Lower;}

	//! Return the upper corner of this bounding box
	inline const GLC_Point3d& upperCorner() const
	{return m_Upper;}

	//! Return the center of this bounding box
	inline GLC_Point3d center() const;

	//! Return the radius of this bounding sphere of bounding box
	inline double boundingSphereRadius() const
	{return GLC_Vector3d(m_Lower - m_Upper).length() / 2.0;}

	//! Return true if this bounding box is equal of the given bounding box
	inline bool operator == (const GLC_BoundingBox& boundingBox);

	//! Return true if this bounding box is not equal of the given bounding box
	inline bool operator != (const GLC_BoundingBox& boundingBox)
	{return !(*this == boundingBox);}

	//! Return the length off this bounding box on x axis
	inline double xLength() const
	{return fabs(m_Upper.x() - m_Lower.x());}

	//! Return the length off this bounding box on y axis
	inline double yLength() const
	{return fabs(m_Upper.y() - m_Lower.y());}

	//! Return the length off this bounding box on z axis
	inline double zLength() const
	{return fabs(m_Upper.z() - m_Lower.z());}


//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Combine this bounding Box with the given 3d point and return a reference of this bounding box
	GLC_BoundingBox& combine(const GLC_Point3d& point);

	//! Combine this bounding Box with the given 3d point and return a reference of this bounding box
	GLC_BoundingBox& combine(const GLC_Point3df& point);

	//! Combine this bounding Box with the given bounding box and return a reference of this bounding box
	GLC_BoundingBox& combine(const GLC_BoundingBox& box);

	//! Transform this bounding Box with the given matrix and return a reference of this bounding box
	GLC_BoundingBox& transform(const GLC_Matrix4x4& matrix);

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Lower corner 3d point
	GLC_Point3d m_Lower;

	//! Upper corner 3d point
	GLC_Point3d m_Upper;

	//! Flag to know if this bounding box is empty
	bool m_IsEmpty;

	//! This class chunk id
	static quint32 m_ChunkId;
};

//! Non-member stream operator
GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_BoundingBox &);
GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_BoundingBox &);

// Return true if the given bounding box intersect this bounding box
bool GLC_BoundingBox::intersect(const GLC_BoundingBox& boundingBox) const
{
	// Distance between bounding box center
	GLC_Vector3d thisCenter= center();
	GLC_Vector3d otherCenter= boundingBox.center();
	const double distanceX= fabs(thisCenter.x() - otherCenter.x());
	const double distanceY= fabs(thisCenter.y() - otherCenter.y());
	const double distanceZ= fabs(thisCenter.z() - otherCenter.z());

	bool intersect= distanceX < ((xLength() + boundingBox.xLength()) * 0.5);
	intersect= intersect && (distanceY < ((yLength() + boundingBox.yLength()) * 0.5));
	intersect= intersect && (distanceZ < ((zLength() + boundingBox.zLength()) * 0.5));
	return intersect;
}

bool GLC_BoundingBox::operator == (const GLC_BoundingBox& box)
{
	return (m_Lower == box.m_Lower) && (m_Upper == box.m_Upper);
}

GLC_Point3d GLC_BoundingBox::center(void) const
{
	GLC_Vector3d vectResult = (m_Lower + m_Upper) * 0.5;
	return vectResult;
}

#endif /*GLC_BOUNDINGBOX_*/
