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
//! \file glc_3drep.h interface for the GLC_3DRep class.

#ifndef GLC_3DREP_H_
#define GLC_3DREP_H_

#include "glc_geometry.h"
#include "glc_rep.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_3DRep
/*! \brief GLC_3DRep : A referrence 3D Representation*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_3DRep : public GLC_Rep
{
	friend GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_3DRep &);
	friend GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_3DRep &);

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default Constructor
	GLC_3DRep();

	//! Construct a 3DRep with a geometry
	GLC_3DRep(GLC_Geometry*);

	//! Copy Constructor
	GLC_3DRep(const GLC_3DRep&);

	//! Assignement operator
	virtual GLC_3DRep &operator=(const GLC_Rep&);

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
	//! Return the class Chunk ID
	static quint32 chunckID();

	//! Return the type of representation
	virtual int type() const;

	//! Get Geometry
	inline GLC_Geometry* geomAt(int index) const
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

	//! Return the 3DRep bounding Box
	GLC_BoundingBox boundingBox() const;

	//! Return true if the 3DRep contains the geometry
	inline bool contains(GLC_Geometry* pGeom)
	{return m_pGeomList->contains(pGeom);}

	//! Return the number of faces of this 3DRep
	unsigned int faceCount() const;

	//! Return number of vertex of this 3DRep
	unsigned int vertexCount() const;

	//! Return number of materials of this 3DRep
	unsigned int materialCount() const;

	//! Return materials Set of this 3DRep
	QSet<GLC_Material*> materialSet() const;

	//! Return the volume of this 3DRep
	double volume() const;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Add Geometry to the 3DRep
	inline void addGeom(GLC_Geometry* pGeom)
	{
		m_pGeomList->append(pGeom);
		*m_pIsLoaded= true;
	}

	//! Remove empty geometries and factorise materials
	void clean();

	//! Reverse geometries normals
	void reverseNormals();

	//! Load the representation and return true if success
	virtual bool load();

	//! UnLoad the representation and return true if success
	virtual bool unload();

	//! Replace the representation
	virtual void replace(GLC_Rep*);

	//! Replace the specified material by a new one
	void replaceMaterial(GLC_uint, GLC_Material*);

	//! Merge this 3Drep with another 3DRep
	void merge(const GLC_3DRep*);

	//! Take the geometry of another 3DRep
	void take(GLC_3DRep* pSource);

	//! Copy VBO to the Client Side
	void copyVboToClientSide();

	//! Release client VBO
	void releaseVboClientSide(bool update= false);

	//! Transform 3DRep sub mesh vertice with the given matrix
	void transformSubGeometries(const GLC_Matrix4x4& matrix);

	//! Set VBO usage
	void setVboUsage(bool usage);

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
	QList<GLC_Geometry*>* m_pGeomList;

	//! The Type of representation
	int* m_pType;

	//! Class chunk id
	static quint32 m_ChunkId;

};

//! Non-member stream operator
GLC_LIB_EXPORT QDataStream &operator<<(QDataStream &, const GLC_3DRep &);
GLC_LIB_EXPORT QDataStream &operator>>(QDataStream &, GLC_3DRep &);

#endif /* GLC_3DREP_H_ */
