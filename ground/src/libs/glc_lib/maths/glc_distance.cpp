/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Copyright (C) 2009 Pierre Soetewey
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

#include "glc_distance.h"
#include "../geometry/glc_extendedmesh.h"
#include <stdio.h>
#include <math.h>
#include <../PQP/PQP.h>

#define PI 3.14159265359
#define LISTS 0

// Default constructor
GLC_Distance::GLC_Distance()
: GLC_Object("DistMin")
, m_ListOfInstances1()
, m_ListOfInstances2()
, m_Point1()
, m_Point2()
, m_DistanceMini(0.0)
, m_RelativeError(0.0)
, m_AbsoluteError(0.0)
{

}

// Construct a distmin with 2 GLC_3DViewInstance
GLC_Distance::GLC_Distance(const GLC_3DViewInstance& instance1, const GLC_3DViewInstance& instance2)
: GLC_Object("DistMin")
, m_ListOfInstances1()
, m_ListOfInstances2()
, m_Point1()
, m_Point2()
, m_DistanceMini(0.0)
, m_RelativeError(0.0)
, m_AbsoluteError(0.0)
{
	m_ListOfInstances1.append(instance1);
	m_ListOfInstances2.append(instance2);
}

// Copy Constructor
GLC_Distance::GLC_Distance(const GLC_Distance& distance)
: GLC_Object(distance)
, m_ListOfInstances1(distance.m_ListOfInstances1)
, m_ListOfInstances2(distance.m_ListOfInstances2)
, m_Point1(distance.m_Point1)
, m_Point2(distance.m_Point2)
, m_DistanceMini(distance.m_DistanceMini)
, m_RelativeError(distance.m_RelativeError)
, m_AbsoluteError(distance.m_AbsoluteError)
{

}

GLC_Distance::~GLC_Distance()
{
	qDebug() << "GLC_Distance::~GLC_Distance()";
}

//////////////////////////////////////////////////////////////////////
// name Set Functions
//////////////////////////////////////////////////////////////////////

// Clear the 2 groups
void GLC_Distance::clear()
{
	m_ListOfInstances1.clear();
	m_ListOfInstances2.clear();
	m_Point1.setVect(GLC_Point4d());
	m_Point2.setVect(GLC_Point4d());
	m_DistanceMini= 0.0;
}

// Add instance in group 1
void GLC_Distance::addInstanceInGroup1(const GLC_3DViewInstance& instance)
{
	m_ListOfInstances1.append(instance);
}

// Add instances list in group 1
void GLC_Distance::addInstancesInGroup1(const QList<GLC_3DViewInstance>& instances)
{
	m_ListOfInstances1.append(instances);
}

// Add instance in group 1
void GLC_Distance::addInstanceInGroup2(const GLC_3DViewInstance& instance)
{
	m_ListOfInstances2.append(instance);
}

// Add instances list in group 1
void GLC_Distance::addInstancesInGroup2(const QList<GLC_3DViewInstance>& instances)
{
	m_ListOfInstances2.append(instances);
}

// Compute the minimum distance between the 2 groups
void GLC_Distance::computeMinimumDistance()
{
	DistanceResult distanceResult;
	if (not m_ListOfInstances1.isEmpty() and not m_ListOfInstances2.isEmpty())
	{
		distanceResult= minimumDistance(m_ListOfInstances1, m_ListOfInstances2);		
	}
	else
	{
		qDebug() << "a list is empty";
		distanceResult.m_Distance= -1.0;
		distanceResult.m_Point1= GLC_Point4d();
		distanceResult.m_Point2= GLC_Point4d();
	}

	m_DistanceMini= distanceResult.m_Distance;
	m_Point1= distanceResult.m_Point1;
	m_Point2= distanceResult.m_Point2;

}

//////////////////////////////////////////////////////////////////////
// private services functions
//////////////////////////////////////////////////////////////////////

// Return distance mini beween to instance
GLC_Distance::DistanceResult GLC_Distance::minimumDistance(QList<GLC_3DViewInstance> &GLC_3DViewList1, QList<GLC_3DViewInstance> &GLC_3DViewList2) const
{
	DistanceResult distanceResult;
	distanceResult.m_Distance= 0.0;
	
	PQP_Model m1, m2;
	GLC_3DViewInstance instance1;
	GLC_3DViewInstance instance2;
	GLC_Matrix4x4 instance1Matrix;
	GLC_Matrix4x4 instance2Matrix;
	
	m1.BeginModel();
	const int GLC_3DViewList1Size= GLC_3DViewList1.size();
	for(int i=0; i < GLC_3DViewList1Size; ++i){
	
		instance1= GLC_3DViewList1.at(i);
	
		//! Create the list ot the instance1 meshs
		QList<GLC_ExtendedMesh*> listOfMesh1;
		{
			const int size= instance1.numberOfGeometry();
			qDebug() << "Size = " << size;
			for (int i= 0; i < size; ++i)
			{
				GLC_ExtendedMesh* pMesh= dynamic_cast<GLC_ExtendedMesh*>(instance1.geomAt(i));
				if (NULL != pMesh) listOfMesh1.append(pMesh);
			}
		}
		if (listOfMesh1.isEmpty())
		{
			qDebug() << "listOfMesh1 empty";
			return distanceResult;
		}

		instance1Matrix= instance1.matrix();
		
		// Avoid scaling problem (useful?)
		//instance1Matrix= instance1Matrix.isomorphMatrix();
		
		// Add first mesh's triangles to the PQP model
		addMeshTrianglesToPQP(&m1, listOfMesh1, instance1Matrix);
	}
	m1.EndModel();
	
	m2.BeginModel();
	const int GLC_3DViewList2Size= GLC_3DViewList2.size();
	for(int i=0; i < GLC_3DViewList2Size; ++i){
	
		instance2= GLC_3DViewList2.at(i);
	
		//! Create the list ot the instance2 meshs
		QList<GLC_ExtendedMesh*> listOfMesh2;
		{
			const int size= instance2.numberOfGeometry();
			for (int i= 0; i < size; ++i)
			{
				GLC_ExtendedMesh* pMesh= dynamic_cast<GLC_ExtendedMesh*>(instance2.geomAt(i));
				if (NULL != pMesh) listOfMesh2.append(pMesh);
			}
		}
		if (listOfMesh2.isEmpty())
		{
			qDebug() << "listOfMesh2 empty";
			return distanceResult;
		}


		instance2Matrix= instance2.matrix();

		// Avoid scaling problem (useful?)
		//instance2Matrix= instance2Matrix.isomorphMatrix();
		
		// Add second mesh's triangles to the PQP model
		addMeshTrianglesToPQP(&m2, listOfMesh2, instance2Matrix);
	
	}
	m2.EndModel();
	
	// PQP Translation
	PQP_REAL T1[3];
	PQP_REAL T2[3];
	// PQP rotation
	PQP_REAL R1[3][3];
	PQP_REAL R2[3][3];

	// Null Vectors (no translation)
	T1[0]= 0;
	T1[1]= 0;
	T1[2]= 0;

	T2[0]= 0;
	T2[1]= 0;
	T2[2]= 0;

	// Unity Matrix (no rotation)
	R1[0][0]= 1;
	R1[1][0]= 0;
	R1[2][0]= 0;
	R1[0][1]= 0;
	R1[1][1]= 1;
	R1[2][1]= 0;
	R1[0][2]= 0;
	R1[1][2]= 0;
	R1[2][2]= 1;

	R2[0][0]= 1;
	R2[1][0]= 0;
	R2[2][0]= 0;
	R2[0][1]= 0;
	R2[1][1]= 1;
	R2[2][1]= 0;
	R2[0][2]= 0;
	R2[1][2]= 0;
	R2[2][2]= 1;

	PQP_DistanceResult dres;
	PQP_Distance(&dres,R1,T1,&m1,R2,T2,&m2,m_RelativeError, m_AbsoluteError);
	double distance = dres.Distance();

	const PQP_REAL * p1 = dres.P1();
	const PQP_REAL * p2 = dres.P2();

	GLC_Point4d point1(p1[0],p1[1],p1[2]);
	GLC_Point4d point2(p2[0],p2[1],p2[2]);

	distanceResult.m_Distance= distance;
	distanceResult.m_Point1= point1;
	distanceResult.m_Point2= point2;

	//qDebug() << "Distance " << distanceResult.m_Distance;
	return distanceResult;
}

void GLC_Distance::getPQPPoint(double &p0, double &p1, double &p2, const double x, const double y, const double z, const GLC_Matrix4x4& instanceMatrix) const
{
	GLC_Vector4d vector(x, y, z);
	vector= instanceMatrix * vector;
	p0= vector.X();
	p1= vector.Y();
	p2= vector.Z();
}

// Add mesh triangles to PQP model
void GLC_Distance::addMeshTrianglesToPQP(PQP_Model* pPQP_Model, const QList<GLC_ExtendedMesh*> listOfMeshes, const GLC_Matrix4x4& instanceMatrix) const
{
	int generalCount=0;
	
	// List of axis scaling values
	//QList<double> scaleFactors;
	//scaleFactors << instanceMatrix.scalingX() << instanceMatrix.scalingY() << instanceMatrix.scalingZ();
	//scaleFactors << 1.0 << 1.0 << 1.0;

	int i, j, k, l, size2, size3, size4, pos1, pos2, pos3;
	GLC_ExtendedMesh * pMesh=NULL;
	GLC_uint materialId;
	GLfloatVector positionVector;
	PQP_REAL p1[3], p2[3], p3[3];
	const int size= listOfMeshes.size();
	
	for (i= 0; i < size; ++i)
	{
		pMesh= listOfMeshes.at(i);
		positionVector= pMesh->positionVector();
		size2= pMesh->numberOfMaterials();
		for(j=0; j< size2;++j){

			materialId= pMesh->material(pMesh->materialIds().at(j))->id();
			if(pMesh->lodContainsMaterial(0, materialId)){

				if (pMesh->containsTriangles(0, materialId)){

					QVector<GLuint> index= pMesh->getTrianglesIndex(0, materialId);
					size3= index.size();
					for (k= 0; k < size3; k= k+3)
					{
						pos1= index.at(k)*3;
						pos2= index.at(k+1)*3;
						pos3= index.at(k+2)*3;
						
						getPQPPoint(p1[0], p1[1], p1[2], positionVector.at(pos1), positionVector.at(pos1+1), positionVector.at(pos1+2), instanceMatrix);
						getPQPPoint(p2[0], p2[1], p2[2], positionVector.at(pos2), positionVector.at(pos2+1), positionVector.at(pos2+2), instanceMatrix);
						getPQPPoint(p3[0], p3[1], p3[2], positionVector.at(pos3), positionVector.at(pos3+1), positionVector.at(pos3+2), instanceMatrix);
						
						pPQP_Model->AddTri(p1, p2, p3, generalCount);
						++generalCount;
					}
				}

				if (pMesh->containsStrips(0, materialId)){

					QList<QVector<GLuint> > index= pMesh->getStripsIndex(0, materialId);
					size3= index.size();
					for (k= 0; k < size3; ++k)
					{
						pos1= index.at(k).at(0)*3;
						pos2= index.at(k).at(1)*3;
						pos3= index.at(k).at(2)*3;
						
						getPQPPoint(p1[0], p1[1], p1[2], positionVector.at(pos1), positionVector.at(pos1+1), positionVector.at(pos1+2), instanceMatrix);
						getPQPPoint(p2[0], p2[1], p2[2], positionVector.at(pos2), positionVector.at(pos2+1), positionVector.at(pos2+2), instanceMatrix);
						getPQPPoint(p3[0], p3[1], p3[2], positionVector.at(pos3), positionVector.at(pos3+1), positionVector.at(pos3+2), instanceMatrix);
						
						pPQP_Model->AddTri(p1, p2, p3, generalCount);
						++generalCount;

						size4= index.at(k).size();
						for (l= 3; l < size4; ++l)
						{
							pos1= index.at(k).at(l)*3;
							pos2= index.at(k).at(l-1)*3;
							pos3= index.at(k).at(l-2)*3;
							
							getPQPPoint(p1[0], p1[1], p1[2], positionVector.at(pos1), positionVector.at(pos1+1), positionVector.at(pos1+2), instanceMatrix);
							getPQPPoint(p2[0], p2[1], p2[2], positionVector.at(pos2), positionVector.at(pos2+1), positionVector.at(pos2+2), instanceMatrix);
							getPQPPoint(p3[0], p3[1], p3[2], positionVector.at(pos3), positionVector.at(pos3+1), positionVector.at(pos3+2), instanceMatrix);
							
							pPQP_Model->AddTri(p1, p2, p3, generalCount);
							++generalCount;
						}
					}
				}

				if (pMesh->containsFans(0, materialId)){

					QList<QVector<GLuint> > index= pMesh->getFansIndex(0, materialId);
					size3= index.size();
					for (k= 0; k < size3; ++k)
					{
						pos1= index.at(k).at(0)*3;
						pos2= index.at(k).at(1)*3;
						pos3= index.at(k).at(2)*3;
						
						getPQPPoint(p1[0], p1[1], p1[2], positionVector.at(pos1), positionVector.at(pos1+1), positionVector.at(pos1+2), instanceMatrix);
						getPQPPoint(p2[0], p2[1], p2[2], positionVector.at(pos2), positionVector.at(pos2+1), positionVector.at(pos2+2), instanceMatrix);
						getPQPPoint(p3[0], p3[1], p3[2], positionVector.at(pos3), positionVector.at(pos3+1), positionVector.at(pos3+2), instanceMatrix);
						
						pPQP_Model->AddTri(p1, p2, p3, generalCount);
						++generalCount;

						size4= index.at(k).size();
						for (l= 3; l < size4; ++l)
						{
							pos2= index.at(k).at(l-1)*3;
							pos3= index.at(k).at(l)*3;
							
							getPQPPoint(p2[0], p2[1], p2[2], positionVector.at(pos2), positionVector.at(pos2+1), positionVector.at(pos2+2), instanceMatrix);
							getPQPPoint(p3[0], p3[1], p3[2], positionVector.at(pos3), positionVector.at(pos3+1), positionVector.at(pos3+2), instanceMatrix);
								
							pPQP_Model->AddTri(p1, p2, p3, generalCount);
							++generalCount;
						}
					}
				}
			}
		}
	}
}
