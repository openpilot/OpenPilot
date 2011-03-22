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

//! \file glc_world.h interface for the GLC_World class.

#ifndef GLC_WORLD_H_
#define GLC_WORLD_H_

#include "glc_3dviewcollection.h"
#include "glc_structoccurence.h"
#include "glc_structreference.h"
#include "glc_structinstance.h"
#include "glc_worldhandle.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_World
/*! \brief GLC_World : The Root of GLC_Lib Scene Graph*/
/*!
 *  GLC_World contain :
 * 		- The Scene root GLC_Product.
 * 		- a GLC_3DViewCollection which manage all scene shapes (GLC_3DViewInstance)
 *
 */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_World
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_World();

	//! Copy constructor
	GLC_World(const GLC_World&);

	//! Destructor
	virtual ~GLC_World();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the entire world Bounding Box
	inline GLC_BoundingBox boundingBox()
	{ return m_pWorldHandle->collection()->boundingBox();}

	//! Return the root of the world
	inline GLC_StructOccurence* rootOccurence() const
	{return m_pRoot;}

	//! Return the world collection
	inline GLC_3DViewCollection* collection()
	{return m_pWorldHandle->collection();}

	//! Return the size of the world
	inline int size() const
	{return m_pWorldHandle->collection()->size();}

	//! Return true if the world is empty
	inline bool isEmpty() const
	{return  m_pWorldHandle->collection()->isEmpty() && !m_pRoot->hasChild();}

	//! Return number of faces
	inline int numberOfFaces() const
	{return m_pRoot->numberOfFaces();}

	//! Return number of vertex
	inline int numberOfVertex() const
	{return m_pRoot->numberOfVertex();}

	//! Return the number of materials
	inline int numberOfMaterials() const
	{return m_pRoot->numberOfMaterials();}

	//! Return the list of material
	inline QList<GLC_Material*> listOfMaterials() const
	{return m_pRoot->materialSet().toList();}

	//! Return list of world's instances
	inline QList<GLC_3DViewInstance*> instancesHandle() const
	{return m_pWorldHandle->collection()->instancesHandle();}

	//! Return all visible GLC_3DViewInstance from the world
	inline QList<GLC_3DViewInstance*> visibleInstancesHandle() const
	{return m_pWorldHandle->collection()->visibleInstancesHandle();}

	//! Return instances name from the specified shading group
	inline QList<QString> instanceNamesFromShadingGroup(GLuint id) const
	{return m_pWorldHandle->collection()->instanceNamesFromShadingGroup(id);}

	//! Return the number of used shading group
	inline int numberOfUsedShadingGroup() const
	{return m_pWorldHandle->collection()->numberOfUsedShadingGroup();}

	//! Return the worldHandle of this world
	inline GLC_WorldHandle* worldHandle()
	{return m_pWorldHandle;}

	//! Return the occurence specified by an id
	/*! Id must be a valid identifier*/
	inline GLC_StructOccurence* occurence(GLC_uint id) const
	{return m_pWorldHandle->getOccurence(id);}

	//! Return the list off occurences
	inline QList<GLC_StructOccurence*> listOfOccurence() const
	{return m_pWorldHandle->occurences();}

	//! Return the number of occurence
	inline int numberOfOccurence() const
	{return m_pWorldHandle->numberOfOccurence();}

	//! Return true if the world contians specified id
	inline int containsOccurence(GLC_uint id) const
	{return m_pWorldHandle->containsOccurence(id);}

	//! Return the list of instance
	inline QList<GLC_StructInstance*> instances() const
	{return m_pWorldHandle->instances();}

	//! Return the list of Reference
	inline QList<GLC_StructReference*> references() const
	{return m_pWorldHandle->references();}

	//! Return the number of body
	inline int numberOfBody() const
	{return m_pWorldHandle->numberOfBody();}

	//! Return the number of representation
	inline int representationCount() const
	{return m_pWorldHandle->representationCount();}

	//! Return the world Up vector
	inline GLC_Vector3d upVector() const
	{return m_pWorldHandle->upVector();}

	//! Return the number of selected occurence
	int selectionSize() const
	{return m_pWorldHandle->selectionSetHandle()->size();}

	//! Return true if the given occurence is selected
	inline bool isSelected(const GLC_StructOccurence* pOccurence) const
	{return m_pWorldHandle->selectionSetHandle()->contains(pOccurence);}

	//! Return true if the given occurence id is selected
	inline bool isSelected(GLC_uint selectionId) const
	{return m_pWorldHandle->selectionSetHandle()->contains(selectionId);}

	//! Return the list of selected occurences
	inline QList<GLC_StructOccurence*> selectedOccurenceList() const
	{return m_pWorldHandle->selectionSetHandle()->occurencesList();}

	//! Return the list of selected occurences
	inline QList<GLC_StructOccurence*> selectedPrimitiveOccurenceList() const
	{return m_pWorldHandle->selectionSetHandle()->occurencesListWithSelectedPrimitive();}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Merge this world with another world
	void mergeWithAnotherWorld(GLC_World &);

	//! Reverse worlds part normal
	inline void reversePartNormal() {m_pRoot->reverseNormals();}

	//! Clear this world
	GLC_World& clear() {return *this= GLC_World();}

	//! Set the World root Name
	inline void setRootName(const QString& name)
	{m_pRoot->setName(name);}

	//! Set the world Up Vector
	inline void setUpVector(const GLC_Vector3d& vect)
	{m_pWorldHandle->setUpVector(vect);}

	//! Set the attached viewport of this world
	inline void setAttachedViewport(GLC_Viewport* pViewport)
	{m_pWorldHandle->setAttachedViewport(pViewport);}

	//! Select the given occurence
	/*! The given occurence must belong to the world handle of this world*/
	inline void select(const GLC_StructOccurence* pOccurence)
	{m_pWorldHandle->select(pOccurence->id());}

	//! Select the given occurence id
	/*! The given occurence id must belong to the world handle of this world*/
	inline void select(GLC_uint occurenceId)
	{m_pWorldHandle->select(occurenceId);}

	//! Unselect the given occurence id
	/*! The given occurence id must belong to the world handle of this world*/
	inline void unselect(GLC_uint occurenceId)
	{m_pWorldHandle->unselect(occurenceId);}

	//! Select all occurence of this world with a 3DViewInstance
	inline void selectAllWith3DViewInstance()
	{m_pWorldHandle->selectAllWith3DViewInstance(true);}

	//! Select all occurence of this world with a 3DViewInstance in the current show state
	inline void selectAllWith3DViewInstanceInCurrentShowState()
	{m_pWorldHandle->selectAllWith3DViewInstance(false);}

	//! Unselect all occurence of this world
	inline void unselectAll()
	{m_pWorldHandle->unselectAll();}

	//! Show / Hide selected 3DViewInstance
	inline void showHideSelected3DViewInstance()
	{m_pWorldHandle->showHideSelected3DViewInstance();}

	//! Show selected 3DViewInstance
	inline void showSelected3DViewInstance()
	{m_pWorldHandle->setSelected3DViewInstanceVisibility(true);}

	//! Hide selected 3DViewInstance
	inline void hideSelected3DViewInstance()
	{m_pWorldHandle->setSelected3DViewInstanceVisibility(false);}


//@}

//////////////////////////////////////////////////////////////////////
/*! @name Operator Overload */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Assignement operator
	GLC_World& operator=(const GLC_World&);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Display the world
	inline void render(GLuint groupId, glc::RenderFlag renderFlag= glc::ShadingFlag)
	{m_pWorldHandle->collection()->render(groupId, renderFlag);}

	//! Display the world's shader group
	inline void renderShaderGroup(glc::RenderFlag renderFlag= glc::ShadingFlag)
	{m_pWorldHandle->collection()->renderShaderGroup(renderFlag);}

//@}
//////////////////////////////////////////////////////////////////////
// private members
//////////////////////////////////////////////////////////////////////
private:
	//! The World Handle
	GLC_WorldHandle* m_pWorldHandle;

	//! The root of the structure
	GLC_StructOccurence* m_pRoot;
};

#endif /*GLC_WORLD_H_*/
