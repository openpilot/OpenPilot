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

//! \file glc_vbogeom.h Interface for the GLC_VboGeom class.

#ifndef GLC_VBOGEOM_H_
#define GLC_VBOGEOM_H_
#include <QColor>
#include "../glc_object.h"
#include "../shading/glc_material.h"
#include "../glc_boundingbox.h"
#include "../glc_ext.h"

typedef QHash<GLC_uint, GLC_Material*> MaterialHash;
typedef QHash<GLC_uint, GLC_uint> MaterialHashMap;

//////////////////////////////////////////////////////////////////////
//! \class GLC_VboGeom
/*! \brief GLC_VboGeom : parent class for all GLC class which contain
 *  vbo geometrical data*/

/*! GLC_VboGeom is a abstract class. \n \n
 *  Main attributes of GLC_VboGeom:
 *		- Material : 	GLC_Material
 * 		- Graphic properties
 * 		- Transformation Matrix
 *
 * GLC_Geometry provide :
 * 		- Function to create VBO : GLC_VboGeom::createVbo
 * 		- Function to draw Geometry : GLC_VboGeom::glExecute
 * 		- Virtual function to overload for visual property: GLC_VboGeom::glPropGeom
 * 		- Virtual function to overload for Object topology: GLC_VboGeom::glDraw
 *
 */
//////////////////////////////////////////////////////////////////////

class GLC_VboGeom
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	/*!
	 * QString Name
	 * const bool typeIsWire
	 */
	GLC_VboGeom(const QString &, const bool);
	//! Copy constructor
	/*!
	 * const GLC_VboGeom geometry to copy
	 */
	GLC_VboGeom(const GLC_VboGeom&);
	//! Destructor
	virtual ~GLC_VboGeom();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Get Object ID
	inline GLC_uint id() const {return m_Uid;}

	//! Get Object Name
	inline const QString name() const {return m_Name;}

	//! Return true if the geometry is valid
	inline bool isValid(void) const
	{return m_GeometryIsValid;}

	//! Return first material of geometry
	inline GLC_Material* firstMaterial(void) const
	{
		if (not m_MaterialHash.isEmpty())
		{
			return m_MaterialHash.begin().value();
		}
		else return NULL;
	}

	//! Return the number of materials
	inline unsigned int numberOfMaterials() const
	{return m_MaterialHash.size();}

	//! Return the specified mesh sub material
	inline GLC_Material* material(const GLC_uint key)
	{return m_MaterialHash[key];}

	//! Get materials Set
	inline QSet<GLC_Material*> materialSet() const
	{return m_MaterialHash.values().toSet();}

	//! Get materials ID List
	inline QList<GLC_uint> materialIds() const
	{return m_MaterialHash.keys();}

	//! Return true if Material key is in the mesh
	inline const bool containsMaterial(const GLC_uint key) const
	{return m_MaterialHash.contains(key);}

	//! Return material index if Material is the same than a material already in the mesh
	/*! Return 0 if the material is not found
	 */
	GLC_uint materialIndex(const GLC_Material& mat) const;

	//! return the geometry bounding box
	virtual GLC_BoundingBox& boundingBox(void) = 0;

	//! Return true if the bounding box is valid
	inline bool boundingBoxIsValid() const {return NULL != m_pBoundingBox;}

	//! clone the geometry
	virtual GLC_VboGeom* clone() const = 0;

	//! Get the geometry transparency
	inline bool isTransparent() const
	{return (m_TransparentMaterialNumber == m_MaterialHash.size()) and hasTransparentMaterials();}

	//! Return true if the geometry contains transparent materials
	inline bool hasTransparentMaterials() const
	{return m_TransparentMaterialNumber > 0;}

	//! return true if color per vertex is used
	inline bool usedColorPerVertex() const
	{return m_UseColorPerVertex;}

	//! Return true if the geometry type is wireframe
	inline bool typeIsWire() const
	{return m_IsWire;}

	//! Get number of faces
	virtual unsigned int numberOfFaces() const;

	//! Get number of vertex
	virtual unsigned int numberOfVertex() const;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Replace the Master material
	//! The number of materials must be <= 1
	void replaceMasterMaterial(GLC_Material*);

	//! Add material to the geometry
	void addMaterial(GLC_Material *);

	inline void colorPerVertex(const bool colorPerVertex)
	{
		if (m_UseColorPerVertex != colorPerVertex)
		{
			m_UseColorPerVertex= colorPerVertex;
			m_GeometryIsValid= false;
		}
	}

	//! Update the transparent material number
	void updateTransparentMaterialNumber();

	//! Reverse normal
	virtual void reverseNormals() {}

	//! Set the lod Index
	/*! The value must be between 0 and 100*/
	virtual void setCurrentLod(const int) {}

	//! Set Object Id
	inline void setId(const GLC_uint id)
	{m_Uid= id;}

	//! Set geometry Name
	inline void setName(const QString name) {m_Name= name;}

//@}
//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! if the geometry have a texture, load it
	virtual void glLoadTexture(void);

	//! Virtual interface for OpenGL execution from GLC_Object.
	/*! This Virtual function is implemented here.\n
	 * At the first call, this function call virtual function
	 * GLC_VboGeom::glPropGeom and set up :
	 * 		- Geometry
	 * 		- VBO
	 * AfterWard this function
	 *		- Call virtual function GLC_VboGeom::glPropGeom
	 *        virtual function GLC_Geometry::glDraw
	 */
	virtual void glExecute(bool, bool transparent= false);


protected:
	//! Virtual interface for OpenGL Geometry set up.
	/*! This Virtual function have to be implemented in concrete class.*/
	virtual void glDraw(bool transparent= false) = 0;

	//! Virtual interface for OpenGL Geometry properties.
	/*! This Virtual function can be modify in concrete class.
	 * bool IsSelected
	 * bool ForceWire
	 */
	virtual void glPropGeom(bool);

//@}
//////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////
protected:

	//! Geometry validity
	bool m_GeometryIsValid;

	//! Bounding box
	GLC_BoundingBox* m_pBoundingBox;

	//! Material Hash table
	MaterialHash m_MaterialHash;

	//! Material Hash map used by the copy constructor
	MaterialHashMap m_MaterialHashMap;

	//! Color per vertex usage
	bool m_UseColorPerVertex;

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:

	//! Geometry type is wire
	bool m_IsWire;

	//! Transparency
	int m_TransparentMaterialNumber;

	//! The Unique id of an Geometry
	/*! Generated on creation*/
	GLC_uint m_Uid;

	//! Name of geometry
	QString m_Name;

};

#endif /*GLC_VBOGEOM_H_*/
