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

//! \file glc_structreference.cpp implementation of the GLC_StructReference class.

#include "glc_structreference.h"
#include "glc_structoccurence.h"

// Default constructor
GLC_StructReference::GLC_StructReference(const QString& name)
: m_SetOfInstance()
, m_pRepresentation(NULL)
, m_Name(name)
, m_pAttributes(NULL)
{


}

// Create reference with representation
GLC_StructReference::GLC_StructReference(GLC_Rep* pRep)
: m_SetOfInstance()
, m_pRepresentation(pRep)
, m_Name(m_pRepresentation->name())
, m_pAttributes(NULL)
{

}

// Copy constructor
GLC_StructReference::GLC_StructReference(const GLC_StructReference& ref)
: m_SetOfInstance()
, m_pRepresentation(NULL)
, m_Name(ref.m_Name)
, m_pAttributes(new GLC_Attributes(*(ref.m_pAttributes)))
{
	if (NULL != ref.m_pRepresentation)
	{
		m_pRepresentation= ref.m_pRepresentation->clone();
	}
}

//! Overload "=" operator
GLC_StructReference& GLC_StructReference::operator=(const GLC_StructReference& ref)
{
	if (this != &ref)
	{
		m_SetOfInstance.clear();
		delete m_pAttributes;
		m_pAttributes= NULL;

		m_Name= ref.m_Name;
		m_pAttributes= new GLC_Attributes(*(ref.m_pAttributes));

		if (NULL != ref.m_pRepresentation)
		{
			m_pRepresentation= ref.m_pRepresentation->clone();
		}
	}
	return *this;
}

GLC_StructReference::~GLC_StructReference()
{
	delete m_pRepresentation;
	delete m_pAttributes;
}


//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

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

