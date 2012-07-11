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

//! \file glc_matrix4x4.h interface for the GLC_Matrix4x4 class.

#ifndef GLC_MATRIX4X4_H_
#define GLC_MATRIX4X4_H_

#include <QVector>
#include "glc_vector3d.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Matrix4x4
/*! \brief GLC_Matrix4x4 is a 4 dimensions Matrix*/

/*! GLC_Matrix4x4 is used to represent 3d homogeneous transformation in 3d space \n
 *  GLC_Matrix4x4 is a row first matrix compatible with OpenGL Matrix
 * */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Matrix4x4
{
	friend class GLC_Vector3d;
public:
	//! matrix possible type
	enum
	{
		General= 0x0000,
		Direct= 0x0001,
		Indirect= 0x0002,
		Identity= 0x0003
	};

//////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////
public:
//! @name Constructor
//@{
	//! Construct an identity matrix
	inline GLC_Matrix4x4();

	//! Construct a matrix from another matrix
	inline GLC_Matrix4x4(const GLC_Matrix4x4 &matrix)
	:m_Type(matrix.m_Type)
	{
		memcpy(m_Matrix, matrix.m_Matrix, sizeof(double) * 16);
	}

	//! Construct a matrix from an array of 16 double elements.
	inline GLC_Matrix4x4(const double *pArray)
	: m_Type(General)
	{
		memcpy(m_Matrix, pArray, sizeof(double) * 16);
	}

	//! Construct a Matrix from an array of 16 float elements.
	inline GLC_Matrix4x4(const float *);

	//! Construct rotation matrix from a 3d vector and an angle in radians
	inline GLC_Matrix4x4(const GLC_Vector3d &Vect, const double &dAngleRad);

	//! Construct rotation matrix from 2 3d vectors
	inline GLC_Matrix4x4(const GLC_Vector3d &Vect1, const GLC_Vector3d &Vect2);

	//! Construct translation matrix from a 3d vector
	inline GLC_Matrix4x4(const GLC_Vector3d &Vect)
	{setMatTranslate(Vect);}

	//! Construct translation matrix from coordinates in double
	inline GLC_Matrix4x4(const double Tx, const double Ty, const double Tz)
	{setMatTranslate(Tx, Ty, Tz);}
//@}

//////////////////////////////////////////////////////////////////////
/*! @name Operator Overload */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Copy the content of the given matrix in this matrix
	inline GLC_Matrix4x4& operator = (const GLC_Matrix4x4 &matrix);

	//! Return the product of this matrix to the given matrix
	inline GLC_Matrix4x4 operator * (const GLC_Matrix4x4 &Mat) const;

	//! Return the result of transforming the given vector by this matrix
	inline GLC_Vector3d operator * (const GLC_Vector3d &Vect) const;

	//! Return true if this matrix is equal to the given matrix
	inline bool operator==(const GLC_Matrix4x4& mat) const;

	//! Return true if this matrix is not equal to the given matrix
	inline bool operator!=(const GLC_Matrix4x4& mat) const
	{return !operator==(mat);}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Function*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the determinant of this matrix
	inline double determinant(void) const;

	//! Return a pointer to the row first array of 16 elements of this matrix
	/*! Don't modify data with this method*/
	inline const double* getData(void)
	{return m_Matrix;}

	//! Return a const pointer to the row first array of 16 elements of this matrix
	inline const double* getData(void) const
	{return m_Matrix;}

	//! Return a pointer to the row first array of 16 elements of this matrix
	inline double* setData(void)
	{
		m_Type= General;
		return m_Matrix;
	}

	//! Return a QVector<double> which contains radians Euler angle of this matrix
	QVector<double> toEuler(void) const;

	//! Return the string representation of this matrix
	QString toString() const;

	//! Return the rotation matrix of this matrix
	inline GLC_Matrix4x4 rotationMatrix() const;

	//! Return the isometric matrix of this matrix
	inline GLC_Matrix4x4 isometricMatrix() const;

	//! Return the x Scaling of this matrix
	inline double scalingX() const
	{return GLC_Vector3d(m_Matrix[0], m_Matrix[1], m_Matrix[2]).length();}

	//! Return the y Scaling of this matrix
	inline double scalingY() const
	{return GLC_Vector3d(m_Matrix[4], m_Matrix[5], m_Matrix[6]).length();}

	//! Return the z Scaling of this matrix
	inline double scalingZ() const
	{return GLC_Vector3d(m_Matrix[8], m_Matrix[9], m_Matrix[10]).length();}

	//! Return the inverse of this matrix
	inline GLC_Matrix4x4 inverted() const
	{return GLC_Matrix4x4(*this).invert();}

	//! Return The type af this matrix
	inline int type() const
	{
		return m_Type;
	}

	//! Return true if this matrix is direct
	inline bool isDirect() const
	{return (m_Type & Direct);}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Function*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set this matrix to a rotation matrix given by a 3d vector and an angle in radian
	inline GLC_Matrix4x4& setMatRot(const GLC_Vector3d &, const double &);

	//! Set this matrix to a rotation matrix given by 2 3d vectors
	inline GLC_Matrix4x4& setMatRot(const GLC_Vector3d &, const GLC_Vector3d &);

	//! Set this matrix to a translation matrix given by a 3d vector
	inline GLC_Matrix4x4& setMatTranslate(const GLC_Vector3d &);

	//! Set this matrix to a translation matrix given by 3 double coordinates
	inline GLC_Matrix4x4& setMatTranslate(const double, const double, const double);

	//! Set this matrix to a scaling matrix define by 3 double
	inline GLC_Matrix4x4& setMatScaling(const double, const double, const double);

	//! Inverse this Matrix and return a reference to this matrix
	inline GLC_Matrix4x4& invert(void);

	//! Set this matrix to the identify matrix and return a reference to this matrix
	inline GLC_Matrix4x4& setToIdentity();

	//! Transpose this matrix and return a reference to this matrix
	inline GLC_Matrix4x4& transpose(void);

	//! Set this matrix with Euler angle and return a reference to this matrix
	GLC_Matrix4x4& fromEuler(const double, const double, const double);

	//! Set this matrix column from the given 3d vector
	GLC_Matrix4x4& setColumn(int index, const GLC_Vector3d& vector);

	//! Optimise the usage of this matrix (Genral, Direct, Identity)
	inline GLC_Matrix4x4& optimise(bool force= false);

//@}

//////////////////////////////////////////////////////////////////////
//! Private services Functions
//////////////////////////////////////////////////////////////////////
private:

	//! Return true if the index (argument) is in the diagonal of this matrix
	inline bool isInDiagonal(const int index) const
	{
		if ((index == 0) || (index == 5) || (index == 10) || (index == 15))
			return true;
		else
			return false;
	}

	//! Return the determinant of this matrix cell given from 2 int
	inline double getDeterminantLC(const int, const int) const;

	//! Compute Sub 3X3 matrix given by 2 int and set the given double pointeur
	inline void getSubMat(const int, const int, double *) const;

	//! Return the transpose matrix of this matrix
	inline GLC_Matrix4x4 getTranspose(void) const;

	//! Return the co-matrix of this matrix
	inline GLC_Matrix4x4 getCoMat4x4(void) const;



//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:

	//! Number of elements of this matrix
	enum {TAILLEMAT4X4 = 16};

	//! Matrix size
	enum {DIMMAT4X4 = 4};

	//! Matrix row first array
	double m_Matrix[TAILLEMAT4X4];

	//! the type of this matrix
	int m_Type;

/*
the matrix :
					a[00] a[04] a[08] a[12]

					a[01] a[05] a[09] a[13]

					a[02] a[06] a[10] a[14]

					a[03] a[07] a[11] a[15]
 */
//					Tx = 12,	Ty = 13,	Tz = 14

};


//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////

GLC_Matrix4x4::GLC_Matrix4x4()
: m_Type(Identity)
{
	setToIdentity();
}

GLC_Matrix4x4::GLC_Matrix4x4(const float *Tableau)
: m_Type(General)
{

	for (int i=0; i < TAILLEMAT4X4; i++)
	{
		m_Matrix[i]= static_cast<double>(Tableau[i]);
	}
}
GLC_Matrix4x4::GLC_Matrix4x4(const GLC_Vector3d &Vect, const double &dAngleRad)
: m_Type(Direct)
{
	setToIdentity();
	setMatRot(Vect, dAngleRad);
}

GLC_Matrix4x4::GLC_Matrix4x4(const GLC_Vector3d &Vect1, const GLC_Vector3d &Vect2)
: m_Type(Direct)
{
	setToIdentity();
	setMatRot(Vect1, Vect2);
}

GLC_Matrix4x4 GLC_Matrix4x4::operator * (const GLC_Matrix4x4 &Mat) const
{
	if (m_Type == Identity)
	{
		return Mat;
	}
	else if (Mat.m_Type == Identity)
	{
		return *this;
	}

	int Colonne;
	int Ligne;
	int i;
	double ValInt;

	int IndexInt;

	GLC_Matrix4x4 MatResult;
	for (Ligne= 0; Ligne < DIMMAT4X4; Ligne++)
	{
		for (Colonne=0; Colonne < DIMMAT4X4; Colonne++)
		{
			ValInt= 0.0;
			IndexInt= Colonne * DIMMAT4X4;

			for (i= 0; i < DIMMAT4X4; i++)
			{
				ValInt+= m_Matrix[ (i * DIMMAT4X4) + Ligne] * Mat.m_Matrix[ IndexInt + i];
			}
			MatResult.m_Matrix[ IndexInt + Ligne]= ValInt;
		}
	}
	if ((m_Type == Indirect) || (Mat.m_Type == Indirect))
	{
		MatResult.m_Type= Indirect;
	}
	else
	{
		MatResult.m_Type= m_Type & Mat.m_Type;
	}

	return MatResult;
}

GLC_Matrix4x4& GLC_Matrix4x4::operator = (const GLC_Matrix4x4 &matrix)
{
	m_Type= matrix.m_Type;
	memcpy(m_Matrix, matrix.m_Matrix, sizeof(double) * 16);

	return *this;
}

GLC_Vector3d GLC_Matrix4x4::operator * (const GLC_Vector3d &Vect) const
{
	double ValInt;
	int i;
	GLC_Vector3d VectResult;
	double mat[4];

	for (int Index= 0; Index < DIMMAT4X4; Index++)
	{
		ValInt= 0.0;
		for (i= 0; i < DIMMAT4X4 - 1; i++)
		{
			ValInt+= m_Matrix[(i * DIMMAT4X4) + Index] * Vect.m_Vector[i];
		}
		ValInt+= m_Matrix[(3 * DIMMAT4X4) + Index];
		mat[Index]= ValInt;
	}

	double invW= 1.0;
	if (fabs(mat[3]) > 0.00001)
	{
		invW/= mat[3];
	}
	VectResult.m_Vector[0]= mat[0] * invW;
	VectResult.m_Vector[1]= mat[1] * invW;
	VectResult.m_Vector[2]= mat[2] * invW;


	return VectResult;
}

bool GLC_Matrix4x4::operator==(const GLC_Matrix4x4& mat) const
{
	bool result= true;
	int i= 0;
	while (result && (i < TAILLEMAT4X4))
	{
		result= (qFuzzyCompare(m_Matrix[i], mat.m_Matrix[i]));
		++i;
	}
	return result;
}

GLC_Matrix4x4 GLC_Matrix4x4::rotationMatrix() const
{
	GLC_Matrix4x4 result(*this);
	const double invScaleX= 1.0 / scalingX();
	const double invScaleY= 1.0 / scalingY();
	const double invScaleZ= 1.0 / scalingZ();
	result.m_Matrix[0]= result.m_Matrix[0] * invScaleX;
	result.m_Matrix[1]= result.m_Matrix[1] * invScaleX;
	result.m_Matrix[2]= result.m_Matrix[2] * invScaleX;

	result.m_Matrix[4]= result.m_Matrix[4] * invScaleY;
	result.m_Matrix[5]= result.m_Matrix[5] * invScaleY;
	result.m_Matrix[6]= result.m_Matrix[6] * invScaleY;

	result.m_Matrix[8]= result.m_Matrix[8] * invScaleZ;
	result.m_Matrix[9]= result.m_Matrix[9] * invScaleZ;
	result.m_Matrix[10]= result.m_Matrix[10] * invScaleZ;

	result.m_Matrix[12]= 0.0; result.m_Matrix[13]= 0.0; result.m_Matrix[14]= 0.0;
	result.m_Matrix[3]= 0.0; result.m_Matrix[7]= 0.0; result.m_Matrix[11]= 0.0;
	result.m_Matrix[15]= 1.0;

	result.m_Type= General;

	return result;
}

GLC_Matrix4x4 GLC_Matrix4x4::isometricMatrix() const
{
	GLC_Matrix4x4 result(*this);
	const double invScaleX= 1.0 / scalingX();
	const double invScaleY= 1.0 / scalingY();
	const double invScaleZ= 1.0 / scalingZ();
	result.m_Matrix[0]= result.m_Matrix[0] * invScaleX;
	result.m_Matrix[1]= result.m_Matrix[1] * invScaleX;
	result.m_Matrix[2]= result.m_Matrix[2] * invScaleX;

	result.m_Matrix[4]= result.m_Matrix[4] * invScaleY;
	result.m_Matrix[5]= result.m_Matrix[5] * invScaleY;
	result.m_Matrix[6]= result.m_Matrix[6] * invScaleY;

	result.m_Matrix[8]= result.m_Matrix[8] * invScaleZ;
	result.m_Matrix[9]= result.m_Matrix[9] * invScaleZ;
	result.m_Matrix[10]= result.m_Matrix[10] * invScaleZ;

	result.m_Type= General;

	return result;
}

GLC_Matrix4x4& GLC_Matrix4x4::setMatRot(const GLC_Vector3d &Vect, const double &dAngleRad)
{
	// Normalize the vector
	GLC_Vector3d VectRot(Vect);
	VectRot.normalize();

	// Code optimisation
	const double SinAngleSur2= sin(dAngleRad / 2.0);

	// Quaternion computation
	const double q0= cos(dAngleRad / 2);
	const double q1= VectRot.m_Vector[0] * SinAngleSur2;
	const double q2= VectRot.m_Vector[1] * SinAngleSur2;
	const double q3= VectRot.m_Vector[2] * SinAngleSur2;

	// Code optimisation
	const double q0Carre= (q0 * q0);
	const double q1Carre= (q1 * q1);
	const double q2Carre= (q2 * q2);
	const double q3Carre= (q3 * q3);

	m_Matrix[0]= q0Carre + q1Carre - q2Carre - q3Carre;
	m_Matrix[1]= 2.0 * (q1 *q2 + q0 * q3);
	m_Matrix[2]= 2.0 * (q1 * q3 - q0 * q2);
	m_Matrix[3]= 0.0;
	m_Matrix[4]= 2.0 * (q1 * q2 - q0 * q3);
	m_Matrix[5]= q0Carre + q2Carre - q3Carre - q1Carre;
	m_Matrix[6]= 2.0 * (q2 * q3 + q0 * q1);
	m_Matrix[7]= 0.0;
	m_Matrix[8]= 2.0 * (q1 * q3 + q0 * q2);
	m_Matrix[9]= 2.0 * (q2 * q3 - q0 * q1);
	m_Matrix[10]= q0Carre + q3Carre - q1Carre - q2Carre;
	m_Matrix[11]= 0.0;

	m_Matrix[12]= 0.0;	//TX
	m_Matrix[13]= 0.0;	//TY
	m_Matrix[14]= 0.0;	//TZ
	m_Matrix[15]= 1.0;

	m_Type= Direct;

	return *this;
}

GLC_Matrix4x4& GLC_Matrix4x4::setMatRot(const GLC_Vector3d &Vect1, const GLC_Vector3d &Vect2)
{

	// Compute rotation matrix
	const GLC_Vector3d VectAxeRot(Vect1 ^ Vect2);
	// Check if rotation vector axis is not null
	if (!VectAxeRot.isNull())
	{  // Ok, vector not null
		const double Angle= acos(Vect1 * Vect2);
		setMatRot(VectAxeRot, Angle);
	}

	return *this;
}

GLC_Matrix4x4& GLC_Matrix4x4::setMatTranslate(const GLC_Vector3d &Vect)
{
	m_Matrix[0]= 1.0; m_Matrix[4]= 0.0; m_Matrix[8]=  0.0; m_Matrix[12]= Vect.m_Vector[0];
	m_Matrix[1]= 0.0; m_Matrix[5]= 1.0; m_Matrix[9]=  0.0; m_Matrix[13]= Vect.m_Vector[1];
	m_Matrix[2]= 0.0; m_Matrix[6]= 0.0; m_Matrix[10]= 1.0; m_Matrix[14]= Vect.m_Vector[2];
	m_Matrix[3]= 0.0; m_Matrix[7]= 0.0; m_Matrix[11]= 0.0; m_Matrix[15]= 1.0;

	m_Type= Direct;

	return *this;
}

GLC_Matrix4x4& GLC_Matrix4x4::setMatTranslate(const double Tx, const double Ty, const double Tz)
{
	m_Matrix[0]= 1.0; m_Matrix[4]= 0.0; m_Matrix[8]=  0.0; m_Matrix[12]= Tx;
	m_Matrix[1]= 0.0; m_Matrix[5]= 1.0; m_Matrix[9]=  0.0; m_Matrix[13]= Ty;
	m_Matrix[2]= 0.0; m_Matrix[6]= 0.0; m_Matrix[10]= 1.0; m_Matrix[14]= Tz;
	m_Matrix[3]= 0.0; m_Matrix[7]= 0.0; m_Matrix[11]= 0.0; m_Matrix[15]= 1.0;

	m_Type= Direct;

	return *this;
}

GLC_Matrix4x4& GLC_Matrix4x4::setMatScaling(const double sX, const double sY, const double sZ)
{
	m_Matrix[0]= sX; m_Matrix[4]= 0.0; m_Matrix[8]=  0.0; m_Matrix[12]= 0.0;
	m_Matrix[1]= 0.0; m_Matrix[5]= sY; m_Matrix[9]=  0.0; m_Matrix[13]= 0.0;
	m_Matrix[2]= 0.0; m_Matrix[6]= 0.0; m_Matrix[10]= sZ; m_Matrix[14]= 0.0;
	m_Matrix[3]= 0.0; m_Matrix[7]= 0.0; m_Matrix[11]= 0.0; m_Matrix[15]= 1.0;

	m_Type= General;

	return *this;
}


GLC_Matrix4x4& GLC_Matrix4x4::invert(void)
{
	const double det= determinant();

	// Test if the inverion is possible
	if (det == 0.0f) return *this;

	const double invDet = 1.0 / det;
	GLC_Matrix4x4 TCoMat= getCoMat4x4().getTranspose();

	for (int i= 0; i < TAILLEMAT4X4; i++)
	{
		m_Matrix[i]= TCoMat.m_Matrix[i] * invDet;
	}

	return *this;
}

GLC_Matrix4x4& GLC_Matrix4x4::setToIdentity()
{
	m_Matrix[0]= 1.0; m_Matrix[4]= 0.0; m_Matrix[8]=  0.0; m_Matrix[12]= 0.0;
	m_Matrix[1]= 0.0; m_Matrix[5]= 1.0; m_Matrix[9]=  0.0; m_Matrix[13]= 0.0;
	m_Matrix[2]= 0.0; m_Matrix[6]= 0.0; m_Matrix[10]= 1.0; m_Matrix[14]= 0.0;
	m_Matrix[3]= 0.0; m_Matrix[7]= 0.0; m_Matrix[11]= 0.0; m_Matrix[15]= 1.0;

	m_Type= Identity;

	return *this;
}

GLC_Matrix4x4& GLC_Matrix4x4::transpose(void)
{
	GLC_Matrix4x4 MatT(m_Matrix);
	int IndexOrigine;
	int IndexTrans;
	for (int Colonne= 0; Colonne < DIMMAT4X4; Colonne++)
	{
		for (int Ligne=0 ; Ligne < DIMMAT4X4; Ligne++)
		{
			IndexOrigine= (Colonne * DIMMAT4X4) + Ligne;
			IndexTrans= (Ligne * DIMMAT4X4) + Colonne;

			MatT.m_Matrix[IndexTrans]= m_Matrix[IndexOrigine];
		}
	}

	// Load the transposed matrix in this matrix
	memcpy(m_Matrix, MatT.m_Matrix, sizeof(double) * 16);

	return *this;
}

GLC_Matrix4x4& GLC_Matrix4x4::optimise(bool force)
{
	if (force || (m_Type == General))
	{
		bool identityVal= (m_Matrix[0] == 1.0f) && (m_Matrix[4] == 0.0f) && (m_Matrix[8] ==  0.0f) && (m_Matrix[12] == 0.0f);
		identityVal= identityVal && (m_Matrix[1] == 0.0f) && (m_Matrix[5] == 1.0f) && (m_Matrix[9] ==  0.0f) && (m_Matrix[13] == 0.0);
		identityVal= identityVal && (m_Matrix[2] == 0.0f) && (m_Matrix[6] == 0.0f) && (m_Matrix[10] == 1.0f) && (m_Matrix[14] == 0.0);
		identityVal= identityVal && (m_Matrix[3] == 0.0f) && (m_Matrix[7] == 0.0f) && (m_Matrix[11] == 0.0f) && (m_Matrix[15] == 1.0f);
		if (identityVal)
		{
			m_Type= Identity;
		}
		else
		{
			if (determinant() > 0)
			{
				m_Type= Direct;
			}
			else
			{
				m_Type= Indirect;
			}
		}
	}
	return *this;
}

double GLC_Matrix4x4::determinant(void) const
{
	double Determinant= 0.0;
	double SubMat3x3[9];
	int Signe= 1;

	for (int Colonne= 0; Colonne < DIMMAT4X4; Colonne++, Signe*= -1)
	{
		getSubMat(0, Colonne, SubMat3x3);
		Determinant+= Signe * m_Matrix[Colonne * DIMMAT4X4] * getDeterminant3x3(SubMat3x3);
	}

	return Determinant;

}

double GLC_Matrix4x4::getDeterminantLC(const int Ligne, const int Colonne) const
{
	double Mat3x3[9];
	double Determinant;

	getSubMat(Ligne, Colonne, Mat3x3);

	if ( 0 == ((Ligne + Colonne) % 2)) // Even number
		Determinant= m_Matrix[(Colonne + DIMMAT4X4) + Ligne] * getDeterminant3x3(Mat3x3);
	else
		Determinant= - m_Matrix[(Colonne + DIMMAT4X4) + Ligne] * getDeterminant3x3(Mat3x3);

	return Determinant;
}

void GLC_Matrix4x4::getSubMat(const int Ligne, const int Colonne, double *ResultMat) const
{

	int LigneResult;
	int ColonneResult;
	int IndexOrigine;
	int IndexResult;

	for (int ColonneOrigine= 0; ColonneOrigine < DIMMAT4X4; ColonneOrigine++)
	{
		if (ColonneOrigine != Colonne)
		{
			if (ColonneOrigine < Colonne)
				ColonneResult= ColonneOrigine;
			else
				ColonneResult= ColonneOrigine - 1;

			for (int LigneOrigine= 0; LigneOrigine < DIMMAT4X4; LigneOrigine++)
			{
				if (LigneOrigine != Ligne)
				{
					if (LigneOrigine < Ligne)
						LigneResult= LigneOrigine;
					else
						LigneResult= LigneOrigine - 1;
					IndexOrigine= (ColonneOrigine * DIMMAT4X4) + LigneOrigine;
					IndexResult= (ColonneResult * (DIMMAT4X4 - 1)) + LigneResult;

					ResultMat[IndexResult]= m_Matrix[IndexOrigine];
				}
			}
		}
	}
}

GLC_Matrix4x4 GLC_Matrix4x4::getTranspose(void) const
{
	GLC_Matrix4x4 MatT(m_Matrix);
	int IndexOrigine;
	int IndexTrans;
	for (int Colonne= 0; Colonne < DIMMAT4X4; Colonne++)
	{
		for (int Ligne=0 ; Ligne < DIMMAT4X4; Ligne++)
		{
			IndexOrigine= (Colonne * DIMMAT4X4) + Ligne;
			IndexTrans= (Ligne * DIMMAT4X4) + Colonne;

			MatT.m_Matrix[IndexTrans]= m_Matrix[IndexOrigine];
		}
	}

	MatT.m_Type= m_Type;
	return MatT;
}

GLC_Matrix4x4 GLC_Matrix4x4::getCoMat4x4(void) const
{
	GLC_Matrix4x4 CoMat(m_Matrix);
	double SubMat3x3[9];
	int Index;

	for (int Colonne= 0; Colonne < DIMMAT4X4; Colonne++)
	{
		for (int Ligne=0 ; Ligne < DIMMAT4X4; Ligne++)
		{
			getSubMat(Ligne, Colonne, SubMat3x3);
			Index= (Colonne * DIMMAT4X4) + Ligne;
			if (((Colonne + Ligne + 2) % 2) == 0) // Even Number
				CoMat.m_Matrix[Index]= getDeterminant3x3(SubMat3x3);
			else
				CoMat.m_Matrix[Index]= -getDeterminant3x3(SubMat3x3);
		}
	}


	CoMat.m_Type= General;
	return CoMat;
}


#endif /*GLC_MATRIX4X4_H_*/
