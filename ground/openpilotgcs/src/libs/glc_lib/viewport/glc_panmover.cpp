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

//! \file glc_panmover.cpp Implementation of the GLC_PanMover class.

#include "glc_panmover.h"
#include "glc_viewport.h"
#include "QMouseEvent"

// Default constructor
GLC_PanMover::GLC_PanMover(GLC_Viewport* pViewport, const QList<GLC_RepMover*>& repsList)
: GLC_Mover(pViewport, repsList)
{

}

// Copy constructor
GLC_PanMover::GLC_PanMover(const GLC_PanMover& panMover)
: GLC_Mover(panMover)
{

}


GLC_PanMover::~GLC_PanMover()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Return a clone of the mover
GLC_Mover* GLC_PanMover::clone() const
{
	return new GLC_PanMover(*this);
}


//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Initialized the mover
void GLC_PanMover::init(const GLC_UserInput& userInput)
{
	m_PreviousVector= m_pViewport->mapPosMouse(static_cast<double>(userInput.x()), static_cast<double>(userInput.y()));
}

// Move the camera
bool GLC_PanMover::move(const GLC_UserInput& userInput)
{
	const GLC_Vector3d VectCur(m_pViewport->mapPosMouse(static_cast<double>(userInput.x()), static_cast<double>(userInput.y())));
	const GLC_Vector3d VectPan= VectCur - m_PreviousVector;	// moving Vector

	// Pan the camera
	m_pViewport->cameraHandle()->pan(-VectPan);
	m_PreviousVector= VectCur;
	return true;
}

