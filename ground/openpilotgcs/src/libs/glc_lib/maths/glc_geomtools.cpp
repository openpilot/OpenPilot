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

//! \file glc_geomtools.cpp implementation of geometry function.

#include "glc_geomtools.h"
#include "glc_matrix4x4.h"

#include <QtGlobal>


double glc::comparedPrecision= glc::defaultPrecision;

//////////////////////////////////////////////////////////////////////
//Tools Functions
//////////////////////////////////////////////////////////////////////

// Test if a polygon is convex
bool glc::polygon2DIsConvex(const QList<GLC_Point2d>& vertices)
{
	bool isConvex= true;
	const int size= vertices.size();
	if (vertices.size() > 3)
	{
		GLC_Point2d s0(vertices.at(0));
		GLC_Point2d s1(vertices.at(1));
		GLC_Point2d s2(vertices.at(2));
		const bool openAngle= ((s1.getX() - s0.getX()) * (s2.getY() - s0.getY()) - (s2.getX() - s0.getX()) * (s1.getY() - s0.getY())) < 0.0;

		int i= 3;
		while ((i < size) && isConvex)
		{
			s0= s1;
			s1= s2;
			s2= vertices.at(i);
			isConvex= openAngle == (((s1.getX() - s0.getX()) * (s2.getY() - s0.getY()) - (s2.getX() - s0.getX()) * (s1.getY() - s0.getY())) < 0.0);
			++i;
		}
	}

	return isConvex;
}

bool glc::polygonIsConvex(QList<GLuint>* pIndexList, const QList<float>& bulkList)
{
	bool isConvex= true;
	const int size= pIndexList->size();
	GLuint currentIndex;
	GLC_Vector3d v0;
	GLC_Vector3d v1;
	int i= 0;
	while ((i < size) && isConvex)
	{
		currentIndex= pIndexList->at(i);
		v0.setVect(bulkList.at(currentIndex * 3), bulkList.at(currentIndex * 3 + 1), bulkList.at(currentIndex * 3 + 2));
		currentIndex= pIndexList->at((i + 1) % size);
		v1.setVect(bulkList.at(currentIndex * 3), bulkList.at(currentIndex * 3 + 1), bulkList.at(currentIndex * 3 + 2));
		isConvex= (v0.angleWithVect(v1) < glc::PI);
		++i;
	}
	return isConvex;
}
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
		if ((s < 0.0) || (s > 1.0))
		{
			// Intersection of lines is not a point on segment s1p1 + s * DO
			return result; // Return empty QVector
		}
		const double t= (E ^ D0) / kross;

		if ((t < 0.0) || (t > 1.0))
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
		if ((s < 0.0) || (s > 1.0))
		{
			// Intersection of lines is not a point on segment s1p1 + s * DO
			return false;
		}
		const double t= (E ^ D0) / kross;

		if ((t < 0.0) || (t > 1.0))
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

		if ((t < 0.0) || (t > 1.0))
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
	if (u1 < v0 || u0 > v1) return result; // Return empty QVector

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
		return (((diff ^ edgeR) < 0.0) && ((diff ^ edgeL) > 0.0));
	}
	else
	{
		// Vertex is reflex
		return (((diff ^ edgeR) < 0.0) || ((diff ^ edgeL) > 0.0));
	}
}

// Return true if the segment is a polygon diagonal
bool glc::isDiagonal(const QList<GLC_Point2d>& polygon, const int i0, const int i1)
{
	const int size= polygon.size();
	int iM= (i0 - 1) % size;
	if (iM < 0) iM= size - 1;
	int iP= (i0 + 1) % size;

	if (!segmentInCone(polygon[i0], polygon[i1], polygon[iM], polygon[iP]))
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
		while (j2 < size && !isIntersect)
		{
			if(j2 != j0 && j3 != j1)
			{
				if (isIntersectedRaySegment(moy, (perp + moy), polygon[j2], polygon[j3]))
					isIntersect= true;
			}
			j3= j2;
			++j2;
		}
		if(!isIntersect) return false;
		j1= j0;
		++j0;
	}

	return true;

}

// Triangulate a no convex polygon
void glc::triangulatePolygon(QList<GLuint>* pIndexList, const QList<float>& bulkList)
{
	int size= pIndexList->size();
	if (polygonIsConvex(pIndexList, bulkList))
	{
		QList<GLuint> indexList(*pIndexList);
		pIndexList->clear();
		for (int i= 0; i < size - 2; ++i)
		{
			pIndexList->append(indexList.at(0));
			pIndexList->append(indexList.at(i + 1));
			pIndexList->append(indexList.at(i + 2));
		}
	}
	else
	{
		// Get the polygon vertice
		QList<GLC_Point3d> originPoints;
		QHash<int, int> indexMap;

		QList<int> face;
		GLC_Point3d currentPoint;
		int delta= 0;
		for (int i= 0; i < size; ++i)
		{
			const int currentIndex= pIndexList->at(i);
			currentPoint= GLC_Point3d(bulkList.at(currentIndex * 3), bulkList.at(currentIndex * 3 + 1), bulkList.at(currentIndex * 3 + 2));
			if (!originPoints.contains(currentPoint))
			{
				originPoints.append(GLC_Point3d(bulkList.at(currentIndex * 3), bulkList.at(currentIndex * 3 + 1), bulkList.at(currentIndex * 3 + 2)));
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
			const GLC_Point3d point1(originPoints[0]);
			const GLC_Point3d point2(originPoints[1]);
			const GLC_Point3d point3(originPoints[2]);

			const GLC_Vector3d edge1(point2 - point1);
			const GLC_Vector3d edge2(point3 - point2);

			GLC_Vector3d polygonPlaneNormal(edge1 ^ edge2);
			polygonPlaneNormal.normalize();

			// Create the transformation matrix
			GLC_Matrix4x4 transformation;

			GLC_Vector3d rotationAxis(polygonPlaneNormal ^ Z_AXIS);
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

			if(!faceIsCounterclockwise)
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

}

bool glc::lineIntersectPlane(const GLC_Line3d& line, const GLC_Plane& plane, GLC_Point3d* pPoint)
{
	const GLC_Vector3d n= plane.normal();
	const GLC_Point3d p= line.startingPoint();
	const GLC_Vector3d d= line.direction();

	const double denominator= d * n;
	if (qFuzzyCompare(fabs(denominator), 0.0))
	{
		qDebug() << " glc::lineIntersectPlane : Line parallel to the plane";
		// The line is parallel to the plane
		return false;
	}
	else
	{
		// The line intersect the plane by one point
		const double t= -((n * p) + plane.coefD()) / denominator;
		(*pPoint)= p + (t * d);

		return true;
	}
}

GLC_Point3d glc::project(const GLC_Point3d& point, const GLC_Line3d& line)
{
	const GLC_Vector3d lineDirection(line.direction().normalize());
	double t= lineDirection * (point - line.startingPoint());
	GLC_Point3d projectedPoint= line.startingPoint() + (t * lineDirection);
	return projectedPoint;
}

double glc::pointLineDistance(const GLC_Point3d& point, const GLC_Line3d& line)
{
	const GLC_Vector3d lineDirection(line.direction().normalize());
	double t= lineDirection * (point - line.startingPoint());
	GLC_Point3d projectedPoint= line.startingPoint() + (t * lineDirection);
	return (point - projectedPoint).length();
}

bool glc::pointsAreCollinear(const GLC_Point3d& p1, const GLC_Point3d& p2, const GLC_Point3d& p3)
{
	bool subject= false;
	if (compare(p1, p2) || compare(p1, p3) || compare(p2, p3))
	{
		subject= true;
	}
	else
	{
		GLC_Vector3d p1p2= (p2 - p1).setLength(1.0);
		GLC_Vector3d p2p3= (p3 - p2).setLength(1.0);
		subject= (compare(p1p2, p2p3)  || compare(p1p2, p2p3.inverted()));
	}
	return subject;
}

bool glc::compare(double p1, double p2)
{
	return qAbs(p1 - p2) <= comparedPrecision;
}

bool glc::compareAngle(double p1, double p2)
{
	const double anglePrecision= toRadian(comparedPrecision);
	return qAbs(p1 - p2) <= anglePrecision;
}

bool glc::compare(const GLC_Vector3d& v1, const GLC_Vector3d& v2)
{
	bool compareResult= (qAbs(v1.x() - v2.x()) <= comparedPrecision);
	compareResult= compareResult && (qAbs(v1.y() - v2.y()) <= comparedPrecision);
	compareResult= compareResult && (qAbs(v1.z() - v2.z()) <= comparedPrecision);

	return compareResult;
}

bool glc::compare(const GLC_Vector2d& v1, const GLC_Vector2d& v2)
{
	bool compareResult= (qAbs(v1.getX() - v2.getX()) <= comparedPrecision);
	return compareResult && (qAbs(v1.getY() - v2.getY()) <= comparedPrecision);
}

bool glc::compare(const QPointF& v1, const QPointF& v2)
{
	bool compareResult= (qAbs(v1.x() - v2.x()) <= comparedPrecision);
	return compareResult && (qAbs(v1.y() - v2.y()) <= comparedPrecision);
}

bool glc::pointInPolygon(const GLC_Point2d& point, const QList<GLC_Point2d>& polygon)
{
	const int polygonSize= polygon.size();
	bool inside= false;
	int i= 0;
	int j= polygonSize - 1;

	while (i < polygonSize)
	{
		const GLC_Point2d point0= polygon.at(i);
		const GLC_Point2d point1= polygon.at(j);
		if (point.getY() < point1.getY())
		{
			//point1 above ray
			if (point0.getY() <= point.getY())
			{
				//point2 on or below ray
				const double val1= (point.getY() - point0.getY()) * (point1.getX() - point0.getX());
				const double val2= (point.getX() - point0.getX()) * (point1.getY() - point0.getY());
				if (val1 > val2) inside= !inside;
			}
		}
		else if (point.getY() < point0.getY())
		{
			// point 1 on or below ray, point0 above ray
			const double val1= (point.getY() - point0.getY()) * (point1.getX() - point0.getX());
			const double val2= (point.getX() - point0.getX()) * (point1.getY() - point0.getY());
			if (val1 < val2) inside= !inside;
		}
		j= i;
		++i;
	}
	return inside;
}

double glc::zeroTo2PIAngle(double angle)
{
	if (qFuzzyCompare(fabs(angle), glc::PI))
	{
		angle= glc::PI;
	}
	else if (angle < 0)
	{
		angle= (2.0 * glc::PI) + angle;
	}
	return angle;
}
