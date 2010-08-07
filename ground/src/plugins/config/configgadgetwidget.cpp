/**
 ******************************************************************************
 *
 * @file       configgadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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
#include "configgadgetwidget.h"
#include "ui_settingswidget.h"


#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

ConfigGadgetWidget::ConfigGadgetWidget(QWidget *parent) : QWidget(parent)
{
    m_config = new Ui_SettingsWidget();
    m_config->setupUi(this);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    // Fill in the dropdown menus for the channel RC Input assignement.
    QStringList channelsList;
    channelsList << "None" << "Roll" << "Pitch" << "Yaw" << "Throttle" << "FlightMode";
    m_config->ch0Assign->addItems(channelsList);
    m_config->ch1Assign->addItems(channelsList);
    m_config->ch2Assign->addItems(channelsList);
    m_config->ch3Assign->addItems(channelsList);
    m_config->ch4Assign->addItems(channelsList);
    m_config->ch5Assign->addItems(channelsList);
    m_config->ch6Assign->addItems(channelsList);
    m_config->ch7Assign->addItems(channelsList);

    // Now connect the widget to the ManualControlCommand / Channel UAVObject
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlCommand")));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateChannels(UAVObject*)));

    // Get the receiver types supported by OpenPilot and fill the corresponding
    // dropdown menu:
    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlSettings")));
    QString fieldName = QString("InputMode");
    UAVObjectField *field = obj->getField(fieldName);
    m_config->receiverType->addItems(field->getOptions());

    // Same for the aircraft types:
    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("SystemSettings")));
    fieldName = QString("AirframeType");
    field = obj->getField(fieldName);
    m_config->aircraftType->addItems(field->getOptions());

    // And for the channel output assignement options
    m_config->ch0Output->addItem("None");
    m_config->ch1Output->addItem("None");
    m_config->ch2Output->addItem("None");
    m_config->ch3Output->addItem("None");
    m_config->ch4Output->addItem("None");
    m_config->ch5Output->addItem("None");
    m_config->ch6Output->addItem("None");
    m_config->ch7Output->addItem("None");

    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        // NOTE: we assume that all options in ActuatorSettings are a channel assignement
        // except for the options called "ChannelXXX"
        if (!field->getName().contains("Channel")) {
            m_config->ch0Output->addItem(field->getName());
            m_config->ch1Output->addItem(field->getName());
            m_config->ch2Output->addItem(field->getName());
            m_config->ch3Output->addItem(field->getName());
            m_config->ch4Output->addItem(field->getName());
            m_config->ch5Output->addItem(field->getName());
            m_config->ch6Output->addItem(field->getName());
            m_config->ch7Output->addItem(field->getName());
        }
    }

    requestRCInputUpdate();

    connect(m_config->saveRCInputToSD, SIGNAL(clicked()), this, SLOT(saveRCInputObject()));
    connect(m_config->saveRCInputToRAM, SIGNAL(clicked()), this, SLOT(sendRCInputUpdate()));
    connect(m_config->getRCInputCurrent, SIGNAL(clicked()), this, SLOT(requestRCInputUpdate()));

    firstUpdate = true;

}

ConfigGadgetWidget::~ConfigGadgetWidget()
{
   // Do nothing
}

void ConfigGadgetWidget::resizeEvent(QResizeEvent *event)
{

    QWidget::resizeEvent(event);
}


/**
  Request the current config from the board (RC Output)
  */


/**
  Request the current config from the board
  */
void ConfigGadgetWidget::requestRCInputUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlSettings")));
    Q_ASSERT(obj);
    obj->requestUpdate();

    // Now update all the slider values:
    QString fieldName = QString("ChannelMax");
    UAVObjectField *field = obj->getField(fieldName);
    m_config->ch0Max->setText(field->getValue(0).toString());
    m_config->ch0Slider->setMaximum(field->getValue(0).toInt());
    m_config->ch1Max->setText(field->getValue(1).toString());
    m_config->ch1Slider->setMaximum(field->getValue(1).toInt());
    m_config->ch2Max->setText(field->getValue(2).toString());
    m_config->ch2Slider->setMaximum(field->getValue(2).toInt());
    m_config->ch3Max->setText(field->getValue(3).toString());
    m_config->ch3Slider->setMaximum(field->getValue(3).toInt());
    m_config->ch4Max->setText(field->getValue(4).toString());
    m_config->ch4Slider->setMaximum(field->getValue(4).toInt());
    m_config->ch5Max->setText(field->getValue(5).toString());
    m_config->ch5Slider->setMaximum(field->getValue(5).toInt());
    m_config->ch6Max->setText(field->getValue(6).toString());
    m_config->ch6Slider->setMaximum(field->getValue(6).toInt());
    m_config->ch7Max->setText(field->getValue(7).toString());
    m_config->ch7Slider->setMaximum(field->getValue(7).toInt());

    fieldName = QString("ChannelMin");
    field = obj->getField(fieldName);
    m_config->ch0Min->setText(field->getValue(0).toString());
    m_config->ch0Slider->setMinimum(field->getValue(0).toInt());
    m_config->ch1Min->setText(field->getValue(1).toString());
    m_config->ch1Slider->setMinimum(field->getValue(1).toInt());
    m_config->ch2Min->setText(field->getValue(2).toString());
    m_config->ch2Slider->setMinimum(field->getValue(2).toInt());
    m_config->ch3Min->setText(field->getValue(3).toString());
    m_config->ch3Slider->setMinimum(field->getValue(3).toInt());
    m_config->ch4Min->setText(field->getValue(4).toString());
    m_config->ch4Slider->setMinimum(field->getValue(4).toInt());
    m_config->ch5Min->setText(field->getValue(5).toString());
    m_config->ch5Slider->setMinimum(field->getValue(5).toInt());
    m_config->ch6Min->setText(field->getValue(6).toString());
    m_config->ch6Slider->setMinimum(field->getValue(6).toInt());
    m_config->ch7Min->setText(field->getValue(7).toString());
    m_config->ch7Slider->setMinimum(field->getValue(7).toInt());

    fieldName = QString("ChannelNeutral");
    field = obj->getField(fieldName);
    m_config->ch0Slider->setValue(field->getValue(0).toInt());
    m_config->ch1Slider->setValue(field->getValue(1).toInt());
    m_config->ch2Slider->setValue(field->getValue(2).toInt());
    m_config->ch3Slider->setValue(field->getValue(3).toInt());
    m_config->ch4Slider->setValue(field->getValue(4).toInt());
    m_config->ch5Slider->setValue(field->getValue(5).toInt());
    m_config->ch6Slider->setValue(field->getValue(6).toInt());
    m_config->ch7Slider->setValue(field->getValue(7).toInt());

    // Update receiver type
    fieldName = QString("InputMode");
    field = obj->getField(fieldName);
    m_config->receiverType->setCurrentIndex(m_config->receiverType->findText(field->getValue().toString()));

    // Reset all channel assignement dropdowns:
    m_config->ch0Assign->setCurrentIndex(0);
    m_config->ch1Assign->setCurrentIndex(0);
    m_config->ch2Assign->setCurrentIndex(0);
    m_config->ch3Assign->setCurrentIndex(0);
    m_config->ch4Assign->setCurrentIndex(0);
    m_config->ch5Assign->setCurrentIndex(0);
    m_config->ch6Assign->setCurrentIndex(0);
    m_config->ch7Assign->setCurrentIndex(0);

    // Update all channels assignements
    assignChannel(obj, field, QString("Roll"));
    assignChannel(obj, field, QString("Pitch"));
    assignChannel(obj, field, QString("Yaw"));
    assignChannel(obj, field, QString("Throttle"));
    assignChannel(obj, field, QString("FlightMode"));

}


/**
  * Sends the config to the board, without saving to the SD card
  */
void ConfigGadgetWidget::sendRCInputUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlSettings")));
    Q_ASSERT(obj);
    // Now update all fields from the sliders:
    QString fieldName = QString("ChannelMax");
    UAVObjectField * field = obj->getField(fieldName);
    field->setValue(m_config->ch0Max->text().toInt(),0);
    field->setValue(m_config->ch1Max->text().toInt(),1);
    field->setValue(m_config->ch2Max->text().toInt(),2);
    field->setValue(m_config->ch3Max->text().toInt(),3);
    field->setValue(m_config->ch4Max->text().toInt(),4);
    field->setValue(m_config->ch5Max->text().toInt(),5);
    field->setValue(m_config->ch6Max->text().toInt(),6);
    field->setValue(m_config->ch7Max->text().toInt(),7);

    fieldName = QString("ChannelMin");
    field = obj->getField(fieldName);
    field->setValue(m_config->ch0Min->text().toInt(),0);
    field->setValue(m_config->ch1Min->text().toInt(),1);
    field->setValue(m_config->ch2Min->text().toInt(),2);
    field->setValue(m_config->ch3Min->text().toInt(),3);
    field->setValue(m_config->ch4Min->text().toInt(),4);
    field->setValue(m_config->ch5Min->text().toInt(),5);
    field->setValue(m_config->ch6Min->text().toInt(),6);
    field->setValue(m_config->ch7Min->text().toInt(),7);

    fieldName = QString("ChannelNeutral");
    field = obj->getField(fieldName);
    field->setValue(m_config->ch0Slider->value(),0);
    field->setValue(m_config->ch1Slider->value(),1);
    field->setValue(m_config->ch2Slider->value(),2);
    field->setValue(m_config->ch3Slider->value(),3);
    field->setValue(m_config->ch4Slider->value(),4);
    field->setValue(m_config->ch5Slider->value(),5);
    field->setValue(m_config->ch6Slider->value(),6);
    field->setValue(m_config->ch7Slider->value(),7);

    // Set RC Receiver type:
    fieldName = QString("InputMode");
    field = obj->getField(fieldName);
    field->setValue(m_config->receiverType->currentText());

    // Set Roll/Pitch/Yaw/Etc assignement:
    // Rule: if two channels have the same setting (which is wrong!) the higher channel
    // will get the setting.
    if (m_config->ch0Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch0Assign->currentText());
        field->setValue(field->getOptions().at(0)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch1Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch1Assign->currentText());
        field->setValue(field->getOptions().at(1));
    }
    if (m_config->ch2Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch2Assign->currentText());
        field->setValue(field->getOptions().at(2));
    }
    if (m_config->ch3Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch3Assign->currentText());
        field->setValue(field->getOptions().at(3));
    }
    if (m_config->ch4Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch4Assign->currentText());
        field->setValue(field->getOptions().at(4));
    }
    if (m_config->ch5Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch5Assign->currentText());
        field->setValue(field->getOptions().at(5));
    }
    if (m_config->ch6Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch6Assign->currentText());
        field->setValue(field->getOptions().at(6));
    }
    if (m_config->ch7Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch7Assign->currentText());
        field->setValue(field->getOptions().at(7));
    }

    // ... and send to the OP Board
    obj->updated();

}


/**
  Sends the config to the board and request saving into the SD card
  */
void ConfigGadgetWidget::saveRCInputObject()
{
    // Send update so that the latest value is saved
    sendRCInputUpdate();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlSettings")));
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);
}


void ConfigGadgetWidget::updateObjectPersistance(ObjectPersistence::OperationOptions op, UAVObject *obj)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    ObjectPersistence* objper = dynamic_cast<ObjectPersistence*>( objManager->getObject(ObjectPersistence::NAME) );
    if (obj != NULL)
    {
        ObjectPersistence::DataFields data;
        data.Operation = op;
        data.Selection = ObjectPersistence::SELECTION_SINGLEOBJECT;
        data.ObjectID = obj->getObjID();
        data.InstanceID = obj->getInstID();
        objper->setData(data);
        objper->updated();
    }
}


/**
  * Set the dropdown option for a channel assignement
  */
void ConfigGadgetWidget::assignChannel(UAVDataObject *obj, UAVObjectField *field, QString str)
{
    field = obj->getField(str);
    QStringList options = field->getOptions();
    switch (options.indexOf(field->getValue().toString())) {
    case 0:
        m_config->ch0Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 1:
        m_config->ch1Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 2:
        m_config->ch2Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 3:
        m_config->ch3Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 4:
        m_config->ch4Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 5:
        m_config->ch5Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 6:
        m_config->ch6Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 7:
        m_config->ch7Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    }
}


/**
  * Updates the slider positions and min/max values
  *
  */
void ConfigGadgetWidget::updateChannels(UAVObject* controlCommand)
{

    QString fieldName = QString("Connected");
    UAVObjectField *field = controlCommand->getField(fieldName);
    if (field->getValue().toBool()) {
        m_config->RCInputConnected->setText("RC Receiver Connected");
    } else {
        m_config->RCInputConnected->setText("RC Receiver Not Connected");
    }
    if (m_config->doRCInputCalibration->isChecked()) {
        fieldName = QString("Channel");
        field =  controlCommand->getField(fieldName);
        // Hey: if you find a nicer way of doing this, be my guest!
        this->updateChannelSlider(&*m_config->ch0Slider,
                            &*m_config->ch0Min,
                            &*m_config->ch0Max,
                            &*m_config->ch0Rev,field->getValue(0).toInt());
        this->updateChannelSlider(&*m_config->ch1Slider,
                            &*m_config->ch1Min,
                            &*m_config->ch1Max,
                            &*m_config->ch1Rev,field->getValue(1).toInt());
        this->updateChannelSlider(&*m_config->ch2Slider,
                            &*m_config->ch2Min,
                            &*m_config->ch2Max,
                            &*m_config->ch2Rev,field->getValue(2).toInt());
        this->updateChannelSlider(&*m_config->ch3Slider,
                            &*m_config->ch3Min,
                            &*m_config->ch3Max,
                            &*m_config->ch3Rev,field->getValue(3).toInt());
        this->updateChannelSlider(&*m_config->ch4Slider,
                            &*m_config->ch4Min,
                            &*m_config->ch4Max,
                            &*m_config->ch4Rev,field->getValue(4).toInt());
        this->updateChannelSlider(&*m_config->ch5Slider,
                            &*m_config->ch5Min,
                            &*m_config->ch5Max,
                            &*m_config->ch5Rev,field->getValue(5).toInt());
        this->updateChannelSlider(&*m_config->ch6Slider,
                            &*m_config->ch6Min,
                            &*m_config->ch6Max,
                            &*m_config->ch6Rev,field->getValue(6).toInt());
        this->updateChannelSlider(&*m_config->ch7Slider,
                            &*m_config->ch7Min,
                            &*m_config->ch7Max,
                            &*m_config->ch7Rev,field->getValue(7).toInt());
        firstUpdate = false;
    } else  {
        firstUpdate = true;
    }
}


void ConfigGadgetWidget::updateChannelSlider(QSlider* slider, QLabel* min, QLabel* max, QCheckBox* rev, int value) {

    if (firstUpdate) {
        // Reset all the min/max values of the sliders since we are
        // starting the calibration.
        slider->setMaximum(value);
        slider->setMinimum(value);
        slider->setValue(value);
        max->setText(QString::number(value));
        min->setText(QString::number(value));
        return;
    }

    if (value != 0) { // Avoids glitches...
        if (value > slider->maximum()) {
            slider->setMaximum(value);
            max->setText(QString::number(value));
        }
        if (value < slider->minimum()) {
            slider->setMinimum(value);
            min->setText(QString::number(value));
        }
        slider->setValue(value);
    }
}


