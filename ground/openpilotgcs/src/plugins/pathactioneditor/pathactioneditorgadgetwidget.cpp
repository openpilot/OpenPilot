/**
 ******************************************************************************
 * @file       pathactioneditorgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup PathAction Editor GCS Plugins
 * @{
 * @addtogroup PathActionEditorGadgetPlugin PathAction Editor Gadget Plugin
 * @{
 * @brief A gadget to edit a list of pathactions
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
#include "pathactioneditorgadgetwidget.h"
#include "ui_pathactioneditor.h"

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include "browseritemdelegate.h"

#include "extensionsystem/pluginmanager.h"

PathActionEditorGadgetWidget::PathActionEditorGadgetWidget(QWidget *parent) : QLabel(parent)
{
    m_pathactioneditor = new Ui_PathActionEditor();
    m_pathactioneditor->setupUi(this);

    m_model = new PathActionEditorTreeModel();
    m_pathactioneditor->pathactions->setModel(m_model);
    m_pathactioneditor->pathactions->setColumnWidth(0, 300);
    m_pathactioneditor->pathactions->setColumnWidth(1, 500);
    m_pathactioneditor->pathactions->expandAll();
    BrowserItemDelegate *m_delegate = new BrowserItemDelegate();
    m_pathactioneditor->pathactions->setItemDelegate(m_delegate);
    m_pathactioneditor->pathactions->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_pathactioneditor->pathactions->setSelectionBehavior(QAbstractItemView::SelectItems);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm != NULL);
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager != NULL);
    pathactionObj = PathAction::GetInstance(objManager);
    Q_ASSERT(pathactionObj != NULL);
    waypointObj = Waypoint::GetInstance(objManager);
    Q_ASSERT(waypointObj != NULL);

    // Connect the signals
    connect(m_pathactioneditor->buttonNewPathAction, SIGNAL(clicked()),
            this, SLOT(addPathActionInstance()));
    connect(m_pathactioneditor->buttonNewWaypoint, SIGNAL(clicked()),
            this, SLOT(addWaypointInstance()));
}

PathActionEditorGadgetWidget::~PathActionEditorGadgetWidget()
{
   // Do nothing
}

void PathActionEditorGadgetWidget::pathactionChanged(UAVObject *)
{
}

void PathActionEditorGadgetWidget::addPathActionInstance()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm != NULL);
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager != NULL);

    qDebug() << "Instances before: " << objManager->getNumInstances(pathactionObj->getObjID());
    PathAction *obj = new PathAction();
    quint32 newInstId = objManager->getNumInstances(pathactionObj->getObjID());
    obj->initialize(newInstId,obj->getMetaObject());
    objManager->registerObject(obj);
    qDebug() << "Instances after: " << objManager->getNumInstances(pathactionObj->getObjID());
}

void PathActionEditorGadgetWidget::addWaypointInstance()
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
