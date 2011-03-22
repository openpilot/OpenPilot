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
//! \file glc_cuttingplane.cpp Implementation of the GLC_CuttingPlane class.

#include "glc_cuttingplane.h"
#include "../glc_factory.h"
#include "../maths/glc_line3d.h"
#include "../maths/glc_geomtools.h"
#include "../geometry/glc_arrow.h"
#include "../geometry/glc_disc.h"
#include "glc_pullmanipulator.h"
#include "glc_rotationmanipulator.h"

GLC_CuttingPlane::GLC_CuttingPlane(const GLC_Point3d& center, const GLC_Vector3d& normal, double l1, double l2, GLC_3DWidgetManagerHandle*  pWidgetManagerHandle)
: GLC_3DWidget(pWidgetManagerHandle)
, m_Center(center)
, m_Normal(normal)
, m_CompMatrix()
, m_L1(l1)
, m_L2(l2)
, m_Color(Qt::darkGreen)
, m_Opacity(0.3)
, m_ManipulatorOffsetFactor(1.0)
, m_ScaleFactor(1.0)
, m_SelectionIndex(-1)
, m_CurrentManipulator(TranslationManipulator)
, m_pCurrentManipulator(NULL)
, m_CurrentNavigatorPosition()
{
	if (NULL != pWidgetManagerHandle)
	{
		create3DviewInstance();
	}

	if (glc::Z_AXIS != m_Normal)
	{
		if (m_Normal != -glc::Z_AXIS)
		{
			m_CompMatrix.setMatRot(glc::Z_AXIS, m_Normal);
		}
		else
		{
			m_CompMatrix.setMatRot(glc::X_AXIS, glc::PI);
		}
	}

}

GLC_CuttingPlane::GLC_CuttingPlane(const GLC_CuttingPlane& cuttingPlane)
: GLC_3DWidget(cuttingPlane)
, m_Center(cuttingPlane.m_Center)
, m_Normal(cuttingPlane.m_Normal)
, m_CompMatrix(cuttingPlane.m_CompMatrix)
, m_L1(cuttingPlane.m_L1)
, m_L2(cuttingPlane.m_L2)
, m_Color(cuttingPlane.m_Color)
, m_Opacity(cuttingPlane.m_Opacity)
, m_ManipulatorOffsetFactor(cuttingPlane.m_ManipulatorOffsetFactor)
, m_ScaleFactor(cuttingPlane.m_ScaleFactor)
, m_SelectionIndex(cuttingPlane.m_SelectionIndex)
, m_CurrentManipulator(cuttingPlane.m_CurrentManipulator)
, m_pCurrentManipulator(NULL)
, m_CurrentNavigatorPosition(cuttingPlane.m_CurrentNavigatorPosition)
{
	if (NULL != cuttingPlane.m_pCurrentManipulator)
	{
		m_pCurrentManipulator= cuttingPlane.m_pCurrentManipulator->clone();
	}
}

GLC_CuttingPlane::~GLC_CuttingPlane()
{
	delete m_pCurrentManipulator;
}

GLC_CuttingPlane& GLC_CuttingPlane::operator=(const GLC_CuttingPlane& cuttingPlane)
{
	GLC_3DWidget::operator=(cuttingPlane);

	m_Center= cuttingPlane.m_Center;
	m_Normal= cuttingPlane.m_Normal;
	m_CompMatrix= cuttingPlane.m_CompMatrix;
	m_L1= cuttingPlane.m_L1;
	m_L2= cuttingPlane.m_L2;
	m_Color= cuttingPlane.m_Color;
	m_Opacity= cuttingPlane.m_Opacity;
	m_CurrentNavigatorPosition= cuttingPlane.m_CurrentNavigatorPosition;
	delete m_pCurrentManipulator;
	if (NULL != cuttingPlane.m_pCurrentManipulator)
	{
		m_pCurrentManipulator= cuttingPlane.m_pCurrentManipulator->clone();
	}

	return *this;
}

void GLC_CuttingPlane::updateLength(double l1, double l2)
{
	m_L1= l1;
	m_L2= l2;

	if (GLC_3DWidget::has3DWidgetManager())
	{
		GLC_3DWidget::remove3DViewInstance();
		create3DviewInstance();
	}
}

void GLC_CuttingPlane::updateWidgetRep()
{
	const double viewTangent= GLC_3DWidget::widgetManagerHandle()->viewportTangent();
	const GLC_Point3d eye(GLC_3DWidget::widgetManagerHandle()->cameraHandle()->eye());
	const double distanceToNormal= (m_CurrentNavigatorPosition - eye).length();
	const double viewWidth= distanceToNormal * viewTangent;

	m_ScaleFactor= viewWidth * 0.1;
	m_ManipulatorOffsetFactor= m_ScaleFactor * (-0.01);

	moveManipulatorRep(m_CurrentNavigatorPosition);
}

glc::WidgetEventFlag GLC_CuttingPlane::select(const GLC_Point3d& pos, GLC_uint)
{
	Q_ASSERT(NULL == m_pCurrentManipulator);
	Q_ASSERT(TranslationManipulator == m_CurrentManipulator);

	//! Create the default manipulator
	GLC_Viewport* pViewport= GLC_3DWidget::widgetManagerHandle()->viewport();
	m_pCurrentManipulator= new GLC_PullManipulator(pViewport, m_Normal);

	m_pCurrentManipulator->enterManipulateState(pos);
	m_CurrentNavigatorPosition= pos;

	GLC_3DWidget::set3DViewInstanceVisibility(1, true);

	updateWidgetRep();

	return glc::BlockedEvent;
}

glc::WidgetEventFlag GLC_CuttingPlane::mousePressed(const GLC_Point3d& pos, Qt::MouseButton button, GLC_uint id)
{
	glc::WidgetEventFlag returnFlag= glc::IgnoreEvent;
	if (button == Qt::LeftButton)
	{
		const int selectedInstanceIndex= GLC_3DWidget::indexOfIntsanceId(id);
		if (selectedInstanceIndex > 0)
		{
			m_SelectionIndex= selectedInstanceIndex;
			if (m_CurrentManipulator == RotationManipulator)
			{
				delete m_pCurrentManipulator;
				m_pCurrentManipulator= rotationNavigator(selectedInstanceIndex);
			}
			m_pCurrentManipulator->enterManipulateState(pos);
		}
		else
		{
			if (NULL != m_pCurrentManipulator)
			{
				if (m_CurrentManipulator == RotationManipulator)
				{
					delete m_pCurrentManipulator;
					m_pCurrentManipulator= NULL;
				}
				else
				{
					m_pCurrentManipulator->enterManipulateState(pos);
				}

			}
			m_CurrentNavigatorPosition= pos;
			updateWidgetRep();
		}

		returnFlag= glc::BlockedEvent;
	}

	return returnFlag;
}

glc::WidgetEventFlag GLC_CuttingPlane::mouseReleased(Qt::MouseButton button)
{
	glc::WidgetEventFlag returnFlag= glc::IgnoreEvent;
	if ((button == Qt::LeftButton) && (m_SelectionIndex != -1))
	{

		// get selected instance index

		if (m_CurrentManipulator == TranslationManipulator)
		{
			GLC_3DWidget::set3DViewInstanceVisibility(1, false);
			GLC_3DWidget::set3DViewInstanceVisibility(2, true);
			GLC_3DWidget::set3DViewInstanceVisibility(3, true);
			GLC_3DWidget::set3DViewInstanceVisibility(4, true);

			returnFlag= glc::BlockedEvent;
			m_CurrentManipulator= RotationManipulator;
			delete m_pCurrentManipulator;
			m_pCurrentManipulator= NULL;

			moveManipulatorRep(m_CurrentNavigatorPosition);
		}
		else if (m_CurrentManipulator == RotationManipulator)
		{
			GLC_3DWidget::set3DViewInstanceVisibility(1, true);
			GLC_3DWidget::set3DViewInstanceVisibility(2, false);
			GLC_3DWidget::set3DViewInstanceVisibility(3, false);
			GLC_3DWidget::set3DViewInstanceVisibility(4, false);

			returnFlag= glc::BlockedEvent;
			m_CurrentManipulator= TranslationManipulator;

			delete m_pCurrentManipulator;

			GLC_Viewport* pViewport= GLC_3DWidget::widgetManagerHandle()->viewport();
			m_pCurrentManipulator= new GLC_PullManipulator(pViewport, m_Normal);
			m_pCurrentManipulator->enterManipulateState(m_CurrentNavigatorPosition);

			moveManipulatorRep(m_CurrentNavigatorPosition);
		}
		m_SelectionIndex= -1;
	}
	return returnFlag;
}

glc::WidgetEventFlag GLC_CuttingPlane::unselect(const GLC_Point3d&, GLC_uint)
{
	resetViewState();
	return glc::AcceptEvent;
}

glc::WidgetEventFlag GLC_CuttingPlane::mouseMove(const GLC_Point3d& pos, Qt::MouseButtons button, GLC_uint)
{
	glc::WidgetEventFlag returnFlag= glc::IgnoreEvent;
	if (button & Qt::LeftButton)
	{
		if (NULL != m_pCurrentManipulator)
		{
			if (m_SelectionIndex != -1)
			{
				moveManipulatorRep(m_CurrentNavigatorPosition);
				m_SelectionIndex= -1;
			}
			GLC_Matrix4x4 moveMatrix(m_pCurrentManipulator->manipulate(pos));

			// Update plane normal
			if (m_CurrentManipulator == RotationManipulator)
			{
				m_Normal= moveMatrix.rotationMatrix() * m_Normal;
			}
			m_CompMatrix= moveMatrix * m_CompMatrix;
			m_Center= moveMatrix * m_Center;
			m_CurrentNavigatorPosition= moveMatrix * m_CurrentNavigatorPosition;

			// Update the instance
			for (int i= 0; i < 5; ++i)
			{
				GLC_3DWidget::instanceHandle(i)->multMatrix(moveMatrix);
			}

			// Plane throw intersection and plane normal and camera up vector
			m_pCurrentManipulator->enterManipulateState(m_pCurrentManipulator->previousPosition());

			emit asChanged();
			returnFlag= glc::AcceptEvent;
		}
	}

	return returnFlag;
}

void GLC_CuttingPlane::create3DviewInstance()
{
	Q_ASSERT(GLC_3DWidget::isEmpty());
	// The cutting plane material
	GLC_Material* pMaterial= new GLC_Material(m_Color);
	pMaterial->setOpacity(m_Opacity);

	// Cutting plane 3Dview instance
	GLC_3DViewInstance cuttingPlaneInstance= GLC_Factory::instance()->createCuttingPlane(m_Center, m_Normal, m_L1, m_L2, pMaterial);
	GLC_3DWidget::add3DViewInstance(cuttingPlaneInstance);

	// Normal arrow geometry
	GLC_Arrow* pArrow= new GLC_Arrow(GLC_Point3d(), -glc::Z_AXIS, GLC_3DWidget::widgetManagerHandle()->cameraHandle()->forward().normalize());
	pArrow->setLineWidth(4.5);
	pArrow->setHeadLength(0.15);
	QColor arrowColor(Qt::red);
	arrowColor.setAlphaF(0.4);
	pArrow->setWireColor(arrowColor);

	//Base arrow disc
	pMaterial= new GLC_Material(Qt::red);
	pMaterial->setOpacity(m_Opacity);
	GLC_Disc* pDisc= new GLC_Disc(0.3);
	pDisc->replaceMasterMaterial(pMaterial);

	// Normal arrow + base instance
	GLC_3DRep normalLine(pArrow);
	normalLine.addGeom(pDisc);
	GLC_3DWidget::add3DViewInstance(GLC_3DViewInstance(normalLine));
	GLC_3DWidget::set3DViewInstanceVisibility(1, false);

	// Rotation manipulator
	const double initRadius= 1;
	// Arrond X axis
	pDisc= new GLC_Disc(initRadius);
	pMaterial= new GLC_Material(Qt::red);
	pMaterial->setOpacity(m_Opacity);
	pDisc->replaceMasterMaterial(pMaterial);
	pDisc->setAngle(glc::PI);
	GLC_3DWidget::add3DViewInstance(GLC_3DViewInstance(pDisc));
	GLC_3DWidget::set3DViewInstanceVisibility(2, false);
	// Arround Y axis
	pDisc= new GLC_Disc(initRadius);
	pMaterial= new GLC_Material(Qt::green);
	pMaterial->setOpacity(m_Opacity);
	pDisc->replaceMasterMaterial(pMaterial);
	pDisc->setAngle(glc::PI);
	GLC_3DWidget::add3DViewInstance(GLC_3DViewInstance(pDisc));
	GLC_3DWidget::set3DViewInstanceVisibility(3, false);
	// Arround Z axis
	pDisc= new GLC_Disc(initRadius);
	pMaterial= new GLC_Material(Qt::blue);
	pMaterial->setOpacity(m_Opacity);
	pDisc->replaceMasterMaterial(pMaterial);
	//pDisc->setAngle(glc::PI / 2.0);
	GLC_3DWidget::add3DViewInstance(GLC_3DViewInstance(pDisc));
	GLC_3DWidget::set3DViewInstanceVisibility(4, false);
}

void GLC_CuttingPlane::resetViewState()
{
	Q_ASSERT(m_SelectionIndex == -1);
	for (int i= 0; i < 4; ++i)
	{
		GLC_3DWidget::set3DViewInstanceVisibility(1 + i, false);
	}
	m_pCurrentManipulator= NULL;

	m_CurrentManipulator= TranslationManipulator;
}

void GLC_CuttingPlane::moveManipulatorRep(const GLC_Point3d& pos)
{
	// Create the widget rotation matrix
	const GLC_Matrix4x4 rotationMatrix(m_CompMatrix.rotationMatrix());

	const GLC_Matrix4x4 translationMatrix(pos);
	const GLC_Matrix4x4 offsetMatrix(m_Normal * m_ManipulatorOffsetFactor);
	GLC_Matrix4x4 scaleMatrix;
	scaleMatrix.setMatScaling(m_ScaleFactor, m_ScaleFactor, m_ScaleFactor);
	GLC_3DWidget::instanceHandle(1)->setMatrix(offsetMatrix * translationMatrix * rotationMatrix *scaleMatrix);

	// Rotation manipulator
	QVector<GLC_Matrix4x4> rotations(3);
	rotations[0].setMatRot(glc::Y_AXIS, glc::PI / 2.0); // X
	rotations[0]= GLC_Matrix4x4(glc::X_AXIS, -glc::PI / 2.0) * rotations[0];
	rotations[1].setMatRot(glc::X_AXIS, -glc::PI / 2.0); // Y
	// Z
	for (int i= 0; i < 3; ++i)
	{
		GLC_3DWidget::instanceHandle(2 + i)->setMatrix(offsetMatrix * translationMatrix * rotationMatrix * rotations.at(i) * scaleMatrix);
	}

	GLC_Arrow* pArrow= dynamic_cast<GLC_Arrow*>(GLC_3DWidget::instanceHandle(1)->geomAt(0));
	Q_ASSERT(NULL != pArrow);

	pArrow->setViewDir(rotationMatrix * GLC_3DWidget::widgetManagerHandle()->cameraHandle()->forward().normalize());
}

GLC_AbstractManipulator* GLC_CuttingPlane::rotationNavigator(int index)
{
	index= index - 2;
	Q_ASSERT((index > -1) && (index < 3));

	const GLC_Matrix4x4 rotationMatrix(m_CompMatrix.rotationMatrix());
	GLC_Vector3d axis;
	if (index == 0)
	{
		axis= rotationMatrix * glc::X_AXIS;
	}
	else if (index == 1)
	{
		axis= rotationMatrix * glc::Y_AXIS;
	}
	else
	{
		axis= rotationMatrix * glc::Z_AXIS;
	}
	GLC_AbstractManipulator* pManipulator= new GLC_RotationManipulator(GLC_3DWidget::widgetManagerHandle()->viewport(), GLC_Line3d(m_CurrentNavigatorPosition, axis));

	return pManipulator;
}

