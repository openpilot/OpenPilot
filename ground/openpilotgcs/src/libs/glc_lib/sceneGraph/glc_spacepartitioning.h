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
//! \file glc_spacepartitioning.h interface for the GLC_SpacePartitioning class.

#ifndef GLC_SPACEPARTITIONING_H_
#define GLC_SPACEPARTITIONING_H_

#include "../glc_config.h"
#include "../glc_boundingbox.h"
#include "../viewport/glc_frustum.h"

class GLC_3DViewCollection;
class GLC_3DViewInstance;

//////////////////////////////////////////////////////////////////////
//! \class GLC_SpacePartitioning
/*! \brief GLC_SpacePartitioning : Abstract class for space partitionning */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_SpacePartitioning
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_SpacePartitioning(GLC_3DViewCollection*);

	//! Copy constructor
	GLC_SpacePartitioning(const GLC_SpacePartitioning&);

	//! Destructor
	virtual ~GLC_SpacePartitioning();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the 3DViewCollection of the space partitioning
	inline GLC_3DViewCollection* collectionHandle()
	{return m_pCollection;}

	//! Return the list off instances inside or intersect the given bounding box
	virtual QList<GLC_3DViewInstance*> listOfIntersectedInstances(const GLC_BoundingBox&)= 0;


//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Update visible GLC_3DViewInstance
	virtual void updateViewableInstances(const GLC_Frustum&)= 0;

	//! Update the space partionning
	virtual void updateSpacePartitioning()= 0;

	//! Clear the space partionning
	virtual void clear()= 0;

//@}

//////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////
protected:
	//! The Collection containing 3dview Instances
	GLC_3DViewCollection* m_pCollection;

};

#endif /* GLC_SPACEPARTITIONING_H_ */
