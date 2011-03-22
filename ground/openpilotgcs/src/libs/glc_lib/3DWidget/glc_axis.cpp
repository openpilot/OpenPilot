/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 *****************************************************************************/
//! \file glc_axis.cpp Implementation of the GLC_Axis class.

#include "glc_axis.h"
#include "../glc_factory.h"
#include "../maths/glc_line3d.h"
#include "../maths/glc_geomtools.h"
#include "../geometry/glc_cylinder.h"
#include "../geometry/glc_cone.h"
#include "glc_pullmanipulator.h"

GLC_Axis::GLC_Axis(const GLC_Point3d& center, GLC_3DWidgetManagerHandle*  pWidgetManagerHandle)
: GLC_3DWidget(pWidgetManagerHandle)
, m_Center(center)
, m_ScaleFactor(1.0)
, m_CurrentManipulator(NoneManipulator)
, m_pCurrentManipulator(NULL)
, m_AxisLength(1.0)
, m_AxisRadiusRatio(0.03)
{

}

GLC_Axis::GLC_Axis(const GLC_Axis& axis)
: GLC_3DWidget(axis)
, m_Center(axis.m_Center)
, m_ScaleFactor(axis.m_ScaleFactor)
, m_CurrentManipulator(axis.m_CurrentManipulator)
, m_pCurrentManipulator(NULL)
, m_AxisLength(axis.m_AxisLength)
, m_AxisRadiusRatio(axis.m_AxisRadiusRatio)
{
	if (NULL != axis.m_pCurrentManipulator)
	{
		m_pCurrentManipulator= axis.m_pCurrentManipulator->clone();
	}
}

GLC_Axis::~GLC_Axis()
{
	delete m_pCurrentManipulator;
}

GLC_Axis& GLC_Axis::operator=(const GLC_Axis& axis)
{
	GLC_3DWidget::operator=(axis);

	m_Center= axis.m_Center;
	delete m_pCurrentManipulator;
	if (NULL != axis.m_pCurrentManipulator)
	{
		m_pCurrentManipulator= axis.m_pCurrentManipulator->clone();
	}

	return *this;
}

void GLC_Axis::updateWidgetRep()
{
	const double viewTangent= GLC_3DWidget::widgetManagerHandle()->viewportTangent();
	const GLC_Point3d eye(GLC_3DWidget::widgetManagerHandle()->cameraHandle()->eye());
	const double distanceToNormal= (m_Center - eye).length();
	const double viewWidth= distanceToNormal * viewTangent;

	m_ScaleFactor= viewWidth * 0.1;
	moveManipulatorRep(m_Center);
}

void GLC_Axis::setAxisLength(double length)
{
	m_AxisLength= length;
	if (!GLC_3DWidget::isEmpty())
	{
		GLC_3DWidget::remove3DViewInstance();
		create3DviewInstance();
	}
}

void GLC_Axis::setCenter(const GLC_Point3d& newCenter)
{
	m_Center= newCenter;
	moveManipulatorRep(m_Center);
}

glc::WidgetEventFlag GLC_Axis::select(const GLC_Point3d& pos, GLC_uint id)
{
	//qDebug() << "GLC_Axis::select";
	Q_ASSERT(NULL == m_pCurrentManipulator);
	Q_ASSERT(NoneManipulator == m_CurrentManipulator);

	const int selectedInstanceIndex= GLC_3DWidget::indexOfIntsanceId(id);
	//! Create the corresponding manipulator
	GLC_Viewport* pViewport= GLC_3DWidget::widgetManagerHandle()->viewport();
	if (selectedInstanceIndex < 2)
	{
		m_pCurrentManipulator= new GLC_PullManipulator(pViewport, glc::X_AXIS);
		m_CurrentManipulator= X_AxisManipulator;
		GLC_3DWidget::instanceHandle(0)->geomAt(0)->firstMaterial()->setDiffuseColor(Qt::yellow);
	}
	else if (selectedInstanceIndex < 4)
	{
		m_pCurrentManipulator= new GLC_PullManipulator(pViewport, glc::Y_AXIS);
		m_CurrentManipulator= Y_AxisManipulator;
		GLC_3DWidget::instanceHandle(2)->geomAt(0)->firstMaterial()->setDiffuseColor(Qt::yellow);
	}
	else
	{
		Q_ASSERT((selectedInstanceIndex < 6) && (selectedInstanceIndex >= 4));
		m_pCurrentManipulator= new GLC_PullManipulator(pViewport, glc::Z_AXIS);
		m_CurrentManipulator= Z_AxisManipulator;
		GLC_3DWidget::instanceHandle(4)->geomAt(0)->firstMaterial()->setDiffuseColor(Qt::yellow);
	}

	m_pCurrentManipulator->enterManipulateState(pos);

	updateWidgetRep();

	return glc::BlockedEvent;
}

glc::WidgetEventFlag GLC_Axis::mousePressed(const GLC_Point3d& pos, Qt::MouseButton button, GLC_uint id)
{
	//qDebug() << "GLC_Axis::mousePressed";
	glc::WidgetEventFlag returnFlag= glc::IgnoreEvent;
	if (button == Qt::LeftButton)
	{
		returnFlag= select(pos, id);
	}

	return returnFlag;
}

glc::WidgetEventFlag GLC_Axis::mouseReleased(Qt::MouseButton button)
{
	//qDebug() << "GLC_Axis::mouseReleased";
	glc::WidgetEventFlag returnFlag= glc::IgnoreEvent;
	if (button == Qt::LeftButton)
	{

		// get selected instance index

		if (m_CurrentManipulator == X_AxisManipulator)
		{
			GLC_3DWidget::instanceHandle(0)->geomAt(0)->firstMaterial()->setDiffuseColor(Qt::red);
		}
		else if (m_CurrentManipulator == Y_AxisManipulator)
		{
			GLC_3DWidget::instanceHandle(2)->geomAt(0)->firstMaterial()->setDiffuseColor(Qt::green);
		}
		else if (m_CurrentManipulator == Z_AxisManipulator)
		{
			GLC_3DWidget::instanceHandle(4)->geomAt(0)->firstMaterial()->setDiffuseColor(Qt::blue);
		}

		m_CurrentManipulator= NoneManipulator;
		delete m_pCurrentManipulator;
		m_pCurrentManipulator= NULL;

		returnFlag= glc::BlockedEvent;
	}
	return returnFlag;
}

glc::WidgetEventFlag GLC_Axis::unselect(const GLC_Point3d&, GLC_uint)
{
	//qDebug() << "GLC_Axis::unselect";
	delete m_pCurrentManipulator;
	m_pCurrentManipulator= NULL;

	m_CurrentManipulator= NoneManipulator;

	return glc::AcceptEvent;
}

glc::WidgetEventFlag GLC_Axis::mouseMove(const GLC_Point3d& pos, Qt::MouseButtons button, GLC_uint)
{
	glc::WidgetEventFlag returnFlag= glc::IgnoreEvent;
	if (button & Qt::LeftButton)
	{
		if (NULL != m_pCurrentManipulator)
		{
			GLC_Matrix4x4 moveMatrix(m_pCurrentManipulator->manipulate(pos));
			m_Center= moveMatrix * m_Center;
			// Update the instance
			for (int i= 0; i < 6; ++i)
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
void GLC_Axis::create3DviewInstance()
{
	Q_ASSERT(GLC_3DWidget::isEmpty());
	const double axisRadius= m_AxisLength * m_AxisRadiusRatio;
	const double arrowLength= m_AxisLength * 0.3;
	const double arrowFactor= 2.5;

	{ // Create X axis
		// The X axis material
		GLC_Material* pMaterial= new GLC_Material(Qt::red);

		GLC_3DViewInstance axisInstance= GLC_Factory::instance()->createCylinder(axisRadius, m_AxisLength);
		axisInstance.representation().geomAt(0)->replaceMasterMaterial(pMaterial);
		GLC_3DViewInstance arrowInstance= GLC_Factory::instance()->createCone(arrowFactor * axisRadius, arrowLength);
		arrowInstance.representation().geomAt(0)->replaceMasterMaterial(pMaterial);
		arrowInstance.translate(0.0, 0.0, m_AxisLength);
		// Rotate the axis
		GLC_Matrix4x4 rotation(glc::Y_AXIS, glc::PI / 2);
		axisInstance.multMatrix(rotation);
		arrowInstance.multMatrix(rotation);

		GLC_3DWidget::add3DViewInstance(axisInstance);
		GLC_3DWidget::add3DViewInstance(arrowInstance);
	}
	{ // Create Y axis
		// The Y axis material
		GLC_Material* pMaterial= new GLC_Material(Qt::green);

		GLC_3DViewInstance axisInstance= GLC_Factory::instance()->createCylinder(axisRadius, m_AxisLength);
		axisInstance.representation().geomAt(0)->replaceMasterMaterial(pMaterial);
		GLC_3DViewInstance arrowInstance= GLC_Factory::instance()->createCone(arrowFactor * axisRadius, arrowLength);
		arrowInstance.representation().geomAt(0)->replaceMasterMaterial(pMaterial);
		arrowInstance.translate(0.0, 0.0, m_AxisLength);
		// Rotate the axis
		GLC_Matrix4x4 rotation(glc::X_AXIS, - glc::PI / 2);
		axisInstance.multMatrix(rotation);
		arrowInstance.multMatrix(rotation);

		GLC_3DWidget::add3DViewInstance(axisInstance);
		GLC_3DWidget::add3DViewInstance(arrowInstance);
	}
	{ // Create Z axis
		// The Z axis material
		GLC_Material* pMaterial= new GLC_Material(Qt::blue);

		GLC_3DViewInstance axisInstance= GLC_Factory::instance()->createCylinder(axisRadius, m_AxisLength);
		axisInstance.representation().geomAt(0)->replaceMasterMaterial(pMaterial);
		GLC_3DViewInstance arrowInstance= GLC_Factory::instance()->createCone(arrowFactor * axisRadius, arrowLength);
		arrowInstance.representation().geomAt(0)->replaceMasterMaterial(pMaterial);
		arrowInstance.translate(0.0, 0.0, m_AxisLength);

		GLC_3DWidget::add3DViewInstance(axisInstance);
		GLC_3DWidget::add3DViewInstance(arrowInstance);
	}
}

void GLC_Axis::resetViewState()
{

}

void GLC_Axis::moveManipulatorRep(const GLC_Point3d& pos)
{

	const GLC_Matrix4x4 translationMatrix(pos);
	GLC_Matrix4x4 scaleMatrix;
	scaleMatrix.setMatScaling(m_ScaleFactor, m_ScaleFactor, m_ScaleFactor);
	for (int i= 0; i < 6; ++i)
	{
		GLC_Matrix4x4 rotationMatrix;
		GLC_Matrix4x4 intTranslation;
		if (i < 2)
		{
			rotationMatrix.setMatRot(glc::Y_AXIS, glc::PI / 2);
			if (i == 1)
			{
				intTranslation.setMatTranslate(0.0, 0.0, m_AxisLength);
			}
		}
		else if (i < 4)
		{
			rotationMatrix.setMatRot(glc::X_AXIS, - glc::PI / 2);
			if (i == 3)
			{
				intTranslation.setMatTranslate(0.0, 0.0, m_AxisLength);
			}

		}
		else if (i == 5)
		{
			intTranslation.setMatTranslate(0.0, 0.0, m_AxisLength);
		}

		GLC_3DWidget::instanceHandle(i)->setMatrix(translationMatrix * scaleMatrix * rotationMatrix * intTranslation);
	}
}
