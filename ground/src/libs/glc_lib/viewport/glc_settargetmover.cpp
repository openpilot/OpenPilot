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

#include "glc_settargetmover.h"
#include "glc_viewport.h"
#include "glc_openglexception.h"

// Default constructor
GLC_SetTargetMover::GLC_SetTargetMover(GLC_Viewport* pViewport, const QList<GLC_RepMover*>& repsList)
: GLC_Mover(pViewport, repsList)
{


}

// Copy constructor
GLC_SetTargetMover::GLC_SetTargetMover(const GLC_SetTargetMover& mover)
: GLC_Mover(mover)
{


}

GLC_SetTargetMover::~GLC_SetTargetMover()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Return a clone of the mover
GLC_Mover* GLC_SetTargetMover::clone() const
{
	return new GLC_SetTargetMover(*this);
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Initialized the mover
void GLC_SetTargetMover::init(int x, int y)
{
	// Z Buffer component of selected point between 0 and 1
	GLfloat Depth;
	// read selected point
	glReadPixels(x, m_pViewport->viewVSize() - y , 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Depth);

	// Current visualisation matrix
	GLdouble ViewMatrix[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, ViewMatrix);
	// Current projection matrix
	GLdouble ProjMatrix[16];
	glGetDoublev(GL_PROJECTION_MATRIX, ProjMatrix);
	// definition of the current viewport
	GLint Viewport[4];
	glGetIntegerv(GL_VIEWPORT, Viewport);

	// OpenGL ccordinate of selected point
	GLdouble pX, pY, pZ;
	gluUnProject((GLdouble) x, (GLdouble) (m_pViewport->viewVSize() - y) , Depth
		, ViewMatrix, ProjMatrix, Viewport, &pX, &pY, &pZ);

	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_Viewport::GlPointing ", error);
		throw(OpenGlException);
	}

	// Test if there is geometry under picking point
	if (not qFuzzyCompare(Depth, 1.0f))
	{	// Geometry find -> Update camera's target position
		const GLC_Point4d target(pX, pY, pZ);
		m_pViewport->cameraHandle()->setTargetCam(target);
	}
	else
	{	// Geometrie not find -> panning

		const GLC_Point4d curPos(m_pViewport->mapPosMouse(x, y));
		const GLC_Point4d prevPos(m_pViewport->mapPosMouse(m_pViewport->viewHSize() / 2, m_pViewport->viewVSize() / 2));
		const GLC_Vector4d VectPan(curPos - prevPos);	// panning vector
		// pan camera
		m_pViewport->cameraHandle()->pan(VectPan);
	}
}
