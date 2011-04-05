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

#include "configoutputwidget.h"

#include "uavtalk/telemetrymanager.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QMessageBox>

ConfigOutputWidget::ConfigOutputWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_config = new Ui_OutputWidget();
    m_config->setupUi(this);

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

	// First of all, put all the channel widgets into lists, so that we can
    // manipulate those:

	// NOTE: for historical reasons, we have objects below called ch0 to ch7, but the convention for OP is Channel 1 to Channel 8.
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

	links << m_config->ch0Link
			<< m_config->ch1Link
			<< m_config->ch2Link
			<< m_config->ch3Link
			<< m_config->ch4Link
			<< m_config->ch5Link
			<< m_config->ch6Link
			<< m_config->ch7Link;

	UAVDataObject * obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    QList<UAVObjectField*> fieldList = obj->getFields();
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

	for (int i = 0; i < links.count(); i++)
		links[i]->setChecked(false);
	for (int i = 0; i < links.count(); i++)
		connect(links[i], SIGNAL(toggled(bool)), this, SLOT(linkToggled(bool)));

	requestRCOutputUpdate();

    connect(m_config->saveRCOutputToSD, SIGNAL(clicked()), this, SLOT(saveRCOutputObject()));
    connect(m_config->saveRCOutputToRAM, SIGNAL(clicked()), this, SLOT(sendRCOutputUpdate()));
    connect(m_config->getRCOutputCurrent, SIGNAL(clicked()), this, SLOT(requestRCOutputUpdate()));

    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(requestRCOutputUpdate()));

    firstUpdate = true;

	enableControls(false);

	// Listen to telemetry connection events
	if (pm)
	{
		TelemetryManager *tm = pm->getObject<TelemetryManager>();
		if (tm)
		{
			connect(tm, SIGNAL(myStart()), this, SLOT(onTelemetryStart()));
			connect(tm, SIGNAL(myStop()), this, SLOT(onTelemetryStop()));
			connect(tm, SIGNAL(connected()), this, SLOT(onTelemetryConnect()));
			connect(tm, SIGNAL(disconnected()), this, SLOT(onTelemetryDisconnect()));
		}
	}
}

ConfigOutputWidget::~ConfigOutputWidget()
{
   // Do nothing
}



// ************************************
// telemetry start/stop connect/disconnect signals

void ConfigOutputWidget::onTelemetryStart()
{
}

void ConfigOutputWidget::onTelemetryStop()
{
}

void ConfigOutputWidget::onTelemetryConnect()
{
	enableControls(true);
}

void ConfigOutputWidget::onTelemetryDisconnect()
{
	enableControls(false);
}

// ************************************

void ConfigOutputWidget::enableControls(bool enable)
{
	m_config->saveRCOutputToSD->setEnabled(enable);
	m_config->saveRCOutputToRAM->setEnabled(enable);
	m_config->getRCOutputCurrent->setEnabled(enable);
}

// ************************************

/**
  Toggles the channel linked state for use in testing mode
  */
void ConfigOutputWidget::linkToggled(bool state)
{
	// find the minimum slider value for the linked ones
	int min = 10000;
	int linked_count = 0;
	for (int i = 0; i < outSliders.count(); i++)
	{
		if (!links[i]->checkState()) continue;
		int value = outSliders[i]->value();
		if (min > value) min = value;
		linked_count++;
	}

	if (linked_count <= 0)
		return;		// no linked channels

	if (!m_config->channelOutTest->checkState())
		return;	// we are not in Test Output mode

	// set the linked channels to the same value
	for (int i = 0; i < outSliders.count(); i++)
	{
		if (!links[i]->checkState()) continue;
		outSliders[i]->setValue(min);
	}
}

/**
  Toggles the channel testing mode by making the GCS take over
  the ActuatorCommand objects
  */
void ConfigOutputWidget::runChannelTests(bool state)
{
    // Confirm this is definitely what they want
    if(state) {
        QMessageBox mbox;
        mbox.setText(QString(tr("This option will requires you to be in the armed state and will start your motors by the amount selected on the sliders.  It is recommended to remove any blades from motors.  Are you sure you want to do this?")));
        mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int retval = mbox.exec();
        if(retval != QMessageBox::Yes) {
            state = false;
            qDebug() << "Cancelled";
            m_config->channelOutTest->setChecked(false);
            return;
        }
    }

    qDebug() << "Running with state " << state;
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

/**
  * Set the dropdown option for a channel output assignement
  */
void ConfigOutputWidget::assignOutputChannel(UAVDataObject *obj, QString str)
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
  Sends the channel value to the UAV to move the servo.
  Returns immediately if we are not in testing mode
  */
void ConfigOutputWidget::sendChannelTest(int value)
{
	int in_value = value;

	QSlider *ob = (QSlider *)QObject::sender();
	if (!ob) return;
    int index = outSliders.indexOf(ob);
	if (index < 0) return;

	if (reversals[index]->isChecked())
		value = outMin[index]->value() - value + outMax[index]->value();	// the chsnnel is reversed

	// update the label
	outLabels[index]->setText(QString::number(value));

	if (links[index]->checkState())
	{	// the channel is linked to other channels
		// set the linked channels to the same value
		for (int i = 0; i < outSliders.count(); i++)
		{
			if (i == index) continue;
			if (!links[i]->checkState()) continue;

			int val = in_value;
			if (val < outSliders[i]->minimum()) val = outSliders[i]->minimum();
			if (val > outSliders[i]->maximum()) val = outSliders[i]->maximum();

			if (outSliders[i]->value() == val) continue;

			outSliders[i]->setValue(val);
			outLabels[i]->setText(QString::number(val));
		}
	}

	if (!m_config->channelOutTest->isChecked())
		return;

	UAVDataObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorCommand")));
	if (!obj) return;
	UAVObjectField *channel = obj->getField("Channel");
	if (!channel) return;
	channel->setValue(value, index);
    obj->updated();
}



/********************************
  *  Output settings
  *******************************/

/**
  Request the current config from the board (RC Output)
  */
void ConfigOutputWidget::requestRCOutputUpdate()
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
void ConfigOutputWidget::sendRCOutputUpdate()
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
void ConfigOutputWidget::saveRCOutputObject()
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
void ConfigOutputWidget::setChOutRange()
{
    QSpinBox *spinbox = (QSpinBox*)QObject::sender();

    int index = outMin.indexOf(spinbox); // This is the channel number
    if (index < 0)
        index = outMax.indexOf(spinbox); // We can't know if the signal came from min or max

    QSlider *slider = outSliders[index];

    int oldMini = slider->minimum();
    int oldMaxi = slider->maximum();

    if (outMin[index]->value()<outMax[index]->value())
    {
        slider->setRange(outMin[index]->value(), outMax[index]->value());
        reversals[index]->setChecked(false);
    }
    else
    {
        slider->setRange(outMax[index]->value(), outMin[index]->value());
        reversals[index]->setChecked(true);
    }

    if (slider->value() == oldMini)
        slider->setValue(slider->minimum());

//    if (slider->value() == oldMaxi)
//        slider->setValue(slider->maximum());  // this can be dangerous if it happens to be controlling a motor at the time!
}

/**
  Reverses the channel when the checkbox is clicked
  */
void ConfigOutputWidget::reverseChannel(bool state)
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



