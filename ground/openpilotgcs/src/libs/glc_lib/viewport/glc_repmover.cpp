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

//! \file glc_repmover.cpp Implementation of the GLC_RepMover class.

#include "glc_repmover.h"
#include "glc_viewport.h"


GLC_RepMover::GLC_RepMover(GLC_Viewport* pViewport)
: m_pViewport(pViewport)
, m_MainColor(Qt::black)
, m_Thickness(1.0)
, m_RenderProperties()
, m_pRepMoverInfo(NULL)
{

}
// Copy constructor
GLC_RepMover::GLC_RepMover(const GLC_RepMover& repMover)
: m_pViewport(repMover.m_pViewport)
, m_MainColor(repMover.m_MainColor)
, m_Thickness(repMover.m_Thickness)
, m_RenderProperties(repMover.m_RenderProperties)
, m_pRepMoverInfo(repMover.m_pRepMoverInfo)
{

}


GLC_RepMover::~GLC_RepMover()
{

}

void GLC_RepMover::setRepMoverInfo(RepMoverInfo* pRepMoverInfo)
{
	m_pRepMoverInfo= pRepMoverInfo;
}

void GLC_RepMover::setMainColor(const QColor& color)
{
	m_MainColor= color;
}

void GLC_RepMover::setThickness(double thickness)
{
	m_Thickness= thickness;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Representation OpenGL Execution
void GLC_RepMover::render()
{
	// Call virtual draw function
	glDraw();
}
