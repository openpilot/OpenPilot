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
#include "manualcontrolsettings.h"
#include "flightmodesettings.h"
#include "stabilizationsettings.h"
#include "stabilizationbank.h"
#include "stabilizationsettingsbank1.h"
#include "revocalibration.h"
#include "accelgyrosettings.h"
#include "gpssettings.h"
#include "airspeedsettings.h"
#include <QtCore/qmath.h>
#include <QJsonObject>
#include "auxmagsettings.h"

VehicleConfigurationHelper::VehicleConfigurationHelper(VehicleConfigurationSource *configSource)
    : m_configSource(configSource), m_uavoManager(0),
    m_transactionOK(false), m_transactionTimeout(false), m_currentTransactionObjectID(-1),
    m_progress(0)
{
    Q_ASSERT(m_configSource);
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_uavoManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(m_uavoManager);
}

bool VehicleConfigurationHelper::setupVehicle(bool save)
{
    m_progress = 0;
    clearModifiedObjects();
    resetVehicleConfig();
    resetGUIData();
    if (!saveChangesToController(save)) {
        return false;
    }

    m_progress = 0;
    applyHardwareConfiguration();
    applyVehicleConfiguration();
    applyActuatorConfiguration();
    applyFlightModeConfiguration();

    if (save) {
        applySensorBiasConfiguration();
    }

    applyStabilizationConfiguration();
    applyManualControlDefaults();

    applyTemplateSettings();

    bool result = saveChangesToController(save);
    emit saveProgress(m_modifiedObjects.count() + 1, ++m_progress, result ? tr("Done!") : tr("Failed!"));
    return result;
}

bool VehicleConfigurationHelper::setupHardwareSettings(bool save)
{
    m_progress = 0;
    clearModifiedObjects();
    applyHardwareConfiguration();
    applyManualControlDefaults();

    bool result = saveChangesToController(save);
    emit saveProgress(m_modifiedObjects.count() + 1, ++m_progress, result ? tr("Done!") : tr("Failed!"));
    return result;
}

bool VehicleConfigurationHelper::isApplicable(UAVObject *dataObj)
{
    return true;
}

void VehicleConfigurationHelper::addModifiedObject(UAVDataObject *object, QString description)
{
    m_modifiedObjects << new QPair<UAVDataObject *, QString>(object, description);
}

void VehicleConfigurationHelper::clearModifiedObjects()
{
    for (int i = 0; i < m_modifiedObjects.count(); i++) {
        QPair<UAVDataObject *, QString> *pair = m_modifiedObjects.at(i);
        delete pair;
    }
    m_modifiedObjects.clear();
}

void VehicleConfigurationHelper::applyHardwareConfiguration()
{
    HwSettings *hwSettings = HwSettings::GetInstance(m_uavoManager);

    Q_ASSERT(hwSettings);
    HwSettings::DataFields data = hwSettings->getData();

    data.OptionalModules[HwSettings::OPTIONALMODULES_GPS] = 0;
    data.OptionalModules[HwSettings::OPTIONALMODULES_AIRSPEED] = 0;

    switch (m_configSource->getControllerType()) {
    case VehicleConfigurationSource::CONTROLLER_REVO:
    case VehicleConfigurationSource::CONTROLLER_DISCOVERYF4:
        // Reset all ports to their defaults
        data.RM_RcvrPort  = HwSettings::RM_RCVRPORT_DISABLED;
        data.RM_FlexiPort = HwSettings::RM_FLEXIPORT_DISABLED;

        // Revo uses inbuilt Modem do not set mainport to be active telemetry link for the Revo
        if (m_configSource->getControllerType() == VehicleConfigurationSource::CONTROLLER_REVO) {
            data.RM_MainPort = HwSettings::RM_MAINPORT_DISABLED;
        } else {
            data.RM_MainPort = HwSettings::RM_MAINPORT_TELEMETRY;
        }

        switch (m_configSource->getInputType()) {
        case VehicleConfigurationSource::INPUT_PWM:
            data.RM_RcvrPort = HwSettings::RM_RCVRPORT_PWM;
            break;
        case VehicleConfigurationSource::INPUT_PPM:
            data.RM_RcvrPort = HwSettings::RM_RCVRPORT_PPM;
            break;
        case VehicleConfigurationSource::INPUT_SBUS:
            data.RM_MainPort = HwSettings::RM_MAINPORT_SBUS;
            // We have to set telemetry on flexport since s.bus needs the mainport on all but Revo.
            if (m_configSource->getControllerType() != VehicleConfigurationSource::CONTROLLER_REVO) {
                data.RM_FlexiPort = HwSettings::RM_FLEXIPORT_TELEMETRY;
            }
            break;
        case VehicleConfigurationSource::INPUT_DSM:
            data.RM_FlexiPort = HwSettings::RM_FLEXIPORT_DSM;
            break;
        default:
            break;
        }

        if (m_configSource->getGpsType() != VehicleConfigurationSource::GPS_DISABLED) {
            data.OptionalModules[HwSettings::OPTIONALMODULES_GPS] = 1;
            data.GPSSpeed = HwSettings::GPSSPEED_57600;

            if (m_configSource->getInputType() == VehicleConfigurationSource::INPUT_SBUS) {
                data.RM_FlexiPort = HwSettings::RM_FLEXIPORT_GPS;
            } else {
                data.RM_MainPort = HwSettings::RM_MAINPORT_GPS;
            }

            GPSSettings *gpsSettings = GPSSettings::GetInstance(m_uavoManager);
            Q_ASSERT(gpsSettings);
            GPSSettings::DataFields gpsData = gpsSettings->getData();
            gpsData.UbxAutoConfig = GPSSettings::UBXAUTOCONFIG_DISABLED;

            switch (m_configSource->getGpsType()) {
            case VehicleConfigurationSource::GPS_NMEA:
                gpsData.DataProtocol = GPSSettings::DATAPROTOCOL_NMEA;
                break;
            case VehicleConfigurationSource::GPS_UBX:
                gpsData.DataProtocol = GPSSettings::DATAPROTOCOL_UBX;
                break;
            case VehicleConfigurationSource::GPS_PLATINUM:
            {
                gpsData.DataProtocol  = GPSSettings::DATAPROTOCOL_UBX;
                gpsData.UbxAutoConfig = GPSSettings::UBXAUTOCONFIG_CONFIGURE;
                AuxMagSettings *magSettings = AuxMagSettings::GetInstance(m_uavoManager);
                Q_ASSERT(magSettings);
                AuxMagSettings::DataFields magsData = magSettings->getData();
                magsData.Usage = AuxMagSettings::USAGE_AUXONLY;
                magSettings->setData(magsData);
                addModifiedObject(magSettings, tr("Writing External Mag sensor settings"));
                break;
            }
            case VehicleConfigurationSource::GPS_DISABLED:
                // Should not be able to reach here
                break;
            }

            gpsSettings->setData(gpsData);
            addModifiedObject(gpsSettings, tr("Writing GPS sensor settings"));
        } else {
            data.OptionalModules[HwSettings::OPTIONALMODULES_GPS] = 0;
        }

        if (m_configSource->getVehicleType() == VehicleConfigurationSource::VEHICLE_FIXEDWING &&
            m_configSource->getAirspeedType() != VehicleConfigurationSource::AIRSPEED_DISABLED) {
            AirspeedSettings *airspeedSettings = AirspeedSettings::GetInstance(m_uavoManager);
            Q_ASSERT(airspeedSettings);
            AirspeedSettings::DataFields airspeedData = airspeedSettings->getData();

            switch (m_configSource->getAirspeedType()) {
            case VehicleConfigurationSource::AIRSPEED_ESTIMATE:
                data.OptionalModules[HwSettings::OPTIONALMODULES_AIRSPEED] = 1;
                airspeedData.AirspeedSensorType = AirspeedSettings::AIRSPEEDSENSORTYPE_GROUNDSPEEDBASEDWINDESTIMATION;
                break;
            case VehicleConfigurationSource::AIRSPEED_EAGLETREE:
                data.OptionalModules[HwSettings::OPTIONALMODULES_AIRSPEED] = 1;
                data.RM_FlexiPort = HwSettings::RM_FLEXIPORT_I2C;
                airspeedData.AirspeedSensorType = AirspeedSettings::AIRSPEEDSENSORTYPE_EAGLETREEAIRSPEEDV3;
                break;
            case VehicleConfigurationSource::AIRSPEED_MS4525:
                data.OptionalModules[HwSettings::OPTIONALMODULES_AIRSPEED] = 1;
                data.RM_FlexiPort = HwSettings::RM_FLEXIPORT_I2C;
                airspeedData.AirspeedSensorType = AirspeedSettings::AIRSPEEDSENSORTYPE_PIXHAWKAIRSPEEDMS4525DO;
                break;
            default:
                data.OptionalModules[HwSettings::OPTIONALMODULES_AIRSPEED] = 0;
                break;
            }

            airspeedSettings->setData(airspeedData);
            addModifiedObject(airspeedSettings, tr("Writing Airspeed sensor settings"));
        }
        break;
    default:
        break;
    }
    hwSettings->setData(data);
    addModifiedObject(hwSettings, tr("Writing hardware settings"));
}

void VehicleConfigurationHelper::applyVehicleConfiguration()
{
    switch (m_configSource->getVehicleType()) {
    case VehicleConfigurationSource::VEHICLE_MULTI:
    {
        switch (m_configSource->getVehicleSubType()) {
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
        case VehicleConfigurationSource::MULTI_ROTOR_HEXA_X:
            setupHexaCopter();
            break;
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO:
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO_X:
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
    {
        switch (m_configSource->getVehicleSubType()) {
        case VehicleConfigurationSource::FIXED_WING_DUAL_AILERON:
            setupDualAileron();
            break;
        case VehicleConfigurationSource::FIXED_WING_AILERON:
            setupAileron();
            break;
        case VehicleConfigurationSource::FIXED_WING_ELEVON:
            setupElevon();
            break;
        case VehicleConfigurationSource::FIXED_WING_VTAIL:
            setupVtail();
            break;
        default:
            break;
        }
        break;
    }
    case VehicleConfigurationSource::VEHICLE_HELI:
        // TODO: Implement settings for Helis
        break;

    case VehicleConfigurationSource::VEHICLE_SURFACE:
    {
        switch (m_configSource->getVehicleSubType()) {
        case VehicleConfigurationSource::GROUNDVEHICLE_CAR:
            setupCar();
            break;
        case VehicleConfigurationSource::GROUNDVEHICLE_DIFFERENTIAL:
            setupTank();
            break;
        case VehicleConfigurationSource::GROUNDVEHICLE_MOTORCYCLE:
            setupMotorcycle();
            break;
        default:
            break;
        }
        break;
    }

    default:
        break;
    }
}

void VehicleConfigurationHelper::applyActuatorConfiguration()
{
    ActuatorSettings *actSettings = ActuatorSettings::GetInstance(m_uavoManager);

    qint16 escFrequence = LEGACY_ESC_FREQUENCY;
    ActuatorSettings::BankModeOptions bankMode = ActuatorSettings::BANKMODE_PWM;

    switch (m_configSource->getEscType()) {
    case VehicleConfigurationSource::ESC_STANDARD:
        escFrequence = LEGACY_ESC_FREQUENCY;
        bankMode     = ActuatorSettings::BANKMODE_PWM;
        break;
    case VehicleConfigurationSource::ESC_RAPID:
        bankMode     = ActuatorSettings::BANKMODE_PWM;
        escFrequence = RAPID_ESC_FREQUENCY;
        break;
    case VehicleConfigurationSource::ESC_SYNCHED:
        bankMode     = ActuatorSettings::BANKMODE_PWMSYNC;
        escFrequence = PWMSYNC_ESC_FREQUENCY;
        break;
    case VehicleConfigurationSource::ESC_ONESHOT:
        bankMode     = ActuatorSettings::BANKMODE_ONESHOT125;
        escFrequence = ONESHOT_ESC_FREQUENCY;
        break;
    default:
        break;
    }

    qint16 servoFrequence = ANALOG_SERVO_FREQUENCY;
    switch (m_configSource->getServoType()) {
    case VehicleConfigurationSource::SERVO_ANALOG:
        servoFrequence = ANALOG_SERVO_FREQUENCY;
        break;
    case VehicleConfigurationSource::SERVO_DIGITAL:
        servoFrequence = DIGITAL_SERVO_FREQUENCY;
        break;
    default:
        break;
    }

    switch (m_configSource->getVehicleType()) {
    case VehicleConfigurationSource::VEHICLE_MULTI:
    {
        ActuatorSettings::DataFields data = actSettings->getData();

        QList<actuatorChannelSettings> actuatorSettings = m_configSource->getActuatorSettings();
        for (quint16 i = 0; i < ActuatorSettings::CHANNELMAX_NUMELEM; i++) {
            data.ChannelType[i]    = ActuatorSettings::CHANNELTYPE_PWM;
            data.ChannelAddr[i]    = i;
            data.ChannelMin[i]     = actuatorSettings[i].channelMin;
            data.ChannelNeutral[i] = actuatorSettings[i].channelNeutral;
            data.ChannelMax[i]     = actuatorSettings[i].channelMax;
        }

        data.MotorsSpinWhileArmed = ActuatorSettings::MOTORSSPINWHILEARMED_FALSE;

        for (quint16 i = 0; i < ActuatorSettings::BANKUPDATEFREQ_NUMELEM; i++) {
            data.BankUpdateFreq[i] = LEGACY_ESC_FREQUENCY;
            data.BankMode[i] = ActuatorSettings::BANKMODE_PWM;
        }

        switch (m_configSource->getVehicleSubType()) {
        case VehicleConfigurationSource::MULTI_ROTOR_TRI_Y:
            // Servo always on channel 4
            data.BankUpdateFreq[0] = escFrequence;
            data.BankMode[0] = bankMode;
            if (m_configSource->getControllerType() == VehicleConfigurationSource::CONTROLLER_REVO) {
                data.BankUpdateFreq[1] = escFrequence;
                data.BankMode[1] = bankMode;
                data.BankUpdateFreq[2] = servoFrequence;
            } 
            break;
        case VehicleConfigurationSource::MULTI_ROTOR_QUAD_X:
        case VehicleConfigurationSource::MULTI_ROTOR_QUAD_PLUS:
            data.BankUpdateFreq[0] = escFrequence;
            data.BankMode[0] = bankMode;
            data.BankUpdateFreq[1] = escFrequence;
            data.BankMode[1] = bankMode;
            if (m_configSource->getControllerType() == VehicleConfigurationSource::CONTROLLER_REVO) {
                data.BankUpdateFreq[2] = escFrequence;
                data.BankMode[2] = bankMode;
            }
            break;
        case VehicleConfigurationSource::MULTI_ROTOR_HEXA:
        case VehicleConfigurationSource::MULTI_ROTOR_HEXA_COAX_Y:
        case VehicleConfigurationSource::MULTI_ROTOR_HEXA_H:
        case VehicleConfigurationSource::MULTI_ROTOR_HEXA_X:
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO:
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO_X:
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_X:
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_PLUS:
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO_V:
            data.BankUpdateFreq[0] = escFrequence;
            data.BankMode[0] = bankMode;
            data.BankUpdateFreq[1] = escFrequence;
            data.BankMode[1] = bankMode;
            data.BankUpdateFreq[2] = escFrequence;
            data.BankMode[2] = bankMode;
            data.BankUpdateFreq[3] = escFrequence;
            data.BankMode[3] = bankMode;
            break;
        default:
            break;
        }
        actSettings->setData(data);
        addModifiedObject(actSettings, tr("Writing actuator settings"));
        break;
    }

    case VehicleConfigurationSource::VEHICLE_FIXEDWING:
    {
        ActuatorSettings::DataFields data = actSettings->getData();

        QList<actuatorChannelSettings> actuatorSettings = m_configSource->getActuatorSettings();
        for (quint16 i = 0; i < ActuatorSettings::CHANNELMAX_NUMELEM; i++) {
            data.ChannelType[i]    = ActuatorSettings::CHANNELTYPE_PWM;
            data.ChannelAddr[i]    = i;
            data.ChannelMin[i]     = actuatorSettings[i].channelMin;
            data.ChannelNeutral[i] = actuatorSettings[i].channelNeutral;
            data.ChannelMax[i]     = actuatorSettings[i].channelMax;
        }

        for (quint16 i = 0; i < ActuatorSettings::BANKUPDATEFREQ_NUMELEM; i++) {
            data.BankUpdateFreq[i] = servoFrequence;
            data.BankMode[i] = ActuatorSettings::BANKMODE_PWM;
            if (m_configSource->getControllerType() == VehicleConfigurationSource::CONTROLLER_REVO) {
                if (i == 1) {
                    data.BankUpdateFreq[i] = escFrequence;
                }
            } 
        }

        actSettings->setData(data);
        addModifiedObject(actSettings, tr("Writing actuator settings"));

        break;
    }

    case VehicleConfigurationSource::VEHICLE_HELI:
        // TODO: Implement settings for Heli vehicle types
        break;

    case VehicleConfigurationSource::VEHICLE_SURFACE:
    {
        ActuatorSettings::DataFields data = actSettings->getData();

        QList<actuatorChannelSettings> actuatorSettings = m_configSource->getActuatorSettings();
        for (quint16 i = 0; i < ActuatorSettings::CHANNELMAX_NUMELEM; i++) {
            data.ChannelType[i]    = ActuatorSettings::CHANNELTYPE_PWM;
            data.ChannelAddr[i]    = i;
            data.ChannelMin[i]     = actuatorSettings[i].channelMin;
            data.ChannelNeutral[i] = actuatorSettings[i].channelNeutral;
            data.ChannelMax[i]     = actuatorSettings[i].channelMax;
        }

        for (quint16 i = 0; i < ActuatorSettings::BANKUPDATEFREQ_NUMELEM; i++) {
            data.BankUpdateFreq[i] = servoFrequence;
            data.BankMode[i] = ActuatorSettings::BANKMODE_PWM;
            if (m_configSource->getControllerType() == VehicleConfigurationSource::CONTROLLER_REVO) {
                if (i == 1) {
                    data.BankUpdateFreq[i] = escFrequence;
                }
            } 
        }

        actSettings->setData(data);
        addModifiedObject(actSettings, tr("Writing actuator settings"));

        break;
    }

    default:
        break;
    }
}

void VehicleConfigurationHelper::applyFlightModeConfiguration()
{
    FlightModeSettings *modeSettings = FlightModeSettings::GetInstance(m_uavoManager);
    ManualControlSettings *controlSettings = ManualControlSettings::GetInstance(m_uavoManager);

    Q_ASSERT(modeSettings);
    Q_ASSERT(controlSettings);

    FlightModeSettings::DataFields data     = modeSettings->getData();
    ManualControlSettings::DataFields data2 = controlSettings->getData();
    data.Stabilization1Settings[0] = FlightModeSettings::STABILIZATION1SETTINGS_ATTITUDE;
    data.Stabilization1Settings[1] = FlightModeSettings::STABILIZATION1SETTINGS_ATTITUDE;
    data.Stabilization1Settings[2] = FlightModeSettings::STABILIZATION1SETTINGS_AXISLOCK;
    data.Stabilization1Settings[3] = FlightModeSettings::STABILIZATION1SETTINGS_MANUAL;
    data.Stabilization2Settings[0] = FlightModeSettings::STABILIZATION2SETTINGS_ATTITUDE;
    data.Stabilization2Settings[1] = FlightModeSettings::STABILIZATION2SETTINGS_ATTITUDE;
    data.Stabilization2Settings[2] = FlightModeSettings::STABILIZATION2SETTINGS_RATE;
    data.Stabilization2Settings[3] = FlightModeSettings::STABILIZATION2SETTINGS_MANUAL;
    data.Stabilization3Settings[0] = FlightModeSettings::STABILIZATION3SETTINGS_RATE;
    data.Stabilization3Settings[1] = FlightModeSettings::STABILIZATION3SETTINGS_RATE;
    data.Stabilization3Settings[2] = FlightModeSettings::STABILIZATION3SETTINGS_RATE;
    data.Stabilization3Settings[3] = FlightModeSettings::STABILIZATION3SETTINGS_MANUAL;
    data.Stabilization4Settings[0] = FlightModeSettings::STABILIZATION4SETTINGS_ATTITUDE;
    data.Stabilization4Settings[1] = FlightModeSettings::STABILIZATION4SETTINGS_ATTITUDE;
    data.Stabilization4Settings[2] = FlightModeSettings::STABILIZATION4SETTINGS_AXISLOCK;
    data.Stabilization4Settings[3] = FlightModeSettings::STABILIZATION4SETTINGS_CRUISECONTROL;
    data.Stabilization5Settings[0] = FlightModeSettings::STABILIZATION5SETTINGS_ATTITUDE;
    data.Stabilization5Settings[1] = FlightModeSettings::STABILIZATION5SETTINGS_ATTITUDE;
    data.Stabilization5Settings[2] = FlightModeSettings::STABILIZATION5SETTINGS_RATE;
    data.Stabilization5Settings[3] = FlightModeSettings::STABILIZATION5SETTINGS_CRUISECONTROL;
    data.Stabilization6Settings[0] = FlightModeSettings::STABILIZATION6SETTINGS_RATE;
    data.Stabilization6Settings[1] = FlightModeSettings::STABILIZATION6SETTINGS_RATE;
    data.Stabilization6Settings[2] = FlightModeSettings::STABILIZATION6SETTINGS_RATE;
    data.Stabilization6Settings[3] = FlightModeSettings::STABILIZATION6SETTINGS_MANUAL;
    data2.FlightModeNumber = 3;
    data.FlightModePosition[0]     = FlightModeSettings::FLIGHTMODEPOSITION_STABILIZED1;
    data.FlightModePosition[1]     = FlightModeSettings::FLIGHTMODEPOSITION_STABILIZED2;
    data.FlightModePosition[2]     = FlightModeSettings::FLIGHTMODEPOSITION_STABILIZED3;
    data.FlightModePosition[3]     = FlightModeSettings::FLIGHTMODEPOSITION_STABILIZED4;
    data.FlightModePosition[4]     = FlightModeSettings::FLIGHTMODEPOSITION_STABILIZED5;
    data.FlightModePosition[5]     = FlightModeSettings::FLIGHTMODEPOSITION_STABILIZED6;
    modeSettings->setData(data);
    addModifiedObject(modeSettings, tr("Writing flight mode settings 1/2"));
    controlSettings->setData(data2);
    addModifiedObject(controlSettings, tr("Writing flight mode settings 2/2"));
}

void VehicleConfigurationHelper::applySensorBiasConfiguration()
{
    if (m_configSource->isCalibrationPerformed()) {
        accelGyroBias bias = m_configSource->getCalibrationBias();
        float G = 9.81f;

        AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(m_uavoManager);
        Q_ASSERT(accelGyroSettings);
        AccelGyroSettings::DataFields accelGyroSettingsData = accelGyroSettings->getData();

        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X] += bias.m_accelerometerXBias;
        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y] += bias.m_accelerometerYBias;
        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z] += bias.m_accelerometerZBias + G;
        accelGyroSettingsData.gyro_bias[AccelGyroSettings::GYRO_BIAS_X]    = bias.m_gyroXBias;
        accelGyroSettingsData.gyro_bias[AccelGyroSettings::GYRO_BIAS_Y]    = bias.m_gyroYBias;
        accelGyroSettingsData.gyro_bias[AccelGyroSettings::GYRO_BIAS_Z]    = bias.m_gyroZBias;

        accelGyroSettings->setData(accelGyroSettingsData);
        addModifiedObject(accelGyroSettings, tr("Writing gyro and accelerometer bias settings"));

        switch (m_configSource->getControllerType()) {        
        case VehicleConfigurationSource::CONTROLLER_REVO:
        {
            RevoCalibration *revolutionCalibration = RevoCalibration::GetInstance(m_uavoManager);
            Q_ASSERT(revolutionCalibration);
            RevoCalibration::DataFields data = revolutionCalibration->getData();

            data.BiasCorrectedRaw = RevoCalibration::BIASCORRECTEDRAW_TRUE;

            revolutionCalibration->setData(data);
            addModifiedObject(revolutionCalibration, tr("Writing board settings"));
            break;
        }
        default:
            // Something went terribly wrong.
            break;
        }
    }
}

void VehicleConfigurationHelper::applyStabilizationConfiguration()
{
    StabilizationSettings *stabSettings = StabilizationSettings::GetInstance(m_uavoManager);

    Q_ASSERT(stabSettings);

    StabilizationSettings defaultSettings;
    stabSettings->setData(defaultSettings.getData());
    addModifiedObject(stabSettings, tr("Writing stabilization settings"));
}

void VehicleConfigurationHelper::applyMixerConfiguration(mixerChannelSettings channels[])
{
    // Set all mixer data
    MixerSettings *mSettings = MixerSettings::GetInstance(m_uavoManager);

    Q_ASSERT(mSettings);

    // Set Mixer types and values
    QString mixerTypePattern   = "Mixer%1Type";
    QString mixerVectorPattern = "Mixer%1Vector";
    for (quint32 i = 0; i < ActuatorSettings::CHANNELADDR_NUMELEM; i++) {
        UAVObjectField *field = mSettings->getField(mixerTypePattern.arg(i + 1));
        Q_ASSERT(field);
        field->setValue(field->getOptions().at(channels[i].type));

        field = mSettings->getField(mixerVectorPattern.arg(i + 1));
        Q_ASSERT(field);
        field->setValue((channels[i].throttle1 * 127) / 100, 0);
        field->setValue((channels[i].throttle2 * 127) / 100, 1);

        // Normalize mixer values, allow a well balanced mixer saved
        if (channels[i].roll < 0) {
            field->setValue(qFloor((double)(channels[i].roll * 127) / 100), 2);
        } else {
            field->setValue(qCeil((double)(channels[i].roll * 127) / 100), 2);
        }

        if (channels[i].pitch < 0) {
            field->setValue(qFloor((double)(channels[i].pitch * 127) / 100), 3);
        } else {
            field->setValue(qCeil((double)(channels[i].pitch * 127) / 100), 3);
        }

        if (channels[i].yaw < 0) {
            field->setValue(qFloor((double)(channels[i].yaw * 127) / 100), 4);
        } else {
            field->setValue(qCeil((double)(channels[i].yaw * 127) / 100), 4);
        }
    }

    // Default maxThrottle and minThrottle
    float maxThrottle = 1;
    float minThrottle = 0;


    // Save mixer values for sliders
    switch (m_configSource->getVehicleType()) {
    case VehicleConfigurationSource::VEHICLE_MULTI:
    {
        switch (m_configSource->getVehicleSubType()) {
        case VehicleConfigurationSource::MULTI_ROTOR_TRI_Y:
        case VehicleConfigurationSource::MULTI_ROTOR_HEXA:
        case VehicleConfigurationSource::MULTI_ROTOR_HEXA_H:
        case VehicleConfigurationSource::MULTI_ROTOR_HEXA_X:
            mSettings->setMixerValueRoll(100);
            mSettings->setMixerValuePitch(100);
            mSettings->setMixerValueYaw(100);
            break;
        case VehicleConfigurationSource::MULTI_ROTOR_QUAD_X:
            mSettings->setMixerValueRoll(50);
            mSettings->setMixerValuePitch(50);
            mSettings->setMixerValueYaw(50);
            break;
        case VehicleConfigurationSource::MULTI_ROTOR_QUAD_PLUS:
            mSettings->setMixerValueRoll(100);
            mSettings->setMixerValuePitch(100);
            mSettings->setMixerValueYaw(50);
            break;
        case VehicleConfigurationSource::MULTI_ROTOR_HEXA_COAX_Y:
            mSettings->setMixerValueRoll(100);
            mSettings->setMixerValuePitch(50);
            mSettings->setMixerValueYaw(66);
            break;
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO:
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO_X:
            mSettings->setMixerValueRoll(100);
            mSettings->setMixerValuePitch(100);
            mSettings->setMixerValueYaw(100);
            break;
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_X:
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_PLUS:
        case VehicleConfigurationSource::MULTI_ROTOR_OCTO_V:
            break;
        default:
            break;
        }
        break;
    }
    case VehicleConfigurationSource::VEHICLE_FIXEDWING:
        mSettings->setMixerValueRoll(100);
        mSettings->setMixerValuePitch(100);
        mSettings->setMixerValueYaw(100);
        maxThrottle = 1;
        break;
    case VehicleConfigurationSource::VEHICLE_HELI:
        break;
    case VehicleConfigurationSource::VEHICLE_SURFACE:
    {
        switch (m_configSource->getVehicleSubType()) {
        case VehicleConfigurationSource::GROUNDVEHICLE_MOTORCYCLE:
            mSettings->setMixerValueRoll(100);
            mSettings->setMixerValuePitch(100);
            mSettings->setMixerValueYaw(100);
            maxThrottle = 1;
            break;
        case VehicleConfigurationSource::GROUNDVEHICLE_CAR:
            mSettings->setMixerValueRoll(100);
            mSettings->setMixerValuePitch(100);
            mSettings->setMixerValueYaw(100);
            maxThrottle = 1;
            minThrottle = 0;
            break;
        case VehicleConfigurationSource::GROUNDVEHICLE_DIFFERENTIAL:
            mSettings->setMixerValueRoll(100);
            mSettings->setMixerValuePitch(100);
            mSettings->setMixerValueYaw(100);
            maxThrottle = 0.8;
            minThrottle = 0;
            break;
        default:
            break;
        }
    }

    break;
    default:
        break;
    }

    // Apply Throttle curve max 90% for Multis, 100% for FixedWing/car/Motorbike, 80% for Tank
    QString throttlePattern = "ThrottleCurve%1";
    for (int i = 1; i <= 2; i++) {
        UAVObjectField *field = mSettings->getField(throttlePattern.arg(i));
        Q_ASSERT(field);
        for (quint32 i = 0; i < field->getNumElements(); i++) {
            field->setValue(minThrottle + (i * ((maxThrottle - minThrottle) / (field->getNumElements() - 1))), i);
        }
    }

    // Apply updates
    mSettings->setData(mSettings->getData());
    addModifiedObject(mSettings, tr("Writing mixer settings"));
}

void VehicleConfigurationHelper::applyMultiGUISettings(SystemSettings::AirframeTypeOptions airframe, GUIConfigDataUnion guiConfig)
{
    SystemSettings *sSettings = SystemSettings::GetInstance(m_uavoManager);

    Q_ASSERT(sSettings);
    SystemSettings::DataFields data = sSettings->getData();
    data.AirframeType = airframe;

    for (int i = 0; i < (int)(SystemSettings::GUICONFIGDATA_NUMELEM); i++) {
        data.GUIConfigData[i] = guiConfig.UAVObject[i];
    }

    sSettings->setData(data);
    addModifiedObject(sSettings, tr("Writing vehicle settings"));
}

void VehicleConfigurationHelper::applyManualControlDefaults()
{
    ManualControlSettings *mcSettings = ManualControlSettings::GetInstance(m_uavoManager);

    Q_ASSERT(mcSettings);
    ManualControlSettings::DataFields cData = mcSettings->getData();

    ManualControlSettings::ChannelGroupsOptions channelType = ManualControlSettings::CHANNELGROUPS_PWM;
    switch (m_configSource->getInputType()) {
    case VehicleConfigurationSource::INPUT_PWM:
        channelType = ManualControlSettings::CHANNELGROUPS_PWM;
        break;
    case VehicleConfigurationSource::INPUT_PPM:
        channelType = ManualControlSettings::CHANNELGROUPS_PPM;
        break;
    case VehicleConfigurationSource::INPUT_SBUS:
        channelType = ManualControlSettings::CHANNELGROUPS_SBUS;
        break;
    case VehicleConfigurationSource::INPUT_DSM:
        channelType = ManualControlSettings::CHANNELGROUPS_DSMFLEXIPORT;
        break;
    default:
        break;
    }

    cData.ChannelGroups[ManualControlSettings::CHANNELGROUPS_THROTTLE]   = channelType;
    cData.ChannelGroups[ManualControlSettings::CHANNELGROUPS_ROLL]       = channelType;
    cData.ChannelGroups[ManualControlSettings::CHANNELGROUPS_YAW] = channelType;
    cData.ChannelGroups[ManualControlSettings::CHANNELGROUPS_PITCH]      = channelType;
    cData.ChannelGroups[ManualControlSettings::CHANNELGROUPS_FLIGHTMODE] = channelType;

    mcSettings->setData(cData);
    addModifiedObject(mcSettings, tr("Writing manual control defaults"));
}

void VehicleConfigurationHelper::applyTemplateSettings()
{
    if (m_configSource->getVehicleTemplate() != NULL) {
        QJsonObject *json = m_configSource->getVehicleTemplate();
        QList<UAVObject *> updatedObjects;
        m_uavoManager->fromJson(*json, &updatedObjects);
        foreach(UAVObject * object, updatedObjects) {
            UAVDataObject *dataObj = dynamic_cast<UAVDataObject *>(object);

            if (dataObj != NULL && isApplicable(dataObj)) {
                addModifiedObject(dataObj, tr("Writing template settings for %1").arg(object->getName()));
            }
        }
    }
}

bool VehicleConfigurationHelper::saveChangesToController(bool save)
{
    qDebug() << "Saving modified objects to controller. " << m_modifiedObjects.count() << " objects in found.";
    const int OUTER_TIMEOUT = 3000 * 20; // 10 seconds timeout for saving all objects
    const int INNER_TIMEOUT = 2000; // 1 second timeout on every save attempt

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    UAVObjectUtilManager *utilMngr     = pm->getObject<UAVObjectUtilManager>();
    Q_ASSERT(utilMngr);

    QTimer outerTimeoutTimer;
    outerTimeoutTimer.setSingleShot(true);

    QTimer innerTimeoutTimer;
    innerTimeoutTimer.setSingleShot(true);

    connect(utilMngr, SIGNAL(saveCompleted(int, bool)), this, SLOT(uAVOTransactionCompleted(int, bool)));
    connect(&innerTimeoutTimer, SIGNAL(timeout()), &m_eventLoop, SLOT(quit()));
    connect(&outerTimeoutTimer, SIGNAL(timeout()), this, SLOT(saveChangesTimeout()));

    outerTimeoutTimer.start(OUTER_TIMEOUT);
    for (int i = 0; i < m_modifiedObjects.count(); i++) {
        QPair<UAVDataObject *, QString> *objPair = m_modifiedObjects.at(i);
        m_transactionOK = false;
        UAVDataObject *obj     = objPair->first;
        QString objDescription = objPair->second;
        if (UAVObject::GetGcsAccess(obj->getMetadata()) != UAVObject::ACCESS_READONLY && obj->isSettingsObject()) {
            emit saveProgress(m_modifiedObjects.count() + 1, ++m_progress, objDescription);

            m_currentTransactionObjectID = obj->getObjID();

            connect(obj, SIGNAL(transactionCompleted(UAVObject *, bool)), this, SLOT(uAVOTransactionCompleted(UAVObject *, bool)));
            while (!m_transactionOK && !m_transactionTimeout) {
                // Allow the transaction to take some time
                innerTimeoutTimer.start(INNER_TIMEOUT);

                // Set object updated
                obj->updated();
                if (!m_transactionOK) {
                    m_eventLoop.exec();
                }
                innerTimeoutTimer.stop();
            }
            disconnect(obj, SIGNAL(transactionCompleted(UAVObject *, bool)), this, SLOT(uAVOTransactionCompleted(UAVObject *, bool)));
            if (m_transactionOK) {
                qDebug() << "Object " << obj->getName() << " was successfully updated.";
                if (save) {
                    m_transactionOK = false;
                    m_currentTransactionObjectID = obj->getObjID();
                    // Try to save until success or timeout
                    while (!m_transactionOK && !m_transactionTimeout) {
                        // Allow the transaction to take some time
                        innerTimeoutTimer.start(INNER_TIMEOUT);

                        // Persist object in controller
                        utilMngr->saveObjectToSD(obj);
                        if (!m_transactionOK) {
                            m_eventLoop.exec();
                        }
                        innerTimeoutTimer.stop();
                    }
                    m_currentTransactionObjectID = -1;
                }
            }

            if (!m_transactionOK) {
                qDebug() << "Transaction timed out when trying to save: " << obj->getName();
            } else {
                qDebug() << "Object " << obj->getName() << " was successfully saved.";
            }
        } else {
            qDebug() << "Trying to save a UAVDataObject that is read only or is not a settings object.";
        }
        if (m_transactionTimeout) {
            qDebug() << "Transaction timed out when trying to save " << m_modifiedObjects.count() << " objects.";
            break;
        }
    }

    outerTimeoutTimer.stop();
    disconnect(&outerTimeoutTimer, SIGNAL(timeout()), this, SLOT(saveChangesTimeout()));
    disconnect(&innerTimeoutTimer, SIGNAL(timeout()), &m_eventLoop, SLOT(quit()));
    disconnect(utilMngr, SIGNAL(saveCompleted(int, bool)), this, SLOT(uAVOTransactionCompleted(int, bool)));

    qDebug() << "Finished saving modified objects to controller. Success = " << m_transactionOK;

    return m_transactionOK;
}

void VehicleConfigurationHelper::uAVOTransactionCompleted(int oid, bool success)
{
    if (oid == m_currentTransactionObjectID) {
        m_transactionOK = success;
        m_eventLoop.quit();
    }
}

void VehicleConfigurationHelper::uAVOTransactionCompleted(UAVObject *object, bool success)
{
    if (object) {
        uAVOTransactionCompleted(object->getObjID(), success);
    }
}

void VehicleConfigurationHelper::saveChangesTimeout()
{
    m_transactionOK = false;
    m_transactionTimeout = true;
    m_eventLoop.quit();
}

void VehicleConfigurationHelper::resetVehicleConfig()
{
    // Reset all vehicle data
    MixerSettings *mSettings = MixerSettings::GetInstance(m_uavoManager);

    // Reset feed forward, accel times etc
    mSettings->setFeedForward(0.0f);
    mSettings->setMaxAccel(1000.0f);
    mSettings->setAccelTime(0.0f);
    mSettings->setDecelTime(0.0f);

    // Reset throttle curves
    QString throttlePattern = "ThrottleCurve%1";
    for (int i = 1; i <= 2; i++) {
        UAVObjectField *field = mSettings->getField(throttlePattern.arg(i));
        Q_ASSERT(field);
        // Set default curve at 90% max for Multirotors
        for (quint32 i = 0; i < field->getNumElements(); i++) {
            field->setValue(i * (1.0f / (field->getNumElements() - 1)), i);
        }
    }

    // Reset Mixer types and values
    QString mixerTypePattern   = "Mixer%1Type";
    QString mixerVectorPattern = "Mixer%1Vector";
    for (int i = 1; i <= 10; i++) {
        UAVObjectField *field = mSettings->getField(mixerTypePattern.arg(i));
        Q_ASSERT(field);
        field->setValue(field->getOptions().at(0));

        field = mSettings->getField(mixerVectorPattern.arg(i));
        Q_ASSERT(field);
        for (quint32 i = 0; i < field->getNumElements(); i++) {
            field->setValue(0, i);
        }
    }

    // Apply updates
    // mSettings->setData(mSettings->getData());
    addModifiedObject(mSettings, tr("Preparing mixer settings"));
}

void VehicleConfigurationHelper::resetGUIData()
{
    SystemSettings *sSettings = SystemSettings::GetInstance(m_uavoManager);

    Q_ASSERT(sSettings);
    SystemSettings::DataFields data = sSettings->getData();
    data.AirframeType = SystemSettings::AIRFRAMETYPE_CUSTOM;
    for (quint32 i = 0; i < SystemSettings::GUICONFIGDATA_NUMELEM; i++) {
        data.GUIConfigData[i] = 0;
    }
    sSettings->setData(data);
    addModifiedObject(sSettings, tr("Preparing vehicle settings"));
}


void VehicleConfigurationHelper::setupTriCopter()
{
    // Typical vehicle setup
    // 1. Setup mixer data
    // 2. Setup GUI data
    // 3. Apply changes

    mixerChannelSettings channels[ActuatorSettings::CHANNELADDR_NUMELEM];
    GUIConfigDataUnion guiSettings = getGUIConfigData();

    channels[0].type      = MIXER_TYPE_MOTOR;
    channels[0].throttle1 = 100;
    channels[0].throttle2 = 0;
    channels[0].roll      = 100;
    channels[0].pitch     = 50;
    channels[0].yaw = 0;

    channels[1].type      = MIXER_TYPE_MOTOR;
    channels[1].throttle1 = 100;
    channels[1].throttle2 = 0;
    channels[1].roll      = -100;
    channels[1].pitch     = 50;
    channels[1].yaw = 0;

    channels[2].type      = MIXER_TYPE_MOTOR;
    channels[2].throttle1 = 100;
    channels[2].throttle2 = 0;
    channels[2].roll      = 0;
    channels[2].pitch     = -100;
    channels[2].yaw = 0;

    channels[3].type      = MIXER_TYPE_SERVO;
    channels[3].throttle1 = 0;
    channels[3].throttle2 = 0;
    channels[3].roll      = 0;
    channels[3].pitch     = 0;
    channels[3].yaw = 100;

    guiSettings.multi.VTOLMotorNW = 1;
    guiSettings.multi.VTOLMotorNE = 2;
    guiSettings.multi.VTOLMotorS  = 3;
    guiSettings.multi.TRIYaw = 4;

    applyMixerConfiguration(channels);
    applyMultiGUISettings(SystemSettings::AIRFRAMETYPE_TRI, guiSettings);
}

GUIConfigDataUnion VehicleConfigurationHelper::getGUIConfigData()
{
    GUIConfigDataUnion configData;

    for (int i = 0; i < (int)(SystemSettings::GUICONFIGDATA_NUMELEM); i++) {
        configData.UAVObject[i] = 0; // systemSettingsData.GUIConfigData[i];
    }

    return configData;
}

void VehicleConfigurationHelper::setupQuadCopter()
{
    mixerChannelSettings channels[ActuatorSettings::CHANNELADDR_NUMELEM];
    GUIConfigDataUnion guiSettings = getGUIConfigData();
    SystemSettings::AirframeTypeOptions frame = SystemSettings::AIRFRAMETYPE_QUADX;

    switch (m_configSource->getVehicleSubType()) {
    case VehicleConfigurationSource::MULTI_ROTOR_QUAD_PLUS:
    {
        frame = SystemSettings::AIRFRAMETYPE_QUADP;
        channels[0].type      = MIXER_TYPE_MOTOR;
        channels[0].throttle1 = 100;
        channels[0].throttle2 = 0;
        channels[0].roll      = 0;
        channels[0].pitch     = 100;
        channels[0].yaw = -50;

        channels[1].type      = MIXER_TYPE_MOTOR;
        channels[1].throttle1 = 100;
        channels[1].throttle2 = 0;
        channels[1].roll      = -100;
        channels[1].pitch     = 0;
        channels[1].yaw = 50;

        channels[2].type      = MIXER_TYPE_MOTOR;
        channels[2].throttle1 = 100;
        channels[2].throttle2 = 0;
        channels[2].roll      = 0;
        channels[2].pitch     = -100;
        channels[2].yaw = -50;

        channels[3].type      = MIXER_TYPE_MOTOR;
        channels[3].throttle1 = 100;
        channels[3].throttle2 = 0;
        channels[3].roll      = 100;
        channels[3].pitch     = 0;
        channels[3].yaw = 50;

        guiSettings.multi.VTOLMotorN = 1;
        guiSettings.multi.VTOLMotorE = 2;
        guiSettings.multi.VTOLMotorS = 3;
        guiSettings.multi.VTOLMotorW = 4;

        break;
    }
    case VehicleConfigurationSource::MULTI_ROTOR_QUAD_X:
    {
        frame = SystemSettings::AIRFRAMETYPE_QUADX;
        channels[0].type      = MIXER_TYPE_MOTOR;
        channels[0].throttle1 = 100;
        channels[0].throttle2 = 0;
        channels[0].roll      = 50;
        channels[0].pitch     = 50;
        channels[0].yaw = -50;

        channels[1].type      = MIXER_TYPE_MOTOR;
        channels[1].throttle1 = 100;
        channels[1].throttle2 = 0;
        channels[1].roll      = -50;
        channels[1].pitch     = 50;
        channels[1].yaw = 50;

        channels[2].type      = MIXER_TYPE_MOTOR;
        channels[2].throttle1 = 100;
        channels[2].throttle2 = 0;
        channels[2].roll      = -50;
        channels[2].pitch     = -50;
        channels[2].yaw = -50;

        channels[3].type      = MIXER_TYPE_MOTOR;
        channels[3].throttle1 = 100;
        channels[3].throttle2 = 0;
        channels[3].roll      = 50;
        channels[3].pitch     = -50;
        channels[3].yaw = 50;

        guiSettings.multi.VTOLMotorNW = 1;
        guiSettings.multi.VTOLMotorNE = 2;
        guiSettings.multi.VTOLMotorSE = 3;
        guiSettings.multi.VTOLMotorSW = 4;

        break;
    }
    default:
        break;
    }
    applyMixerConfiguration(channels);
    applyMultiGUISettings(frame, guiSettings);
}

void VehicleConfigurationHelper::setupHexaCopter()
{
    mixerChannelSettings channels[ActuatorSettings::CHANNELADDR_NUMELEM];
    GUIConfigDataUnion guiSettings = getGUIConfigData();
    SystemSettings::AirframeTypeOptions frame = SystemSettings::AIRFRAMETYPE_HEXA;

    switch (m_configSource->getVehicleSubType()) {
    case VehicleConfigurationSource::MULTI_ROTOR_HEXA:
    {
        frame = SystemSettings::AIRFRAMETYPE_HEXA;
        // HexaPlus according to new mixer table and pitch-roll-yaw mixing at 100%
        // Pitch Roll Yaw
        // M1 {  1  , 0 , -1 },
        // M2 {  0.5, -1,  1 },
        // M3 { -0.5, -1, -1 },
        // M4 { -1  , 0 ,  1 },
        // M5 { -0.5, 1 , -1 },
        // M6 {  0.5, 1 ,  1 },
        channels[0].type      = MIXER_TYPE_MOTOR;
        channels[0].throttle1 = 100;
        channels[0].throttle2 = 0;
        channels[0].roll      = 0;
        channels[0].pitch     = 100;
        channels[0].yaw = -100;

        channels[1].type      = MIXER_TYPE_MOTOR;
        channels[1].throttle1 = 100;
        channels[1].throttle2 = 0;
        channels[1].roll      = -100;
        channels[1].pitch     = 50;
        channels[1].yaw = 100;

        channels[2].type      = MIXER_TYPE_MOTOR;
        channels[2].throttle1 = 100;
        channels[2].throttle2 = 0;
        channels[2].roll      = -100;
        channels[2].pitch     = -50;
        channels[2].yaw = -100;

        channels[3].type      = MIXER_TYPE_MOTOR;
        channels[3].throttle1 = 100;
        channels[3].throttle2 = 0;
        channels[3].roll      = 0;
        channels[3].pitch     = -100;
        channels[3].yaw = 100;

        channels[4].type      = MIXER_TYPE_MOTOR;
        channels[4].throttle1 = 100;
        channels[4].throttle2 = 0;
        channels[4].roll      = 100;
        channels[4].pitch     = -50;
        channels[4].yaw = -100;

        channels[5].type      = MIXER_TYPE_MOTOR;
        channels[5].throttle1 = 100;
        channels[5].throttle2 = 0;
        channels[5].roll      = 100;
        channels[5].pitch     = 50;
        channels[5].yaw = 100;

        guiSettings.multi.VTOLMotorN  = 1;
        guiSettings.multi.VTOLMotorNE = 2;
        guiSettings.multi.VTOLMotorSE = 3;
        guiSettings.multi.VTOLMotorS  = 4;
        guiSettings.multi.VTOLMotorSW = 5;
        guiSettings.multi.VTOLMotorNW = 6;

        break;
    }
    case VehicleConfigurationSource::MULTI_ROTOR_HEXA_COAX_Y:
    {
        frame = SystemSettings::AIRFRAMETYPE_HEXACOAX;

        channels[0].type      = MIXER_TYPE_MOTOR;
        channels[0].throttle1 = 100;
        channels[0].throttle2 = 0;
        channels[0].roll      = 100;
        channels[0].pitch     = 25;
        channels[0].yaw = -66;

        channels[1].type      = MIXER_TYPE_MOTOR;
        channels[1].throttle1 = 100;
        channels[1].throttle2 = 0;
        channels[1].roll      = 100;
        channels[1].pitch     = 25;
        channels[1].yaw = 66;

        channels[2].type      = MIXER_TYPE_MOTOR;
        channels[2].throttle1 = 100;
        channels[2].throttle2 = 0;
        channels[2].roll      = -100;
        channels[2].pitch     = 25;
        channels[2].yaw = -66;

        channels[3].type      = MIXER_TYPE_MOTOR;
        channels[3].throttle1 = 100;
        channels[3].throttle2 = 0;
        channels[3].roll      = -100;
        channels[3].pitch     = 25;
        channels[3].yaw = 66;

        channels[4].type      = MIXER_TYPE_MOTOR;
        channels[4].throttle1 = 100;
        channels[4].throttle2 = 0;
        channels[4].roll      = 0;
        channels[4].pitch     = -50;
        channels[4].yaw = -66;

        channels[5].type      = MIXER_TYPE_MOTOR;
        channels[5].throttle1 = 100;
        channels[5].throttle2 = 0;
        channels[5].roll      = 0;
        channels[5].pitch     = -50;
        channels[5].yaw = 66;

        guiSettings.multi.VTOLMotorNW = 1;
        guiSettings.multi.VTOLMotorW  = 2;
        guiSettings.multi.VTOLMotorNE = 3;
        guiSettings.multi.VTOLMotorE  = 4;
        guiSettings.multi.VTOLMotorS  = 5;
        guiSettings.multi.VTOLMotorSE = 6;

        break;
    }
    case VehicleConfigurationSource::MULTI_ROTOR_HEXA_H:
    {
        frame = SystemSettings::AIRFRAMETYPE_HEXAH;
        // HexaH according to new mixer table and pitch-roll-yaw mixing at 100%
        // Pitch Roll Yaw
        // M1 {  1 , -0.5, -0.5 },
        // M2 {  0 , -1  ,  1   },
        // M3 { -1 , -0.5, -0.5 },
        // M4 { -1 ,  0.5,  0.5 },
        // M5 {  0 ,  1  , -1   },
        // M6 {  1 ,  0.5,  0.5 },
        channels[0].type      = MIXER_TYPE_MOTOR;
        channels[0].throttle1 = 100;
        channels[0].throttle2 = 0;
        channels[0].roll      = -50;
        channels[0].pitch     = 100;
        channels[0].yaw = -50;

        channels[1].type      = MIXER_TYPE_MOTOR;
        channels[1].throttle1 = 100;
        channels[1].throttle2 = 0;
        channels[1].roll      = -100;
        channels[1].pitch     = 0;
        channels[1].yaw = 100;

        channels[2].type      = MIXER_TYPE_MOTOR;
        channels[2].throttle1 = 100;
        channels[2].throttle2 = 0;
        channels[2].roll      = -50;
        channels[2].pitch     = -100;
        channels[2].yaw = -50;

        channels[3].type      = MIXER_TYPE_MOTOR;
        channels[3].throttle1 = 100;
        channels[3].throttle2 = 0;
        channels[3].roll      = 50;
        channels[3].pitch     = -100;
        channels[3].yaw = 50;

        channels[4].type      = MIXER_TYPE_MOTOR;
        channels[4].throttle1 = 100;
        channels[4].throttle2 = 0;
        channels[4].roll      = 100;
        channels[4].pitch     = 0;
        channels[4].yaw = -100;

        channels[5].type      = MIXER_TYPE_MOTOR;
        channels[5].throttle1 = 100;
        channels[5].throttle2 = 0;
        channels[5].roll      = 50;
        channels[5].pitch     = 100;
        channels[5].yaw = 50;

        guiSettings.multi.VTOLMotorNE = 1;
        guiSettings.multi.VTOLMotorE  = 2;
        guiSettings.multi.VTOLMotorSE = 3;
        guiSettings.multi.VTOLMotorSW = 4;
        guiSettings.multi.VTOLMotorW  = 5;
        guiSettings.multi.VTOLMotorNW = 6;

        break;
    }
    case VehicleConfigurationSource::MULTI_ROTOR_HEXA_X:
    {
        frame = SystemSettings::AIRFRAMETYPE_HEXAX;
        // HexaX according to new mixer table and pitch-roll-yaw mixing at 100%
        // Pitch Roll Yaw
        // M1 {  1, -0.5, -1 },
        // M2 {  0, -1  ,  1 },
        // M3 { -1, -0.5, -1 },
        // M4 { -1,  0.5,  1 },
        // M5 {  0,  1  , -1 },
        // M6 {  1,  0.5,  1 },
        channels[0].type      = MIXER_TYPE_MOTOR;
        channels[0].throttle1 = 100;
        channels[0].throttle2 = 0;
        channels[0].roll      = -50;
        channels[0].pitch     = 100;
        channels[0].yaw = -100;

        channels[1].type      = MIXER_TYPE_MOTOR;
        channels[1].throttle1 = 100;
        channels[1].throttle2 = 0;
        channels[1].roll      = -100;
        channels[1].pitch     = 0;
        channels[1].yaw = 100;

        channels[2].type      = MIXER_TYPE_MOTOR;
        channels[2].throttle1 = 100;
        channels[2].throttle2 = 0;
        channels[2].roll      = -50;
        channels[2].pitch     = -100;
        channels[2].yaw = -100;

        channels[3].type      = MIXER_TYPE_MOTOR;
        channels[3].throttle1 = 100;
        channels[3].throttle2 = 0;
        channels[3].roll      = 50;
        channels[3].pitch     = -100;
        channels[3].yaw = 100;

        channels[4].type      = MIXER_TYPE_MOTOR;
        channels[4].throttle1 = 100;
        channels[4].throttle2 = 0;
        channels[4].roll      = 100;
        channels[4].pitch     = 0;
        channels[4].yaw = -100;

        channels[5].type      = MIXER_TYPE_MOTOR;
        channels[5].throttle1 = 100;
        channels[5].throttle2 = 0;
        channels[5].roll      = 50;
        channels[5].pitch     = 100;
        channels[5].yaw = 100;

        guiSettings.multi.VTOLMotorNE = 1;
        guiSettings.multi.VTOLMotorE  = 2;
        guiSettings.multi.VTOLMotorSE = 3;
        guiSettings.multi.VTOLMotorSW = 4;
        guiSettings.multi.VTOLMotorW  = 5;
        guiSettings.multi.VTOLMotorNW = 6;

        break;
    }
    default:
        break;
    }
    applyMixerConfiguration(channels);
    applyMultiGUISettings(frame, guiSettings);
}

void VehicleConfigurationHelper::setupOctoCopter()
{
    mixerChannelSettings channels[ActuatorSettings::CHANNELADDR_NUMELEM];
    GUIConfigDataUnion guiSettings = getGUIConfigData();
    SystemSettings::AirframeTypeOptions frame = SystemSettings::AIRFRAMETYPE_OCTO;

    switch (m_configSource->getVehicleSubType()) {
    case VehicleConfigurationSource::MULTI_ROTOR_OCTO:
    {
        frame = SystemSettings::AIRFRAMETYPE_OCTO;
        // OctoP according to new mixer table and pitch-roll-yaw mixing at 100%
        // Pitch Roll Yaw
        // M1{  1   , 0   , -1 },
        // M2{  0.71,-0.71,  1 },
        // M3{  0   ,-1   , -1 },
        // M4{ -0.71,-0.71,  1 },
        // M5{ -1   , 0   , -1 },
        // M6{ -0.71, 0.71,  1 },
        // M7{  0   ,  1  , -1 },
        // M8{  0.71, 0.71,  1 }
        channels[0].type      = MIXER_TYPE_MOTOR;
        channels[0].throttle1 = 100;
        channels[0].throttle2 = 0;
        channels[0].roll      = 0;
        channels[0].pitch     = 100;
        channels[0].yaw = -100;

        channels[1].type      = MIXER_TYPE_MOTOR;
        channels[1].throttle1 = 100;
        channels[1].throttle2 = 0;
        channels[1].roll      = -71;
        channels[1].pitch     = 71;
        channels[1].yaw = 100;

        channels[2].type      = MIXER_TYPE_MOTOR;
        channels[2].throttle1 = 100;
        channels[2].throttle2 = 0;
        channels[2].roll      = -71;
        channels[2].pitch     = 0;
        channels[2].yaw = -100;

        channels[3].type      = MIXER_TYPE_MOTOR;
        channels[3].throttle1 = 100;
        channels[3].throttle2 = 0;
        channels[3].roll      = -71;
        channels[3].pitch     = -71;
        channels[3].yaw = 100;

        channels[4].type      = MIXER_TYPE_MOTOR;
        channels[4].throttle1 = 100;
        channels[4].throttle2 = 0;
        channels[4].roll      = 0;
        channels[4].pitch     = -100;
        channels[4].yaw = -100;

        channels[5].type      = MIXER_TYPE_MOTOR;
        channels[5].throttle1 = 100;
        channels[5].throttle2 = 0;
        channels[5].roll      = 71;
        channels[5].pitch     = -71;
        channels[5].yaw = 100;

        channels[6].type      = MIXER_TYPE_MOTOR;
        channels[6].throttle1 = 100;
        channels[6].throttle2 = 0;
        channels[6].roll      = 100;
        channels[6].pitch     = 0;
        channels[6].yaw = -100;

        channels[7].type      = MIXER_TYPE_MOTOR;
        channels[7].throttle1 = 100;
        channels[7].throttle2 = 0;
        channels[7].roll      = 71;
        channels[7].pitch     = 71;
        channels[7].yaw = 100;

        guiSettings.multi.VTOLMotorN  = 1;
        guiSettings.multi.VTOLMotorNE = 2;
        guiSettings.multi.VTOLMotorE  = 3;
        guiSettings.multi.VTOLMotorSE = 4;
        guiSettings.multi.VTOLMotorS  = 5;
        guiSettings.multi.VTOLMotorSW = 6;
        guiSettings.multi.VTOLMotorW  = 7;
        guiSettings.multi.VTOLMotorNW = 8;

        break;
    }
    case VehicleConfigurationSource::MULTI_ROTOR_OCTO_X:
    {
        frame = SystemSettings::AIRFRAMETYPE_OCTOX;
        // OctoX according to new mixer table and pitch-roll-yaw mixing at 100%
        // Pitch  Roll   Yaw
        // M1{  1   ,-0.41,  -1 },
        // M2{  0.41,  -1 ,   1 },
        // M3{ -0.41,  -1 ,  -1 },
        // M4{ -1   ,-0.41,   1 },
        // M5{ -1   , 0.41,  -1 },
        // M6{ -0.41,   1 ,   1 },
        // M7{  0.41,   1 ,  -1 },
        // M8{  1   , 0.41,   1 }
        channels[0].type      = MIXER_TYPE_MOTOR;
        channels[0].throttle1 = 100;
        channels[0].throttle2 = 0;
        channels[0].roll      = -41;
        channels[0].pitch     = 100;
        channels[0].yaw = -100;

        channels[1].type      = MIXER_TYPE_MOTOR;
        channels[1].throttle1 = 100;
        channels[1].throttle2 = 0;
        channels[1].roll      = -100;
        channels[1].pitch     = 41;
        channels[1].yaw = 100;

        channels[2].type      = MIXER_TYPE_MOTOR;
        channels[2].throttle1 = 100;
        channels[2].throttle2 = 0;
        channels[2].roll      = -100;
        channels[2].pitch     = -41;
        channels[2].yaw = -100;

        channels[3].type      = MIXER_TYPE_MOTOR;
        channels[3].throttle1 = 100;
        channels[3].throttle2 = 0;
        channels[3].roll      = -41;
        channels[3].pitch     = -100;
        channels[3].yaw = 100;

        channels[4].type      = MIXER_TYPE_MOTOR;
        channels[4].throttle1 = 100;
        channels[4].throttle2 = 0;
        channels[4].roll      = 41;
        channels[4].pitch     = -100;
        channels[4].yaw = -100;

        channels[5].type      = MIXER_TYPE_MOTOR;
        channels[5].throttle1 = 100;
        channels[5].throttle2 = 0;
        channels[5].roll      = 100;
        channels[5].pitch     = -41;
        channels[5].yaw = 100;

        channels[6].type      = MIXER_TYPE_MOTOR;
        channels[6].throttle1 = 100;
        channels[6].throttle2 = 0;
        channels[6].roll      = 100;
        channels[6].pitch     = 41;
        channels[6].yaw = -100;

        channels[7].type      = MIXER_TYPE_MOTOR;
        channels[7].throttle1 = 100;
        channels[7].throttle2 = 0;
        channels[7].roll      = 41;
        channels[7].pitch     = 100;
        channels[7].yaw = 100;

        guiSettings.multi.VTOLMotorNNE = 1;
        guiSettings.multi.VTOLMotorENE = 2;
        guiSettings.multi.VTOLMotorESE = 3;
        guiSettings.multi.VTOLMotorSSE = 4;
        guiSettings.multi.VTOLMotorSSW = 5;
        guiSettings.multi.VTOLMotorWSW = 6;
        guiSettings.multi.VTOLMotorWNW = 7;
        guiSettings.multi.VTOLMotorNNW = 8;

        break;
    }
    case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_X:
    {
        frame = SystemSettings::AIRFRAMETYPE_OCTOCOAXX;

        channels[0].type      = MIXER_TYPE_MOTOR;
        channels[0].throttle1 = 100;
        channels[0].throttle2 = 0;
        channels[0].roll      = 50;
        channels[0].pitch     = 50;
        channels[0].yaw = -50;

        channels[1].type      = MIXER_TYPE_MOTOR;
        channels[1].throttle1 = 100;
        channels[1].throttle2 = 0;
        channels[1].roll      = 50;
        channels[1].pitch     = 50;
        channels[1].yaw = 50;

        channels[2].type      = MIXER_TYPE_MOTOR;
        channels[2].throttle1 = 100;
        channels[2].throttle2 = 0;
        channels[2].roll      = -50;
        channels[2].pitch     = 50;
        channels[2].yaw = -50;

        channels[3].type      = MIXER_TYPE_MOTOR;
        channels[3].throttle1 = 100;
        channels[3].throttle2 = 0;
        channels[3].roll      = -50;
        channels[3].pitch     = 50;
        channels[3].yaw = 50;

        channels[4].type      = MIXER_TYPE_MOTOR;
        channels[4].throttle1 = 100;
        channels[4].throttle2 = 0;
        channels[4].roll      = -50;
        channels[4].pitch     = -50;
        channels[4].yaw = -50;

        channels[5].type      = MIXER_TYPE_MOTOR;
        channels[5].throttle1 = 100;
        channels[5].throttle2 = 0;
        channels[5].roll      = -50;
        channels[5].pitch     = -50;
        channels[5].yaw = 50;

        channels[6].type      = MIXER_TYPE_MOTOR;
        channels[6].throttle1 = 100;
        channels[6].throttle2 = 0;
        channels[6].roll      = 50;
        channels[6].pitch     = -50;
        channels[6].yaw = -50;

        channels[7].type      = MIXER_TYPE_MOTOR;
        channels[7].throttle1 = 100;
        channels[7].throttle2 = 0;
        channels[7].roll      = 50;
        channels[7].pitch     = -50;
        channels[7].yaw = 50;

        guiSettings.multi.VTOLMotorNW = 1;
        guiSettings.multi.VTOLMotorN  = 2;
        guiSettings.multi.VTOLMotorNE = 3;
        guiSettings.multi.VTOLMotorE  = 4;
        guiSettings.multi.VTOLMotorSE = 5;
        guiSettings.multi.VTOLMotorS  = 6;
        guiSettings.multi.VTOLMotorSW = 7;
        guiSettings.multi.VTOLMotorW  = 8;

        break;
    }
    case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_PLUS:
    {
        frame = SystemSettings::AIRFRAMETYPE_OCTOCOAXP;

        channels[0].type      = MIXER_TYPE_MOTOR;
        channels[0].throttle1 = 100;
        channels[0].throttle2 = 0;
        channels[0].roll      = 0;
        channels[0].pitch     = 100;
        channels[0].yaw = -50;

        channels[1].type      = MIXER_TYPE_MOTOR;
        channels[1].throttle1 = 100;
        channels[1].throttle2 = 0;
        channels[1].roll      = 0;
        channels[1].pitch     = 100;
        channels[1].yaw = 50;

        channels[2].type      = MIXER_TYPE_MOTOR;
        channels[2].throttle1 = 100;
        channels[2].throttle2 = 0;
        channels[2].roll      = -100;
        channels[2].pitch     = 0;
        channels[2].yaw = -50;

        channels[3].type      = MIXER_TYPE_MOTOR;
        channels[3].throttle1 = 100;
        channels[3].throttle2 = 0;
        channels[3].roll      = -100;
        channels[3].pitch     = 0;
        channels[3].yaw = 50;

        channels[4].type      = MIXER_TYPE_MOTOR;
        channels[4].throttle1 = 100;
        channels[4].throttle2 = 0;
        channels[4].roll      = 0;
        channels[4].pitch     = -100;
        channels[4].yaw = -50;

        channels[5].type      = MIXER_TYPE_MOTOR;
        channels[5].throttle1 = 100;
        channels[5].throttle2 = 0;
        channels[5].roll      = 0;
        channels[5].pitch     = -100;
        channels[5].yaw = 50;

        channels[6].type      = MIXER_TYPE_MOTOR;
        channels[6].throttle1 = 100;
        channels[6].throttle2 = 0;
        channels[6].roll      = 100;
        channels[6].pitch     = 0;
        channels[6].yaw = -50;

        channels[7].type      = MIXER_TYPE_MOTOR;
        channels[7].throttle1 = 100;
        channels[7].throttle2 = 0;
        channels[7].roll      = 100;
        channels[7].pitch     = 0;
        channels[7].yaw = 50;

        guiSettings.multi.VTOLMotorN  = 1;
        guiSettings.multi.VTOLMotorNE = 2;
        guiSettings.multi.VTOLMotorE  = 3;
        guiSettings.multi.VTOLMotorSE = 4;
        guiSettings.multi.VTOLMotorS  = 5;
        guiSettings.multi.VTOLMotorSW = 6;
        guiSettings.multi.VTOLMotorW  = 7;
        guiSettings.multi.VTOLMotorNW = 8;

        break;
    }
    case VehicleConfigurationSource::MULTI_ROTOR_OCTO_V:
    {
        frame = SystemSettings::AIRFRAMETYPE_OCTOV;
        channels[0].type      = MIXER_TYPE_MOTOR;
        channels[0].throttle1 = 100;
        channels[0].throttle2 = 0;
        channels[0].roll      = -25;
        channels[0].pitch     = 8;
        channels[0].yaw = -25;

        channels[1].type      = MIXER_TYPE_MOTOR;
        channels[1].throttle1 = 100;
        channels[1].throttle2 = 0;
        channels[1].roll      = -25;
        channels[1].pitch     = 25;
        channels[1].yaw = 25;

        channels[2].type      = MIXER_TYPE_MOTOR;
        channels[2].throttle1 = 100;
        channels[2].throttle2 = 0;
        channels[2].roll      = -25;
        channels[2].pitch     = -25;
        channels[2].yaw = -25;

        channels[3].type      = MIXER_TYPE_MOTOR;
        channels[3].throttle1 = 100;
        channels[3].throttle2 = 0;
        channels[3].roll      = -25;
        channels[3].pitch     = -8;
        channels[3].yaw = 25;

        channels[4].type      = MIXER_TYPE_MOTOR;
        channels[4].throttle1 = 100;
        channels[4].throttle2 = 0;
        channels[4].roll      = 25;
        channels[4].pitch     = -8;
        channels[4].yaw = -25;

        channels[5].type      = MIXER_TYPE_MOTOR;
        channels[5].throttle1 = 100;
        channels[5].throttle2 = 0;
        channels[5].roll      = 25;
        channels[5].pitch     = -25;
        channels[5].yaw = 25;

        channels[6].type      = MIXER_TYPE_MOTOR;
        channels[6].throttle1 = 100;
        channels[6].throttle2 = 0;
        channels[6].roll      = 25;
        channels[6].pitch     = 25;
        channels[6].yaw = -25;

        channels[7].type      = MIXER_TYPE_MOTOR;
        channels[7].throttle1 = 100;
        channels[7].throttle2 = 0;
        channels[7].roll      = 25;
        channels[7].pitch     = 8;
        channels[7].yaw = 25;

        guiSettings.multi.VTOLMotorN  = 1;
        guiSettings.multi.VTOLMotorNE = 2;
        guiSettings.multi.VTOLMotorE  = 3;
        guiSettings.multi.VTOLMotorSE = 4;
        guiSettings.multi.VTOLMotorS  = 5;
        guiSettings.multi.VTOLMotorSW = 6;
        guiSettings.multi.VTOLMotorW  = 7;
        guiSettings.multi.VTOLMotorNW = 8;

        break;
    }
    default:
        break;
    }

    applyMixerConfiguration(channels);
    applyMultiGUISettings(frame, guiSettings);
}

void VehicleConfigurationHelper::setupElevon()
{
    mixerChannelSettings channels[ActuatorSettings::CHANNELADDR_NUMELEM];
    GUIConfigDataUnion guiSettings = getGUIConfigData();

    // Motor (Chan 3)
    channels[2].type      = MIXER_TYPE_MOTOR;
    channels[2].throttle1 = 100;
    channels[2].throttle2 = 0;
    channels[2].roll      = 0;
    channels[2].pitch     = 0;
    channels[2].yaw       = 0;

    // Elevon Servo 1 (Chan 1)
    channels[0].type      = MIXER_TYPE_SERVO;
    channels[0].throttle1 = 0;
    channels[0].throttle2 = 0;
    channels[0].roll      = 100;
    channels[0].pitch     = -100;
    channels[0].yaw       = 0;

    // Elevon Servo 2 (Chan 2)
    channels[1].type      = MIXER_TYPE_SERVO;
    channels[1].throttle1 = 0;
    channels[1].throttle2 = 0;
    channels[1].roll      = 100;
    channels[1].pitch     = 100;
    channels[1].yaw       = 0;

    guiSettings.fixedwing.FixedWingThrottle = 3;
    guiSettings.fixedwing.FixedWingRoll1    = 1;
    guiSettings.fixedwing.FixedWingRoll2    = 2;

    applyMixerConfiguration(channels);
    applyMultiGUISettings(SystemSettings::AIRFRAMETYPE_FIXEDWINGELEVON, guiSettings);
}

void VehicleConfigurationHelper::setupDualAileron()
{
    // Typical vehicle setup
    // 1. Setup mixer data
    // 2. Setup GUI data
    // 3. Apply changes

    mixerChannelSettings channels[ActuatorSettings::CHANNELADDR_NUMELEM];
    GUIConfigDataUnion guiSettings = getGUIConfigData();

    // Motor (Chan 3)
    channels[2].type      = MIXER_TYPE_MOTOR;
    channels[2].throttle1 = 100;
    channels[2].throttle2 = 0;
    channels[2].roll      = 0;
    channels[2].pitch     = 0;
    channels[2].yaw       = 0;

    // Aileron Servo 1 (Chan 1)
    channels[0].type      = MIXER_TYPE_SERVO;
    channels[0].throttle1 = 0;
    channels[0].throttle2 = 0;
    channels[0].roll      = 100;
    channels[0].pitch     = 0;
    channels[0].yaw       = 0;

    // Aileron Servo 2 (Chan 6)
    channels[5].type      = MIXER_TYPE_SERVO;
    channels[5].throttle1 = 0;
    channels[5].throttle2 = 0;
    channels[5].roll      = 100;
    channels[5].pitch     = 0;
    channels[5].yaw       = 0;

    // Elevator Servo (Chan 2)
    channels[1].type      = MIXER_TYPE_SERVO;
    channels[1].throttle1 = 0;
    channels[1].throttle2 = 0;
    channels[1].roll      = 0;
    channels[1].pitch     = 100;
    channels[1].yaw       = 0;

    // Rudder Servo (Chan 4)
    channels[3].type      = MIXER_TYPE_SERVO;
    channels[3].throttle1 = 0;
    channels[3].throttle2 = 0;
    channels[3].roll      = 0;
    channels[3].pitch     = 0;
    channels[3].yaw       = -100;

    guiSettings.fixedwing.FixedWingThrottle = 3;
    guiSettings.fixedwing.FixedWingRoll1    = 1;
    guiSettings.fixedwing.FixedWingRoll2    = 6;
    guiSettings.fixedwing.FixedWingPitch1   = 2;
    guiSettings.fixedwing.FixedWingYaw1     = 4;

    applyMixerConfiguration(channels);
    applyMultiGUISettings(SystemSettings::AIRFRAMETYPE_FIXEDWING, guiSettings);
}

void VehicleConfigurationHelper::setupAileron()
{
    // Typical vehicle setup
    // 1. Setup mixer data
    // 2. Setup GUI data
    // 3. Apply changes

    mixerChannelSettings channels[ActuatorSettings::CHANNELADDR_NUMELEM];
    GUIConfigDataUnion guiSettings = getGUIConfigData();

    // Motor (Chan 3)
    channels[2].type      = MIXER_TYPE_MOTOR;
    channels[2].throttle1 = 100;
    channels[2].throttle2 = 0;
    channels[2].roll      = 0;
    channels[2].pitch     = 0;
    channels[2].yaw       = 0;

    // Aileron Servo (Chan 1)
    channels[0].type      = MIXER_TYPE_SERVO;
    channels[0].throttle1 = 0;
    channels[0].throttle2 = 0;
    channels[0].roll      = 100;
    channels[0].pitch     = 0;
    channels[0].yaw       = 0;

    // Elevator Servo (Chan 2)
    channels[1].type      = MIXER_TYPE_SERVO;
    channels[1].throttle1 = 0;
    channels[1].throttle2 = 0;
    channels[1].roll      = 0;
    channels[1].pitch     = 100;
    channels[1].yaw       = 0;

    // Rudder Servo (Chan 4)
    channels[3].type      = MIXER_TYPE_SERVO;
    channels[3].throttle1 = 0;
    channels[3].throttle2 = 0;
    channels[3].roll      = 0;
    channels[3].pitch     = 0;
    channels[3].yaw       = -100;

    guiSettings.fixedwing.FixedWingThrottle = 3;
    guiSettings.fixedwing.FixedWingRoll1    = 1;
    guiSettings.fixedwing.FixedWingPitch1   = 2;
    guiSettings.fixedwing.FixedWingYaw1     = 4;

    applyMixerConfiguration(channels);
    applyMultiGUISettings(SystemSettings::AIRFRAMETYPE_FIXEDWING, guiSettings);
}

void VehicleConfigurationHelper::setupVtail()
{
    // Typical vehicle setup
    // 1. Setup mixer data
    // 2. Setup GUI data
    // 3. Apply changes

    mixerChannelSettings channels[ActuatorSettings::CHANNELADDR_NUMELEM];
    GUIConfigDataUnion guiSettings = getGUIConfigData();

    // Motor (Chan 3)
    channels[2].type      = MIXER_TYPE_MOTOR;
    channels[2].throttle1 = 100;
    channels[2].throttle2 = 0;
    channels[2].roll      = 0;
    channels[2].pitch     = 0;
    channels[2].yaw       = 0;

    // Aileron Servo (Chan 1)
    channels[0].type      = MIXER_TYPE_SERVO;
    channels[0].throttle1 = 0;
    channels[0].throttle2 = 0;
    channels[0].roll      = 100;
    channels[0].pitch     = 0;
    channels[0].yaw       = 0;

    // Aileron Servo 2 (Chan 6)
    channels[5].type      = MIXER_TYPE_SERVO;
    channels[5].throttle1 = 0;
    channels[5].throttle2 = 0;
    channels[5].roll      = 100;
    channels[5].pitch     = 0;
    channels[5].yaw       = 0;

    // Right Vtail Servo (Chan 2)
    channels[1].type      = MIXER_TYPE_SERVO;
    channels[1].throttle1 = 0;
    channels[1].throttle2 = 0;
    channels[1].roll      = 0;
    channels[1].pitch     = 100;
    channels[1].yaw       = -100;

    // Left Vtail Servo (Chan 4)
    channels[3].type      = MIXER_TYPE_SERVO;
    channels[3].throttle1 = 0;
    channels[3].throttle2 = 0;
    channels[3].roll      = 0;
    channels[3].pitch     = -100;
    channels[3].yaw       = -100;

    guiSettings.fixedwing.FixedWingThrottle = 3;
    guiSettings.fixedwing.FixedWingRoll1    = 1;
    guiSettings.fixedwing.FixedWingRoll2    = 6;
    guiSettings.fixedwing.FixedWingPitch1   = 4; // Vtail left (top view, nose up)
    guiSettings.fixedwing.FixedWingPitch2   = 2; // Vtail right

    applyMixerConfiguration(channels);
    applyMultiGUISettings(SystemSettings::AIRFRAMETYPE_FIXEDWINGVTAIL, guiSettings);
}

/*
 *
 * Ground vehicles
 *
 */

void VehicleConfigurationHelper::setupCar()
{
    // Typical vehicle setup
    // 1. Setup mixer data
    // 2. Setup GUI data
    // 3. Apply changes

    mixerChannelSettings channels[ActuatorSettings::CHANNELADDR_NUMELEM];
    GUIConfigDataUnion guiSettings = getGUIConfigData();

    // Steering Servo (Chan 1)
    channels[0].type      = MIXER_TYPE_SERVO;
    channels[0].throttle1 = 0;
    channels[0].throttle2 = 0;
    channels[0].roll      = 0;
    channels[0].pitch     = 0;
    channels[0].yaw       = 100;

    // Motor (Chan 2)
    channels[1].type      = MIXER_TYPE_REVERSABLEMOTOR;
    channels[1].throttle1 = 100;
    channels[1].throttle2 = 0;
    channels[1].roll      = 0;
    channels[1].pitch     = 0;
    channels[1].yaw       = 0;

    guiSettings.ground.GroundVehicleSteering1 = 1;
    guiSettings.ground.GroundVehicleThrottle2 = 2;

    applyMixerConfiguration(channels);
    applyMultiGUISettings(SystemSettings::AIRFRAMETYPE_GROUNDVEHICLECAR, guiSettings);
}

void VehicleConfigurationHelper::setupTank()
{
    // Typical vehicle setup
    // 1. Setup mixer data
    // 2. Setup GUI data
    // 3. Apply changes

    mixerChannelSettings channels[ActuatorSettings::CHANNELADDR_NUMELEM];
    GUIConfigDataUnion guiSettings = getGUIConfigData();

    // Left Motor (Chan 1)
    channels[0].type      = MIXER_TYPE_REVERSABLEMOTOR;
    channels[0].throttle1 = 100;
    channels[0].throttle2 = 0;
    channels[0].roll      = 0;
    channels[0].pitch     = 0;
    channels[0].yaw       = 100;

    // Right Motor (Chan 2)
    channels[1].type      = MIXER_TYPE_REVERSABLEMOTOR;
    channels[1].throttle1 = 100;
    channels[1].throttle2 = 0;
    channels[1].roll      = 0;
    channels[1].pitch     = 0;
    channels[1].yaw       = -100;

    guiSettings.ground.GroundVehicleThrottle1 = 1;
    guiSettings.ground.GroundVehicleThrottle2 = 2;

    applyMixerConfiguration(channels);
    applyMultiGUISettings(SystemSettings::AIRFRAMETYPE_GROUNDVEHICLEDIFFERENTIAL, guiSettings);
}

void VehicleConfigurationHelper::setupMotorcycle()
{
    // Typical vehicle setup
    // 1. Setup mixer data
    // 2. Setup GUI data
    // 3. Apply changes

    mixerChannelSettings channels[ActuatorSettings::CHANNELADDR_NUMELEM];
    GUIConfigDataUnion guiSettings = getGUIConfigData();

    // Steering Servo (Chan 1)
    channels[0].type      = MIXER_TYPE_SERVO;
    channels[0].throttle1 = 0;
    channels[0].throttle2 = 0;
    channels[0].roll      = 0;
    channels[0].pitch     = 0;
    channels[0].yaw       = 100;

    // Motor (Chan 2)
    channels[1].type      = MIXER_TYPE_MOTOR;
    channels[1].throttle1 = 100;
    channels[1].throttle2 = 0;
    channels[1].roll      = 0;
    channels[1].pitch     = 0;
    channels[1].yaw       = 0;

    guiSettings.ground.GroundVehicleSteering1 = 1;
    guiSettings.ground.GroundVehicleThrottle2 = 2;

    applyMixerConfiguration(channels);
    applyMultiGUISettings(SystemSettings::AIRFRAMETYPE_GROUNDVEHICLEMOTORCYCLE, guiSettings);
}
