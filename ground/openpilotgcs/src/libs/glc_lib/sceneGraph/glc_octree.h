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
//! \file glc_octree.h interface for the GLC_Octree class.

#ifndef GLC_OCTREE_H_
#define GLC_OCTREE_H_

#include "glc_spacepartitioning.h"
#include "../glc_config.h"

class GLC_OctreeNode;
class GLC_Material;
class GLC_3DViewInstance;

//////////////////////////////////////////////////////////////////////
//! \class GLC_Octree
/*! \brief GLC_Octree : represent space partioning implementation with octree */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Octree : public GLC_SpacePartitioning
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Create an empty octree of the given 3D view collection
	GLC_Octree(GLC_3DViewCollection*);

	//! Create the octree from the given octree
	GLC_Octree(const GLC_Octree&);

	//! Destructor
	virtual ~GLC_Octree();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the default octree Depth
	static int defaultDepth();

	//! Return the list off instances inside or intersect the given bounding box
	virtual QList<GLC_3DViewInstance*> listOfIntersectedInstances(const GLC_BoundingBox& bBox);

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Update the viewable 3d view instance of this octree from the given frustum
	virtual void updateViewableInstances(const GLC_Frustum&);

	//! Update this octree space partionning
	virtual void updateSpacePartitioning();

	//! Clear the space partionning
	virtual void clear();

	//! Set this octree depth
	/*! If space partitionning is already done, update it*/
	void setDepth(int);

	//! Create octree box representation in the given collection with the specified material
	void createBox(GLC_Material*, GLC_3DViewCollection* pCol= NULL);

	//! Set the default octree depth
	static void setDefaultDepth(int depth);

//@}

//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////
private:

	//! Create octree box representation in the given 3d view collection
	/*! Create box of the given Octree node with the given material*/
	void createBoxWithMaterial(GLC_3DViewCollection*, GLC_OctreeNode*, GLC_Material*);

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Octree root node
	GLC_OctreeNode* m_pRootNode;

	//! Octree depth
	int m_OctreeDepth;

	//! The default octree Depth
	static int m_DefaultOctreeDepth;
};

#endif /* GLC_OCTREE_H_ */
