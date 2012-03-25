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

//! \file glc_3dviewcollection.h interface for the GLC_3DViewCollection class.

#ifndef GLC_3DVIEWCOLLECTION_H_
#define GLC_3DVIEWCOLLECTION_H_


#include <QHash>
#include "glc_3dviewinstance.h"
#include "../glc_global.h"
#include "../viewport/glc_frustum.h"

#include "../glc_config.h"

class GLC_SpacePartitioning;
class GLC_Material;
class GLC_Shader;
class GLC_Viewport;

//! GLC_3DViewInstance Hash table
typedef QHash< GLC_uint, GLC_3DViewInstance> ViewInstancesHash;

//! GLC_3DViewInstance pointer Hash table
typedef QHash<GLC_uint, GLC_3DViewInstance*> PointerViewInstanceHash;

//! Hash table of GLC_3DViewInstance Hash table which use a shader
typedef QHash<GLC_uint, PointerViewInstanceHash*> HashList;

//! Map Shader id to instances id (instances which use the shader)
typedef QHash<GLC_uint, GLC_uint> ShaderIdToInstancesId;

//////////////////////////////////////////////////////////////////////
//! \class GLC_3DViewCollection
/*! \brief GLC_3DViewCollection : GLC_3DViewInstance flat collection */

/*! An GLC_3DViewCollection contains  :
 * 		- A hash table containing GLC_3DViewInstance Class
 * 		- A hash table use to associate shader with GLC_3DViewInstance
 */
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_3DViewCollection
{

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_3DViewCollection();

	//! Destructor
	/*! Delete all Node in the Hash Table and clear the Hash Table*/
	virtual ~GLC_3DViewCollection();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Return true if the collection is empty
	inline bool isEmpty() const
	{return m_3DViewInstanceHash.size() == 0;}

	//! Return the number of Node in the collection
	inline int size(void) const
	{return m_3DViewInstanceHash.size();}

	//! Return all GLC_3DViewInstance from collection
	QList<GLC_3DViewInstance*> instancesHandle();

	//! Return all visible GLC_3DViewInstance from the collection
	QList<GLC_3DViewInstance*> visibleInstancesHandle();

	//! Return all viewable GLC_3DViewInstance from the collection
	QList<GLC_3DViewInstance*> viewableInstancesHandle();

	//! Return a GLC_3DViewInstance from collection
	/*! If the element is not found in collection a empty node is return*/
	GLC_3DViewInstance* instanceHandle(GLC_uint Key);

	//! Return the entire collection Bounding Box
	/*! If all object is set to true, visible and non visible object are used*/
	GLC_BoundingBox boundingBox(bool allObject= false);

	//! Return the number of Node in the selection Hash
	inline int selectionSize(void) const
	{return m_SelectedInstances.size();}

	//! Get the Hash table of Selected Nodes
	inline PointerViewInstanceHash* selection()
	{return &m_SelectedInstances;}

	//! Return true if the Instance Id is in the collection
	inline bool contains(GLC_uint key) const
	{return m_3DViewInstanceHash.contains(key);}

	//! Return true if the element is selected
	inline bool isSelected(GLC_uint key) const
	{return m_SelectedInstances.contains(key);}

	//! Return the showing state
	inline bool showState() const
	{return m_IsInShowSate;}

	//! Return the number of drawable objects
	int drawableObjectsSize() const;

	//! Return the element shading group
	inline GLC_uint shadingGroup(GLC_uint key) const
	{ return m_ShaderGroup.value(key);}

	//! Return true if the element is in a shading group
	inline bool isInAShadingGroup(GLC_uint key) const
	{ return m_ShaderGroup.contains(key);}

	//! Return instances name from the specified shading group
	QList<QString> instanceNamesFromShadingGroup(GLuint) const;

	//! Return the number of used shading group
	int numberOfUsedShadingGroup() const;

	//! Return true if the space partitioning is used
	inline bool spacePartitioningIsUsed() const
	{return m_UseSpacePartitioning;}

	//! Return an handle to  the space partitioning
	inline GLC_SpacePartitioning* spacePartitioningHandle()
	{return m_pSpacePartitioning;}

	//! Return true if the collection is viewable
	inline bool isViewable() const
	{return m_IsViewable;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Bind the specified shader to the collection
	 /* return true if success false if shader is already bind*/
	bool bindShader(GLC_uint shaderId);

	//! Unbind the specified shader from the collection
	/* return true if success false otherwise*/
	bool unBindShader(GLC_uint shaderId);

	//! Unbind All shader
	bool unBindAllShader();

	//! Add a GLC_3DViewInstance in the collection
	/*! return true if success false otherwise
	 * If shading group is specified, add instance in desire shading group*/
	bool add(const GLC_3DViewInstance& ,GLC_uint shaderID=0);

	//! Change instance shading group
	/* Move the specified instances into
	 * the specified shading group
	 * Return true if success false otherwise*/
	void changeShadingGroup(GLC_uint instanceId, GLC_uint shaderId);

	//! Remove a GLC_Geometry from the collection and delete it
	/*! 	- Find the GLC_Geometry in the collection
	 * 		- Delete the GLC_Geometry
	 * 		- Remove the Geometry container from collection
	 * 		- Delete the associated OpenGL Display list
	 * 		- Remove the Display list container from collection
	 * 		- Invalidate the collection
	 * return true if success false otherwise*/
	bool remove(GLC_uint instanceId);

	//! Remove and delete all GLC_Geometry from the collection
	void clear(void);

	//! Select a Instance
	bool select(GLC_uint instanceId, bool primitive= false);

	//! Select all instances in current show state or in all show state
	void selectAll(bool allShowState= false);

	//! unselect a Instance
	bool unselect(GLC_uint instanceId);

	//! unselect all Instance
	void unselectAll();

	//! Set the polygon mode for all Instance
	void setPolygonModeForAll(GLenum face, GLenum mode);

	//! Set Instance visibility
	void setVisibility(const GLC_uint instanceId, const bool visible);

	//! Show all instances of the collection
	void showAll();

	//! Hide all instances of collection
	void hideAll();

	//! Set the Show or noShow state
	inline void swapShowState()
	{m_IsInShowSate= !m_IsInShowSate;}

	//! Set the LOD usage
	inline void setLodUsage(const bool usage, GLC_Viewport* pView)
	{
		m_UseLod= usage;
		m_pViewport= pView;
	}

	//! Bind the space partitioning
	void bindSpacePartitioning(GLC_SpacePartitioning*);

	//! Unbind the space partitioning
	void unbindSpacePartitioning();

	//! Use the space partitioning
	inline void setSpacePartitionningUsage(bool use)
	{m_UseSpacePartitioning= use;}

	//! Update the instance viewable state
	/*! Update the frustrum culling from the viewport
	 * If the specified matrix pointer is not null*/
	void updateInstanceViewableState(GLC_Matrix4x4* pMatrix= NULL);

	//! Update the instance viewable state with the specified frustum
	void updateInstanceViewableState(const GLC_Frustum&);

	//! Set the attached viewport of this collection
	inline void setAttachedViewport(GLC_Viewport* pViewport)
	{m_pViewport= pViewport;}

	//! Set the collection viewable state
	inline void setViewable(bool viewable)
	{m_IsViewable= viewable;}

	//! Set VBO usage
	void setVboUsage(bool usage);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Display the specified collection group
	/*! The main group is 0
	 * The selection group is 1
	 * User group are identified by user id
	 */
	void render(GLuint, glc::RenderFlag);

	//! Display all shader group
	void renderShaderGroup(glc::RenderFlag);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////

private:
	//! Display collection's member
	void glDraw(GLC_uint groupID, glc::RenderFlag renderFlag);

	//! Draw instances of a PointerViewInstanceHash
	inline void glDrawInstancesOf(PointerViewInstanceHash*, glc::RenderFlag);

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! GLC_3DViewInstance Hash Table
	ViewInstancesHash m_3DViewInstanceHash;

	//! Selected Node Hash Table
	PointerViewInstanceHash m_SelectedInstances;

	//! List of other Node Hash Table
	HashList m_ShadedPointerViewInstanceHash;

	//! Shader groups hash
	ShaderIdToInstancesId m_ShaderGroup;

	//! Normal Node Hash Table
	PointerViewInstanceHash m_MainInstances;

	//! Show State
	bool m_IsInShowSate;

	//! Level of detail usage
	bool m_UseLod;

	//! The viewport associted to the collection for LOD Usage
	GLC_Viewport* m_pViewport;

	//! The space partitioning
	GLC_SpacePartitioning* m_pSpacePartitioning;

	//! The space partition usage
	bool m_UseSpacePartitioning;

	//! Viewable state
	bool m_IsViewable;


};

// Draw instances of a PointerViewInstanceHash
void GLC_3DViewCollection::glDrawInstancesOf(PointerViewInstanceHash* pHash, glc::RenderFlag renderFlag)
{
	bool forceDisplay= false;
	if (GLC_State::isInSelectionMode())
	{
		forceDisplay= true;
	}

	PointerViewInstanceHash::iterator iEntry= pHash->begin();
	// The current instance
	GLC_3DViewInstance* pCurInstance;
	if (forceDisplay)
	{
		while (iEntry != pHash->constEnd())
		{
			pCurInstance= iEntry.value();
			if ((pCurInstance->viewableFlag() != GLC_3DViewInstance::NoViewable) && (pCurInstance->isVisible() == m_IsInShowSate))
			{
				pCurInstance->render(renderFlag, m_UseLod, m_pViewport);
			}
			++iEntry;
		}
	}
	else
	{
		if (!(renderFlag == glc::TransparentRenderFlag))
		{
			while (iEntry != pHash->constEnd())
			{
				pCurInstance= iEntry.value();
				if ((pCurInstance->viewableFlag() != GLC_3DViewInstance::NoViewable) && (pCurInstance->isVisible() == m_IsInShowSate))
				{
					if (!pCurInstance->isTransparent() || pCurInstance->renderPropertiesHandle()->isSelected() || (renderFlag == glc::WireRenderFlag))
					{
						pCurInstance->render(renderFlag, m_UseLod, m_pViewport);
					}
				}

				++iEntry;
			}

		}
		else
		{
			while (iEntry != pHash->constEnd())
			{
				pCurInstance= iEntry.value();
				if ((pCurInstance->viewableFlag() != GLC_3DViewInstance::NoViewable) && (pCurInstance->isVisible() == m_IsInShowSate))
				{
					if (pCurInstance->hasTransparentMaterials())
					{
						pCurInstance->render(renderFlag, m_UseLod, m_pViewport);
					}
				}

				++iEntry;
			}
	   }

	}
}

#endif //GLC_3DVIEWCOLLECTION_H_
