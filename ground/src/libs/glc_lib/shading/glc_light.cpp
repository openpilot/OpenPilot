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

//! \file glc_light.cpp implementation of the GLC_Light class.

#include "glc_light.h"
#include "../glc_openglexception.h"

#include <QtDebug>

//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////
GLC_Light::GLC_Light()
:GLC_Object("Light")
, m_LightID(GL_LIGHT1)	// Default Light ID
, m_ListID(0)			// By default display list ID= 0
, m_ListIsValid(false)	// By default display list is not valid
, m_AmbientColor(Qt::black)		// By default diffuse color is set to black
, m_DiffuseColor(Qt::white)		// By default diffuse color is set to white
, m_SpecularColor(Qt::white)	// By default specular color is set to white
, m_Position()
, m_TwoSided(false)
{
	//! \todo modify class to support multi light
}

GLC_Light::GLC_Light(const QColor& color)
:GLC_Object("Light")
, m_LightID(GL_LIGHT1)	// Default Light ID
, m_ListID(0)			// By default display list ID= 0
, m_ListIsValid(false)	// By default display list is not valid
, m_AmbientColor(Qt::black)		// By default diffuse color is set to black
, m_DiffuseColor(color)			// Diffuse color is set to color
, m_SpecularColor(Qt::white)	// By default specular color is set to white
, m_Position()
, m_TwoSided(false)
{
	//! \todo modify class to support multi light
}

GLC_Light::~GLC_Light(void)
{
	deleteList();
}

/////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set the light position by a 4D point
void GLC_Light::setPosition(const GLC_Point4d &pos)
{
	m_Position= pos;
	m_ListIsValid = false;
}

// Set the light position by a 3 GLfloat
void GLC_Light::setPosition(GLfloat x, GLfloat y, GLfloat z)
{
	m_Position.setVect(static_cast<double>(x), static_cast<double>(y), static_cast<double>(z));
	m_ListIsValid = false;
}

// Set light's ambient color by a QColor
void GLC_Light::setAmbientColor(const QColor& color)
{
	m_AmbientColor= color;
	m_ListIsValid = false;
}

// Set light's diffuse color by a QColor
void GLC_Light::setDiffuseColor(const QColor& color)
{
	m_DiffuseColor= color;
	m_ListIsValid = false;
}

// Set light's specular color by a QColor
void GLC_Light::setSpecularColor(const QColor& color)
{
	m_SpecularColor= color;
	m_ListIsValid = false;
}

//! Set Mode
void setMode(const GLenum);


//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Create light's OpenGL list
void GLC_Light::creationList(GLenum Mode)
{
	if(!m_ListID)		// OpenGL list not created
	{
		m_ListID= glGenLists(1);

		if (!m_ListID)	// OpenGL List Id not get
		{
			glDraw();
			qDebug("GLC_Lumiere::CreationListe Display list not create");
		}
	}
	// OpenGL list creation and execution
	glNewList(m_ListID, Mode);

	// Light execution
	glDraw();

	glEndList();

	m_ListIsValid= true;

	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_Light::CreationList ", error);
		throw(OpenGlException);
	}
}

// Execute OpenGL light
void GLC_Light::glExecute(GLenum Mode)
{
	// Object Name
	glLoadName(id());


	if (!m_ListIsValid)
	{
		// OpenGL list is not valid

		creationList(Mode);
	}
	else
	{
		glCallList(m_ListID);
	}

	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_Light::GlExecute ", error);
		throw(OpenGlException);
	}

}

// OpenGL light set up
void GLC_Light::glDraw(void)
{
	// Set the lighting model
	if (m_TwoSided)
	{
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
	}
	else
	{
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
	}

	// Color
	GLfloat setArray[4]= {static_cast<GLfloat>(m_AmbientColor.redF()),
									static_cast<GLfloat>(m_AmbientColor.greenF()),
									static_cast<GLfloat>(m_AmbientColor.blueF()),
									static_cast<GLfloat>(m_AmbientColor.alphaF())};
	glLightfv(m_LightID, GL_AMBIENT, setArray);		// Setup The Ambient Light

	setArray[0]= static_cast<GLfloat>(m_DiffuseColor.redF());
	setArray[1]= static_cast<GLfloat>(m_DiffuseColor.greenF());
	setArray[2]= static_cast<GLfloat>(m_DiffuseColor.blueF());
	setArray[3]= static_cast<GLfloat>(m_DiffuseColor.alphaF());
	glLightfv(m_LightID, GL_DIFFUSE, setArray);		// Setup The Diffuse Light


	setArray[0]= static_cast<GLfloat>(m_SpecularColor.redF());
	setArray[1]= static_cast<GLfloat>(m_SpecularColor.greenF());
	setArray[2]= static_cast<GLfloat>(m_SpecularColor.blueF());
	setArray[3]= static_cast<GLfloat>(m_SpecularColor.alphaF());
	glLightfv(m_LightID, GL_SPECULAR, setArray);	// Setup The specular Light

	// Position
	setArray[0]= static_cast<GLfloat>(m_Position.X());
	setArray[1]= static_cast<GLfloat>(m_Position.Y());
	setArray[2]= static_cast<GLfloat>(m_Position.Z());
	setArray[3]= static_cast<GLfloat>(m_Position.W());
	glLightfv(m_LightID, GL_POSITION, setArray);	// Position The Light

	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_Light::GlDraw ", error);
		throw(OpenGlException);
	}

}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

// Delete OpenGL Display list
void GLC_Light::deleteList(void)
{
	// if the list is valid, the list is deleted
	if (glIsList(m_ListID))
	{
		glDeleteLists(m_ListID, 1);
	}
}
