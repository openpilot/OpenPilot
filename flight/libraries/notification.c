/**
 ******************************************************************************
 *
 * @file       notification.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      brief goes here. 
 *             --
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
#include "inc/notification.h"
#include <openpilot.h>
#include <pios_struct_helper.h>
#include <systemalarms.h>
#include <flightstatus.h>

#ifdef PIOS_LED_ALARM
#define ALARM_LED_ON()  PIOS_LED_On(PIOS_LED_ALARM)
#define ALARM_LED_OFF() PIOS_LED_Off(PIOS_LED_ALARM)
#else
#define ALARM_LED_ON()
#define ALARM_LED_OFF()
#endif

#ifdef PIOS_LED_HEARTBEAT
#define HEARTBEAT_LED_ON()  PIOS_LED_On(PIOS_LED_HEARTBEAT)
#define HEARTBEAT_LED_OFF() PIOS_LED_Off(PIOS_LED_HEARTBEAT)
#else
#define HEARTBEAT_LED_ON()
#define HEARTBEAT_LED_OFF()
#endif

#define ALARM_BLINK_COUNT(x) \
    (x == SYSTEMALARMS_ALARM_OK ? 0 : \
     x == SYSTEMALARMS_ALARM_WARNING ? 1 : \
     x == SYSTEMALARMS_ALARM_ERROR ? 2 : \
     x == SYSTEMALARMS_ALARM_CRITICAL ? 0 : 0)


#define BLINK_COUNT(x) \
    (x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED1 ? 2 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED2 ? 3 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED3 ? 4 : \
     x == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD ? 2 : \
     x == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEVARIO ? 2 : \
     x == FLIGHTSTATUS_FLIGHTMODE_VELOCITYCONTROL ? 2 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ? 3 : \
     x == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE ? 4 : \
     x == FLIGHTSTATUS_FLIGHTMODE_LAND ? 4 : \
     x == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER ? 3 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POI ? 3 : 1)

#define BLINK_RED(x) \
    (x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED1 ? false : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED2 ? false : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED3 ? false : \
     x == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD ? true : \
     x == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEVARIO ? true : \
     x == FLIGHTSTATUS_FLIGHTMODE_VELOCITYCONTROL ? true : \
     x == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ? true : \
     x == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE ? true : \
     x == FLIGHTSTATUS_FLIGHTMODE_LAND ? true : \
     x == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER ? true : \
     x == FLIGHTSTATUS_FLIGHTMODE_POI ? true : false)

// led notification handling
static volatile SystemAlarmsAlarmOptions currentAlarmLevel = SYSTEMALARMS_ALARM_OK;
static volatile FlightStatusData currentFlightStatus;
static volatile bool started = false;

void NotificationUpdateStatus(){
    started = true;
    // get values to be used for led handling
    FlightStatusGet((FlightStatusData *)&currentFlightStatus);
    currentAlarmLevel = AlarmsGetHighestSeverity();
}

void NotificationOnboardLedsRun(){
    static portTickType lastRunTimestamp;

    if (!started || (xTaskGetTickCount() - lastRunTimestamp) < (LED_BLINK_PERIOD_MS * portTICK_RATE_MS / 2)) {
        return;
    }
    lastRunTimestamp = xTaskGetTickCount();
    // the led will show various status information, subdivided in three phases
    // - Notification
    // - Alarm
    // - Flight status
    // they are shown using the above priority
    // a phase last exactly 8 cycles (so bit 1<<4 is used to determine if a phase end

    static enum {
        STATUS_NOTIFY,
        STATUS_ALARM,
        STATUS_FLIGHTMODE,
        STATUS_LENGHT
    } status;

    static uint8_t cycleCount;
    cycleCount++;
    // a blink last 2 cycles.
    static uint8_t blinkCount;
    blinkCount = (cycleCount & 0xF) >> 1;

    if (cycleCount & 0x08) {
        // add a short pause between each phase
        if (cycleCount > 0xA) {
            cycleCount = 0xFF;
            status     = (status + 1) % STATUS_LENGHT;
        }
        HEARTBEAT_LED_OFF();
        ALARM_LED_OFF();
        return;
    }

    if (status == STATUS_NOTIFY) {
        // Not implemented yet
        status++;
    }

    // Handles Alarm display
    if (status == STATUS_ALARM) {
#if defined(PIOS_LED_ALARM)
        if (currentAlarmLevel > SYSTEMALARMS_ALARM_OK) {
            if (currentAlarmLevel == SYSTEMALARMS_ALARM_CRITICAL) {
                // Slow blink
                ALARM_LED_OFF();
                if (cycleCount & 0x4) {
                    ALARM_LED_OFF();
                } else {
                    ALARM_LED_ON();
                }
            } else {
                if ((blinkCount < (ALARM_BLINK_COUNT(currentAlarmLevel))) &&
                    (cycleCount & 0x1)) {
                    ALARM_LED_ON();
                } else {
                    ALARM_LED_OFF();
                }
            }
        } else {
            status++;
        }
#else /* if defined(PIOS_LED_ALARM) */
        // no alarms, handle next phase
        status++;
        // #endif
#endif /* PIOS_LED_ALARM */
    }

    // **** Handles flightmode display
    if (status == STATUS_FLIGHTMODE) {
        uint8_t flightmode = currentFlightStatus.FlightMode;

        // Flash the heartbeat LED
#if defined(PIOS_LED_HEARTBEAT)

        if (currentFlightStatus.Armed == FLIGHTSTATUS_ARMED_DISARMED) {
            // Slow blink
            if (blinkCount < 3) {
                HEARTBEAT_LED_ON();
            } else {
                HEARTBEAT_LED_OFF();
            }
        } else {
            if ((blinkCount < BLINK_COUNT(flightmode)) &&
                (cycleCount & 0x1)) {
                // red led will be active active in last or last two (4 blinks case) blinks
                if (BLINK_RED(flightmode) &&
                    ((blinkCount == BLINK_COUNT(flightmode) - 1) ||
                     blinkCount > 1)) {
                    ALARM_LED_ON();
                }
                HEARTBEAT_LED_ON();
            } else {
                HEARTBEAT_LED_OFF();
                ALARM_LED_OFF();
            }
        }
    }
#endif /* PIOS_LED_HEARTBEAT */
}
