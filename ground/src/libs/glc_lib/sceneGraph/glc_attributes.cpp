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
//! \file glc_attributes.cpp implementation of the GLC_Attributes class.

#include "glc_attributes.h"

// Default constructor
GLC_Attributes::GLC_Attributes()
: m_AttributesHash()
{

}

// Copy Constructor
GLC_Attributes::GLC_Attributes(const GLC_Attributes& attr)
: m_AttributesHash(attr.m_AttributesHash)
{

}

// Destructor
GLC_Attributes::~GLC_Attributes()
{

}
