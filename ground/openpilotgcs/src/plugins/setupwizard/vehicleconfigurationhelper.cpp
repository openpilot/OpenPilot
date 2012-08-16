/**
 ******************************************************************************
 *
 * @file       vehicleconfigurationhelper.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup VehicleConfigurationHelper
 * @{
 * @brief
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

#include "vehicleconfigurationhelper.h"
#include "extensionsystem/pluginmanager.h"
#include "hwsettings.h"
#include "actuatorsettings.h"
#include "attitudesettings.h"
#include "mixersettings.h"
#include "systemsettings.h"

VehicleConfigurationHelper::VehicleConfigurationHelper(VehicleConfigurationSource *configSource)
    : m_configSource(configSource), m_uavoManager(0)
{
    Q_ASSERT(m_configSource);
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_uavoManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(m_uavoManager);
}

void VehicleConfigurationHelper::setupVehicle()
{
    applyHardwareConfiguration();
    applyVehicleConfiguration();
    applyOutputConfiguration();
    applyFlighModeConfiguration();
    applyLevellingConfiguration();
}

void VehicleConfigurationHelper::applyHardwareConfiguration()
{
    HwSettings* hwSettings = HwSettings::GetInstance(m_uavoManager);
    HwSettings::DataFields data = hwSettings->getData();
    switch(m_configSource->getControllerType())
    {
        case VehicleConfigurationSource::CONTROLLER_CC:
        case VehicleConfigurationSource::CONTROLLER_CC3D:
            // Reset all ports
            data.CC_RcvrPort = HwSettings::CC_RCVRPORT_DISABLED;
            data.CC_FlexiPort = HwSettings::CC_FLEXIPORT_DISABLED;
            data.CC_MainPort = HwSettings::CC_MAINPORT_DISABLED;
            switch(m_configSource->getInputType())
            {
                case VehicleConfigurationSource::INPUT_PWM:
                    data.CC_RcvrPort = HwSettings::CC_RCVRPORT_PWM;
                    break;
                case VehicleConfigurationSource::INPUT_PPM:
                    data.CC_RcvrPort = HwSettings::CC_RCVRPORT_PPM;
                    break;
                case VehicleConfigurationSource::INPUT_SBUS:
                    data.CC_MainPort = HwSettings::CC_MAINPORT_SBUS;
                    break;
                case VehicleConfigurationSource::INPUT_DSM:
                    // TODO: Handle all of the DSM types ?? Which is most common?
                    data.CC_MainPort = HwSettings::CC_MAINPORT_DSM2;
                    break;
                default:
                    break;
            }
            break;
        case VehicleConfigurationSource::CONTROLLER_REVO:
            // TODO: Implement Revo settings
            break;
        default:
            break;
    }
    hwSettings->setData(data);
}

void VehicleConfigurationHelper::applyVehicleConfiguration()
{

    switch(m_configSource->getVehicleType())
    {
        case VehicleConfigurationSource::VEHICLE_MULTI:
        {
            resetGUIData();
            resetVehicleConfig();

            switch(m_configSource->getVehicleSubType())
            {
                case VehicleConfigurationSource::MULTI_ROTOR_TRI_Y:
                    setupTriCopter();
                    break;
                case VehicleConfigurationSource::MULTI_ROTOR_QUAD_X:
                case VehicleConfigurationSource::MULTI_ROTOR_QUAD_PLUS:
                    setupQuadCopter();
                    break;
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA:
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA_COAX_Y:
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA_H:
                    setupHexaCopter();
                    break;
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_X:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_PLUS:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_V:
                    setupOctoCopter();
                    break;
                default:
                    break;
            }
            break;
        }
        case VehicleConfigurationSource::VEHICLE_FIXEDWING:
        case VehicleConfigurationSource::VEHICLE_HELI:
        case VehicleConfigurationSource::VEHICLE_SURFACE:
            // TODO: Implement settings for other vehicle types?
            break;
        default:
            break;
    }
}

void VehicleConfigurationHelper::applyOutputConfiguration()
{
    ActuatorSettings* actSettings = ActuatorSettings::GetInstance(m_uavoManager);
    switch(m_configSource->getVehicleType())
    {
        case VehicleConfigurationSource::VEHICLE_MULTI:
        {
            ActuatorSettings::DataFields data = actSettings->getData();

            data.ChannelUpdateFreq[0] = LEGACY_ESC_FREQUENCE;
            data.ChannelUpdateFreq[1] = LEGACY_ESC_FREQUENCE;
            data.ChannelUpdateFreq[3] = LEGACY_ESC_FREQUENCE;
            data.ChannelUpdateFreq[4] = LEGACY_ESC_FREQUENCE;

            qint16 updateFrequence = LEGACY_ESC_FREQUENCE;
            switch(m_configSource->getESCType())
            {
                case VehicleConfigurationSource::ESC_LEGACY:
                    updateFrequence = LEGACY_ESC_FREQUENCE;
                    break;
                case VehicleConfigurationSource::ESC_RAPID:
                    updateFrequence = RAPID_ESC_FREQUENCE;
                    break;
                default:
                    break;
            }

            switch(m_configSource->getVehicleSubType())
            {
                case VehicleConfigurationSource::MULTI_ROTOR_TRI_Y:
                    data.ChannelUpdateFreq[0] = updateFrequence;
                    break;
                case VehicleConfigurationSource::MULTI_ROTOR_QUAD_X:
                case VehicleConfigurationSource::MULTI_ROTOR_QUAD_PLUS:
                    data.ChannelUpdateFreq[0] = updateFrequence;
                    data.ChannelUpdateFreq[1] = updateFrequence;
                    break;
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA:
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA_COAX_Y:
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA_H:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_X:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_PLUS:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_V:
                    data.ChannelUpdateFreq[0] = updateFrequence;
                    data.ChannelUpdateFreq[1] = updateFrequence;
                    data.ChannelUpdateFreq[3] = updateFrequence;
                    data.ChannelUpdateFreq[4] = updateFrequence;
                    break;
                default:
                    break;
            }
            actSettings->setData(data);
            break;
        }
        case VehicleConfigurationSource::VEHICLE_FIXEDWING:
        case VehicleConfigurationSource::VEHICLE_HELI:
        case VehicleConfigurationSource::VEHICLE_SURFACE:
            // TODO: Implement settings for other vehicle types?
            break;
        default:
            break;
    }
}

void VehicleConfigurationHelper::applyFlighModeConfiguration()
{
}

void VehicleConfigurationHelper::applyLevellingConfiguration()
{
    if(m_configSource->isLevellingPerformed())
    {
        accelGyroBias bias = m_configSource->getLevellingBias();
        AttitudeSettings::DataFields attitudeSettingsData = AttitudeSettings::GetInstance(m_uavoManager)->getData();
        attitudeSettingsData.AccelBias[0] += bias.m_accelerometerXBias;
        attitudeSettingsData.AccelBias[1] += bias.m_accelerometerYBias;
        attitudeSettingsData.AccelBias[2] += bias.m_accelerometerZBias;
        attitudeSettingsData.GyroBias[0] = -bias.m_gyroXBias;
        attitudeSettingsData.GyroBias[1] = -bias.m_gyroYBias;
        attitudeSettingsData.GyroBias[2] = -bias.m_gyroZBias;
        AttitudeSettings::GetInstance(m_uavoManager)->setData(attitudeSettingsData);
    }
}


void VehicleConfigurationHelper::resetVehicleConfig()
{
    // Reset all vehicle data
    MixerSettings* mSettings = MixerSettings::GetInstance(m_uavoManager);

    // Reset feed forward, accel times etc
    mSettings->setFeedForward(0.0f);
    mSettings->setMaxAccel(1000.0f);
    mSettings->setAccelTime(0.0f);
    mSettings->setDecelTime(0.0f);

    // Reset throttle curves
    QString throttlePattern = "ThrottleCurve%1";
    for(int i = 1; i <= 2; i++) {
        UAVObjectField *field = mSettings->getField(throttlePattern.arg(i));
        Q_ASSERT(field);
        for(int i = 0; i < field->getNumElements(); i++){
            field->setValue(i * ( 1.0f / field->getNumElements()), i);
        }
    }

    // Reset Mixer types and values
    QString mixerTypePattern = "Mixer%1Type";
    QString mixerVectorPattern = "Mixer%1Vector";
    for(int i = 1; i <= 10; i++) {
        UAVObjectField *field = mSettings->getField(mixerTypePattern.arg(i));
        Q_ASSERT(field);
        field->setValue(field->getOptions().at(0));

        field = mSettings->getField(mixerVectorPattern.arg(i));
        Q_ASSERT(field);
        for(int i = 0; i < field->getNumElements(); i++){
            field->setValue(0, i);
        }
    }

    // Apply updates
    mSettings->setData(mSettings->getData());
}

void VehicleConfigurationHelper::resetGUIData()
{
    SystemSettings * sSettings = SystemSettings::GetInstance(m_uavoManager);
    Q_ASSERT(sSettings);
    SystemSettings::DataFields data = sSettings->getData();
    data.AirframeType = SystemSettings::AIRFRAMETYPE_CUSTOM;
    for(int i = 0; i < SystemSettings::GUICONFIGDATA_NUMELEM; i++)
    {
        data.GUIConfigData[i] = 0;
    }
    sSettings->setData(data);
}


void VehicleConfigurationHelper::setupTriCopter()
{
    // Typical vehicle setup
    // 1. Setup and apply mixer
    // 2. Setup GUI data
    mixerSettings mixer;
    mixer.channels[0].type = 0;
}

void VehicleConfigurationHelper::setupQuadCopter()
{
}

void VehicleConfigurationHelper::setupHexaCopter()
{
}

void VehicleConfigurationHelper::setupOctoCopter()
{
}
