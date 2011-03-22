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

//! \file glc_imagePlane.cpp implementation of the GLC_ImagePlane class.
#include "glc_imageplane.h"
#include "glc_viewport.h"
#include "../glc_openglexception.h"
#include "../glc_factory.h"
#include <QtDebug>

//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////

GLC_ImagePlane::GLC_ImagePlane(const QGLContext *pContext, const QString& ImageName)
: m_Material()
{
	GLC_Texture* pImgTexture= GLC_Factory::instance(pContext)->createTexture(ImageName);
	pImgTexture->setMaxTextureSize(pImgTexture->imageOfTexture().size());
	m_Material.setTexture(pImgTexture);
}

GLC_ImagePlane::GLC_ImagePlane(const QGLContext *pContext, const QImage& image)
: m_Material()
{
	GLC_Texture* pImgTexture= GLC_Factory::instance(pContext)->createTexture(image);
	pImgTexture->setMaxTextureSize(image.size());
	m_Material.setTexture(pImgTexture);
}

GLC_ImagePlane::~GLC_ImagePlane()
{

}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_ImagePlane::render()
{
	m_Material.glExecute();
	// Display info area
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1,1,-1,1,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_QUADS);

		glNormal3d(0.0, 0.0, 1.0);	// Z
		glTexCoord2f(0.0f, 0.0f); glVertex3d(-1.0, -1.0, 0.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3d(1.0, -1.0, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3d(1.0, 1.0, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3d(-1.0, 1.0, 0.0);

	glEnd();
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}
