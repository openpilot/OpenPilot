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
//! \file glc_context.cpp implementation of the GLC_Context class.

#include "glc_context.h"
#include "glc_contextmanager.h"
#include "shading/glc_shader.h"

#include "glc_state.h"

GLC_Context* GLC_Context::m_pCurrentContext= NULL;

GLC_Context::GLC_Context(const QGLFormat& format)
: QGLContext(format)
, m_CurrentMatrixMode()
, m_MatrixStackHash()
, m_ContextSharedData()
, m_UniformShaderData()
, m_LightingIsEnable()
{
	qDebug() << "GLC_Context::GLC_Context";
	GLC_ContextManager::instance()->addContext(this);
	init();
}

GLC_Context::~GLC_Context()
{
	qDebug() << "GLC_Context::~GLC_Context()";
	GLC_ContextManager::instance()->remove(this);
	QHash<GLenum, QStack<GLC_Matrix4x4>* >::iterator iStack= m_MatrixStackHash.begin();
	while (iStack != m_MatrixStackHash.end())
	{
		delete iStack.value();
		++iStack;
	}
}


//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

GLC_Context* GLC_Context::current()
{
	return m_pCurrentContext;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////
void GLC_Context::glcMatrixMode(GLenum mode)
{
	Q_ASSERT(QGLContext::isValid());
	Q_ASSERT((mode == GL_MODELVIEW) || (mode == GL_PROJECTION));

	m_CurrentMatrixMode= mode;
#ifdef GLC_OPENGL_ES_2

#else
	glMatrixMode(m_CurrentMatrixMode);
#endif

}

void GLC_Context::glcLoadIdentity()
{
	Q_ASSERT(QGLContext::isValid());
	m_MatrixStackHash.value(m_CurrentMatrixMode)->top().setToIdentity();

#ifdef GLC_OPENGL_ES_2
	m_UniformShaderData.setModelViewProjectionMatrix(m_MatrixStackHash.value(GL_MODELVIEW)->top(), m_MatrixStackHash.value(GL_PROJECTION)->top());
#else
	if (GLC_Shader::hasActiveShader())
	{
		m_UniformShaderData.setModelViewProjectionMatrix(m_MatrixStackHash.value(GL_MODELVIEW)->top(), m_MatrixStackHash.value(GL_PROJECTION)->top());
	}
	glLoadIdentity();
#endif

}

void GLC_Context::glcPushMatrix()
{
	Q_ASSERT(QGLContext::isValid());
	m_MatrixStackHash.value(m_CurrentMatrixMode)->push(m_MatrixStackHash.value(m_CurrentMatrixMode)->top());

#ifndef GLC_OPENGL_ES_2
	glPushMatrix();
#endif

}

void GLC_Context::glcPopMatrix()
{
	Q_ASSERT(QGLContext::isValid());
	m_MatrixStackHash.value(m_CurrentMatrixMode)->pop();

#ifdef GLC_OPENGL_ES_2
	this->glcLoadMatrix(m_MatrixStackHash.value(m_CurrentMatrixMode)->top());
#else
	if (GLC_Shader::hasActiveShader())
	{
		this->glcLoadMatrix(m_MatrixStackHash.value(m_CurrentMatrixMode)->top());
	}
	glPopMatrix();
#endif

}


void GLC_Context::glcLoadMatrix(const GLC_Matrix4x4& matrix)
{
	m_MatrixStackHash.value(m_CurrentMatrixMode)->top()= matrix;

#ifdef GLC_OPENGL_ES_2
	m_UniformShaderData.setModelViewProjectionMatrix(m_MatrixStackHash.value(GL_MODELVIEW)->top(), m_MatrixStackHash.value(GL_PROJECTION)->top());
#else
	if (GLC_Shader::hasActiveShader())
	{
		m_UniformShaderData.setModelViewProjectionMatrix(m_MatrixStackHash.value(GL_MODELVIEW)->top(), m_MatrixStackHash.value(GL_PROJECTION)->top());
	}
	::glLoadMatrixd(matrix.getData());
#endif

}

void GLC_Context::glcMultMatrix(const GLC_Matrix4x4& matrix)
{
	const GLC_Matrix4x4 current= m_MatrixStackHash.value(m_CurrentMatrixMode)->top();
	m_MatrixStackHash.value(m_CurrentMatrixMode)->top()= m_MatrixStackHash.value(m_CurrentMatrixMode)->top() * matrix;
#ifdef GLC_OPENGL_ES_2
	m_UniformShaderData.setModelViewProjectionMatrix(m_MatrixStackHash.value(GL_MODELVIEW)->top(), m_MatrixStackHash.value(GL_PROJECTION)->top());
#else
	if (GLC_Shader::hasActiveShader())
	{
		m_UniformShaderData.setModelViewProjectionMatrix(m_MatrixStackHash.value(GL_MODELVIEW)->top(), m_MatrixStackHash.value(GL_PROJECTION)->top());
	}
	::glMultMatrixd(matrix.getData());
#endif

}

void GLC_Context::glcScaled(double x, double y, double z)
{
	GLC_Matrix4x4 scale;
	scale.setMatScaling(x, y, z);
	glcMultMatrix(scale);
}

void GLC_Context::glcOrtho(double left, double right, double bottom, double top, double nearVal, double farVal)
{
	GLC_Matrix4x4 orthoMatrix;
	double* m= orthoMatrix.setData();

	const double tx= - (right + left) / (right - left);
	const double ty= - (top + bottom) / (top - bottom);
	const double tz= - (farVal + nearVal) / (farVal - nearVal);
	m[0]= 2.0 / (right - left);
	m[5]= 2.0 / (top - bottom);
	m[10]= -2.0 / (farVal - nearVal);
	m[12]= tx;
	m[13]= ty;
	m[14]= tz;

	glcMultMatrix(orthoMatrix);
}

void GLC_Context::glcFrustum(double left, double right, double bottom, double top, double nearVal, double farVal)
{
	GLC_Matrix4x4 perspMatrix;
	double* m= perspMatrix.setData();

	const double a= (right + left) / (right - left);
	const double b= (top + bottom) / (top - bottom);
	const double c= - (farVal + nearVal) / (farVal - nearVal);
	const double d= - (2.0 * farVal * nearVal) / (farVal - nearVal);

	m[0]= (2.0 * nearVal) / (right - left);
	m[5]= (2.0 * nearVal) / (top - bottom);
	m[8]= a;
	m[9]= b;
	m[10]= c;
	m[11]= -1.0;
	m[14]= d;
	m[15]= 0.0;

	glcMultMatrix(perspMatrix);
}

void GLC_Context::glcEnableLighting(bool enable)
{
	if (enable != m_LightingIsEnable.top())
	{
		m_LightingIsEnable.top()= enable;

#ifdef GLC_OPENGL_ES_2

		m_UniformShaderData.setLightingState(m_LightingIsEnable);
#else
		if (GLC_Shader::hasActiveShader())
		{
			m_UniformShaderData.setLightingState(m_LightingIsEnable.top());
		}
		if (m_LightingIsEnable.top()) ::glEnable(GL_LIGHTING);
		else ::glDisable(GL_LIGHTING);
#endif

	}
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

void GLC_Context::makeCurrent()
{
	QGLContext::makeCurrent();
	if (!GLC_State::isValid())
	{
		GLC_State::init();
	}
	GLC_ContextManager::instance()->setCurrent(this);
	m_pCurrentContext= this;
}

void GLC_Context::doneCurrent()
{
	QGLContext::doneCurrent();
	GLC_ContextManager::instance()->setCurrent(NULL);
	m_pCurrentContext= NULL;
}

bool GLC_Context::chooseContext(const QGLContext* shareContext)
{
	qDebug() << "GLC_Context::chooseContext";
	const bool success= QGLContext::chooseContext(shareContext);
	if (!success)
	{
		qDebug() << "enable to create context " << this;
	}
	else if (NULL != shareContext)
	{
		GLC_Context* pContext= const_cast<GLC_Context*>(dynamic_cast<const GLC_Context*>(shareContext));
		Q_ASSERT(NULL != pContext);
		m_ContextSharedData= pContext->m_ContextSharedData;
	}
	else
	{
		m_ContextSharedData= QSharedPointer<GLC_ContextSharedData>(new GLC_ContextSharedData());
	}

	return success;
}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
void GLC_Context::init()
{
	QStack<GLC_Matrix4x4>* pStack1= new QStack<GLC_Matrix4x4>();
	pStack1->push(GLC_Matrix4x4());
	m_MatrixStackHash.insert(GL_MODELVIEW, pStack1);

	QStack<GLC_Matrix4x4>* pStack2= new QStack<GLC_Matrix4x4>();
	pStack2->push(GLC_Matrix4x4());
	m_MatrixStackHash.insert(GL_PROJECTION, pStack2);

	m_LightingIsEnable.push(false);
}

