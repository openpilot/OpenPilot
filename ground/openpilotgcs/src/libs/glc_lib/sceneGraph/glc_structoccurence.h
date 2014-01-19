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

//! \file glc_structoccurence.h interface for the GLC_StructOccurence class.

#ifndef GLC_STRUCTOCCURENCE_H_
#define GLC_STRUCTOCCURENCE_H_

#include "../maths/glc_matrix4x4.h"
#include "../glc_boundingbox.h"
#include "glc_structinstance.h"
#include <QSet>

#include "../glc_config.h"

class GLC_WorldHandle;
class GLC_Material;
class GLC_RenderProperties;

//////////////////////////////////////////////////////////////////////
//! \class GLC_StructOccurence
/*! \brief GLC_StructOccurence : A scene graph occurence node */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_StructOccurence
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

public:
	//! Default constructor
	GLC_StructOccurence();

	//! Create Occurence of the specified instance
	GLC_StructOccurence(GLC_StructInstance* pStructInstance, GLC_WorldHandle* pWorldHandle= NULL, GLuint shaderId=0);

	//! Create Occurence of the specified instance and Uid
	GLC_StructOccurence(GLC_StructInstance* pStructInstance, GLC_uint id, GLC_WorldHandle* pWorldHandle= NULL, GLuint shaderId=0);

	//! Construct Occurence with the specified GLC_3DRep
	GLC_StructOccurence(GLC_3DRep* pRep);

	//! Construct Occurence from the given GLC_3DRep and Uid
	GLC_StructOccurence(GLC_3DRep* pRep, GLC_uint id);

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
	//! Return this Occurence id
	inline GLC_uint id() const
	{return m_Uid;}

	//! Return the instance name of this occurence
	inline const QString name() const
	{return m_pStructInstance->name();}

	//! Return the absolute matrix of this occurence
	inline GLC_Matrix4x4 absoluteMatrix() const
	{ return m_AbsoluteMatrix;}

	//! Return the surcharged relative matrix
	GLC_Matrix4x4 occurrenceRelativeMatrix() const;

	//! Return true if this occurence is orphan
	inline bool isOrphan() const
	{ return NULL == m_pParent;}

	//! Return true if this occurence has parent
	inline bool hasParent() const
	{ return NULL != m_pParent;}

	//! Return true if this occurence has a representation
	bool hasRepresentation() const;

	//! Return true if this occurence has 3DViewInstance
	bool has3DViewInstance() const;

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

	//! Return true if the given occurence can be added to this occurence children
	bool canBeAddedToChildren(GLC_StructOccurence* pOccurence) const;

	//! Return The parent of this occurence
	inline GLC_StructOccurence* parent() const
	{return m_pParent;}

	//! Return a child of this occurence
	/*! The index must exist*/
	inline GLC_StructOccurence* child(const int index) const
	{return m_Childs.at(index);}

	//! Return the list of children of this occurence
	inline QList<GLC_StructOccurence*> children() const
	{return m_Childs;}

	//! Return the list of all accurence under this occurence
	QList<GLC_StructOccurence*> subOccurenceList() const;

	//! Return the number of faces of the representation of this occurence
	unsigned int numberOfFaces() const;

	//! Return the number of vertex of the representation of this occurence
	unsigned int numberOfVertex() const;

	//! Return the number of materials of the representation of this occurence
	unsigned int numberOfMaterials() const;

	//! Return the materials List of the representation of this occurence
	QSet<GLC_Material*> materialSet() const;

	//! Return a clone this occurence
	GLC_StructOccurence* clone(GLC_WorldHandle*, bool shareInstance) const;

	//! Return true if this occurence is visible
	bool isVisible() const;

	//! Return the occurence Bounding Box
	GLC_BoundingBox boundingBox() const;

	//! Return the occurence number of this occurence
	inline unsigned int occurenceNumber() const
	{return m_OccurenceNumber;}

	//! Return an handle of the renderProperties of this occurence
	GLC_RenderProperties* renderPropertiesHandle() const
	{return m_pRenderProperties;}

	//! Return the number of node of this branch
	unsigned int nodeCount() const;

	//! Return the world handle of this occurence
	inline GLC_WorldHandle* worldHandle() const
	{return m_pWorldHandle;}

	//! Return the Set of children references of this occurence
	QSet<GLC_StructReference*> childrenReferences() const;

	//! Return the set of parents references of the given occurence
	static QSet<GLC_StructReference*> parentsReferences(const GLC_StructOccurence* pOccurence);

	//! Return true if the automatic creation of 3DViewInstance is used
	inline bool useAutomatic3DViewInstanceCreation() const
	{return m_AutomaticCreationOf3DViewInstance;}

	//! Return true if this occurence is flexible
	inline bool isFlexible() const
	{return (m_pRelativeMatrix != NULL);}

	//! Return the index position of the given occurrence
	/*! Return -1 if the given occurence is not a child of this occurence*/
	int indexOf(const GLC_StructOccurence* pOcc) const;

	//! Return true if this occurence contains the given occurence has child
	bool containsChild(const GLC_StructOccurence* pOcc) const;
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

	//! insert Child at the given position
	/*! The new child must be orphan and index >= childcount*/
	void insertChild(int index, GLC_StructOccurence* pChild);

	//! Add Child instance and returns the newly created occurence
	GLC_StructOccurence* addChild(GLC_StructInstance*);

	//! Insert Child instance and returns the newly created occurence
	GLC_StructOccurence* insertChild(int index, GLC_StructInstance* pInstance);

	//! make the occurence orphan
	void makeOrphan();

	//! Remove the specified child
	/*! The removed child will not be deleted*/
	bool removeChild(GLC_StructOccurence* pChild);

	//! Reverse Normals of this Occurence and childs
	void reverseNormals();

	//! Create the 3DViewInstance of this occurence if there is a valid 3DRep
	bool create3DViewInstance(GLuint shaderId= 0);

	//! Remove the 3DViewInstance of this occurence
	bool remove3DViewInstance();

	//! Set this occurence world Handle
	void setWorldHandle(GLC_WorldHandle*);

	//! Load the representation and return true if success
	bool loadRepresentation();

	//! UnLoad the representation and return true if success
	bool unloadRepresentation();

	//! Set the occurence number of this occurence
	inline void setOccurenceNumber(unsigned int n)
	{m_OccurenceNumber= n;}

	//! Update the occurence number of this occurence branch
	unsigned int updateOccurenceNumber(unsigned int n);

    //! Set this occurence and children visibility
	void setVisibility(bool visibility);

	//! set the renderProperties of this occurence
	void setRenderProperties(const GLC_RenderProperties& renderProperties);

	//! Remove empty children
	void removeEmptyChildren();

	//! Set the given reference to this occurence
	void setReference(GLC_StructReference* pRef);

	//! Set the automatic creation of 3DViewInstance usage
	inline void setAutomatic3DViewInstanceCreationUsage(bool usage)
	{m_AutomaticCreationOf3DViewInstance= usage;}

	//! Make this occurence a flexible occurence
	void makeFlexible(const GLC_Matrix4x4& relativeMatrix);

	//! Make this occurence rigid
	void makeRigid();

	//! Exchange the occurrence at index position i with the occurrence at index position j
	/*!This function assumes that both i and j are at least 0 but less than childCount().*/
	void swap(int i, int j);
//@}

//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////
private:
	//! Detach the occurence from the GLC_World
	void detach();

	//! Create occurrence from instance and given shader id
	void doCreateOccurrenceFromInstance(GLuint shaderId);

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

	//! The occurence number
	unsigned int m_OccurenceNumber;

	//! Flag to know if a occurence without instance is visible
	bool m_IsVisible;

	//! The occurence rendering properties
	GLC_RenderProperties* m_pRenderProperties;

	//! Automatique création of 3DViewInstance
	bool m_AutomaticCreationOf3DViewInstance;

	//! The relative matrix of this occurence if this occurence is flexible
	GLC_Matrix4x4* m_pRelativeMatrix;

};

#endif /* GLC_STRUCTOCCURENCE_H_ */
