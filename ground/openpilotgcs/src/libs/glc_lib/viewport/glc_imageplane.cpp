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
#include "../glc_context.h"
#include <QtDebug>

//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////

GLC_ImagePlane::GLC_ImagePlane(const QString& ImageName)
: m_Representation(GLC_Factory::instance()->createRectangle(2.0, 2.0))
{
	GLC_Texture* pImgTexture= GLC_Factory::instance()->createTexture(ImageName);
	pImgTexture->setMaxTextureSize(pImgTexture->imageOfTexture().size());
	m_Representation.geomAt(0)->addMaterial(new GLC_Material(pImgTexture));
}

GLC_ImagePlane::GLC_ImagePlane(const QImage& image)
: m_Representation(GLC_Factory::instance()->createRectangle(2.0, 2.0))
{
	GLC_Texture* pImgTexture= GLC_Factory::instance()->createTexture(image);
	pImgTexture->setMaxTextureSize(image.size());
	m_Representation.geomAt(0)->addMaterial(new GLC_Material(pImgTexture));
}

GLC_ImagePlane::~GLC_ImagePlane()
{

}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_ImagePlane::render()
{
	GLC_Context::current()->glcMatrixMode(GL_PROJECTION);
	GLC_Context::current()->glcPushMatrix();
	GLC_Context::current()->glcLoadIdentity();
	GLC_Context::current()->glcOrtho(-1,1,-1,1,-1,1);
	GLC_Context::current()->glcMatrixMode(GL_MODELVIEW);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	m_Representation.render();

	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_DEPTH_TEST);

	GLC_Context::current()->glcMatrixMode(GL_PROJECTION);
	GLC_Context::current()->glcPopMatrix();
	GLC_Context::current()->glcMatrixMode(GL_MODELVIEW);
}
