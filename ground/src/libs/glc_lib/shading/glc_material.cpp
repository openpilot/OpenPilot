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

//! \file glc_material.cpp implementation of the GLC_Material class.

#include "glc_material.h"
#include "../geometry/glc_vbogeom.h"

#include <QtDebug>

//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////

GLC_Material::GLC_Material()
:GLC_Object("Material")
, m_AmbientColor()
, m_DiffuseColor()
, m_SpecularColor()
, m_LightEmission()
, m_fShininess(50.0)		// By default shininess 50
, m_WhereUsed()
, m_pTexture(NULL)			// no texture
, m_Transparency(1.0)
{
	//qDebug() << "GLC_Material::GLC_Material" << id();
	// Diffuse Color
	initDiffuseColor();

	// Others
	initOtherColor();
}

GLC_Material::GLC_Material(const QColor &diffuseColor)
:GLC_Object("Material")
, m_AmbientColor()
, m_DiffuseColor(diffuseColor)
, m_SpecularColor()
, m_LightEmission()
, m_fShininess(50.0)		// By default shininess 50
, m_WhereUsed()
, m_pTexture(NULL)			// no texture
, m_Transparency(1.0)
{
	// Others
	initOtherColor();
}


GLC_Material::GLC_Material(const QString& name ,const GLfloat *pDiffuseColor)
:GLC_Object(name)
, m_AmbientColor()
, m_DiffuseColor()
, m_SpecularColor()
, m_LightEmission()
, m_fShininess(50.0)		// By default shininess 50
, m_WhereUsed()
, m_pTexture(NULL)			// no texture
, m_Transparency(1.0)
{
	//qDebug() << "GLC_Material::GLC_Material" << id();
	// Init Diffuse Color
	if (pDiffuseColor != 0)
	{
		m_DiffuseColor.setRgbF(static_cast<qreal>(pDiffuseColor[0]),
								static_cast<qreal>(pDiffuseColor[1]),
								static_cast<qreal>(pDiffuseColor[2]),
								static_cast<qreal>(pDiffuseColor[3]));
	}
	else
	{
		initDiffuseColor();
	}
	// Others
	initOtherColor();
}
GLC_Material::GLC_Material(GLC_Texture* pTexture, const char *pName)
:GLC_Object(pName)
, m_AmbientColor()
, m_DiffuseColor()
, m_SpecularColor()
, m_LightEmission()
, m_fShininess(50.0)		// By default shininess 50
, m_WhereUsed()
, m_pTexture(pTexture)			// init texture
, m_Transparency(1.0)
{
	Q_ASSERT(NULL != m_pTexture);
	//qDebug() << "GLC_Material::GLC_Material" << id();

	// Diffuse Color
	initDiffuseColor();

	// Others
	initOtherColor();

	//if (m_pTexture->hasAlphaChannel()) m_Transparency= 0.99;
}

// Copy constructor
GLC_Material::GLC_Material(const GLC_Material &InitMaterial)
:GLC_Object(InitMaterial.name())
, m_AmbientColor(InitMaterial.m_AmbientColor)
, m_DiffuseColor(InitMaterial.m_DiffuseColor)
, m_SpecularColor(InitMaterial.m_SpecularColor)
, m_LightEmission(InitMaterial.m_LightEmission)
, m_fShininess(InitMaterial.m_fShininess)
, m_WhereUsed()
, m_pTexture(NULL)
, m_Transparency(InitMaterial.m_Transparency)
{
	//qDebug() << "GLC_Material::GLC_Material" << id();
	if (NULL != InitMaterial.m_pTexture)
	{
		m_pTexture= new GLC_Texture(*(InitMaterial.m_pTexture));
		Q_ASSERT(m_pTexture != NULL);
	}

}

// Destructor
GLC_Material::~GLC_Material(void)
{
	//qDebug() << "GLC_Material::~GLC_Material" << id();
    // clear whereUSED Hash table
    m_WhereUsed.clear();

    if (NULL != m_pTexture)
    {
   		delete m_pTexture;
    	m_pTexture= NULL;
    }


}


//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Get Ambiant color
QColor GLC_Material::ambientColor() const
{
	return m_AmbientColor;
}

// Get diffuse color
QColor GLC_Material::diffuseColor() const
{
	return m_DiffuseColor;
}

// Get specular color
QColor GLC_Material::specularColor() const
{
	return m_SpecularColor;
}

// Get the emissive color
QColor GLC_Material::lightEmission() const
{
	return m_LightEmission;
}
// Get the texture File Name
QString GLC_Material::textureFileName() const
{
	if (m_pTexture != NULL)
	{
		return m_pTexture->fileName();
	}
	else
	{
		return "";
	}
}

// Get Texture Id
GLuint GLC_Material::textureID() const
{
	if (m_pTexture != NULL)
	{
		return m_pTexture->GL_ID();
	}
	else
	{
		return 0;
	}

}

// return true if the texture is loaded
bool GLC_Material::textureIsLoaded() const
{
	if (m_pTexture != NULL)
	{
		return m_pTexture->isLoaded();
	}
	else
	{
		return false;
	}
}

// Return true if material are the same
bool GLC_Material::operator==(const GLC_Material& mat) const
{
	bool result;
	if (this == &mat)
	{
		result= true;
	}
	else
	{
		result= m_AmbientColor == mat.m_AmbientColor;
		result= result and (m_DiffuseColor == mat.m_DiffuseColor);
		result= result and (m_SpecularColor == mat.m_SpecularColor);
		result= result and (m_LightEmission == mat.m_LightEmission);
		result= result and (m_fShininess == mat.m_fShininess);
		if ((NULL != m_pTexture) and (NULL != mat.m_pTexture))
		{
			result= result and ((*m_pTexture) == (*mat.m_pTexture));
		}
		else
		{
			result= result and (m_pTexture == mat.m_pTexture);
		}
		result= result and (m_Transparency == mat.m_Transparency);
	}
	return result;
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
// Set Material properties
 void GLC_Material::setMaterial(const GLC_Material* pMat)
 {
	if (NULL != pMat->m_pTexture)
	{
		GLC_Texture* pTexture= new GLC_Texture(*(pMat->m_pTexture));
		setTexture(pTexture);
	}
	else if (NULL != m_pTexture)
	{
		qDebug() << "Delete texture";
		delete m_pTexture;
		m_pTexture= NULL;
	}
	// Ambient Color
	m_AmbientColor= pMat->m_AmbientColor;
	// Diffuse Color
	m_DiffuseColor= pMat->m_DiffuseColor;
	// Specular Color
	m_SpecularColor= pMat->m_SpecularColor;
	// Lighting emit
	m_LightEmission= pMat->m_LightEmission;
 	// Shininess
 	m_fShininess= pMat->m_fShininess;
 	// Transparency
 	m_Transparency= pMat->m_Transparency;
	// Update geometry which use this material
	CWhereUsed::const_iterator iGeom= m_WhereUsed.constBegin();
	while (iGeom != m_WhereUsed.constEnd())
	{
		iGeom.value()->updateTransparentMaterialNumber();
		++iGeom;
	}

 }

// Set Ambiant Color
void GLC_Material::setAmbientColor(const QColor& ambientColor)
{
	m_AmbientColor= ambientColor;
	m_AmbientColor.setAlphaF(m_Transparency);
}

// Set Diffuse color
void GLC_Material::setDiffuseColor(const QColor& diffuseColor)
{
	m_DiffuseColor= diffuseColor;
	m_DiffuseColor.setAlphaF(m_Transparency);
}

// Set Specular color
void GLC_Material::setSpecularColor(const QColor& specularColor)
{
	m_SpecularColor= specularColor;
	m_SpecularColor.setAlphaF(m_Transparency);
}

// Set Emissive
void GLC_Material::setLightEmission(const QColor& lightEmission)
{
	m_LightEmission= lightEmission;
	m_LightEmission.setAlphaF(m_Transparency);
}

// Set Texture
void GLC_Material::setTexture(GLC_Texture* pTexture)
{
	Q_ASSERT(NULL != pTexture);
	//qDebug() << "GLC_Material::SetTexture";
	if (m_pTexture != NULL)
	{
		delete m_pTexture;
		m_pTexture= pTexture;
		glLoadTexture();
	}
	else
	{
		// It is not sure that there is OpenGL context
		m_pTexture= pTexture;
	}

	//if (m_pTexture->hasAlphaChannel()) m_Transparency= 0.99;
}

// remove Material Texture
void GLC_Material::removeTexture()
{
	if (m_pTexture != NULL)
	{
		delete m_pTexture;
		m_pTexture= NULL;
	}
}

// Add Geometry to where used hash table
bool GLC_Material::addGLC_Geom(GLC_VboGeom* pGeom)
{
	CWhereUsed::iterator iGeom= m_WhereUsed.find(pGeom->id());

	if (iGeom == m_WhereUsed.end())
	{	// Ok, ID doesn't exist
		// Add Geometry to where used hash table
		m_WhereUsed.insert(pGeom->id(), pGeom);
		return true;
	}
	else
	{	// KO, ID exist
		qDebug("GLC_Material::addGLC_Geom : Geometry not added");
		return false;
	}
}

// Remove a geometry from the collection
bool GLC_Material::delGLC_Geom(GLC_uint Key)
{
	CWhereUsed::iterator iGeom= m_WhereUsed.find(Key);

	if (iGeom != m_WhereUsed.end())
	{	// Ok, ID exist
		m_WhereUsed.remove(Key);	// Remove container

		return true;
	}
	else
	{	// KO doesn't exist
		qDebug("GLC_Material::delGLC_Geom : Geometry not remove");
		return false;
	}

}

// Set the material transparency
void GLC_Material::setTransparency(const qreal alpha)
{
	m_Transparency= alpha;
	m_AmbientColor.setAlphaF(m_Transparency);
	m_DiffuseColor.setAlphaF(m_Transparency);
	m_SpecularColor.setAlphaF(m_Transparency);
	m_LightEmission.setAlphaF(m_Transparency);
	// Update geometry which use this material
	CWhereUsed::const_iterator iGeom= m_WhereUsed.constBegin();
	while (iGeom != m_WhereUsed.constEnd())
	{
		iGeom.value()->updateTransparentMaterialNumber();
		++iGeom;
	}
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Load the texture
void GLC_Material::glLoadTexture(void)
{
	if (m_pTexture != NULL)
	{
		m_pTexture->glLoadTexture();
	}
}

// Execute OpenGL Material
void GLC_Material::glExecute()
{
	GLfloat pAmbientColor[4]= {ambientColor().redF(),
								ambientColor().greenF(),
								ambientColor().blueF(),
								ambientColor().alphaF()};

	GLfloat pDiffuseColor[4]= {diffuseColor().redF(),
								diffuseColor().greenF(),
								diffuseColor().blueF(),
								diffuseColor().alphaF()};

	GLfloat pSpecularColor[4]= {specularColor().redF(),
								specularColor().greenF(),
								specularColor().blueF(),
								specularColor().alphaF()};

	GLfloat pLightEmission[4]= {lightEmission().redF(),
								lightEmission().greenF(),
								lightEmission().blueF(),
								lightEmission().alphaF()};

	if (m_pTexture != NULL)
	{
		// for blend managing
		//glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		m_pTexture->glcBindTexture();
	}
	glColor4fv(pDiffuseColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pAmbientColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pDiffuseColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pSpecularColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, pLightEmission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &m_fShininess);

	// OpenGL Error handler
	GLenum errCode;
	if ((errCode= glGetError()) != GL_NO_ERROR)
	{
		const GLubyte* errString;
		errString = gluErrorString(errCode);
		qDebug("GLC_Material::GlExecute OpenGL Error %s", errString);
	}

}
//////////////////////////////////////////////////////////////////////
// Private servicies Functions
//////////////////////////////////////////////////////////////////////

// Init Ambiant Color
void GLC_Material::initDiffuseColor(void)
{
	m_DiffuseColor.setRgbF(1.0, 1.0, 1.0, 1.0);
}

// Init default color
void GLC_Material::initOtherColor(void)
{
	//Ambiant Color
	m_AmbientColor.setRgbF(0.8, 0.8, 0.8, 1.0);

	// Specular Color
	m_SpecularColor.setRgbF(0.5, 0.5, 0.5, 1.0);

	// Lighting emit
	m_LightEmission.setRgbF(0.0, 0.0, 0.0, 1.0);
}
