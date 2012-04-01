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

//! \file glc_shader.cpp implementation of the GLC_Shader class.

#include "glc_shader.h"
#include <QTextStream>
#include <QMutexLocker>
#include "../glc_exception.h"
#include "../glc_state.h"
#include "../glc_context.h"
#include "glc_light.h"

// Static member initialization
QStack<GLC_uint> GLC_Shader::m_ShadingGroupStack;
GLuint GLC_Shader::m_CurrentShadingGroupId= 0;
QHash<GLC_uint, GLC_Shader*> GLC_Shader::m_ShaderProgramHash;

GLC_Shader::GLC_Shader()
: m_VertexShader(QGLShader::Vertex)
, m_FragmentShader(QGLShader::Fragment)
, m_ProgramShader()
, m_ProgramShaderId(glc::GLC_GenShaderGroupID())
, m_Name("Empty Shader")
, m_PositionAttributeId(-1)
, m_TextcoordAttributeId(-1)
, m_ColorAttributeId(-1)
, m_NormalAttributeId(-1)
, m_ModelViewLocationId(-1)
, m_MvpLocationId(-1)
, m_InvModelViewLocationId(-1)
, m_EnableLightingId(-1)
, m_LightsEnableStateId(-1)
, m_LightsPositionId()
, m_LightsAmbientColorId()
, m_LightsDiffuseColorId()
, m_LightsSpecularColorId()
, m_LightsSpotDirectionId()
, m_LightsAttenuationFactorsId()
, m_LightsSpotExponentId()
, m_LightsSpotCutoffAngleId()
, m_LightsComputeDistanceAttenuationId()
{
	initLightsUniformId();
	m_ShaderProgramHash.insert(m_ProgramShaderId, this);
}

GLC_Shader::GLC_Shader(QFile& vertex, QFile& fragment)
: m_VertexShader(QGLShader::Vertex)
, m_FragmentShader(QGLShader::Fragment)
, m_ProgramShader()
, m_ProgramShaderId(glc::GLC_GenShaderGroupID())
, m_Name("Empty Shader")
, m_PositionAttributeId(-1)
, m_TextcoordAttributeId(-1)
, m_ColorAttributeId(-1)
, m_NormalAttributeId(-1)
, m_ModelViewLocationId(-1)
, m_MvpLocationId(-1)
, m_InvModelViewLocationId(-1)
, m_EnableLightingId(-1)
, m_LightsEnableStateId(-1)
, m_LightsPositionId()
, m_LightsAmbientColorId()
, m_LightsDiffuseColorId()
, m_LightsSpecularColorId()
, m_LightsSpotDirectionId()
, m_LightsAttenuationFactorsId()
, m_LightsSpotExponentId()
, m_LightsSpotCutoffAngleId()
, m_LightsComputeDistanceAttenuationId()
{
	initLightsUniformId();
	m_ShaderProgramHash.insert(m_ProgramShaderId, this);
	setVertexAndFragmentShader(vertex, fragment);
}

GLC_Shader::GLC_Shader(const GLC_Shader& shader)
: m_VertexShader(QGLShader::Vertex)
, m_FragmentShader(QGLShader::Fragment)
, m_ProgramShader()
, m_ProgramShaderId(glc::GLC_GenShaderGroupID())
, m_Name(shader.m_Name)
, m_PositionAttributeId(-1)
, m_TextcoordAttributeId(-1)
, m_ColorAttributeId(-1)
, m_NormalAttributeId(-1)
, m_ModelViewLocationId(-1)
, m_MvpLocationId(-1)
, m_InvModelViewLocationId(-1)
, m_EnableLightingId(-1)
, m_LightsEnableStateId(-1)
, m_LightsPositionId()
, m_LightsAmbientColorId()
, m_LightsDiffuseColorId()
, m_LightsSpecularColorId()
, m_LightsSpotDirectionId()
, m_LightsAttenuationFactorsId()
, m_LightsSpotExponentId()
, m_LightsSpotCutoffAngleId()
, m_LightsComputeDistanceAttenuationId()
{
	initLightsUniformId();
	m_ShaderProgramHash.insert(m_ProgramShaderId, this);

	if (shader.m_VertexShader.isCompiled())
	{
		m_VertexShader.compileSourceCode(shader.m_VertexShader.sourceCode());
	}
	if (shader.m_FragmentShader.isCompiled())
	{
		m_FragmentShader.compileSourceCode(shader.m_FragmentShader.sourceCode());
	}

	createAndCompileProgrammShader();
}

GLC_Shader::~GLC_Shader()
{
	deleteShader();
}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

bool GLC_Shader::canBeDeleted() const
{
	return m_CurrentShadingGroupId != m_ProgramShaderId;
}

int GLC_Shader::shaderCount()
{
	return m_ShaderProgramHash.size();
}

bool GLC_Shader::asShader(GLC_uint shadingGroupId)
{
	return m_ShaderProgramHash.contains(shadingGroupId);
}

GLC_Shader* GLC_Shader::shaderHandle(GLC_uint shadingGroupId)
{
	return m_ShaderProgramHash.value(shadingGroupId);
}

bool GLC_Shader::hasActiveShader()
{
	return 0 != m_CurrentShadingGroupId;
}

GLC_Shader* GLC_Shader::currentShaderHandle()
{
	return m_ShaderProgramHash.value(m_CurrentShadingGroupId);
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_Shader::use()
{
	if (GLC_State::isInSelectionMode()) return;
	// Program shader must be valid
	Q_ASSERT(m_ProgramShader.isLinked());

	m_ShadingGroupStack.push(m_ProgramShaderId);
	// Test if the program shader is not already the current one
	if (m_CurrentShadingGroupId != m_ProgramShaderId)
	{
		m_CurrentShadingGroupId= m_ProgramShaderId;
		m_ShaderProgramHash.value(m_CurrentShadingGroupId)->m_ProgramShader.bind();
		GLC_Context::current()->updateUniformVariables();
	}

}

bool GLC_Shader::use(GLC_uint shaderId)
{
	Q_ASSERT(0 != shaderId);
	if (GLC_State::isInSelectionMode()) return false;

	if (m_ShaderProgramHash.contains(shaderId))
	{
		m_ShadingGroupStack.push(shaderId);
		// Test if the program shader is not already the current one
		if (m_CurrentShadingGroupId != shaderId)
		{
			m_CurrentShadingGroupId= shaderId;
			m_ShaderProgramHash.value(m_CurrentShadingGroupId)->m_ProgramShader.bind();
			GLC_Context::current()->updateUniformVariables();
		}

		return true;
	}
	else
	{
		return false;
	}
}

void GLC_Shader::unuse()
{

	if (GLC_State::isInSelectionMode()) return;

	Q_ASSERT(!m_ShadingGroupStack.isEmpty());

	const GLC_uint stackShadingGroupId= m_ShadingGroupStack.pop();
	if (m_ShadingGroupStack.isEmpty())
	{
		m_CurrentShadingGroupId= 0;
		m_ShaderProgramHash.value(stackShadingGroupId)->m_ProgramShader.release();
	}
	else
	{
		m_CurrentShadingGroupId= m_ShadingGroupStack.top();
		m_ShaderProgramHash.value(m_CurrentShadingGroupId)->m_ProgramShader.bind();
	}
}

void GLC_Shader::createAndCompileProgrammShader()
{
	qDebug() << "GLC_Shader::createAndCompileProgrammShader()";
	m_ProgramShader.addShader(&m_VertexShader);
	m_ProgramShader.addShader(&m_FragmentShader);

	if (!m_ProgramShader.link())
	{
		QString message("GLC_Shader::setVertexAndFragmentShader Failed to link program ");
		GLC_Exception exception(message);
		throw(exception);
	}
	else
	{
		m_PositionAttributeId= m_ProgramShader.attributeLocation("a_position");
		//qDebug() << "m_PositionAttributeId " << m_PositionAttributeId;
		m_TextcoordAttributeId= m_ProgramShader.attributeLocation("a_textcoord0");
		//qDebug() << "m_TextcoordAttributeId " << m_TextcoordAttributeId;
		m_ColorAttributeId= m_ProgramShader.attributeLocation("a_color");
		//qDebug() << "m_ColorAttributeId " << m_ColorAttributeId;
		m_NormalAttributeId= m_ProgramShader.attributeLocation("a_normal");
		//qDebug() << "m_NormalAttributeId " << m_NormalAttributeId;

		m_ModelViewLocationId= m_ProgramShader.uniformLocation("modelview_matrix");
		//qDebug() << "m_ModelViewLocationId " << m_ModelViewLocationId;
		m_MvpLocationId= m_ProgramShader.uniformLocation("mvp_matrix");
		//qDebug() << "m_MvpLocationId " << m_MvpLocationId;
		m_InvModelViewLocationId= m_ProgramShader.uniformLocation("inv_modelview_matrix");
		//qDebug() << "m_InvModelViewLocationId " << m_InvModelViewLocationId;
		m_EnableLightingId= m_ProgramShader.uniformLocation("enable_lighting");
		//qDebug() << "m_EnableLightingId " << m_EnableLightingId;
		m_LightsEnableStateId= m_ProgramShader.uniformLocation("light_enable_state");
		//qDebug() << "m_LightsEnableStateId " << m_LightsEnableStateId;
		const int size= GLC_Light::maxLightCount();
		for (int i= (GL_LIGHT0); i < (size + GL_LIGHT0); ++i)
		{
			m_LightsPositionId[i]= m_ProgramShader.uniformLocation("light_state[" + QString::number(i) + "].position");
			//qDebug() << "Position id " << m_LightsPositionId.value(i);
			m_LightsAmbientColorId[i]= m_ProgramShader.uniformLocation("light_state[" + QString::number(i) + "].ambient_color");
			//qDebug() << "m_LightsAmbientColorId " << m_LightsAmbientColorId.value(i);
			m_LightsDiffuseColorId[i]= m_ProgramShader.uniformLocation("light_state[" + QString::number(i) + "].diffuse_color");
			//qDebug() << "m_LightsDiffuseColorId " << m_LightsDiffuseColorId.value(i);
			m_LightsSpecularColorId[i]= m_ProgramShader.uniformLocation("light_state[" + QString::number(i) + "].specular_color");
			//qDebug() << "m_LightsSpecularColorId " << m_LightsSpecularColorId.value(i);
			m_LightsSpotDirectionId[i]= m_ProgramShader.uniformLocation("light_state[" + QString::number(i) + "].spot_direction");
			//qDebug() << "m_LightsSpotDirectionId " << m_LightsSpotDirectionId.value(i);
			m_LightsAttenuationFactorsId[i]= m_ProgramShader.uniformLocation("light_state[" + QString::number(i) + "].attenuation_factors");
			//qDebug() << "m_LightsAttenuationFactorsId " << m_LightsAttenuationFactorsId.value(i);
			m_LightsSpotExponentId[i]= m_ProgramShader.uniformLocation("light_state[" + QString::number(i) + "].spot_exponent");
			//qDebug() << "m_LightsSpotExponentId " << m_LightsSpotExponentId.value(i);
			m_LightsSpotCutoffAngleId[i]= m_ProgramShader.uniformLocation("light_state[" + QString::number(i) + "].spot_cutoff_angle");
			//qDebug() << "m_LightsSpotCutoffAngleId " << m_LightsSpotCutoffAngleId.value(i);
			m_LightsComputeDistanceAttenuationId[i]= m_ProgramShader.uniformLocation("light_state[" + QString::number(i) + "].compute_distance_attenuation");
			//qDebug() << "m_LightsComputeDistanceAttenuationId " << m_LightsComputeDistanceAttenuationId.value(i);


		}
	}
}

void GLC_Shader::deleteShader()
{
	if (m_ProgramShaderId != 0)
	{
		// Test if the shader is the current one
		if (m_CurrentShadingGroupId == m_ProgramShaderId)
		{
			qDebug() << "Warning deleting current shader";
		}
		//removing shader id from the stack
		if (m_ShadingGroupStack.contains(m_ProgramShaderId))
		{
			int indexToDelete= m_ShadingGroupStack.indexOf(m_ProgramShaderId);
			while (indexToDelete != -1)
			{
				m_ShadingGroupStack.remove(indexToDelete);
				indexToDelete= m_ShadingGroupStack.indexOf(m_ProgramShaderId);
			}
		}
		m_ShaderProgramHash.remove(m_ProgramShaderId);
	}

}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////


void GLC_Shader::setVertexAndFragmentShader(QFile& vertexFile, QFile& fragmentFile)
{
	m_Name= QFileInfo(vertexFile).baseName();
	vertexFile.open(QIODevice::ReadOnly);
	m_VertexShader.compileSourceCode(vertexFile.readAll());
	vertexFile.close();

	fragmentFile.open(QIODevice::ReadOnly);
	m_FragmentShader.compileSourceCode(fragmentFile.readAll());
	fragmentFile.close();
}


void GLC_Shader::replaceShader(const GLC_Shader& sourceShader)
{
	Q_ASSERT(isUsable() == sourceShader.isUsable());

	// Test if the source shader is the same than this shader
	if (this == &sourceShader)
	{
		return;
	}
	m_ProgramShader.removeAllShaders();

	if (sourceShader.m_VertexShader.isCompiled())
	{
		m_VertexShader.compileSourceCode(sourceShader.m_VertexShader.sourceCode());
	}
	if (sourceShader.m_FragmentShader.isCompiled())
	{
		m_FragmentShader.compileSourceCode(sourceShader.m_FragmentShader.sourceCode());
	}

	m_ProgramShader.link();

}

void GLC_Shader::initLightsUniformId()
{
	m_LightsPositionId.clear();
	m_LightsAmbientColorId.clear();
	m_LightsDiffuseColorId.clear();
	m_LightsSpecularColorId.clear();
	m_LightsSpotDirectionId.clear();
	m_LightsAttenuationFactorsId.clear();
	m_LightsSpotExponentId.clear();
	m_LightsSpotCutoffAngleId.clear();
	m_LightsComputeDistanceAttenuationId.clear();

	for (int i= 0; i < GLC_Light::maxLightCount(); ++i)
	{
		m_LightsPositionId.insert(GL_LIGHT0 + i, -1);
		m_LightsAmbientColorId.insert(GL_LIGHT0 + i, -1);
		m_LightsDiffuseColorId.insert(GL_LIGHT0 + i, -1);
		m_LightsSpecularColorId.insert(GL_LIGHT0 + i, -1);
		m_LightsSpotDirectionId.insert(GL_LIGHT0 + i, -1);
		m_LightsAttenuationFactorsId.insert(GL_LIGHT0 + i, -1);
		m_LightsSpotExponentId.insert(GL_LIGHT0 + i, -1);
		m_LightsSpotCutoffAngleId.insert(GL_LIGHT0 + i, -1);
		m_LightsComputeDistanceAttenuationId.insert(GL_LIGHT0 + i, -1);
	}
}

