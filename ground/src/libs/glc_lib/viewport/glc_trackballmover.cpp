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

#include "glc_trackballmover.h"
#include "glc_viewport.h"

// Default constructor
GLC_TrackBallMover::GLC_TrackBallMover(GLC_Viewport* pViewport, const QList<GLC_RepMover*>& repsList)
: GLC_Mover(pViewport, repsList)
, m_Ratio(0.95)
{

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
void GLC_TrackBallMover::init(int x, int y)
{
	m_PreviousVector.setVect(mapForTracking(static_cast<double>(x), static_cast<double>(y)));

	const double Angle= acos(glc::Z_AXIS * m_PreviousVector);
	const GLC_Vector4d AxeRot(glc::Z_AXIS ^ m_PreviousVector);

	GLC_Matrix4x4 Matrice(AxeRot, Angle);

	// Update trackball representations
	initRepresentation(m_PreviousVector, Matrice);
}

// Move the camera
void GLC_TrackBallMover::move(int x, int y)
{
	const GLC_Vector4d VectCurOrbit(mapForTracking(static_cast<double>(x), static_cast<double>(y)));

	// Update camera position (orbit)
	m_pViewport->cameraHandle()->orbit(m_PreviousVector, VectCurOrbit);

	// Update arcs of circle's positionning matrix
	const GLC_Matrix4x4 MatRot(m_PreviousVector, VectCurOrbit);
	updateRepresentation(MatRot);

	// Previous vector become current vector
	m_PreviousVector = VectCurOrbit;
}

/////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

// Convert mouse View coordinate to tracking coordinate (Centred and betwen (-1,-1) and (1,1))
GLC_Vector4d GLC_TrackBallMover::mapForTracking( double x, double y) const
{
	double AspectRatio;
	const double winHSize= static_cast<double>(m_pViewport->viewHSize());
	const double winVSize= static_cast<double>(m_pViewport->viewVSize());

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
	GLC_Vector4d mousePos(x, y, 0.0);
	if (mousePos.norm() > 1.0)
	{
		mousePos.setNormal(1.0);
	}
	else
	{
		mousePos.setZ(sqrt(1.0 - mousePos.X() *  mousePos.X() - mousePos.Y() * mousePos.Y()));
	}

	return mousePos;

}
