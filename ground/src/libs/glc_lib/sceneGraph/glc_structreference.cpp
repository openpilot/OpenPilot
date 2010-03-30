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

//! \file glc_structreference.cpp implementation of the GLC_StructReference class.

#include "glc_structreference.h"
#include "glc_structoccurence.h"

// Default constructor
GLC_StructReference::GLC_StructReference(const QString& name)
: m_ListOfInstance()
, m_pRepresentation(NULL)
, m_Name(name)
, m_pAttributes(NULL)
{


}

// Create reference with representation
GLC_StructReference::GLC_StructReference(GLC_Rep* pRep)
: m_ListOfInstance()
, m_pRepresentation(pRep)
, m_Name(m_pRepresentation->name())
, m_pAttributes(NULL)
{

}

GLC_StructReference::~GLC_StructReference()
{
	delete m_pRepresentation;
	delete m_pAttributes;
}


//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Create a Struct instance of this reference
GLC_StructInstance* GLC_StructReference::createStructInstance()
{
	GLC_StructInstance* pInstance= new GLC_StructInstance(this);
	return pInstance;
}

// Return the list of occurence of this reference
QList<GLC_StructOccurence*> GLC_StructReference::listOfStructOccurence() const
{
	QList<GLC_StructInstance*> instanceList= listOfStructInstances();
	QSet<GLC_StructOccurence*> occurenceSet;
	const int size= instanceList.size();
	for (int i= 0; i < size; ++i)
	{
		QList<GLC_StructOccurence*> occurenceList= instanceList.at(i)->listOfStructOccurences();
		const int occurenceSize= occurenceList.size();
		for (int occIndex= 0; occIndex < occurenceSize; ++occIndex)
		{
			occurenceSet.insert(occurenceList.at(occIndex));
		}
	}
	return occurenceSet.toList();
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
// Set the reference representation
void GLC_StructReference::setRepresentation(const GLC_3DRep& rep)
{
	Q_ASSERT(NULL == m_pRepresentation);
	m_pRepresentation= new GLC_3DRep(rep);

	// Update all occurence of this reference if exist
	if (hasStructInstance())
	{
		GLC_StructInstance* pInstance= firstInstanceHandle();
		if (pInstance->hasStructOccurence())
		{
			QList<GLC_StructOccurence*> occurenceList= pInstance->listOfStructOccurences();
			const int size= occurenceList.size();
			for (int i= 0; i < size; ++i)
			{
				occurenceList[i]->checkForRepresentation();
			}
		}
	}
}

