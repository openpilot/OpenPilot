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
    m_camerastabilization = new Ui_CameraStabilizationWidget();
    m_camerastabilization->setupUi(this);

    // These comboboxes require special processing
    QComboBox *outputs[3] = {
        m_camerastabilization->rollChannel,
        m_camerastabilization->pitchChannel,
        m_camerastabilization->yawChannel,
    };

    for (int i = 0; i < 3; i++) {
        outputs[i]->clear();
        outputs[i]->addItem("None");
        for (quint32 j = 0; j < ActuatorCommand::CHANNEL_NUMELEM; j++)
            outputs[i]->addItem(QString("Channel %1").arg(j+1));
    }

    autoLoadWidgets();
    populateWidgets();
    refreshWidgetsValues();
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
    // TODO: This will need to support both CC and OP later
    connect(HwSettings::GetInstance(getObjectManager()), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshValues()));
}

void ConfigCameraStabilizationWidget::disconnectUpdates()
{
    // Now connect the widget to the StabilizationSettings object
    disconnect(MixerSettings::GetInstance(getObjectManager()), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshValues()));
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

    // Because multiple objects are updated, and all of them trigger the callback
    // they must be done together (if update one then load settings from second
    // the first update would wipe the UI controls).  However to be extra cautious
    // I'm also disabling updates during the setting to the UAVObjects
    disconnectUpdates();
    hwSettings->setData(hwSettingsData);
    mixerSettings->setData(mixerSettingsData);
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

void ConfigCameraStabilizationWidget::refreshValues()
{
    HwSettings *hwSettings = HwSettings::GetInstance(getObjectManager());
    HwSettings::DataFields hwSettingsData = hwSettings->getData();
    m_camerastabilization->enableCameraStabilization->setChecked(
                hwSettingsData.OptionalModules[HwSettings::OPTIONALMODULES_CAMERASTAB] ==
                HwSettings::OPTIONALMODULES_ENABLED);

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

/**
  @}
  @}
  */
