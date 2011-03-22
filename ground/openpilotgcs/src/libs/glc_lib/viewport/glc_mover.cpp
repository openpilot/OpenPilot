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

#include "glc_mover.h"
#include "glc_viewport.h"

GLC_Mover::GLC_Mover(GLC_Viewport* pViewport, const QList<GLC_RepMover*>& repsList)
: QObject()
, m_RepMoverList(repsList)
, m_PreviousVector()
, m_pViewport(pViewport)
, m_MoverInfo()
{
	const int size= m_RepMoverList.size();
	for (int i= 0; i < size; ++i)
	{
		m_RepMoverList[i]->setRepMoverInfo(&m_MoverInfo);
	}
}

// Copy constructor
GLC_Mover::GLC_Mover(const GLC_Mover& mover)
: QObject()
, m_RepMoverList()
, m_PreviousVector(mover.m_PreviousVector)
, m_pViewport(mover.m_pViewport)
, m_MoverInfo(mover.m_MoverInfo)
{
	const int size= mover.m_RepMoverList.size();
	for (int i= 0; i < size; ++i)
	{
		m_RepMoverList.append(mover.m_RepMoverList.at(i)->clone());
		m_RepMoverList.last()->setRepMoverInfo(&m_MoverInfo);
	}
}

GLC_Mover::~GLC_Mover()
{
	clearMoverRepresentation();
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set the mover representation list
void GLC_Mover::setRepresentationsList(const QList<GLC_RepMover*>& listOfRep)
{
	qDebug() << "GLC_Mover::setRepresentationsList";
	clearMoverRepresentation();
	m_RepMoverList= listOfRep;
	const int size= m_RepMoverList.size();
	for (int i= 0; i < size; ++i)
	{
		m_RepMoverList[i]->setRepMoverInfo(&m_MoverInfo);
	}
}

// Update representation
void GLC_Mover::initRepresentation()
{
	const int size= m_RepMoverList.size();
	for (int i= 0; i < size; ++i)
	{
		m_RepMoverList[i]->init();
	}
}

// Update representation
void GLC_Mover::updateRepresentation()
{
	const int size= m_RepMoverList.size();
	for (int i= 0; i < size; ++i)
	{
		m_RepMoverList[i]->update();
	}

}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Mover representations list display
void GLC_Mover::renderRepresentation()
{
	const int size= m_RepMoverList.size();
	for (int i= 0; i < size; ++i)
	{
		m_RepMoverList[i]->render();
	}
}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

// Clear mover representation
void GLC_Mover::clearMoverRepresentation()
{
	// Delete mover representations
	const int size= m_RepMoverList.size();
	for (int i= 0; i < size; ++i)
	{
		delete m_RepMoverList.at(i);
	}
}


