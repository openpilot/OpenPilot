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
    stabSettings = StabilizationSettings::GetInstance(getObjectManager());

    m_stabilization = new Ui_StabilizationWidget();
    m_stabilization->setupUi(this);

    // Now connect the widget to the ManualControlCommand / Channel UAVObject
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("StabilizationSettings")));

    requestStabilizationUpdate();
    connect(m_stabilization->saveStabilizationToSD, SIGNAL(clicked()), this, SLOT(saveStabilizationUpdate()));
    connect(m_stabilization->saveStabilizationToRAM, SIGNAL(clicked()), this, SLOT(sendStabilizationUpdate()));
    connect(m_stabilization->getStabilizationCurrent, SIGNAL(clicked()), this, SLOT(requestStabilizationUpdate()));

    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(requestStabilizationUpdate()));

    // Create a timer to regularly send the object update in case
    // we want realtime updates.
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(sendStabilizationUpdate()));
    connect(m_stabilization->realTimeUpdates, SIGNAL(toggled(bool)), this, SLOT(realtimeUpdateToggle(bool)));

    // Connect the updates of the stab values
    connect(m_stabilization->rateRollKp, SIGNAL(valueChanged(double)), this, SLOT(updateRateRollKP(double)));
    connect(m_stabilization->rateRollKi, SIGNAL(valueChanged(double)), this, SLOT(updateRateRollKI(double)));
    connect(m_stabilization->rateRollILimit, SIGNAL(valueChanged(double)), this, SLOT(updateRateRollILimit(double)));

    connect(m_stabilization->ratePitchKp, SIGNAL(valueChanged(double)), this, SLOT(updateRatePitchKP(double)));
    connect(m_stabilization->ratePitchKi, SIGNAL(valueChanged(double)), this, SLOT(updateRatePitchKI(double)));
    connect(m_stabilization->ratePitchILimit, SIGNAL(valueChanged(double)), this, SLOT(updateRatePitchILimit(double)));

    connect(m_stabilization->rollKp, SIGNAL(valueChanged(double)), this, SLOT(updateRollKP(double)));
    connect(m_stabilization->rollKi, SIGNAL(valueChanged(double)), this, SLOT(updateRollKI(double)));
    connect(m_stabilization->rollILimit, SIGNAL(valueChanged(double)), this, SLOT(updateRollILimit(double)));

    connect(m_stabilization->pitchKp, SIGNAL(valueChanged(double)), this, SLOT(updatePitchKP(double)));
    connect(m_stabilization->pitchKi, SIGNAL(valueChanged(double)), this, SLOT(updatePitchKI(double)));
    connect(m_stabilization->pitchILimit, SIGNAL(valueChanged(double)), this, SLOT(updatePitchILimit(double)));

}

ConfigStabilizationWidget::~ConfigStabilizationWidget()
{
   // Do nothing
}


void ConfigStabilizationWidget::updateRateRollKP(double val)
{
    if (m_stabilization->linkRateRP->isChecked()) {
        m_stabilization->ratePitchKp->setValue(val);
    }
}

void ConfigStabilizationWidget::updateRateRollKI(double val)
{
    if (m_stabilization->linkRateRP->isChecked()) {
        m_stabilization->ratePitchKi->setValue(val);
    }
}

void ConfigStabilizationWidget::updateRateRollILimit(double val)
{
    if (m_stabilization->linkRateRP->isChecked()) {
        m_stabilization->ratePitchILimit->setValue(val);
    }
}

void ConfigStabilizationWidget::updateRatePitchKP(double val)
{
    if (m_stabilization->linkRateRP->isChecked()) {
        m_stabilization->rateRollKp->setValue(val);
    }
}

void ConfigStabilizationWidget::updateRatePitchKI(double val)
{
    if (m_stabilization->linkRateRP->isChecked()) {
        m_stabilization->rateRollKi->setValue(val);
    }
}

void ConfigStabilizationWidget::updateRatePitchILimit(double val)
{
    if (m_stabilization->linkRateRP->isChecked()) {
        m_stabilization->rateRollILimit->setValue(val);
    }
}

void ConfigStabilizationWidget::updateRollKP(double val)
{
    if (m_stabilization->linkAttitudeRP->isChecked()) {
        m_stabilization->pitchKp->setValue(val);
    }
}

void ConfigStabilizationWidget::updateRollKI(double val)
{
    if (m_stabilization->linkAttitudeRP->isChecked()) {
        m_stabilization->pitchKi->setValue(val);
    }

}

void ConfigStabilizationWidget::updateRollILimit(double val)
{
    if (m_stabilization->linkAttitudeRP->isChecked()) {
        m_stabilization->pitchILimit->setValue(val);
    }
}

void ConfigStabilizationWidget::updatePitchKP(double val)
{
    if (m_stabilization->linkAttitudeRP->isChecked()) {
        m_stabilization->rollKp->setValue(val);
    }
}

void ConfigStabilizationWidget::updatePitchKI(double val)
{
    if (m_stabilization->linkAttitudeRP->isChecked()) {
        m_stabilization->rollKi->setValue(val);
    }

}

void ConfigStabilizationWidget::updatePitchILimit(double val)
{
    if (m_stabilization->linkAttitudeRP->isChecked()) {
        m_stabilization->rollILimit->setValue(val);
    }
}


/*******************************
 * Stabilization Settings
 *****************************/

/**
  Request stabilization settings from the board
  */
void ConfigStabilizationWidget::requestStabilizationUpdate()
{
    stabSettings->requestUpdate();
    StabilizationSettings::DataFields stabData = stabSettings->getData();
    // Now fill in all the fields, this is fairly tedious:
    m_stabilization->rateRollKp->setValue(stabData.RollRatePI[StabilizationSettings::ROLLRATEPI_KP]);
    m_stabilization->rateRollKi->setValue(stabData.RollRatePI[StabilizationSettings::ROLLRATEPI_KI]);
    m_stabilization->rateRollILimit->setValue(stabData.RollRatePI[StabilizationSettings::ROLLRATEPI_ILIMIT]);

    m_stabilization->ratePitchKp->setValue(stabData.PitchRatePI[StabilizationSettings::PITCHRATEPI_KP]);
    m_stabilization->ratePitchKi->setValue(stabData.PitchRatePI[StabilizationSettings::PITCHRATEPI_KI]);
    m_stabilization->ratePitchILimit->setValue(stabData.PitchRatePI[StabilizationSettings::PITCHRATEPI_ILIMIT]);

    m_stabilization->rateYawKp->setValue(stabData.YawRatePI[StabilizationSettings::YAWRATEPI_KP]);
    m_stabilization->rateYawKi->setValue(stabData.YawRatePI[StabilizationSettings::YAWRATEPI_KI]);
    m_stabilization->rateYawILimit->setValue(stabData.YawRatePI[StabilizationSettings::YAWRATEPI_ILIMIT]);

    m_stabilization->rollKp->setValue(stabData.RollPI[StabilizationSettings::ROLLPI_KP]);
    m_stabilization->rollKi->setValue(stabData.RollPI[StabilizationSettings::ROLLPI_KI]);
    m_stabilization->rollILimit->setValue(stabData.RollPI[StabilizationSettings::ROLLPI_ILIMIT]);

    m_stabilization->pitchKp->setValue(stabData.PitchPI[StabilizationSettings::PITCHPI_KP]);
    m_stabilization->pitchKi->setValue(stabData.PitchPI[StabilizationSettings::PITCHPI_KI]);
    m_stabilization->pitchILimit->setValue(stabData.PitchPI[StabilizationSettings::PITCHPI_ILIMIT]);

    m_stabilization->yawKp->setValue(stabData.YawPI[StabilizationSettings::YAWPI_KP]);
    m_stabilization->yawKi->setValue(stabData.YawPI[StabilizationSettings::YAWPI_KI]);
    m_stabilization->yawILimit->setValue(stabData.YawPI[StabilizationSettings::YAWPI_ILIMIT]);

    m_stabilization->rollMax->setValue(stabData.RollMax);
    m_stabilization->pitchMax->setValue(stabData.PitchMax);
    m_stabilization->yawMax->setValue(stabData.YawMax);

}


/**
  Send telemetry settings to the board
  */

void ConfigStabilizationWidget::sendStabilizationUpdate()
{
    StabilizationSettings::DataFields stabData = stabSettings->getData();

    stabData.RollRatePI[StabilizationSettings::ROLLRATEPI_KP] = m_stabilization->rateRollKp->value();
    stabData.RollRatePI[StabilizationSettings::ROLLRATEPI_KI] = m_stabilization->rateRollKi->value();
    stabData.RollRatePI[StabilizationSettings::ROLLRATEPI_ILIMIT] = m_stabilization->rateRollILimit->value();

    stabData.PitchRatePI[StabilizationSettings::PITCHRATEPI_KP] = m_stabilization->ratePitchKp->value();
    stabData.PitchRatePI[StabilizationSettings::PITCHRATEPI_KI] = m_stabilization->ratePitchKi->value();
    stabData.PitchRatePI[StabilizationSettings::PITCHRATEPI_ILIMIT] = m_stabilization->ratePitchILimit->value();

    stabData.YawRatePI[StabilizationSettings::YAWRATEPI_KP] = m_stabilization->rateYawKp->value();
    stabData.YawRatePI[StabilizationSettings::YAWRATEPI_KI] = m_stabilization->rateYawKi->value();
    stabData.YawRatePI[StabilizationSettings::YAWRATEPI_ILIMIT] = m_stabilization->rateYawILimit->value();

    stabData.RollPI[StabilizationSettings::ROLLPI_KP] = m_stabilization->rollKp->value();
    stabData.RollPI[StabilizationSettings::ROLLPI_KI] = m_stabilization->rollKi->value();
    stabData.RollPI[StabilizationSettings::ROLLPI_ILIMIT] = m_stabilization->rollILimit->value();

    stabData.PitchPI[StabilizationSettings::PITCHPI_KP] = m_stabilization->pitchKp->value();
    stabData.PitchPI[StabilizationSettings::PITCHPI_KI] = m_stabilization->pitchKi->value();
    stabData.PitchPI[StabilizationSettings::PITCHPI_ILIMIT] = m_stabilization->pitchILimit->value();

    stabData.YawPI[StabilizationSettings::YAWPI_KP] = m_stabilization->yawKp->value();
    stabData.YawPI[StabilizationSettings::YAWPI_KI] = m_stabilization->yawKi->value();
    stabData.YawPI[StabilizationSettings::YAWPI_ILIMIT] = m_stabilization->yawILimit->value();

    stabData.RollMax = m_stabilization->rollMax->value();
    stabData.PitchMax = m_stabilization->pitchMax->value();
    stabData.YawMax = m_stabilization->yawMax->value();

    stabSettings->setData(stabData); // this is atomic
}


/**
  Send telemetry settings to the board and request saving to SD card
  */

void ConfigStabilizationWidget::saveStabilizationUpdate()
{
    // Send update so that the latest value is saved
    sendStabilizationUpdate();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("StabilizationSettings")));
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);
}


void ConfigStabilizationWidget::realtimeUpdateToggle(bool state)
{
    if (state)
        updateTimer.start(300);
    else
        updateTimer.stop();
}
