/**
 ******************************************************************************
 *
 * @file       baseview.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief The Core GCS plugin
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "baseview.h"

#include <QtGui/QWidget>

using namespace Core;

BaseView::BaseView(QObject *parent)
        : IView(parent),
    m_viewName(""),
    m_widget(0),
    m_context(QList<int>()),
    m_defaultPosition(IView::First)
{
}

BaseView::~BaseView()
{
    delete m_widget;
}

QList<int> BaseView::context() const
{
    return m_context;
}

QWidget *BaseView::widget()
{
    return m_widget;
}

const char *BaseView::uniqueViewName() const
{
    return m_viewName;
}


IView::ViewPosition BaseView::defaultPosition() const
{
    return m_defaultPosition;
}

void BaseView::setUniqueViewName(const char *name)
{
    m_viewName = name;
}

QWidget *BaseView::setWidget(QWidget *widget)
{
    QWidget *oldWidget = m_widget;
    m_widget = widget;
    return oldWidget;
}

void BaseView::setContext(const QList<int> &context)
{
    m_context = context;
}

void BaseView::setDefaultPosition(IView::ViewPosition position)
{
    m_defaultPosition = position;
}

