/**
 ******************************************************************************
 *
 * @file       notification.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      notification library.
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
#include <pios_notify.h>

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
    (x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED1 ? 1 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED2 ? 2 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED3 ? 3 : \
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
static volatile pios_notify_notification nextNotification = NOTIFY_NONE;

#ifdef PIOS_LED_ALARM
static bool handleAlarms(uint8_t cycleCount, uint8_t blinkCount);
#endif // PIOS_LED_ALARM
static bool handleNotifications(uint8_t cycleCount, pios_notify_notification runningNotification);
static void handleStatus(uint8_t cycleCount, uint8_t blinkCount);

void NotificationUpdateStatus()
{
    started = true;
    // get values to be used for led handling
    FlightStatusGet((FlightStatusData *)&currentFlightStatus);
    currentAlarmLevel = AlarmsGetHighestSeverity();
    if (nextNotification == NOTIFY_NONE) {
        nextNotification = PIOS_NOTIFY_GetActiveNotification(true);
    }
}

void NotificationOnboardLedsRun()
{
    static portTickType lastRunTimestamp;
    static uint8_t blinkCount; // number of blinks since phase start
    static uint8_t cycleCount; // cound the number of cycles (half of a blink)
    static pios_notify_notification runningNotification = NOTIFY_NONE;

    static enum {
        STATUS_NOTIFY,
        STATUS_ALARM,
        STATUS_FLIGHTMODE,
        STATUS_LENGHT
    } status;


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

    cycleCount++;

    // Notifications are "modal" to other statuses so they takes precedence
    if (status != STATUS_NOTIFY && nextNotification != NOTIFY_NONE) {
        // Force a notification status
        runningNotification = nextNotification;
        nextNotification    = NOTIFY_NONE;

        status     = STATUS_NOTIFY;
        cycleCount = 0; // instantly start a notify cycle
    }

    // check if a phase has just finished
    if (cycleCount & 0x08) {
        // add a short pause between each phase
        if (cycleCount > 0x8 + LED_PAUSE_BETWEEN_PHASES) {
            // ready to start a new phase
            cycleCount = 0x0;

            // Notification has been already shown, so clear the running one
            if (status == STATUS_NOTIFY) {
                runningNotification = NOTIFY_NONE;
            }
            status = (status + 1) % STATUS_LENGHT;
        } else {
            HEARTBEAT_LED_OFF();
            ALARM_LED_OFF();
            return;
        }
    }

    // a blink last 2 cycles.
    blinkCount = (cycleCount & 0xF) >> 1;

    if (status == STATUS_NOTIFY) {
        if (!handleNotifications(cycleCount, runningNotification)) {
            status++;
        }
    }

    // Handles Alarm display
    if (status == STATUS_ALARM) {
#ifdef PIOS_LED_ALARM
        if (!handleAlarms(cycleCount, blinkCount)) {
            status++;
        }
#else
        status++;
#endif // PIOS_LED_ALARM
    }
    // **** Handles flightmode display
    if (status == STATUS_FLIGHTMODE) {
        handleStatus(cycleCount, blinkCount);
    }
}

#if defined(PIOS_LED_ALARM)
static bool handleAlarms(uint8_t cycleCount, uint8_t blinkCount)
{
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
        return true;
    } else {
        return false;
    }
}
#endif /* PIOS_LED_ALARM */


static bool handleNotifications(uint8_t cycleCount, pios_notify_notification runningNotification)
{
    if (runningNotification == NOTIFY_NONE) {
        return false;
    }
    if (cycleCount & 0x1) {
        ALARM_LED_OFF();
        HEARTBEAT_LED_ON();
    } else {
        ALARM_LED_ON();
        HEARTBEAT_LED_OFF();
    }
    return true;
}

static void handleStatus(uint8_t cycleCount, uint8_t blinkCount)
{
    // Flash the heartbeat LED
#if defined(PIOS_LED_HEARTBEAT)
    uint8_t flightmode = currentFlightStatus.FlightMode;
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
#endif /* PIOS_LED_HEARTBEAT */
}
