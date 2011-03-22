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
//! \file glc_octreenode.cpp implementation for the GLC_OctreeNode class.

#include "glc_octreenode.h"

bool GLC_OctreeNode::m_useBoundingSphere= true;

GLC_OctreeNode::GLC_OctreeNode(const GLC_BoundingBox& boundingBox, GLC_OctreeNode* pParent)
: m_BoundingBox(boundingBox)
, m_pParent(pParent)
, m_Children()
, m_3DViewInstanceSet()
, m_Empty(true)
{


}

GLC_OctreeNode::GLC_OctreeNode(const GLC_OctreeNode& octreeNode, GLC_OctreeNode* pParent)
: m_BoundingBox(octreeNode.m_BoundingBox)
, m_pParent(pParent)
, m_Children()
, m_3DViewInstanceSet(octreeNode.m_3DViewInstanceSet)
, m_Empty(octreeNode.m_Empty)
{
	if (!octreeNode.m_Children.isEmpty())
	{
		const int size= octreeNode.m_Children.size();
		for (int i= 0; i < size; ++i)
		{
			m_Children.append(new GLC_OctreeNode(*(octreeNode.m_Children.at(i)), this));
		}
	}
}

GLC_OctreeNode::~GLC_OctreeNode()
{
	const int size= m_Children.size();
	for (int i= 0; i < size; ++i)
	{
		delete m_Children.at(i);
	}
}

bool GLC_OctreeNode::intersectionWithBoundingSphereUsed()
{
	return m_useBoundingSphere;
}

QSet<GLC_3DViewInstance*> GLC_OctreeNode::setOfIntersectedInstances(const GLC_BoundingBox& bBox)
{
	QSet<GLC_3DViewInstance*> instanceSet;
	if (intersect(bBox))
	{
		QSet<GLC_3DViewInstance*>::iterator iInstance= m_3DViewInstanceSet.begin();
		while (m_3DViewInstanceSet.constEnd() != iInstance)
		{
			if ((*iInstance)->boundingBox().intersect(bBox))
			{
				instanceSet << *(iInstance);
			}
			++iInstance;
		}
		const int childCount= m_Children.size();
		for (int i= 0; i < childCount; ++i)
		{
			instanceSet.unite(m_Children[i]->setOfIntersectedInstances(bBox));
		}
	}

	return instanceSet;
}
//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

void GLC_OctreeNode::addChildren()
{
	Q_ASSERT(m_Children.isEmpty());
	Q_ASSERT(!m_BoundingBox.isEmpty());

	const double xLower=  m_BoundingBox.lowerCorner().x();
	const double yLower=  m_BoundingBox.lowerCorner().y();
	const double zLower=  m_BoundingBox.lowerCorner().z();

	const double xUpper=  m_BoundingBox.upperCorner().x();
	const double dX= (xUpper - xLower) / 2.0;
	const double yUpper=  m_BoundingBox.upperCorner().y();
	const double dY= (yUpper - yLower) / 2.0;
	const double zUpper=  m_BoundingBox.upperCorner().z();
	const double dZ= (zUpper - zLower) / 2.0;


	// Add 8 Children
	GLC_Point3d lower;
	GLC_Point3d upper;
	GLC_OctreeNode* pOctreeNode= NULL;

	{ // Child 1
		lower.setVect(xLower, yLower, zLower);
		upper.setVect(xLower + dX, yLower + dY, zLower + dZ);
		GLC_BoundingBox box(lower, upper);
		pOctreeNode= new GLC_OctreeNode(box, this);
		m_Children.append(pOctreeNode);
	}
	{ // Child 2
		lower.setVect(xLower + dX, yLower, zLower);
		upper.setVect(xUpper, yLower + dY, zLower + dZ);
		GLC_BoundingBox box(lower, upper);
		pOctreeNode= new GLC_OctreeNode(box, this);
		m_Children.append(pOctreeNode);
	}
	{ // Child 3
		lower.setVect(xLower + dX, yLower + dY, zLower);
		upper.setVect(xUpper, yUpper, zLower + dZ);
		GLC_BoundingBox box(lower, upper);
		pOctreeNode= new GLC_OctreeNode(box, this);
		m_Children.append(pOctreeNode);
	}
	{ // Child 4
		lower.setVect(xLower, yLower + dY, zLower);
		upper.setVect(xLower + dX, yUpper, zLower + dZ);
		GLC_BoundingBox box(lower, upper);
		pOctreeNode= new GLC_OctreeNode(box, this);
		m_Children.append(pOctreeNode);
	}
	{ // Child 5
		lower.setVect(xLower, yLower, zLower + dZ);
		upper.setVect(xLower + dX, yLower + dY, zUpper);
		GLC_BoundingBox box(lower, upper);
		pOctreeNode= new GLC_OctreeNode(box, this);
		m_Children.append(pOctreeNode);
	}
	{ // Child 6
		lower.setVect(xLower + dX, yLower, zLower + dZ);
		upper.setVect(xUpper, yLower + dY, zUpper);
		GLC_BoundingBox box(lower, upper);
		pOctreeNode= new GLC_OctreeNode(box, this);
		m_Children.append(pOctreeNode);
	}
	{ // Child 7
		lower.setVect(xLower + dX, yLower + dY, zLower + dZ);
		upper.setVect(xUpper, yUpper, zUpper);
		GLC_BoundingBox box(lower, upper);
		pOctreeNode= new GLC_OctreeNode(box, this);
		m_Children.append(pOctreeNode);
	}
	{ // Child 8
		lower.setVect(xLower, yLower + dY, zLower + dZ);
		upper.setVect(xLower + dX, yUpper, zUpper);
		GLC_BoundingBox box(lower, upper);
		pOctreeNode= new GLC_OctreeNode(box, this);
		m_Children.append(pOctreeNode);
	}
}


void GLC_OctreeNode::addInstance(GLC_3DViewInstance* pInstance, int depth)
{
	m_Empty= false;
	const GLC_BoundingBox instanceBox= pInstance->boundingBox();
	// Check if the instance's bounding box intersect this node bounding box
	if (!instanceBox.isEmpty() && intersect(instanceBox))
	{
		if (0 == depth)
		{
			m_3DViewInstanceSet.insert(pInstance);
		}
		else
		{
			if (m_Children.isEmpty())
			{
				// Create children
				addChildren();
			}
			QVector<bool> childIntersect(8);
			bool allIntersect= true;
			bool currentIntersect= false;
			for (int i= 0; i < 8; ++i)
			{
				currentIntersect= m_Children.at(i)->intersect(instanceBox);
				allIntersect= allIntersect && currentIntersect;
				childIntersect[i]= currentIntersect;
			}
			if (allIntersect)
			{
				m_3DViewInstanceSet.insert(pInstance);
			}
			else
			{
				for (int i= 0; i < 8; ++i)
				{
					if (childIntersect[i])
					{
						m_Children[i]->addInstance(pInstance, depth - 1);
					}
				}
			}
		}

	}
}


void GLC_OctreeNode::updateViewableInstances(const GLC_Frustum& frustum, QSet<GLC_3DViewInstance*>* pInstanceSet)
{

	bool firstCall= false;
	// Create the set of viewable instance if necessary
	if (NULL == pInstanceSet)
	{
		pInstanceSet= new QSet<GLC_3DViewInstance*>();
		firstCall= true;
	}

	// Test the localisation of current octree node
	GLC_Frustum::Localisation nodeLocalisation= frustum.localizeBoundingBox(m_BoundingBox);
	if (nodeLocalisation == GLC_Frustum::OutFrustum)
	{
		disableViewFlag(pInstanceSet);
	}
	else if (nodeLocalisation == GLC_Frustum::InFrustum)
	{
		unableViewFlag(pInstanceSet);
	}
	else // The current node intersect the frustum
	{
		QSet<GLC_3DViewInstance*>::iterator iInstance= m_3DViewInstanceSet.begin();
		while (m_3DViewInstanceSet.constEnd() != iInstance)
		{
			// Test if the instances is in the viewable set
			if (!pInstanceSet->contains(*iInstance))
			{
				GLC_3DViewInstance* pCurrentInstance= (*iInstance);
				// Test the localisation of the current instance
				GLC_Frustum::Localisation instanceLocalisation= frustum.localizeBoundingBox(pCurrentInstance->boundingBox());

				if (instanceLocalisation == GLC_Frustum::OutFrustum)
				{
					pCurrentInstance->setViewable(GLC_3DViewInstance::NoViewable);
				}
				else if (instanceLocalisation == GLC_Frustum::InFrustum)
				{
					pInstanceSet->insert(pCurrentInstance);
					pCurrentInstance->setViewable(GLC_3DViewInstance::FullViewable);
				}
				else
				{
					pInstanceSet->insert(pCurrentInstance);
					pCurrentInstance->setViewable(GLC_3DViewInstance::PartialViewable);
					//Update the geometries viewable property of the instance
					GLC_Matrix4x4 instanceMat= pCurrentInstance->matrix();
					const int size= pCurrentInstance->numberOfBody();
					for (int i= 0; i < size; ++i)
					{
						// Get the geometry bounding box
						GLC_BoundingBox geomBox= pCurrentInstance->geomAt(i)->boundingBox();
						GLC_Point3d center(instanceMat * geomBox.center());
						double radius= geomBox.boundingSphereRadius() * instanceMat.scalingX();
						GLC_Frustum::Localisation geomLocalisation= frustum.localizeSphere(center, radius);

						pCurrentInstance->setGeomViewable(i, geomLocalisation != GLC_Frustum::OutFrustum);
					}
				}
			}

			++iInstance;
		}
		const int size= m_Children.size();
		for (int i= 0; i < size; ++i)
		{
			m_Children.at(i)->updateViewableInstances(frustum, pInstanceSet);
		}
	}
	if (firstCall) delete pInstanceSet;
}


void GLC_OctreeNode::removeEmptyChildren()
{
	NodeList::iterator iList= m_Children.begin();
	while(m_Children.constEnd() != iList)
	{
		GLC_OctreeNode* pCurrentChild= *iList;
		if (pCurrentChild->isEmpty())
		{
			delete pCurrentChild;
			iList= m_Children.erase(iList);
		}
		else
		{
			pCurrentChild->removeEmptyChildren();
			if (pCurrentChild->isEmpty())
			{
				delete pCurrentChild;
				iList= m_Children.erase(iList);
			}
			else ++iList;
		}
	}
	// Update empty flag
	if (m_Children.isEmpty() && (NULL != m_pParent))
	{
		if (1 == m_3DViewInstanceSet.size())
		{
			m_pParent->m_3DViewInstanceSet.insert(*(m_3DViewInstanceSet.begin()));
			m_3DViewInstanceSet.clear();
		}
		m_Empty= m_3DViewInstanceSet.isEmpty();
	}
}

void GLC_OctreeNode::useBoundingSphereIntersection(bool use)
{
	m_useBoundingSphere= use;
}

void GLC_OctreeNode::unableViewFlag(QSet<GLC_3DViewInstance*>* pInstanceSet)
{
	QSet<GLC_3DViewInstance*>::iterator iInstance= m_3DViewInstanceSet.begin();
	while (m_3DViewInstanceSet.constEnd() != iInstance)
	{
		if (!pInstanceSet->contains(*iInstance))
		{
			(*iInstance)->setViewable(GLC_3DViewInstance::FullViewable);
			pInstanceSet->insert(*iInstance);
		}

		++iInstance;
	}
	const int size= m_Children.size();
	for (int i= 0; i < size; ++i)
	{
		m_Children.at(i)->unableViewFlag(pInstanceSet);
	}
}


void GLC_OctreeNode::disableViewFlag(QSet<GLC_3DViewInstance*>* pInstanceSet)
{
	QSet<GLC_3DViewInstance*>::iterator iInstance= m_3DViewInstanceSet.begin();
	while (m_3DViewInstanceSet.constEnd() != iInstance)
	{
		if (!pInstanceSet->contains(*iInstance))
			(*iInstance)->setViewable(GLC_3DViewInstance::NoViewable);

		++iInstance;
	}
	const int size= m_Children.size();
	for (int i= 0; i < size; ++i)
	{
		m_Children.at(i)->disableViewFlag(pInstanceSet);
	}
}

