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

//! \file glc_3dviewcollection.cpp implementation of the GLC_3DViewCollection class.


#include "glc_3dviewcollection.h"
#include "../shading/glc_material.h"
#include "../glc_openglexception.h"
#include "../shading/glc_selectionmaterial.h"
#include "../glc_state.h"
#include "../shading/glc_shader.h"
#include "../viewport/glc_viewport.h"
#include "glc_spacepartitioning.h"

#include <QtDebug>

//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////

GLC_3DViewCollection::GLC_3DViewCollection()
: m_3DViewInstanceHash()
, m_pBoundingBox(NULL)
, m_SelectedInstances()
, m_ShadedPointerViewInstanceHash()
, m_ShaderGroup()
, m_MainInstances()
, m_IsInShowSate(true)
, m_UseLod(false)
, m_pViewport(NULL)
, m_pSpacePartitioning(NULL)
, m_UseSpacePartitioning(false)
{
}

GLC_3DViewCollection::~GLC_3DViewCollection()
{
	// Delete all collection's elements and the collection bounding box
	clear();
}
//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Add the specified shader to the collection
bool GLC_3DViewCollection::bindShader(GLuint shaderId)
{
	if (m_ShadedPointerViewInstanceHash.contains(shaderId))
	{
		return false;
	}
	else
	{
		PointerViewInstanceHash* pNodeHash= new PointerViewInstanceHash;
		m_ShadedPointerViewInstanceHash.insert(shaderId, pNodeHash);
		return true;
	}
}

// Unbind the specified shader from the collection
bool GLC_3DViewCollection::unBindShader(GLuint shaderId)
{
	bool result= false;
	if (m_ShadedPointerViewInstanceHash.contains(shaderId))
	{
		// Find node which use the shader
		QList<GLC_uint> nodeId(m_ShaderGroup.keys(shaderId));

		// Move these node in the standard hash and remove them from shader group
		PointerViewInstanceHash* pShaderNodeHash= m_ShadedPointerViewInstanceHash.take(shaderId);
		for (int i= 0; i < nodeId.size(); ++i)
		{
			const GLC_uint id= nodeId[i];
			GLC_3DViewInstance* pInstance= pShaderNodeHash->value(id);

			if (!pInstance->isSelected())
			{
				m_MainInstances.insert(id, pInstance);
			}
			else
			{
				m_SelectedInstances.insert(id, pInstance);
			}
			m_ShaderGroup.remove(id);
		}
		pShaderNodeHash->clear();
		delete pShaderNodeHash;
		result= true;
	}
	Q_ASSERT(!m_ShadedPointerViewInstanceHash.contains(shaderId));
	return result;
}

// Unbind All shader
bool GLC_3DViewCollection::unBindAllShader()
{
	bool result= true;
	HashList::iterator iEntry= m_ShadedPointerViewInstanceHash.begin();
	QList<GLuint> shaderList;
    while (iEntry != m_ShadedPointerViewInstanceHash.constEnd())
    {
    	shaderList.append(iEntry.key());
    	++iEntry;
    }
    const int size= shaderList.size();
    for (int i=0; i < size; ++i)
    {
    	result= result && unBindShader(shaderList[i]);
    }
    return result;
}

// Add GLC_3DViewInstance in the collection
bool GLC_3DViewCollection::add(const GLC_3DViewInstance& node, GLuint shaderID)
{
	bool result= false;
	const GLC_uint key= node.id();
	if (m_3DViewInstanceHash.contains(key))
	{
		qDebug() << "Instance already in collection";
		return false;
	}
	m_3DViewInstanceHash.insert(key, node);
	// Create an GLC_3DViewInstance pointer of the inserted instance
	ViewInstancesHash::iterator iNode= m_3DViewInstanceHash.find(key);
	GLC_3DViewInstance* pInstance= &(iNode.value());
	// Chose the hash where instance is
	if(0 != shaderID)
	{
		// Test if shaderId group exist
		if (m_ShadedPointerViewInstanceHash.contains(shaderID))
		{
			m_ShaderGroup.insert(key, shaderID);

			if(pInstance->isSelected())
			{
				m_SelectedInstances.insert(key, pInstance);
			}
			else
			{
				m_ShadedPointerViewInstanceHash.value(shaderID)->insert(key, pInstance);
			}
			result=true;
		}
	}
	else if (!pInstance->isSelected())
	{
		m_MainInstances.insert(key, pInstance);
		result=true;
	}
	else
	{
		m_SelectedInstances.insert(key, pInstance);
		result=true;
	}

	if(result)
	{
		// Bounding box validity
		if (NULL != m_pBoundingBox)
		{
			delete m_pBoundingBox;
			m_pBoundingBox= NULL;
		}
	}
	return result;
}
// Change instance shading group
void GLC_3DViewCollection::changeShadingGroup(GLC_uint instanceId, GLuint shaderId)
{
	// Test if the specified instance exist
	Q_ASSERT(m_3DViewInstanceHash.contains(instanceId));
	// Get the instance shading group
	const GLuint instanceShadingGroup= shadingGroup(instanceId);
	// Get a pointer to the instance
	GLC_3DViewInstance* pInstance= NULL;
	if (0 == instanceShadingGroup)
	{
		// The instance is not in a shading group
		if (m_MainInstances.contains(instanceId))
		{
			pInstance= m_MainInstances.take(instanceId);
		}
		else if (m_SelectedInstances.contains(instanceId))
		{
			// The instance is selected don't take it
			pInstance= m_SelectedInstances.value(instanceId);
		}
		else
		{
			Q_ASSERT(false);
		}
	}
	else
	{
		m_ShaderGroup.remove(instanceId);
		// The instance is in a shading group
		if (m_SelectedInstances.contains(instanceId))
		{
			// The instance is selected don't take it
			pInstance= m_SelectedInstances.value(instanceId);
		}
		else
		{
			pInstance= m_ShadedPointerViewInstanceHash.value(instanceShadingGroup)->take(instanceId);
		}

	}
	// Put the instance in specified shading group
	if (0 != shaderId)
	{
		m_ShaderGroup.insert(instanceId, shaderId);
		if (!pInstance->isSelected())
		{
			m_ShadedPointerViewInstanceHash.value(shaderId)->insert(instanceId, pInstance);
		}
	}
	else if (!pInstance->isSelected())
	{
		m_MainInstances.insert(instanceId, pInstance);
	}
}

// Delete geometry from the collection
bool GLC_3DViewCollection::remove(GLC_uint Key)
{

	ViewInstancesHash::iterator iNode= m_3DViewInstanceHash.find(Key);

	if (iNode != m_3DViewInstanceHash.end())
	{	// Ok, the key exist

		if (selectionSize() > 0)
		{
			// if the geometry is selected, unselect it
			unselect(Key);
		}

		m_MainInstances.remove(Key);

		m_3DViewInstanceHash.remove(Key);		// Delete the conteneur

		// Bounding box validity
		if (NULL != m_pBoundingBox)
		{
			delete m_pBoundingBox;
			m_pBoundingBox= NULL;
		}

		//qDebug("GLC_3DViewCollection::removeNode : Element succesfuly deleted");
		return true;

	}
	else
	{	// KO, key doesn't exist
		qDebug("GLC_Collection::removeNode : Element not deleted");
		return false;
	}

}

// Clear the collection
void GLC_3DViewCollection::clear(void)
{
	// Clear Selected node Hash Table
	m_SelectedInstances.clear();
	// Clear the not transparent Hash Table
	m_MainInstances.clear();
	// Clear Other Node Hash List
	HashList::iterator iEntry= m_ShadedPointerViewInstanceHash.begin();
    while (iEntry != m_ShadedPointerViewInstanceHash.constEnd())
    {
    	iEntry.value()->clear();
    	delete iEntry.value();
    	iEntry= m_ShadedPointerViewInstanceHash.erase(iEntry);
    }

	m_ShadedPointerViewInstanceHash.clear();
	m_ShaderGroup.clear();

	// Clear main Hash table
    m_3DViewInstanceHash.clear();

	// delete the boundingBox
	if (m_pBoundingBox != NULL)
	{
		delete m_pBoundingBox;
		m_pBoundingBox= NULL;
	}

	// delete the space partitioning
	delete m_pSpacePartitioning;
}

// Select a node
bool GLC_3DViewCollection::select(GLC_uint key, bool primitive)
{
	if (!m_3DViewInstanceHash.contains(key)) return false;
	//qDebug() << "GLC_Collection::select " << key;

	GLC_3DViewInstance* pSelectedNode;
	ViewInstancesHash::iterator iNode= m_3DViewInstanceHash.find(key);
	PointerViewInstanceHash::iterator iSelectedNode= m_SelectedInstances.find(key);

	if ((iNode != m_3DViewInstanceHash.end()) && (iSelectedNode == m_SelectedInstances.end()))
	{	// Ok, the key exist and the node is not selected
		pSelectedNode= &(iNode.value());
		m_SelectedInstances.insert(pSelectedNode->id(), pSelectedNode);

		// Remove Selected Node from is previous collection
		if (isInAShadingGroup(key))
		{
			m_ShadedPointerViewInstanceHash.value(shadingGroup(key))->remove(key);
			//qDebug() << "remove from shader list";
		}
		else
		{
			m_MainInstances.remove(key);
		}
		pSelectedNode->select(primitive);

		//qDebug("GLC_3DViewCollection::selectNode : Element succesfuly selected");
		return true;
	}
	else
	{	// KO, key doesn't exist or node allready selected
		qDebug("GLC_Collection::selectNode : Element not selected");
		return false;
	}
}

// Select all instances in current show state
void GLC_3DViewCollection::selectAll()
{
	unselectAll();
	ViewInstancesHash::iterator iNode= m_3DViewInstanceHash.begin();
	while (iNode != m_3DViewInstanceHash.end())
	{
		GLC_3DViewInstance *pCurrentInstance= &(iNode.value());
		const GLC_uint instanceId= pCurrentInstance->id();

		if (pCurrentInstance->isVisible() == m_IsInShowSate)
		{
			pCurrentInstance->select(false);
			m_SelectedInstances.insert(instanceId, pCurrentInstance);
			m_MainInstances.remove(instanceId);
			if(isInAShadingGroup(instanceId))
			{
				m_ShadedPointerViewInstanceHash.value(shadingGroup(instanceId))->remove(instanceId);
			}
		}
		iNode++;
	}
}

// unselect a node
bool GLC_3DViewCollection::unselect(GLC_uint key)
{
	GLC_3DViewInstance* pSelectedNode;

	PointerViewInstanceHash::iterator iSelectedNode= m_SelectedInstances.find(key);

	if (iSelectedNode != m_SelectedInstances.end())
	{	// Ok, the key exist and the node is selected
		iSelectedNode.value()->unselect();

		m_SelectedInstances.remove(key);

		pSelectedNode= iSelectedNode.value();

		// Insert Selected Node to the right collection
		if (isInAShadingGroup(key))
		{
			m_ShadedPointerViewInstanceHash.value(shadingGroup(key))->insert(key, pSelectedNode);
		}
		else
		{
			m_MainInstances.insert(key, pSelectedNode);
		}

		//qDebug("GLC_3DViewCollection::unselectNode : Node succesfuly unselected");
		return true;

	}
	else
	{	// KO, key doesn't exist or node allready selected
		//qDebug("GLC_3DViewCollection::unselectNode : Node not unselected");
		return false;
	}
}

// Unselect all Node
void GLC_3DViewCollection::unselectAll()
{
	PointerViewInstanceHash::iterator iSelectedNode= m_SelectedInstances.begin();

    while (iSelectedNode != m_SelectedInstances.end())
    {
    	GLC_3DViewInstance* pInstance= iSelectedNode.value();
    	pInstance->unselect();
		if (isInAShadingGroup(pInstance->id()))
		{
			m_ShadedPointerViewInstanceHash.value(shadingGroup(pInstance->id()))->insert(pInstance->id(), pInstance);
		}
		else
		{
			m_MainInstances.insert(pInstance->id(), pInstance);
		}

        ++iSelectedNode;
    }
    // Clear selected node hash table
    m_SelectedInstances.clear();
}

// Set the polygon mode for all Instance
void GLC_3DViewCollection::setPolygonModeForAll(GLenum face, GLenum mode)
{
	ViewInstancesHash::iterator iEntry= m_3DViewInstanceHash.begin();

    while (iEntry != m_3DViewInstanceHash.constEnd())
    {
    	// Update Instance Polygon Mode
    	iEntry.value().setPolygonMode(face, mode);
    	iEntry++;
    }

}

// Set Instance visibility
void GLC_3DViewCollection::setVisibility(const GLC_uint key, const bool visibility)
{
	ViewInstancesHash::iterator iNode= m_3DViewInstanceHash.find(key);
	if (iNode != m_3DViewInstanceHash.end())
	{	// Ok, the key exist
		iNode.value().setVisibility(visibility);
		// Bounding box validity
		if (NULL != m_pBoundingBox)
		{
			delete m_pBoundingBox;
			m_pBoundingBox= NULL;
		}
	}
}

// Show all instance of the collection
void GLC_3DViewCollection::showAll()
{
	ViewInstancesHash::iterator iEntry= m_3DViewInstanceHash.begin();

    while (iEntry != m_3DViewInstanceHash.constEnd())
    {
    	// Update Instance Polygon Mode
    	iEntry.value().setVisibility(true);
    	iEntry++;
    }

    // Bounding box validity
	if (NULL != m_pBoundingBox)
	{
		delete m_pBoundingBox;
		m_pBoundingBox= NULL;
	}

}

// Hide all instance of the collection
void GLC_3DViewCollection::hideAll()
{
	ViewInstancesHash::iterator iEntry= m_3DViewInstanceHash.begin();

    while (iEntry != m_3DViewInstanceHash.constEnd())
    {
    	// Update Instance Polygon Mode
    	iEntry.value().setVisibility(false);
    	iEntry++;
    }

	// Bounding box validity
	if (NULL != m_pBoundingBox)
	{
		delete m_pBoundingBox;
		m_pBoundingBox= NULL;
	}

}

// Bind the space partitioning
void GLC_3DViewCollection::bindSpacePartitioning(GLC_SpacePartitioning* pSpacePartitioning)
{
	Q_ASSERT(NULL != pSpacePartitioning);
	Q_ASSERT(pSpacePartitioning->collectionHandle() == this);

	delete m_pSpacePartitioning;
	m_pSpacePartitioning= pSpacePartitioning;
}

// Unbind the space partitioning
void GLC_3DViewCollection::unbindSpacePartitioning()
{
	delete m_pSpacePartitioning;
	m_pSpacePartitioning= NULL;
	m_UseSpacePartitioning= false;

	ViewInstancesHash::iterator iEntry= m_3DViewInstanceHash.begin();
    while (iEntry != m_3DViewInstanceHash.constEnd())
    {
    	// Update Instance viewable flag
    	iEntry.value().setViewable(GLC_3DViewInstance::FullViewable);
    	iEntry++;
    }

}

// Update the instance viewble state using frustrum culling
void GLC_3DViewCollection::updateInstanceViewableState(GLC_Matrix4x4* pMatrix)
{
	if ((NULL != m_pViewport) && m_UseSpacePartitioning && (NULL != m_pSpacePartitioning))
	{
		if (m_pViewport->updateFrustum(pMatrix))
			m_pSpacePartitioning->updateViewableInstances(m_pViewport->frustum());
	}
}
// Update the instance viewable state with the specified frustum
void GLC_3DViewCollection::updateInstanceViewableState(const GLC_Frustum& frustum)
{
	m_pSpacePartitioning->updateViewableInstances(frustum);
}

// Return all GLC_3DViewInstance from collection
QList<GLC_3DViewInstance*> GLC_3DViewCollection::instancesHandle()
{
	QList<GLC_3DViewInstance*> instancesList;

	ViewInstancesHash::iterator iEntry= m_3DViewInstanceHash.begin();

    while (iEntry != m_3DViewInstanceHash.constEnd())
    {
    	instancesList.append(&(iEntry.value()));
    	iEntry++;
    }
	return instancesList;
}

// Return all visible GLC_3DViewInstance from the collection
QList<GLC_3DViewInstance*> GLC_3DViewCollection::visibleInstancesHandle()
{
	QList<GLC_3DViewInstance*> instancesList;

	ViewInstancesHash::iterator iEntry= m_3DViewInstanceHash.begin();

    while (iEntry != m_3DViewInstanceHash.constEnd())
    {
    	if (iEntry.value().isVisible())
    	{
        	instancesList.append(&(iEntry.value()));
    	}
    	iEntry++;
    }
	return instancesList;

}

//! Return all viewable GLC_3DViewInstance from the collection
QList<GLC_3DViewInstance*> GLC_3DViewCollection::viewableInstancesHandle()
{
	QList<GLC_3DViewInstance*> instancesList;

	ViewInstancesHash::iterator iEntry= m_3DViewInstanceHash.begin();

    while (iEntry != m_3DViewInstanceHash.constEnd())
    {
    	if (iEntry.value().isVisible() == m_IsInShowSate)
    	{
        	instancesList.append(&(iEntry.value()));
    	}
    	iEntry++;
    }
	return instancesList;
}

// Return a GLC_3DViewInstance pointer from the collection
GLC_3DViewInstance* GLC_3DViewCollection::instanceHandle(GLC_uint Key)
{
	return &(m_3DViewInstanceHash[Key]);
}

// return the entire collection Bounding Box
GLC_BoundingBox GLC_3DViewCollection::boundingBox(void)
{
	// If the bounding box is not valid delete it
	setBoundingBoxValidity();

	// Check if the bounding box have to be updated
	if ((m_pBoundingBox == NULL) && !m_3DViewInstanceHash.isEmpty())
	{
		// There is objects in the collection and the collection or bounding box is not valid
		m_pBoundingBox= new GLC_BoundingBox();

		ViewInstancesHash::iterator iEntry= m_3DViewInstanceHash.begin();
	    while (iEntry != m_3DViewInstanceHash.constEnd())
	    {
	        if(iEntry.value().isVisible() == m_IsInShowSate)
	        {
	        	// Combine Collection BoundingBox with element Bounding Box
	        	m_pBoundingBox->combine(iEntry.value().boundingBox());
	        }
	        ++iEntry;
	    }
	}
	else if (NULL == m_pBoundingBox)
	{
		// The collection is empty and m_pBoundingBox == NULL
		m_pBoundingBox= new GLC_BoundingBox();
	}

	return *m_pBoundingBox;
}

// Return the number of drawable objects
int GLC_3DViewCollection::drawableObjectsSize() const
{
	// The number of object to drw
	int numberOffDrawnHit= 0;

	// Count the number off instance to draw
	ViewInstancesHash::const_iterator i= m_3DViewInstanceHash.begin();
	while (i != m_3DViewInstanceHash.constEnd())
	{
		//qDebug() << "transparent";
		if (i.value().isVisible() == m_IsInShowSate)
		{
			++numberOffDrawnHit;
		}
		++i;
	}
	return numberOffDrawnHit;
}

// Return instances name from the specified shading group
QList<QString> GLC_3DViewCollection::instanceNamesFromShadingGroup(GLuint shaderId) const
{
	QList<QString> listOfInstanceName;
	QList<GLC_uint> listOfInstanceNameId= m_ShaderGroup.keys(shaderId);
	if (!listOfInstanceNameId.isEmpty())
	{
		const int size= listOfInstanceNameId.size();
		for (int i= 0; i < size; ++i)
		{
			listOfInstanceName << m_3DViewInstanceHash.value(listOfInstanceNameId[i]).name();
		}
	}
	return listOfInstanceName;
}

// Return the number of used shading group
int GLC_3DViewCollection::numberOfUsedShadingGroup() const
{
	return m_ShaderGroup.values().toSet().size();
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_3DViewCollection::render(GLuint groupId, glc::RenderFlag renderFlag)
{
	if (!isEmpty())
	{
		if (renderFlag == glc::WireRenderFlag)
		{
	        glEnable(GL_POLYGON_OFFSET_FILL);
	        glPolygonOffset (1.0, 1.0);
		}
		if (GLC_State::isInSelectionMode())
		{
			glDisable(GL_BLEND);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
		}
		else
		{
			glEnable(GL_LIGHTING);
		}
		glDraw(groupId, renderFlag);

		if (renderFlag == glc::WireRenderFlag)
		{
	        glDisable(GL_POLYGON_OFFSET_FILL);
		}
	}
}
// Display all shader group
void GLC_3DViewCollection::renderShaderGroup(glc::RenderFlag renderFlag)
{
	if (!isEmpty())
	{
		if (GLC_State::isInSelectionMode())
		{
			glDisable(GL_BLEND);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
		}

		HashList::iterator iEntry= m_ShadedPointerViewInstanceHash.begin();
	    while (iEntry != m_ShadedPointerViewInstanceHash.constEnd())
	    {
	    	glDraw(iEntry.key(), renderFlag);
	    	++iEntry;
	    }
	}
}

// Display the specified collection group
void GLC_3DViewCollection::glDraw(GLuint groupId, glc::RenderFlag renderFlag)
{
	// Set render Mode and OpenGL state
	if (!GLC_State::isInSelectionMode() && (groupId == 0))
	{
		if (renderFlag == glc::TransparentRenderFlag)
		{
	        glEnable(GL_BLEND);
	        glDepthMask(GL_FALSE);
	        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		}
		else
		{
		    glDisable(GL_BLEND);
		    glDepthMask(GL_TRUE);
		    glEnable(GL_DEPTH_TEST);
		}
	}

	// Normal GLC_3DViewInstance
	if ((groupId == 0) && !m_MainInstances.isEmpty())
	{
		glDrawInstancesOf(&m_MainInstances, renderFlag);

	}
	// Selected GLC_3DVIewInstance
	else if ((groupId == 1) && !m_SelectedInstances.isEmpty())
	{
		if (GLC_State::selectionShaderUsed()) GLC_SelectionMaterial::useShader();

		glDrawInstancesOf(&m_SelectedInstances, renderFlag);

		if (GLC_State::selectionShaderUsed()) GLC_SelectionMaterial::unUseShader();
	}
	// GLC_3DViewInstance with shader
	else if (!m_ShadedPointerViewInstanceHash.isEmpty())
	{
	    if(m_ShadedPointerViewInstanceHash.contains(groupId) && !m_ShadedPointerViewInstanceHash.value(groupId)->isEmpty())
	    {
	    	PointerViewInstanceHash* pNodeHash= m_ShadedPointerViewInstanceHash.value(groupId);

	    	GLC_Shader::use(groupId);
	    	glDrawInstancesOf(pNodeHash, renderFlag);
	    	GLC_Shader::unuse();
	    }
	}

	// Restore OpenGL state
	if (renderFlag && !GLC_State::isInSelectionMode() && (groupId == 0))
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}
}

// Set the Bounding box validity
void GLC_3DViewCollection::setBoundingBoxValidity(void)
{
	if (NULL != m_pBoundingBox)
	{
		if (!m_3DViewInstanceHash.isEmpty())
		{
			// Check instance bounding box validity
			ViewInstancesHash::iterator iEntry= m_3DViewInstanceHash.begin();
		    while (iEntry != m_3DViewInstanceHash.constEnd())
		    {
		    	if (!iEntry.value().boundingBoxValidity())
		    	{
					delete m_pBoundingBox;
					m_pBoundingBox= NULL;
					return;
		    	}
		    	iEntry++;
		    }
		}
		else if (!m_pBoundingBox->isEmpty())
		{
			delete m_pBoundingBox;
			m_pBoundingBox= NULL;
		}
	}
}
