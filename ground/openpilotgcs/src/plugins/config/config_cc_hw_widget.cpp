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
#include "config_cc_hw_widget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>


ConfigCCHWWidget::ConfigCCHWWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_telemetry = new Ui_CC_HW_Widget();
    m_telemetry->setupUi(this);
    smartsave=new smartSaveButton(m_telemetry->saveTelemetryToRAM,m_telemetry->saveTelemetryToSD);
    // Now connect the widget to the ManualControlCommand / Channel UAVObject
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVObject *obj = objManager->getObject(QString("TelemetrySettings"));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshValues()));
    smartsave->addObject(obj);
    UAVObjectField *field = obj->getField(QString("Speed"));
    m_telemetry->telemetrySpeed->addItems(field->getOptions());

    obj = objManager->getObject(QString("HwSettings"));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshValues()));
    smartsave->addObject(obj);
    field = obj->getField(QString("CC_FlexiPort"));
    m_telemetry->cbFlexi->addItems(field->getOptions());

    field = obj->getField(QString("CC_MainPort"));
    m_telemetry->cbTele->addItems(field->getOptions());

    connect(smartsave, SIGNAL(preProcessOperations()), this, SLOT(saveTelemetryUpdate()));

    enableControls(false);
    refreshValues();
    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(onAutopilotConnect()));
    connect(parent, SIGNAL(autopilotDisconnected()),this, SLOT(onAutopilotDisconnect()));
}

ConfigCCHWWidget::~ConfigCCHWWidget()
{
   // Do nothing
}


/*******************************
 * Telemetry Settings
 *****************************/

void ConfigCCHWWidget::enableControls(bool enable)
{
    m_telemetry->saveTelemetryToSD->setEnabled(enable);
    //m_telemetry->saveTelemetryToRAM->setEnabled(enable);
}

/**
  Request telemetry settings from the board
  */
void ConfigCCHWWidget::refreshValues()
{
    qDebug()<<"refreshvalues";
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("TelemetrySettings")));
    Q_ASSERT(obj);
    UAVObjectField *field = obj->getField(QString("Speed"));
    m_telemetry->telemetrySpeed->setCurrentIndex(m_telemetry->telemetrySpeed->findText(field->getValue().toString()));
    qDebug()<<field->getValue().toString();
    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("HwSettings")));
    Q_ASSERT(obj);
    field = obj->getField(QString("CC_FlexiPort"));
    m_telemetry->cbFlexi->setCurrentIndex(m_telemetry->cbFlexi->findText(field->getValue().toString()));
    qDebug()<<field->getValue().toString();
    field = obj->getField(QString("CC_MainPort"));
    m_telemetry->cbTele->setCurrentIndex(m_telemetry->cbTele->findText(field->getValue().toString()));
    qDebug()<<field->getValue().toString();
}


/**
  Send telemetry settings to the board and request saving to SD card
  */
void ConfigCCHWWidget::saveTelemetryUpdate()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("TelemetrySettings")));
    Q_ASSERT(obj);
    UAVObjectField* field = obj->getField(QString("Speed"));
    field->setValue(m_telemetry->telemetrySpeed->currentText());
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("HwSettings")));
    Q_ASSERT(obj);
    field = obj->getField(QString("CC_FlexiPort"));
    field->setValue(m_telemetry->cbFlexi->currentText());
    field = obj->getField(QString("CC_MainPort"));
    field->setValue(m_telemetry->cbTele->currentText());


}
