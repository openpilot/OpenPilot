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

//! \file glc_vector2d.h interface for the GLC_Vector2d class.

#ifndef GLC_VECTOR2D_H_
#define GLC_VECTOR2D_H_

#include <QString>
#include "glc_utils_maths.h"
#include "glc_vector2df.h"

#include "glc_config.h"

//////////////////////////////////////////////////////////////////////
// definition global
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//! \class GLC_Vector2d
/*! \brief GLC_Vector2d is a 2 dimensions Vector*/

/*! GLC_Vector2d is used to represent 2D position and vectors.
 * */
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_Vector2d
{
	friend class GLC_Vector4d;
	friend class GLC_Vector3d;

	/*! Overload unary "-" operator*/
	inline friend GLC_Vector2d operator - (const GLC_Vector2d &Vect)
	{
		return GLC_Vector2d(-Vect.dVecteur[0], -Vect.dVecteur[1]);
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
	*/
	inline GLC_Vector2d()
	{
		dVecteur[0]= 0.0;
		dVecteur[1]= 0.0;
	}

	/*! Standard constructor With x, y = 0.0*/
	inline GLC_Vector2d(const double &dX, const double &dY)
	{
		dVecteur[0]= dX;
		dVecteur[1]= dY;
	}

	/*! Recopy constructor
	 * Sample use
	 * \code
	 * NewVect = new GLC_Vector2d(OldVect);
	 * \endcode
	 */
	inline GLC_Vector2d(const GLC_Vector2d &Vect)
	{
		dVecteur[0]= Vect.dVecteur[0];
		dVecteur[1]= Vect.dVecteur[1];
	}
//@}
//////////////////////////////////////////////////////////////////////
/*! @name Operator Overload */
//@{
//////////////////////////////////////////////////////////////////////
public:

	/*! Overload binary "+" operator*/
	inline GLC_Vector2d operator + (const GLC_Vector2d &Vect) const
	{
		GLC_Vector2d VectResult(dVecteur[0] + Vect.dVecteur[0], dVecteur[1] + Vect.dVecteur[1]);

		return VectResult;
	}

	/*! Overload "=" operator*/
	inline GLC_Vector2d& operator = (const GLC_Vector2d &Vect)
	{
		dVecteur[0]= Vect.dVecteur[0];
		dVecteur[1]= Vect.dVecteur[1];

		return *this;
	}

	/*! Overload "=" operator*/
	inline GLC_Vector2d& operator = (const GLC_Vector2df &Vect)
	{
		dVecteur[0]= static_cast<double>(Vect.vector[0]);
		dVecteur[1]= static_cast<double>(Vect.vector[1]);

		return *this;
	}


	/*! Overload "+=" operator*/
	inline GLC_Vector2d* operator += (const GLC_Vector2d &Vect)
	{
		*this= *this + Vect;
		return this;
	}


	/*! Overload binary "-" operator*/
	inline GLC_Vector2d operator - (const GLC_Vector2d &Vect) const
	{
		GLC_Vector2d VectResult(dVecteur[0] - Vect.dVecteur[0], dVecteur[1] - Vect.dVecteur[1]);

		return VectResult;
	}

	/*! Overload binary "-=" operator*/
	inline GLC_Vector2d* operator -= (const GLC_Vector2d &Vect)
	{
		*this= *this - Vect;
		return this;
	}

	/*! Overload dot product "^" operator*/
	inline double operator ^ (const GLC_Vector2d &Vect) const
	{
		return dVecteur[0] * Vect.dVecteur[1] - dVecteur[1] * Vect.dVecteur[0];
	}

	/*! Overload scalar product "*" operator between 2 vector*/
	inline double operator * (const GLC_Vector2d &Vect) const
	{
		return dVecteur[0] * Vect.dVecteur[0] + dVecteur[1] * Vect.dVecteur[1];
	}

	/*! Overload scalar product "*" operator between 1 vector and one scalar*/
	inline GLC_Vector2d operator * (double Scalaire) const
	{
		return GLC_Vector2d(dVecteur[0] * Scalaire, dVecteur[1] * Scalaire);;
	}


	/*! Overload equality "==" operator*/
	inline bool operator == (const GLC_Vector2d &Vect) const
	{
		bool bResult= qFuzzyCompare(dVecteur[0], Vect.dVecteur[0]);
		bResult= bResult && qFuzzyCompare(dVecteur[1], Vect.dVecteur[1]);

		return bResult;
	}

	/*! Overload "!=" operator*/
	inline bool operator != (const GLC_Vector2d &Vect) const
	{
		return !(*this == Vect);
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	/*! X Composante*/
	inline GLC_Vector2d& setX(const double &dX)
	{
		dVecteur[0]= dX;
		return *this;
	}

	/*! Y Composante*/
	inline GLC_Vector2d& setY(const double &dY)
	{
		dVecteur[1]= dY;
		return *this;
	}

	/*! All Composante*/
	inline GLC_Vector2d& setVect(const double &dX, const double &dY)
	{
		dVecteur[0]= dX;
		dVecteur[1]= dY;
		return *this;
	}

	/*! From another Vector*/
	inline GLC_Vector2d& setVect(const GLC_Vector2d &Vect)
	{
		dVecteur[0]= Vect.dVecteur[0];
		dVecteur[1]= Vect.dVecteur[1];
		return *this;
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	/*! X Composante*/
	inline double getX(void) const
	{
		return dVecteur[0];
	}
	/*! Y Composante*/
	inline double getY(void) const
	{
		return dVecteur[1];
	}
	/*! retourne un pointeur constant vers le tableau du vecteur.*/
	inline const double *return_dVect(void) const
	{
		return dVecteur;
	}
	/*! Return true if the vector is null*/
	inline bool isNull(void) const
	{
		return qFuzzyCompare(dVecteur[0], 0.0) && qFuzzyCompare(dVecteur[1], 0.0);
	}
	//! return the string representation of vector
	inline QString toString() const
	{
		return QString("[") + QString::number(dVecteur[0]) + QString(" , ") + QString::number(dVecteur[1]) + QString("]");
	}
	//! return a vector perpendicular to this
	inline GLC_Vector2d perp() const
	{
		return GLC_Vector2d(-dVecteur[1], dVecteur[0]);
	}
//@}

//////////////////////////////////////////////////////////////////////
//name Private attributes
//////////////////////////////////////////////////////////////////////
private:
	/*! Vector array definition \n
	*	vector[0]	X \n
	*	vector[1]	Y \n
	*/
	double dVecteur[2];

}; //class GLC_Vector2d

//! Define GLC_Point2D
typedef GLC_Vector2d GLC_Point2d;

#endif /*GLC_VECTOR2D_H_*/
