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
//! \file glc_3dwidgetmanagerhandle.cpp Implementation of the GLC_3DWidgetManagerHandle class.

#include "glc_3dwidgetmanagerhandle.h"

#include "../viewport/glc_viewport.h"
#include "../sceneGraph/glc_3dviewinstance.h"
#include "glc_3dwidget.h"
#include <QMouseEvent>

GLC_3DWidgetManagerHandle::GLC_3DWidgetManagerHandle(GLC_Viewport* pViewport)
: m_Collection()
, m_Count(1)
, m_3DWidgetHash()
, m_MapBetweenInstanceWidget()
, m_pViewport(pViewport)
, m_Active3DWidgetId(0)
, m_Preselected3DWidgetId(0)
{

}

GLC_3DWidgetManagerHandle::~GLC_3DWidgetManagerHandle()
{
	QHash<GLC_uint, GLC_3DWidget*>::iterator iWidget= m_3DWidgetHash.begin();
	while (m_3DWidgetHash.constEnd() != iWidget)
	{
		delete iWidget.value();
		++iWidget;
	}
}

void GLC_3DWidgetManagerHandle::add3DWidget(GLC_3DWidget* p3DWidget)
{
	Q_ASSERT(!m_MapBetweenInstanceWidget.contains(p3DWidget->id()));
	m_3DWidgetHash.insert(p3DWidget->id(), p3DWidget);
	p3DWidget->setWidgetManager(this);
}

void GLC_3DWidgetManagerHandle::remove3DWidget(GLC_uint id)
{
	Q_ASSERT(m_3DWidgetHash.contains(id));
	delete m_3DWidgetHash.take(id);
	if (m_Active3DWidgetId == id) m_Active3DWidgetId= 0;

}

GLC_3DWidget* GLC_3DWidgetManagerHandle::take(GLC_uint id)
{
	Q_ASSERT(m_3DWidgetHash.contains(id));
	return m_3DWidgetHash.take(id);
}

void GLC_3DWidgetManagerHandle::add3DViewInstance(const GLC_3DViewInstance& instance, GLC_uint widgetId)
{
	const GLC_uint instanceId= instance.id();
	Q_ASSERT(!m_MapBetweenInstanceWidget.contains(instanceId));
	Q_ASSERT(!m_Collection.contains(instanceId));

	m_MapBetweenInstanceWidget.insert(instanceId, widgetId);
	m_Collection.add(instance, 0);
}

void GLC_3DWidgetManagerHandle::remove3DViewInstance(GLC_uint id)
{
	Q_ASSERT(m_MapBetweenInstanceWidget.contains(id));
	Q_ASSERT(m_Collection.contains(id));
	m_Collection.remove(id);
	m_MapBetweenInstanceWidget.remove(id);
}

void GLC_3DWidgetManagerHandle::clear()
{
	m_Active3DWidgetId= 0;
	QHash<GLC_uint, GLC_3DWidget*>::iterator iWidget= m_3DWidgetHash.begin();
	while (m_3DWidgetHash.constEnd() != iWidget)
	{
		delete iWidget.value();
		++iWidget;
	}
	m_3DWidgetHash.clear();
	m_Collection.clear();
	m_MapBetweenInstanceWidget.clear();
}

void GLC_3DWidgetManagerHandle::setWidgetVisible(GLC_uint id, bool visible)
{
	if (id == m_Active3DWidgetId) m_Active3DWidgetId= 0;
	Q_ASSERT(m_3DWidgetHash.contains(id));
	m_3DWidgetHash.value(id)->setVisible(visible);
}

glc::WidgetEventFlag GLC_3DWidgetManagerHandle::mouseDoubleClickEvent(QMouseEvent *)
{

	if (hasAnActiveWidget())
	{

	}
	return glc::IgnoreEvent;
}

glc::WidgetEventFlag GLC_3DWidgetManagerHandle::mouseMoveEvent(QMouseEvent * pEvent)
{
	glc::WidgetEventFlag eventFlag= glc::IgnoreEvent;
	// Get the 3D cursor position and the id under
	QPair<GLC_uint, GLC_Point3d> cursorInfo= select(pEvent);
	const GLC_uint selectedId= cursorInfo.first;
	const GLC_Point3d pos(cursorInfo.second);

	if (hasAnActiveWidget())
	{
		GLC_3DWidget* pActiveWidget= m_3DWidgetHash.value(m_Active3DWidgetId);
		eventFlag= pActiveWidget->mouseMove(pos, pEvent->buttons(), selectedId);
	}
	else
	{
		if (m_MapBetweenInstanceWidget.contains(selectedId))
		{
			const GLC_uint select3DWidgetId= m_MapBetweenInstanceWidget.value(selectedId);

			if (m_Preselected3DWidgetId != select3DWidgetId)
			{
				m_Preselected3DWidgetId= m_MapBetweenInstanceWidget.value(selectedId);
				GLC_3DWidget* pActiveWidget= m_3DWidgetHash.value(m_Preselected3DWidgetId);
				eventFlag= pActiveWidget->mouseOver(pos, selectedId);
			}
			else if (0 != m_Preselected3DWidgetId && (m_Preselected3DWidgetId != select3DWidgetId))
			{
				eventFlag= m_3DWidgetHash.value(m_Preselected3DWidgetId)->unselect(pos, selectedId);
			}

		}
		else if (0 != m_Preselected3DWidgetId)
		{
			eventFlag= m_3DWidgetHash.value(m_Preselected3DWidgetId)->unselect(pos, selectedId);
			m_Preselected3DWidgetId= 0;
		}
	}
	return eventFlag;
}

glc::WidgetEventFlag GLC_3DWidgetManagerHandle::mousePressEvent(QMouseEvent * pEvent)
{
	glc::WidgetEventFlag eventFlag= glc::IgnoreEvent;

	if (pEvent->button() == Qt::LeftButton)
	{
		// Get the 3D cursor position and the id under
		QPair<GLC_uint, GLC_Point3d> cursorInfo= select(pEvent);
		const GLC_uint selectedId= cursorInfo.first;
		const GLC_Point3d pos(cursorInfo.second);

		if (hasAnActiveWidget())
		{
			GLC_3DWidget* pActiveWidget= m_3DWidgetHash.value(m_Active3DWidgetId);
			const bool activeWidgetUnderMouse= pActiveWidget->instanceBelongTo(selectedId);
			if (activeWidgetUnderMouse)
			{
				eventFlag= pActiveWidget->mousePressed(pos, pEvent->button(), selectedId);
			}
			else
			{
				eventFlag= pActiveWidget->unselect(pos, selectedId);
				if (m_MapBetweenInstanceWidget.contains(selectedId))
				{
					m_Active3DWidgetId= m_MapBetweenInstanceWidget.value(selectedId);
					pActiveWidget= m_3DWidgetHash.value(m_Active3DWidgetId);
					eventFlag= pActiveWidget->select(pos, selectedId);
				}
				else
				{
					m_Active3DWidgetId= 0;
				}
			}

		}
		else
		{
			if (m_MapBetweenInstanceWidget.contains(selectedId))
			{
				m_Active3DWidgetId= m_MapBetweenInstanceWidget.value(selectedId);
				GLC_3DWidget* pActiveWidget= m_3DWidgetHash.value(m_Active3DWidgetId);
				eventFlag= pActiveWidget->select(pos, selectedId);
			}
		}
	}

	return eventFlag;
}

glc::WidgetEventFlag GLC_3DWidgetManagerHandle::mouseReleaseEvent(QMouseEvent * pEvent)
{
	glc::WidgetEventFlag eventFlag= glc::IgnoreEvent;
	if (hasAnActiveWidget() && (pEvent->button() == Qt::LeftButton))
	{

		GLC_3DWidget* pActiveWidget= m_3DWidgetHash.value(m_Active3DWidgetId);

		eventFlag= pActiveWidget->mouseReleased(pEvent->button());
	}
	return eventFlag;
}

void GLC_3DWidgetManagerHandle::render()
{
	// Signal 3DWidget that the view as changed
	QHash<GLC_uint, GLC_3DWidget*>::iterator iWidget= m_3DWidgetHash.begin();
	while (m_3DWidgetHash.constEnd() != iWidget)
	{
		iWidget.value()->updateWidgetRep();
		++iWidget;
	}

	// Render the 3D widget
	m_Collection.render(0, glc::WireRenderFlag);
	m_Collection.render(0, glc::TransparentRenderFlag);
	m_Collection.render(1, glc::WireRenderFlag);
	if (GLC_State::glslUsed())
	{
		m_Collection.renderShaderGroup(glc::WireRenderFlag);
		m_Collection.renderShaderGroup(glc::TransparentRenderFlag);
	}
}

QPair<GLC_uint, GLC_Point3d> GLC_3DWidgetManagerHandle::select(QMouseEvent* event)
{

	GLC_uint selectionId= m_pViewport->selectOnPreviousRender(event->x(), event->y());
	const GLC_Point3d selectedPoint(m_pViewport->unProject(event->x(), event->y()));

	QPair<GLC_uint, GLC_Point3d> selection;
	selection.first= selectionId;
	selection.second= selectedPoint;

	return selection;
}
