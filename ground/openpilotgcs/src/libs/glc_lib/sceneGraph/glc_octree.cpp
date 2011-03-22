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
//! \file glc_octree.cpp implementation for the GLC_Octree class.

#include "glc_octree.h"
#include "glc_octreenode.h"
#include "glc_3dviewcollection.h"
#include "../glc_factory.h"

int GLC_Octree::m_DefaultOctreeDepth= 3;

GLC_Octree::GLC_Octree(GLC_3DViewCollection* pCollection)
: GLC_SpacePartitioning(pCollection)
, m_pRootNode(NULL)
, m_OctreeDepth(m_DefaultOctreeDepth)
{


}

GLC_Octree::GLC_Octree(const GLC_Octree& octree)
: GLC_SpacePartitioning(octree)
, m_pRootNode(NULL)
, m_OctreeDepth(octree.m_OctreeDepth)
{
	if (NULL != octree.m_pRootNode)
	{
		m_pRootNode= new GLC_OctreeNode(*(octree.m_pRootNode));
	}
}

GLC_Octree::~GLC_Octree()
{
	delete m_pRootNode;
}

int  GLC_Octree::defaultDepth()
{
	return m_DefaultOctreeDepth;
}

QList<GLC_3DViewInstance*> GLC_Octree::listOfIntersectedInstances(const GLC_BoundingBox& bBox)
{
	if (NULL == m_pRootNode)
	{
		updateSpacePartitioning();
	}
	return m_pRootNode->setOfIntersectedInstances(bBox).toList();
}

void GLC_Octree::updateViewableInstances(const GLC_Frustum& frustum)
{
	if (NULL == m_pRootNode)
	{
		updateSpacePartitioning();
	}
	m_pRootNode->updateViewableInstances(frustum);
}

void GLC_Octree::updateSpacePartitioning()
{
	delete m_pRootNode;
	m_pRootNode= new GLC_OctreeNode(m_pCollection->boundingBox(true));
	// fill the octree
	QList<GLC_3DViewInstance*> instanceList(m_pCollection->instancesHandle());
	const int size= instanceList.size();
	for (int i= 0; i < size; ++i)
	{
		m_pRootNode->addInstance(instanceList.at(i), m_OctreeDepth);
	}
	m_pRootNode->removeEmptyChildren();
}

void GLC_Octree::clear()
{
	delete m_pRootNode;
	m_pRootNode= NULL;
}

void GLC_Octree::setDepth(int depth)
{
	m_OctreeDepth= depth;
	if (NULL != m_pRootNode)
	{
		updateSpacePartitioning();
	}
}

void GLC_Octree::createBox(GLC_Material* pMat, GLC_3DViewCollection* pCol)
{
	if (NULL == m_pRootNode)
	{
		updateSpacePartitioning();
	}

	if (NULL == pCol) pCol= m_pCollection;
	if (!m_pRootNode->isEmpty())
	{
		createBoxWithMaterial(pCol, m_pRootNode, pMat);
	}
}

void GLC_Octree::setDefaultDepth(int depth)
{
	m_DefaultOctreeDepth= depth;
}

void GLC_Octree::createBoxWithMaterial(GLC_3DViewCollection* pCol, GLC_OctreeNode* pNode, GLC_Material* pMat)
{
	if (!pNode->isEmpty())
	{
		if (pNode->hasGeometry())
		{
			GLC_3DViewInstance box= GLC_Factory::instance()->createBox(pNode->boundingBox());
			box.geomAt(0)->replaceMasterMaterial(pMat);
			pCol->add(box);
		}

		if (pNode->hasChild())
		{
			const int size= pNode->childCount();
			for (int i= 0; i < size; ++i)
			{
				createBoxWithMaterial(pCol, pNode->childAt(i), pMat);
			}
		}

	}

}

