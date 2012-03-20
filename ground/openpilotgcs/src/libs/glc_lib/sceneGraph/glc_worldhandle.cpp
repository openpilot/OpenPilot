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

#include "glc_worldhandle.h"
#include "glc_structreference.h"
#include <QSet>

GLC_WorldHandle::GLC_WorldHandle()
: m_Collection()
, m_NumberOfWorld(1)
, m_OccurenceHash()
, m_UpVector(glc::Z_AXIS)
, m_SelectionSet(this)
{

}

GLC_WorldHandle::~GLC_WorldHandle()
{

}

// Return the list of instance
QList<GLC_StructInstance*> GLC_WorldHandle::instances() const
{
	QSet<GLC_StructInstance*> instancesSet;
	QHash<GLC_uint, GLC_StructOccurence*>::const_iterator iOccurence= m_OccurenceHash.constBegin();
	while (iOccurence != m_OccurenceHash.constEnd())
	{
		instancesSet.insert(iOccurence.value()->structInstance());
		++iOccurence;
	}
	return instancesSet.toList();
}

// Return the list of Reference
QList<GLC_StructReference*> GLC_WorldHandle::references() const
{
	QSet<GLC_StructReference*> referencesSet;
	QHash<GLC_uint, GLC_StructOccurence*>::const_iterator iOccurence= m_OccurenceHash.constBegin();
	while (iOccurence != m_OccurenceHash.constEnd())
	{
		referencesSet.insert(iOccurence.value()->structReference());
		++iOccurence;
	}
	return referencesSet.toList();
}

// Return the number of body
int GLC_WorldHandle::numberOfBody() const
{
	QList<GLC_StructReference*> referenceList(references());
	const int size= referenceList.size();
	int numberOfBody= 0;
	for (int i= 0; i < size; ++i)
	{
		numberOfBody+= referenceList.at(i)->numberOfBody();
	}
	return numberOfBody;
}

int GLC_WorldHandle::representationCount() const
{
	QList<GLC_StructReference*> referenceList(references());
	const int size= referenceList.size();
	int count= 0;
	for (int i= 0; i < size; ++i)
	{
		if (referenceList.at(i)->hasRepresentation()) ++count;
	}
	return count;

}

// An Occurence has been added
void GLC_WorldHandle::addOccurence(GLC_StructOccurence* pOccurence, bool isSelected, GLuint shaderId)
{
	Q_ASSERT(!m_OccurenceHash.contains(pOccurence->id()));
	m_OccurenceHash.insert(pOccurence->id(), pOccurence);
	GLC_StructReference* pRef= pOccurence->structReference();
	Q_ASSERT(NULL != pRef);

	// Add instance representation in the collection
	if (pOccurence->useAutomatic3DViewInstanceCreation() && pRef->representationIsLoaded())
	{
		//qDebug() << "GLC_WorldHandle::addOccurence with rep";
		GLC_3DRep* p3DRep= dynamic_cast<GLC_3DRep*>(pRef->representationHandle());
		GLC_3DViewInstance representation(*p3DRep);
		// Force instance representation id
		representation.setId(pOccurence->id());
		// Force instance representation name
		representation.setName(pOccurence->name());
		if (0 != shaderId) m_Collection.bindShader(shaderId);
		m_Collection.add(representation, shaderId);
		if (isSelected)
		{
			//qDebug() << pOccurence->name() << "selected";
			m_Collection.select(pOccurence->id());
		}
	}
}

// An Occurence has been removed
void GLC_WorldHandle::removeOccurence(GLC_StructOccurence* pOccurence)
{
	Q_ASSERT(m_OccurenceHash.contains(pOccurence->id()));
	// Remove the occurence from the selection set
	m_SelectionSet.remove(pOccurence);
	// Remove the occurence from the main occurence hash table
	m_OccurenceHash.remove(pOccurence->id());
	// Remove instance representation from the collection
	m_Collection.remove(pOccurence->id());
}

void GLC_WorldHandle::select(GLC_uint occurenceId)
{
	Q_ASSERT(m_OccurenceHash.contains(occurenceId));
	m_SelectionSet.insert(occurenceId);
	m_Collection.select(occurenceId);

	const GLC_StructOccurence* pSelectedOccurence= m_OccurenceHash.value(occurenceId);
	if (pSelectedOccurence->hasChild())
	{
		QList<GLC_StructOccurence*> subOccurenceList= pSelectedOccurence->subOccurenceList();
		const int subOccurenceCount= subOccurenceList.size();
		for (int i= 0; i < subOccurenceCount; ++i)
		{
			const GLC_uint currentOccurenceId= subOccurenceList.at(i)->id();
			if (m_Collection.contains(currentOccurenceId))
			{
				m_Collection.select(currentOccurenceId);
			}
		}
	}
}

void GLC_WorldHandle::unselect(GLC_uint occurenceId, bool propagate)
{
	Q_ASSERT(m_OccurenceHash.contains(occurenceId));
	m_SelectionSet.remove(occurenceId);
	m_Collection.unselect(occurenceId);

	const GLC_StructOccurence* pSelectedOccurence= m_OccurenceHash.value(occurenceId);
	if (propagate && pSelectedOccurence->hasChild())
	{
		QList<GLC_StructOccurence*> subOccurenceList= pSelectedOccurence->subOccurenceList();
		const int subOccurenceCount= subOccurenceList.size();
		for (int i= 0; i < subOccurenceCount; ++i)
		{
			const GLC_uint currentOccurenceId= subOccurenceList.at(i)->id();
			m_Collection.unselect(currentOccurenceId);
		}
	}
}

void GLC_WorldHandle::selectAllWith3DViewInstance(bool allShowState)
{
	m_Collection.selectAll(allShowState);
	QList<GLC_uint> selectedId= m_Collection.selection()->keys();
	m_SelectionSet.clear();
	const int selectionCount= selectedId.count();
	for (int i= 0; i < selectionCount; ++i)
	{
		m_SelectionSet.insert(selectedId.at(i));
	}
}

void GLC_WorldHandle::unselectAll()
{
	m_SelectionSet.clear();
	m_Collection.unselectAll();
}

void GLC_WorldHandle::showHideSelected3DViewInstance()
{
	QList<GLC_3DViewInstance*> selected3dviewInstance= m_Collection.selection()->values();
	const int instanceCount= selected3dviewInstance.count();
	for(int i= 0; i < instanceCount; ++i)
	{
		GLC_3DViewInstance* pCurrentInstance= selected3dviewInstance.at(i);
		pCurrentInstance->setVisibility(!pCurrentInstance->isVisible());
	}
}

void GLC_WorldHandle::setSelected3DViewInstanceVisibility(bool isVisible)
{
	QList<GLC_3DViewInstance*> selected3dviewInstance= m_Collection.selection()->values();
	const int instanceCount= selected3dviewInstance.count();
	for(int i= 0; i < instanceCount; ++i)
	{
		GLC_3DViewInstance* pCurrentInstance= selected3dviewInstance.at(i);
		pCurrentInstance->setVisibility(isVisible);
	}
}

