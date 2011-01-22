/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 2.0.0, packaged on July 2010.

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


// An Occurence has been added
void GLC_WorldHandle::addOccurence(GLC_StructOccurence* pOccurence, bool isSelected, GLuint shaderId)
{
	Q_ASSERT(!m_OccurenceHash.contains(pOccurence->id()));
	m_OccurenceHash.insert(pOccurence->id(), pOccurence);

	// Add instance representation in the collection
	if (pOccurence->hasRepresentation())
	{
		GLC_3DRep* p3DRep= dynamic_cast<GLC_3DRep*>(pOccurence->structReference()->representationHandle());
		GLC_3DViewInstance representation(*p3DRep);
		// Force instance representation id
		representation.setId(pOccurence->id());
		// Force instance representation name
		representation.setName(pOccurence->name());
		if (0 != shaderId) m_Collection.bindShader(shaderId);
		m_Collection.add(representation, shaderId);
		if (isSelected)
		{
			qDebug() << pOccurence->name() << "selected";
			m_Collection.select(pOccurence->id());
		}
	}
}

// An Occurence has been removed
void GLC_WorldHandle::removeOccurence(GLC_StructOccurence* pOccurence)
{
	Q_ASSERT(m_OccurenceHash.contains(pOccurence->id()));
	m_OccurenceHash.remove(pOccurence->id());
	// Remove instance representation from the collection
	if (pOccurence->hasRepresentation())
	{
		m_Collection.remove(pOccurence->id());
	}
}
