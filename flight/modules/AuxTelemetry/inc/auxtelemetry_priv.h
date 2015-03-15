/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup AuxTelemetryModule AuxTelemetry Module
 * @brief Auxiliary telemetry module
 * Exports basic telemetry data using a selectable serial protocol
 * to external devices such as on screen displays.
 * @{
 *
 * @file       auxtelemetry_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Auxiliary telemetry module, exports basic telemetry.
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

#ifndef AUXTELEMETRY_PRIV_H
#define AUXTELEMETRY_PRIV_H

#include <attitudestate.h>
#include <flightstatus.h>
#include <systemalarms.h>
#include <gpspositionsensor.h>
#include <gpsvelocitysensor.h>
#include <flightbatterystate.h>
#include <manualcontrolcommand.h>
#include <auxtelemetrysettings.h>
#include <pios_com.h>

typedef struct {
    void (*initialize)(uint32_t comPort);
    void (*updateData)(AuxTelemetrySettingsUpdateIntervalsElem data);
} protocolHandler_t;

#endif // AUXTELEMETRY_PRIV_H

/**
 * @}
 * @}
 */
