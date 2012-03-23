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

//! \file glc_structoccurence.cpp implementation of the GLC_StructOccurence class.

#include "glc_structoccurence.h"
#include "glc_3dviewcollection.h"
#include "glc_structreference.h"
#include "glc_worldhandle.h"
#include "../glc_errorlog.h"

// Default constructor
GLC_StructOccurence::GLC_StructOccurence()
: m_Uid(glc::GLC_GenID())
, m_pWorldHandle(NULL)
, m_pNumberOfOccurence(new int(1))
, m_pStructInstance(new GLC_StructInstance())
, m_pParent(NULL)
, m_Childs()
, m_AbsoluteMatrix()
, m_OccurenceNumber(0)
, m_IsVisible(true)
, m_pRenderProperties(NULL)
, m_AutomaticCreationOf3DViewInstance(true)
, m_pRelativeMatrix(NULL)
{
	// Update instance
	m_pStructInstance->structOccurenceCreated(this);
}


// Default constructor
GLC_StructOccurence::GLC_StructOccurence(GLC_StructInstance* pStructInstance, GLC_WorldHandle* pWorldHandle, GLuint shaderId)
: m_Uid(glc::GLC_GenID())
, m_pWorldHandle(pWorldHandle)
, m_pNumberOfOccurence(NULL)
, m_pStructInstance(pStructInstance)
, m_pParent(NULL)
, m_Childs()
, m_AbsoluteMatrix()
, m_OccurenceNumber(0)
, m_IsVisible(true)
, m_pRenderProperties(NULL)
, m_AutomaticCreationOf3DViewInstance(true)
, m_pRelativeMatrix(NULL)
{
	// Update the number of occurences
	if (pStructInstance->hasStructOccurence())
	{
		GLC_StructOccurence* pFirstOccurence= pStructInstance->firstOccurenceHandle();
		m_pNumberOfOccurence= pFirstOccurence->m_pNumberOfOccurence;
		++(*m_pNumberOfOccurence);
		QList<GLC_StructOccurence*> childs= pFirstOccurence->m_Childs;
		const int size= childs.size();
		for (int i= 0; i < size; ++i)
		{
			GLC_StructOccurence* pChild= childs.at(i)->clone(m_pWorldHandle, true);
			addChild(pChild);
		}
	}
	else
	{
		m_pNumberOfOccurence= new int(1);
	}

	setName(m_pStructInstance->name());

	// Inform the world Handle
	if (NULL != m_pWorldHandle)
	{
		m_pWorldHandle->addOccurence(this, shaderId);
	}

	// Update Absolute matrix
	updateAbsoluteMatrix();

	// Update instance
	m_pStructInstance->structOccurenceCreated(this);
}
// Construct Occurence with the specified GLC_3DRep
GLC_StructOccurence::GLC_StructOccurence(GLC_3DRep* pRep)
: m_Uid(glc::GLC_GenID())
, m_pWorldHandle(NULL)
, m_pNumberOfOccurence(new int(1))
, m_pStructInstance(NULL)
, m_pParent(NULL)
, m_Childs()
, m_AbsoluteMatrix()
, m_OccurenceNumber(0)
, m_IsVisible(true)
, m_pRenderProperties(NULL)
, m_AutomaticCreationOf3DViewInstance(true)
, m_pRelativeMatrix(NULL)
{
	m_pStructInstance= new GLC_StructInstance(pRep);
	setName(m_pStructInstance->name());

	// Update instance
	m_pStructInstance->structOccurenceCreated(this);
}

// Copy constructor
GLC_StructOccurence::GLC_StructOccurence(GLC_WorldHandle* pWorldHandle, const GLC_StructOccurence& structOccurence, bool shareInstance)
: m_Uid(glc::GLC_GenID())
, m_pWorldHandle(pWorldHandle)
, m_pNumberOfOccurence(NULL)
, m_pStructInstance(NULL)
, m_pParent(NULL)
, m_Childs()
, m_AbsoluteMatrix(structOccurence.m_AbsoluteMatrix)
, m_OccurenceNumber(0)
, m_IsVisible(structOccurence.m_IsVisible)
, m_pRenderProperties(NULL)
, m_AutomaticCreationOf3DViewInstance(structOccurence.m_AutomaticCreationOf3DViewInstance)
, m_pRelativeMatrix(NULL)
{
	if (shareInstance)
	{
		m_pStructInstance= structOccurence.m_pStructInstance;
		m_pNumberOfOccurence= structOccurence.m_pNumberOfOccurence;
		++(*m_pNumberOfOccurence);
	}
	else
	{
		m_pNumberOfOccurence= new int(1);
		m_pStructInstance= new GLC_StructInstance(structOccurence.m_pStructInstance);
	}


	// Test if structOccurence has representation and has a shader
	GLuint shaderId= 0;
	bool instanceIsSelected= false;
	if ((NULL != m_pWorldHandle) && (NULL != structOccurence.m_pWorldHandle) && structOccurence.m_pWorldHandle->collection()->contains(structOccurence.id()))
	{
		GLC_3DViewInstance* p3DViewInstance= structOccurence.m_pWorldHandle->collection()->instanceHandle(structOccurence.id());

		if(structOccurence.m_pWorldHandle->collection()->isInAShadingGroup(structOccurence.id()))
		{
			shaderId= structOccurence.m_pWorldHandle->collection()->shadingGroup(structOccurence.id());
		}

		instanceIsSelected= p3DViewInstance->isSelected();
		// Test the rendering properties
		if (! p3DViewInstance->renderPropertiesHandle()->isDefault())
		{
			m_pRenderProperties= new GLC_RenderProperties(*(p3DViewInstance->renderPropertiesHandle()));
		}
	}
	else if (NULL != structOccurence.m_pRenderProperties)
	{
		m_pRenderProperties= new GLC_RenderProperties(*(structOccurence.m_pRenderProperties));
	}

	// Inform the world Handle
	if (NULL != m_pWorldHandle)
	{
		m_pWorldHandle->addOccurence(this, instanceIsSelected, shaderId);
		if (NULL != m_pRenderProperties && this->has3DViewInstance())
		{
			m_pWorldHandle->collection()->instanceHandle(id())->setRenderProperties(*m_pRenderProperties);
			delete m_pRenderProperties;
			m_pRenderProperties= NULL;
		}
	}

	// Check flexibility
	if (NULL != structOccurence.m_pRelativeMatrix)
	{
		m_pRelativeMatrix= new GLC_Matrix4x4(*(structOccurence.m_pRelativeMatrix));
	}

	// Update Absolute matrix
	updateAbsoluteMatrix();


	// Create childs
	const int size= structOccurence.childCount();
	for (int i= 0; i < size; ++i)
	{
		GLC_StructOccurence* pChild= structOccurence.child(i)->clone(m_pWorldHandle, true);
		addChild(pChild);
	}
	updateChildrenAbsoluteMatrix();
	// Update instance
	m_pStructInstance->structOccurenceCreated(this);
}

// Destructor
GLC_StructOccurence::~GLC_StructOccurence()
{
	//qDebug() << "Delete " << id();
	Q_ASSERT(m_pNumberOfOccurence != NULL);
	// Remove from the GLC_WorldHandle
	if (NULL != m_pWorldHandle)
	{
		m_pWorldHandle->removeOccurence(this);
	}

	// Remove Childs
	const int size= m_Childs.size();
	for (int i= 0; i < size; ++i)
	{
		GLC_StructOccurence* pChild= m_Childs.first();
		removeChild(pChild);
		delete pChild;
	}
	// Update number of occurence and instance
	if ((--(*m_pNumberOfOccurence)) == 0)
	{
		delete m_pStructInstance;
		delete m_pNumberOfOccurence;
	}
	else
	{
		m_pStructInstance->structOccurenceDeleted(this);
		if (!m_pStructInstance->hasStructOccurence())
		{

			QStringList errorList;
			errorList << "StructOccurence count error";
			errorList << ("ref name = " + m_pStructInstance->structReference()->name());
			GLC_ErrorLog::addError(errorList);

			delete m_pStructInstance;
			//delete m_pNumberOfOccurence;
		}
	}

	delete m_pRenderProperties;
	delete m_pRelativeMatrix;
}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

GLC_Matrix4x4 GLC_StructOccurence::occurrenceRelativeMatrix() const
{
	GLC_Matrix4x4 matrix;
	if (NULL != m_pRelativeMatrix)
	{
		matrix= *m_pRelativeMatrix;
	}
	return matrix;
}

bool GLC_StructOccurence::hasRepresentation() const
{
	if ((NULL != m_pStructInstance) && (m_pStructInstance->hasStructOccurence()))
	{
		return this->structReference()->hasRepresentation();
	}
	else return false;
}

bool GLC_StructOccurence::has3DViewInstance() const
{
	if ( NULL != m_pWorldHandle)
	{
		return m_pWorldHandle->collection()->contains(m_Uid);
	}
	else return false;
}

bool GLC_StructOccurence::canBeAddedToChildren(GLC_StructOccurence* pOccurence) const
{
	bool canBeAdded= false;
	if ((NULL != m_pStructInstance) && (m_pStructInstance->hasStructOccurence()) && (NULL != pOccurence->m_pStructInstance) && (NULL != pOccurence->structReference()))
	{
		if (this->structReference() != pOccurence->structReference())
		{
			QSet<GLC_StructReference*> thisRefSet= GLC_StructOccurence::parentsReferences(this);
			thisRefSet << this->structReference();
			QSet<GLC_StructReference*> childRefSet= pOccurence->childrenReferences();

			canBeAdded= thisRefSet == (thisRefSet - childRefSet);
		}
	}
	else
	{
		canBeAdded= true;
	}
	return canBeAdded;
}

QList<GLC_StructOccurence*> GLC_StructOccurence::subOccurenceList() const
{
	QList<GLC_StructOccurence*> subOccurence;
	const int childCount= m_Childs.size();
	for (int i= 0; i < childCount; ++i)
	{
		GLC_StructOccurence* pCurrentChild= m_Childs.at(i);
		subOccurence.append(pCurrentChild);
		if (pCurrentChild->hasChild())
		{
			subOccurence.append(pCurrentChild->subOccurenceList());
		}
	}

	return subOccurence;
}

unsigned int GLC_StructOccurence::numberOfFaces() const
{
	unsigned int result= 0;
	if (hasRepresentation())
	{
		result= structInstance()->structReference()->numberOfFaces();
	}

	const int size= m_Childs.size();
	for (int i= 0; i < size; ++i)
	{
		result+= m_Childs.at(i)->numberOfFaces();
	}

	return result;
}

unsigned int GLC_StructOccurence::numberOfVertex() const
{
	unsigned int result= 0;
	if (hasRepresentation())
	{
		result= structInstance()->structReference()->numberOfVertex();
	}
	const int size= m_Childs.size();
	for (int i= 0; i < size; ++i)
	{
		result+= m_Childs.at(i)->numberOfVertex();
	}

	return result;
}

// Get number of materials
unsigned int GLC_StructOccurence::numberOfMaterials() const
{
	unsigned int result= 0;
	QSet<GLC_Material*> materialSet;
	if (hasRepresentation())
	{
		result= structInstance()->structReference()->numberOfMaterials();
	}

	const int size= m_Childs.size();
	for (int i= 0; i < size; ++i)
	{
		materialSet.unite(m_Childs.at(i)->materialSet());
	}
	result= static_cast<unsigned int>(materialSet.size());

	return result;
}

// Get materials List
QSet<GLC_Material*> GLC_StructOccurence::materialSet() const
{
	QSet<GLC_Material*> materialSet;
	if (hasRepresentation())
	{
		materialSet= structInstance()->structReference()->materialSet();
	}

	const int size= m_Childs.size();
	for (int i= 0; i < size; ++i)
	{
		materialSet.unite(m_Childs.at(i)->materialSet());
	}

	return materialSet;
}

// Clone the occurence
GLC_StructOccurence* GLC_StructOccurence::clone(GLC_WorldHandle* pWorldHandle, bool shareInstance) const
{
	return new GLC_StructOccurence(pWorldHandle, *this, shareInstance);
}

// Return true if the occurence is visible
bool GLC_StructOccurence::isVisible() const
{
	bool isHidden= true;

	if ((NULL != m_pWorldHandle) && m_pWorldHandle->collection()->contains(m_Uid))
	{
		isHidden= !m_pWorldHandle->collection()->instanceHandle(m_Uid)->isVisible();
	}
	else if (childCount() > 0)
	{
		const int size= childCount();
		int i= 0;
		while ((i < size) && isHidden)
		{
			isHidden= isHidden && !child(i)->isVisible();
			++i;
		}
	}
	else
	{
		isHidden= !m_IsVisible;
	}
	return !isHidden;
}

// Return the occurence Bounding Box
GLC_BoundingBox GLC_StructOccurence::boundingBox() const
{
	GLC_BoundingBox boundingBox;

	if (NULL != m_pWorldHandle)
	{
		if (has3DViewInstance())
		{
			Q_ASSERT(m_pWorldHandle->collection()->contains(id()));
			boundingBox= m_pWorldHandle->collection()->instanceHandle(id())->boundingBox();
		}
		else
		{
			if (hasChild())
			{
				QList<GLC_StructOccurence*> childrenList= children();
				const int size= childrenList.size();

				for (int i= 0; i < size; ++i)
				{
					boundingBox.combine(childrenList.at(i)->boundingBox());
				}
			}
		}
	}

	return boundingBox;
}

unsigned int GLC_StructOccurence::nodeCount() const
{
	unsigned int result= 1;
	const int size= m_Childs.size();
	for (int i= 0; i < size; ++i)
	{
		result+= m_Childs.at(i)->nodeCount();
	}
	return result;
}

QSet<GLC_StructReference*> GLC_StructOccurence::childrenReferences() const
{
	QSet<GLC_StructReference*> refChildrenSet;
	const int childCount= m_Childs.size();
	for (int i= 0; i < childCount; ++i)
	{
		GLC_StructOccurence* pCurrentChild= m_Childs.at(i);
		if ((NULL != pCurrentChild->structInstance()) && (NULL != pCurrentChild->structReference()))
		{
			refChildrenSet << pCurrentChild->structReference();
		}
	}

	return refChildrenSet;
}

QSet<GLC_StructReference*> GLC_StructOccurence::parentsReferences(const GLC_StructOccurence* pOccurence)
{
	QSet<GLC_StructReference*> parentSet;
	GLC_StructOccurence* pParent= pOccurence->parent();
	if (NULL != pParent)
	{
		if ((NULL != pParent->structInstance()) && (NULL != pParent->structReference()))
		{
			parentSet << pParent->structReference();
			parentSet.unite(GLC_StructOccurence::parentsReferences(pParent));
		}
	}

	return parentSet;
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Update the absolute matrix
GLC_StructOccurence* GLC_StructOccurence::updateAbsoluteMatrix()
{
	GLC_Matrix4x4 relativeMatrix;
	if (NULL == m_pRelativeMatrix)
	{
		relativeMatrix= m_pStructInstance->relativeMatrix();
	}
	else
	{
		relativeMatrix= *m_pRelativeMatrix;
	}

	if (NULL != m_pParent)
	{
		m_AbsoluteMatrix= m_pParent->absoluteMatrix() * relativeMatrix;
	}
	else
	{
		m_AbsoluteMatrix= relativeMatrix;
	}
	// If the occurence have a representation, update it.

	if ((NULL != m_pWorldHandle) && m_pWorldHandle->collection()->contains(m_Uid))
	{
		m_pWorldHandle->collection()->instanceHandle(m_Uid)->setMatrix(m_AbsoluteMatrix);
	}
	return this;
}

// Update children obsolute Matrix
GLC_StructOccurence* GLC_StructOccurence::updateChildrenAbsoluteMatrix()
{
	updateAbsoluteMatrix();
	const int size= m_Childs.size();
	for (int i= 0; i < size; ++i)
	{
		m_Childs[i]->updateChildrenAbsoluteMatrix();
	}
	return this;
}

// Add Child
void GLC_StructOccurence::addChild(GLC_StructOccurence* pChild)
{
	Q_ASSERT(pChild->isOrphan());
	Q_ASSERT((NULL == pChild->m_pWorldHandle) || (m_pWorldHandle == pChild->m_pWorldHandle));

	//qDebug() << "Add Child " << pChild->name() << "id=" << pChild->id() << " to " << name() << " id=" << id();
	// Add the child to the list of child
	// Get occurence reference
	m_Childs.append(pChild);
	pChild->m_pParent= this;
	if (NULL == pChild->m_pWorldHandle)
	{
		pChild->setWorldHandle(m_pWorldHandle);
	}
	pChild->updateChildrenAbsoluteMatrix();
}

// Add Child instance and returns the newly created occurence
GLC_StructOccurence* GLC_StructOccurence::addChild(GLC_StructInstance* pInstance)
{
	GLC_StructOccurence* pOccurence;
	pOccurence= new GLC_StructOccurence(pInstance, m_pWorldHandle);

	addChild(pOccurence);

	return pOccurence;
}

// make the occurence orphan
void GLC_StructOccurence::makeOrphan()
{
	//qDebug() << "GLC_StructOccurence::makeOrphan() " << id();
	//qDebug() << name() << " " << id();
	Q_ASSERT(!isOrphan());
	m_pParent->removeChild(this);
	//qDebug() << "GLC_StructOccurence::makeOrphan() DONE!";
}

// Remove the specified child
bool GLC_StructOccurence::removeChild(GLC_StructOccurence* pChild)
{
	Q_ASSERT(pChild->m_pParent == this);
	Q_ASSERT(m_Childs.contains(pChild));
	pChild->m_pParent= NULL;
	pChild->detach();

	return m_Childs.removeOne(pChild);
}


// Reverse Normals of this Occurence and childs
void GLC_StructOccurence::reverseNormals()
{
	if (has3DViewInstance())
	{
		m_pWorldHandle->collection()->instanceHandle(id())->reverseGeometriesNormals();
	}
}

// Check the presence of representation
bool GLC_StructOccurence::create3DViewInstance()
{
	bool creationSuccess= false;
	if ((NULL != m_pWorldHandle) && hasRepresentation())
	{
		GLC_3DRep* p3DRep= dynamic_cast<GLC_3DRep*>(structReference()->representationHandle());
		if (NULL != p3DRep)
		{
			GLC_3DViewInstance instance(*p3DRep);
			instance.setName(name());

			// Force instance representation id
			instance.setId(id());

			if (NULL != m_pRenderProperties)
			{
				instance.setRenderProperties(*m_pRenderProperties);
				delete m_pRenderProperties;
				m_pRenderProperties= NULL;
			}

			creationSuccess= m_pWorldHandle->collection()->add(instance);
			m_pWorldHandle->collection()->setVisibility(m_Uid, m_IsVisible);
			if (m_pWorldHandle->selectionSetHandle()->contains(m_Uid))
			{
				m_pWorldHandle->collection()->select(m_Uid);
			}
		}
	}
	return creationSuccess;
}

bool GLC_StructOccurence::remove3DViewInstance()
{
	if (NULL != m_pWorldHandle)
	{
		return m_pWorldHandle->collection()->remove(m_Uid);
	}
	else return false;
}

// Set the occurence world Handle
void GLC_StructOccurence::setWorldHandle(GLC_WorldHandle* pWorldHandle)
{
	// Check if world handles are equal
	if (m_pWorldHandle == pWorldHandle) return;

	if (NULL != m_pWorldHandle)
	{
		m_pWorldHandle->removeOccurence(this);
	}

	m_pWorldHandle= pWorldHandle;

	if (NULL != m_pWorldHandle)
	{
		m_pWorldHandle->addOccurence(this);
		m_pWorldHandle->collection()->setVisibility(m_Uid, m_IsVisible);

		const int size= m_Childs.size();
		for (int i= 0; i < size; ++i)
		{
			m_Childs[i]->setWorldHandle(m_pWorldHandle);
		}
	}
}

// Load the representation and return true if success
bool GLC_StructOccurence::loadRepresentation()
{
	Q_ASSERT(!this->has3DViewInstance());

	bool loadSuccess= false;
	if (hasRepresentation())
	{
		GLC_StructReference* pReference= this->structReference();
		if (pReference->representationIsLoaded())
		{
			loadSuccess= create3DViewInstance();
		}
		else
		{
			loadSuccess=  m_pStructInstance->structReference()->loadRepresentation();
			if (loadSuccess && !m_AutomaticCreationOf3DViewInstance)
			{
				loadSuccess= create3DViewInstance();
			}
		}
	}

	return loadSuccess;
}

// UnLoad the representation and return true if success
bool GLC_StructOccurence::unloadRepresentation()
{
	bool unloadResult= false;
	if (hasRepresentation())
	{
		GLC_StructReference* pRef= this->structReference();
		if (pRef->representationIsLoaded())
		{
			if (this->has3DViewInstance())
			{
				unloadResult= m_pWorldHandle->collection()->remove(m_Uid);
				QSet<GLC_StructOccurence*> occurenceSet= pRef->setOfStructOccurence();
				QSet<GLC_StructOccurence*>::const_iterator iOcc= occurenceSet.constBegin();
				bool unloadReferenceRep= true;
				while (occurenceSet.constEnd() != iOcc)
				{
					unloadReferenceRep= unloadReferenceRep && !(*iOcc)->has3DViewInstance();
					++iOcc;
				}
				if (unloadReferenceRep)
				{
					pRef->unloadRepresentation();
				}
			}
		}
	}
	return unloadResult;
}

unsigned int GLC_StructOccurence::updateOccurenceNumber(unsigned int n)
{
	m_OccurenceNumber= n++;
	const int childCount= m_Childs.size();
	for (int i= 0; i < childCount; ++i)
	{
		n= m_Childs[i]->updateOccurenceNumber(n);
	}
	return n;
}

void GLC_StructOccurence::setVisibility(bool visibility)
{
	m_IsVisible= visibility;
	if (has3DViewInstance())
	{
		m_pWorldHandle->collection()->setVisibility(m_Uid, m_IsVisible);
	}
	const int childCount= m_Childs.size();
	for (int i= 0; i < childCount; ++i)
	{
		m_Childs[i]->setVisibility(m_IsVisible);
	}
}

void GLC_StructOccurence::setRenderProperties(const GLC_RenderProperties& renderProperties)
{
	delete m_pRenderProperties;
	if (has3DViewInstance())
	{
		m_pWorldHandle->collection()->instanceHandle(m_Uid)->setRenderProperties(renderProperties);
	}
	else if (hasChild())
	{
		const int childCount= m_Childs.size();
		for (int i= 0; i < childCount; ++i)
		{
			m_Childs[i]->setRenderProperties(renderProperties);
		}
	}
	else
	{
		m_pRenderProperties= new GLC_RenderProperties(renderProperties);
	}
}

void GLC_StructOccurence::removeEmptyChildren()
{
	QList<GLC_StructOccurence*>::iterator iChild= m_Childs.begin();
	while (m_Childs.constEnd() != iChild)
	{
		if (!((*iChild)->hasChild()) && !((*iChild)->hasRepresentation()))
		{
			delete *iChild;
			iChild= m_Childs.erase(iChild);
		}
		else
		{
			(*iChild)->removeEmptyChildren();
			++iChild;
		}
	}
}

void GLC_StructOccurence::setReference(GLC_StructReference* pRef)
{
	Q_ASSERT(m_pStructInstance->structReference() == NULL);
	Q_ASSERT((*m_pNumberOfOccurence) == 1);

	if (pRef->hasStructInstance())
	{
		GLC_StructInstance* pExistingInstance= pRef->firstInstanceHandle();
		if (pExistingInstance->hasStructOccurence())
		{
			GLC_StructOccurence* pFirstOccurence= pExistingInstance->firstOccurenceHandle();
			QList<GLC_StructOccurence*> childs= pFirstOccurence->m_Childs;
			const int size= childs.size();
			for (int i= 0; i < size; ++i)
			{
				GLC_StructOccurence* pChild= childs.at(i)->clone(m_pWorldHandle, true);
				addChild(pChild);
			}

			QList<GLC_StructInstance*> instances= pRef->listOfStructInstances();
			const int instanceCount= instances.size();
			int i= 0;
			bool continu= true;
			while (continu && (i < instanceCount))
			{
				if (m_pStructInstance == instances.at(i))
				{
					continu= false;
					delete m_pNumberOfOccurence;
					m_pNumberOfOccurence= instances.at(i)->firstOccurenceHandle()->m_pNumberOfOccurence;
					++(*m_pNumberOfOccurence);
				}
				++i;
			}
		}
	}

	m_pStructInstance->setReference(pRef);
}

void GLC_StructOccurence::makeFlexible(const GLC_Matrix4x4& relativeMatrix)
{
	delete m_pRelativeMatrix;
	m_pRelativeMatrix= new GLC_Matrix4x4(relativeMatrix);

	updateChildrenAbsoluteMatrix();
}

void GLC_StructOccurence::makeRigid()
{
	delete m_pRelativeMatrix;
	m_pRelativeMatrix= NULL;

	updateChildrenAbsoluteMatrix();
}


//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////

void GLC_StructOccurence::detach()
{
	if (NULL != m_pWorldHandle)
	{
		// retrieve renderProperties if needed
		if (m_pWorldHandle->collection()->contains(m_Uid))
		{
			GLC_3DViewInstance* pInstance= m_pWorldHandle->collection()->instanceHandle(m_Uid);
			if (!pInstance->renderPropertiesHandle()->isDefault())
			{
				Q_ASSERT(NULL == m_pRenderProperties);
				m_pRenderProperties= new GLC_RenderProperties(*(pInstance->renderPropertiesHandle()));
			}
		}
		m_pWorldHandle->removeOccurence(this);
		m_pWorldHandle= NULL;
		if (!m_Childs.isEmpty())
		{
			const int size= m_Childs.size();
			for (int i= 0; i < size; ++i)
			{
				m_Childs[i]->detach();
			}
		}
	}
}
