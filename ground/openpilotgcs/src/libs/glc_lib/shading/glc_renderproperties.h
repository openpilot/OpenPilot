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

//! \file glc_renderproperties.h interface for the GLC_RenderProperties class.

#ifndef GLC_RENDERPROPERTIES_H_
#define GLC_RENDERPROPERTIES_H_

#include "glc_material.h"
#include "../glc_global.h"

#include <QSet>
#include <QHash>
#include <QtOpenGL>

#include "../glc_config.h"

//! Define render mode enum in glc namespace
namespace glc
{
	//! Geometry rendering mode enumeration
	enum RenderMode
	{
		NormalRenderMode,
		OverwriteMaterial,
		OverwriteTransparency,
		OverwriteTransparencyAndMaterial,
		PrimitiveSelected,
		OverwritePrimitiveMaterial,
		BodySelection,
		PrimitiveSelection
	};

	//! Geometry rendring flag enumaration
	enum RenderFlag
	{
		ShadingFlag= 800,
		WireRenderFlag,
		TransparentRenderFlag
	};
};
//////////////////////////////////////////////////////////////////////
//! \class GLC_RenderProperties
/*! \brief GLC_RenderProperties encapsulate the render properties
 * off all GLC_3DViewInstance class*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_RenderProperties
{

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_RenderProperties();

	//! Copy constructor
	GLC_RenderProperties(const GLC_RenderProperties&);

	//! Assignement operator
	GLC_RenderProperties &operator=(const GLC_RenderProperties&);

	//! Destructor
	virtual ~GLC_RenderProperties();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return true if it is selected
	inline bool isSelected() const
	{return m_IsSelected;}

	//! Return the rendering mode
	inline glc::RenderMode renderingMode() const
	{return m_RenderMode;}

	//! Return the saved rendering mode
	inline glc::RenderMode savedRenderingMode() const
	{return m_SavedRenderMode;}

	//! Return an handle to the overwrite material
	inline GLC_Material* overwriteMaterial() const
	{return m_pOverwriteMaterial;}

	//! Return the overwrite transparency
	inline float overwriteTransparency() const
	{return m_OverwriteOpacity;}

	//! Return an handle to the set of selected primitives id of the current body
	inline QSet<GLC_uint>* setOfSelectedPrimitivesId() const
	{
		Q_ASSERT(NULL != m_pBodySelectedPrimitvesId);
		if (m_pBodySelectedPrimitvesId->contains(m_CurrentBody))
			return m_pBodySelectedPrimitvesId->value(m_CurrentBody);
		else return NULL;
	}

	//! Return true if the set of selected primitive id is empty
	inline bool setOfSelectedPrimitiveIdIsEmpty() const
	{return (!((NULL != m_pBodySelectedPrimitvesId) && m_pBodySelectedPrimitvesId->contains(m_CurrentBody)));}

	//! Return true if the specified primitive id of the specified body index is selected
	bool primitiveIsSelected(int index, GLC_uint id) const;

	//! Return an handle to the overwrite primitive material Hash
	inline QHash<GLC_uint, GLC_Material*>* hashOfOverwritePrimitiveMaterials() const
	{
		Q_ASSERT(NULL != m_pOverwritePrimitiveMaterialMaps);
		if (m_pOverwritePrimitiveMaterialMaps->contains(m_CurrentBody))
			return m_pOverwritePrimitiveMaterialMaps->value(m_CurrentBody);
		else return NULL;
	}

	//! Return true if the hash of overwrite primitive material is empty
	inline bool hashOfOverwritePrimitiveMaterialsIsEmpty() const
	{return (!((NULL != m_pOverwritePrimitiveMaterialMaps) && m_pOverwritePrimitiveMaterialMaps->contains(m_CurrentBody)));}

	//! Get the PolyFace mode
	/*! PolyFace Mode can Be : GL_FRONT_AND_BACK, GL_FRONT, or GL_BACK*/
	inline GLenum polyFaceMode() const
	{return m_PolyFace;}

	//! Get the Polygon mode
	/*! Polygon Mode can Be : GL_POINT, GL_LINE, or GL_FILL*/
	inline GLenum polygonMode() const
	{return m_PolyMode;}

	//! Return rendering flag render flag
	inline glc::RenderFlag renderingFlag() const
	{return m_RenderingFlag;}

	//! Return true if rendering properties needs to render with transparency
	bool needToRenderWithTransparency() const;

	//! Return the current body index
	inline int currentBodyIndex() const
	{return m_CurrentBody;}

	//! Return true if this rendering properties has defaut value
	bool isDefault() const;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Clear the content of the render properties and update materials usage
	void clear();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Select the instance
	inline void select(bool primitive);

	//! Unselect the instance
	inline void unselect(void);

	//! Set the rendering mode
	inline void setRenderingMode(glc::RenderMode mode)
	{m_RenderMode= mode;}

	//! Set the overwrite material
	void setOverwriteMaterial(GLC_Material*);

	//! Set the overwrite transparency
	inline void setOverwriteTransparency(float alpha)
	{m_OverwriteOpacity= alpha;}

	//! Add the set of selected primitives id of the specified body
	void addSetOfSelectedPrimitivesId(const QSet<GLC_uint>&, int body= 0);

	//! Add a selected primitive of the specified body
	void addSelectedPrimitive(GLC_uint, int body= 0);

	//! Clear selectedPrimitive Set
	void clearSelectedPrimitives();

	//! Add an overwrite primitive material
	void addOverwritePrimitiveMaterial(GLC_uint, GLC_Material*, int bodyIndex= 0);

	//! Clear overwrite primitive materials
	void clearOverwritePrimitiveMaterials();

	//! Polygon's display style
	/*! Face Polygon Mode can be : GL_FRONT_AND_BACK, GL_FRONT, or GL_BACK
	 *  mode can be : GL_POINT, GL_LINE, or GL_FILL */
	inline void setPolygonMode(GLenum Face, GLenum Mode)
	{
		m_PolyFace= Face;
		m_PolyMode= Mode;
	}

	//! Set the rendering flag
	inline void setRenderingFlag(glc::RenderFlag flag)
	{m_RenderingFlag= flag;}

	//! Set the current body index
	inline void setCurrentBodyIndex(int index)
	{m_CurrentBody= index;}

	//! Used the specified material
	inline void useMaterial(GLC_Material*);

	//! Unused the specified material
	inline void unUseMaterial(GLC_Material*);


//@}

//////////////////////////////////////////////////////////////////////
//Private attributes
//////////////////////////////////////////////////////////////////////
private:
	//! The render properties uid : GLC_GenUserID (For GLC_Material usage)
	GLC_uint m_Uid;

	//! Flag to know if it is selected
	bool m_IsSelected;

	//! OpenGL polygon rendering mode
	GLenum m_PolyFace;
	GLenum m_PolyMode;

	//! Geometry rendering mode
	glc::RenderMode m_RenderMode;

	//! Geometry saved rendering mode
	glc::RenderMode m_SavedRenderMode;

	//! The overwrite material
	GLC_Material* m_pOverwriteMaterial;

	//! The overwrite opacity
	float m_OverwriteOpacity;

	//! The selected primitive id regrouped by body
	QHash<int, QSet<GLC_uint>* >* m_pBodySelectedPrimitvesId;

	//! The overwrite primitive material mapping
	QHash<int, QHash<GLC_uint, GLC_Material* >* >* m_pOverwritePrimitiveMaterialMaps;

	//! Transparent material render flag
	glc::RenderFlag m_RenderingFlag;

	//! The current rendere body
	int m_CurrentBody;

	//! The Hash table of overwrite primitive maped to the number of usages in this render properties
	QHash<GLC_Material*, int> m_MaterialsUsage;

};

// Select the instance
void GLC_RenderProperties::select(bool primitive)
{
	m_IsSelected= true;
	if (primitive && (m_RenderMode != glc::PrimitiveSelected))
	{
		m_SavedRenderMode= m_RenderMode;
		m_RenderMode= glc::PrimitiveSelected;
	}
}

// Unselect the instance
void GLC_RenderProperties::unselect(void)
{
	m_IsSelected= false;
	if (m_RenderMode == glc::PrimitiveSelected)
	{
		m_RenderMode= m_SavedRenderMode;
	}
}
// Used the specified material
void GLC_RenderProperties::useMaterial(GLC_Material* pMaterial)
{
	if (m_MaterialsUsage.contains(pMaterial))
	{
		QHash<GLC_Material*, int>::iterator iMat= m_MaterialsUsage.find(pMaterial);
		iMat.value()= iMat.value() + 1;
	}
	else
	{
		m_MaterialsUsage.insert(pMaterial, 1);
		pMaterial->addUsage(m_Uid);
	}

}

// Unused the specified material
void GLC_RenderProperties::unUseMaterial(GLC_Material* pMaterial)
{
	Q_ASSERT(m_MaterialsUsage.contains(pMaterial));
	QHash<GLC_Material*, int>::iterator iMat= m_MaterialsUsage.find(pMaterial);
	iMat.value()= iMat.value() - 1;
	if (iMat.value() == 0)
	{
		pMaterial->delUsage(m_Uid);
		if (pMaterial->isUnused()) delete pMaterial;
		m_MaterialsUsage.remove(pMaterial);
	}
}

#endif /* GLC_RENDERPROPERTIES_H_ */
