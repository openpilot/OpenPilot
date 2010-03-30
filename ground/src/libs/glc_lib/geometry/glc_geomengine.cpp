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

//! \file glc_geomengine.cpp Implementation of the GLC_GeomEngine class.

#include "glc_geomengine.h"
#include "../glc_state.h"

GLC_GeomEngine::GLC_GeomEngine()
: m_VboId(0)
{


}
// Copy constructor
GLC_GeomEngine::GLC_GeomEngine(const GLC_GeomEngine&)
: m_VboId(0)
{

}

// Destructor
GLC_GeomEngine::~GLC_GeomEngine()
{
	clear();
}
//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Clear the engine
void GLC_GeomEngine::clear()
{
	if (0 != m_VboId)
	{
		glDeleteBuffers(1, &m_VboId);
		m_VboId= 0;
	}
}
