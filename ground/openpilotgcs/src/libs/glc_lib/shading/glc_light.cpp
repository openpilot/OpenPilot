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

//! \file glc_light.cpp implementation of the GLC_Light class.

#include <QGLContext>
#include <QtDebug>

#include "glc_light.h"
#include "../glc_openglexception.h"
#include "../glc_context.h"

GLint GLC_Light::m_MaxLight= 8;
QHash<const QGLContext*, QSet<GLenum> > GLC_Light::m_ContextToFreeLightSet;

//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////
GLC_Light::GLC_Light(const QGLContext* pContext, const QColor& color)
:GLC_Object("Light")
, m_LightID(-1)
, m_LightType(LightPosition)
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
, m_pContext(const_cast<QGLContext*>(pContext))
, m_IsValid(false)
{
	addNewLight();
}

GLC_Light::GLC_Light(LightType lightType, const QGLContext* pContext, const QColor& color)
:GLC_Object("Light")
, m_LightID(-1)
, m_LightType(lightType)
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
, m_pContext(const_cast<QGLContext*>(pContext))
, m_IsValid(false)
{
	addNewLight();
}

GLC_Light::GLC_Light(const GLC_Light& light)
:GLC_Object(light)
, m_LightID(-1)
, m_LightType(light.m_LightType)
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
, m_pContext(light.m_pContext)
, m_IsValid(false)
{
	addNewLight();
}

GLC_Light::~GLC_Light(void)
{
	removeThisLight();
}

/////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

int GLC_Light::maxLightCount()
{
	return m_MaxLight;
}

int GLC_Light::builtAbleLightCount(QGLContext* pContext)
{
	if (m_ContextToFreeLightSet.contains(pContext))
	{
		return m_ContextToFreeLightSet.value(pContext).size();
	}
	else return m_MaxLight;
}

/////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
void GLC_Light::initForThisContext()
{
	for (int i= 0; i < m_MaxLight; ++i)
	{
		m_ContextToFreeLightSet[m_pContext].insert(GL_LIGHT0 + i);
	}
}

void GLC_Light::setPosition(const GLC_Point3d &pos)
{
	m_Position= pos;
}

void GLC_Light::setPosition(GLfloat x, GLfloat y, GLfloat z)
{
	m_Position.setVect(static_cast<double>(x), static_cast<double>(y), static_cast<double>(z));
}

void GLC_Light::setAmbientColor(const QColor& color)
{
	m_AmbientColor= color;
	m_IsValid= false;
}

void GLC_Light::setDiffuseColor(const QColor& color)
{
	m_DiffuseColor= color;
	m_IsValid= false;
}

void GLC_Light::setSpecularColor(const QColor& color)
{
	m_SpecularColor= color;
	m_IsValid= false;
}

void GLC_Light::setTwoSided(const bool mode)
{
	m_TwoSided= mode;
	m_IsValid= false;
}

void GLC_Light::setConstantAttenuation(GLfloat constantAttenuation)
{
	m_ConstantAttenuation= constantAttenuation;
	m_IsValid= false;
}

void GLC_Light::setLinearAttenuation(GLfloat linearAttenuation)
{
	m_LinearAttenuation= linearAttenuation;
	m_IsValid= false;
}

void GLC_Light::setQuadraticAttenuation(GLfloat quadraticAttenuation)
{
	m_QuadraticAttenuation= quadraticAttenuation;
	m_IsValid= false;
}

void GLC_Light::setSpotDirection(const GLC_Vector3d& direction)
{
	m_SpotDirection= direction;
	m_IsValid= false;
}

void GLC_Light::setSpotCutoffAngle(GLfloat cutoffAngle)
{
	m_SpotCutoffAngle= cutoffAngle;
	m_IsValid= false;
}

void GLC_Light::setSpotEponent(GLfloat exponent)
{
	m_SpotExponent= exponent;
	m_IsValid= false;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////


void GLC_Light::disable()
{
	if (NULL != m_pContext)
	{
		glDisable(m_LightID);
	}
}


void GLC_Light::glExecute()
{
	if (NULL == m_pContext)
	{
		m_pContext= const_cast<QGLContext*>(QGLContext::currentContext());
		Q_ASSERT(NULL != m_pContext);
		addNewLight();
	}

	GLC_Context::current()->glcEnableLighting(true);
	glEnable(m_LightID);

	if (m_pContext != QGLContext::currentContext())
	{
		Q_ASSERT(QGLContext::areSharing(m_pContext, QGLContext::currentContext()));
		m_IsValid= false;
	}
	Q_ASSERT(m_pContext->isValid());

	GLfloat setArray[4];

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
	}


	if (!m_IsValid)
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
		setArray[0]= static_cast<GLfloat>(m_AmbientColor.redF());
		setArray[1]= static_cast<GLfloat>(m_AmbientColor.greenF());
		setArray[2]= static_cast<GLfloat>(m_AmbientColor.blueF());
		setArray[3]= static_cast<GLfloat>(m_AmbientColor.alphaF());
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

		if (LightDirection != m_LightType)
			glLightf(m_LightID, GL_CONSTANT_ATTENUATION, m_ConstantAttenuation);
			glLightf(m_LightID, GL_LINEAR_ATTENUATION, m_LinearAttenuation);
			glLightf(m_LightID, GL_QUADRATIC_ATTENUATION, m_QuadraticAttenuation);

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

		m_IsValid= true;
	}

	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		qDebug() << "GLC_Light::glExecute Exception, id= " << m_LightID;
		GLC_OpenGlException OpenGlException("GLC_Light::glExecute ", error);
		throw(OpenGlException);
	}

}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Private services fonction
//////////////////////////////////////////////////////////////////////
void GLC_Light::addNewLight()
{
	if (NULL != m_pContext)
	{
		if (!m_ContextToFreeLightSet.contains(m_pContext))
		{
			m_ContextToFreeLightSet.insert(m_pContext, QSet<GLenum>());
			initForThisContext();
		}

		// Some OpenGL driver support only Light0 ???
		if (m_ContextToFreeLightSet.value(m_pContext).size() == m_MaxLight)
		{
			m_LightID= GL_LIGHT0;
		}
		else
		{
			m_LightID= *(m_ContextToFreeLightSet[m_pContext].constBegin());
		}

		m_ContextToFreeLightSet[m_pContext].remove(m_LightID);
	}
}

void GLC_Light::removeThisLight()
{
	if (NULL != m_pContext)
	{
		Q_ASSERT(m_ContextToFreeLightSet.contains(m_pContext));
		Q_ASSERT(!m_ContextToFreeLightSet[m_pContext].contains(m_LightID));
		m_ContextToFreeLightSet[m_pContext].insert(m_LightID);
		if (m_ContextToFreeLightSet[m_pContext].size() == m_MaxLight)
		{
			m_ContextToFreeLightSet.remove(m_pContext);
		}
	}
}
