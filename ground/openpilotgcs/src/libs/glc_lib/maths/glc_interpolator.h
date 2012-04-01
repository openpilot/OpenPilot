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

//! \file glc_interpolator.h interface for the GLC_Interpolator class.

#ifndef GLC_INTERPOLATOR_H_
#define GLC_INTERPOLATOR_H_

#include "glc_vector3d.h"
#include "glc_matrix4x4.h"

#include "../glc_config.h"

// Types d'interpolation
enum INTERPOL_TYPE
{
	INTERPOL_LINEAIRE,
	INTERPOL_ANGULAIRE,
	INTERPOL_HOMOTETIE
};

//////////////////////////////////////////////////////////////////////
//! \class GLC_Interpolator
/*! \brief GLC_Interpolator : Matrix interpolation class*/

/*! An GLC_Interpolator is a class used to interpolate 2 4D matrix*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Interpolator
{

public:
	//! Default linear interpolation constructor
	GLC_Interpolator();

//////////////////////////////////////////////////////////////////////
// Set Function
//////////////////////////////////////////////////////////////////////
public:
	//! Set interpolation matrix
	void SetInterpolMat(int NbrPas, const GLC_Vector3d &VectDepart, const GLC_Vector3d &VectArrive
		, INTERPOL_TYPE Interpolation = INTERPOL_LINEAIRE);

	//! Set interpolation type
	void SetType(INTERPOL_TYPE Interpolation);

	// Number of step
	void SetNbrPas(int NbrPas);

	//! Set start and end vector
	void SetVecteurs(const GLC_Vector3d &VectDepart, const GLC_Vector3d &VectArrive);

//////////////////////////////////////////////////////////////////////
// Get Function
//////////////////////////////////////////////////////////////////////
public:
	//! Return th interpolation matrix
	inline GLC_Matrix4x4 GetInterpolMat(void) const
	{return m_InterpolMat;}

//////////////////////////////////////////////////////////////////////
// Private services functions
//////////////////////////////////////////////////////////////////////
private:
	//! Compute interpolation matrix
	bool CalcInterpolMat(void);

	//! Compute linear interolation matrix
	bool CalcInterpolLineaireMat(void);

	//! Compute angular interpolation matrix
	bool CalcInterpolAngulaireMat(void);

//////////////////////////////////////////////////////////////////////
// Membres privés
//////////////////////////////////////////////////////////////////////
private:
	//! Start Point
	GLC_Point3d m_StartPoint;

	//! End Point
	GLC_Point3d m_EndPoint;

	//! Interpolation type
	INTERPOL_TYPE m_InterpolType;

	//! Interpolation step count
	int m_StepCount;

	//! Interpolation matrix
	GLC_Matrix4x4 m_InterpolMat;
};

#endif /*GLC_INTERPOLATOR_H_*/
