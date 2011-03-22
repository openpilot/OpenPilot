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
//! \file glc_spacepartitioning.cpp implementation for the GLC_SpacePartitioning class.

#include "glc_spacepartitioning.h"
#include "glc_3dviewcollection.h"

#include <QtGlobal>

// Default constructor
GLC_SpacePartitioning::GLC_SpacePartitioning(GLC_3DViewCollection* pCollection)
: m_pCollection(pCollection)
{
	Q_ASSERT(m_pCollection != NULL);
}

// Copy constructor
GLC_SpacePartitioning::GLC_SpacePartitioning(const GLC_SpacePartitioning& spacePartitionning)
: m_pCollection(spacePartitionning.m_pCollection)
{

}

GLC_SpacePartitioning::~GLC_SpacePartitioning()
{

}
