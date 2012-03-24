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

//! \file glc_geometry.cpp Implementation of the GLC_Geometry class.

#include "../shading/glc_selectionmaterial.h"
#include "../glc_openglexception.h"
#include "../glc_state.h"
#include "../glc_context.h"
#include "glc_geometry.h"

//////////////////////////////////////////////////////////////////////
// Constructor destructor
//////////////////////////////////////////////////////////////////////
// Default constructor
GLC_Geometry::GLC_Geometry(const QString& name, const bool typeIsWire)
: m_GeometryIsValid(false)	// By default geometry is invalid
, m_pBoundingBox(NULL)
, m_MaterialHash()
, m_UseColorPerVertex(false)
, m_IsSelected(false)
, m_WireData()
, m_WireColor(Qt::black)
, m_LineWidth(1.0f)
, m_IsWire(typeIsWire)		// the geometry type
, m_TransparentMaterialNumber(0)
, m_Id(glc::GLC_GenGeomID())
, m_Name(name)
, m_UseVbo(false)
{

}
// Copy constructor
GLC_Geometry::GLC_Geometry(const GLC_Geometry& sourceGeom)
: m_GeometryIsValid(false)	// By default geometry is invalid
, m_pBoundingBox(NULL)
, m_MaterialHash(sourceGeom.m_MaterialHash)
, m_UseColorPerVertex(sourceGeom.m_UseColorPerVertex)
, m_IsSelected(false)
, m_WireData(sourceGeom.m_WireData)
, m_WireColor(sourceGeom.m_WireColor)
, m_LineWidth(sourceGeom.m_LineWidth)
, m_IsWire(sourceGeom.m_IsWire)
, m_TransparentMaterialNumber(sourceGeom.m_TransparentMaterialNumber)
, m_Id(glc::GLC_GenGeomID())
, m_Name(sourceGeom.m_Name)
, m_UseVbo(sourceGeom.m_UseVbo)
{
	// Add this mesh to inner material
	MaterialHash::const_iterator i= sourceGeom.m_MaterialHash.constBegin();
    while (i != sourceGeom.m_MaterialHash.constEnd())
    {
        // update inner material use table
        i.value()->addGLC_Geom(this);
        ++i;
    }

	if (NULL != sourceGeom.m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox(*sourceGeom.m_pBoundingBox);
	}
}

// Overload "=" operator
GLC_Geometry& GLC_Geometry::operator=(const GLC_Geometry& sourceGeom)
{
	if (this != &sourceGeom)
	{
		clear();
		m_GeometryIsValid= false;
		m_pBoundingBox= NULL;
		m_MaterialHash= sourceGeom.m_MaterialHash;
		m_UseColorPerVertex= sourceGeom.m_UseColorPerVertex;
		m_IsSelected= false;
		m_WireData= sourceGeom.m_WireData;
		m_WireColor= sourceGeom.m_WireColor;
		m_LineWidth= sourceGeom.m_LineWidth;
		m_IsWire= sourceGeom.m_IsWire;
		m_TransparentMaterialNumber= sourceGeom.m_TransparentMaterialNumber;
		m_Id= glc::GLC_GenGeomID();
		m_Name= sourceGeom.m_Name;
		m_UseVbo= sourceGeom.m_UseVbo;
	}
	return *this;
}

GLC_Geometry::~GLC_Geometry()
{
	// delete mesh inner material
	{
		MaterialHash::const_iterator i= m_MaterialHash.begin();
	    while (i != m_MaterialHash.constEnd())
	    {
	        // delete the material if necessary
	        i.value()->delGLC_Geom(id());
	        if (i.value()->isUnused()) delete i.value();
	        ++i;
	    }
	}
	m_MaterialHash.clear();

	delete m_pBoundingBox;

}

/////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Get number of faces
unsigned int GLC_Geometry::faceCount(int) const
{
	return 0;
}

// Get number of vertex
unsigned int GLC_Geometry::VertexCount() const
{
	return 0;
}

double GLC_Geometry::volume()
{
	return 0.0;
}

/////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
// Clear the content of the geometry and makes it empty
void GLC_Geometry::clear()
{
	clearGeometry();
}

// Replace the Master material
void GLC_Geometry::replaceMasterMaterial(GLC_Material* pMaterial)
{
	Q_ASSERT(!m_IsWire);
	if (!m_MaterialHash.isEmpty())
	{
		if (pMaterial != firstMaterial())
		{
			// Remove the first material
			MaterialHash::iterator iMaterial= m_MaterialHash.begin();
			removeMaterial(iMaterial.value()->id());

			// Add the new material
			addMaterial(pMaterial);
		}
	}
	else
	{
		addMaterial(pMaterial);
	}
}
//! Update the transparent material number
void GLC_Geometry::updateTransparentMaterialNumber()
{
	m_TransparentMaterialNumber= 0;
	MaterialHash::const_iterator iMat= m_MaterialHash.constBegin();
	while (iMat != m_MaterialHash.constEnd())
	{
		if (iMat.value()->isTransparent())
		{
			++m_TransparentMaterialNumber;
		}
		++iMat;
	}
	if (m_WireColor.alpha() != 255)
	{
		++m_TransparentMaterialNumber;
	}
}

// Add material to mesh
void GLC_Geometry::addMaterial(GLC_Material* pMaterial)
{
	if (pMaterial != NULL)
	{
		const GLC_uint materialID= pMaterial->id();
		MaterialHash::const_iterator iMaterial= m_MaterialHash.find(materialID);
		// Check if there is a material at specified index
		Q_ASSERT(iMaterial == m_MaterialHash.end());

		// Add this geometry in the material use table
		pMaterial->addGLC_Geom(this);
		// Add the Material to Material hash table
		m_MaterialHash.insert(materialID, pMaterial);

		// Test if the material is transparent
		if (pMaterial->isTransparent())
		{
			//qDebug() << "Add transparent material";
			++m_TransparentMaterialNumber;
		}
	}
}

void GLC_Geometry::setWireColor(const QColor& color)
{
	bool previousColorIsTransparent= (m_WireColor.alpha() != 255);
	bool newColorIsTransparent= (color.alpha() != 255);

	if (previousColorIsTransparent != newColorIsTransparent)
	{
		if (newColorIsTransparent) ++m_TransparentMaterialNumber;
		else if (previousColorIsTransparent) --m_TransparentMaterialNumber;
	}

	m_WireColor= color;
}

void GLC_Geometry::copyVboToClientSide()
{
	m_WireData.copyVboToClientSide();
}

void GLC_Geometry::releaseVboClientSide(bool update)
{
	m_WireData.releaseVboClientSide(update);
}

void GLC_Geometry::setVboUsage(bool usage)
{
	m_UseVbo= usage;
	if (!usage || (usage && GLC_State::vboSupported()))
	{
		m_WireData.setVboUsage(m_UseVbo);
	}
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// if the geometry have a texture, load it
void GLC_Geometry::glLoadTexture(void)
{
	MaterialHash::iterator iMaterial= m_MaterialHash.begin();

    while (iMaterial != m_MaterialHash.constEnd())
    {
        // Load texture of mesh materials
        iMaterial.value()->glLoadTexture();
        ++iMaterial;
    }
}

// Geometry display
void GLC_Geometry::render(const GLC_RenderProperties& renderProperties)
{
	Q_ASSERT(!m_IsWire || (m_IsWire && m_MaterialHash.isEmpty()));
	bool renderWire= (renderProperties.renderingFlag() == glc::TransparentRenderFlag) && isTransparent();
	renderWire= renderWire || ((renderProperties.renderingFlag() != glc::TransparentRenderFlag) && !isTransparent());
	if (!m_IsWire || renderWire)
	{
		if (m_MaterialHash.isEmpty() && !m_IsWire)
		{
			GLC_Material* pMaterial= new GLC_Material();
			pMaterial->setName(name());
			addMaterial(pMaterial);
		}

		m_IsSelected= renderProperties.isSelected();

		// Define Geometry's property
		if(!GLC_State::isInSelectionMode())
		{
			glPropGeom(renderProperties);
		}

		glDraw(renderProperties);

		m_IsSelected= false;
		m_GeometryIsValid= true;

		// OpenGL error handler
		GLenum error= glGetError();
		if (error != GL_NO_ERROR)
		{
			GLC_OpenGlException OpenGlException("GLC_Geometry::render " + name(), error);
			throw(OpenGlException);
		}
	}
}

// Virtual interface for OpenGL Geometry properties.
void GLC_Geometry::glPropGeom(const GLC_RenderProperties& renderProperties)
{
	glLineWidth(lineWidth());

	if(m_IsWire)
	{
		glLineWidth(m_LineWidth);
		GLC_Context::current()->glcEnableLighting(false);;
		if (!renderProperties.isSelected())
		{
			// Set polyline colors
			GLfloat color[4]= {static_cast<float>(m_WireColor.redF()),
									static_cast<float>(m_WireColor.greenF()),
									static_cast<float>(m_WireColor.blueF()),
									static_cast<float>(m_WireColor.alphaF())};

			glColor4fv(color);
		}
		else
		{
			GLC_SelectionMaterial::glExecute();
		}
	}
	else if (m_MaterialHash.size() == 1)
	{
		GLC_Material* pCurrentMaterial= m_MaterialHash.begin().value();
		if (pCurrentMaterial->hasTexture())
		{
			GLC_Context::current()->glcEnableLighting(true);
			pCurrentMaterial->glExecute();
			if (renderProperties.isSelected()) GLC_SelectionMaterial::glExecute();
		}
		else
		{
			GLC_Context::current()->glcEnableLighting(true);
			if (renderProperties.isSelected()) GLC_SelectionMaterial::glExecute();
			else pCurrentMaterial->glExecute();
		}
	}
}

// Remove the specified material from the geometry
void GLC_Geometry::removeMaterial(GLC_uint id)
{
	Q_ASSERT(containsMaterial(id));
	// Remove the first material
	GLC_Material* pMaterial= m_MaterialHash.value(id);
    // delete the material if necessary
	pMaterial->delGLC_Geom(this->id());
	if (pMaterial->isTransparent())
	{
		--m_TransparentMaterialNumber;
	}
    if (pMaterial->isUnused()) delete pMaterial;
	m_MaterialHash.remove(id);

}

// Clear the content of this object and makes it empty
void  GLC_Geometry::clearGeometry()
{
	m_GeometryIsValid= false;

	delete m_pBoundingBox;
	m_pBoundingBox= NULL;

	// delete mesh inner material
	{
		MaterialHash::const_iterator i= m_MaterialHash.begin();
	    while (i != m_MaterialHash.constEnd())
	    {
	        // delete the material if necessary
	        i.value()->delGLC_Geom(id());
	        if (i.value()->isUnused()) delete i.value();
	        ++i;
	    }
	}
	m_MaterialHash.clear();

	m_UseColorPerVertex= false;
	m_IsSelected= false;
	m_WireData.clear();
	m_IsWire= false;
	m_TransparentMaterialNumber= 0;
	m_Name.clear();

}
