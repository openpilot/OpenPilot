/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2010 Laurent Bauer
 Copyright (C) 2010 Laurent Ribon (laumaya@users.sourceforge.net)
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
//! \file glc_sphere.cpp implementation of the GLC_Sphere class.

#include "glc_sphere.h"

// Class chunk id
quint32 GLC_Sphere::m_ChunkId= 0xA710;

GLC_Sphere::GLC_Sphere(double radius)
: GLC_Mesh()
, m_Radius (radius)
, m_Discret(glc::GLC_POLYDISCRET)
, m_ThetaMin (0.0)
, m_ThetaMax(2 * glc::PI)
, m_PhiMin(-glc::PI / 2.0)
, m_PhiMax(glc::PI / 2.0)
{
	createMesh();
}


GLC_Sphere::GLC_Sphere(const GLC_Sphere & sphere)
:GLC_Mesh(sphere)
, m_Radius (sphere.m_Radius)
, m_Discret(sphere.m_Discret)
, m_ThetaMin (sphere.m_ThetaMin)
, m_ThetaMax(sphere.m_ThetaMax)
, m_PhiMin(sphere.m_PhiMin)
, m_PhiMax(sphere.m_PhiMax)
{
	createMesh();
}

GLC_Sphere::~GLC_Sphere()
{

}

GLC_Geometry* GLC_Sphere::clone() const
{
	return new GLC_Sphere (*this);
}

const GLC_BoundingBox& GLC_Sphere::boundingBox()
{
	if ( GLC_Mesh::isEmpty() )
	{
		createMesh();
	}
	return GLC_Mesh::boundingBox();
}

void GLC_Sphere::setRadius(double Radius)
{
	Q_ASSERT(Radius > 0.0);
	m_Radius= Radius;

	GLC_Mesh::clearMeshWireAndBoundingBox();
}


void GLC_Sphere::setDiscretion(int TargetDiscret)
{
	Q_ASSERT(TargetDiscret > 0);
	if (TargetDiscret != m_Discret)
	{
		m_Discret= TargetDiscret;
		if (m_Discret < 6) m_Discret= 6;

		GLC_Mesh::clearMeshWireAndBoundingBox();
	}
}

void GLC_Sphere::glDraw(const GLC_RenderProperties& renderProperties)
{
	if (GLC_Mesh::isEmpty())
	{
		createMesh();
	}

	GLC_Mesh::glDraw(renderProperties);
}

void GLC_Sphere::createMesh()
{

	Q_ASSERT(GLC_Mesh::isEmpty());

	GLfloatVector verticeFloat;
	GLfloatVector normalsFloat;
	GLfloatVector texelVector;

	int currentIndex=0;

	float wishedThetaStep= glc::PI / m_Discret;
	float thetaRange= m_ThetaMax-m_ThetaMin;
	int nbThetaSteps= (int) (thetaRange / wishedThetaStep) + 1 ;
	float thetaStep= thetaRange / nbThetaSteps;

	float wishedPhiStep= wishedThetaStep;
	float phiRange= m_PhiMax-m_PhiMin;
	int nbPhiSteps= (int) (phiRange / wishedPhiStep) + 1 ;
	float phiStep= phiRange / nbPhiSteps;

	float cost, sint, cosp, sinp, cospp, sinpp;
	float xi, yi, zi, xf, yf, zf;
	float theta= m_ThetaMin;
	float phi= m_PhiMin;

	GLfloatVector thetaMinWire;
	GLfloatVector thetaMaxWire;
	GLfloatVector phiMinWire;
	GLfloatVector phiMaxWire;

	GLC_Material* pMaterial;
	if (hasMaterial())
		pMaterial= this->firstMaterial();
	else
		pMaterial= new GLC_Material();

	// shaded face
	for (int p= 0; p < nbPhiSteps; ++p)
	{
		cosp= cos (phi);
		sinp= sin (phi);
		cospp= cos (phi + phiStep);
		sinpp= sin (phi + phiStep);

		zi = m_Radius * sinp;
		zf = m_Radius * sinpp;

		IndexList indexFace;

		theta = m_ThetaMin;
		int t;
		for (t= 0; t <= nbThetaSteps; ++t)
		{
			cost= cos( theta );
			sint= sin( theta );

			xi= m_Radius * cost * cosp;
			yi= m_Radius * sint * cosp;
			xf= m_Radius * cost * cospp;
			yf= m_Radius * sint * cospp;

			verticeFloat << xf << yf << zf << xi << yi << zi;
			normalsFloat << cost * cospp << sint * cospp << sinpp << cost * cosp << sint * cosp << sinp;
 			texelVector << static_cast<double>(t) * 1.0 / static_cast<double>(nbThetaSteps)
						<< static_cast<double>(p) * 1.0 / static_cast<double>(nbPhiSteps)
						<< static_cast<double>(t) * 1.0 / static_cast<double>(nbThetaSteps)
						<< static_cast<double>(p+1) * 1.0 / static_cast<double>(nbPhiSteps);

			indexFace << currentIndex + 2 * t << currentIndex + 2 * t + 1 ;
			theta+= thetaStep;

		}

		currentIndex+= 2 * t;
		addTrianglesStrip(pMaterial, indexFace);
		phi+= phiStep;
	}

	addVertice(verticeFloat);
	addNormals(normalsFloat);
	addTexels(texelVector);

	finish();
}


