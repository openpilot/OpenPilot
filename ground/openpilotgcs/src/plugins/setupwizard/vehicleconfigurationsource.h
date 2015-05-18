/**
 ******************************************************************************
 *
 * @file       vehicleconfigurationsource.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup VehicleConfigurationSource
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

#ifndef VEHICLECONFIGURATIONSOURCE_H
#define VEHICLECONFIGURATIONSOURCE_H

#include <QString>
#include <QJsonObject>
#include "actuatorsettings.h"

struct accelGyroBias {
    float m_accelerometerXBias;
    float m_accelerometerYBias;
    float m_accelerometerZBias;

    float m_gyroXBias;
    float m_gyroYBias;
    float m_gyroZBias;
};

struct actuatorChannelSettings {
    quint16 channelMin;
    quint16 channelNeutral;
    quint16 channelMax;
    bool    isReversableMotor;

    // Default values
    actuatorChannelSettings() : channelMin(1000), channelNeutral(1000), channelMax(1900), isReversableMotor(false) {}
};


class VehicleConfigurationSource {
public:
    VehicleConfigurationSource();

    enum CONTROLLER_TYPE { CONTROLLER_UNKNOWN, CONTROLLER_REVO, CONTROLLER_NANO, CONTROLLER_OPLINK, CONTROLLER_DISCOVERYF4 };
    enum VEHICLE_TYPE { VEHICLE_UNKNOWN, VEHICLE_MULTI, VEHICLE_FIXEDWING, VEHICLE_HELI, VEHICLE_SURFACE };
    enum VEHICLE_SUB_TYPE { MULTI_ROTOR_UNKNOWN, MULTI_ROTOR_TRI_Y, MULTI_ROTOR_QUAD_X, MULTI_ROTOR_QUAD_PLUS, MULTI_ROTOR_QUAD_H,
                            MULTI_ROTOR_HEXA, MULTI_ROTOR_HEXA_H, MULTI_ROTOR_HEXA_X, MULTI_ROTOR_HEXA_COAX_Y, MULTI_ROTOR_OCTO,
                            MULTI_ROTOR_OCTO_X, MULTI_ROTOR_OCTO_V, MULTI_ROTOR_OCTO_COAX_X, MULTI_ROTOR_OCTO_COAX_PLUS,
                            FIXED_WING_DUAL_AILERON, FIXED_WING_AILERON, FIXED_WING_ELEVON, FIXED_WING_VTAIL, HELI_CCPM,
                            GROUNDVEHICLE_MOTORCYCLE, GROUNDVEHICLE_CAR, GROUNDVEHICLE_DIFFERENTIAL };
    enum ESC_TYPE { ESC_ONESHOT, ESC_SYNCHED, ESC_RAPID, ESC_STANDARD, ESC_UNKNOWN };
    enum SERVO_TYPE { SERVO_ANALOG, SERVO_DIGITAL, SERVO_UNKNOWN };
    enum INPUT_TYPE { INPUT_PWM, INPUT_PPM, INPUT_SBUS, INPUT_DSM, INPUT_UNKNOWN };
    enum AIRSPEED_TYPE { AIRSPEED_ESTIMATE, AIRSPEED_EAGLETREE, AIRSPEED_MS4525, AIRSPEED_DISABLED };
    enum GPS_TYPE { GPS_PLATINUM, GPS_UBX, GPS_NMEA, GPS_DISABLED };
    enum RADIO_SETTING { RADIO_TELEMETRY, RADIO_DISABLED };

    virtual VehicleConfigurationSource::CONTROLLER_TYPE getControllerType() const  = 0;
    virtual VehicleConfigurationSource::VEHICLE_TYPE getVehicleType() const = 0;
    virtual VehicleConfigurationSource::VEHICLE_SUB_TYPE getVehicleSubType() const = 0;
    virtual VehicleConfigurationSource::INPUT_TYPE getInputType() const = 0;
    virtual VehicleConfigurationSource::ESC_TYPE getEscType() const     = 0;
    virtual VehicleConfigurationSource::SERVO_TYPE getServoType() const = 0;
    virtual VehicleConfigurationSource::AIRSPEED_TYPE getAirspeedType() const = 0;
    virtual VehicleConfigurationSource::GPS_TYPE getGpsType() const     = 0;
    virtual VehicleConfigurationSource::RADIO_SETTING getRadioSetting() const = 0;
    virtual QJsonObject *getVehicleTemplate() const  = 0;

    virtual bool isCalibrationPerformed() const      = 0;
    virtual accelGyroBias getCalibrationBias() const = 0;

    virtual bool isMotorCalibrationPerformed() const = 0;
    virtual QList<actuatorChannelSettings> getActuatorSettings() const = 0;

    virtual bool isRestartNeeded() const = 0;

    virtual QString getSummaryText()     = 0;
};

#endif // VEHICLECONFIGURATIONSOURCE_H
