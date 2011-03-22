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
//! \file glc_line3d.cpp Implementation of the GLC_Line3d class.

#include "glc_line3d.h"

GLC_Line3d::GLC_Line3d()
: m_Point()
, m_Vector()
{

}

GLC_Line3d::GLC_Line3d(const GLC_Point3d& point, const GLC_Vector3d& vector)
: m_Point(point)
, m_Vector(vector)
{

}


GLC_Line3d::GLC_Line3d(const GLC_Line3d& line)
: m_Point(line.m_Point)
, m_Vector(line.m_Vector)
{

}


GLC_Line3d::~GLC_Line3d()
{

}
