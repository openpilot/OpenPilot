/**
 ******************************************************************************
 *
 * @file       configairframewidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Airframe configuration panel
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
#include "configairframewidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>



ConfigAirframeWidget::ConfigAirframeWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_aircraft = new Ui_AircraftWidget();
    m_aircraft->setupUi(this);

    // Now connect the widget to the ManualControlCommand / Channel UAVObject
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVObject *obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("SystemSettings")));
    QString fieldName = QString("AirframeType");
    UAVObjectField *field = obj->getField(fieldName);
    m_aircraft->aircraftType->addItems(field->getOptions());

    requestAircraftUpdate();
    connect(m_aircraft->saveAircraftToSD, SIGNAL(clicked()), this, SLOT(saveAircraftUpdate()));
    connect(m_aircraft->saveAircraftToRAM, SIGNAL(clicked()), this, SLOT(sendAircraftUpdate()));
    connect(m_aircraft->getAircraftCurrent, SIGNAL(clicked()), this, SLOT(requestAircraftUpdate()));

    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(requestAircraftUpdate()));

}

ConfigAirframeWidget::~ConfigAirframeWidget()
{
   // Do nothing
}


/**************************
  * Aircraft settings
  **************************/
/**
  Request the current value of the SystemSettings which holds the aircraft type
  */
void ConfigAirframeWidget::requestAircraftUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    // Get the Airframe type from the system settings:
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("SystemSettings")));
    Q_ASSERT(obj);
    obj->requestUpdate();
    UAVObjectField *field = obj->getField(QString("AirframeType"));
    m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText(field->getValue().toString()));

}


/**
  Sends the config to the board (airframe type)
  */
void ConfigAirframeWidget::sendAircraftUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("SystemSettings")));
    Q_ASSERT(obj);
    UAVObjectField* field = obj->getField(QString("AirframeType"));
    field->setValue(m_aircraft->aircraftType->currentText());
    obj->updated();
}

/**
  Send airframe type to the board and request saving to SD card
  */
void ConfigAirframeWidget::saveAircraftUpdate()
{
    // Send update so that the latest value is saved
    sendAircraftUpdate();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("SystemSettings")));
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);
}

