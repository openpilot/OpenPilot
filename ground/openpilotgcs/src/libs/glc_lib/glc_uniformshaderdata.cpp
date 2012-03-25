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
//! \file glc_uniformshaderdata.cpp implementation of the GLC_UniformShaderData class.

#include <QtDebug>

#include "shading/glc_shader.h"
#include "glc_context.h"
#include "glc_uniformshaderdata.h"


GLC_UniformShaderData::GLC_UniformShaderData()
{


}

GLC_UniformShaderData::~GLC_UniformShaderData()
{

}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
void GLC_UniformShaderData::setLightValues(const GLC_Light& light)
{

}

void GLC_UniformShaderData::setLightingState(bool enable)
{
	GLC_Shader* pCurrentShader= GLC_Shader::currentShaderHandle();
	pCurrentShader->programShaderHandle()->setUniformValue(pCurrentShader->enableLightingId(), enable);
}

void GLC_UniformShaderData::setModelViewProjectionMatrix(const GLC_Matrix4x4& modelView, const GLC_Matrix4x4& projection)
{
	// Set model view matrix
	const double* pMvmatrixData= modelView.getData();
	GLfloat mvFloatMatrix[4][4];
	GLfloat* pData= &(mvFloatMatrix[0][0]);
	for (int i= 0; i < 16; ++i)
	{
		pData[i]= static_cast<GLfloat>(pMvmatrixData[i]);
	}

	// Set model view projection matrix
	GLC_Matrix4x4 modelViewProjectionMatrix= projection * modelView;
	const double* pMvpmatrixData= modelViewProjectionMatrix.getData();
	GLfloat mvpFloatMatrix[4][4];
	pData= &(mvpFloatMatrix[0][0]);
	for (int i= 0; i < 16; ++i)
	{
		pData[i]= static_cast<GLfloat>(pMvpmatrixData[i]);
	}

	// Set the transpose of inv model view matrix (For normal computation)
	GLC_Matrix4x4 invTransposeModelView= modelView.inverted();
	invTransposeModelView.transpose();
	GLfloat invTmdv[3][3];
	{
		const double* data= invTransposeModelView.getData();

		invTmdv[0][0]= static_cast<GLfloat>(data[0]); invTmdv[1][0]= static_cast<GLfloat>(data[4]); invTmdv[2][0]= static_cast<GLfloat>(data[8]);
		invTmdv[0][1]= static_cast<GLfloat>(data[1]); invTmdv[1][1]= static_cast<GLfloat>(data[5]); invTmdv[2][1]= static_cast<GLfloat>(data[9]);
		invTmdv[0][2]= static_cast<GLfloat>(data[2]); invTmdv[1][2]= static_cast<GLfloat>(data[6]); invTmdv[2][2]= static_cast<GLfloat>(data[10]);
	}

	Q_ASSERT(GLC_Shader::hasActiveShader());

	GLC_Shader* pCurrentShader= GLC_Shader::currentShaderHandle();
	pCurrentShader->programShaderHandle()->setUniformValue(pCurrentShader->modelViewLocationId(), mvFloatMatrix);
	pCurrentShader->programShaderHandle()->setUniformValue(pCurrentShader->mvpLocationId(), mvpFloatMatrix);
	pCurrentShader->programShaderHandle()->setUniformValue(pCurrentShader->invModelViewLocationId(), invTmdv);
}

void GLC_UniformShaderData::updateAll(const GLC_Context* pContext)
{
	setModelViewProjectionMatrix(pContext->modelViewMatrix(), pContext->projectionMatrix());
	setLightingState(pContext->lightingIsEnable());
}
