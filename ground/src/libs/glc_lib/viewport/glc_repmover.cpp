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

//! \file glc_repmover.cpp Implementation of the GLC_RepMover class.

#include "glc_repmover.h"
#include "glc_viewport.h"


GLC_RepMover::GLC_RepMover(GLC_Viewport* pViewport)
: m_pViewport(pViewport)
, m_MainColor()
{

}
// Copy constructor
GLC_RepMover::GLC_RepMover(const GLC_RepMover& repMover)
: m_pViewport(repMover.m_pViewport)
, m_MainColor(repMover.m_MainColor)
{

}


GLC_RepMover::~GLC_RepMover()
{

}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Representation OpenGL Execution
void GLC_RepMover::glExecute()
{
	// Call virtual draw function
	glDraw();
}
