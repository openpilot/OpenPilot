/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup osdinputModule osdinput Module
 * @brief Process osdinput information
 * @{
 *
 * @file       osdinput.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      osdinput module, handles osdinput stream
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

// ****************

#include <openpilot.h>

#include "osdinput.h"

#include "attitudeactual.h"
#include "taskinfo.h"
#include "flightstatus.h"

#include "fifo_buffer.h"

// ****************
// Private functions

static void osdinputTask(void *parameters);

// ****************
// Private constants

#define STACK_SIZE_BYTES            1024
#define TASK_PRIORITY                   (tskIDLE_PRIORITY + 4)
#define MAX_PACKET_LENGTH 33
// ****************
// Private variables

static uint32_t oposdPort;

static xTaskHandle osdinputTaskHandle;

static char* oposd_rx_buffer;
t_fifo_buffer rx;

enum osd_pkt_type
{
    OSD_PKT_TYPE_MISC = 0, OSD_PKT_TYPE_NAV = 1, OSD_PKT_TYPE_MAINT = 2, OSD_PKT_TYPE_ATT = 3, OSD_PKT_TYPE_MODE = 4,
};

// ****************
/**
 * Initialise the osdinput module
 * \return -1 if initialisation failed
 * \return 0 on success
 */

int32_t osdinputStart(void)
{
    // Start osdinput task
    xTaskCreate(osdinputTask, (signed char *) "OSDINPUT", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &osdinputTaskHandle);

    return 0;
}
/**
 * Initialise the gps module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t osdinputInitialize(void)
{
    AttitudeActualInitialize();
    FlightStatusInitialize();
    // Initialize quaternion
    AttitudeActualData attitude;
    AttitudeActualGet(&attitude);
    attitude.q1 = 1;
    attitude.q2 = 0;
    attitude.q3 = 0;
    attitude.q4 = 0;
    attitude.Roll = 0;
    attitude.Pitch = 0;
    attitude.Yaw = 0;
    AttitudeActualSet(&attitude);

    oposdPort = PIOS_COM_OSD;

    oposd_rx_buffer = pvPortMalloc(MAX_PACKET_LENGTH);
    PIOS_Assert(oposd_rx_buffer);

    return 0;
}
MODULE_INITCALL( osdinputInitialize, osdinputStart)

// ****************
/**
 * Main osdinput task. It does not return.
 */

static void osdinputTask(__attribute__((unused)) void *parameters)
{
    portTickType xDelay = 100 / portTICK_RATE_MS;
    portTickType lastSysTime;
    lastSysTime = xTaskGetTickCount();

    uint8_t rx_count = 0;
    bool start_flag = false;
    int32_t osdRxOverflow = 0;
    uint8_t c = 0xAA;
    // Loop forever
    while (1) {
        // This blocks the task until there is something on the buffer
        while (PIOS_COM_ReceiveBuffer(oposdPort, &c, 1, xDelay) > 0) {

            // detect start while acquiring stream
            if (!start_flag && ((c == 0xCB) || (c == 0x34))) {
                start_flag = true;
                rx_count = 0;
            } else if (!start_flag) {
                continue;
            }

            if (rx_count >= 11) {
                // Flush the buffer and note the overflow event.
                osdRxOverflow++;
                start_flag = false;
                rx_count = 0;
            } else {
                oposd_rx_buffer[rx_count] = c;
                rx_count++;
            }
            if (rx_count == 11) {
                if (oposd_rx_buffer[1] == OSD_PKT_TYPE_ATT) {
                    AttitudeActualData attitude;
                    AttitudeActualGet(&attitude);
                    attitude.q1 = 1;
                    attitude.q2 = 0;
                    attitude.q3 = 0;
                    attitude.q4 = 0;
                    attitude.Roll = (float) ((int16_t)(oposd_rx_buffer[3] | oposd_rx_buffer[4] << 8)) / 10.0f;
                    attitude.Pitch = (float) ((int16_t)(oposd_rx_buffer[5] | oposd_rx_buffer[6] << 8)) / 10.0f;
                    attitude.Yaw = (float) ((int16_t)(oposd_rx_buffer[7] | oposd_rx_buffer[8] << 8)) / 10.0f;
                    AttitudeActualSet(&attitude);
                } else if (oposd_rx_buffer[1] == OSD_PKT_TYPE_MODE) {
                    FlightStatusData status;
                    FlightStatusGet(&status);
                    status.Armed = oposd_rx_buffer[8];
                    status.FlightMode = oposd_rx_buffer[3];
                    FlightStatusSet(&status);
                }
                //frame completed
                start_flag = false;
                rx_count = 0;
            }
        }
        vTaskDelayUntil(&lastSysTime, 50 / portTICK_RATE_MS);
    }
}

// ****************

/**
 * @}
 * @}
 */
