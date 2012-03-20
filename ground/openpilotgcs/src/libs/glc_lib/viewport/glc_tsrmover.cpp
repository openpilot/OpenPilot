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

//! \file glc_tsrmover.cpp Implementation of the GLC_TsrMover class.

#include "glc_tsrmover.h"
#include "glc_viewport.h"

#include "../geometry/glc_point.h"

// Default constructor
GLC_TsrMover::GLC_TsrMover(GLC_Viewport* pViewport, const QList<GLC_RepMover*>& repsList)
: GLC_Mover(pViewport, repsList)
{

}

// Copy constructor
GLC_TsrMover::GLC_TsrMover(const GLC_TsrMover& tsrMover)
: GLC_Mover(tsrMover)
{

}


GLC_TsrMover::~GLC_TsrMover()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Return a clone of the mover
GLC_Mover* GLC_TsrMover::clone() const
{
	return new GLC_TsrMover(*this);
}


//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Initialized the mover
void GLC_TsrMover::init(const GLC_UserInput& userInput)
{
	m_PreviousVector= GLC_Point3d(userInput.normalyzeXTouchCenter(), userInput.normalyzeYTouchCenter(), 0.0);
}

// Move the camera
bool GLC_TsrMover::move(const GLC_UserInput& userInput)
{
	if (!(userInput.normalyzeXTouchCenter() < 0.0) && !(userInput.normalyzeYTouchCenter() < 0.0))
	{
		m_PreviousVector= GLC_Point3d(userInput.normalyzeXTouchCenter(), userInput.normalyzeYTouchCenter(), 0.0);
	}
	else
	{
		qDebug() << "Pas cool";
		if (!userInput.translation().isNull())
		{
			m_PreviousVector= GLC_Vector3d(userInput.translation().getX(), userInput.translation().getY(), 0.0) + m_PreviousVector;			
		}
	}
	
	const double x= m_PreviousVector.x();
	const double y= m_PreviousVector.y();
	//GLC_Point3d center2= m_pViewport->unProject(x * m_pViewport->viewHSize(), y * m_pViewport->viewVSize());
	
	//qDebug() << "touch center= " << x << " , " << y;

	
	if (!qFuzzyCompare(userInput.scaleFactor(), 0))
	{
		GLC_Point dummy(m_pViewport->cameraHandle()->target());
		m_pViewport->setDistMinAndMax(dummy.boundingBox());

		GLC_Point2d nPos= m_pViewport->mapNormalyzeToOpenGLScreen(x, y);
		GLC_Point3d nPos3D(nPos.getX(), nPos.getY(), 1.0);
		GLC_Point3d projected= m_pViewport->compositionMatrix().inverted() * nPos3D;

		m_pViewport->cameraHandle()->zoom(userInput.scaleFactor());

		m_pViewport->setDistMinAndMax(dummy.boundingBox());
		GLC_Point3d projected2= m_pViewport->compositionMatrix().inverted() * nPos3D;
		GLC_Vector3d delta= projected - projected2;
		m_pViewport->cameraHandle()->translate(delta);
	}

	if (!qFuzzyCompare(userInput.rotationAngle(), 0))
	{
		GLC_Point dummy(m_pViewport->cameraHandle()->target());
		m_pViewport->setDistMinAndMax(dummy.boundingBox());

		GLC_Point2d nPos= m_pViewport->mapNormalyzeToOpenGLScreen(x, y);
		GLC_Point3d nPos3D(nPos.getX(), nPos.getY(), 1.0);
		GLC_Point3d center= m_pViewport->compositionMatrix().inverted() * nPos3D;

		GLC_Vector3d axis= m_pViewport->cameraHandle()->forward();

		m_pViewport->cameraHandle()->rotateAround(axis, userInput.rotationAngle(), center);
	}

	if (!userInput.translation().isNull())
	{
		double transX= userInput.translation().getX() * m_pViewport->viewHSize();
		double transY= userInput.translation().getY() * m_pViewport->viewVSize();

		GLC_Vector3d mappedTranslation(-transX, -transY, 0.0);
		// Compute the length of camera's field of view
		const double ChampsVision = m_pViewport->cameraHandle()->distEyeTarget() *  m_pViewport->viewTangent();

		// the side of camera's square is mapped on Vertical length of window
		// Ratio OpenGL/Pixel = dimend GL / dimens Pixel
		const double Ratio= ChampsVision / static_cast<double>(m_pViewport->viewVSize());

		mappedTranslation= mappedTranslation * Ratio;
		m_pViewport->cameraHandle()->pan(mappedTranslation);
	}

	return true;
}

