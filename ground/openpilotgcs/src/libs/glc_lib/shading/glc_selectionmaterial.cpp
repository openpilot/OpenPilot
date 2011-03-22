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
//! \file glc_selectionmaterial.cpp implementation of the GLC_SelectionMaterial class.

#include "glc_selectionmaterial.h"
#include "glc_material.h"

GLC_Shader* GLC_SelectionMaterial::m_pSelectionShader= NULL;
GLC_uint GLC_SelectionMaterial::m_SelectionMaterialId= 0;
GLC_Material* GLC_SelectionMaterial::m_pMaterial= NULL;

GLC_SelectionMaterial::GLC_SelectionMaterial()
{

}

void GLC_SelectionMaterial::useMaterial(GLC_Material* pMaterial)
{
	if (0 == m_SelectionMaterialId)
	{
		m_SelectionMaterialId= glc::GLC_GenUserID();
	}
	Q_ASSERT(NULL != pMaterial);
	if (NULL != m_pMaterial)
	{
		m_pMaterial->delUsage(m_SelectionMaterialId);
		if (m_pMaterial->isUnused())
		{
			delete m_pMaterial;
		}
	}
		m_pMaterial= pMaterial;
		m_pMaterial->addUsage(m_SelectionMaterialId);
}

void GLC_SelectionMaterial::useDefautSelectionColor()
{
	if (NULL != m_pMaterial)
	{
		m_pMaterial->delUsage(m_SelectionMaterialId);
		if (m_pMaterial->isUnused())
		{
			delete m_pMaterial;
		}
		m_pMaterial= NULL;
	}
}


// Execute OpenGL Material
void GLC_SelectionMaterial::glExecute()
{
	if (NULL != m_pMaterial)
	{
		m_pMaterial->glExecute();
	}
	else
	{
		// Use default selection color
		static GLfloat pAmbientColor[4]= {1.0f, 0.376f, 0.223f, 1.0f};

		static GLfloat pDiffuseColor[4]= {1.0f, 0.376f, 0.223f, 1.0f};

		static GLfloat pSpecularColor[4]= {1.0f, 1.0f, 1.0f, 1.0f};

		static GLfloat pLightEmission[4]= {0.0f, 0.0f, 0.0f, 1.0f};

		static float shininess= 50.0f;

		glColor4fv(pAmbientColor);

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pAmbientColor);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pDiffuseColor);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pSpecularColor);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, pLightEmission);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shininess);
	}
}

void GLC_SelectionMaterial::initShader()
{
	if (m_pSelectionShader == NULL)
	{
		m_pSelectionShader= new GLC_Shader;
	}
	m_pSelectionShader->createAndCompileProgrammShader();
}

void GLC_SelectionMaterial::setShaders(QFile& vertex, QFile& fragment)
{
	if (m_pSelectionShader == NULL)
	{
		m_pSelectionShader= new GLC_Shader;
	}

	m_pSelectionShader->setVertexAndFragmentShader(vertex, fragment);
}

//! delete shader
void GLC_SelectionMaterial::deleteShader()
{
	if (NULL != m_pSelectionShader)
	{
		m_pSelectionShader->deleteShader();
		delete m_pSelectionShader;
		m_pSelectionShader= NULL;
	}
}
