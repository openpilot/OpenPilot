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

//! \file glc_material.cpp implementation of the GLC_Material class.

#include "glc_material.h"
#include "../geometry/glc_geometry.h"
#include "../glc_factory.h"
#include "../glc_openglexception.h"

#include <QtDebug>

// Class chunk id
quint32 GLC_Material::m_ChunkId= 0xA703;

//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////
// Default constructor
GLC_Material::GLC_Material()
:GLC_Object("Material")
, m_AmbientColor()
, m_DiffuseColor()
, m_SpecularColor()
, m_EmissiveColor()
, m_Shininess(50.0)		// By default shininess 50
, m_WhereUsed()
, m_OtherUsage()
, m_pTexture(NULL)			// no texture
, m_Opacity(1.0)
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
, m_EmissiveColor()
, m_Shininess(50.0)		// By default shininess 50
, m_WhereUsed()
, m_OtherUsage()
, m_pTexture(NULL)			// no texture
, m_Opacity(1.0)
{
	// Others
	initOtherColor();
}


GLC_Material::GLC_Material(const QString& name ,const GLfloat *pDiffuseColor)
:GLC_Object(name)
, m_AmbientColor()
, m_DiffuseColor()
, m_SpecularColor()
, m_EmissiveColor()
, m_Shininess(50.0)		// By default shininess 50
, m_WhereUsed()
, m_OtherUsage()
, m_pTexture(NULL)			// no texture
, m_Opacity(1.0)
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
GLC_Material::GLC_Material(GLC_Texture* pTexture, const QString& name)
:GLC_Object(name)
, m_AmbientColor()
, m_DiffuseColor()
, m_SpecularColor()
, m_EmissiveColor()
, m_Shininess(50.0)		// By default shininess 50
, m_WhereUsed()
, m_OtherUsage()
, m_pTexture(pTexture)			// init texture
, m_Opacity(1.0)
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
:GLC_Object(InitMaterial)
, m_AmbientColor(InitMaterial.m_AmbientColor)
, m_DiffuseColor(InitMaterial.m_DiffuseColor)
, m_SpecularColor(InitMaterial.m_SpecularColor)
, m_EmissiveColor(InitMaterial.m_EmissiveColor)
, m_Shininess(InitMaterial.m_Shininess)
, m_WhereUsed()
, m_OtherUsage()
, m_pTexture(NULL)
, m_Opacity(InitMaterial.m_Opacity)
{
	//qDebug() << "GLC_Material::GLC_Material copy constructor" << id();
	if (NULL != InitMaterial.m_pTexture)
	{
		m_pTexture= new GLC_Texture(*(InitMaterial.m_pTexture));
		Q_ASSERT(m_pTexture != NULL);
	}

}

// Destructor
GLC_Material::~GLC_Material(void)
{
   	delete m_pTexture;
}


//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////
// Return the class Chunk ID
quint32 GLC_Material::chunckID()
{
	return m_ChunkId;
}

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
QColor GLC_Material::emissiveColor() const
{
	return m_EmissiveColor;
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

// Return true if materials are equivalent
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
		result= result && (m_DiffuseColor == mat.m_DiffuseColor);
		result= result && (m_SpecularColor == mat.m_SpecularColor);
		result= result && (m_EmissiveColor == mat.m_EmissiveColor);
		result= result && (m_Shininess == mat.m_Shininess);
		if ((NULL != m_pTexture) && (NULL != mat.m_pTexture))
		{
			result= result && ((*m_pTexture) == (*mat.m_pTexture));
		}
		else
		{
			result= result && (m_pTexture == mat.m_pTexture);
		}
		result= result && (m_Opacity == mat.m_Opacity);
	}
	return result;
}

// Return the material hash code
uint GLC_Material::hashCode() const
{
	QString stringKey= QString::number(m_AmbientColor.rgba());
	stringKey+= QString::number(m_DiffuseColor.rgba());
	stringKey+= QString::number(m_SpecularColor.rgba());
	stringKey+= QString::number(m_EmissiveColor.rgba());
	stringKey+= QString::number(m_Shininess);
	stringKey+= QString::number(m_Opacity);
	if (NULL != m_pTexture)
	{
		stringKey+= m_pTexture->fileName();
	}

	return qHash(stringKey);
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
	m_EmissiveColor= pMat->m_EmissiveColor;
 	// Shininess
 	m_Shininess= pMat->m_Shininess;
 	// Transparency
 	m_Opacity= pMat->m_Opacity;
	// Update geometry which use this material
	WhereUsed::const_iterator iGeom= m_WhereUsed.constBegin();
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
	m_AmbientColor.setAlphaF(m_Opacity);
}

// Set Diffuse color
void GLC_Material::setDiffuseColor(const QColor& diffuseColor)
{
	m_DiffuseColor= diffuseColor;
	m_DiffuseColor.setAlphaF(m_Opacity);
}

// Set Specular color
void GLC_Material::setSpecularColor(const QColor& specularColor)
{
	m_SpecularColor= specularColor;
	m_SpecularColor.setAlphaF(m_Opacity);
}

// Set Emissive
void GLC_Material::setEmissiveColor(const QColor& lightEmission)
{
	m_EmissiveColor= lightEmission;
	m_EmissiveColor.setAlphaF(m_Opacity);
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
bool GLC_Material::addGLC_Geom(GLC_Geometry* pGeom)
{
	QMutexLocker mutexLocker(&m_Mutex);
	//qDebug() << "GLC_Material::addGLC_Geom" << pGeom->id();
	WhereUsed::iterator iGeom= m_WhereUsed.find(pGeom->id());

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
	QMutexLocker mutexLocker(&m_Mutex);

	if (m_WhereUsed.contains(Key))
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
// Add the id to the other used Set
bool GLC_Material::addUsage(GLC_uint id)
{
	QMutexLocker mutexLocker(&m_Mutex);
	if (!m_OtherUsage.contains(id))
	{
		m_OtherUsage << id;
		return true;
	}
	else
	{
		qDebug("GLC_Material::addUsage : id not added");
		return false;
	}
}

// Remove the id to the other used Set
bool GLC_Material::delUsage(GLC_uint id)
{
	QMutexLocker mutexLocker(&m_Mutex);
	if (m_OtherUsage.contains(id))
	{
		m_OtherUsage.remove(id);
		return true;
	}
	else
	{
		qDebug() << "GLC_Material::delUsage : id not removed " << m_Uid;
		return false;
	}
}


// Set the material opacity
void GLC_Material::setOpacity(const qreal alpha)
{
	m_Opacity= alpha;
	m_AmbientColor.setAlphaF(m_Opacity);
	m_DiffuseColor.setAlphaF(m_Opacity);
	m_SpecularColor.setAlphaF(m_Opacity);
	m_EmissiveColor.setAlphaF(m_Opacity);
	// Update geometry which use this material
	WhereUsed::const_iterator iGeom= m_WhereUsed.constBegin();
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
void GLC_Material::glLoadTexture(QGLContext* pContext)
{
	if (m_pTexture != NULL)
	{
		m_pTexture->glLoadTexture(pContext);
	}
	else
	{
		qDebug() << "GLC_Material::glLoadTexture : Material without texture !";
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

	GLfloat pLightEmission[4]= {emissiveColor().redF(),
								emissiveColor().greenF(),
								emissiveColor().blueF(),
								emissiveColor().alphaF()};

	const bool textureIsEnable= glIsEnabled(GL_TEXTURE_2D);
	if (m_pTexture != NULL)
	{
		if (!textureIsEnable) glEnable(GL_TEXTURE_2D);
		m_pTexture->glcBindTexture();
		if (GLC_State::glslUsed())
		{
			if (GLC_Shader::hasActiveShader())
			{
				GLC_Shader::currentShaderHandle()->programShaderHandle()->setUniformValue("tex", GLint(0));
				GLC_Shader::currentShaderHandle()->programShaderHandle()->setUniformValue("useTexture", true);
			}
		}

	}
	else
	{

		if (GLC_State::glslUsed() && GLC_Shader::hasActiveShader())
		{
				if (!textureIsEnable) glEnable(GL_TEXTURE_2D);
				GLC_Shader::currentShaderHandle()->programShaderHandle()->setUniformValue("tex", GLint(0));
				GLC_Shader::currentShaderHandle()->programShaderHandle()->setUniformValue("useTexture", false);
		}
		else
		{
			if (textureIsEnable) glDisable(GL_TEXTURE_2D);
		}

	}

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pAmbientColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pDiffuseColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pSpecularColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, pLightEmission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &m_Shininess);

	glColor4fv(pDiffuseColor);


	// OpenGL Error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_Material::glExecute() ", error);
		throw(OpenGlException);
	}

}

// Execute OpenGL Material
void GLC_Material::glExecute(float overwriteTransparency)
{
	GLfloat pAmbientColor[4]= {ambientColor().redF(),
								ambientColor().greenF(),
								ambientColor().blueF(),
								overwriteTransparency};

	GLfloat pDiffuseColor[4]= {diffuseColor().redF(),
								diffuseColor().greenF(),
								diffuseColor().blueF(),
								overwriteTransparency};

	GLfloat pSpecularColor[4]= {specularColor().redF(),
								specularColor().greenF(),
								specularColor().blueF(),
								overwriteTransparency};

	GLfloat pLightEmission[4]= {emissiveColor().redF(),
								emissiveColor().greenF(),
								emissiveColor().blueF(),
								overwriteTransparency};

	const bool textureIsEnable= glIsEnabled(GL_TEXTURE_2D);

	if (m_pTexture != NULL)
	{
		if (!textureIsEnable) glEnable(GL_TEXTURE_2D);
		m_pTexture->glcBindTexture();
		if (GLC_State::glslUsed())
		{
			if (GLC_Shader::hasActiveShader())
			{
				GLC_Shader::currentShaderHandle()->programShaderHandle()->setUniformValue("tex", GLint(0));
				GLC_Shader::currentShaderHandle()->programShaderHandle()->setUniformValue("useTexture", true);
			}
		}
	}
	else
	{
		if (textureIsEnable) glDisable(GL_TEXTURE_2D);
		if (GLC_State::glslUsed())
		{
			if (GLC_Shader::hasActiveShader())
			{
				GLC_Shader::currentShaderHandle()->programShaderHandle()->setUniformValue("tex", GLint(0));
				GLC_Shader::currentShaderHandle()->programShaderHandle()->setUniformValue("useTexture", false);
			}
		}
	}

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pAmbientColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pDiffuseColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pSpecularColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, pLightEmission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &m_Shininess);

	glColor4fv(pDiffuseColor);

	// OpenGL Error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_Material::glExecute(float overwriteTransparency) ", error);
		throw(OpenGlException);
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
	m_EmissiveColor.setRgbF(0.0, 0.0, 0.0, 1.0);
}

// Non Member methods
// Non-member stream operator
QDataStream &operator<<(QDataStream &stream, const GLC_Material &material)
{
	quint32 chunckId= GLC_Material::m_ChunkId;
	stream << chunckId;

	// Store GLC_Object class members
	stream << material.id() << material.name();

	// Store GLC_Material class members
	stream << material.ambientColor() << material.diffuseColor() << material.specularColor();
	stream << material.emissiveColor() << material.shininess() << material.opacity();

	// Test if the material has texture
	bool hasTexture= material.hasTexture();
	stream << hasTexture;
	if (hasTexture)
	{
		GLC_Texture texture(*(material.textureHandle()));
		stream << texture;
	}

	return stream;
}
QDataStream &operator>>(QDataStream &stream, GLC_Material &material)
{
	quint32 chunckId;
	stream >> chunckId;

	Q_ASSERT(chunckId == GLC_Material::m_ChunkId);

	// Retrieve GLC_Object members
	GLC_uint id;
	QString name;
	stream >> id >> name;
	material.setId(id);
	material.setName(name);

	// Retrieve GLC_Material members
	QColor ambient, diffuse, specular, lightEmission;
	float shininess;
	double alpha;
	stream >> ambient >> diffuse >> specular >> lightEmission;
	stream >> shininess >> alpha;
	material.setAmbientColor(ambient);
	material.setDiffuseColor(diffuse);
	material.setSpecularColor(specular);
	material.setEmissiveColor(lightEmission);
	material.setShininess(shininess);
	material.setOpacity(alpha);

	// Test if material has texture
	bool hasTexture;
	stream >> hasTexture;
	if (hasTexture)
	{
		GLC_Texture texture;
		stream >> texture;
		material.setTexture(new GLC_Texture(texture));
	}
	return stream;
}
