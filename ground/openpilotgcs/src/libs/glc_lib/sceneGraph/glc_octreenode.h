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
//! \file glc_octreenode.h interface for the GLC_OctreeNode class.


#ifndef GLC_OCTREENODE_H_
#define GLC_OCTREENODE_H_

#include "glc_3dviewinstance.h"
#include "../glc_boundingbox.h"
#include "../glc_config.h"
#include "../viewport/glc_frustum.h"
#include <QList>
#include <QSet>

class GLC_LIB_EXPORT GLC_OctreeNode;

//////////////////////////////////////////////////////////////////////
//! \class GLC_OctreeNode
/*! \brief GLC_OctreeNode : A node of Space partioning implemented with octree */
//////////////////////////////////////////////////////////////////////
class GLC_OctreeNode
{
	typedef QList<GLC_OctreeNode*> NodeList;
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct a octree node from the given bounding box within the given octree node
	GLC_OctreeNode(const GLC_BoundingBox&, GLC_OctreeNode* pParent= NULL);

	//! Construct a octree node from the first given octree node within the second given octree node
	GLC_OctreeNode(const GLC_OctreeNode&, GLC_OctreeNode* pParent= NULL);

	//! Destructor
	virtual ~GLC_OctreeNode();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	// Return this octree node bounding box
	inline GLC_BoundingBox& boundingBox()
	{return m_BoundingBox;}

	//! Return True if this octree node intersect the bounding box
	inline bool intersect(const GLC_BoundingBox& boundingBox);

	//! Return true if this octree has child octree node
	inline bool hasChild() const
	{return !m_Children.isEmpty();}

	//! Return the child octree node at the given index
	/*! The child must exist*/
	inline GLC_OctreeNode* childAt(int i) const
	{
		Q_ASSERT(i < m_Children.size());
		return m_Children.at(i);
	}

	//! Return this octree node child octree node count
	inline int childCount() const
	{return m_Children.size();}

	//! Return true if this node contains 3D view instances
	inline bool hasGeometry() const
	{return !m_3DViewInstanceSet.isEmpty();}

	//! Return true if this octree node is empty
	/*! An empty node doesn't contains child and 3d view instance*/
	inline bool isEmpty() const
	{return m_Empty;}

	//! Return true if intersection are calculated with bounded sphere
	static bool intersectionWithBoundingSphereUsed();

	//! Return the list off instances inside or intersect the given bounding box
	QSet<GLC_3DViewInstance*> setOfIntersectedInstances(const GLC_BoundingBox& bBox);


//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Add 8 octree node children to this octree node
	void addChildren();

	//! Add 3d view instance in this octree node branch
	void addInstance(GLC_3DViewInstance*, int);

	//! Update 3d view instances visibility of this octree node branch from the given frustum
	/*! Viewable 3d view instance are inserted the the given set if exist also the set is created*/
	void updateViewableInstances(const GLC_Frustum&, QSet<GLC_3DViewInstance*>* pInstanceSet= NULL);

	//! Remove empty child octree node from this octree node
	void removeEmptyChildren();

	//! Set intersection to bounding sphere
	static void useBoundingSphereIntersection(bool);
//@}

//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////
private:
	//! Unable the node and sub node view flag
	void unableViewFlag(QSet<GLC_3DViewInstance*>*);

	//! Disable the node and sub node view flag
	void disableViewFlag(QSet<GLC_3DViewInstance*>*);

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Octree node bounding box
	GLC_BoundingBox m_BoundingBox;

	//! Parent Octree node
	GLC_OctreeNode* m_pParent;

	//! Children list of octree node
	NodeList m_Children;

	//! This node set of 3DViewInstance
	QSet<GLC_3DViewInstance*> m_3DViewInstanceSet;

	//! Flag to know if the node is empty
	bool m_Empty;

	//! Flag to know if intersection is calculated with bounding sphere
	static bool m_useBoundingSphere;

};

bool GLC_OctreeNode::intersect(const GLC_BoundingBox& boundingBox)
{
	if (m_useBoundingSphere)
		return m_BoundingBox.intersectBoundingSphere(boundingBox);
	else
		return m_BoundingBox.intersect(boundingBox);
}

#endif /* GLC_OCTREENODE_H_ */
