/**
 ******************************************************************************
 *
 * @file       configcamerastabilizationwidget.cpp
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
#include "configcamerastabilizationwidget.h"

#include <QDebug>
#include <QTimer>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QThread>
#include <iostream>
#include <QUrl>
#include <QDesktopServices>

#include "camerastabsettings.h"
#include "hwsettings.h"
#include "mixersettings.h"

ConfigCameraStabilizationWidget::ConfigCameraStabilizationWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_camerastabilization = new Ui_CameraStabilizationWidget();
    m_camerastabilization->setupUi(this);

    connectUpdates();

    // Connect buttons
    connect(m_camerastabilization->camerastabilizationSaveRAM,SIGNAL(clicked()),this,SLOT(applySettings()));
    connect(m_camerastabilization->camerastabilizationSaveSD,SIGNAL(clicked()),this,SLOT(saveSettings()));
    connect(m_camerastabilization->camerastabilizationHelp, SIGNAL(clicked()), this, SLOT(openHelp()));
}

ConfigCameraStabilizationWidget::~ConfigCameraStabilizationWidget()
{
   // Do nothing
}

void ConfigCameraStabilizationWidget::connectUpdates()
{
    // Now connect the widget to the StabilizationSettings object
    connect(MixerSettings::GetInstance(getObjectManager()),SIGNAL(objectUpdated(UAVObject*)),this,SLOT(refreshValues()));
    connect(CameraStabSettings::GetInstance(getObjectManager()),SIGNAL(objectUpdated(UAVObject*)),this,SLOT(refreshValues()));
    // TODO: This will need to support both CC and OP later
    connect(HwSettings::GetInstance(getObjectManager()),SIGNAL(objectUpdated(UAVObject*)),this,SLOT(refreshValues()));
}

void ConfigCameraStabilizationWidget::disconnectUpdates()
{
    // Now connect the widget to the StabilizationSettings object
    disconnect(MixerSettings::GetInstance(getObjectManager()),SIGNAL(objectUpdated(UAVObject*)),this,SLOT(refreshValues()));
    disconnect(CameraStabSettings::GetInstance(getObjectManager()),SIGNAL(objectUpdated(UAVObject*)),this,SLOT(refreshValues()));
    // TODO: This will need to support both CC and OP later
    disconnect(HwSettings::GetInstance(getObjectManager()),SIGNAL(objectUpdated(UAVObject*)),this,SLOT(refreshValues()));
}

/**
  * @brief Populate the gui settings into the appropriate
  * UAV structures
  */
void ConfigCameraStabilizationWidget::applySettings()
{
    // Enable or disable the settings
    HwSettings * hwSettings = HwSettings::GetInstance(getObjectManager());
    HwSettings::DataFields hwSettingsData = hwSettings->getData();
    hwSettingsData.OptionalModules[HwSettings::OPTIONALMODULES_CAMERASTABILIZATION] =
            m_camerastabilization->enableCameraStabilization->isChecked() ?
                HwSettings::OPTIONALMODULES_ENABLED :
                HwSettings::OPTIONALMODULES_DISABLED;

    // Update the mixer settings
    MixerSettings * mixerSettings = MixerSettings::GetInstance(getObjectManager());
    MixerSettings::DataFields mixerSettingsData = mixerSettings->getData();
    const int NUM_MIXERS = 8;

    QComboBox * selectors[3] = {
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
    };

    for (int i = 0; i < 3; i++)
    {
        // Channel 1 is second entry, so becomes zero
        int mixerNum = selectors[i]->currentIndex() - 1;

        if ( *mixerTypes[mixerNum] != MixerSettings::MIXER1TYPE_DISABLED &&
             (*mixerTypes[mixerNum] != MixerSettings::MIXER1TYPE_CAMERAROLL + i) ) {
            // If the mixer channel already to something that isn't what we are
            // about to set it to reset to none
            selectors[i]->setCurrentIndex(0);
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

    // Update the ranges
    CameraStabSettings * cameraStab = CameraStabSettings::GetInstance(getObjectManager());
    CameraStabSettings::DataFields cameraStabData = cameraStab->getData();
    cameraStabData.OutputRange[CameraStabSettings::OUTPUTRANGE_ROLL] = m_camerastabilization->rollOutputRange->value();
    cameraStabData.OutputRange[CameraStabSettings::OUTPUTRANGE_PITCH] = m_camerastabilization->pitchOutputRange->value();
    cameraStabData.OutputRange[CameraStabSettings::OUTPUTRANGE_YAW] = m_camerastabilization->yawOutputRange->value();

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
    UAVObject * obj = HwSettings::GetInstance(getObjectManager());
    saveObjectToSD(obj);
    obj = MixerSettings::GetInstance(getObjectManager());
    saveObjectToSD(obj);
    obj = CameraStabSettings::GetInstance(getObjectManager());
    saveObjectToSD(obj);
}

void ConfigCameraStabilizationWidget::refreshValues()
{
    HwSettings * hwSettings = HwSettings::GetInstance(getObjectManager());
    HwSettings::DataFields hwSettingsData = hwSettings->getData();
    m_camerastabilization->enableCameraStabilization->setChecked(
                hwSettingsData.OptionalModules[HwSettings::OPTIONALMODULES_CAMERASTABILIZATION] ==
                HwSettings::OPTIONALMODULES_ENABLED);

    CameraStabSettings * cameraStabSettings = CameraStabSettings::GetInstance(getObjectManager());
    CameraStabSettings::DataFields cameraStab = cameraStabSettings->getData();
    m_camerastabilization->rollOutputRange->setValue(cameraStab.OutputRange[CameraStabSettings::OUTPUTRANGE_ROLL]);
    m_camerastabilization->pitchOutputRange->setValue(cameraStab.OutputRange[CameraStabSettings::OUTPUTRANGE_PITCH]);
    m_camerastabilization->yawOutputRange->setValue(cameraStab.OutputRange[CameraStabSettings::OUTPUTRANGE_YAW]);

    MixerSettings * mixerSettings = MixerSettings::GetInstance(getObjectManager());
    MixerSettings::DataFields mixerSettingsData = mixerSettings->getData();
    const int NUM_MIXERS = 8;
    QComboBox * selectors[3] = {
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
    };

    for (int i = 0; i < 3; i++)
    {
        // Default to none if not found.  Then search for any mixer channels set to
        // this
        selectors[i]->setCurrentIndex(0);
        for (int j = 0; j < NUM_MIXERS; j++)
            if (*mixerTypes[j] == (MixerSettings::MIXER1TYPE_CAMERAROLL + i) &&
                    selectors[i]->currentIndex() != (j + 1))
                selectors[i]->setCurrentIndex(j + 1);
    }
}

void ConfigCameraStabilizationWidget::openHelp()
{
    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/display/Doc/Camera+Configuration", QUrl::StrictMode) );
}

void ConfigCameraStabilizationWidget::enableControls(bool enable)
{
    m_camerastabilization->camerastabilizationSaveSD->setEnabled(enable);
    m_camerastabilization->camerastabilizationSaveRAM->setEnabled(enable);
}

/**
  @}
  @}
  */
