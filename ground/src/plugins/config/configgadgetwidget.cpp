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

    // Now connect the widget to the ManualControlCommand / Channel UAVObject
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlCommand")));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateChannels(UAVObject*)));

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


