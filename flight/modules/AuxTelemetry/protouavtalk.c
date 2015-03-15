/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup AuxTelemetryModule AuxTelemetry Module
 * @brief Auxiliary telemetry module UAVTalk protocol
 * Implements output in UAVTalk format. Little more than a wrapper for
 * the UAVTalk library.
 * @{
 *
 * @file       protouavtalk.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Lightweight telemetry module UAVTalk protocol.
 * @see        The GNU Public License (GPL) Version 3
 *
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

#include <openpilot.h>
#include <pios_com.h>

#include "auxtelemetry_priv.h"

// Private constants

// Private types

// Private variables
static uint32_t outputPort;
static UAVTalkConnection uavTalkCon;

// Private functions
static void initialize(uint32_t comPort);
static void updateData(AuxTelemetrySettingsUpdateIntervalsElem data);
static int32_t transmitData(uint8_t *data, int32_t length);

protocolHandler_t uavtalkProtocolHandler = {
    .initialize = initialize,
    .updateData = updateData,
};

static void initialize(uint32_t comPort)
{
    outputPort = comPort;
    uavTalkCon = UAVTalkInitialize(&transmitData);
}

static void updateData(AuxTelemetrySettingsUpdateIntervalsElem data)
{
    UAVObjHandle obj1 = NULL;
    UAVObjHandle obj2 = NULL;

    switch (data) {
    case AUXTELEMETRYSETTINGS_UPDATEINTERVALS_ATTITUDE:
        obj1 = AttitudeStateHandle();
        break;
    case AUXTELEMETRYSETTINGS_UPDATEINTERVALS_GPSDATA:
        obj1 = GPSPositionSensorHandle();
        obj2 = GPSVelocitySensorHandle();
        break;
    case AUXTELEMETRYSETTINGS_UPDATEINTERVALS_MANUALCONTROL:
        obj1 = ManualControlCommandHandle();
        break;
    case AUXTELEMETRYSETTINGS_UPDATEINTERVALS_SYSTEMSTATUS:
        obj1 = FlightStatusHandle();
        obj2 = SystemAlarmsHandle();
        break;
    case AUXTELEMETRYSETTINGS_UPDATEINTERVALS_FLIGHTBATTERY:
        obj1 = FlightBatteryStateHandle();
        break;
    }

    if (obj1) {
        UAVTalkSendObject(uavTalkCon, obj1, 0, 0, 0);
    }

    if (obj2) {
        UAVTalkSendObject(uavTalkCon, obj2, 0, 0, 0);
    }
}

static int32_t transmitData(uint8_t *data, int32_t length)
{
    return PIOS_COM_SendBuffer(outputPort, data, length);
}

/**
 * @}
 * @}
 */
