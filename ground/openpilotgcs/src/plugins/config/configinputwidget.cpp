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

#include "configinputwidget.h"

#include "uavtalk/telemetrymanager.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

ConfigInputWidget::ConfigInputWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_config = new Ui_InputWidget();
    m_config->setupUi(this);

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

	// First of all, put all the channel widgets into lists, so that we can
    // manipulate those:


	inMaxLabels << m_config->ch0Max
			<< m_config->ch1Max
			<< m_config->ch2Max
			<< m_config->ch3Max
			<< m_config->ch4Max
			<< m_config->ch5Max
			<< m_config->ch6Max
			<< m_config->ch7Max;

	inMinLabels << m_config->ch0Min
			<< m_config->ch1Min
			<< m_config->ch2Min
			<< m_config->ch3Min
			<< m_config->ch4Min
			<< m_config->ch5Min
			<< m_config->ch6Min
			<< m_config->ch7Min;

	inNeuLabels << m_config->ch0Cur
			<< m_config->ch1Cur
			<< m_config->ch2Cur
			<< m_config->ch3Cur
			<< m_config->ch4Cur
			<< m_config->ch5Cur
			<< m_config->ch6Cur
			<< m_config->ch7Cur;

	inSliders << m_config->inSlider0
			  << m_config->inSlider1
			  << m_config->inSlider2
			  << m_config->inSlider3
			  << m_config->inSlider4
			  << m_config->inSlider5
			  << m_config->inSlider6
			  << m_config->inSlider7;

        inRevCheckboxes << m_config->ch0Rev
                        << m_config->ch1Rev
                        << m_config->ch2Rev
                        << m_config->ch3Rev
                        << m_config->ch4Rev
                        << m_config->ch5Rev
                        << m_config->ch6Rev
                        << m_config->ch7Rev;


    // Now connect the widget to the ManualControlCommand / Channel UAVObject

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

    // And the flight mode settings:
    field = obj->getField(QString("FlightModePosition"));
    m_config->fmsModePos1->addItems(field->getOptions());
    m_config->fmsModePos2->addItems(field->getOptions());
    m_config->fmsModePos3->addItems(field->getOptions());
    field = obj->getField(QString("Stabilization1Settings"));
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

    // And the Armin configurations:
    field = obj->getField(QString("Arming"));
    m_config->armControl->clear();
    m_config->armControl->addItems(field->getOptions());

    requestRCInputUpdate();

    connect(m_config->saveRCInputToSD, SIGNAL(clicked()), this, SLOT(saveRCInputObject()));
    connect(m_config->saveRCInputToRAM, SIGNAL(clicked()), this, SLOT(sendRCInputUpdate()));
    connect(m_config->getRCInputCurrent, SIGNAL(clicked()), this, SLOT(requestRCInputUpdate()));

       // Flightmode panel is connected to the same as rcinput because
    // the underlying object is the same!
    connect(m_config->saveFmsToSD, SIGNAL(clicked()), this, SLOT(saveRCInputObject()));
    connect(m_config->saveFmsToRAM, SIGNAL(clicked()), this, SLOT(sendRCInputUpdate()));
    connect(m_config->getFmsCurrent, SIGNAL(clicked()), this, SLOT(requestRCInputUpdate()));

    connect(m_config->saveArmToSD, SIGNAL(clicked()), this, SLOT(saveRCInputObject()));
    connect(m_config->saveArmToRAM, SIGNAL(clicked()), this, SLOT(sendRCInputUpdate()));
    connect(m_config->getArmCurrent, SIGNAL(clicked()), this, SLOT(requestRCInputUpdate()));

    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(requestRCInputUpdate()));

    connect(m_config->inSlider0, SIGNAL(valueChanged(int)),this, SLOT(onInSliderValueChanged0(int)));
    connect(m_config->inSlider1, SIGNAL(valueChanged(int)),this, SLOT(onInSliderValueChanged1(int)));
    connect(m_config->inSlider2, SIGNAL(valueChanged(int)),this, SLOT(onInSliderValueChanged2(int)));
    connect(m_config->inSlider3, SIGNAL(valueChanged(int)),this, SLOT(onInSliderValueChanged3(int)));
    connect(m_config->inSlider4, SIGNAL(valueChanged(int)),this, SLOT(onInSliderValueChanged4(int)));
    connect(m_config->inSlider5, SIGNAL(valueChanged(int)),this, SLOT(onInSliderValueChanged5(int)));
    connect(m_config->inSlider6, SIGNAL(valueChanged(int)),this, SLOT(onInSliderValueChanged6(int)));
    connect(m_config->inSlider7, SIGNAL(valueChanged(int)),this, SLOT(onInSliderValueChanged7(int)));

    connect(m_config->ch0Rev, SIGNAL(toggled(bool)), this, SLOT(reverseCheckboxClicked(bool)));
    connect(m_config->ch1Rev, SIGNAL(toggled(bool)), this, SLOT(reverseCheckboxClicked(bool)));
    connect(m_config->ch2Rev, SIGNAL(toggled(bool)), this, SLOT(reverseCheckboxClicked(bool)));
    connect(m_config->ch3Rev, SIGNAL(toggled(bool)), this, SLOT(reverseCheckboxClicked(bool)));
    connect(m_config->ch4Rev, SIGNAL(toggled(bool)), this, SLOT(reverseCheckboxClicked(bool)));
    connect(m_config->ch5Rev, SIGNAL(toggled(bool)), this, SLOT(reverseCheckboxClicked(bool)));
    connect(m_config->ch6Rev, SIGNAL(toggled(bool)), this, SLOT(reverseCheckboxClicked(bool)));
    connect(m_config->ch7Rev, SIGNAL(toggled(bool)), this, SLOT(reverseCheckboxClicked(bool)));

    firstUpdate = true;

    enableControls(false);

    // Listen to telemetry connection events
    if (pm) {
        TelemetryManager *tm = pm->getObject<TelemetryManager>();
        if (tm) {
            connect(tm, SIGNAL(myStart()), this, SLOT(onTelemetryStart()));
            connect(tm, SIGNAL(myStop()), this, SLOT(onTelemetryStop()));
            connect(tm, SIGNAL(connected()), this, SLOT(onTelemetryConnect()));
            connect(tm, SIGNAL(disconnected()), this, SLOT(onTelemetryDisconnect()));
        }
    }
}

ConfigInputWidget::~ConfigInputWidget()
{
   // Do nothing
}

/**
  Slot called whenever we revert a signal
  */
void ConfigInputWidget::reverseCheckboxClicked(bool state)
{
    QObject* obj = sender();
    int i = inRevCheckboxes.indexOf((QCheckBox*)obj);

    inSliders[i]->setInvertedAppearance(state);
    int max = inMaxLabels[i]->text().toInt();
    int min = inMinLabels[i]->text().toInt();
    if ((state && (max>min)) ||
        (!state && (max < min))) {
        inMaxLabels[i]->setText(QString::number(min));
        inMinLabels[i]->setText(QString::number(max));
    }
}

// ************************************
// slider value changed signals

void ConfigInputWidget::onInSliderValueChanged0(int value)
{
	inNeuLabels[0]->setText(QString::number(value));
}

void ConfigInputWidget::onInSliderValueChanged1(int value)
{
	inNeuLabels[1]->setText(QString::number(value));
}

void ConfigInputWidget::onInSliderValueChanged2(int value)
{
	inNeuLabels[2]->setText(QString::number(value));
}

void ConfigInputWidget::onInSliderValueChanged3(int value)
{
	inNeuLabels[3]->setText(QString::number(value));
}

void ConfigInputWidget::onInSliderValueChanged4(int value)
{
	inNeuLabels[4]->setText(QString::number(value));
}

void ConfigInputWidget::onInSliderValueChanged5(int value)
{
	inNeuLabels[5]->setText(QString::number(value));
}

void ConfigInputWidget::onInSliderValueChanged6(int value)
{
	inNeuLabels[6]->setText(QString::number(value));
}

void ConfigInputWidget::onInSliderValueChanged7(int value)
{
	inNeuLabels[7]->setText(QString::number(value));
}

// ************************************
// telemetry start/stop connect/disconnect signals

void ConfigInputWidget::onTelemetryStart()
{
}

void ConfigInputWidget::onTelemetryStop()
{
}

void ConfigInputWidget::onTelemetryConnect()
{
	enableControls(true);
}

void ConfigInputWidget::onTelemetryDisconnect()
{
	enableControls(false);
	m_config->doRCInputCalibration->setChecked(false);
}

// ************************************

void ConfigInputWidget::enableControls(bool enable)
{
	m_config->getRCInputCurrent->setEnabled(enable);
	m_config->saveRCInputToRAM->setEnabled(enable);
	m_config->saveRCInputToSD->setEnabled(enable);

	m_config->saveFmsToSD->setEnabled(enable);
	m_config->saveFmsToRAM->setEnabled(enable);
	m_config->getFmsCurrent->setEnabled(enable);

	m_config->saveArmToSD->setEnabled(enable);
	m_config->saveArmToRAM->setEnabled(enable);
	m_config->getArmCurrent->setEnabled(enable);


	m_config->doRCInputCalibration->setEnabled(enable);

	m_config->ch0Assign->setEnabled(enable);
	m_config->ch1Assign->setEnabled(enable);
	m_config->ch2Assign->setEnabled(enable);
	m_config->ch3Assign->setEnabled(enable);
	m_config->ch4Assign->setEnabled(enable);
	m_config->ch5Assign->setEnabled(enable);
	m_config->ch6Assign->setEnabled(enable);
	m_config->ch7Assign->setEnabled(enable);
}


/********************************
  *  Input settings
  *******************************/

/**
  Request the current config from the board
  */
void ConfigInputWidget::requestRCInputUpdate()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ManualControlSettings")));
    Q_ASSERT(obj);
    obj->requestUpdate();
    UAVObjectField *field;

    // Now update all the slider values:

    UAVObjectField *field_max = obj->getField(QString("ChannelMax"));
    UAVObjectField *field_min = obj->getField(QString("ChannelMin"));
    UAVObjectField *field_neu = obj->getField(QString("ChannelNeutral"));
    Q_ASSERT(field_max);
    Q_ASSERT(field_min);
    Q_ASSERT(field_neu);
    for (int i = 0; i < 8; i++) {
        QVariant max = field_max->getValue(i);
        QVariant min = field_min->getValue(i);
        QVariant neutral = field_neu->getValue(i);
        inMaxLabels[i]->setText(max.toString());
        inMinLabels[i]->setText(min.toString());
        if (max.toInt()> min.toInt()) {
            inRevCheckboxes[i]->setChecked(false);
            inSliders[i]->setMaximum(max.toInt());
            inSliders[i]->setMinimum(min.toInt());
        } else {
            inRevCheckboxes[i]->setChecked(true);
            inSliders[i]->setMaximum(min.toInt());
            inSliders[i]->setMinimum(max.toInt());
        }
        inSliders[i]->setValue(neutral.toInt());
    }

    // Update receiver type
    field = obj->getField(QString("InputMode"));
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
    QList<UAVObjectField *> fieldList = obj->getFields();
    foreach (UAVObjectField *field, fieldList) {
        if (field->getUnits().contains("channel"))
            assignChannel(obj, field->getName());
    }

    // Update all the flight mode settingsin the relevant tab
    field = obj->getField(QString("FlightModePosition"));
    m_config->fmsModePos1->setCurrentIndex((m_config->fmsModePos1->findText(field->getValue(0).toString())));
    m_config->fmsModePos2->setCurrentIndex((m_config->fmsModePos2->findText(field->getValue(1).toString())));
    m_config->fmsModePos3->setCurrentIndex((m_config->fmsModePos3->findText(field->getValue(2).toString())));

    field = obj->getField(QString("Stabilization1Settings"));
	m_config->fmsSsPos1Roll->setCurrentIndex(m_config->fmsSsPos1Roll->findText(field->getValue(field->getElementNames().indexOf("Roll")).toString()));
	m_config->fmsSsPos1Pitch->setCurrentIndex(m_config->fmsSsPos1Pitch->findText(field->getValue(field->getElementNames().indexOf("Pitch")).toString()));
	m_config->fmsSsPos1Yaw->setCurrentIndex(m_config->fmsSsPos1Yaw->findText(field->getValue(field->getElementNames().indexOf("Yaw")).toString()));
    field = obj->getField(QString("Stabilization2Settings"));
	m_config->fmsSsPos2Roll->setCurrentIndex(m_config->fmsSsPos2Roll->findText(field->getValue(field->getElementNames().indexOf("Roll")).toString()));
	m_config->fmsSsPos2Pitch->setCurrentIndex(m_config->fmsSsPos2Pitch->findText(field->getValue(field->getElementNames().indexOf("Pitch")).toString()));
	m_config->fmsSsPos2Yaw->setCurrentIndex(m_config->fmsSsPos2Yaw->findText(field->getValue(field->getElementNames().indexOf("Yaw")).toString()));
    field = obj->getField(QString("Stabilization3Settings"));
	m_config->fmsSsPos3Roll->setCurrentIndex(m_config->fmsSsPos3Roll->findText(field->getValue(field->getElementNames().indexOf("Roll")).toString()));
	m_config->fmsSsPos3Pitch->setCurrentIndex(m_config->fmsSsPos3Pitch->findText(field->getValue(field->getElementNames().indexOf("Pitch")).toString()));
	m_config->fmsSsPos3Yaw->setCurrentIndex(m_config->fmsSsPos3Yaw->findText(field->getValue(field->getElementNames().indexOf("Yaw")).toString()));

    // Load the arming settings
    field = obj->getField(QString("Arming"));
    m_config->armControl->setCurrentIndex(m_config->armControl->findText(field->getValue().toString()));
    field = obj->getField(QString("ArmedTimeout"));
    m_config->armTimeout->setValue(field->getValue().toInt()/1000);
}


/**
  * Sends the config to the board, without saving to the SD card
  */
void ConfigInputWidget::sendRCInputUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlSettings")));
    Q_ASSERT(obj);
    // Now update all fields from the sliders:
    QString fieldName = QString("ChannelMax");
    UAVObjectField * field = obj->getField(fieldName);
    for (int i = 0; i < 8; i++) {
        field->setValue(inMaxLabels[i]->text().toInt(), i);
    }

    fieldName = QString("ChannelMin");
    field = obj->getField(fieldName);
    for (int i = 0; i < 8; i++) {
        field->setValue(inMinLabels[i]->text().toInt(), i);
    }

    fieldName = QString("ChannelNeutral");
    field = obj->getField(fieldName);
    for (int i = 0; i < 8; i++)
        field->setValue(inSliders[i]->value(), i);

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
    field = obj->getField(QString("FlightModePosition"));
    field->setValue(m_config->fmsModePos1->currentText(),0);
    field->setValue(m_config->fmsModePos2->currentText(),1);
    field->setValue(m_config->fmsModePos3->currentText(),2);

    field = obj->getField(QString("Stabilization1Settings"));
    field->setValue(m_config->fmsSsPos1Roll->currentText(), field->getElementNames().indexOf("Roll"));
    field->setValue(m_config->fmsSsPos1Pitch->currentText(), field->getElementNames().indexOf("Pitch"));
    field->setValue(m_config->fmsSsPos1Yaw->currentText(), field->getElementNames().indexOf("Yaw"));
    field = obj->getField(QString("Stabilization2Settings"));
    field->setValue(m_config->fmsSsPos2Roll->currentText(), field->getElementNames().indexOf("Roll"));
    field->setValue(m_config->fmsSsPos2Pitch->currentText(), field->getElementNames().indexOf("Pitch"));
    field->setValue(m_config->fmsSsPos2Yaw->currentText(), field->getElementNames().indexOf("Yaw"));
    field = obj->getField(QString("Stabilization3Settings"));
    field->setValue(m_config->fmsSsPos3Roll->currentText(), field->getElementNames().indexOf("Roll"));
    field->setValue(m_config->fmsSsPos3Pitch->currentText(), field->getElementNames().indexOf("Pitch"));
    field->setValue(m_config->fmsSsPos3Yaw->currentText(), field->getElementNames().indexOf("Yaw"));

    // Save the arming settings
    field = obj->getField(QString("Arming"));
    field->setValue(m_config->armControl->currentText());
    field = obj->getField(QString("ArmedTimeout"));
    field->setValue(m_config->armTimeout->value()*1000);

    // ... and send to the OP Board
    obj->updated();

}

/**
  Sends the config to the board and request saving into the SD card
  */
void ConfigInputWidget::saveRCInputObject()
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
void ConfigInputWidget::assignChannel(UAVDataObject *obj, QString str)
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
  * Updates the slider positions and min/max values
  *
  */
void ConfigInputWidget::updateChannels(UAVObject* controlCommand)
{

    QString fieldName = QString("Connected");
    UAVObjectField *field = controlCommand->getField(fieldName);
    if (field->getValue().toBool())
        m_config->RCInputConnected->setText("RC Receiver connected");
    else
        m_config->RCInputConnected->setText("RC Receiver not connected or invalid input configuration (missing channels)");

    if (m_config->doRCInputCalibration->isChecked()) {
        if (firstUpdate) {
                // Increase the data rate from the board so that the sliders
                // move faster
                UAVObject::Metadata mdata = controlCommand->getMetadata();
                mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
                mccDataRate = mdata.flightTelemetryUpdatePeriod;
                mdata.flightTelemetryUpdatePeriod = 150;
                controlCommand->setMetadata(mdata);

                // Also protect the user by setting all values to zero
                // and making the ActuatorCommand object readonly
                UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorCommand")));
                mdata = obj->getMetadata();
                mdata.flightAccess = UAVObject::ACCESS_READONLY;
                obj->setMetadata(mdata);
                UAVObjectField *field = obj->getField("Channel");
                for (int i=0; i< field->getNumElements(); i++) {
                    field->setValue(0,i);
                }
                obj->updated();

            }

        field = controlCommand->getField(QString("Channel"));
        for (int i = 0; i < 8; i++)
            updateChannelInSlider(inSliders[i], inMinLabels[i], inMaxLabels[i], field->getValue(i).toInt(),inRevCheckboxes[i]->isChecked());
        firstUpdate = false;
        }
        else {
        if (!firstUpdate) {
            // Restore original data rate from the board:
            UAVObject::Metadata mdata = controlCommand->getMetadata();
            mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
            mdata.flightTelemetryUpdatePeriod = mccDataRate;
            controlCommand->setMetadata(mdata);

            UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorCommand")));
            mdata = obj->getMetadata();
            mdata.flightAccess = UAVObject::ACCESS_READWRITE;
            obj->setMetadata(mdata);
        }
        firstUpdate = true;
    }
    //Update the Flight mode channel slider
    UAVObject* obj = getObjectManager()->getObject("ManualControlSettings");
    // Find the channel currently assigned to flightmode
    field = obj->getField("FlightMode");
    int chIndex = field->getOptions().indexOf(field->getValue().toString());
    if (chIndex < field->getOptions().length() - 1) {
        float valueScaled;

        int chMin = inSliders[chIndex]->minimum();
        int chMax = inSliders[chIndex]->maximum();
        int chNeutral = inSliders[chIndex]->value();

        int value = controlCommand->getField("Channel")->getValue(chIndex).toInt();
        if ((chMax > chMin && value >= chNeutral) || (chMin > chMax && value <= chNeutral))
        {
                if (chMax != chNeutral)
                        valueScaled = (float)(value - chNeutral) / (float)(chMax - chNeutral);
                else
                        valueScaled = 0;
        }
        else
        {
                if (chMin != chNeutral)
                        valueScaled = (float)(value - chNeutral) / (float)(chNeutral - chMin);
                else
                        valueScaled = 0;
        }

        // Bound
		if (valueScaled >  1.0) valueScaled =  1.0;
		else
		if (valueScaled < -1.0) valueScaled = -1.0;

		m_config->fmsSlider->setValue(valueScaled * 100);
	}
}

void ConfigInputWidget::updateChannelInSlider(QSlider *slider, QLabel *min, QLabel *max, int value, bool reversed)
{
    if (!slider || !min || !max)
        return;

    if (firstUpdate) {
        // Reset all the min/max values of the progress bar since we are starting the calibration.
        slider->setMaximum(value);
        slider->setMinimum(value);
        slider->setValue(value);
        max->setText(QString::number(value));
        min->setText(QString::number(value));
        return;
    }

    if (value > 0) {
        // avoids glitches...
        if (value > slider->maximum()) {
            slider->setMaximum(value);
            if (reversed)
                min->setText(QString::number(value));
            else
                max->setText(QString::number(value));
        }
        if (value < slider->minimum()) {
            slider->setMinimum(value);
            if (reversed)
                max->setText(QString::number(value));
            else
                min->setText(QString::number(value));
        }
        slider->setValue(value);
    }
}
