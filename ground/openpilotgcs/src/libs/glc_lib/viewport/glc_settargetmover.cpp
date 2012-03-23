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
void GLC_SetTargetMover::init(const GLC_UserInput& userInput)
{
	// Z Buffer component of selected point between 0 and 1
	GLfloat Depth;
	// read selected point
	glReadPixels(userInput.x(), m_pViewport->viewVSize() - userInput.y() , 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Depth);

	// Test if there is geometry under picking point
	if (!qFuzzyCompare(Depth, 1.0f))
	{	// Geometry find -> Update camera's target position
		const GLC_Point3d target(m_pViewport->unProject(userInput.x(), userInput.y()));
		m_pViewport->cameraHandle()->setTargetCam(target);
	}
	else
	{	// Geometry not find -> panning

		const GLC_Point3d curPos(m_pViewport->mapPosMouse(userInput.x(), userInput.y()));
		const GLC_Point3d prevPos(m_pViewport->mapPosMouse(m_pViewport->viewHSize() / 2, m_pViewport->viewVSize() / 2));
		const GLC_Vector3d VectPan(curPos - prevPos);	// panning vector
		// pan camera
		m_pViewport->cameraHandle()->pan(VectPan);
	}
}
