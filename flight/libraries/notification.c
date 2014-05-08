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

#define BLINK_R_ALARM_PATTERN(x) \
    (x == SYSTEMALARMS_ALARM_OK ? 0 : \
     x == SYSTEMALARMS_ALARM_WARNING ? 0b0000000001000000 : \
     x == SYSTEMALARMS_ALARM_ERROR ? 0b0000001000100000 : \
     x == SYSTEMALARMS_ALARM_CRITICAL ? 0b0111111111111110 : 0)
#define BLINK_B_ALARM_PATTERN(x) \
    (x == SYSTEMALARMS_ALARM_OK ? 0 : \
     x == SYSTEMALARMS_ALARM_WARNING ? 0 : \
     x == SYSTEMALARMS_ALARM_ERROR ? 0 : \
     x == SYSTEMALARMS_ALARM_CRITICAL ? 0 : 0)


#define BLINK_B_FM_ARMED_PATTERN(x) \
    (x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED1 ? 0b0000000000000001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED2 ? 0b0000000000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED3 ? 0b0000010000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED4 ? 0b0000000000000001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED5 ? 0b0000000000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED6 ? 0b0000010000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ? 0b0000010000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE ? 0b0001000100010001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_LAND ? 0b0001000100010001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER ? 0b0000010000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POI ? 0b0000010000100001 : 0b0000000000000001)

#define BLINK_R_FM_ARMED_PATTERN(x) \
    (x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED1 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED2 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED3 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED4 ? 0b0000000000000001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED5 ? 0b0000000000000001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED6 ? 0b0000000000000001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ? 0b0000010000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE ? 0b0001000100000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_LAND ? 0b0001000100000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER ? 0b0000010000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POI ? 0b0000010000000000 : 0b0000010000000000)

#define BLINK_B_FM_DISARMED_PATTERN(x) \
    (x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED1 ? 0b0000000000000011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED2 ? 0b0000000001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED3 ? 0b0000110001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED4 ? 0b0000000000000011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED5 ? 0b0000000001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED6 ? 0b0000110001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ? 0b0000110001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE ? 0b0011001100110011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_LAND ? 0b0011001100110011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER ? 0b0000110001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POI ? 0b0000110001100011 : 0b0000000000000011)

#define BLINK_R_FM_DISARMED_PATTERN(x) \
    (x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED1 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED2 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED3 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED4 ? 0b0000000000000011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED5 ? 0b0000000000000011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED6 ? 0b0000000000000011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ? 0b0000110000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE ? 0b0011001100000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_LAND ? 0b0011001100000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER ? 0b0000110000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POI ? 0b0000110000000000 : 0b0000110000000000)

#define BLINK_B_HEARTBEAT_PATTERN 0b0001111111111111
#define BLINK_R_HEARTBEAT_PATTERN 0

#define BLINK_B_NOTIFY_PATTERN(x) \
    (x == NOTIFY_NONE ? 0 : \
     x == NOTIFY_OK ? 0b0000100100111111 : \
     x == NOTIFY_NOK ? 0b0000000000111111 : \
     x == NOTIFY_DRAW_ATTENTION ? 0b0101010101010101 : 0b0101010101010101)

#define BLINK_R_NOTIFY_PATTERN(x) \
    (x == NOTIFY_NONE ? 0 : \
     x == NOTIFY_OK ? 0b0000000000001111 : \
     x == NOTIFY_NOK ? 0b0011000011001111 : \
     x == NOTIFY_DRAW_ATTENTION ? 0b1010101010101010 : 0b1010101010101010)

// led notification handling
static volatile SystemAlarmsAlarmOptions currentAlarmLevel = SYSTEMALARMS_ALARM_OK;
static volatile FlightStatusData currentFlightStatus;
static volatile bool started = false;
static volatile pios_notify_notification nextNotification = NOTIFY_NONE;

#ifdef PIOS_LED_ALARM
static bool handleAlarms(uint16_t *r_pattern, uint16_t *b_pattern);
#endif // PIOS_LED_ALARM
static bool handleNotifications(pios_notify_notification runningNotification, uint16_t *r_pattern, uint16_t *b_pattern);
static void handleFlightMode(uint16_t *r_pattern, uint16_t *b_pattern);
static void handleHeartbeat(uint16_t *r_pattern, uint16_t *b_pattern);
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
    static uint16_t r_pattern;
    static uint16_t b_pattern;
    static uint8_t cycleCount; // count the number of cycles
    static uint8_t lastFlightMode   = -1;
    static bool forceShowFlightMode = false;
    static pios_notify_notification runningNotification = NOTIFY_NONE;
    static enum {
        STATUS_NOTIFY,
        STATUS_ALARM,
        STATUS_FLIGHTMODE, // flightMode/HeartBeat
        STATUS_LENGHT
    } status;

    if (!started || (xTaskGetTickCount() - lastRunTimestamp) < (LED_BLINK_PERIOD_MS * portTICK_RATE_MS / 4)) {
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
        // read next notification to show
        runningNotification = nextNotification;
        nextNotification    = NOTIFY_NONE;
        // Force a notification status
        status     = STATUS_NOTIFY;
        cycleCount = 0; // instantly start a notify cycle
    } else {
        if (lastFlightMode != currentFlightStatus.FlightMode) {
            status         = STATUS_FLIGHTMODE;
            lastFlightMode = currentFlightStatus.FlightMode;
            cycleCount     = 0; // instantly start a flightMode cycle
            forceShowFlightMode = true;
        }
    }

    // check if a phase has just finished
    if (cycleCount & 0x10) {
        HEARTBEAT_LED_OFF();
        ALARM_LED_OFF();
        cycleCount = 0x0;
        forceShowFlightMode = false;
        // Notification has been just shown, cleanup
        if (status == STATUS_NOTIFY) {
            runningNotification = NOTIFY_NONE;
        }
        status = (status + 1) % STATUS_LENGHT;
    }

    if (status == STATUS_NOTIFY) {
        if (!cycleCount && !handleNotifications(runningNotification, &r_pattern, &b_pattern)) {
            // no notifications, advance
            status++;
        }
    }

    // Handles Alarm display
    if (status == STATUS_ALARM) {
#ifdef PIOS_LED_ALARM
        if (!cycleCount && !handleAlarms(&r_pattern, &b_pattern)) {
            // no alarms, advance
            status++;
        }
#else
        // no alarms leds, advance
        status++;
#endif // PIOS_LED_ALARM
    }

    // **** Handles flightmode display
    if (status == STATUS_FLIGHTMODE && !cycleCount) {
        if (forceShowFlightMode || currentFlightStatus.Armed != FLIGHTSTATUS_ARMED_DISARMED) {
            handleFlightMode(&r_pattern, &b_pattern);
        } else {
            handleHeartbeat(&r_pattern, &b_pattern);
        }
    }

    // led output
    if (b_pattern & 0x1) {
        HEARTBEAT_LED_ON();
    } else {
        HEARTBEAT_LED_OFF();
    }
    if (r_pattern & 0x1) {
        ALARM_LED_ON();
    } else {
        ALARM_LED_OFF();
    }
    r_pattern >>= 1;
    b_pattern >>= 1;
}

#if defined(PIOS_LED_ALARM)
static bool handleAlarms(uint16_t *r_pattern, uint16_t *b_pattern)
{
    if (currentAlarmLevel == SYSTEMALARMS_ALARM_OK) {
        return false;
    }
    *b_pattern = BLINK_B_ALARM_PATTERN(currentAlarmLevel);
    *r_pattern = BLINK_R_ALARM_PATTERN(currentAlarmLevel);
    return true;
}
#endif /* PIOS_LED_ALARM */


static bool handleNotifications(pios_notify_notification runningNotification, uint16_t *r_pattern, uint16_t *b_pattern)
{
    if (runningNotification == NOTIFY_NONE) {
        return false;
    }
    *b_pattern = BLINK_B_NOTIFY_PATTERN(runningNotification);
    *r_pattern = BLINK_R_NOTIFY_PATTERN(runningNotification);
    return true;
}

static void handleFlightMode(uint16_t *r_pattern, uint16_t *b_pattern)
{
    // Flash the heartbeat LED
    uint8_t flightmode = currentFlightStatus.FlightMode;

    if (currentFlightStatus.Armed == FLIGHTSTATUS_ARMED_DISARMED) {
        *b_pattern = BLINK_B_FM_DISARMED_PATTERN(flightmode);
        *r_pattern = BLINK_R_FM_DISARMED_PATTERN(flightmode);
    } else {
        *b_pattern = BLINK_B_FM_ARMED_PATTERN(flightmode);
        *r_pattern = BLINK_R_FM_ARMED_PATTERN(flightmode);
    }
}

static void handleHeartbeat(uint16_t *r_pattern, uint16_t *b_pattern)
{
    // Flash the heartbeat LED
    *b_pattern = BLINK_B_HEARTBEAT_PATTERN;
    *r_pattern = BLINK_R_HEARTBEAT_PATTERN;
}
