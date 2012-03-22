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

//! \file glc_interpolator.cpp implementation of the GLC_Interpolator class.

#include "glc_interpolator.h"

using namespace glc;

GLC_Interpolator::GLC_Interpolator()
: m_InterpolType(INTERPOL_LINEAIRE)
, m_StepCount(1)
{

}

//////////////////////////////////////////////////////////////////////
// Set Function
//////////////////////////////////////////////////////////////////////
void GLC_Interpolator::SetInterpolMat(int NbrPas, const GLC_Vector3d &VectDepart, const GLC_Vector3d &VectArrive
								   , INTERPOL_TYPE Interpolation)
{
	m_InterpolType= Interpolation;
	if (NbrPas != 0)
	m_StepCount= NbrPas;

	m_StartPoint= VectDepart;
	m_EndPoint= VectArrive;

	CalcInterpolMat();
}

void GLC_Interpolator::SetType(INTERPOL_TYPE Interpolation)
{
	if (m_InterpolType != Interpolation)
	{
		m_InterpolType= Interpolation;

		CalcInterpolMat();
	}
}

void GLC_Interpolator::SetNbrPas(int NbrPas)
{

	if ((NbrPas != 0) && (m_StepCount != NbrPas))
	{
		m_StepCount= NbrPas;

		CalcInterpolMat();
	}
}

void GLC_Interpolator::SetVecteurs(const GLC_Vector3d &VectDepart, const GLC_Vector3d &VectArrive)
{
	m_StartPoint= VectDepart;
	m_EndPoint= VectArrive;


	CalcInterpolMat();

}

//////////////////////////////////////////////////////////////////////
// Private sevices functions
//////////////////////////////////////////////////////////////////////

bool GLC_Interpolator::CalcInterpolMat(void)
{

	if (m_StartPoint != m_EndPoint)
	{
		switch (m_InterpolType)
		{
		case INTERPOL_LINEAIRE:
			return CalcInterpolLineaireMat();
			break;

		case INTERPOL_ANGULAIRE:
			return CalcInterpolAngulaireMat();
			break;

		case INTERPOL_HOMOTETIE:
			return false;
			break;

		default:
			return false;
		}
	}
	else return false;

}


bool GLC_Interpolator::CalcInterpolLineaireMat(void)
{

	const GLC_Vector3d VectTrans= (m_EndPoint - m_StartPoint) * (1.0 / m_StepCount);
	if(VectTrans.isNull())
	{
		m_InterpolMat.setToIdentity();
		return false;
	}
	else
	{
		m_InterpolMat.setMatTranslate(VectTrans);
		return true;
	}
}

bool GLC_Interpolator::CalcInterpolAngulaireMat(void)
{

	const GLC_Vector3d AxeRot(m_StartPoint ^ m_EndPoint);

	const double Angle= m_EndPoint.angleWithVect(m_StartPoint) / m_StepCount;

	if (qFuzzyCompare(Angle, 0.0))
	{
		m_InterpolMat.setToIdentity();
		return false;
	}
	else
	{
		m_InterpolMat.setMatRot( AxeRot, Angle);
		return true;
	}
}



