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
//! \file glc_frustum.cpp Implementation of the GLC_Frustum class.

#include "glc_frustum.h"
#include "glc_viewport.h"

GLC_Frustum::GLC_Frustum()
: m_PlaneList()
, m_PreviousMatrix()
{
	for (int i= 0; i < 6; ++i)
	{
		m_PlaneList.append(GLC_Plane());
	}
}

GLC_Frustum::GLC_Frustum(const GLC_Frustum& frustum)
: m_PlaneList(frustum.m_PlaneList)
, m_PreviousMatrix(frustum.m_PreviousMatrix)
{

}

GLC_Frustum::~GLC_Frustum()
{

}

GLC_Frustum::Localisation GLC_Frustum::localizeBoundingBox(const GLC_BoundingBox& box) const
{
	const GLC_Point3d center= box.center();
	const double radius= box.boundingSphereRadius();

	return localizeSphere(center, radius);
}

GLC_Frustum::Localisation GLC_Frustum::localizeSphere(const GLC_Point3d& center, double radius) const
{
	GLC_Frustum::Localisation localisationResult= InFrustum;

	int i= 0;
	bool continu= true;
	while (continu && (i < 6))
	{
		//qDebug() << "Localisation of plane " << i;
		localisationResult= static_cast<GLC_Frustum::Localisation>(localisationResult | localizeSphereToPlane(center, radius, m_PlaneList.at(i)));
		continu= (localisationResult != GLC_Frustum::OutFrustum);
		++i;
	}

	return localisationResult;
}

GLC_Frustum::Localisation GLC_Frustum::localizeSphereToPlane(const GLC_Point3d& center, double radius, const GLC_Plane& plane) const
{
	GLC_Frustum::Localisation localisationResult;
	const double signedDistance= plane.distanceToPoint(center);
	const double distance= fabs(signedDistance);
	if (distance > radius)
	{
		if (signedDistance > 0) localisationResult= GLC_Frustum::InFrustum;
		else localisationResult= GLC_Frustum::OutFrustum;
	}
	else
	{
		localisationResult= GLC_Frustum::IntersectFrustum;
	}

	return localisationResult;
}

bool GLC_Frustum::update(const GLC_Matrix4x4& compMatrix)
{

	// Test if the frustum change
	if (compMatrix == m_PreviousMatrix)
	{
		//qDebug() << "No change in frustum";
		return false;
	}
	else
	{
		m_PreviousMatrix= compMatrix;
		// Left plane
		m_PlaneList[LeftPlane].setA(compMatrix.getData()[3] + compMatrix.getData()[0]);
		m_PlaneList[LeftPlane].setB(compMatrix.getData()[7] + compMatrix.getData()[4]);
		m_PlaneList[LeftPlane].setC(compMatrix.getData()[11] + compMatrix.getData()[8]);
		m_PlaneList[LeftPlane].setD(compMatrix.getData()[15] + compMatrix.getData()[12]);
		m_PlaneList[LeftPlane].normalize();

		// Right plane
		m_PlaneList[RightPlane].setA(compMatrix.getData()[3] - compMatrix.getData()[0]);
		m_PlaneList[RightPlane].setB(compMatrix.getData()[7] - compMatrix.getData()[4]);
		m_PlaneList[RightPlane].setC(compMatrix.getData()[11] - compMatrix.getData()[8]);
		m_PlaneList[RightPlane].setD(compMatrix.getData()[15] - compMatrix.getData()[12]);
		m_PlaneList[RightPlane].normalize();

		//Top plane
		m_PlaneList[TopPlane].setA(compMatrix.getData()[3] + compMatrix.getData()[1]);
		m_PlaneList[TopPlane].setB(compMatrix.getData()[7] + compMatrix.getData()[5]);
		m_PlaneList[TopPlane].setC(compMatrix.getData()[11] + compMatrix.getData()[9]);
		m_PlaneList[TopPlane].setD(compMatrix.getData()[15] + compMatrix.getData()[13]);
		m_PlaneList[TopPlane].normalize();

		//Bottom plane
		m_PlaneList[BottomPlane].setA(compMatrix.getData()[3] - compMatrix.getData()[1]);
		m_PlaneList[BottomPlane].setB(compMatrix.getData()[7] - compMatrix.getData()[5]);
		m_PlaneList[BottomPlane].setC(compMatrix.getData()[11] - compMatrix.getData()[9]);
		m_PlaneList[BottomPlane].setD(compMatrix.getData()[15] - compMatrix.getData()[13]);
		m_PlaneList[BottomPlane].normalize();

		//Near plane
		m_PlaneList[NearPlane].setA(compMatrix.getData()[3] + compMatrix.getData()[2]);
		m_PlaneList[NearPlane].setB(compMatrix.getData()[7] + compMatrix.getData()[6]);
		m_PlaneList[NearPlane].setC(compMatrix.getData()[11] + compMatrix.getData()[10]);
		m_PlaneList[NearPlane].setD(compMatrix.getData()[15] + compMatrix.getData()[14]);
		m_PlaneList[NearPlane].normalize();

		//Far plane
		m_PlaneList[FarPlane].setA(compMatrix.getData()[3] - compMatrix.getData()[2]);
		m_PlaneList[FarPlane].setB(compMatrix.getData()[7] - compMatrix.getData()[6]);
		m_PlaneList[FarPlane].setC(compMatrix.getData()[11] - compMatrix.getData()[10]);
		m_PlaneList[FarPlane].setD(compMatrix.getData()[15] - compMatrix.getData()[14]);
		m_PlaneList[FarPlane].normalize();
		return true;
	}
}
