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
//! \file glc_pointcloud.h interface for the GLC_PointCloud class.

#ifndef GLC_POINTCLOUD_H_
#define GLC_POINTCLOUD_H_

#include "glc_geometry.h"
#include "../maths/glc_vector3d.h"
#include "../maths/glc_vector3df.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_PointCloud
/*! \brief GLC_PointCloud : OpenGL 3D cloud of points*/

/*! An GLC_PointCloud is a group of points
 * All points of this class have the same color*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_PointCloud : public GLC_Geometry
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

public:
	//! Construct an empty cloud of points
	GLC_PointCloud();

	//! Copy constructor
	GLC_PointCloud(const GLC_PointCloud& pointCloud);

	//! Destructor
	virtual ~GLC_PointCloud();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the point cloud bounding box
	const GLC_BoundingBox& boundingBox();

	//! Return a copy of the geometry
	virtual GLC_Geometry* clone() const;

	//! Return true if this point cloud is empty
	inline bool isEmpty() const
	{return GLC_Geometry::m_WireData.isEmpty();}

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Add a point to this wire and returns its id if id are managed
	inline GLC_uint addPoint(const GLfloatVector& data)
	{return GLC_Geometry::m_WireData.addVerticeGroup(data);}

	//! Add the given list of points to this cloud and returns its id if id are managed
	GLC_uint addPoint(const QList<GLC_Point3d>& pointsList);

	//! Add the given list of points to this cloud and returns its id if id are managed
	GLC_uint addPoint(const QList<GLC_Point3df>& pointsList);

	//! Add Colors
	inline void addColors(const GLfloatVector& colors)
	{GLC_Geometry::m_WireData.addColors(colors);}

	//! Add Colors
	void addColors(const QList<QColor>& colors);

	//! Set this point cloud from the given point cloud and return a reference of this point cloud
	GLC_PointCloud& operator=(const GLC_PointCloud& pointcloud);

	//! Clear the content of this point cloud Data and makes it empty
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

#endif /* GLC_POINTCLOUD_H_ */
