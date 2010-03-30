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

//! \file glc_3dviewcollection.h interface for the GLC_3DViewCollection class.

#ifndef GLC_3DVIEWCOLLECTION_H_
#define GLC_3DVIEWCOLLECTION_H_


#include <QHash>
#include "glc_3dviewinstance.h"
#include "../glc_enum.h"

class GLC_Material;
class GLC_Shader;
class GLC_Viewport;

//! Geometry hash table
typedef QHash< GLC_uint, GLC_3DViewInstance> CNodeMap;
typedef QHash< GLC_uint, GLC_3DViewInstance*> PointerNodeHash;

//! Hash of geometry hash table which use a shader
typedef QHash<GLuint, PointerNodeHash*> HashList;

//! Hash of shader group id
typedef QHash<GLC_uint, GLuint> ShaderGroup;

//////////////////////////////////////////////////////////////////////
//! \class GLC_3DViewCollection
/*! \brief GLC_3DViewCollection : GLC_3DViewInstance flat collection */

/*! An GLC_3DViewCollection contain  :
 * 		- A hash table containing GLC_3DViewInstance Class
 */
//////////////////////////////////////////////////////////////////////

class GLC_3DViewCollection
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
	{return m_NodeMap.size() == 0;}

	//! Return the number of Node in the collection
	inline int size(void) const
	{return m_NodeMap.size();}

	//! Return all GLC_3DViewInstance from collection
	QList<GLC_3DViewInstance*> instancesHandle();

	//! Return all visible GLC_3DViewInstance from the collection
	QList<GLC_3DViewInstance*> visibleInstanceHandle();

	//! Return list of invisible instance name
	QList<QString> invisibleInstanceName() const;

	//! Return a GLC_3DViewInstance from collection
	/*! If the element is not found in collection a empty node is return*/
	GLC_3DViewInstance* instanceHandle(GLC_uint Key);

	//! Return the entire collection Bounding Box
	GLC_BoundingBox boundingBox(void);

	//! Return the number of Node in the selection Hash
	inline int selectionSize(void) const
	{return m_SelectedNodes.size();}

	//! Get the Hash table of Selected Nodes
	inline PointerNodeHash* selection()
	{return &m_SelectedNodes;}

	//! Return true if the Instance Id is in the collection
	inline bool contains(GLC_uint key) const
	{return m_NodeMap.contains(key);}

	//! Return true if the element is selected
	inline bool isSelected(GLC_uint key) const
	{return m_SelectedNodes.contains(key);}

	//! Return the showing state
	inline bool showState() const
	{return m_IsInShowSate;}

	//! Return the number of drawable objects
	int drawableObjectsSize() const;

	//! Return the element shading group
	inline GLuint shadingGroup(GLC_uint key) const
	{ return m_ShaderGroup.value(key);}

	//! Return true if the element is in a shading group
	inline bool isInAShadingGroup(GLC_uint key) const
	{ return m_ShaderGroup.contains(key);}

	//! Return instances name from the specified shading group
	QList<QString> instanceNamesFromShadingGroup(GLuint) const;

	//! Return the number of used shading group
	int numberOfUsedShadingGroup() const;


//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Bind the specified shader to the collection
	 /* return true if success false if shader is already bind*/
	bool bindShader(GLuint);

	//! Unbind the specified shader from the collection
	/* return true if success false otherwise*/
	bool unBindShader(GLuint);

	//! Unbind All shader
	bool unBindAllShader();

	//! Add a GLC_3DViewInstance in the collection
	/*! return true if success false otherwise
	 * If shading group is specified, add instance in desire shading group*/
	bool add(GLC_3DViewInstance& ,GLuint shaderID=0);

	//! Change instance shading group
	/* Move the specified instances into
	 * the specified shading group
	 * Return true if success false otherwise*/
	void changeShadingGroup(GLC_uint, GLuint);

	//! Remove a GLC_Geometry from the collection and delete it
	/*! 	- Find the GLC_Geometry in the collection
	 * 		- Delete the GLC_Geometry
	 * 		- Remove the Geometry container from collection
	 * 		- Delete the associated OpenGL Display list
	 * 		- Remove the Display list container from collection
	 * 		- Invalidate the collection
	 * return true if success false otherwise*/
	bool remove(GLC_uint Key);

	//! Remove and delete all GLC_Geometry from the collection
	void clear(void);

	//! Select a Instance
	bool select(GLC_uint);

	//! Select all instances in current show state
	void selectAll();

	//! unselect a Instance
	bool unselect(GLC_uint);

	//! unselect all Instance
	void unselectAll();

	//! Set the polygon mode for all Instance
	void setPolygonModeForAll(GLenum, GLenum);

	//! Set Instance visibility
	void setVisibility(const GLC_uint, const bool);

	//! Show all instances of the collection
	void showAll();

	//! Hide all instances of collection
	void hideAll();

	//! Set the Show or noShow state
	inline void swapShowState()
	{
		m_IsInShowSate= !m_IsInShowSate;
		// Bounding box validity
		if (NULL != m_pBoundingBox)
		{
			delete m_pBoundingBox;
			m_pBoundingBox= NULL;
		}
	}

	//! Set the LOD usage
	inline void setLodUsage(const bool usage, GLC_Viewport* pView)
	{
		m_UseLod= usage;
		m_pViewport= pView;
	}

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
	void glExecute(GLuint, bool);

	//! Display all shader group
	void glExecuteShaderGroup(bool);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////

private:
	//! Display collection's member
	void glDraw(GLuint, bool);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Privates services Functions*/
//@{
//////////////////////////////////////////////////////////////////////

private:
	//! Set the Bounding box validity
	void setBoundingBoxValidity(void);

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! GLC_3DViewInstance Hash Table
	CNodeMap m_NodeMap;

	//! BoundingBox of the collection
	GLC_BoundingBox* m_pBoundingBox;

	//! Selected Node Hash Table
	PointerNodeHash m_SelectedNodes;

	//! List of other Node Hash Table
	HashList m_OtherNodeHashList;

	//! Shader groups hash
	ShaderGroup m_ShaderGroup;

	//! Normal Node Hash Table
	PointerNodeHash m_MainNodes;

	//! Show State
	bool m_IsInShowSate;

	//! Level of detail usage
	bool m_UseLod;

	//! The viewport associted to the collection for LOD Usage
	GLC_Viewport* m_pViewport;

};
#endif //GLC_3DVIEWCOLLECTION_H_
