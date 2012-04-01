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

//! \file glc_mover.cpp Implementation of the GLC_Mover class.

#include "glc_userinput.h"

GLC_UserInput::GLC_UserInput(int x, int y)
: m_X(x)
, m_Y(y)
, m_NormalyzeX(0.0)
, m_NormalyzeY(0.0)
, m_Translation()
, m_Rotation(0.0)
, m_ScaleFactor(1.0)
, m_TransformationIsSet(false)
{

}

GLC_UserInput::~GLC_UserInput()
{

}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
void GLC_UserInput::setPosition(int x, int y)
{
	m_X= x;
	m_Y= y;
}

void GLC_UserInput::setNormalyzeTouchCenterPosition(double x, double y)
{
	m_NormalyzeX= x;
	m_NormalyzeY= y;
}

void GLC_UserInput::setTransformation(const GLC_Vector2d& translation, double rotation, double scaleFactor)
{
	m_Translation= translation;
	m_Rotation= rotation;
	m_ScaleFactor= scaleFactor;

	m_TransformationIsSet= true;
}
