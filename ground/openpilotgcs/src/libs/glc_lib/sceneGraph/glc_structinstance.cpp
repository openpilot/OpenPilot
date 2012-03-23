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

//! \file glc_structinstance.cpp implementation of the GLC_StructInstance class.

#include "glc_structinstance.h"
#include "glc_structreference.h"
#include "glc_structoccurence.h"

// Default constructor
GLC_StructInstance::GLC_StructInstance(GLC_StructReference* pStructReference)
: m_pNumberOfInstance(NULL)
, m_pStructReference(pStructReference)
, m_ListOfOccurences()
, m_RelativeMatrix()
, m_Name()
, m_pAttributes(NULL)
{
	if (NULL == m_pStructReference)
	{
		m_pStructReference= new GLC_StructReference();
	}
	m_Name= m_pStructReference->name();

	if (m_pStructReference->hasStructInstance())
	{
		m_pNumberOfInstance= m_pStructReference->firstInstanceHandle()->m_pNumberOfInstance;
		++(*m_pNumberOfInstance);
	}
	else
	{
		m_pNumberOfInstance= new int(1);
	}
	// Inform reference that an instance has been created
	m_pStructReference->structInstanceCreated(this);
	//qDebug() << "GLC_StructInstance::GLC_StructInstance : " << (*m_pNumberOfInstance) << " " << m_pNumberOfInstance;
}

// Create instance with a rep
GLC_StructInstance::GLC_StructInstance(GLC_Rep* pRep)
: m_pNumberOfInstance(NULL)
, m_pStructReference(new GLC_StructReference(pRep))
, m_ListOfOccurences()
, m_RelativeMatrix()
, m_Name(m_pStructReference->name())
, m_pAttributes(NULL)
{
	if (m_pStructReference->hasStructInstance())
	{
		m_pNumberOfInstance= m_pStructReference->firstInstanceHandle()->m_pNumberOfInstance;
		++(*m_pNumberOfInstance);
	}
	else
	{
		m_pNumberOfInstance= new int(1);
	}
	// Inform reference that an instance has been created
	m_pStructReference->structInstanceCreated(this);
}

// Copy constructor
GLC_StructInstance::GLC_StructInstance(const GLC_StructInstance& structInstance)
: m_pNumberOfInstance(structInstance.m_pNumberOfInstance)
, m_pStructReference(structInstance.m_pStructReference)
, m_ListOfOccurences()
, m_RelativeMatrix(structInstance.m_RelativeMatrix)
, m_Name(structInstance.name())
, m_pAttributes(NULL)
{
	//qDebug() << "Instance Copy constructor";
	Q_ASSERT(NULL != m_pStructReference);
	// Copy attributes if necessary
	if (NULL != structInstance.m_pAttributes)
	{
		m_pAttributes= new GLC_Attributes(*(structInstance.m_pAttributes));
	}

	++(*m_pNumberOfInstance);

	// Inform reference that an instance has been created
	m_pStructReference->structInstanceCreated(this);
}

// Copy constructor
GLC_StructInstance::GLC_StructInstance(GLC_StructInstance* pStructInstance)
: m_pNumberOfInstance(pStructInstance->m_pNumberOfInstance)
, m_pStructReference(pStructInstance->m_pStructReference)
, m_ListOfOccurences()
, m_RelativeMatrix(pStructInstance->m_RelativeMatrix)
, m_Name(pStructInstance->name())
, m_pAttributes(NULL)
{
	//qDebug() << "Instance Copy constructor";
	Q_ASSERT(NULL != m_pStructReference);
	// Copy attributes if necessary
	if (NULL != pStructInstance->m_pAttributes)
	{
		m_pAttributes= new GLC_Attributes(*(pStructInstance->m_pAttributes));
	}

	++(*m_pNumberOfInstance);

	// Inform reference that an instance has been created
	m_pStructReference->structInstanceCreated(this);
}

// Create empty instance
GLC_StructInstance::GLC_StructInstance(const QString& name)
: m_pNumberOfInstance(NULL)
, m_pStructReference(NULL)
, m_ListOfOccurences()
, m_RelativeMatrix()
, m_Name(name)
, m_pAttributes(NULL)
{
}

// Set the reference of an empty instance
void GLC_StructInstance::setReference(GLC_StructReference* pStructReference)
{
	Q_ASSERT(NULL == m_pStructReference);
	Q_ASSERT(NULL == m_pNumberOfInstance);
	m_pStructReference= pStructReference;
	if (m_pStructReference->hasStructInstance())
	{
		m_pNumberOfInstance= m_pStructReference->firstInstanceHandle()->m_pNumberOfInstance;
		++(*m_pNumberOfInstance);
	}
	else
	{
		m_pNumberOfInstance= new int(1);
	}
	// Inform reference that an instance has been created
	m_pStructReference->structInstanceCreated(this);

	if (m_Name.isEmpty())
	{
		m_Name= pStructReference->name();
	}
}

// Destructor
GLC_StructInstance::~GLC_StructInstance()
{
	if(m_pNumberOfInstance != NULL)
	{
		// Update number of instance
		if ((--(*m_pNumberOfInstance)) == 0)
		{
			delete m_pStructReference;
			delete m_pNumberOfInstance;
		}
		else
		{
			// Inform reference that an instance has been deleted
			m_pStructReference->structInstanceDeleted(this);
			Q_ASSERT(m_pStructReference->hasStructInstance());
		}
		delete m_pAttributes;
	}
	else qDebug() << "GLC_StructInstance::~GLC_StructInstance() of empty instance";

}

void GLC_StructInstance::updateOccurencesAbsoluteMatrix()
{
	const int occurenceCount= m_ListOfOccurences.count();
	for (int i= 0; i < occurenceCount; ++i)
	{
		m_ListOfOccurences.at(i)->updateChildrenAbsoluteMatrix();
	}
}
