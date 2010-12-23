/**
 ******************************************************************************
 *
 * @file       configservowidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Servo input/output configuration panel for the config gadget
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
#include "configservowidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>



ConfigServoWidget::ConfigServoWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_config = new Ui_SettingsWidget();
    m_config->setupUi(this);

    // First of all, put all the channel widgets into lists, so that we can
    // manipulate those:

    // NOTE: for historical reasons, we have objects below called ch0 to ch7, but the
    // convention for OP is Channel 1 to Channel 8.
    outLabels << m_config->ch0OutValue
            << m_config->ch1OutValue
            << m_config->ch2OutValue
            << m_config->ch3OutValue
            << m_config->ch4OutValue
            << m_config->ch5OutValue
            << m_config->ch6OutValue
            << m_config->ch7OutValue;
    outSliders << m_config->ch0OutSlider
            << m_config->ch1OutSlider
            << m_config->ch2OutSlider
            << m_config->ch3OutSlider
            << m_config->ch4OutSlider
            << m_config->ch5OutSlider
            << m_config->ch6OutSlider
            << m_config->ch7OutSlider;
    outMin << m_config->ch0OutMin
            << m_config->ch1OutMin
            << m_config->ch2OutMin
            << m_config->ch3OutMin
            << m_config->ch4OutMin
            << m_config->ch5OutMin
            << m_config->ch6OutMin
            << m_config->ch7OutMin;
    outMax << m_config->ch0OutMax
            << m_config->ch1OutMax
            << m_config->ch2OutMax
            << m_config->ch3OutMax
            << m_config->ch4OutMax
            << m_config->ch5OutMax
            << m_config->ch6OutMax
            << m_config->ch7OutMax;
    reversals << m_config->ch0Rev
            << m_config->ch1Rev
            << m_config->ch2Rev
            << m_config->ch3Rev
            << m_config->ch4Rev
            << m_config->ch5Rev
            << m_config->ch6Rev
            << m_config->ch7Rev;
    inSliders << m_config->ch0Slider
            << m_config->ch1Slider
            << m_config->ch2Slider
            << m_config->ch3Slider
            << m_config->ch4Slider
            << m_config->ch5Slider
            << m_config->ch6Slider
            << m_config->ch7Slider;


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
    m_config->receiverType->setDisabled(true); // This option does not work for now, it is a compile-time option.

    // Fill in the dropdown menus for the channel RC Input assignement.
    QStringList channelsList;
        channelsList << "None";
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        if (field->getUnits().contains("channel")) {
            channelsList.append(field->getName());
        }
    }

    m_config->ch0Assign->addItems(channelsList);
    m_config->ch1Assign->addItems(channelsList);
    m_config->ch2Assign->addItems(channelsList);
    m_config->ch3Assign->addItems(channelsList);
    m_config->ch4Assign->addItems(channelsList);
    m_config->ch5Assign->addItems(channelsList);
    m_config->ch6Assign->addItems(channelsList);
    m_config->ch7Assign->addItems(channelsList);

    // And for the channel output assignement options
    m_config->ch0Output->addItem("None");
    m_config->ch1Output->addItem("None");
    m_config->ch2Output->addItem("None");
    m_config->ch3Output->addItem("None");
    m_config->ch4Output->addItem("None");
    m_config->ch5Output->addItem("None");
    m_config->ch6Output->addItem("None");
    m_config->ch7Output->addItem("None");

    // And the flight mode settings:
    field = obj->getField(QString("Pos1FlightMode"));
    m_config->fmsModePos1->addItems(field->getOptions());
    m_config->fmsModePos2->addItems(field->getOptions());
    m_config->fmsModePos3->addItems(field->getOptions());
    field = obj->getField(QString("Pos1StabilizationSettings"));
    channelsList.clear();
    channelsList.append(field->getOptions());
    m_config->fmsSsPos1Roll->addItems(channelsList);
    m_config->fmsSsPos1Pitch->addItems(channelsList);
    m_config->fmsSsPos1Yaw->addItems(channelsList);
    m_config->fmsSsPos2Roll->addItems(channelsList);
    m_config->fmsSsPos2Pitch->addItems(channelsList);
    m_config->fmsSsPos2Yaw->addItems(channelsList);
    m_config->fmsSsPos3Roll->addItems(channelsList);
    m_config->fmsSsPos3Pitch->addItems(channelsList);
    m_config->fmsSsPos3Yaw->addItems(channelsList);


    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        if (field->getUnits().contains("channel")) {
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

    for (int i = 0; i < 8; i++) {
        connect(outMin[i], SIGNAL(editingFinished()), this, SLOT(setChOutRange()));
        connect(outMax[i], SIGNAL(editingFinished()), this, SLOT(setChOutRange()));
        connect(reversals[i], SIGNAL(toggled(bool)), this, SLOT(reverseChannel(bool)));
        // Now connect the channel out sliders to our signal to send updates in test mode
        connect(outSliders[i], SIGNAL(valueChanged(int)), this, SLOT(sendChannelTest(int)));
    }

    connect(m_config->channelOutTest, SIGNAL(toggled(bool)), this, SLOT(runChannelTests(bool)));

    requestRCInputUpdate();
    requestRCOutputUpdate();

    connect(m_config->saveRCInputToSD, SIGNAL(clicked()), this, SLOT(saveRCInputObject()));
    connect(m_config->saveRCInputToRAM, SIGNAL(clicked()), this, SLOT(sendRCInputUpdate()));
    connect(m_config->getRCInputCurrent, SIGNAL(clicked()), this, SLOT(requestRCInputUpdate()));

       // Flightmode panel is connected to the same as rcinput because
    // the underlying object is the same!
    connect(m_config->saveFmsToSD, SIGNAL(clicked()), this, SLOT(saveRCInputObject()));
    connect(m_config->saveFmsToRAM, SIGNAL(clicked()), this, SLOT(sendRCInputUpdate()));
    connect(m_config->getFmsCurrent, SIGNAL(clicked()), this, SLOT(requestRCInputUpdate()));

    connect(m_config->saveRCOutputToSD, SIGNAL(clicked()), this, SLOT(saveRCOutputObject()));
    connect(m_config->saveRCOutputToRAM, SIGNAL(clicked()), this, SLOT(sendRCOutputUpdate()));
    connect(m_config->getRCOutputCurrent, SIGNAL(clicked()), this, SLOT(requestRCOutputUpdate()));


    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(requestRCInputUpdate()));
    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(requestRCOutputUpdate()));

    firstUpdate = true;

}

ConfigServoWidget::~ConfigServoWidget()
{
   // Do nothing
}


/**
  Sends the channel value to the UAV to move the servo.
  Returns immediately if we are not in testing mode
  */
void ConfigServoWidget::sendChannelTest(int value)
{
    // First of all, update the label:
    QSlider *ob = (QSlider*)QObject::sender();
    int index = outSliders.indexOf(ob);
    if (reversals[index]->isChecked())
        value = outMin[index]->value()-value+outMax[index]->value();
    else
        outLabels[index]->setText(QString::number(value));

    outLabels[index]->setText(QString::number(value));
    if (!m_config->channelOutTest->isChecked())
        return;

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorCommand")));

    UAVObjectField * channel = obj->getField("Channel");
    channel->setValue(value,index);
    obj->updated();

}

/**
  Toggles the channel testing mode by making the GCS take over
  the ActuatorCommand objects
  */
void ConfigServoWidget::runChannelTests(bool state)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorCommand")));
    UAVObject::Metadata mdata = obj->getMetadata();
    if (state)
    {
        accInitialData = mdata;
        mdata.flightAccess = UAVObject::ACCESS_READONLY;
        mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
        mdata.gcsTelemetryAcked = false;
        mdata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
        mdata.gcsTelemetryUpdatePeriod = 100;
    }
    else
    {
        mdata = accInitialData; // Restore metadata
    }
    obj->setMetadata(mdata);

}



/********************************
  *  Output settings
  *******************************/

/**
  Request the current config from the board (RC Output)
  */
void ConfigServoWidget::requestRCOutputUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    // Get the Airframe type from the system settings:
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("SystemSettings")));
    Q_ASSERT(obj);
    obj->requestUpdate();
    UAVObjectField *field = obj->getField(QString("AirframeType"));
    m_config->aircraftType->setText(QString("Aircraft type: ") + field->getValue().toString());

    // Reset all channel assignements:
    m_config->ch0Output->setCurrentIndex(0);
    m_config->ch1Output->setCurrentIndex(0);
    m_config->ch2Output->setCurrentIndex(0);
    m_config->ch3Output->setCurrentIndex(0);
    m_config->ch4Output->setCurrentIndex(0);
    m_config->ch5Output->setCurrentIndex(0);
    m_config->ch6Output->setCurrentIndex(0);
    m_config->ch7Output->setCurrentIndex(0);

    // Get the channel assignements:
    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
    obj->requestUpdate();
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        if (field->getUnits().contains("channel")) {
            assignOutputChannel(obj,field->getName());
        }
    }

    // Get Output rates for both banks
    field = obj->getField(QString("ChannelUpdateFreq"));
    m_config->outputRate1->setValue(field->getValue(0).toInt());
    m_config->outputRate2->setValue(field->getValue(1).toInt());

    // Get Channel ranges:
    for (int i=0;i<8;i++) {
        field = obj->getField(QString("ChannelMin"));
        int minValue = field->getValue(i).toInt();
        outMin[i]->setValue(minValue);
        field = obj->getField(QString("ChannelMax"));
        int maxValue = field->getValue(i).toInt();
        outMax[i]->setValue(maxValue);
        if (maxValue>minValue) {
            outSliders[i]->setMinimum(minValue);
            outSliders[i]->setMaximum(maxValue);
            reversals[i]->setChecked(false);
        } else {
            outSliders[i]->setMinimum(maxValue);
            outSliders[i]->setMaximum(minValue);
            reversals[i]->setChecked(true);
        }
    }

    field = obj->getField(QString("ChannelNeutral"));
    for (int i=0; i<8; i++) {
        int value = field->getValue(i).toInt();
        outSliders[i]->setValue(value);
        outLabels[i]->setText(QString::number(value));
    }


}

/**
  * Sends the config to the board, without saving to the SD card (RC Output)
  */
void ConfigServoWidget::sendRCOutputUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);

    // Now send channel ranges:
    UAVObjectField * field = obj->getField(QString("ChannelMax"));
    for (int i = 0; i < 8; i++) {
        field->setValue(outMax[i]->value(),i);
    }

    field = obj->getField(QString("ChannelMin"));
    for (int i = 0; i < 8; i++) {
        field->setValue(outMin[i]->value(),i);
    }

    field = obj->getField(QString("ChannelNeutral"));
    for (int i = 0; i < 8; i++) {
        field->setValue(outSliders[i]->value(),i);
    }

    field = obj->getField(QString("ChannelUpdateFreq"));
    field->setValue(m_config->outputRate1->value(),0);
    field->setValue(m_config->outputRate2->value(),1);

    // Set Actuator assignement for each channel:
    // Rule: if two channels have the same setting (which is wrong!) the higher channel
    // will get the setting.

    // First, reset all channel assignements:
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        // NOTE: we assume that all options in ActuatorSettings are a channel assignement
        // except for the options called "ChannelXXX"
        if (field->getUnits().contains("channel")) {
            field->setValue(field->getOptions().last());
        }
    }

    if (m_config->ch0Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch0Output->currentText());
        field->setValue(field->getOptions().at(0)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch1Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch1Output->currentText());
        field->setValue(field->getOptions().at(1)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch2Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch2Output->currentText());
        field->setValue(field->getOptions().at(2)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch3Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch3Output->currentText());
        field->setValue(field->getOptions().at(3)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch4Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch4Output->currentText());
        field->setValue(field->getOptions().at(4)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch5Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch5Output->currentText());
        field->setValue(field->getOptions().at(5)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch6Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch6Output->currentText());
        field->setValue(field->getOptions().at(6)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch7Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch7Output->currentText());
        field->setValue(field->getOptions().at(7)); // -> This way we don't depend on channel naming convention
    }

    // ... and send to the OP Board
    obj->updated();


}


/**
  Sends the config to the board and request saving into the SD card (RC Output)
  */
void ConfigServoWidget::saveRCOutputObject()
{
    // Send update so that the latest value is saved
    sendRCOutputUpdate();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);

}


/**
  Sets the minimum/maximum value of the channel 0 to seven output sliders.
  Have to do it here because setMinimum is not a slot.

  One added trick: if the slider is at either its max or its min when the value
  is changed, then keep it on the max/min.
  */
void ConfigServoWidget::setChOutRange()
{
    QSpinBox *spinbox = (QSpinBox*)QObject::sender();
    int index = outMin.indexOf(spinbox); // This is the channel number
    if (index < 0)
        index = outMax.indexOf(spinbox); // We can't know if the signal came from min or max
    QSlider *slider = outSliders[index];
    int oldMini = slider->minimum();
    int oldMaxi = slider->maximum();
    if (outMin[index]->value()<outMax[index]->value()) {
        slider->setRange(outMin[index]->value(),
                         outMax[index]->value());
        reversals[index]->setChecked(false);
    } else {
        slider->setRange(outMax[index]->value(),
                         outMin[index]->value());
        reversals[index]->setChecked(true);
    }
    if (slider->value()==oldMini)
        slider->setValue(slider->minimum());
    if (slider->value()==oldMaxi)
        slider->setValue(slider->maximum());
}

/**
  Reverses the channel when the checkbox is clicked
  */
void ConfigServoWidget::reverseChannel(bool state)
{
    QCheckBox *checkbox = (QCheckBox*)QObject::sender();
    int index = reversals.indexOf(checkbox); // This is the channel number

    // Sanity check: if state became true, make sure the Maxvalue was higher than Minvalue
    // the situations below can happen!
    if (state && (outMax[index]->value()<outMin[index]->value()))
        return;
    if (!state && (outMax[index]->value()>outMin[index]->value()))
        return;

    // Now, swap the min & max values (only on the spinboxes, the slider
    // does not change!
    int temp = outMax[index]->value();
    outMax[index]->setValue(outMin[index]->value());
    outMin[index]->setValue(temp);

    // Also update the channel value
    // This is a trick to force the slider to update its value and
    // emit the right signal itself, because our sendChannelTest(int) method
    // relies on the object sender's identity.
    if (outSliders[index]->value()<outSliders[index]->maximum()) {
        outSliders[index]->setValue(outSliders[index]->value()+1);
        outSliders[index]->setValue(outSliders[index]->value()-1);
    } else {
        outSliders[index]->setValue(outSliders[index]->value()-1);
        outSliders[index]->setValue(outSliders[index]->value()+1);
    }

}


/********************************
  *  Input settings
  *******************************/

/**
  Request the current config from the board
  */
void ConfigServoWidget::requestRCInputUpdate()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ManualControlSettings")));
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
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        if (field->getUnits().contains("channel")) {
            assignChannel(obj, field->getName());
        }
    }

    // Update all the flight mode settingsin the relevant tab
    field = obj->getField(QString("Pos1FlightMode"));
    m_config->fmsModePos1->setCurrentIndex((m_config->fmsModePos1->findText(field->getValue().toString())));
    field = obj->getField(QString("Pos2FlightMode"));
    m_config->fmsModePos2->setCurrentIndex((m_config->fmsModePos2->findText(field->getValue().toString())));
    field = obj->getField(QString("Pos3FlightMode"));
    m_config->fmsModePos3->setCurrentIndex((m_config->fmsModePos3->findText(field->getValue().toString())));
    field = obj->getField(QString("Pos1StabilizationSettings"));
    m_config->fmsSsPos1Roll->setCurrentIndex(m_config->fmsSsPos1Roll->findText(
            field->getValue(field->getElementNames().indexOf("Roll")).toString()));
    m_config->fmsSsPos1Pitch->setCurrentIndex(m_config->fmsSsPos1Pitch->findText(
            field->getValue(field->getElementNames().indexOf("Pitch")).toString()));
    m_config->fmsSsPos1Yaw->setCurrentIndex(m_config->fmsSsPos1Yaw->findText(
            field->getValue(field->getElementNames().indexOf("Yaw")).toString()));
    field = obj->getField(QString("Pos2StabilizationSettings"));
    m_config->fmsSsPos2Roll->setCurrentIndex(m_config->fmsSsPos2Roll->findText(
            field->getValue(field->getElementNames().indexOf("Roll")).toString()));
    m_config->fmsSsPos2Pitch->setCurrentIndex(m_config->fmsSsPos2Pitch->findText(
            field->getValue(field->getElementNames().indexOf("Pitch")).toString()));
    m_config->fmsSsPos2Yaw->setCurrentIndex(m_config->fmsSsPos2Yaw->findText(
            field->getValue(field->getElementNames().indexOf("Yaw")).toString()));
    field = obj->getField(QString("Pos3StabilizationSettings"));
    m_config->fmsSsPos3Roll->setCurrentIndex(m_config->fmsSsPos3Roll->findText(
            field->getValue(field->getElementNames().indexOf("Roll")).toString()));
    m_config->fmsSsPos3Pitch->setCurrentIndex(m_config->fmsSsPos3Pitch->findText(
            field->getValue(field->getElementNames().indexOf("Pitch")).toString()));
    m_config->fmsSsPos3Yaw->setCurrentIndex(m_config->fmsSsPos3Yaw->findText(
            field->getValue(field->getElementNames().indexOf("Yaw")).toString()));

}


/**
  * Sends the config to the board, without saving to the SD card
  */
void ConfigServoWidget::sendRCInputUpdate()
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

    // First, reset all channel assignements:
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        if (field->getUnits().contains("channel")) {
            field->setValue(field->getOptions().last());
        }
    }

    // Then assign according to current GUI state:
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

    // Send all the flight mode settings
    field = obj->getField(QString("Pos1FlightMode"));
    field->setValue(m_config->fmsModePos1->currentText());
    field = obj->getField(QString("Pos2FlightMode"));
    field->setValue(m_config->fmsModePos2->currentText());
    field = obj->getField(QString("Pos3FlightMode"));
    field->setValue(m_config->fmsModePos3->currentText());

    field = obj->getField(QString("Pos1StabilizationSettings"));
    field->setValue(m_config->fmsSsPos1Roll->currentText(), field->getElementNames().indexOf("Roll"));
    field->setValue(m_config->fmsSsPos1Pitch->currentText(), field->getElementNames().indexOf("Pitch"));
    field->setValue(m_config->fmsSsPos1Yaw->currentText(), field->getElementNames().indexOf("Yaw"));
    field = obj->getField(QString("Pos2StabilizationSettings"));
    field->setValue(m_config->fmsSsPos2Roll->currentText(), field->getElementNames().indexOf("Roll"));
    field->setValue(m_config->fmsSsPos2Pitch->currentText(), field->getElementNames().indexOf("Pitch"));
    field->setValue(m_config->fmsSsPos2Yaw->currentText(), field->getElementNames().indexOf("Yaw"));
    field = obj->getField(QString("Pos3StabilizationSettings"));
    field->setValue(m_config->fmsSsPos3Roll->currentText(), field->getElementNames().indexOf("Roll"));
    field->setValue(m_config->fmsSsPos3Pitch->currentText(), field->getElementNames().indexOf("Pitch"));
    field->setValue(m_config->fmsSsPos3Yaw->currentText(), field->getElementNames().indexOf("Yaw"));

    // ... and send to the OP Board
    obj->updated();

}


/**
  Sends the config to the board and request saving into the SD card
  */
void ConfigServoWidget::saveRCInputObject()
{
    // Send update so that the latest value is saved
    sendRCInputUpdate();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlSettings")));
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);
}


/**
  * Set the dropdown option for a channel Input assignement
  */
void ConfigServoWidget::assignChannel(UAVDataObject *obj, QString str)
{
    UAVObjectField* field = obj->getField(str);
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
  * Set the dropdown option for a channel output assignement
  */
void ConfigServoWidget::assignOutputChannel(UAVDataObject *obj, QString str)
{
    UAVObjectField* field = obj->getField(str);
    QStringList options = field->getOptions();
    switch (options.indexOf(field->getValue().toString())) {
    case 0:
        m_config->ch0Output->setCurrentIndex(m_config->ch0Output->findText(str));
        break;
    case 1:
        m_config->ch1Output->setCurrentIndex(m_config->ch1Output->findText(str));
        break;
    case 2:
        m_config->ch2Output->setCurrentIndex(m_config->ch2Output->findText(str));
        break;
    case 3:
        m_config->ch3Output->setCurrentIndex(m_config->ch3Output->findText(str));
        break;
    case 4:
        m_config->ch4Output->setCurrentIndex(m_config->ch4Output->findText(str));
        break;
    case 5:
        m_config->ch5Output->setCurrentIndex(m_config->ch5Output->findText(str));
        break;
    case 6:
        m_config->ch6Output->setCurrentIndex(m_config->ch6Output->findText(str));
        break;
    case 7:
        m_config->ch7Output->setCurrentIndex(m_config->ch7Output->findText(str));
        break;
    }
}



/**
  * Updates the slider positions and min/max values
  *
  */
void ConfigServoWidget::updateChannels(UAVObject* controlCommand)
{

    QString fieldName = QString("Connected");
    UAVObjectField *field = controlCommand->getField(fieldName);
    if (field->getValue().toBool()) {
        m_config->RCInputConnected->setText("RC Receiver Connected");
    } else {
        m_config->RCInputConnected->setText("RC Receiver Not Connected");
    }
    if (m_config->doRCInputCalibration->isChecked()) {
        if (firstUpdate) {
            // Increase the data rate from the board so that the sliders
            // move faster
            UAVObject::Metadata mdata = controlCommand->getMetadata();
            mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
            mccDataRate = mdata.flightTelemetryUpdatePeriod;
            mdata.flightTelemetryUpdatePeriod = 150;
            controlCommand->setMetadata(mdata);
        }
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
        if (!firstUpdate) {
            // Restore original data rate from the board:
            UAVObject::Metadata mdata = controlCommand->getMetadata();
            mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
            mdata.flightTelemetryUpdatePeriod = mccDataRate;
            controlCommand->setMetadata(mdata);
        }
        firstUpdate = true;
    }
    //Update the Flight mode channel slider
    UAVObject* obj = getObjectManager()->getObject("ManualControlSettings");
    // Find the channel currently assigned to flightmode
    field = obj->getField("FlightMode");
    int chIndex = field->getOptions().indexOf(field->getValue().toString());
    if ( chIndex < field->getOptions().length()-1) {
        float valueScaled;
        int chMin = inSliders.at(chIndex)->minimum();
        int chMax = inSliders.at(chIndex)->maximum();
        int chNeutral = inSliders.at(chIndex)->value();
        int value = controlCommand->getField("Channel")->getValue(chIndex).toInt();
        if ((chMax > chMin && value >= chNeutral) || (chMin > chMax && value <= chNeutral)) {
                if (chMax != chNeutral) {
                        valueScaled = (float)(value - chNeutral) / (float)(chMax - chNeutral);
                } else {
                        valueScaled = 0;
                }
        } else {
                if (chMin != chNeutral) {
                        valueScaled = (float)(value - chNeutral) / (float)(chNeutral - chMin);
                } else {
                        valueScaled = 0;
                }
        }
        // Bound
        if (valueScaled > 1.0) {
                valueScaled = 1.0;
        } else if (valueScaled < -1.0) {
                valueScaled = -1.0;
        }
        m_config->fmsSlider->setValue(valueScaled*100);
        }
}


void ConfigServoWidget::updateChannelSlider(QSlider* slider, QLabel* min, QLabel* max, QCheckBox* rev, int value)
{

    Q_UNUSED(rev);
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


