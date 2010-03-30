/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 1.2.0, packaged on September 2009.

 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 *****************************************************************************/

#ifndef GLC_3DREP_H_
#define GLC_3DREP_H_

#include "../geometry/glc_vbogeom.h"
#include "glc_rep.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_3DRep
/*! \brief GLC_3DRep : A referrence 3D Representation*/
//////////////////////////////////////////////////////////////////////
class GLC_3DRep : public GLC_Rep
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default Constructor
	GLC_3DRep();

	//! Construct a 3DRep with a geometry
	GLC_3DRep(GLC_VboGeom*);

	//! Copy Constructor
	GLC_3DRep(const GLC_3DRep&);

	//! Assignement operator
	virtual GLC_3DRep &operator=(const GLC_3DRep&);

	//! Clone the representation
	virtual GLC_Rep* clone() const;

	//! Make a deep copy of the 3DRep
	virtual GLC_Rep* deepCopy() const;

	//! Destructor
	virtual ~GLC_3DRep();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the type of representation
	virtual int type() const;

	//! Get Geometry
	inline GLC_VboGeom* geomAt(int index) const
	{
		Q_ASSERT(NULL != m_pGeomList);
		Q_ASSERT(m_pGeomList->size() > index);
		return m_pGeomList->at(index);
	}

	//! Return the number of body
	inline int numberOfBody() const
	{
		Q_ASSERT(NULL != m_pGeomList);
		return m_pGeomList->size();
	}

	//! Return true if the representation is empty
	inline virtual bool isEmpty() const
	{
		Q_ASSERT(NULL != m_pGeomList);
		return m_pGeomList->isEmpty();
	}

	//! Return true if the rep bounding box is valid
	bool boundingBoxIsValid() const;

	//! Return true if the 3DRep contains the geometry
	inline bool contains(GLC_VboGeom* pGeom)
	{return m_pGeomList->contains(pGeom);}

	//! Get number of faces
	unsigned int numberOfFaces() const;

	//! Get number of vertex
	unsigned int numberOfVertex() const;

	//! Get number of materials
	unsigned int numberOfMaterials() const;

	//! Get materials List
	QSet<GLC_Material*> materialSet() const;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Add Geometry to the 3DRep
	inline void addGeom(GLC_VboGeom* pGeom)
	{m_pGeomList->append(pGeom);}

	//! Remove empty geometries
	void removeEmptyGeometry();

	//! Reverse geometries normals
	void reverseNormals();

	//! Load the representation and return true if success
	virtual bool load();

	//! UnLoad the representation and return true if success
	virtual bool unload();

	//! Replace the representation
	virtual void replace(GLC_Rep*);

//@}

//////////////////////////////////////////////////////////////////////
// private services functions
//////////////////////////////////////////////////////////////////////
private:
	//! Clear current representation
	void clear();

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Geometries of the 3D representation
	QList<GLC_VboGeom*>* m_pGeomList;

	//! The Type of representation
	int* m_pType;
};

#endif /* GLC_3DREP_H_ */
