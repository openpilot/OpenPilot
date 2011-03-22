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
, m_InstanceToPrimitiveId()
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
	return m_OccurenceHash.size() - m_InstanceToPrimitiveId.size();
}

QList<GLC_StructOccurence*> GLC_SelectionSet::occurencesList() const
{
	QList<GLC_StructOccurence*> listOfOccurence;
	QHash<GLC_uint, GLC_StructOccurence*>::const_iterator iOcc= m_OccurenceHash.constBegin();
	while (m_OccurenceHash.constEnd() != iOcc)
	{
		if (!m_InstanceToPrimitiveId.contains(iOcc.key()))
		{
			listOfOccurence.append(iOcc.value());
		}
		++iOcc;
	}
	return listOfOccurence;
}

QList<GLC_StructOccurence*> GLC_SelectionSet::occurencesListWithSelectedPrimitive() const
{
	QList<GLC_StructOccurence*> listOfOccurence;
	QHash<GLC_uint, QSet<GLC_uint> >::const_iterator iOcc= m_InstanceToPrimitiveId.constBegin();
	while (m_InstanceToPrimitiveId.constEnd() != iOcc)
	{
		listOfOccurence.append(m_OccurenceHash.value(iOcc.key()));
		++iOcc;
	}
	return listOfOccurence;
}

QSet<GLC_uint> GLC_SelectionSet::selectedPrimitivesId(GLC_uint instanceId) const
{
	QSet<GLC_uint> primitivesIdSet;
	if (m_InstanceToPrimitiveId.contains(instanceId))
	{
		primitivesIdSet= m_InstanceToPrimitiveId.value(instanceId);
	}
	return primitivesIdSet;
}

bool GLC_SelectionSet::hasPrimitiveSelected(GLC_uint instanceId) const
{
	return m_InstanceToPrimitiveId.contains(instanceId);
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
		m_InstanceToPrimitiveId.remove(occurenceId);
		return true;
	}
	else return false;
}

void GLC_SelectionSet::clear()
{
	m_OccurenceHash.clear();
	m_InstanceToPrimitiveId.clear();
}

bool GLC_SelectionSet::insertPrimitiveId(GLC_StructOccurence* pOccurence, GLC_uint primitiveId)
{
	return insertPrimitiveId(pOccurence->id(), primitiveId);
}

bool GLC_SelectionSet::insertPrimitiveId(GLC_uint occurenceId, GLC_uint primitiveId)
{
	bool returnValue= false;
	insert(occurenceId);
	if (m_InstanceToPrimitiveId.contains(occurenceId))
	{
		if (!m_InstanceToPrimitiveId[occurenceId].contains(primitiveId))
		{
			m_InstanceToPrimitiveId[occurenceId].insert(primitiveId);
			returnValue= true;
		}
		else returnValue= false;
	}
	else
	{
		QSet<GLC_uint> primitiveSet;
		primitiveSet.insert(primitiveId);
		m_InstanceToPrimitiveId.insert(occurenceId, primitiveSet);
		returnValue= true;
	}

	return returnValue;

}

void GLC_SelectionSet::insertSetOfPrimitivesId(GLC_StructOccurence* pOccurence, const QSet<GLC_uint>& setOfPrimitivesId)
{
	insertSetOfPrimitivesId(pOccurence->id(), setOfPrimitivesId);
}

void GLC_SelectionSet::insertSetOfPrimitivesId(GLC_uint occurenceId, const QSet<GLC_uint>& setOfPrimitivesId)
{
	insert(occurenceId);
	if (m_InstanceToPrimitiveId.contains(occurenceId))
	{
		m_InstanceToPrimitiveId[occurenceId].unite(setOfPrimitivesId);
	}
	else
	{
		m_InstanceToPrimitiveId.insert(occurenceId, setOfPrimitivesId);
	}
}

bool GLC_SelectionSet::removePrimitiveId(GLC_StructOccurence* pOccurence, GLC_uint primitiveId)
{
	return removePrimitiveId(pOccurence->id(), primitiveId);
}

bool GLC_SelectionSet::removePrimitiveId(GLC_uint occurenceId, GLC_uint primitiveId)
{
	Q_ASSERT(m_pWorldHandle->containsOccurence(occurenceId));

	if (m_InstanceToPrimitiveId.contains(occurenceId))
	{
		if (m_InstanceToPrimitiveId[occurenceId].contains(primitiveId))
		{
			if (m_InstanceToPrimitiveId[occurenceId].size() == 1)
			{
				remove(occurenceId);
			}
			else
			{
				m_InstanceToPrimitiveId[occurenceId].remove(primitiveId);
			}
			return true;
		}
		else return false;
	}
	else return false;
}
