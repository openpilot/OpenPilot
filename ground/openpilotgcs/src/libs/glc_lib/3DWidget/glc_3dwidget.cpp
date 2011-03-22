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
//! \file glc_3dwidget.cpp Implementation of the GLC_3DWidget class.

#include "glc_3dwidget.h"


GLC_3DWidget::GLC_3DWidget(GLC_3DWidgetManagerHandle*  pWidgetManagerHandle)
: QObject()
, m_Uid(glc::GLC_Gen3DWidgetID())
, m_pWidgetManagerHandle(pWidgetManagerHandle)
, m_InstanceIdList()
{

}

GLC_3DWidget::GLC_3DWidget(const GLC_3DWidget& widget)
: QObject()
, m_Uid(glc::GLC_Gen3DWidgetID())
, m_pWidgetManagerHandle(widget.m_pWidgetManagerHandle)
, m_InstanceIdList()
{
	// Copy the 3Dview instance of the widget
	const int size= widget.m_InstanceIdList.size();
	for (int i= 0; i < size; ++i)
	{
		GLC_3DViewInstance newInstance(widget.m_pWidgetManagerHandle->instanceHandle(widget.m_InstanceIdList.at(i))->deepCopy());
		GLC_uint newId= newInstance.id();
		m_InstanceIdList.append(newId);
		m_pWidgetManagerHandle->add3DViewInstance(newInstance, m_Uid);
	}
}

GLC_3DWidget::~GLC_3DWidget()
{
	remove3DViewInstance();
}

bool GLC_3DWidget::operator==(const GLC_3DWidget& widget) const
{
	return this == &widget;
}

GLC_3DWidget& GLC_3DWidget::operator=(const GLC_3DWidget& widget)
{
	if (this != &widget)
	{
		remove3DViewInstance();

		m_Uid= widget.m_Uid;
		m_pWidgetManagerHandle= widget.m_pWidgetManagerHandle;
		m_InstanceIdList= widget.m_InstanceIdList;

		// Copy the 3Dview instance of the widget
		const int size= widget.m_InstanceIdList.size();
		for (int i= 0; i < size; ++i)
		{
			GLC_3DViewInstance newInstance(widget.m_pWidgetManagerHandle->instanceHandle(widget.m_InstanceIdList.at(i))->deepCopy());
			GLC_uint newId= newInstance.id();
			m_InstanceIdList.append(newId);
			m_pWidgetManagerHandle->add3DViewInstance(newInstance, m_Uid);
		}
	}
	return *this;
}

void GLC_3DWidget::setWidgetManager(GLC_3DWidgetManagerHandle* pWidgetManagerHandle)
{
	if (NULL != m_pWidgetManagerHandle)
	{
		m_pWidgetManagerHandle->take(m_Uid);
		remove3DViewInstance();
	}
	m_pWidgetManagerHandle= pWidgetManagerHandle;

	create3DviewInstance();
}

void GLC_3DWidget::setVisible(bool visible)
{
	if (NULL != m_pWidgetManagerHandle)
	{
		const int instanceCount= m_InstanceIdList.size();
		for (int i= 0; i < instanceCount; ++i)
		{
			m_pWidgetManagerHandle->instanceHandle(m_InstanceIdList.at(i))->setVisibility(visible);
		}
		resetViewState();
	}
}
//////////////////////////////////////////////////////////////////////
// Interaction Functions
//////////////////////////////////////////////////////////////////////
glc::WidgetEventFlag GLC_3DWidget::select(const GLC_Point3d&, GLC_uint)
{
	return glc::IgnoreEvent;
}

glc::WidgetEventFlag GLC_3DWidget::unselect(const GLC_Point3d&, GLC_uint)
{
	return glc::IgnoreEvent;
}

glc::WidgetEventFlag GLC_3DWidget::mouseOver(const GLC_Point3d&, GLC_uint)
{
	return glc::IgnoreEvent;
}

glc::WidgetEventFlag GLC_3DWidget::mousePressed(const GLC_Point3d&, Qt::MouseButton, GLC_uint)
{
	return glc::IgnoreEvent;
}

glc::WidgetEventFlag GLC_3DWidget::mouseReleased(Qt::MouseButton)
{
	return glc::IgnoreEvent;
}

glc::WidgetEventFlag GLC_3DWidget::mouseMove(const GLC_Point3d&, Qt::MouseButtons, GLC_uint)
{
	return glc::IgnoreEvent;
}


//////////////////////////////////////////////////////////////////////
// Protected services functions
//////////////////////////////////////////////////////////////////////
void GLC_3DWidget::add3DViewInstance(const GLC_3DViewInstance& instance)
{
	m_pWidgetManagerHandle->add3DViewInstance(instance, m_Uid);
	const GLC_uint instanceId= instance.id();
	m_InstanceIdList.append(instanceId);
}

void GLC_3DWidget::remove3DViewInstance()
{
	if (NULL != m_pWidgetManagerHandle)
	{
		const int size= m_InstanceIdList.size();
		for (int i= 0; i < size; ++i)
		{
			m_pWidgetManagerHandle->remove3DViewInstance(m_InstanceIdList.at(i));
		}
	}
}

void GLC_3DWidget::set3DViewInstanceVisibility(int index, bool visibility)
{
	m_pWidgetManagerHandle->instanceHandle(m_InstanceIdList[index])->setVisibility(visibility);
}


