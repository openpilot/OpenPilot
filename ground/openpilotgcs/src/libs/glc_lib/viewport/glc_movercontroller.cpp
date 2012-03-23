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

//! \file glc_movercontroller.cpp Implementation of the GLC_MoverController class.

#include "glc_movercontroller.h"

// Default constructor
GLC_MoverController::GLC_MoverController()
: QObject()
, m_ActiveMoverId(0)
, m_MoverHash()
{


}

// Copy constructor
GLC_MoverController::GLC_MoverController(const GLC_MoverController& controller)
: QObject()
, m_ActiveMoverId(controller.m_ActiveMoverId)
, m_MoverHash()
{
	MoverHash::const_iterator iMover= controller.m_MoverHash.constBegin();
	while (iMover != controller.m_MoverHash.constEnd())
	{
		m_MoverHash.insert(iMover.key(), iMover.value()->clone());
		++iMover;
	}
}

// Destructor
GLC_MoverController::~GLC_MoverController()
{
	MoverHash::iterator iMover= m_MoverHash.begin();
	while (iMover != m_MoverHash.constEnd())
	{
		delete iMover.value();

		++iMover;
	}
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

//! Assign another mover controller
GLC_MoverController& GLC_MoverController::operator = (const GLC_MoverController& controller)
{
	if (&controller != this)
	{
		 m_ActiveMoverId= controller.m_ActiveMoverId;

		 // Clear the inner movers
		 {
			 MoverHash::iterator iMover= m_MoverHash.begin();
			 while (iMover != m_MoverHash.constEnd())
			 {
				 delete iMover.value();

				 ++iMover;
			 }
			 m_MoverHash.clear();
		 }
		 // Copy movers
		 MoverHash::const_iterator iMover= controller.m_MoverHash.constBegin();
		 while (iMover != controller.m_MoverHash.constEnd())
		 {
			 m_MoverHash.insert(iMover.key(), iMover.value()->clone());
			 ++iMover;
		 }
	}
	return *this;
}

// Add a mover to the controller
void GLC_MoverController::addMover(GLC_Mover* pMover, const int id)
{
	Q_ASSERT(!m_MoverHash.contains(id));
	m_MoverHash.insert(id, pMover);
}

// Remove mover from the controller
void GLC_MoverController::removeMover(const int id)
{
	Q_ASSERT(m_MoverHash.contains(id));
	m_MoverHash.remove(id);
	if (id == m_ActiveMoverId)
	{
		m_ActiveMoverId= 0;
	}
}

void GLC_MoverController::setActiveMover(const int id, const GLC_UserInput& userInput)
{
	Q_ASSERT(m_MoverHash.contains(id));
	m_ActiveMoverId= id;
	connect(m_MoverHash.value(m_ActiveMoverId), SIGNAL(updated()), this, SIGNAL(repaintNeeded()));
	m_MoverHash.value(m_ActiveMoverId)->init(userInput);
}

void GLC_MoverController::setNoMover()
{
	if (0 != m_ActiveMoverId)
	{
		m_MoverHash.value(m_ActiveMoverId)->ends();
		disconnect(m_MoverHash.value(m_ActiveMoverId), SIGNAL(updated()), this, SIGNAL(repaintNeeded()));
		m_ActiveMoverId= 0;
	}
}

