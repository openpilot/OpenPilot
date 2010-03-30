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

//! \file glc_matrix4x4.h interface for the GLC_Matrix4x4 class.

#ifndef GLC_MATRIX4X4_H_
#define GLC_MATRIX4X4_H_

#include <QVector>
#include "glc_vector4d.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Matrix4x4
/*! \brief GLC_Matrix4x4 is a 4 dimensions Matrix*/

/*! GLC_Matrix4x4 is used to represent 3D homogeneous 3d transformation \n
 *  GLC_Matrix4x4 is a row first matrix compatible with OpenGL Matrix
 * */
//////////////////////////////////////////////////////////////////////
class GLC_Matrix4x4
{
	friend class GLC_Vector4d;

//////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////
public:
//! @name Constructor
//@{
	//! Default Constructor
	/*! Create an identity Matrix */
	GLC_Matrix4x4();

	//! Construct a Matrix by copy
	GLC_Matrix4x4(const GLC_Matrix4x4 &);

	//! Construct a Matrix by an array of 16 elements.
	GLC_Matrix4x4(const double *);

	//! Construct a Matrix by an array of 16 elements.
	GLC_Matrix4x4(const float *);

	//! Construct rotation matrix by a vector and an angle
	inline GLC_Matrix4x4(const GLC_Vector4d &Vect, const double &dAngleRad)
	{
		setMatRot(Vect, dAngleRad);
	}

	//! Construct rotation matrix by 2 vectors
	inline GLC_Matrix4x4(const GLC_Vector4d &Vect1, const GLC_Vector4d &Vect2)
	{
		setMatRot(Vect1, Vect2);
	}

	//! Construct translation matrix by a vector
	inline GLC_Matrix4x4(const GLC_Vector4d &Vect)
	{
		setMatTranslate(Vect);
	}

	//! Construct translation matrix by 3 coordinates
	inline GLC_Matrix4x4(const double Tx, const double Ty, const double Tz)
	{
		setMatTranslate(Tx, Ty, Tz);
	}
//@}

//////////////////////////////////////////////////////////////////////
/*! @name Operator Overload */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Matrix cross product
	GLC_Matrix4x4 operator * (const GLC_Matrix4x4 &Mat) const;

	//! Vector transformation
	GLC_Vector4d operator * (const GLC_Vector4d &Vect) const;

	//! Return true if the 2 matrix are equals
	bool operator==(const GLC_Matrix4x4& mat) const;

	//! Return true if the 2 matrix are not equals
	inline bool operator!=(const GLC_Matrix4x4& mat) const
	{return not operator==(mat);}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Function*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the matrix determinant
	double determinant(void) const;

	//! return a pointer to a row first array of 16 elements
	inline const double *data(void) const
	{
		return matrix;
	}
	//! Return a vector which contains radians Euler angle of the matrix
	QVector<double> toEuler(void) const;

	//! Return the matrix string
	QString toString() const;

	//! Return the rotation matrix of this matrix
	inline GLC_Matrix4x4 rotationMatrix() const
	{
		GLC_Matrix4x4 result(*this);
		const double invScaleX= 1.0 / scalingX();
		const double invScaleY= 1.0 / scalingY();
		const double invScaleZ= 1.0 / scalingZ();
		result.matrix[0]= result.matrix[0] * invScaleX;
		result.matrix[1]= result.matrix[1] * invScaleX;
		result.matrix[2]= result.matrix[2] * invScaleX;

		result.matrix[4]= result.matrix[4] * invScaleY;
		result.matrix[5]= result.matrix[5] * invScaleY;
		result.matrix[6]= result.matrix[6] * invScaleY;

		result.matrix[8]= result.matrix[8] * invScaleZ;
		result.matrix[9]= result.matrix[9] * invScaleZ;
		result.matrix[10]= result.matrix[10] * invScaleZ;

		result.matrix[12]= 0.0; result.matrix[13]= 0.0; result.matrix[14]= 0.0;
		result.matrix[3]= 0.0; result.matrix[7]= 0.0; result.matrix[11]= 0.0;
		result.matrix[15]= 1.0;
		return result;
	}

	//! Return the ismetric of this matrix
	inline GLC_Matrix4x4 isometricMatrix() const
	{
		GLC_Matrix4x4 result(*this);
		const double invScaleX= 1.0 / scalingX();
		const double invScaleY= 1.0 / scalingY();
		const double invScaleZ= 1.0 / scalingZ();
		result.matrix[0]= result.matrix[0] * invScaleX;
		result.matrix[1]= result.matrix[1] * invScaleX;
		result.matrix[2]= result.matrix[2] * invScaleX;

		result.matrix[4]= result.matrix[4] * invScaleY;
		result.matrix[5]= result.matrix[5] * invScaleY;
		result.matrix[6]= result.matrix[6] * invScaleY;

		result.matrix[8]= result.matrix[8] * invScaleZ;
		result.matrix[9]= result.matrix[9] * invScaleZ;
		result.matrix[10]= result.matrix[10] * invScaleZ;
		return result;
	}

	//! Return the x Scaling
	inline double scalingX() const
	{
		GLC_Vector4d x(matrix[0], matrix[1], matrix[2]);
		return x.norm();
	}

	//! Return the y Scaling
	inline double scalingY() const
	{
		GLC_Vector4d y(matrix[4], matrix[5], matrix[6]);
		return y.norm();
	}

	//! Return the z Scaling
	inline double scalingZ() const
	{
		GLC_Vector4d z(matrix[8], matrix[9], matrix[10]);
		return z.norm();
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Function*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set matrix to a rotation matrix define by a vector and an angle in radians
	GLC_Matrix4x4& setMatRot(const GLC_Vector4d &, const double &);

	//! Set matrix to a rotation matrix define by 2 vectors
	GLC_Matrix4x4& setMatRot(const GLC_Vector4d &, const GLC_Vector4d &);

	//! Set Matrix to a translation matrix by a vector
	GLC_Matrix4x4& setMatTranslate(const GLC_Vector4d &);

	//! Set Matrix to a translation matrix by 3 coordinates
	GLC_Matrix4x4& setMatTranslate(const double, const double, const double);

	//! Set Matrix to a scaling matrix define by 3 double
	GLC_Matrix4x4& setMatScaling(const double, const double, const double);

	//! Reverse the Matrix
	GLC_Matrix4x4& invert(void);

	//! Set the matrix to identify matrix
	GLC_Matrix4x4& setToIdentity();

	//! Set the matrix by its transpose
	GLC_Matrix4x4& transpose(void);

	//! Set the matrix with Euler angle
	GLC_Matrix4x4& fromEuler(const double, const double, const double);


//@}

//////////////////////////////////////////////////////////////////////
//! Private services Functions
//////////////////////////////////////////////////////////////////////
private:

	//! Return true if the index (argument) is in the diagonal of the matrix
	inline bool isDiagonal(const int index) const
	{
		if ((index == 0) || (index == 5) || (index == 10) || (index == 15))
			return true;
		else
			return false;
	}

	//! Return the determinant of a matrix cell
	double getDeterminantLC(const int, const int) const;

	//! Compute Sub 3X3 matrix
	void getSubMat(const int, const int, double *) const;

	// Return the Matrix 3X3 determinant
	double getDeterminant3x3(const double *) const;

	//! Return the transpose matrix
	GLC_Matrix4x4 getTranspose(void) const;

	//! Return the co-matrix
	GLC_Matrix4x4 getCoMat4x4(void) const;



//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:

	//! Number of elements in the matrix
	enum {TAILLEMAT4X4 = 16};
	//! Matrix size
	enum {DIMMAT4X4 = 4};
	//! Matrix row first array
	double matrix[TAILLEMAT4X4];
/*
the matrix :
					a[00] a[04] a[08] a[12]

					a[01] a[05] a[09] a[13]

					a[02] a[06] a[10] a[14]

					a[03] a[07] a[11] a[15]
 */
//					Tx = 12,	Ty = 13,	Tz = 14
};
#endif /*GLC_MATRIX4X4_H_*/
