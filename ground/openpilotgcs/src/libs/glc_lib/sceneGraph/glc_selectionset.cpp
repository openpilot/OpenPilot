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
//! \file glc_selectionset.cpp implementation for the GLC_SelectionSet class.

#include "glc_selectionset.h"
#include "glc_worldhandle.h"

GLC_SelectionSet::GLC_SelectionSet(GLC_WorldHandle* pWorldHandle)
: m_pWorldHandle(pWorldHandle)
, m_OccurenceHash()
{
	Q_ASSERT(0 == m_pWorldHandle->collection()->selectionSize());

}

GLC_SelectionSet::~GLC_SelectionSet()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

bool GLC_SelectionSet::isEmpty() const
{
	return m_OccurenceHash.isEmpty();
}


int GLC_SelectionSet::count() const
{
    return m_OccurenceHash.size();
}

QList<GLC_StructOccurence*> GLC_SelectionSet::occurencesList() const
{
    return m_OccurenceHash.values();
}


//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
bool GLC_SelectionSet::insert(GLC_StructOccurence* pOccurence)
{
	return insert(pOccurence->id());
}

bool GLC_SelectionSet::insert(GLC_uint occurenceId)
{
	Q_ASSERT(m_pWorldHandle->containsOccurence(occurenceId));
	if (!m_OccurenceHash.contains(occurenceId))
	{
		GLC_StructOccurence* pOccurence= m_pWorldHandle->getOccurence(occurenceId);
		m_OccurenceHash.insert(occurenceId, pOccurence);
		return true;
	}
	else return false;
}

bool GLC_SelectionSet::remove(GLC_StructOccurence* pOccurence)
{
	return remove(pOccurence->id());
}

bool GLC_SelectionSet::remove(GLC_uint occurenceId)
{
	Q_ASSERT(m_pWorldHandle->containsOccurence(occurenceId));
	if (m_OccurenceHash.contains(occurenceId))
	{
		m_OccurenceHash.remove(occurenceId);
		return true;
	}
	else return false;
}

void GLC_SelectionSet::clear()
{
	m_OccurenceHash.clear();
}
