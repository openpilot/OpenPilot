/**
 ******************************************************************************
 *
 * @file       configtelemetrywidget.h
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
#include "configtelemetrywidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>


ConfigTelemetryWidget::ConfigTelemetryWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_telemetry = new Ui_TelemetryWidget();
    m_telemetry->setupUi(this);

    // Now connect the widget to the ManualControlCommand / Channel UAVObject
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVObject *obj = objManager->getObject(QString("TelemetrySettings"));
    UAVObjectField *field = obj->getField(QString("Speed"));
    m_telemetry->telemetrySpeed->addItems(field->getOptions());

    requestTelemetryUpdate();
    connect(m_telemetry->saveTelemetryToSD, SIGNAL(clicked()), this, SLOT(saveTelemetryUpdate()));
    connect(m_telemetry->saveTelemetryToRAM, SIGNAL(clicked()), this, SLOT(sendTelemetryUpdate()));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(requestTelemetryUpdate()));

    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(requestTelemetryUpdate()));
    requestTelemetryUpdate();
}

ConfigTelemetryWidget::~ConfigTelemetryWidget()
{
   // Do nothing
}


/*******************************
 * Telemetry Settings
 *****************************/

/**
  Request telemetry settings from the board
  */
void ConfigTelemetryWidget::requestTelemetryUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("TelemetrySettings")));
    Q_ASSERT(obj);
    UAVObjectField *field = obj->getField(QString("Speed"));
    m_telemetry->telemetrySpeed->setCurrentIndex(m_telemetry->telemetrySpeed->findText(field->getValue().toString()));
}

/**
  Send telemetry settings to the board
  */
void ConfigTelemetryWidget::sendTelemetryUpdate()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("TelemetrySettings")));
    Q_ASSERT(obj);
    UAVObjectField* field = obj->getField(QString("Speed"));
    field->setValue(m_telemetry->telemetrySpeed->currentText());
    obj->updated();
}

/**
  Send telemetry settings to the board and request saving to SD card
  */
void ConfigTelemetryWidget::saveTelemetryUpdate()
{
    // Send update so that the latest value is saved
    sendTelemetryUpdate();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("TelemetrySettings")));
    Q_ASSERT(obj);
    saveObjectToSD(obj);
}


