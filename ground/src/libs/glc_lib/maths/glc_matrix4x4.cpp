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


//! \file glc_matrix4x4.cpp implementation of the GLC_Matrix4x4 class.

#include "glc_matrix4x4.h"

#include <QtDebug>

using namespace glc;
//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////

// Default Constructor
GLC_Matrix4x4::GLC_Matrix4x4()
{
	matrix[0]= 1.0; matrix[4]= 0.0; matrix[8]=  0.0; matrix[12]= 0.0;
	matrix[1]= 0.0; matrix[5]= 1.0; matrix[9]=  0.0; matrix[13]= 0.0;
	matrix[2]= 0.0; matrix[6]= 0.0; matrix[10]= 1.0; matrix[14]= 0.0;
	matrix[3]= 0.0; matrix[7]= 0.0; matrix[11]= 0.0; matrix[15]= 1.0;
}

// Construct a Matrix by copy
GLC_Matrix4x4::GLC_Matrix4x4(const GLC_Matrix4x4 &Mat)
{

	for (int i=0; i < TAILLEMAT4X4; i++)
	{
		matrix[i]= Mat.matrix[i];
	}
}


// Construct a Matrix by an array of 16 elements.
GLC_Matrix4x4::GLC_Matrix4x4(const double *Tableau)
{

	for (int i=0; i < TAILLEMAT4X4; i++)
	{
		matrix[i]= Tableau[i];
	}
}

// Construct a Matrix by an array of 16 elements.
GLC_Matrix4x4::GLC_Matrix4x4(const float *Tableau)
{

	for (int i=0; i < TAILLEMAT4X4; i++)
	{
		matrix[i]= static_cast<double>(Tableau[i]);
	}
}

//////////////////////////////////////////////////////////////////////
// Operator Overload
//////////////////////////////////////////////////////////////////////

// Matrix cross product
GLC_Matrix4x4 GLC_Matrix4x4::operator * (const GLC_Matrix4x4 &Mat) const
{
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
				ValInt+= matrix[ (i * DIMMAT4X4) + Ligne] * Mat.matrix[ IndexInt + i];
			}
			MatResult.matrix[ IndexInt + Ligne]= ValInt;
		}
	}
	return MatResult;
}

// Vector transformation
GLC_Vector4d GLC_Matrix4x4::operator * (const GLC_Vector4d &Vect) const
{
	double ValInt;
	int i;
	GLC_Vector4d VectResult;

	for (int Index= 0; Index < DIMMAT4X4; Index++)
	{
		ValInt= 0.0;
		for (i= 0; i < DIMMAT4X4; i++)
		{
			ValInt+= matrix[(i * DIMMAT4X4) + Index] * Vect.vector[i];
		}
		VectResult.vector[Index]= ValInt;
	}

	if (VectResult.vector[3] != 1.0)
	{
		//qDebug("Matrice4x4::operator * : Changement de W");
		VectResult.normalizeW();
	}

	return VectResult;
}

// Return true if the 2 matrix are equals
bool GLC_Matrix4x4::operator==(const GLC_Matrix4x4& mat) const
{
	bool result= true;
	int i= 0;
	while (result and (i < TAILLEMAT4X4))
	{
		result= (matrix[i] == mat.matrix[i]);
		++i;
	}
	return result;
}


//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Compute matrix determinant
double GLC_Matrix4x4::determinant(void) const
{
	double Determinant= 0.0;
	double SubMat3x3[9];
	int Signe= 1;

	for (int Colonne= 0; Colonne < DIMMAT4X4; Colonne++, Signe*= -1)
	{
		getSubMat(0, Colonne, SubMat3x3);
		Determinant+= Signe * matrix[Colonne * DIMMAT4X4] * getDeterminant3x3(SubMat3x3);
	}

	return Determinant;

}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set matrix to a rotation matrix define by a vector and an angle in radians
GLC_Matrix4x4& GLC_Matrix4x4::setMatRot(const GLC_Vector4d &Vect, const double &dAngleRad)
{
	// Normalize the vector
	GLC_Vector4d VectRot(Vect);
	VectRot.setNormal(1);

	// Code optimisation
	const double SinAngleSur2= sin(dAngleRad / 2.0);

	// Quaternion computation
	const double q0= cos(dAngleRad / 2);
	const double q1= VectRot.vector[0] * SinAngleSur2;
	const double q2= VectRot.vector[1] * SinAngleSur2;
	const double q3= VectRot.vector[2] * SinAngleSur2;

	// Code optimisation
	const double q0Carre= (q0 * q0);
	const double q1Carre= (q1 * q1);
	const double q2Carre= (q2 * q2);
	const double q3Carre= (q3 * q3);

	matrix[0]= q0Carre + q1Carre - q2Carre - q3Carre;
	matrix[1]= 2.0 * (q1 *q2 + q0 * q3);
	matrix[2]= 2.0 * (q1 * q3 - q0 * q2);
	matrix[3]= 0.0;
	matrix[4]= 2.0 * (q1 * q2 - q0 * q3);
	matrix[5]= q0Carre + q2Carre - q3Carre - q1Carre;
	matrix[6]= 2.0 * (q2 * q3 + q0 * q1);
	matrix[7]= 0.0;
	matrix[8]= 2.0 * (q1 * q3 + q0 * q2);
	matrix[9]= 2.0 * (q2 * q3 - q0 * q1);
	matrix[10]= q0Carre + q3Carre - q1Carre - q2Carre;
	matrix[11]= 0.0;

	matrix[12]= 0.0;	//TX
	matrix[13]= 0.0;	//TY
	matrix[14]= 0.0;	//TZ
	matrix[15]= 1.0;

	return *this;
}

// Set matrix to a rotation matrix define by 2 vectors
GLC_Matrix4x4& GLC_Matrix4x4::setMatRot(const GLC_Vector4d &Vect1, const GLC_Vector4d &Vect2)
{

	// Compute rotation matrix
	const GLC_Vector4d VectAxeRot(Vect1 ^ Vect2);
	// Check if rotation vector axis is not null
	if (!VectAxeRot.isNull())
	{  // Ok, vector not null
		const double Angle= acos(Vect1 * Vect2);
		setMatRot(VectAxeRot, Angle);
	}

	return *this;
}

// Set Matrix to a translation matrix by a vector
GLC_Matrix4x4& GLC_Matrix4x4::setMatTranslate(const GLC_Vector4d &Vect)
{
	matrix[0]= 1.0; matrix[4]= 0.0; matrix[8]=  0.0; matrix[12]= Vect.vector[0];
	matrix[1]= 0.0; matrix[5]= 1.0; matrix[9]=  0.0; matrix[13]= Vect.vector[1];
	matrix[2]= 0.0; matrix[6]= 0.0; matrix[10]= 1.0; matrix[14]= Vect.vector[2];
	matrix[3]= 0.0; matrix[7]= 0.0; matrix[11]= 0.0; matrix[15]= 1.0;

	return *this;
}

// Set Matrix to a translation matrix by 3 coordinates
GLC_Matrix4x4& GLC_Matrix4x4::setMatTranslate(const double Tx, const double Ty, const double Tz)
{
	matrix[0]= 1.0; matrix[4]= 0.0; matrix[8]=  0.0; matrix[12]= Tx;
	matrix[1]= 0.0; matrix[5]= 1.0; matrix[9]=  0.0; matrix[13]= Ty;
	matrix[2]= 0.0; matrix[6]= 0.0; matrix[10]= 1.0; matrix[14]= Tz;
	matrix[3]= 0.0; matrix[7]= 0.0; matrix[11]= 0.0; matrix[15]= 1.0;

	return *this;
}

// Set Matrix to a scaling matrix define by 3 double
GLC_Matrix4x4& GLC_Matrix4x4::setMatScaling(const double sX, const double sY, const double sZ)
{
	matrix[0]= sX; matrix[4]= 0.0; matrix[8]=  0.0; matrix[12]= 0.0;
	matrix[1]= 0.0; matrix[5]= sY; matrix[9]=  0.0; matrix[13]= 0.0;
	matrix[2]= 0.0; matrix[6]= 0.0; matrix[10]= sZ; matrix[14]= 0.0;
	matrix[3]= 0.0; matrix[7]= 0.0; matrix[11]= 0.0; matrix[15]= 1.0;

	return *this;
}


// Reverse the Matrix
GLC_Matrix4x4& GLC_Matrix4x4::invert(void)
{
	const double det= determinant();

	// Verifie si l'inversion est possible
	if (qFuzzyCompare(det, 0.0))
	{
		qDebug() << "Det < EPSILON";
		return *this;
	}

	const double invDet = 1.0 / det;
	GLC_Matrix4x4 TCoMat= getCoMat4x4().getTranspose();

	for (int i= 0; i < TAILLEMAT4X4; i++)
	{
		matrix[i]= TCoMat.matrix[i] * invDet;
	}

	return *this;
}

// Set the matrix to identify matrix
GLC_Matrix4x4& GLC_Matrix4x4::setToIdentity()
{
	for (int i= 0; i < TAILLEMAT4X4; i++)
	{
		if (isDiagonal(i))
			matrix[i]= 1;
		else
			matrix[i]= 0;
	}

	return *this;
}

// Set the matrix by its transpose
GLC_Matrix4x4& GLC_Matrix4x4::transpose(void)
{
	GLC_Matrix4x4 MatT(matrix);
	int IndexOrigine;
	int IndexTrans;
	for (int Colonne= 0; Colonne < DIMMAT4X4; Colonne++)
	{
		for (int Ligne=0 ; Ligne < DIMMAT4X4; Ligne++)
		{
			IndexOrigine= (Colonne * DIMMAT4X4) + Ligne;
			IndexTrans= (Ligne * DIMMAT4X4) + Colonne;

			MatT.matrix[IndexTrans]= matrix[IndexOrigine];
		}
	}

	// Load the transposed in matrix in this matrix
	for (int Index= 0; Index < TAILLEMAT4X4; Index++)
		matrix[Index]= MatT.matrix[Index];

	return *this;
}

// Set the matrix with Euler angle
GLC_Matrix4x4& GLC_Matrix4x4::fromEuler(const double angle_x, const double angle_y, const double angle_z)
{
    const double A= cos(angle_x);
    const double B= sin(angle_x);
    const double C= cos(angle_y);
    const double D= sin(angle_y);
    const double E= cos(angle_z);
    const double F= sin(angle_z);

    const double AD= A * D;
    const double BD= B * D;

    matrix[0]  = C * E;
    matrix[4]  = -C * F;
    matrix[8]  = -D;
    matrix[1]  = -BD * E + A * F;
    matrix[5]  = BD * F + A * E;
    matrix[9]  = -B * C;
    matrix[2]  = AD * E + B * F;
    matrix[6]  = -AD * F + B * E;
    matrix[10] = A * C;

    matrix[12]=  0.0; matrix[13]= 0.0; matrix[14]= 0.0; matrix[3]= 0.0; matrix[7]= 0.0; matrix[11] = 0.0;
    matrix[15] =  1.0;

	return *this;
}



//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////

// Compute matrix determinant
double GLC_Matrix4x4::getDeterminantLC(const int Ligne, const int Colonne) const
{
	double Mat3x3[9];
	double Determinant;

	getSubMat(Ligne, Colonne, Mat3x3);

	if ( 0 == ((Ligne + Colonne) % 2)) // Even number
		Determinant= matrix[(Colonne + DIMMAT4X4) + Ligne] * getDeterminant3x3(Mat3x3);
	else
		Determinant= - matrix[(Colonne + DIMMAT4X4) + Ligne] * getDeterminant3x3(Mat3x3);

	return Determinant;
}

// Return a vector which contains radians Euler angle of the matrix
QVector<double> GLC_Matrix4x4::toEuler(void) const
{
	double angle_x;
	double angle_y;
	double angle_z;
	double tracex, tracey;
	angle_y= -asin(matrix[8]);
	double C= cos(angle_y);

	if (not qFuzzyCompare(C, 0.0)) // Gimball lock?
	{
		tracex= matrix[10] / C;
		tracey= - matrix[9] / C;
		angle_x= atan2( tracey, tracex);

		tracex= matrix[0] / C;
		tracey= - matrix[4] / C;
		angle_z= atan2( tracey, tracex);
	}
	else // Gimball lock?
	{
		angle_x= 0.0;
		tracex= matrix[5] / C;
		tracey= matrix[1] / C;
		angle_z= atan2( tracey, tracex);
	}
	QVector<double> result;
	result.append(fmod(angle_x, 2.0 * PI));
	result.append(fmod(angle_y, 2.0 * PI));
	result.append(fmod(angle_z, 2.0 * PI));

	return result;
}

// Return the matrix string
QString GLC_Matrix4x4::toString() const
{
	QString result;
	for (int i= 0; i < DIMMAT4X4; ++i)
	{
		result+= (QString::number(matrix[0 + i])) + QString(" ");
		result+= (QString::number(matrix[4 + i])) + QString(" ");
		result+= (QString::number(matrix[8 + i])) + QString(" ");
		result+= (QString::number(matrix[12 + i])) + QString("\n");
	}
	result.remove(result.size() - 1, 1);
	return result;
}


// Compute Sub 3X3 matrix
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

					ResultMat[IndexResult]= matrix[IndexOrigine];
				}
			}
		}
	}
}

// Return determinant of a 3x3 matrix
double GLC_Matrix4x4::getDeterminant3x3(const double *Mat3x3) const
{
	double Determinant;

	Determinant= Mat3x3[0] * ( Mat3x3[4] * Mat3x3[8] - Mat3x3[7] * Mat3x3[5]);
	Determinant+= - Mat3x3[3] * ( Mat3x3[1] * Mat3x3[8] - Mat3x3[7] * Mat3x3[2]);
	Determinant+= Mat3x3[6] * ( Mat3x3[1] * Mat3x3[5] - Mat3x3[4] * Mat3x3[2]);

	return Determinant;
}

// Return transposed matrix
GLC_Matrix4x4 GLC_Matrix4x4::getTranspose(void) const
{
	GLC_Matrix4x4 MatT(matrix);
	int IndexOrigine;
	int IndexTrans;
	for (int Colonne= 0; Colonne < DIMMAT4X4; Colonne++)
	{
		for (int Ligne=0 ; Ligne < DIMMAT4X4; Ligne++)
		{
			IndexOrigine= (Colonne * DIMMAT4X4) + Ligne;
			IndexTrans= (Ligne * DIMMAT4X4) + Colonne;

			MatT.matrix[IndexTrans]= matrix[IndexOrigine];
		}
	}

	return MatT;
}

// Return the comatrix
GLC_Matrix4x4 GLC_Matrix4x4::getCoMat4x4(void) const
{
	GLC_Matrix4x4 CoMat(matrix);
	double SubMat3x3[9];
	int Index;

	for (int Colonne= 0; Colonne < DIMMAT4X4; Colonne++)
	{
		for (int Ligne=0 ; Ligne < DIMMAT4X4; Ligne++)
		{
			getSubMat(Ligne, Colonne, SubMat3x3);
			Index= (Colonne * DIMMAT4X4) + Ligne;
			if (((Colonne + Ligne + 2) % 2) == 0) // Even Number
				CoMat.matrix[Index]= getDeterminant3x3(SubMat3x3);
			else
				CoMat.matrix[Index]= -getDeterminant3x3(SubMat3x3);
		}
	}

	return CoMat;
}
