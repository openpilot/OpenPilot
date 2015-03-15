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
 * @file       auxtelemetry.c
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

#include <openpilot.h>
#include <pios_com.h>

#include "auxtelemetrysettings.h"
#include "hwsettings.h"

#include "auxtelemetry.h"
#include "auxtelemetry_priv.h"

#include "taskinfo.h"

// Private constants
#define STACK_SIZE_BYTES 512

#define TASK_PRIORITY    (tskIDLE_PRIORITY + 2)

// Private types

// Private variables
static uint32_t comPort = 0;
static xTaskHandle taskHandle;
static AuxTelemetryProtocolHandler *activeProtocolHandler = NULL;
static uint16_t updatePeriod;
static uint8_t intervalCounts[AUXTELEMETRYSETTINGS_UPDATEINTERVALS_NUMELEM];
static uint8_t updateIntervals[AUXTELEMETRYSETTINGS_UPDATEINTERVALS_NUMELEM];

// Private functions
static void telemetryTask(void *parameters);
static void updateSettings(UAVObjEvent *ev);

/**
 * Start the auxiliary telemetry module
 * \return -1 if failed
 * \return 0 on success
 */
int32_t AuxTelemetryStart(void)
{
    // Start task, if valid configuration (port and protocol)
    if (comPort && activeProtocolHandler) {
        xTaskCreate(telemetryTask, "SecondTel", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &taskHandle);
        PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_AUXTELEMETRY, taskHandle);
    }
    return 0;
}

/**
 * Initialise the auxiliary telemetry module
 * \return -1 if failed
 * \return 0 on success
 */
int32_t AuxTelemetryInitialize(void)
{
    uint8_t proto;

    /* Initialize the settings object */
    AuxTelemetrySettingsInitialize();
    AuxTelemetrySettingsConnectCallback(updateSettings);

    /* Serial port and protocol are only set at startup. */
    comPort = PIOS_COM_AUXTELEM;

    AuxTelemetrySettingsOutputProtocolGet(&proto);

    switch ((AuxTelemetrySettingsOutputProtocolOptions)proto) {
    case AUXTELEMETRYSETTINGS_OUTPUTPROTOCOL_UAVTALK:
        activeProtocolHandler = &uavtalkProtocolHandler;
        break;
    default:
        // No handler, module will be disabled
        break;
    }

    /* If there is a valid basic configuration (port and protocol), initialize */
    if (comPort && activeProtocolHandler) {
        updateSettings(0);
        activeProtocolHandler->initialize(comPort);
    }

    return 0;
}

MODULE_INITCALL(AuxTelemetryInitialize, AuxTelemetryStart);

/**
 * Main task. Handles the scheduling of updates and calls
 * the protocol handler.
 */
static void telemetryTask(__attribute__((unused)) void *parameters)
{
    uint8_t i;
    portTickType lastSysTime = xTaskGetTickCount();

    // Loop forever
    while (1) {
        vTaskDelayUntil(&lastSysTime, updatePeriod / portTICK_RATE_MS);

        /* Update interval counters */
        for (i = 0; i < AUXTELEMETRYSETTINGS_UPDATEINTERVALS_NUMELEM; i++) {
            if (updateIntervals[i]) {
                intervalCounts[i]++;
                if (intervalCounts[i] >= updateIntervals[i]) {
                    intervalCounts[i] = 0;
                    activeProtocolHandler->updateData(i);
                }
            }
        }
        /* Let protocol handler do background processing */
        if (activeProtocolHandler->periodTick) {
            activeProtocolHandler->periodTick();
        }
    }
}

/**
 * Update the auxiliary telemetry settings, called on startup and on change.
 */
static void updateSettings(__attribute__((unused)) UAVObjEvent *ev)
{
    /* Fetch and apply settings if there is a valid basic configuration (port and protocol) */
    if (comPort && activeProtocolHandler) {
        uint8_t speed;
        // Retrieve settings
        AuxTelemetrySettingsOutputSpeedGet(&speed);
        AuxTelemetrySettingsUpdatePeriodGet(&updatePeriod);
        AuxTelemetrySettingsUpdateIntervalsArrayGet(updateIntervals);

        // Set port speed
        switch (speed) {
        case AUXTELEMETRYSETTINGS_OUTPUTSPEED_2400:
            PIOS_COM_ChangeBaud(comPort, 2400);
            break;
        case AUXTELEMETRYSETTINGS_OUTPUTSPEED_4800:
            PIOS_COM_ChangeBaud(comPort, 4800);
            break;
        case AUXTELEMETRYSETTINGS_OUTPUTSPEED_9600:
            PIOS_COM_ChangeBaud(comPort, 9600);
            break;
        case AUXTELEMETRYSETTINGS_OUTPUTSPEED_19200:
            PIOS_COM_ChangeBaud(comPort, 19200);
            break;
        case AUXTELEMETRYSETTINGS_OUTPUTSPEED_38400:
            PIOS_COM_ChangeBaud(comPort, 38400);
            break;
        case AUXTELEMETRYSETTINGS_OUTPUTSPEED_57600:
            PIOS_COM_ChangeBaud(comPort, 57600);
            break;
        case AUXTELEMETRYSETTINGS_OUTPUTSPEED_115200:
            PIOS_COM_ChangeBaud(comPort, 115200);
            break;
        }
    }
}

/**
 * @}
 * @}
 */
