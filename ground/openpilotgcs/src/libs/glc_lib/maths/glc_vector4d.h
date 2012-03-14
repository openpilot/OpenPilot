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

//! \file glc_vector4d.h interface for the GLC_Vector4d class.

#ifndef GLC_VECTOR4D_H_
#define GLC_VECTOR4D_H_

#include <QVector>
#include <QDataStream>

#include "glc_utils_maths.h"
#include "glc_vector2d.h"
#include "glc_vector3d.h"
#include "glc_vector3df.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Vector4d
/*! \brief GLC_Vector4d is a 4 dimensions Vector*/

/*! GLC_Vector4d is used to represent 3D position and vectors. \n
 *  it had 4 Dimensions for compatibility purpose with GLC_Matrix4x4
 * */
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_Vector4d
{
	//! GLC_Matrix4x4 class
	friend class GLC_Matrix4x4;

	//! Overload unary "-" operator
	inline friend GLC_Vector4d operator - (const GLC_Vector4d &Vect)
	{
		return GLC_Vector4d(-Vect.vector[0], -Vect.vector[1], -Vect.vector[2]);
	}


//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	/*! Default constructor
	*  Value is set to
	* \n X = 0.0
	* \n Y =  0.0
	* \n Z =  0.0
	* \n W =  1.0*/
	inline GLC_Vector4d()
	{
		vector[0]= 0.0;
		vector[1]= 0.0;
		vector[2]= 0.0;

		vector[3]= 1.0;
	}
	//! Standard constructor With x, y, z and w with default value of 1.0
	inline GLC_Vector4d(const double &dX, const double &dY, const double &dZ, const double &dW= 1.0)
	{
		setVect(dX, dY, dZ, dW);
	}
	//! Copy constructor
	inline GLC_Vector4d(const GLC_Vector4d &Vect)
	{
		vector[0]= Vect.vector[0];
		vector[1]= Vect.vector[1];
		vector[2]= Vect.vector[2];
		vector[3]= Vect.vector[3];
	}

	//! Copy from an GLC_Vector3d
	inline GLC_Vector4d(const GLC_Vector3d &Vect)
	{
		vector[0]= Vect.m_Vector[0];
		vector[1]= Vect.m_Vector[1];
		vector[2]= Vect.m_Vector[2];
		vector[3]= 1.0;
	}

	//! Copy from an GLC_Vector3d
	inline GLC_Vector4d(const GLC_Vector3df &Vect)
	{
		vector[0]= static_cast<double>(Vect.m_Vector[0]);
		vector[1]= static_cast<double>(Vect.m_Vector[1]);
		vector[2]= static_cast<double>(Vect.m_Vector[2]);
		vector[3]= 1.0;
	}

	//! Copy from an GLC_Vector3d
	inline GLC_Vector4d(const GLC_Vector2d &Vect)
	{
		vector[0]= Vect.m_Vector[0];
		vector[1]= Vect.m_Vector[1];
		vector[2]= 0.0;
		vector[3]= 1.0;
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! @name Operator Overload */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Overload binary "+" operator
	inline GLC_Vector4d operator + (const GLC_Vector4d &Vect) const
	{
		GLC_Vector4d VectResult(vector[0] + Vect.vector[0], vector[1] + Vect.vector[1],
			vector[2] + Vect.vector[2]);

		return VectResult;
	}

	//! Overload "=" operator
	inline GLC_Vector4d& operator = (const GLC_Vector4d &Vect)
	{
		vector[0]= Vect.vector[0];
		vector[1]= Vect.vector[1];
		vector[2]= Vect.vector[2];
		vector[3]= Vect.vector[3];

		return *this;
	}

	//! Overload "=" operator
	inline GLC_Vector4d& operator = (const GLC_Vector3d &Vect)
	{
		vector[0]= Vect.m_Vector[0];
		vector[1]= Vect.m_Vector[1];
		vector[2]= Vect.m_Vector[2];
		vector[3]= 1.0;

		return *this;
	}

	//! Overload "=" operator
	inline GLC_Vector4d& operator = (const GLC_Vector3df &Vect)
	{
		vector[0]= static_cast<double>(Vect.m_Vector[0]);
		vector[1]= static_cast<double>(Vect.m_Vector[1]);
		vector[2]= static_cast<double>(Vect.m_Vector[2]);
		vector[3]= 1.0;

		return *this;
	}

	//! Overload "=" operator
	inline GLC_Vector4d& operator = (const GLC_Vector2d &Vect)
	{
		vector[0]= Vect.m_Vector[0];
		vector[1]= Vect.m_Vector[1];
		vector[2]= 0.0;
		vector[3]= 1.0;

		return *this;
	}

	//! Overload "+=" operator
	inline GLC_Vector4d* operator += (const GLC_Vector4d &Vect)
	{
		*this= *this + Vect;
		return this;
	}


	//! Overload binary "-" operator
	inline GLC_Vector4d operator - (const GLC_Vector4d &Vect) const
	{
		GLC_Vector4d VectResult(vector[0] - Vect.vector[0], vector[1] - Vect.vector[1],
			vector[2] - Vect.vector[2]);

		return VectResult;
	}

	//! Overload binary "-=" operator
	GLC_Vector4d* operator -= (const GLC_Vector4d &Vect)
	{
		*this= *this - Vect;
		return this;
	}

	//! Overload dot product "^" operator
	GLC_Vector4d operator ^ (const GLC_Vector4d &Vect) const;

	//! Overload scalar product "*" operator between 2 vector
	inline double operator * (const GLC_Vector4d &Vect) const
	{
		// W Component is ignored
		return vector[0] * Vect.vector[0] + vector[1] * Vect.vector[1] +
			vector[2] * Vect.vector[2];
	}

	//! Overload scalar product "*" operator between 1 vector and one scalar
	inline GLC_Vector4d operator * (double Scalaire) const
	{
		return GLC_Vector4d(vector[0] * Scalaire, vector[1] * Scalaire, vector[2] * Scalaire);
	}


	//! Overload equality "==" operator
	bool operator == (const GLC_Vector4d &Vect) const;

	//! Overload dot product "!=" operator
	inline bool operator != (const GLC_Vector4d &Vect) const
	{
		return !(*this == Vect);
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! X Compound
	inline GLC_Vector4d& setX(const double &dX)
	{
		vector[0]= dX;
		return *this;
	}

	//! Y Compound
	inline GLC_Vector4d& setY(const double &dY)
	{
		vector[1]= dY;
		return *this;
	}

	//! Z Compound
	inline GLC_Vector4d& setZ(const double &dZ)
	{
		vector[2]= dZ;
		return *this;
	}

	//! W Compound
	GLC_Vector4d& setW(const double &dW);

	//! All Compound
	GLC_Vector4d& setVect(const double &dX, const double &dY, const double &dZ, const double &dW= 1);

	//! From another Vector
	inline GLC_Vector4d& setVect(const GLC_Vector4d &Vect)
	{
		vector[0]= Vect.vector[0];
		vector[1]= Vect.vector[1];
		vector[2]= Vect.vector[2];
		vector[3]= Vect.vector[3];
		return *this;
	}

	//! Vector Normal
	GLC_Vector4d& setNormal(const double &Norme);

	/*! Invert Vector*/
	inline GLC_Vector4d& invert(void)
	{
		vector[0]= - vector[0];
		vector[1]= - vector[1];
		vector[2]= - vector[2];
		return *this;
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return X Compound
	inline double X(void) const
	{
		return vector[0];
	}
	//! Return Y Compound
	inline double Y(void) const
	{
		return vector[1];
	}
	//! Return Z Compound
	inline double Z(void) const
	{
		return vector[2];
	}
	//! Return W Compound
	inline double W(void) const
	{
		return vector[3];
	}
	inline GLC_Vector3d toVector3d() const
	{
		return GLC_Vector3d(vector[0], vector[1], vector[2]);
	}
	inline GLC_Vector3df toVector3df() const
	{
		return GLC_Vector3df(static_cast<float>(vector[0]), static_cast<float>(vector[1]), static_cast<float>(vector[2]));
	}
	//! Return a pointer to vector data
	inline const double *data(void) const
	{
		return vector;
	}
	//! Return Vector Norm
	inline double norm(void) const
	{
		return sqrt(vector[0] * vector[0] + vector[1] * vector[1]
			+ vector[2] * vector[2]);
	}
	/*! Vector is null*/
	inline bool isNull(void) const
	{
		bool result;

		result= qFuzzyCompare(vector[0], 0.0) && qFuzzyCompare(vector[1], 0.0)
			&& qFuzzyCompare(vector[2], 0.0);

		return result;
	}

	//! Return the Angle with another vector
	double getAngleWithVect(GLC_Vector4d Vect) const;

	//! Return the vector string
	QString toString() const;

	//! Return the 2D vector specified by a mask vector
	/*! retrieve component corresponding to
	 * mask NULL component*/
	GLC_Vector2d toVector2d(const GLC_Vector4d&) const;

	//! Return a QVector<float> of 3 values
	inline QVector<float> toFloat3dQVector() const
	{
		QVector<float> result;
		result << static_cast<float>(vector[0]) << static_cast<float>(vector[1]) << static_cast<float>(vector[2]);
		return result;
	}
//@}

//////////////////////////////////////////////////////////////////////
// Private services functions
//////////////////////////////////////////////////////////////////////
private:

	//! Normalize Vector w <- 1
	void normalizeW(void);

//////////////////////////////////////////////////////////////////////
//name Private attributes
//////////////////////////////////////////////////////////////////////
private:
	/*! Vector array definition \n
	*	vector[0]	X \n
	*	vector[1]	Y \n
	*	vector[2]	Z \n
	*	vector[3]	1
	*/
	enum {VECT4DIMENSION = 4};
	double vector[VECT4DIMENSION];

}; //class GLC_Vector4d

//! Define GLC_Point4D
//typedef GLC_Vector4d GLC_Point4d;

//! Non-member stream operator
QDataStream &operator<<(QDataStream &, const GLC_Vector4d &);
QDataStream &operator>>(QDataStream &, GLC_Vector4d &);

#endif /*GLC_VECTOR4D_H_*/
