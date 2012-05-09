/**
 ******************************************************************************
 * @file       waypointeditorgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup Waypoint Editor GCS Plugins
 * @{
 * @addtogroup WaypointEditorGadgetPlugin Waypoint Editor Gadget Plugin
 * @{
 * @brief A gadget to edit a list of waypoints
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
#include "waypointeditorgadgetwidget.h"
#include "ui_waypointeditor.h"

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

#include "extensionsystem/pluginmanager.h"

WaypointEditorGadgetWidget::WaypointEditorGadgetWidget(QWidget *parent) : QLabel(parent)
{
    m_waypointeditor = new Ui_WaypointEditor();
    m_waypointeditor->setupUi(this);

    waypointTable = new WaypointTable(this);
    m_waypointeditor->waypoints->setModel(waypointTable);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm != NULL);
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager != NULL);
    waypointObj = Waypoint::GetInstance(objManager);
    Q_ASSERT(waypointObj != NULL);

    // Connect the signals
    connect(m_waypointeditor->buttonNewWaypoint, SIGNAL(clicked()),
            this, SLOT(addInstance()));
}

WaypointEditorGadgetWidget::~WaypointEditorGadgetWidget()
{
   // Do nothing
}

void WaypointEditorGadgetWidget::waypointChanged(UAVObject *)
{
}

void WaypointEditorGadgetWidget::waypointActiveChanged(UAVObject *)
{
}

void WaypointEditorGadgetWidget::addInstance()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm != NULL);
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager != NULL);

    qDebug() << "Instances before: " << objManager->getNumInstances(waypointObj->getObjID());
    Waypoint *obj = new Waypoint();
    quint32 newInstId = objManager->getNumInstances(waypointObj->getObjID());
    obj->initialize(newInstId,obj->getMetaObject());
    objManager->registerObject(obj);
    qDebug() << "Instances after: " << objManager->getNumInstances(waypointObj->getObjID());
}

/**
  * @}
  * @}
  */
