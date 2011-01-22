/**
 ******************************************************************************
 *
 * @file       viewmanager.cpp
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

#include "viewmanager.h"

#include "coreconstants.h"
#include "mainwindow.h"
#include "uniqueidmanager.h"
#include "iview.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <aggregation/aggregate.h>
#include <extensionsystem/pluginmanager.h>

#include <QtCore/QSettings>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QStatusBar>

using namespace Core;
using namespace Core::Internal;

ViewManager::ViewManager(MainWindow *mainWnd)
  : QObject(mainWnd),
    m_mainWnd(mainWnd)
{
    for (int i = 0; i < 3; ++i) {
        QWidget *w = new QWidget();
        m_mainWnd->statusBar()->insertPermanentWidget(i, w);
        w->setLayout(new QHBoxLayout);
        w->setVisible(true);
        w->layout()->setMargin(0);
        m_statusBarWidgets.append(w);
    }
    QLabel *l = new QLabel();
    m_mainWnd->statusBar()->insertPermanentWidget(3, l, 1);
}

ViewManager::~ViewManager()
{
}

void ViewManager::init()
{
    connect(ExtensionSystem::PluginManager::instance(), SIGNAL(objectAdded(QObject*)),
            this, SLOT(objectAdded(QObject*)));
    connect(ExtensionSystem::PluginManager::instance(), SIGNAL(aboutToRemoveObject(QObject*)),
            this, SLOT(aboutToRemoveObject(QObject*)));
}

void ViewManager::objectAdded(QObject *obj)
{
    IView *view = Aggregation::query<IView>(obj);
    if (!view)
        return;

    QWidget *viewWidget = 0;
    viewWidget = view->widget();
    m_statusBarWidgets.at(view->defaultPosition())->layout()->addWidget(viewWidget);

    m_viewMap.insert(view, viewWidget);
    viewWidget->setObjectName(view->uniqueViewName());
    m_mainWnd->addContextObject(view);
}

void ViewManager::aboutToRemoveObject(QObject *obj)
{
    IView *view = Aggregation::query<IView>(obj);
    if (!view)
        return;
    m_mainWnd->removeContextObject(view);
}

void ViewManager::readSettings(QSettings *settings)
{
    m_mainWnd->restoreState(settings->value(QLatin1String("ViewGroup_Default"), QByteArray()).toByteArray());
}

void ViewManager::saveSettings(QSettings *settings)
{
    settings->setValue(QLatin1String("ViewGroup_Default"), m_mainWnd->saveState());
}

IView *ViewManager::view(const QString &id)
{
    QList<IView *> list =
        ExtensionSystem::PluginManager::instance()->getObjects<IView>();
    foreach (IView *view, list) {
        if (view->uniqueViewName() == id)
            return view;
    }
    return 0;
}
