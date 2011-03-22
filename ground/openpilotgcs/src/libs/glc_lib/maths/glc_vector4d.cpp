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


//! \file glc_vector4d.cpp implementation of the GLC_Vector4d class.
#include "glc_vector4d.h"
#include <QtDebug>

using namespace glc;
//////////////////////////////////////////////////////////////////////
// Operators overload
//////////////////////////////////////////////////////////////////////


// Overload dot product "^" operator
GLC_Vector4d GLC_Vector4d::operator^ (const GLC_Vector4d &Vect) const
{
	GLC_Vector4d VectResult;
	VectResult.vector[0]= vector[1] * Vect.vector[2] - vector[2] * Vect.vector[1];
	VectResult.vector[1]= vector[2] * Vect.vector[0] - vector[0] * Vect.vector[2];
	VectResult.vector[2]= vector[0] * Vect.vector[1] - vector[1] * Vect.vector[0];

	return VectResult;
}

// Overload equality "==" operator
bool GLC_Vector4d::operator == (const GLC_Vector4d &Vect) const
{
	bool bResult= qFuzzyCompare(vector[0], Vect.vector[0]);
	bResult= bResult && qFuzzyCompare(vector[1], Vect.vector[1]);
	bResult= bResult && qFuzzyCompare(vector[2], Vect.vector[2]);
	bResult= bResult && qFuzzyCompare(vector[3], Vect.vector[3]);

	return bResult;
}
//////////////////////////////////////////////////////////////////////
// Set Function
//////////////////////////////////////////////////////////////////////

GLC_Vector4d& GLC_Vector4d::setW(const double &dW)
{
	if (dW != 0)
	{
		const double invDW= 1.0 / dW;
		vector[0]*= invDW;
		vector[1]*= invDW;
		vector[2]*= invDW;
		vector[3]= 1.0;		// For calculation, W = 1.
	}
	return *this;
}

GLC_Vector4d& GLC_Vector4d::setVect(const double &dX, const double &dY,
	const double &dZ, const double &dW)
{
	if ((dW == 1.0) || (dW <= 0.0))
	{
		vector[0]= dX;
		vector[1]= dY;
		vector[2]= dZ;
	}
	else
	{
		const double invDW= 1.0 / dW;
		vector[0]= dX * invDW;
		vector[1]= dY * invDW;
		vector[2]= dZ * invDW;
	}

	vector[3]= 1.0;		// For calculation, W = 1.

	return *this;
}

GLC_Vector4d& GLC_Vector4d::setNormal(const double &Norme)
{
	const double dNormeCur= sqrt( vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);

	if (dNormeCur != 0)
	{
		const double Coef = Norme / dNormeCur;

		vector[0] = vector[0] * Coef;
		vector[1] = vector[1] * Coef;
		vector[2] = vector[2] * Coef;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
// Get Function
//////////////////////////////////////////////////////////////////////

// Return the Angle with another vector
double GLC_Vector4d::getAngleWithVect(GLC_Vector4d Vect) const
{
	GLC_Vector4d ThisVect(*this);
	ThisVect.setNormal(1);
	Vect.setNormal(1);
	// Rotation axis
	const GLC_Vector4d VectAxeRot(ThisVect ^ Vect);
	// Check if the rotation axis vector is null
	if (!VectAxeRot.isNull())
	{
		return acos(ThisVect * Vect);
	}
	else return 0.0;
}

// return the vector string
QString GLC_Vector4d::toString() const
{
	QString result("[");

	result+= QString::number(vector[0]) + QString(" , ");
	result+= QString::number(vector[1]) + QString(" , ");
	result+= QString::number(vector[2]) + QString(" , ");
	result+= QString::number(vector[3]) + QString("]");

	return result;
}

// return the 2D vector
GLC_Vector2d GLC_Vector4d::toVector2d(const GLC_Vector4d& mask) const
{
	double x;
	double y;
	if (mask.vector[0] == 0.0)
	{
		x= vector[0];
		if (mask.vector[1] == 0.0)
			y= vector[1];
		else
			y= vector[2];

	}
	else
	{
		x= vector[1];
		y= vector[2];

	}
	return GLC_Vector2d(x, y);
}

//////////////////////////////////////////////////////////////////////
// Services private function
//////////////////////////////////////////////////////////////////////

// Normalize Vector w <- 1
void GLC_Vector4d::normalizeW(void)
{
	if (fabs(vector[3]) > 0.00001)
	{
		const double invW= 1.0 / vector[3];
		vector[0]*= invW;
		vector[1]*= invW;
		vector[2]*= invW;
	}
	vector[3]= 1.0;
}

// Non-member stream operator
QDataStream &operator<<(QDataStream &stream, const GLC_Vector4d &vector)
{
	stream << vector.X() << vector.Y() << vector.Z() << vector.W();
	return stream;
}
QDataStream &operator>>(QDataStream &stream, GLC_Vector4d &vector)
{
	double x, y, z, w;
	stream >> x >> y >> z >> w;
	vector.setVect(x, y, z, w);
	return stream;
}


