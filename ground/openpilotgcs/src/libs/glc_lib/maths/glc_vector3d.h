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

//! \file glc_vector3d.h interface for the GLC_Vector3d class.

#ifndef GLC_VECTOR3D_H_
#define GLC_VECTOR3D_H_

#include <QDataStream>

#include "glc_utils_maths.h"
#include "glc_vector3df.h"
#include "glc_vector2d.h"
#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Vector3d
/*! \brief GLC_Vector3d is a 3 dimensions Vector*/

/*! GLC_Vector3d is used to represent 3D vectors in 3D space coordinate.
 * */
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_Vector3d
{
	friend class GLC_Vector4d;
	friend class GLC_Matrix4x4;

	//! Overload unary "-" operator
	inline friend GLC_Vector3d operator - (const GLC_Vector3d &Vect)
	{return GLC_Vector3d(-Vect.m_Vector[0], -Vect.m_Vector[1], -Vect.m_Vector[2]);}

	//! Overload scalar operator
	inline friend GLC_Vector3d operator*(double s, const GLC_Vector3d &v)
	{return GLC_Vector3d(s * v.m_Vector[0], s * v.m_Vector[1], s * v.m_Vector[2]);}

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	/*!  Value is set to
	* \n X = 0.0
	* \n Y =  0.0
	* \n Z =  0.0
	*/
	inline GLC_Vector3d();

	//! Standard constructor with coordinate: (x, y, z)
	inline GLC_Vector3d(double x, double y, double z);

	//! Construct a 3d vector from another 3d vector
	inline GLC_Vector3d(const GLC_Vector3d &vector)
	{memcpy(m_Vector, vector.m_Vector, sizeof(double) * 3);}

	//! Construct a 3d vector from another 3d float vector
	inline GLC_Vector3d(const GLC_Vector3df &vector);

	//! Construct a 3d vector from a 2d float vector
	inline GLC_Vector3d(const GLC_Vector2d &vector);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the x coordinate of this vector
	inline double x() const
	{return m_Vector[0];}

	//! Return the y coordinate of this vector
	inline double y() const
	{return m_Vector[1];}

	//! Return the z coordinate of this vector
	inline double z() const
	{return m_Vector[2];}

	//! Return a const pointer to this vector data
	inline const double *data() const
	{return m_Vector;}

	//! Return true if this vector is null
	inline bool isNull() const
	{return (m_Vector[0] == 0.0f) && (m_Vector[1] == 0.0f) && (m_Vector[2] == 0.0f);}

	//! Return the length of this vector
	inline double length() const
	{return sqrt(m_Vector[0] * m_Vector[0] + m_Vector[1] * m_Vector[1] + m_Vector[2] * m_Vector[2]);}

	//! Return the 2D vector specified by the given mask vector
	/*! retrieve component corresponding to mask vector NULL component*/
	inline GLC_Vector2d toVector2d(const GLC_Vector3d& mask) const;

	//! Return the Angle from this vector to the given vector (from 0 to PI)
	inline double angleWithVect(GLC_Vector3d Vect) const;

	//! Return the signed angle from this vector to th given vector with the given direction (from 0 to -PI and 0 to PI)
	inline double signedAngleWithVect(GLC_Vector3d Vect, const GLC_Vector3d& dir) const;

	//! Return the float 3D vector from this vector
	inline GLC_Vector3df toVector3df() const
	{return GLC_Vector3df(static_cast<float>(m_Vector[0]), static_cast<float>(m_Vector[1]), static_cast<float>(m_Vector[2]));}

	//! Return the string of this vector
	inline QString toString() const;

	//! Return the inverted vector of this vector
	inline GLC_Vector3d inverted() const
	{return GLC_Vector3d(*this).invert();}

//@}

//////////////////////////////////////////////////////////////////////
/*! @name Operator Overload */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the Addition of this vector to the given vector
	inline GLC_Vector3d operator + (const GLC_Vector3d &vector) const
	{return GLC_Vector3d(m_Vector[0] + vector.m_Vector[0], m_Vector[1] + vector.m_Vector[1], m_Vector[2] + vector.m_Vector[2]);}

	//! Copy the given vector to this vector and return a reference to this vector
	inline GLC_Vector3d& operator = (const GLC_Vector3d &vector)
	{
		if (this != &vector) memcpy(m_Vector, vector.m_Vector, sizeof(double) * 3);
		return *this;
	}

	//! Copy the given float vector to this vector and return a reference to this vector
	inline GLC_Vector3d& operator = (const GLC_Vector3df &);

	//! Add this vector to the given vector and return a reference to this vector
	inline GLC_Vector3d& operator += (const GLC_Vector3d &vector)
	{
		*this= *this + vector;
		return *this;
	}

	//! Return the substracts of the given vector to this vector
	inline GLC_Vector3d operator - (const GLC_Vector3d &Vect) const
	{return GLC_Vector3d(m_Vector[0] - Vect.m_Vector[0], m_Vector[1] - Vect.m_Vector[1], m_Vector[2] - Vect.m_Vector[2]);}

	//! Substracts the given vector to this vector and return a reference to this vector
	GLC_Vector3d& operator -= (const GLC_Vector3d &Vect)
	{
		*this= *this - Vect;
		return *this;
	}

	//! Return the cross product of this vector to the given vector
	inline GLC_Vector3d operator ^ (const GLC_Vector3d &vector) const;

	//! Return the scalar product of this vector to the given vector
	inline double operator * (const GLC_Vector3d &Vect) const
	{return m_Vector[0] * Vect.m_Vector[0] + m_Vector[1] * Vect.m_Vector[1] + m_Vector[2] * Vect.m_Vector[2];}

	//! Return the scalar product of this vector to the given scalar
	inline GLC_Vector3d operator * (double Scalaire) const
	{return GLC_Vector3d(m_Vector[0] * Scalaire, m_Vector[1] * Scalaire, m_Vector[2] * Scalaire);}


	//! Return true if this vector is fuzzyequal to the given vector
	inline bool operator == (const GLC_Vector3d &vector) const;

	//! Return true if this vector is > to the given vector
	inline bool operator > (const GLC_Vector3d &vector) const;

	//! Return true if this vector is < to the given vector
	inline bool operator < (const GLC_Vector3d &vector) const;

	//! Return false if this vector is fuzzyequal to the given vector
	inline bool operator != (const GLC_Vector3d &Vect) const
	{return !(*this == Vect);}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set x coordinate of this vector from the given x coordinate
	inline GLC_Vector3d& setX(const double &dX)
	{
		m_Vector[0]= dX;
		return *this;
	}

	//! Set y coordinate of this vector from the given y coordinate
	inline GLC_Vector3d& setY(const double &dY)
	{
		m_Vector[1]= dY;
		return *this;
	}

	//! Set z coordinate of this vector from the given z coordinate
	inline GLC_Vector3d& setZ(const double &dZ)
	{
		m_Vector[2]= dZ;
		return *this;
	}

	//! Set (x, y, z) coordinate of this vector from the given (x, y, z) coordinates
	inline GLC_Vector3d& setVect(double, double, double);

	//! Set (x, y, z) coordinate of this vector from the given vector coordinates
	GLC_Vector3d& setVect(const GLC_Vector3d &vector)
	{
		memcpy(m_Vector, vector.m_Vector, sizeof(double) * 3);
		return *this;
	}

	//! Set vector lenght from the given scalar and return a reference of this vector
	inline GLC_Vector3d& setLength(double);

	//! Normalize this vector and return a reference to it
	inline GLC_Vector3d& normalize()
	{return setLength(1.0);}

	//! Invert this vector and return a reference to it
	inline GLC_Vector3d& invert();

//@}


//////////////////////////////////////////////////////////////////////
//Private attributes
//////////////////////////////////////////////////////////////////////
private:
	/*! Vector array definition \n
	*	vector[0]	X \n
	*	vector[1]	Y \n
	*	vector[2]	Z \n
	*/
	double m_Vector[3];

}; //class GLC_Vector3d

// Vector constant in glc namespace
namespace glc
{
	// Axis definition
	/*! \var X_AXIS
	 *  \brief X axis Vector*/
	const GLC_Vector3d X_AXIS(1.0, 0.0, 0.0);

	/*! \var Y_AXIS
	 *  \brief Y axis Vector*/
	const GLC_Vector3d Y_AXIS(0.0, 1.0, 0.0);

	/*! \var Z_AXIS
	 *  \brief Z axis Vector*/
	const GLC_Vector3d Z_AXIS(0.0, 0.0, 1.0);
};

//! Define GLC_Point3D
typedef GLC_Vector3d GLC_Point3d;

//! Write the vector to stream
inline QDataStream &operator<<(QDataStream & stream, const GLC_Vector3d & vector)
{
	stream << vector.x() << vector.y() << vector.z();
	return stream;
}

//! Read the vector from stream
inline QDataStream &operator>>(QDataStream &stream, GLC_Vector3d &vector)
{
	double x, y, z;
	stream >> x >> y >> z;
	vector.setVect(x, y, z);
	return stream;
}

//! Return the determinant of the given Matrix 3X3
inline double getDeterminant3x3(const double *Mat3x3)
{
	double Determinant;

	Determinant= Mat3x3[0] * ( Mat3x3[4] * Mat3x3[8] - Mat3x3[7] * Mat3x3[5]);
	Determinant+= - Mat3x3[3] * ( Mat3x3[1] * Mat3x3[8] - Mat3x3[7] * Mat3x3[2]);
	Determinant+= Mat3x3[6] * ( Mat3x3[1] * Mat3x3[5] - Mat3x3[4] * Mat3x3[2]);

	return Determinant;
}

//////////////////////////////////////////////////////////////////////
// inline method implementation
//////////////////////////////////////////////////////////////////////

GLC_Vector3d::GLC_Vector3d()
{
	m_Vector[0]= 0.0;
	m_Vector[1]= 0.0;
	m_Vector[2]= 0.0;
}

GLC_Vector3d::GLC_Vector3d(double x, double y, double z)
{
	m_Vector[0]= x;
	m_Vector[1]= y;
	m_Vector[2]= z;
}

GLC_Vector3d::GLC_Vector3d(const GLC_Vector3df &vector)
{
	m_Vector[0]= static_cast<double>(vector.m_Vector[0]);
	m_Vector[1]= static_cast<double>(vector.m_Vector[1]);
	m_Vector[2]= static_cast<double>(vector.m_Vector[2]);
}

GLC_Vector3d::GLC_Vector3d(const GLC_Vector2d &vector)
{
	m_Vector[0]= vector.getX();
	m_Vector[1]= vector.getY();
	m_Vector[2]= 0.0;
}

GLC_Vector3d& GLC_Vector3d::operator = (const GLC_Vector3df &Vect)
{
	m_Vector[0]= static_cast<double>(Vect.m_Vector[0]);
	m_Vector[1]= static_cast<double>(Vect.m_Vector[1]);
	m_Vector[2]= static_cast<double>(Vect.m_Vector[2]);

	return *this;
}

GLC_Vector3d GLC_Vector3d::operator ^ (const GLC_Vector3d &vector) const
{
	GLC_Vector3d vectResult;
	vectResult.m_Vector[0]= m_Vector[1] * vector.m_Vector[2] - m_Vector[2] * vector.m_Vector[1];
	vectResult.m_Vector[1]= m_Vector[2] * vector.m_Vector[0] - m_Vector[0] * vector.m_Vector[2];
	vectResult.m_Vector[2]= m_Vector[0] * vector.m_Vector[1] - m_Vector[1] * vector.m_Vector[0];

	return vectResult;
}

bool GLC_Vector3d::operator == (const GLC_Vector3d &vector) const
{
	bool bResult= qFuzzyCompare(m_Vector[0], vector.m_Vector[0]);
	bResult= bResult && qFuzzyCompare(m_Vector[1], vector.m_Vector[1]);
	bResult= bResult && qFuzzyCompare(m_Vector[2], vector.m_Vector[2]);

	return bResult;
}

bool GLC_Vector3d::operator > (const GLC_Vector3d &vector) const
{
	bool result= m_Vector[0] > vector.m_Vector[0];
	result= result && (m_Vector[1] > vector.m_Vector[1]);
	result= result && (m_Vector[2] > vector.m_Vector[2]);
	return result;
}

bool GLC_Vector3d::operator < (const GLC_Vector3d &vector) const
{
	bool result= m_Vector[0] < vector.m_Vector[0];
	result= result && (m_Vector[1] < vector.m_Vector[1]);
	result= result && (m_Vector[2] < vector.m_Vector[2]);
	return result;
}

GLC_Vector3d& GLC_Vector3d::setVect(double x, double y, double z)
{
	m_Vector[0]= x;
	m_Vector[1]= y;
	m_Vector[2]= z;

	return *this;
}

inline GLC_Vector3d& GLC_Vector3d::setLength(double norme)
{
	const double normCur= sqrt( m_Vector[0] * m_Vector[0] + m_Vector[1] * m_Vector[1] + m_Vector[2] * m_Vector[2]);

	if (normCur != 0.0f)
	{
		const double Coef = norme / normCur;

		m_Vector[0] = m_Vector[0] * Coef;
		m_Vector[1] = m_Vector[1] * Coef;
		m_Vector[2] = m_Vector[2] * Coef;
	}
	return *this;
}

GLC_Vector3d& GLC_Vector3d::invert()
{
	m_Vector[0]= - m_Vector[0];
	m_Vector[1]= - m_Vector[1];
	m_Vector[2]= - m_Vector[2];
	return *this;
}

GLC_Vector2d GLC_Vector3d::toVector2d(const GLC_Vector3d& mask) const
{
	GLC_Vector2d resultVect;
	if (mask.m_Vector[0] == 0.0)
	{
		resultVect.setX(m_Vector[0]);
		if (mask.m_Vector[1] == 0.0) resultVect.setY(m_Vector[1]);
		else resultVect.setY(m_Vector[2]);
	}
	else resultVect.setVect(m_Vector[1], m_Vector[2]);

	return resultVect;
}

double GLC_Vector3d::angleWithVect(GLC_Vector3d Vect) const
{
	GLC_Vector3d ThisVect(*this);
	ThisVect.normalize();
	Vect.normalize();
	// Rotation axis
	const GLC_Vector3d VectAxeRot(ThisVect ^ Vect);
	// Check if the rotation axis vector is null
	if (!VectAxeRot.isNull())
	{
		return acos(ThisVect * Vect);
	}
	else return 0.0;
}

double GLC_Vector3d::signedAngleWithVect(GLC_Vector3d Vect, const GLC_Vector3d& dir) const
{
	double angle= 0.0;

	GLC_Vector3d ThisVect(*this);
	ThisVect.normalize();
	Vect.normalize();
	if (Vect == ThisVect.inverted())
	{
		angle= glc::PI;
	}
	else if (Vect != ThisVect)
	{
		// Rotation axis
		const GLC_Vector3d VectAxeRot(ThisVect ^ Vect);
		// Check if the rotation axis vector is null
		if (!VectAxeRot.isNull())
		{
			double mat3x3[9];
			mat3x3[0]= ThisVect.m_Vector[0];
			mat3x3[1]= ThisVect.m_Vector[1];
			mat3x3[2]= ThisVect.m_Vector[2];

			mat3x3[3]= Vect.m_Vector[0];
			mat3x3[4]= Vect.m_Vector[1];
			mat3x3[5]= Vect.m_Vector[2];

			mat3x3[6]= dir.m_Vector[0];
			mat3x3[7]= dir.m_Vector[1];
			mat3x3[8]= dir.m_Vector[2];

			double det= getDeterminant3x3(mat3x3);

			double sign= 1.0;
			if (det != 0) sign= fabs(det) / det;
			angle= acos(ThisVect * Vect) * sign;
		}
	}

	return angle;
}

QString GLC_Vector3d::toString() const
{
	QString result("[");

	result+= QString::number(m_Vector[0]) + QString(" , ");
	result+= QString::number(m_Vector[1]) + QString(" , ");
	result+= QString::number(m_Vector[2]) + QString("]");

	return result;
}

#endif /*GLC_VECTOR3D_H_*/
