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
//! \file glc_trackballmover.cpp Implementation of the GLC_TrackBallMover class.

#include "glc_trackballmover.h"
#include "glc_viewport.h"
#include "glc_reptrackballmover.h"

// Default constructor
GLC_TrackBallMover::GLC_TrackBallMover(GLC_Viewport* pViewport, const QList<GLC_RepMover*>& repsList)
: GLC_Mover(pViewport, repsList)
, m_Ratio(0.95)
{
	GLC_Mover::m_MoverInfo.m_MatrixInfo.append(GLC_Matrix4x4());
	GLC_Mover::m_MoverInfo.m_VectorInfo.append(GLC_Vector3d());
}

// Copy constructor
GLC_TrackBallMover::GLC_TrackBallMover(const GLC_TrackBallMover& mover)
: GLC_Mover(mover)
, m_Ratio(mover.m_Ratio)
{

}

GLC_TrackBallMover::~GLC_TrackBallMover()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Return a clone of the mover
GLC_Mover* GLC_TrackBallMover::clone() const
{
	return new GLC_TrackBallMover(*this);
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Initialized the mover
void GLC_TrackBallMover::init(const GLC_UserInput& userInput)
{
	GLC_Mover::m_PreviousVector.setVect(mapForTracking(static_cast<double>(userInput.x()), static_cast<double>(userInput.y())));

	const double Angle= acos(glc::Z_AXIS * GLC_Mover::m_PreviousVector);
	const GLC_Vector3d AxeRot(glc::Z_AXIS ^ GLC_Mover::m_PreviousVector);

	GLC_Matrix4x4 Matrice(AxeRot, Angle);

	GLC_Mover::m_MoverInfo.m_MatrixInfo.first()= Matrice;
	GLC_Mover::m_MoverInfo.m_VectorInfo.first()= GLC_Mover::m_PreviousVector;
	// Update trackball representations
	initRepresentation();
}

// Move the camera
bool GLC_TrackBallMover::move(const GLC_UserInput& userInput)
{
	const GLC_Vector3d VectCurOrbit(mapForTracking(static_cast<double>(userInput.x()), static_cast<double>(userInput.y())));

	// Update camera position (orbit)
	GLC_Mover::m_pViewport->cameraHandle()->orbit(GLC_Mover::m_PreviousVector, VectCurOrbit);

	// Update arcs of circle's positionning matrix
	const GLC_Matrix4x4 MatRot(GLC_Mover::m_PreviousVector, VectCurOrbit);

	GLC_Mover::m_MoverInfo.m_MatrixInfo.first()= MatRot;
	updateRepresentation();

	// Previous vector become current vector
	GLC_Mover::m_PreviousVector = VectCurOrbit;

	return true;
}

void GLC_TrackBallMover::setRatio(double ratio)
{
	m_Ratio= ratio;
	const int repCount= m_RepMoverList.count();
	for (int i= 0; i < repCount; ++i)
	{
		GLC_RepTrackBallMover* pRep= dynamic_cast<GLC_RepTrackBallMover*>(m_RepMoverList.at(i));
		if (NULL != pRep)
		{
			pRep->setRatio(ratio);
		}
	}
}
/////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

// Convert mouse View coordinate to tracking coordinate (Centred and betwen (-1,-1) and (1,1))
GLC_Vector3d GLC_TrackBallMover::mapForTracking( double x, double y) const
{
	double AspectRatio;
	const double winHSize= static_cast<double>(GLC_Mover::m_pViewport->viewHSize());
	const double winVSize= static_cast<double>(GLC_Mover::m_pViewport->viewVSize());

	// Change origine and cover
	if (winHSize < winVSize)
	{
		AspectRatio= winVSize / winHSize;
		x= ( (x - winHSize  / 2.0 ) / ( winHSize / 2.0) ) / m_Ratio;
		y= AspectRatio * ( ( winVSize / 2.0 - y) / ( winVSize / 2.0 ) ) / m_Ratio;
	}
	else
	{
		AspectRatio= winHSize / winVSize;
		x= AspectRatio * ( (x - winHSize  / 2.0 ) / ( winHSize / 2.0) ) / m_Ratio;
		y= ( (winVSize / 2.0 - y) / (winVSize / 2.0 ) ) / m_Ratio;
	}

	// Distance between pick point and origine can't be over then 1 (1 is radius of orbit circle)
	GLC_Vector3d mousePos(x, y, 0.0);
	if (mousePos.length() > 1.0)
	{
		mousePos.setLength(1.0);
	}
	else
	{
		mousePos.setZ(sqrt(1.0 - mousePos.x() *  mousePos.x() - mousePos.y() * mousePos.y()));
	}

	return mousePos;

}
