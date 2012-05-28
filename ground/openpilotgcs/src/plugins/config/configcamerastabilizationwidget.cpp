/**
 ******************************************************************************
 *
 * @file       configcamerastabilizationwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
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
#include "configcamerastabilizationwidget.h"
#include "camerastabsettings.h"
#include "hwsettings.h"
#include "mixersettings.h"
#include "actuatorcommand.h"

#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

ConfigCameraStabilizationWidget::ConfigCameraStabilizationWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    // TODO: this widget should use the addUAVObjectToWidgetRelation()
    m_camerastabilization = new Ui_CameraStabilizationWidget();
    m_camerastabilization->setupUi(this);

    QComboBox *outputs[3] = {
        m_camerastabilization->rollChannel,
        m_camerastabilization->pitchChannel,
        m_camerastabilization->yawChannel,
    };

    QComboBox *inputs[3] = {
        m_camerastabilization->rollInputChannel,
        m_camerastabilization->pitchInputChannel,
        m_camerastabilization->yawInputChannel,
    };

    QComboBox *stabilizationMode[3] = {
        m_camerastabilization->rollStabilizationMode,
        m_camerastabilization->pitchStabilizationMode,
        m_camerastabilization->yawStabilizationMode,
    };

    CameraStabSettings *cameraStab = CameraStabSettings::GetInstance(getObjectManager());
    CameraStabSettings::DataFields cameraStabData = cameraStab->getData();

    for (int i = 0; i < 3; i++) {
        outputs[i]->clear();
        outputs[i]->addItem("None");
        for (quint32 j = 0; j < ActuatorCommand::CHANNEL_NUMELEM; j++)
            outputs[i]->addItem(QString("Channel %1").arg(j+1));

        UAVObjectField *field;

        field = cameraStab->getField("Input");
        Q_ASSERT(field);
        inputs[i]->clear();
        inputs[i]->addItems(field->getOptions());
        inputs[i]->setCurrentIndex(cameraStabData.Input[i]);

        field = cameraStab->getField("StabilizationMode");
        Q_ASSERT(field);
        stabilizationMode[i]->clear();
        stabilizationMode[i]->addItems(field->getOptions());
        stabilizationMode[i]->setCurrentIndex(cameraStabData.StabilizationMode[i]);
    }

    connectUpdates();

    // Connect buttons
    connect(m_camerastabilization->camerastabilizationResetToDefaults, SIGNAL(clicked()), this, SLOT(resetToDefaults()));
    connect(m_camerastabilization->camerastabilizationSaveRAM, SIGNAL(clicked()), this, SLOT(applySettings()));
    connect(m_camerastabilization->camerastabilizationSaveSD, SIGNAL(clicked()), this, SLOT(saveSettings()));
    connect(m_camerastabilization->camerastabilizationHelp, SIGNAL(clicked()), this, SLOT(openHelp()));

    disableMouseWheelEvents();
}

ConfigCameraStabilizationWidget::~ConfigCameraStabilizationWidget()
{
   // Do nothing
}

void ConfigCameraStabilizationWidget::connectUpdates()
{
    // Now connect the widget to the StabilizationSettings object
    connect(MixerSettings::GetInstance(getObjectManager()), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshValues()));
    connect(CameraStabSettings::GetInstance(getObjectManager()), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshValues()));
    // TODO: This will need to support both CC and OP later
    connect(HwSettings::GetInstance(getObjectManager()), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshValues()));
}

void ConfigCameraStabilizationWidget::disconnectUpdates()
{
    // Now connect the widget to the StabilizationSettings object
    disconnect(MixerSettings::GetInstance(getObjectManager()), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshValues()));
    disconnect(CameraStabSettings::GetInstance(getObjectManager()), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshValues()));
    // TODO: This will need to support both CC and OP later
    disconnect(HwSettings::GetInstance(getObjectManager()), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshValues()));
}

/**
  * @brief Populate the gui settings into the appropriate
  * UAV structures
  */
void ConfigCameraStabilizationWidget::applySettings()
{
    // Enable or disable the settings
    HwSettings *hwSettings = HwSettings::GetInstance(getObjectManager());
    HwSettings::DataFields hwSettingsData = hwSettings->getData();
    hwSettingsData.OptionalModules[HwSettings::OPTIONALMODULES_CAMERASTAB] =
            m_camerastabilization->enableCameraStabilization->isChecked() ?
                HwSettings::OPTIONALMODULES_ENABLED :
                HwSettings::OPTIONALMODULES_DISABLED;

    // Update the mixer settings
    MixerSettings *mixerSettings = MixerSettings::GetInstance(getObjectManager());
    MixerSettings::DataFields mixerSettingsData = mixerSettings->getData();
    const int NUM_MIXERS = 10;

    QComboBox *outputs[3] = {
        m_camerastabilization->rollChannel,
        m_camerastabilization->pitchChannel,
        m_camerastabilization->yawChannel,
    };

    // TODO: Need to reformat object so types are an
    // array themselves.  This gets really awkward
    quint8 * mixerTypes[NUM_MIXERS] = {
        &mixerSettingsData.Mixer1Type,
        &mixerSettingsData.Mixer2Type,
        &mixerSettingsData.Mixer3Type,
        &mixerSettingsData.Mixer4Type,
        &mixerSettingsData.Mixer5Type,
        &mixerSettingsData.Mixer6Type,
        &mixerSettingsData.Mixer7Type,
        &mixerSettingsData.Mixer8Type,
        &mixerSettingsData.Mixer9Type,
        &mixerSettingsData.Mixer10Type,
    };

    m_camerastabilization->message->setText("");
    for (int i = 0; i < 3; i++)
    {
        // Channel 1 is second entry, so becomes zero
        int mixerNum = outputs[i]->currentIndex() - 1;

        if ( mixerNum >= 0 && // Short circuit in case of none
             *mixerTypes[mixerNum] != MixerSettings::MIXER1TYPE_DISABLED &&
             (*mixerTypes[mixerNum] != MixerSettings::MIXER1TYPE_CAMERAROLL + i) ) {
            // If the mixer channel already to something that isn't what we are
            // about to set it to reset to none
            outputs[i]->setCurrentIndex(0);
            m_camerastabilization->message->setText("One of the channels is already assigned, reverted to none");
        } else {
            // Make sure no other channels have this output set
            for (int j = 0; j < NUM_MIXERS; j++)
                if (*mixerTypes[j] == (MixerSettings::MIXER1TYPE_CAMERAROLL + i))
                    *mixerTypes[j] = MixerSettings::MIXER1TYPE_DISABLED;

            // If this channel is assigned to one of the outputs that is not disabled
            // set it
            if(mixerNum >= 0 && mixerNum < NUM_MIXERS)
                *mixerTypes[mixerNum] = MixerSettings::MIXER1TYPE_CAMERAROLL + i;
        }
    }

    // Update the settings
    CameraStabSettings *cameraStab = CameraStabSettings::GetInstance(getObjectManager());
    CameraStabSettings::DataFields cameraStabData = cameraStab->getData();

    cameraStabData.OutputRange[CameraStabSettings::OUTPUTRANGE_ROLL] = m_camerastabilization->rollOutputRange->value();
    cameraStabData.OutputRange[CameraStabSettings::OUTPUTRANGE_PITCH] = m_camerastabilization->pitchOutputRange->value();
    cameraStabData.OutputRange[CameraStabSettings::OUTPUTRANGE_YAW] = m_camerastabilization->yawOutputRange->value();

    cameraStabData.Input[CameraStabSettings::INPUT_ROLL] = m_camerastabilization->rollInputChannel->currentIndex();
    cameraStabData.Input[CameraStabSettings::INPUT_PITCH] = m_camerastabilization->pitchInputChannel->currentIndex();
    cameraStabData.Input[CameraStabSettings::INPUT_YAW] = m_camerastabilization->yawInputChannel->currentIndex();

    cameraStabData.StabilizationMode[CameraStabSettings::STABILIZATIONMODE_ROLL] = m_camerastabilization->rollStabilizationMode->currentIndex();
    cameraStabData.StabilizationMode[CameraStabSettings::STABILIZATIONMODE_PITCH] = m_camerastabilization->pitchStabilizationMode->currentIndex();
    cameraStabData.StabilizationMode[CameraStabSettings::STABILIZATIONMODE_YAW] = m_camerastabilization->yawStabilizationMode->currentIndex();

    cameraStabData.InputRange[CameraStabSettings::INPUTRANGE_ROLL] = m_camerastabilization->rollInputRange->value();
    cameraStabData.InputRange[CameraStabSettings::INPUTRANGE_PITCH] = m_camerastabilization->pitchInputRange->value();
    cameraStabData.InputRange[CameraStabSettings::INPUTRANGE_YAW] = m_camerastabilization->yawInputRange->value();

    cameraStabData.InputRate[CameraStabSettings::INPUTRATE_ROLL] = m_camerastabilization->rollInputRate->value();
    cameraStabData.InputRate[CameraStabSettings::INPUTRATE_PITCH] = m_camerastabilization->pitchInputRate->value();
    cameraStabData.InputRate[CameraStabSettings::INPUTRATE_YAW] = m_camerastabilization->yawInputRate->value();

    cameraStabData.ResponseTime[CameraStabSettings::RESPONSETIME_ROLL] = m_camerastabilization->rollResponseTime->value();
    cameraStabData.ResponseTime[CameraStabSettings::RESPONSETIME_PITCH] = m_camerastabilization->pitchResponseTime->value();
    cameraStabData.ResponseTime[CameraStabSettings::RESPONSETIME_YAW] = m_camerastabilization->yawResponseTime->value();

    cameraStabData.MaxAxisLockRate = m_camerastabilization->MaxAxisLockRate->value();

    // Because multiple objects are updated, and all of them trigger the callback
    // they must be done together (if update one then load settings from second
    // the first update would wipe the UI controls).  However to be extra cautious
    // I'm also disabling updates during the setting to the UAVObjects
    disconnectUpdates();
    hwSettings->setData(hwSettingsData);
    mixerSettings->setData(mixerSettingsData);
    cameraStab->setData(cameraStabData);
    connectUpdates();
}

/**
  * Push settings into UAV objects then save them
  */
void ConfigCameraStabilizationWidget::saveSettings()
{
    applySettings();
    UAVObject *obj = HwSettings::GetInstance(getObjectManager());
    saveObjectToSD(obj);
    obj = MixerSettings::GetInstance(getObjectManager());
    saveObjectToSD(obj);
    obj = CameraStabSettings::GetInstance(getObjectManager());
    saveObjectToSD(obj);
}

/**
  * Refresh UI with new settings of CameraStabSettings object
  * (either from active configuration or just loaded defaults
  * to be applied or saved)
  */
void ConfigCameraStabilizationWidget::refreshUIValues(CameraStabSettings::DataFields &cameraStabData)
{
    m_camerastabilization->rollOutputRange->setValue(cameraStabData.OutputRange[CameraStabSettings::OUTPUTRANGE_ROLL]);
    m_camerastabilization->pitchOutputRange->setValue(cameraStabData.OutputRange[CameraStabSettings::OUTPUTRANGE_PITCH]);
    m_camerastabilization->yawOutputRange->setValue(cameraStabData.OutputRange[CameraStabSettings::OUTPUTRANGE_YAW]);

    m_camerastabilization->rollInputChannel->setCurrentIndex(cameraStabData.Input[CameraStabSettings::INPUT_ROLL]);
    m_camerastabilization->pitchInputChannel->setCurrentIndex(cameraStabData.Input[CameraStabSettings::INPUT_PITCH]);
    m_camerastabilization->yawInputChannel->setCurrentIndex(cameraStabData.Input[CameraStabSettings::INPUT_YAW]);

    m_camerastabilization->rollStabilizationMode->setCurrentIndex(cameraStabData.StabilizationMode[CameraStabSettings::STABILIZATIONMODE_ROLL]);
    m_camerastabilization->pitchStabilizationMode->setCurrentIndex(cameraStabData.StabilizationMode[CameraStabSettings::STABILIZATIONMODE_PITCH]);
    m_camerastabilization->yawStabilizationMode->setCurrentIndex(cameraStabData.StabilizationMode[CameraStabSettings::STABILIZATIONMODE_YAW]);

    m_camerastabilization->rollInputRange->setValue(cameraStabData.InputRange[CameraStabSettings::INPUTRANGE_ROLL]);
    m_camerastabilization->pitchInputRange->setValue(cameraStabData.InputRange[CameraStabSettings::INPUTRANGE_PITCH]);
    m_camerastabilization->yawInputRange->setValue(cameraStabData.InputRange[CameraStabSettings::INPUTRANGE_YAW]);

    m_camerastabilization->rollInputRate->setValue(cameraStabData.InputRate[CameraStabSettings::INPUTRATE_ROLL]);
    m_camerastabilization->pitchInputRate->setValue(cameraStabData.InputRate[CameraStabSettings::INPUTRATE_PITCH]);
    m_camerastabilization->yawInputRate->setValue(cameraStabData.InputRate[CameraStabSettings::INPUTRATE_YAW]);

    m_camerastabilization->rollResponseTime->setValue(cameraStabData.ResponseTime[CameraStabSettings::RESPONSETIME_ROLL]);
    m_camerastabilization->pitchResponseTime->setValue(cameraStabData.ResponseTime[CameraStabSettings::RESPONSETIME_PITCH]);
    m_camerastabilization->yawResponseTime->setValue(cameraStabData.ResponseTime[CameraStabSettings::RESPONSETIME_YAW]);

    m_camerastabilization->MaxAxisLockRate->setValue(cameraStabData.MaxAxisLockRate);
}

void ConfigCameraStabilizationWidget::refreshValues()
{
    HwSettings *hwSettings = HwSettings::GetInstance(getObjectManager());
    HwSettings::DataFields hwSettingsData = hwSettings->getData();
    m_camerastabilization->enableCameraStabilization->setChecked(
                hwSettingsData.OptionalModules[HwSettings::OPTIONALMODULES_CAMERASTAB] ==
                HwSettings::OPTIONALMODULES_ENABLED);

    CameraStabSettings *cameraStabSettings = CameraStabSettings::GetInstance(getObjectManager());
    CameraStabSettings::DataFields cameraStabData = cameraStabSettings->getData();
    refreshUIValues(cameraStabData);

    MixerSettings *mixerSettings = MixerSettings::GetInstance(getObjectManager());
    MixerSettings::DataFields mixerSettingsData = mixerSettings->getData();
    const int NUM_MIXERS = 10;
    QComboBox *outputs[3] = {
        m_camerastabilization->rollChannel,
        m_camerastabilization->pitchChannel,
        m_camerastabilization->yawChannel
    };

    // TODO: Need to reformat object so types are an
    // array themselves.  This gets really awkward
    quint8 * mixerTypes[NUM_MIXERS] = {
        &mixerSettingsData.Mixer1Type,
        &mixerSettingsData.Mixer2Type,
        &mixerSettingsData.Mixer3Type,
        &mixerSettingsData.Mixer4Type,
        &mixerSettingsData.Mixer5Type,
        &mixerSettingsData.Mixer6Type,
        &mixerSettingsData.Mixer7Type,
        &mixerSettingsData.Mixer8Type,
        &mixerSettingsData.Mixer9Type,
        &mixerSettingsData.Mixer10Type,
    };

    for (int i = 0; i < 3; i++)
    {
        // Default to none if not found.  Then search for any mixer channels set to
        // this
        outputs[i]->setCurrentIndex(0);
        for (int j = 0; j < NUM_MIXERS; j++)
            if (*mixerTypes[j] == (MixerSettings::MIXER1TYPE_CAMERAROLL + i) &&
                    outputs[i]->currentIndex() != (j + 1))
                outputs[i]->setCurrentIndex(j + 1);
    }
}

void ConfigCameraStabilizationWidget::resetToDefaults()
{
    CameraStabSettings cameraStabDefaults;
    CameraStabSettings::DataFields defaults = cameraStabDefaults.getData();
    refreshUIValues(defaults);
}

void ConfigCameraStabilizationWidget::openHelp()
{
    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/display/Doc/Camera+Stabilization+Configuration", QUrl::StrictMode) );
}

void ConfigCameraStabilizationWidget::enableControls(bool enable)
{
    m_camerastabilization->camerastabilizationResetToDefaults->setEnabled(enable);
    m_camerastabilization->camerastabilizationSaveSD->setEnabled(enable);
    m_camerastabilization->camerastabilizationSaveRAM->setEnabled(enable);
}

/**
  @}
  @}
  */
