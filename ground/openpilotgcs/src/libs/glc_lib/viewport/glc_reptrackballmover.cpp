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

//! \file glc_reptrackballmover.cpp Implementation of the GLC_RepTrackBallMover class.

#include "glc_reptrackballmover.h"
#include "glc_viewport.h"
#include "../glc_factory.h"
#include "../glc_context.h"
#include <QGLContext>

using namespace glc;
//! The angle of arcs
#define ARCANGLE (30 * PI / 180)

GLC_RepTrackBallMover::GLC_RepTrackBallMover(GLC_Viewport* pViewport)
: GLC_RepMover(pViewport)
, m_Radius(1.0)
, m_MainCircle(m_Radius)
, m_Arc1(GLC_Factory::instance()->createCircle(m_Radius, ARCANGLE))
, m_MatArc1()
, m_Arc2(GLC_Factory::instance()->createCircle(m_Radius, ARCANGLE))
, m_MatArc2()
, m_Ratio(0.95)
{
	m_MainCircle.setWireColor(GLC_RepMover::m_MainColor);
	m_Arc1.geomAt(0)->setWireColor(GLC_RepMover::m_MainColor);
	m_Arc2.geomAt(0)->setWireColor(GLC_RepMover::m_MainColor);

	// 2 circle arcs position
	GLC_Matrix4x4 MatRot(Z_AXIS, -ARCANGLE / 2);
	GLC_Matrix4x4 MatInt(Y_AXIS, -PI / 2);
	MatRot= MatInt * MatRot;

	m_MatArc1= MatRot;

	MatInt.setMatRot(Z_AXIS, PI/2);
	MatRot= MatInt * MatRot;

	m_MatArc2= MatRot;

}

// Copy constructor
GLC_RepTrackBallMover::GLC_RepTrackBallMover(const GLC_RepTrackBallMover& repMover)
: GLC_RepMover(repMover)
, m_Radius(repMover.m_Radius)
, m_MainCircle(repMover.m_MainCircle)
, m_Arc1(repMover.m_Arc1.deepCopy())
, m_MatArc1(repMover.m_MatArc1)
, m_Arc2(repMover.m_Arc2.deepCopy())
, m_MatArc2(repMover.m_MatArc2)
, m_Ratio(repMover.m_Ratio)
{

}

GLC_RepTrackBallMover::~GLC_RepTrackBallMover()
{

}


//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Return a clone of the repmover
GLC_RepMover* GLC_RepTrackBallMover::clone() const
{
	return new GLC_RepTrackBallMover(*this);
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set Arcs orientation and position in concordance with mouse position
void GLC_RepTrackBallMover::init()
{
	Q_ASSERT(NULL != m_pRepMoverInfo);
	Q_ASSERT(!m_pRepMoverInfo->m_VectorInfo.isEmpty());
	Q_ASSERT(!m_pRepMoverInfo->m_MatrixInfo.isEmpty());

	GLC_Vector3d VectAngle(m_pRepMoverInfo->m_VectorInfo.first());
	VectAngle.setZ(0);
	VectAngle.setLength(1);

	GLC_Matrix4x4 MatRot;
	double Angle;

	// Compute the 2 arcs orientation
	if (VectAngle.y() > 0)
	{	// Angle entre 0 et PI
		Angle= acos(VectAngle.x());
		MatRot.setMatRot(Z_AXIS, Angle);
	}
	else
	{	// Angle between 0 et -PI
		Angle= -acos(VectAngle.x());
		MatRot.setMatRot(Z_AXIS, Angle);
	}

	// Composition of orientation matrix and mapping matrix
	MatRot= m_pRepMoverInfo->m_MatrixInfo.first() * MatRot;

	m_Arc1.setMatrix(MatRot * m_MatArc1);
	m_Arc2.setMatrix(MatRot * m_MatArc2);
}

// Set Arcs position in concordance with mouse position
void GLC_RepTrackBallMover::update()
{
	Q_ASSERT(NULL != m_pRepMoverInfo);
	Q_ASSERT(!m_pRepMoverInfo->m_MatrixInfo.isEmpty());
	const GLC_Matrix4x4 matrix(m_pRepMoverInfo->m_MatrixInfo.first());
	m_Arc1.multMatrix(matrix);
	m_Arc2.multMatrix(matrix);
}

// overload function setColor(color);
void GLC_RepTrackBallMover::setMainColor(const QColor& color)
{
	GLC_RepMover::setMainColor(color);
	m_MainCircle.setWireColor(GLC_RepMover::m_MainColor);
	m_Arc1.geomAt(0)->setWireColor(GLC_RepMover::m_MainColor);
	m_Arc2.geomAt(0)->setWireColor(GLC_RepMover::m_MainColor);
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Virtual interface for OpenGL Geometry set up.
void GLC_RepTrackBallMover::glDraw()
{
	computeRadius();
	const double aspectRatio= static_cast<double>(m_pViewport->viewHSize())/static_cast<double>(m_pViewport->viewVSize());

	// orbit circle must be shown
	glDisable(GL_DEPTH_TEST);

	GLC_Context::current()->glcMatrixMode(GL_PROJECTION);
	GLC_Context::current()->glcPushMatrix();
	GLC_Context::current()->glcLoadIdentity();
	GLC_Context::current()->glcOrtho(aspectRatio * -1.0 ,aspectRatio * 1.0 ,-1.0 ,1.0 ,-1.0 ,1.0);
	GLC_Context::current()->glcMatrixMode(GL_MODELVIEW);
	GLC_Context::current()->glcPushMatrix();
	GLC_Context::current()->glcLoadIdentity();

	glDisable(GL_BLEND);
	m_RenderProperties.setRenderingFlag(glc::WireRenderFlag);
	// Display arcs
	m_Arc1.render(glc::WireRenderFlag);
	m_Arc2.render(glc::WireRenderFlag);
	// Display base class (Main circle)
	m_MainCircle.render(m_RenderProperties);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_RenderProperties.setRenderingFlag(glc::TransparentRenderFlag);
	// Display arcs
	m_Arc1.render(glc::TransparentRenderFlag);
	m_Arc2.render(glc::TransparentRenderFlag);
	// Display base class (Main circle)
	m_MainCircle.render(m_RenderProperties);

	GLC_Context::current()->glcPopMatrix();
	GLC_Context::current()->glcMatrixMode(GL_PROJECTION);
	GLC_Context::current()->glcPopMatrix();
	GLC_Context::current()->glcMatrixMode(GL_MODELVIEW);

}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

// Compute trackball radius
void GLC_RepTrackBallMover::computeRadius()
{
	int nRayonSph;
	const double winHSize= static_cast<double>(m_pViewport->viewHSize());
	const double winVSize= static_cast<double>(m_pViewport->viewVSize());

	if (winHSize > winVSize)
	{
		nRayonSph = static_cast<int>(m_Ratio * winVSize / 2.0);
	}
	else
	{
		nRayonSph = static_cast<int>(m_Ratio * winHSize / 2.0);
	}

	// Compute the length of camera's field of view
	const double ChampsVision = 2.0;

	// the side of camera's square is mapped on Vertical length of window
	// Circle radius in OpenGL unit = Radius(Pixel) * (dimend GL / dimens Pixel)
	const double RayonSph= fabs((static_cast<double>(nRayonSph) * ChampsVision / winVSize));

	if ((!qFuzzyCompare(RayonSph, 0.0) && !qFuzzyCompare(RayonSph - m_Radius, 0.0)) || (RayonSph < 2.0))
	{
		// Main circle radius
		m_MainCircle.setRadius(RayonSph);

		GLC_Circle* pCircle;
		// Arc 1 radius
		pCircle= static_cast<GLC_Circle*>(m_Arc1.geomAt(0));
		pCircle->setRadius(RayonSph);
		// Arc 2 radius
		pCircle= static_cast<GLC_Circle*>(m_Arc2.geomAt(0));
		pCircle->setRadius(RayonSph);
	}

}
