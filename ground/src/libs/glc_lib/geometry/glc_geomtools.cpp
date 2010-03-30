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

//! \file glc_geomtools.cpp implementation of geometry function.

#include "glc_geomtools.h"
#include "../maths/glc_matrix4x4.h"

//////////////////////////////////////////////////////////////////////
//Tools Functions
//////////////////////////////////////////////////////////////////////

// test if a polygon of mesh is convex
/*
bool glc::polygonIsConvex(const VertexList* pVertexList)
{
	const int max= pVertexList->size();
	if (max < 4) return true;

	GLC_Vector4d vertex1((*pVertexList)[0].x, (*pVertexList)[0].y, (*pVertexList)[0].z);
	GLC_Vector4d vertex2((*pVertexList)[1].x, (*pVertexList)[1].y, (*pVertexList)[1].z);
	GLC_Vector4d edge1(vertex2 - vertex1);

	vertex1= vertex2;
	vertex2= GLC_Vector4d((*pVertexList)[2].x, (*pVertexList)[2].y, (*pVertexList)[2].z);
	GLC_Vector4d edge2(vertex2 - vertex1);

	const bool direction= (edge1 ^ edge2).Z() >= 0.0;

	for (int i= 3; i < max; ++i)
	{
		edge1= edge2;
		vertex1= vertex2;
		vertex2= GLC_Vector4d((*pVertexList)[i].x, (*pVertexList)[i].y, (*pVertexList)[i].z);
		edge2= vertex2 - vertex1;
		if (((edge1 ^ edge2).Z() >= 0.0) != direction)
			return false;
	}
	// The last edge with the first
	edge1= edge2;
	vertex1= vertex2;
	vertex2= GLC_Vector4d((*pVertexList)[0].x, (*pVertexList)[0].y, (*pVertexList)[0].z);
	edge2= vertex2 - vertex1;
	if (((edge1 ^ edge2).Z() >= 0.0) != direction)
		return false;

	// Ok, the polygon is convexe
	return true;
}
*/
// find intersection between two 2D segments
QVector<GLC_Point2d> glc::findIntersection(const GLC_Point2d& s1p1, const GLC_Point2d& s1p2, const GLC_Point2d& s2p1, const GLC_Point2d& s2p2)
{
	const GLC_Vector2d D0= s1p2 - s1p1;
	const GLC_Vector2d D1= s2p2 - s2p1;
	// The QVector of result points
	QVector<GLC_Point2d> result;

	const GLC_Vector2d E(s2p1 - s1p1);
	double kross= D0 ^ D1;
	double sqrKross= kross * kross;
	const double sqrLen0= D0.getX() * D0.getX() + D0.getY() * D0.getY();
	const double sqrLen1= D1.getX() * D1.getX() + D1.getY() * D1.getY();
	// Test if the line are nor parallel
	if (sqrKross > (EPSILON * sqrLen0 * sqrLen1))
	{
		const double s= (E ^ D1) / kross;
		if ((s < 0.0) or (s > 1.0))
		{
			// Intersection of lines is not a point on segment s1p1 + s * DO
			return result; // Return empty QVector
		}
		const double t= (E ^ D0) / kross;

		if ((t < 0.0) or (t > 1.0))
		{
			// Intersection of lines is not a point on segment s2p1 + t * D1
			return result; // Return empty QVector
		}

		// Intersection of lines is a point on each segment
		result << (s1p1 + (D0 * s));
		return result; // Return a QVector of One Point
	}

	// Lines of the segments are parallel
	const double sqrLenE= E.getX() * E.getX() + E.getY() * E.getY();
	kross= E ^ D0;
	sqrKross= kross * kross;
	if (sqrKross > (EPSILON * sqrLen0 * sqrLenE))
	{
		// Lines of the segments are different
		return result; // Return empty QVector
	}

	// Lines of the segments are the same. Need to test for overlap of segments.
	const double s0= (D0 * E) / sqrLen0;
	const double s1= (D0 * D1) / sqrLen0;
	const double sMin= qMin(s0, s1);
	const double sMax= qMax(s0, s1);
	QVector<double> overlaps= findIntersection(0.0, 1.0, sMin, sMax);
	const int iMax= overlaps.size();
	for (int i= 0; i < iMax; ++i)
	{
		result.append(s1p1 + (D0 * overlaps[i]));
	}
	return result;
}

// return true if there is an intersection between 2 segments
bool glc::isIntersected(const GLC_Point2d& s1p1, const GLC_Point2d& s1p2, const GLC_Point2d& s2p1, const GLC_Point2d& s2p2)
{
	const GLC_Vector2d D0= s1p2 - s1p1;
	const GLC_Vector2d D1= s2p2 - s2p1;

	const GLC_Vector2d E(s2p1 - s1p1);
	double kross= D0 ^ D1;
	double sqrKross= kross * kross;
	const double sqrLen0= D0.getX() * D0.getX() + D0.getY() * D0.getY();
	const double sqrLen1= D1.getX() * D1.getX() + D1.getY() * D1.getY();
	// Test if the line are nor parallel
	if (sqrKross > (EPSILON * sqrLen0 * sqrLen1))
	{
		const double s= (E ^ D1) / kross;
		if ((s < 0.0) or (s > 1.0))
		{
			// Intersection of lines is not a point on segment s1p1 + s * DO
			return false;
		}
		const double t= (E ^ D0) / kross;

		if ((t < 0.0) or (t > 1.0))
		{
			// Intersection of lines is not a point on segment s2p1 + t * D1
			return false;
		}

		// Intersection of lines is a point on each segment
		return true;
	}

	// Lines of the segments are parallel
	const double sqrLenE= E.getX() * E.getX() + E.getY() * E.getY();
	kross= E ^ D0;
	sqrKross= kross * kross;
	if (sqrKross > (EPSILON * sqrLen0 * sqrLenE))
	{
		// Lines of the segments are different
		return false;
	}

	// Lines of the segments are the same. Need to test for overlap of segments.
	const double s0= (D0 * E) / sqrLen0;
	const double s1= s0 + (D0 * D1) / sqrLen0;
	const double sMin= qMin(s0, s1);
	const double sMax= qMax(s0, s1);
	if (findIntersection(0.0, 1.0, sMin, sMax).size() == 0) return false; else return true;

}

// return true if there is an intersection between a ray and a segment
bool glc::isIntersectedRaySegment(const GLC_Point2d& s1p1, const GLC_Vector2d& s1p2, const GLC_Point2d& s2p1, const GLC_Point2d& s2p2)
{
	const GLC_Vector2d D0= s1p2 - s1p1;
	const GLC_Vector2d D1= s2p2 - s2p1;

	const GLC_Vector2d E(s2p1 - s1p1);
	double kross= D0 ^ D1;
	double sqrKross= kross * kross;
	const double sqrLen0= D0.getX() * D0.getX() + D0.getY() * D0.getY();
	const double sqrLen1= D1.getX() * D1.getX() + D1.getY() * D1.getY();
	// Test if the line are nor parallel
	if (sqrKross > (EPSILON * sqrLen0 * sqrLen1))
	{
		const double s= (E ^ D1) / kross;
		if ((s < 0.0))
		{
			// Intersection of lines is not a point on segment s1p1 + s * DO
			return false;
		}
		const double t= (E ^ D0) / kross;

		if ((t < 0.0) or (t > 1.0))
		{
			// Intersection of lines is not a point on segment s2p1 + t * D1
			return false;
		}

		// Intersection of lines is a point on each segment
		return true;
	}

	// Lines of the segments are parallel
	const double sqrLenE= E.getX() * E.getX() + E.getY() * E.getY();
	kross= E ^ D0;
	sqrKross= kross * kross;
	if (sqrKross > (EPSILON * sqrLen0 * sqrLenE))
	{
		// Lines of are different
		return false;
	}
	else return true;

}

// find intersection of two intervals [u0, u1] and [v0, v1]
QVector<double> glc::findIntersection(const double& u0, const double& u1, const double& v0, const double& v1)
{
	//Q_ASSERT((u0 < u1) and (v0 < v1));
	QVector<double> result;
	if (u1 < v0 or u0 > v1) return result; // Return empty QVector

	if (u1 > v0)
	{
		if (u0 < v1)
		{
			if (u0 < v0) result.append(v0); else result.append(u0);
			if (u1 > v1) result.append(v1); else result.append(u1);
			return result;
		}
		else // u0 == v1
		{
			result.append(u0);
			return result;
		}
	}
	else // u1 == v0
	{
		result.append(u1);
		return result;
	}
}

// return true if the segment is in polygon cone
bool glc::segmentInCone(const GLC_Point2d& V0, const GLC_Point2d& V1, const GLC_Point2d& VM, const GLC_Point2d& VP)
{
	// assert: VM, V0, VP are not collinear
	const GLC_Vector2d diff(V1 - V0);
	const GLC_Vector2d edgeL(VM - V0);
	const GLC_Vector2d edgeR(VP - V0);
	if ((edgeR ^ edgeL) > 0)
	{
		// Vertex is convex
		return (((diff ^ edgeR) < 0.0) and ((diff ^ edgeL) > 0.0));
	}
	else
	{
		// Vertex is reflex
		return (((diff ^ edgeR) < 0.0) or ((diff ^ edgeL) > 0.0));
	}
}

// Return true if the segment is a polygon diagonal
bool glc::isDiagonal(const QList<GLC_Point2d>& polygon, const int i0, const int i1)
{
	const int size= polygon.size();
	int iM= (i0 - 1) % size;
	if (iM < 0) iM= size - 1;
	int iP= (i0 + 1) % size;

	if (not segmentInCone(polygon[i0], polygon[i1], polygon[iM], polygon[iP]))
	{
		return false;
	}

	int j0= 0;
	int j1= size - 1;
	// test segment <polygon[i0], polygon[i1]> to see if it is a diagonal
	while (j0 < size)
	{
		if (j0 != i0 && j0 != i1 && j1 != i0 && j1 != i1)
		{
			if (isIntersected(polygon[i0], polygon[i1], polygon[j0], polygon[j1]))
				return false;
		}

		j1= j0;
		++j0;
	}

	return true;
}

// Triangulate a polygon
void glc::triangulate(QList<GLC_Point2d>& polygon, QList<int>& index, QList<int>& tList)
{
	const int size= polygon.size();
	if (size == 3)
	{
		tList << index[0] << index[1] << index[2];
		return;
	}
	int i0= 0;
	int i1= 1;
	int i2= 2;
	while(i0 < size)
	{
		if (isDiagonal(polygon, i0, i2))
		{
			// Add the triangle before removing the ear.
			tList << index[i0] << index[i1] << index[i2];
			// Remove the ear from polygon and index lists
			polygon.removeAt(i1);
			index.removeAt(i1);
			// recurse to the new polygon
			triangulate(polygon, index, tList);
			return;
		}
		++i0;
		i1= (i1 + 1) % size;
		i2= (i2 + 1) % size;
	}
}

// return true if the polygon is couterclockwise ordered
bool glc::isCounterclockwiseOrdered(const QList<GLC_Point2d>& polygon)
{
	const int size= polygon.size();
	int j0= 0;
	int j1= size - 1;
	// test segment <polygon[i0], polygon[i1]> to see if it is a diagonal
	while (j0 < size)
	{
		GLC_Vector2d perp((polygon[j0] - polygon[j1]).perp());
		int j2= 0;
		int j3= size - 1;
		bool isIntersect= false;
		// Application of perp vector
		GLC_Point2d moy((polygon[j0] + polygon[j1]) * 0.5);
		while (j2 < size and not isIntersect)
		{
			if(j2 != j0 and j3 != j1)
			{
				if (isIntersectedRaySegment(moy, (perp + moy), polygon[j2], polygon[j3]))
					isIntersect= true;
			}
			j3= j2;
			++j2;
		}
		if(not isIntersect) return false;
		j1= j0;
		++j0;
	}

	return true;

}

// Triangulate a no convex polygon
void glc::triangulatePolygon(QList<GLuint>* pIndexList, const QList<float>& bulkList)
{
	// Get the polygon vertice
	QList<GLC_Point4d> originPoints;
	QHash<int, int> indexMap;
	int size= pIndexList->size();
	QList<int> face;
	GLC_Point4d currentPoint;
	int delta= 0;
	for (int i= 0; i < size; ++i)
	{
		const int currentIndex= pIndexList->at(i);
		currentPoint= GLC_Point4d(bulkList.at(currentIndex * 3), bulkList.at(currentIndex * 3 + 1), bulkList.at(currentIndex * 3 + 2));
		if (not originPoints.contains(currentPoint))
		{
			originPoints.append(GLC_Point4d(bulkList.at(currentIndex * 3), bulkList.at(currentIndex * 3 + 1), bulkList.at(currentIndex * 3 + 2)));
			indexMap.insert(i - delta, currentIndex);
			face.append(i - delta);
		}
		else
		{
			qDebug() << "Multi points";
			++delta;
		}
	}
	// Values of PindexList must be reset
	pIndexList->clear();

	// Update size
	size= size - delta;

	// Check new size
	if (size < 3) return;

	//-------------- Change frame to mach polygon plane
		// Compute face normal
		const GLC_Point4d point1(originPoints[0]);
		const GLC_Point4d point2(originPoints[1]);
		const GLC_Point4d point3(originPoints[2]);

		const GLC_Vector4d edge1(point2 - point1);
		const GLC_Vector4d edge2(point3 - point2);

		GLC_Vector4d polygonPlaneNormal(edge1 ^ edge2);
		polygonPlaneNormal.setNormal(1.0);

		// Create the transformation matrix
		GLC_Matrix4x4 transformation;

		GLC_Vector4d rotationAxis(polygonPlaneNormal ^ Z_AXIS);
		if (!rotationAxis.isNull())
		{
			const double angle= acos(polygonPlaneNormal * Z_AXIS);
			transformation.setMatRot(rotationAxis, angle);
		}

		QList<GLC_Point2d> polygon;
		// Transform polygon vertexs
		for (int i=0; i < size; ++i)
		{
			originPoints[i]= transformation * originPoints[i];
			// Create 2d vector
			polygon << originPoints[i].toVector2d(Z_AXIS);
		}
		// Create the index
		QList<int> index= face;

		QList<int> tList;
		const bool faceIsCounterclockwise= isCounterclockwiseOrdered(polygon);

		if(not faceIsCounterclockwise)
		{
			//qDebug() << "face Is Not Counterclockwise";
			const int max= size / 2;
			for (int i= 0; i < max; ++i)
			{
				polygon.swap(i, size - 1 -i);
				int temp= face[i];
				face[i]= face[size - 1 - i];
				face[size - 1 - i]= temp;
			}
		}
		triangulate(polygon, index, tList);
		size= tList.size();
		for (int i= 0; i < size; i+= 3)
		{
			// Avoid normal problem
			if (faceIsCounterclockwise)
			{
				pIndexList->append(indexMap.value(face[tList[i]]));
				pIndexList->append(indexMap.value(face[tList[i + 1]]));
				pIndexList->append(indexMap.value(face[tList[i + 2]]));
			}
			else
			{
				pIndexList->append(indexMap.value(face[tList[i + 2]]));
				pIndexList->append(indexMap.value(face[tList[i + 1]]));
				pIndexList->append(indexMap.value(face[tList[i]]));
			}
		}
		Q_ASSERT(size == pIndexList->size());
}





