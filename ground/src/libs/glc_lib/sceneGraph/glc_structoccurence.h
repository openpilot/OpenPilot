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

//! \file glc_structoccurence.h interface for the GLC_StructOccurence class.

#ifndef GLC_STRUCTOCCURENCE_H_
#define GLC_STRUCTOCCURENCE_H_

#include "../maths/glc_matrix4x4.h"
#include "../glc_boundingbox.h"
#include "glc_structinstance.h"
#include <QSet>

class GLC_WorldHandle;
class GLC_Material;

//////////////////////////////////////////////////////////////////////
//! \class GLC_StructOccurence
/*! \brief GLC_StructOccurence : A scene graph occurence node */
//////////////////////////////////////////////////////////////////////
class GLC_StructOccurence
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

public:
	//! Default constructor
	GLC_StructOccurence(GLC_WorldHandle*, GLC_StructInstance*, GLuint shaderId=0);

	//! Copy constructor
	GLC_StructOccurence(GLC_WorldHandle*, const GLC_StructOccurence&, bool shareInstance);

	//! Destructor
	virtual ~GLC_StructOccurence();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the Occurence id
	inline GLC_uint id() const
	{return m_Uid;}

	//! Return Occurence instance name
	inline const QString name() const
	{return m_pStructInstance->name();}

	//! Return the absolute matrix
	inline GLC_Matrix4x4 absoluteMatrix() const
	{ return m_AbsoluteMatrix;}

	//! Return true if this occurence is orphan
	inline bool isOrphan() const
	{ return NULL == m_pParent;}

	//! Return true if this occurence has a representation
	inline bool hasRepresentation() const
	{ return m_HasRepresentation;}

	//! Return the instance of this occurence
	inline GLC_StructInstance* structInstance() const
	{ return m_pStructInstance;}

	//! Return the reference of this occurence
	inline GLC_StructReference* structReference() const
	{
		Q_ASSERT(NULL != m_pStructInstance);
		return m_pStructInstance->structReference();
	}

	//! Return the number of childs
	inline int childCount() const
	{ return m_Childs.size();}

	//! Return true if the occurence has child
	inline bool hasChild() const
	{return childCount() > 0;}

	//! Return The parent
	inline GLC_StructOccurence* parent() const
	{return m_pParent;}

	//! Return a child
	/*! The index must exist*/
	inline GLC_StructOccurence* child(const int index) const
	{return m_Childs.at(index);}

	//! Return the list of children
	inline QList<GLC_StructOccurence*> children() const
	{ return m_Childs;}

	//! Get number of faces
	unsigned int numberOfFaces() const;

	//! Get number of vertex
	unsigned int numberOfVertex() const;

	//! Get number of materials
	unsigned int numberOfMaterials() const;

	//! Get materials List
	QSet<GLC_Material*> materialSet() const;

	//! Clone the occurence
	GLC_StructOccurence* clone(GLC_WorldHandle*, bool shareInstance) const;

	//! Return true if the occurence is visible
	bool isVisible() const;

	//! Return the occurence Bounding Box
	GLC_BoundingBox boundingBox() const;



//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Set Occurence instance Name
	inline void setName(const QString name) {m_pStructInstance->setName(name);}

	//! Update the absolute matrix
	GLC_StructOccurence* updateAbsoluteMatrix();

	//! Update children obsolute Matrix
	GLC_StructOccurence* updateChildrenAbsoluteMatrix();

	//! Add Child
	/*! The new child must be orphan*/
	void addChild(GLC_StructOccurence*);

	//! Add Child instance (the occurence is created)
	void addChild(GLC_StructInstance*);

	//! make the occurence orphan
	void makeOrphan();

	//! Remove the specified child
	/*! The removed child will not be deleted*/
	bool removeChild(GLC_StructOccurence* pChild);

	//! Reverse Normals of this Occurence and childs
	void reverseNormals();

	//! Check the presence of representation
	void checkForRepresentation();

	//! Set the occurence world Handle
	void setWorldHandle(GLC_WorldHandle*);

	//! Load the representation and return true if success
	bool loadRepresentation();

	//! UnLoad the representation and return true if success
	bool unloadRepresentation();

//@}

//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////
private:
	//! Detach the occurence from the GLC_World
	void detach();

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Occurence Unique ID
	GLC_uint m_Uid;

	//! the occurence's World Handle
	GLC_WorldHandle* m_pWorldHandle;

	//! Number of this Occurence
	int* m_pNumberOfOccurence;

	//! The struct instance of this occurence
	GLC_StructInstance* m_pStructInstance;

	//! The parent of this occurence
	GLC_StructOccurence* m_pParent;

	//! The Child of this occurence
	QList<GLC_StructOccurence*> m_Childs;

	//! The absolute matrix of the occurence
	GLC_Matrix4x4 m_AbsoluteMatrix;

	//! true if occurence has a representation
	bool m_HasRepresentation;

};

#endif /* GLC_STRUCTOCCURENCE_H_ */
