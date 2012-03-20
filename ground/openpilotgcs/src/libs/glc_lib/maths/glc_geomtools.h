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

//! \file glc_geomtools.h declaration of geometry tools functions

#ifndef GLC_GEOMTOOLS_H_
#define GLC_GEOMTOOLS_H_

#include <QVector>
#include <QList>
#include <QPointF>

#include "glc_vector3d.h"
#include "glc_line3d.h"
#include "glc_plane.h"

#include "../geometry/glc_mesh.h"

#include "../glc_config.h"

namespace glc
{
	const double defaultPrecision= 0.01;
	extern double comparedPrecision;
//////////////////////////////////////////////////////////////////////
/*! \name Tools Functions*/
//@{
//////////////////////////////////////////////////////////////////////
	//! test if the given 2D polygon is convex
	GLC_LIB_EXPORT bool polygon2DIsConvex(const QList<GLC_Point2d>& vertices);

	//! Test if the given 3d polygon is convex
	GLC_LIB_EXPORT bool polygonIsConvex(QList<GLuint>* pIndexList, const QList<float>& bulkList);

	//! find intersection between two 2D segments
	/*! Return the intersection as QVector of GLC_Point2d
	 *  - Empty QVector if there is no intersection
	 *  - Qvector size 1 if there is a unique intersection
	 *  - Qvector size 2 if the segement overlap*/
	GLC_LIB_EXPORT QVector<GLC_Point2d> findIntersection(const GLC_Point2d&, const GLC_Point2d&, const GLC_Point2d&, const GLC_Point2d&);

	//! Return true if there is an intersection between 2 segments
	GLC_LIB_EXPORT bool isIntersected(const GLC_Point2d&, const GLC_Point2d&, const GLC_Point2d&, const GLC_Point2d&);

	//! return true if there is an intersection between a ray and a segment
	GLC_LIB_EXPORT bool isIntersectedRaySegment(const GLC_Point2d&, const GLC_Vector2d&, const GLC_Point2d&, const GLC_Point2d&);

	//! Find intersection of two intervals [u0, u1] and [v0, v1]
	/*! Return the intersection as QVector of GLC_Vector2d
	 *  - Empty QVector if there is no intersection
	 *  - Qvector size 1 if there is a unique intersection
	 *  - Qvector size 2 if the segement overlap*/
	GLC_LIB_EXPORT QVector<double> findIntersection(const double&, const double&, const double&, const double&);

	//! Return true if the segment is in polygon cone
	GLC_LIB_EXPORT bool segmentInCone(const GLC_Point2d&, const GLC_Point2d&, const GLC_Point2d&, const GLC_Point2d&);

	//! Return true if the segment is a polygon diagonal
	GLC_LIB_EXPORT bool isDiagonal(const QList<GLC_Point2d>&, const int, const int);

	//! Triangulate a polygon
	GLC_LIB_EXPORT void triangulate(QList<GLC_Point2d>&, QList<int>&, QList<int>&);

	//! Return true if the polygon is couterclockwise ordered
	GLC_LIB_EXPORT bool isCounterclockwiseOrdered(const QList<GLC_Point2d>&);

	//! Triangulate a polygon return true if the polygon is convex
	/*! If the polygon is convex the returned index is a fan*/
	GLC_LIB_EXPORT void triangulatePolygon(QList<GLuint>*, const QList<float>&);

	//! Return true if the given 3d line intersect the given plane
	/*! If there is an intersection point is set to the given 3d point
	 *  If the line lie on the plane this method return false*/
	GLC_LIB_EXPORT bool lineIntersectPlane(const GLC_Line3d& line, const GLC_Plane& plane, GLC_Point3d* pPoint);

	//! Return the projected point on the given line form the given point
	GLC_LIB_EXPORT GLC_Point3d project(const GLC_Point3d& point, const GLC_Line3d& line);

	//! Return the midpoint of the two given points
	inline GLC_Point3d midPoint(const GLC_Point3d& point1, const GLC_Point3d point2)
	{return point1 + (point2 - point1) * 0.5;}

	//! Return the perpendicular 2D vector of the given 2D vector
	inline GLC_Vector2d perpVector(const GLC_Vector2d& vect)
	{return GLC_Vector2d(-vect.getY(), vect.getX());}

	//! Return the distance between the given point and line
	GLC_LIB_EXPORT double pointLineDistance(const GLC_Point3d& point, const GLC_Line3d& line);

	//! Return true if the given 3 points are collinear
	GLC_LIB_EXPORT bool pointsAreCollinear(const GLC_Point3d& p1, const GLC_Point3d& p2, const GLC_Point3d& p3);

	GLC_LIB_EXPORT bool compare(double p1, double p2);

	GLC_LIB_EXPORT bool compareAngle(double p1, double p2);

	GLC_LIB_EXPORT bool compare(const GLC_Vector3d& v1, const GLC_Vector3d& v2);

	GLC_LIB_EXPORT bool compare(const GLC_Vector2d& v1, const GLC_Vector2d& v2);

	GLC_LIB_EXPORT bool compare(const QPointF& v1, const QPointF& v2);

	//! Return true if the given 2d point is inside the given polygon
	GLC_LIB_EXPORT bool pointInPolygon(const GLC_Point2d& point, const QList<GLC_Point2d>& polygon);

	//! Return the angle from 0 to 2PI from an given angle from -PI to PI
	GLC_LIB_EXPORT double zeroTo2PIAngle(double angle);

//@}

};

#endif /*GLC_GEOMTOOLS_H_*/
