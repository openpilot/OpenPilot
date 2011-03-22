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

QSet<GLC_StructOccurence*> GLC_StructReference::setOfStructOccurence() const
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
	return occurenceSet;
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
// Set the reference representation
void GLC_StructReference::setRepresentation(const GLC_3DRep& rep)
{
	// Unload occurence representation
	{
		QSet<GLC_StructOccurence*> structOccurenceSet= this->setOfStructOccurence();
		QSet<GLC_StructOccurence*>::iterator iOcc= structOccurenceSet.begin();
		while (structOccurenceSet.constEnd() != iOcc)
		{
			(*iOcc)->remove3DViewInstance();
			++iOcc;
		}
	}

	if(NULL == m_pRepresentation)
	{
		m_pRepresentation= new GLC_3DRep(rep);
	}
	else
	{
		*(m_pRepresentation)= rep;
	}

	if (m_pRepresentation->isLoaded())
	{
		QSet<GLC_StructOccurence*> structOccurenceSet= this->setOfStructOccurence();
		QSet<GLC_StructOccurence*>::iterator iOcc= structOccurenceSet.begin();
		while (structOccurenceSet.constEnd() != iOcc)
		{
			GLC_StructOccurence* pOccurence= *iOcc;
			Q_ASSERT(!pOccurence->has3DViewInstance());
			if (pOccurence->useAutomatic3DViewInstanceCreation())
			{
				pOccurence->create3DViewInstance();
			}
			++iOcc;
		}
	}
}

GLC_Rep* GLC_StructReference::representationHandle() const
{
	Q_ASSERT(NULL != m_pRepresentation);
	return m_pRepresentation;
}

QString GLC_StructReference::representationName() const
{
	if (NULL != m_pRepresentation)
	{
		return m_pRepresentation->name();
	}
	else return QString();
}

bool GLC_StructReference::representationIsLoaded() const
{
	if (NULL != m_pRepresentation)
	{
		return m_pRepresentation->isLoaded();
	}
	else return false;

}

QString GLC_StructReference::representationFileName() const
{
	if (NULL != m_pRepresentation)
	{
		return m_pRepresentation->fileName();
	}
	else return QString();
}

bool GLC_StructReference::representationIsEmpty() const
{
	if (NULL != m_pRepresentation)
	{
		return m_pRepresentation->isEmpty();
	}
	else return true;

}

void GLC_StructReference::setRepresentationName(const QString& representationName)
{
	if (NULL != m_pRepresentation)
	{
		m_pRepresentation->setName(representationName);
	}
}

bool GLC_StructReference::loadRepresentation()
{
	Q_ASSERT(NULL != m_pRepresentation);
	if (m_pRepresentation->load())
	{
		QSet<GLC_StructOccurence*> structOccurenceSet= this->setOfStructOccurence();
		QSet<GLC_StructOccurence*>::iterator iOcc= structOccurenceSet.begin();
		while (structOccurenceSet.constEnd() != iOcc)
		{
			GLC_StructOccurence* pOccurence= *iOcc;
			Q_ASSERT(!pOccurence->has3DViewInstance());
			if (pOccurence->useAutomatic3DViewInstanceCreation())
			{
				pOccurence->create3DViewInstance();
			}
			++iOcc;
		}
		return true;
	}
	else return false;
}

bool GLC_StructReference::unloadRepresentation()
{
	Q_ASSERT(NULL != m_pRepresentation);
	if (m_pRepresentation->unload())
	{
		QSet<GLC_StructOccurence*> structOccurenceSet= this->setOfStructOccurence();
		QSet<GLC_StructOccurence*>::iterator iOcc= structOccurenceSet.begin();
		while (structOccurenceSet.constEnd() != iOcc)
		{
			(*iOcc)->remove3DViewInstance();
			++iOcc;
		}
		return true;
	}
	else return false;
}

bool GLC_StructReference::addChild(GLC_StructOccurence* pOccurence)
{
	if (hasStructInstance() && firstInstanceHandle()->hasStructOccurence())
	{
		GLC_StructOccurence* pCurrentChildOccurence= pOccurence;

		QSet<GLC_StructInstance*>::iterator iInstance= m_SetOfInstance.begin();
		while (m_SetOfInstance.constEnd() != iInstance)
		{
			GLC_StructInstance* pCurrentInstance= *iInstance;
			QList<GLC_StructOccurence*> occurenceList= pCurrentInstance->listOfStructOccurences();
			const int occurenceCount= occurenceList.count();
			for (int i= 0; i < occurenceCount; ++i)
			{
				GLC_StructOccurence* pCurrentOccurence= occurenceList.at(i);

				if ((i != 0) || (NULL == pCurrentChildOccurence))
				{
					pCurrentChildOccurence= pOccurence->clone(pCurrentOccurence->worldHandle(), true);
				}

				pCurrentOccurence->addChild(pCurrentChildOccurence);
			}
			pCurrentChildOccurence= NULL;
			++iInstance;
		}
		return true;
	}
	else
	{
		return false;
	}
}

