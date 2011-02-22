/**
 ******************************************************************************
 *
 * @file       configstabilizationwidget.h
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#include "configstabilizationwidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>



ConfigStabilizationWidget::ConfigStabilizationWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_stabilization = new Ui_StabilizationWidget();
    m_stabilization->setupUi(this);

    // Now connect the widget to the ManualControlCommand / Channel UAVObject
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("StabilizationSettings")));

    requestStabilizationUpdate();
    //connect(m_telemetry->saveTelemetryToSD, SIGNAL(clicked()), this, SLOT(saveTelemetryUpdate()));
    //connect(m_telemetry->saveTelemetryToRAM, SIGNAL(clicked()), this, SLOT(sendTelemetryUpdate()));
    //connect(m_telemetry->getTelemetryCurrent, SIGNAL(clicked()), this, SLOT(requestTelemetryUpdate()));

    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(requestStabilizationUpdate()));

}

ConfigStabilizationWidget::~ConfigStabilizationWidget()
{
   // Do nothing
}


/*******************************
 * Stabilization Settings
 *****************************/

/**
  Request stabilization settings from the board
  */
void ConfigStabilizationWidget::requestStabilizationUpdate()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("StabilizationSettings")));
    Q_ASSERT(obj);
    obj->requestUpdate();
}

/**
  Send telemetry settings to the board
  */
/*
void ConfigStabilizationWidget::sendTelemetryUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("TelemetrySettings")));
    Q_ASSERT(obj);
    UAVObjectField* field = obj->getField(QString("Speed"));
    field->setValue(m_telemetry->telemetrySpeed->currentText());
    obj->updated();
}
*/

/**
  Send telemetry settings to the board and request saving to SD card
  */
/*
void ConfigStabilizationWidget::saveTelemetryUpdate()
{
    // Send update so that the latest value is saved
    sendTelemetryUpdate();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("TelemetrySettings")));
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);
}
*/


