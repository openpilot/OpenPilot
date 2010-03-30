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

//! \file glc_imagePlane.cpp implementation of the GLC_ImagePlane class.
#include "glc_imageplane.h"
#include "glc_viewport.h"
#include "../glc_openglexception.h"

#include <QtDebug>

using namespace glc;
//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////
GLC_ImagePlane::GLC_ImagePlane(GLC_Viewport* pViewport)
:GLC_VboGeom("Image Plane", false)
, m_pViewport(pViewport)
, m_dLgImage(0)
, m_dZpos(0)
, m_PolyFace(GL_FRONT_AND_BACK)	// Default Faces style
, m_PolyMode(GL_FILL)			// Default polyganal mode
, m_BoundingBox()
{
	addMaterial(new GLC_Material());

	updateZPosition();
}

GLC_ImagePlane::~GLC_ImagePlane(void)
{

}
/////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Return a copy of the current geometry
GLC_VboGeom* GLC_ImagePlane::clone() const
{
	return NULL;
}

//! Return the geometry bounding box
GLC_BoundingBox& GLC_ImagePlane::boundingBox()
{
	return m_BoundingBox;
}

/////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
// Load image
void GLC_ImagePlane::loadImageFile(const QGLContext *pContext, const QString ImageName)
{
	GLC_Texture* pImgTexture= new GLC_Texture(pContext, ImageName);
	firstMaterial()->setTexture(pImgTexture);
}

// Update image size
void GLC_ImagePlane::updatePlaneSize(void)
{

	// compute quad size
	int nCote;
	if (m_pViewport->viewHSize() < m_pViewport->viewVSize())
	{
		nCote= m_pViewport->viewVSize();
	}
	else
	{
		nCote= m_pViewport->viewHSize();
	}

	// Calcul du coté du carré de vision de la caméra
	// Le coté du carré de la caméra est mappé sur la hauteur de la fenètre
	const double ChampsVision = 2 * m_dZpos *  tan((m_pViewport->viewAngle() * PI / 180)/ 2);

	// Circle radius in openGL unit = RayonPixel * (dimens GL / dimens Pixel)
	m_dLgImage= ((double)nCote * ChampsVision / (double)m_pViewport->viewVSize());

	// Invalidate OpenGL Display list
	m_GeometryIsValid= false;
}

// Update Plane Z position
void GLC_ImagePlane::updateZPosition(void)
{
	// Compute Plane Z position
	const double n= m_pViewport->nearClippingPlaneDist();
	const double f= m_pViewport->farClippingPlaneDist();
	int nbrBits;

	//glGetIntegerv(GL_DEPTH_BITS, &nbrBits);
	// glGetIntegerv seems to not work
	// force to minimum Depth : 16 bits
	nbrBits= 16;

	double zw= pow(2, static_cast<double>(nbrBits)) - 2.0;

	m_dZpos= -f * n / (((zw - 1) / zw) * (f - n) - f);

	updatePlaneSize();
	// Invalidate Geometry
	m_GeometryIsValid= false;

}


//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////
// Plane Display
void GLC_ImagePlane::glDraw(bool)
{
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	const double LgImgSur2= m_dLgImage / 2;
	glPushMatrix();
	glLoadIdentity();
	glTranslated(0.0, 0.0, -m_dZpos);
	glPolygonMode(m_PolyFace, m_PolyMode);
	glBegin(GL_QUADS);

		glNormal3d(0.0, 0.0, 1.0);	// Z
		glTexCoord2f(0.0f, 0.0f); glVertex3d(-LgImgSur2, -LgImgSur2, 0.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3d(LgImgSur2, -LgImgSur2, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3d(LgImgSur2, LgImgSur2, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3d(-LgImgSur2, LgImgSur2, 0.0);

	glEnd();
	glPopMatrix();
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_ImagePlane::GlDraw ", error);
		throw(OpenGlException);
	}
}


