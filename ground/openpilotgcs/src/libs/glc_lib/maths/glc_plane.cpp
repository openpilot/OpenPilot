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
//! \file glc_plane.cpp Implementation of the GLC_Plane class.

#include <QtDebug>
#include "glc_plane.h"

GLC_Plane::GLC_Plane()
{
	m_Eq[0]= 0.0;
	m_Eq[1]= 0.0;
	m_Eq[2]= 0.0;
	m_Eq[3]= 0.0;
}

GLC_Plane::GLC_Plane(double a, double b, double c, double d)
{
	m_Eq[0]= a;
	m_Eq[1]= b;
	m_Eq[2]= c;
	m_Eq[3]= d;

}

GLC_Plane::GLC_Plane(const GLC_Vector3d& normal, double d)
{
	m_Eq[0]= normal.x();
	m_Eq[1]= normal.y();
	m_Eq[2]= normal.z();
	m_Eq[3]= d;
}

GLC_Plane::GLC_Plane(const GLC_Vector3d& normal, const GLC_Point3d& point)
{
	m_Eq[0]= normal.x();
	m_Eq[1]= normal.y();
	m_Eq[2]= normal.z();
	m_Eq[3]= -normal * point;
}


GLC_Plane::GLC_Plane(const GLC_Point3d& p1, const GLC_Point3d& p2, const GLC_Point3d& p3)
{
	const GLC_Vector3d v1(p2 - p1);
	const GLC_Vector3d v2(p3 - p1);
	const GLC_Vector3d normal((v1 ^ v2).normalize());
	m_Eq[0]= normal.x();
	m_Eq[1]= normal.y();
	m_Eq[2]= normal.z();
	m_Eq[3]= -normal * p1;
}


GLC_Plane::GLC_Plane(const GLC_Plane& plane)
{
	memcpy(m_Eq, plane.m_Eq, sizeof(double) * 4);
}


GLC_Plane& GLC_Plane::operator=(const GLC_Plane& p)
{
	if ((this != &p) && (*this != p))
	{
		memcpy(m_Eq, p.m_Eq, sizeof(double) * 4);
	}
	return *this;
}


GLC_Plane::~GLC_Plane()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////


bool GLC_Plane::operator==(GLC_Plane p2) const
{
	GLC_Plane p1(*this);
	p1.normalize();
	p2.normalize();
	bool areEqual= qFuzzyCompare(p1.m_Eq[0], p2.m_Eq[0]);
	areEqual= areEqual && qFuzzyCompare(p1.m_Eq[1], p2.m_Eq[1]);
	areEqual= areEqual && qFuzzyCompare(p1.m_Eq[2], p2.m_Eq[2]);
	areEqual= areEqual && qFuzzyCompare(p1.m_Eq[3], p2.m_Eq[3]);

	return areEqual;
}

QString GLC_Plane::toString() const
{
	return QString::number(m_Eq[0]) + "x + " + QString::number(m_Eq[1]) + "y + " + QString::number(m_Eq[2]) + "z + " + QString::number(m_Eq[3]);
}
//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Normalize the plane
void GLC_Plane::normalize()
{
	const double invMag= 1.0 / sqrt(m_Eq[0] * m_Eq[0] + m_Eq[1] * m_Eq[1] + m_Eq[2] * m_Eq[2]);

	m_Eq[0]= m_Eq[0] * invMag;
	m_Eq[1]= m_Eq[1] * invMag;
	m_Eq[2]= m_Eq[2] * invMag;
	m_Eq[3]= m_Eq[3] * invMag;
}

GLC_Plane& GLC_Plane::setPlane(const GLC_Vector3d& normal, const GLC_Point3d& point)
{
	m_Eq[0]= normal.x();
	m_Eq[1]= normal.y();
	m_Eq[2]= normal.z();
	m_Eq[3]= -normal * point;

	return *this;
}
