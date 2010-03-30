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

//! \file glc_mover.cpp Implementation of the GLC_Mover class.

#include "glc_mover.h"
#include "glc_viewport.h"

GLC_Mover::GLC_Mover(GLC_Viewport* pViewport, const QList<GLC_RepMover*>& repsList)
: m_RepMoverList(repsList)
, m_PreviousVector()
, m_pViewport(pViewport)
{


}

// Copy constructor
GLC_Mover::GLC_Mover(const GLC_Mover& mover)
: m_RepMoverList()
, m_PreviousVector(mover.m_PreviousVector)
, m_pViewport(mover.m_pViewport)
{
	const int size= mover.m_RepMoverList.size();
	for (int i= 0; i < size; ++i)
	{
		m_RepMoverList.append(mover.m_RepMoverList.at(i)->clone());
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
	clearMoverRepresentation();
	m_RepMoverList= listOfRep;
}

// Update representation
void GLC_Mover::initRepresentation(const GLC_Vector4d& vector, const GLC_Matrix4x4& matrix)
{
	const int size= m_RepMoverList.size();
	for (int i= 0; i < size; ++i)
	{
		m_RepMoverList[i]->init(vector, matrix);
	}
}

// Update representation
void GLC_Mover::updateRepresentation(const GLC_Matrix4x4& matrix)
{
	const int size= m_RepMoverList.size();
	for (int i= 0; i < size; ++i)
	{
		m_RepMoverList[i]->update(matrix);
	}

}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Mover representations list display
void GLC_Mover::glExecute()
{
	const int size= m_RepMoverList.size();
	for (int i= 0; i < size; ++i)
	{
		m_RepMoverList[i]->glExecute();
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


