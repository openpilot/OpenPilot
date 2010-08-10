/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 2.0.0, packaged on July 2010.

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

//! \file glc_light.cpp implementation of the GLC_Light class.

#include "glc_light.h"
#include "../glc_openglexception.h"

#include <QtDebug>

GLint GLC_Light::m_MaxLight= 0;
QSet<GLenum> GLC_Light::m_FreeLightSet;

//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////
GLC_Light::GLC_Light(const QColor& color)
:GLC_Object("Light")
, m_LightID(-1)
, m_LightType(LightPosition)
, m_ListID(0)
, m_ListIsValid(false)
, m_AmbientColor(Qt::black)
, m_DiffuseColor(color)
, m_SpecularColor(Qt::white)
, m_Position()
, m_SpotDirection(0.0, 0.0, -1.0)
, m_SpotExponent(0.0f)
, m_SpotCutoffAngle(180.0f)
, m_ConstantAttenuation(1.0f)
, m_LinearAttenuation(0.0f)
, m_QuadraticAttenuation(0.0f)
, m_TwoSided(false)
{
	if (0 == m_MaxLight) init();
	Q_ASSERT(!m_FreeLightSet.isEmpty());
	m_LightID= *(m_FreeLightSet.constBegin());
	m_FreeLightSet.remove(m_LightID);
}

GLC_Light::GLC_Light(LightType lightType, const QColor& color)
:GLC_Object("Light")
, m_LightID(-1)
, m_LightType(lightType)
, m_ListID(0)
, m_ListIsValid(false)
, m_AmbientColor(Qt::black)
, m_DiffuseColor(color)
, m_SpecularColor(Qt::white)
, m_Position()
, m_SpotDirection(0.0, 0.0, -1.0)
, m_SpotExponent(0.0f)
, m_SpotCutoffAngle(180.0f)
, m_ConstantAttenuation(1.0f)
, m_LinearAttenuation(0.0f)
, m_QuadraticAttenuation(0.0f)
, m_TwoSided(false)
{
	if (0 == m_MaxLight) init();
	Q_ASSERT(!m_FreeLightSet.isEmpty());
	m_LightID= *(m_FreeLightSet.constBegin());
	m_FreeLightSet.remove(m_LightID);
}

GLC_Light::GLC_Light(const GLC_Light& light)
:GLC_Object(light)
, m_LightID(-1)
, m_LightType(light.m_LightType)
, m_ListID(0)
, m_ListIsValid(false)
, m_AmbientColor(light.m_AmbientColor)
, m_DiffuseColor(light.m_DiffuseColor)
, m_SpecularColor(light.m_SpecularColor)
, m_Position(light.m_Position)
, m_SpotDirection(light.m_SpotDirection)
, m_SpotExponent(light.m_SpotExponent)
, m_SpotCutoffAngle(light.m_SpotCutoffAngle)
, m_ConstantAttenuation(light.m_ConstantAttenuation)
, m_LinearAttenuation(light.m_LinearAttenuation)
, m_QuadraticAttenuation(light.m_QuadraticAttenuation)
, m_TwoSided(light.m_TwoSided)
{
	if (0 == m_MaxLight) init();
	Q_ASSERT(!m_FreeLightSet.isEmpty());
	m_LightID= *(m_FreeLightSet.constBegin());
	m_FreeLightSet.remove(m_LightID);
}

GLC_Light::~GLC_Light(void)
{
	deleteList();
	m_FreeLightSet << m_LightID;
}

/////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

int GLC_Light::maxLightCount()
{
	return m_MaxLight;
}

int GLC_Light::builtAbleLightCount()
{
	return m_FreeLightSet.size();
}

/////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
void GLC_Light::init()
{
	m_MaxLight= 8;
	for (int i= 0; i < m_MaxLight; ++i)
	{
		m_FreeLightSet.insert(GL_LIGHT0 + i);
	}
}

void GLC_Light::setPosition(const GLC_Point3d &pos)
{
	m_Position= pos;
	m_ListIsValid = false;
}

void GLC_Light::setPosition(GLfloat x, GLfloat y, GLfloat z)
{
	m_Position.setVect(static_cast<double>(x), static_cast<double>(y), static_cast<double>(z));
	m_ListIsValid = false;
}

void GLC_Light::setAmbientColor(const QColor& color)
{
	m_AmbientColor= color;
	m_ListIsValid = false;
}

void GLC_Light::setDiffuseColor(const QColor& color)
{
	m_DiffuseColor= color;
	m_ListIsValid = false;
}

void GLC_Light::setSpecularColor(const QColor& color)
{
	m_SpecularColor= color;
	m_ListIsValid = false;
}

void GLC_Light::setTwoSided(const bool mode)
{
	if (m_TwoSided != mode)
	{
		m_TwoSided= mode;
		m_ListIsValid = false;
	}
}

void GLC_Light::setConstantAttenuation(GLfloat constantAttenuation)
{
	m_ConstantAttenuation= constantAttenuation;
	m_ListIsValid = false;
}

void GLC_Light::setLinearAttenuation(GLfloat linearAttenuation)
{
	m_LinearAttenuation= linearAttenuation;
	m_ListIsValid = false;
}

void GLC_Light::setQuadraticAttenuation(GLfloat quadraticAttenuation)
{
	m_QuadraticAttenuation= quadraticAttenuation;
	m_ListIsValid = false;
}

void GLC_Light::setSpotDirection(const GLC_Vector3d& direction)
{
	m_SpotDirection= direction;
	m_ListIsValid = false;
}

void GLC_Light::setSpotCutoffAngle(GLfloat cutoffAngle)
{
	m_SpotCutoffAngle= cutoffAngle;
	m_ListIsValid = false;
}

void GLC_Light::setSpotEponent(GLfloat exponent)
{
	m_SpotExponent= exponent;
	m_ListIsValid = false;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_Light::creationList(GLenum Mode)
{
	if(0 == m_ListID)		// OpenGL list not created
	{
		m_ListID= glGenLists(1);

		if (0 == m_ListID)	// OpenGL List Id not get
		{
			glDraw();
			qDebug("GLC_Light::CreationListe Display list not create");
		}
	}
	if (0 != m_ListID)
	{
		// OpenGL list creation and execution
		glNewList(m_ListID, Mode);

		// Light execution
		glDraw();

		glEndList();

		m_ListIsValid= true;
	}

	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_Light::CreationList ", error);
		throw(OpenGlException);
	}
}

void GLC_Light::glExecute(GLenum Mode)
{

	if (!m_ListIsValid)
	{
		// OpenGL list is not valid
		/*
		GLfloat value;
		glGetFloatv(GL_CONSTANT_ATTENUATION, &value);
		qDebug() << "GL_CONSTANT_ATTENUATION " << value;
		glGetFloatv(GL_LINEAR_ATTENUATION, &value);
		qDebug() << "GL_LINEAR_ATTENUATION " << value;
		glGetFloatv(GL_QUADRATIC_ATTENUATION, &value);
		qDebug() << "GL_QUADRATIC_ATTENUATION " << value;
		*/
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
	setArray[0]= static_cast<GLfloat>(m_Position.x());
	setArray[1]= static_cast<GLfloat>(m_Position.y());
	setArray[2]= static_cast<GLfloat>(m_Position.z());

	if (LightDirection == m_LightType)
	{
		setArray[3]= 0.0f;
		glLightfv(m_LightID, GL_POSITION, setArray);	// Direction of the Light
	}
	else
	{
		setArray[3]= 1.0f;
		glLightfv(m_LightID, GL_POSITION, setArray);	// Position of the Light
		glLightf(m_LightID, GL_CONSTANT_ATTENUATION, m_ConstantAttenuation);
		glLightf(m_LightID, GL_LINEAR_ATTENUATION, m_LinearAttenuation);
		glLightf(m_LightID, GL_QUADRATIC_ATTENUATION, m_QuadraticAttenuation);

	}

	// Spot light parameters
	if (LightSpot == m_LightType)
	{
		// Spot Direction
		setArray[0]= static_cast<GLfloat>(m_SpotDirection.x());
		setArray[1]= static_cast<GLfloat>(m_SpotDirection.y());
		setArray[2]= static_cast<GLfloat>(m_SpotDirection.z());
		glLightfv(m_LightID, GL_SPOT_DIRECTION, setArray);
		glLightf(m_LightID, GL_SPOT_EXPONENT, m_SpotExponent);
		glLightf(m_LightID, GL_SPOT_CUTOFF, m_SpotCutoffAngle);
	}


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

void GLC_Light::deleteList(void)
{
	// if the list is valid, the list is deleted
	if (glIsList(m_ListID))
	{
		disable();
		glDeleteLists(m_ListID, 1);
	}
}
