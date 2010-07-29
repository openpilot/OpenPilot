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
  * Updates the slider positions and min/max values
  *
  */
void ConfigGadgetWidget::updateChannels(UAVObject* controlCommand)
{

    QString fieldName = QString("Connected");
    UAVObjectField *field = controlCommand->getField(fieldName);
    if (!field->getValue().toBool()) {
        // Currently causes a problem because throttle going too low
        // makes "Connected" go to false. Some kind of failsafe??
        firstUpdate = true;
    } else {
        fieldName = QString("Channel");
        field =  controlCommand->getField(fieldName);
        // Hey: if you find a nicer way of doing this, be my guest!
        this->updateChannelSlider(&*m_config->ch0Slider,
                            &*m_config->ch0Min,
                            &*m_config->ch0Max,
                            &*m_config->ch0Cur,
                            &*m_config->ch0Rev,field->getValue(0).toInt());
        this->updateChannelSlider(&*m_config->ch1Slider,
                            &*m_config->ch1Min,
                            &*m_config->ch1Max,
                            &*m_config->ch1Cur,
                            &*m_config->ch1Rev,field->getValue(1).toInt());
        this->updateChannelSlider(&*m_config->ch2Slider,
                            &*m_config->ch2Min,
                            &*m_config->ch2Max,
                            &*m_config->ch2Cur,
                            &*m_config->ch2Rev,field->getValue(2).toInt());
        this->updateChannelSlider(&*m_config->ch3Slider,
                            &*m_config->ch3Min,
                            &*m_config->ch3Max,
                            &*m_config->ch3Cur,
                            &*m_config->ch3Rev,field->getValue(3).toInt());
        this->updateChannelSlider(&*m_config->ch4Slider,
                            &*m_config->ch4Min,
                            &*m_config->ch4Max,
                            &*m_config->ch4Cur,
                            &*m_config->ch4Rev,field->getValue(4).toInt());
        this->updateChannelSlider(&*m_config->ch5Slider,
                            &*m_config->ch5Min,
                            &*m_config->ch5Max,
                            &*m_config->ch5Cur,
                            &*m_config->ch5Rev,field->getValue(5).toInt());
        this->updateChannelSlider(&*m_config->ch6Slider,
                            &*m_config->ch6Min,
                            &*m_config->ch6Max,
                            &*m_config->ch6Cur,
                            &*m_config->ch6Rev,field->getValue(6).toInt());
        this->updateChannelSlider(&*m_config->ch7Slider,
                            &*m_config->ch7Min,
                            &*m_config->ch7Max,
                            &*m_config->ch7Cur,
                            &*m_config->ch7Rev,field->getValue(7).toInt());
        firstUpdate = false;
    }
}

void ConfigGadgetWidget::updateChannelSlider(QSlider* slider, QLabel* min, QLabel* max, QLabel* cur, QCheckBox* rev, int value) {

    if (firstUpdate) {
        slider->setMaximum(value);
        slider->setMinimum(value);
        slider->setValue(value);
        max->setText(QString::number(value));
        min->setText(QString::number(value));
        cur->setText(QString::number(value));
        return;
    }

    if (value > slider->maximum()) {
        slider->setMaximum(value);
        max->setText(QString::number(value));
    }
    if (value < slider->minimum()) {
        slider->setMinimum(value);
        min->setText(QString::number(value));
    }
    slider->setValue(value);
    cur->setText(QString::number(value));
}


