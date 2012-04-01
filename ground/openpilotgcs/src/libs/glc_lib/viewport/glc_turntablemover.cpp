/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Copyright (C) 2009 Laurent Bauer
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

#include "glc_turntablemover.h"
#include "glc_viewport.h"

// Default constructor
GLC_TurnTableMover::GLC_TurnTableMover(GLC_Viewport* pViewport, const QList<GLC_RepMover*>& repsList)
: GLC_Mover(pViewport, repsList)
, m_Sign(1.0)
{

}


// Copy constructor
GLC_TurnTableMover::GLC_TurnTableMover(const GLC_TurnTableMover& mover)
: GLC_Mover(mover)
, m_Sign(mover.m_Sign)
{
}


GLC_TurnTableMover::~GLC_TurnTableMover()
{
}


//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Return a clone of the mover
GLC_Mover* GLC_TurnTableMover::clone() const
{
	return new GLC_TurnTableMover(*this);
}


//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Initialized the mover
void GLC_TurnTableMover::init(const GLC_UserInput& userInput)
{
	GLC_Mover::m_PreviousVector.setVect(static_cast<double>(userInput.x()), static_cast<double>(userInput.y()),0.0);
	GLC_Camera* pCamera= GLC_Mover::m_pViewport->cameraHandle();
	// Calculate angle sign
	m_Sign= pCamera->defaultUpVector() * pCamera->upVector();
	if (m_Sign == 0)
	{
		m_Sign= 1;
	}
	else
	{
		m_Sign= m_Sign / fabs(m_Sign);
	}

	pCamera->setUpCam(pCamera->defaultUpVector() * m_Sign);
}


bool GLC_TurnTableMover::move(const GLC_UserInput& userInput)
{
	GLC_Camera* pCamera= GLC_Mover::m_pViewport->cameraHandle();
	// Turn table rotation
	const double rotSpeed= 2.3;
	const double width= static_cast<double> ( GLC_Mover::m_pViewport->viewVSize() );
	const double height= static_cast<double> ( GLC_Mover::m_pViewport->viewHSize() );

	const double alpha = -((static_cast<double>(userInput.x()) - GLC_Mover::m_PreviousVector.x()) / width) * rotSpeed;
	const double beta = ((static_cast<double>(userInput.y()) - GLC_Mover::m_PreviousVector.y()) / height) * rotSpeed;

	// Rotation around the screen vertical axis
	pCamera->rotateAroundTarget(pCamera->defaultUpVector(), alpha * m_Sign);

	// Rotation around the screen horizontal axis
	GLC_Vector3d incidentVector= -pCamera->forward();
	GLC_Vector3d rightVector= incidentVector ^ pCamera->upVector();
	if (!rightVector.isNull())
	{
		pCamera->rotateAroundTarget(rightVector, beta);
	}

	m_PreviousVector.setVect(static_cast<double>(userInput.x()), static_cast<double>(userInput.y()), 0.0);

	return true;
}
