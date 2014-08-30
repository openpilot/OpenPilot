/**
 ******************************************************************************
 *
 * @file       sequences.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Notify module, sequences configuration.
 *
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
#ifndef SEQUENCES_H_
#define SEQUENCES_H_
#include <optypes.h>
#include <pios_notify.h>
#include <flightstatus.h>
#include <systemalarms.h>
#include <pios_helpers.h>

typedef enum {
    NOTIFY_SEQUENCE_ARMED_FM_MANUAL = 0,
    NOTIFY_SEQUENCE_ARMED_FM_STABILIZED1 = 1,
    NOTIFY_SEQUENCE_ARMED_FM_STABILIZED2 = 2,
    NOTIFY_SEQUENCE_ARMED_FM_STABILIZED3 = 3,
    NOTIFY_SEQUENCE_ARMED_FM_STABILIZED4 = 4,
    NOTIFY_SEQUENCE_ARMED_FM_STABILIZED5 = 5,
    NOTIFY_SEQUENCE_ARMED_FM_STABILIZED6 = 6,
    NOTIFY_SEQUENCE_ARMED_FM_AUTOTUNE    = 7,
    NOTIFY_SEQUENCE_ARMED_FM_GPS      = 8,
    NOTIFY_SEQUENCE_ARMED_FM_RTH      = 9,
    NOTIFY_SEQUENCE_ARMED_FM_LAND     = 10,
    NOTIFY_SEQUENCE_ARMED_FM_AUTO     = 11,
    NOTIFY_SEQUENCE_ALM_WARN_GPS      = 12,
    NOTIFY_SEQUENCE_ALM_ERROR_GPS     = 13,
    NOTIFY_SEQUENCE_ALM_WARN_BATTERY  = 14,
    NOTIFY_SEQUENCE_ALM_ERROR_BATTERY = 15,
    NOTIFY_SEQUENCE_ALM_MAG    = 16,
    NOTIFY_SEQUENCE_ALM_CONFIG = 17,
    NOTIFY_SEQUENCE_DISARMED   = 18,
} NotifySequences;

typedef struct {
    uint32_t timeBetweenNotifications;
    uint8_t  alarmIndex;
    uint8_t  warnNotification;
    uint8_t  errorNotification;
} AlarmDefinition_t;

// consts
const LedSequence_t notifications[] = {
    [NOTIFY_SEQUENCE_DISARMED] =             {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 1000,
                                             .time_on  = 1000,
                                             .color    = COLOR_BLUE,
                                             .repeats  = 1,
                                             },
                                             {
                                             .time_off = 1000,
                                             .time_on  = 1000,
                                             .color    = COLOR_LIME,
                                             .repeats  = 1,
                                             },
                                             {
                                             .time_off = 1000,
                                             .time_on  = 1000,
                                             .color    = COLOR_RED,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ARMED_FM_MANUAL] =      {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 900,
                                             .time_on  = 100,
                                             .color    = COLOR_YELLOW,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ARMED_FM_STABILIZED1] = {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 900,
                                             .time_on  = 100,
                                             .color    = COLOR_BLUE,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ARMED_FM_STABILIZED2] = {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 100,
                                             .time_on  = 100,
                                             .color    = COLOR_BLUE,
                                             .repeats  = 1,
                                             },
                                             {
                                             .time_off = 700,
                                             .time_on  = 100,
                                             .color    = COLOR_BLUE,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ARMED_FM_STABILIZED3] = {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 100,
                                             .time_on  = 100,
                                             .color    = COLOR_BLUE,
                                             .repeats  = 2,
                                             },
                                             {
                                             .time_off = 500,
                                             .time_on  = 100,
                                             .color    = COLOR_BLUE,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ARMED_FM_STABILIZED4] = {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 900,
                                             .time_on  = 100,
                                             .color    = COLOR_PURPLE,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ARMED_FM_STABILIZED5] = {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 100,
                                             .time_on  = 100,
                                             .color    = COLOR_PURPLE,
                                             .repeats  = 1,
                                             },
                                             {
                                             .time_off = 700,
                                             .time_on  = 100,
                                             .color    = COLOR_BLUE,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ARMED_FM_STABILIZED6] = {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 100,
                                             .time_on  = 100,
                                             .color    = COLOR_PURPLE,
                                             .repeats  = 1,
                                             },
                                             {
                                             .time_off = 100,
                                             .time_on  = 100,
                                             .color    = COLOR_PURPLE,
                                             .repeats  = 1,
                                             },
                                             {
                                             .time_off = 500,
                                             .time_on  = 100,
                                             .color    = COLOR_BLUE,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ARMED_FM_AUTOTUNE] =    {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 800,
                                             .time_on  = 200,
                                             .color    = COLOR_BLUE,
                                             .repeats  = 1,
                                             },
        },
    },

    [NOTIFY_SEQUENCE_ARMED_FM_GPS] =         {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 800,
                                             .time_on  = 200,
                                             .color    = COLOR_GREEN,
                                             .repeats  = 1,
                                             },
        },
    },

    [NOTIFY_SEQUENCE_ARMED_FM_RTH] =         {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 100,
                                             .time_on  = 200,
                                             .color    = COLOR_GREEN,
                                             .repeats  = 1,
                                             },
                                             {
                                             .time_off = 500,
                                             .time_on  = 200,
                                             .color    = COLOR_GREEN,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ARMED_FM_LAND] =        {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 100,
                                             .time_on  = 200,
                                             .color    = COLOR_GREEN,
                                             .repeats  = 1,
                                             },
                                             {
                                             .time_off = 100,
                                             .time_on  = 200,
                                             .color    = COLOR_BLUE,
                                             .repeats  = 1,
                                             },
                                             {
                                             .time_off = 100,
                                             .time_on  = 200,
                                             .color    = COLOR_GREEN,
                                             .repeats  = 1,
                                             },
                                             {
                                             .time_off = 100,
                                             .time_on  = 200,
                                             .color    = COLOR_BLUE,
                                             .repeats  = 1,
                                             },
                                             {
                                             .time_off = 100,
                                             .time_on  = 200,
                                             .color    = COLOR_GREEN,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ARMED_FM_AUTO] =        {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 100,
                                             .time_on  = 200,
                                             .color    = COLOR_GREEN,
                                             .repeats  = 2,
                                             },
                                             {
                                             .time_off = 500,
                                             .time_on  = 200,
                                             .color    = COLOR_GREEN,
                                             .repeats  = 1,
                                             },
        },
    },

    [NOTIFY_SEQUENCE_ALM_WARN_GPS] =         {
        .repeats = 2,
        .steps   =                           {
                                             {
                                             .time_off = 300,
                                             .time_on  = 300,
                                             .color    = COLOR_ORANGE,
                                             .repeats  = 2,
                                             },
                                             {
                                             .time_off = 300,
                                             .time_on  = 300,
                                             .color    = COLOR_YELLOW,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ALM_ERROR_GPS] =        {
        .repeats = 2,
        .steps   =                           {
                                             {
                                             .time_off = 300,
                                             .time_on  = 300,
                                             .color    = COLOR_RED,
                                             .repeats  = 2,
                                             },
                                             {
                                             .time_off = 300,
                                             .time_on  = 300,
                                             .color    = COLOR_YELLOW,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ALM_WARN_BATTERY] =     {
        .repeats = 1,
        .steps   =                           {
                                             {
                                             .time_off = 100,
                                             .time_on  = 100,
                                             .color    = COLOR_ORANGE,
                                             .repeats  = 10,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ALM_ERROR_BATTERY] =    {
        .repeats = 1,
        .steps   =                           {
                                             {
                                             .time_off = 100,
                                             .time_on  = 100,
                                             .color    = COLOR_RED,
                                             .repeats  = 10,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ALM_MAG] =              {
        .repeats = 1,
        .steps   =                           {
                                             {
                                             .time_off = 300,
                                             .time_on  = 300,
                                             .color    = COLOR_RED,
                                             .repeats  = 2,
                                             },
                                             {
                                             .time_off = 300,
                                             .time_on  = 300,
                                             .color    = COLOR_GREEN,
                                             .repeats  = 1,
                                             },
        },
    },
    [NOTIFY_SEQUENCE_ALM_CONFIG] =           {
        .repeats = -1,
        .steps   =                           {
                                             {
                                             .time_off = 500,
                                             .time_on  = 100,
                                             .color    = COLOR_RED,
                                             .repeats  = 1,
                                             },
        },
    },
};

const LedSequence_t *flightModeMap[] = {
    [FLIGHTSTATUS_FLIGHTMODE_MANUAL] = &notifications[NOTIFY_SEQUENCE_ARMED_FM_MANUAL],
    [FLIGHTSTATUS_FLIGHTMODE_STABILIZED1]       = &notifications[NOTIFY_SEQUENCE_ARMED_FM_STABILIZED1],
    [FLIGHTSTATUS_FLIGHTMODE_STABILIZED2]       = &notifications[NOTIFY_SEQUENCE_ARMED_FM_STABILIZED2],
    [FLIGHTSTATUS_FLIGHTMODE_STABILIZED3]       = &notifications[NOTIFY_SEQUENCE_ARMED_FM_STABILIZED3],
    [FLIGHTSTATUS_FLIGHTMODE_STABILIZED4]       = &notifications[NOTIFY_SEQUENCE_ARMED_FM_STABILIZED4],
    [FLIGHTSTATUS_FLIGHTMODE_STABILIZED5]       = &notifications[NOTIFY_SEQUENCE_ARMED_FM_STABILIZED5],
    [FLIGHTSTATUS_FLIGHTMODE_STABILIZED6]       = &notifications[NOTIFY_SEQUENCE_ARMED_FM_STABILIZED6],
    [FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER]       = &notifications[NOTIFY_SEQUENCE_ARMED_FM_AUTO],
    [FLIGHTSTATUS_FLIGHTMODE_AUTOTUNE]          = &notifications[NOTIFY_SEQUENCE_ARMED_FM_AUTOTUNE],
    [FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD]      = &notifications[NOTIFY_SEQUENCE_ARMED_FM_GPS],
    [FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIOFPV]  = &notifications[NOTIFY_SEQUENCE_ARMED_FM_GPS],
    [FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIOLOS]  = &notifications[NOTIFY_SEQUENCE_ARMED_FM_GPS],
    [FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIONSEW] = &notifications[NOTIFY_SEQUENCE_ARMED_FM_GPS],
    [FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE]      = &notifications[NOTIFY_SEQUENCE_ARMED_FM_RTH],
    [FLIGHTSTATUS_FLIGHTMODE_LAND] = &notifications[NOTIFY_SEQUENCE_ARMED_FM_LAND],
    [FLIGHTSTATUS_FLIGHTMODE_POI] = &notifications[NOTIFY_SEQUENCE_ARMED_FM_GPS],
    [FLIGHTSTATUS_FLIGHTMODE_AUTOCRUISE]        = &notifications[NOTIFY_SEQUENCE_ARMED_FM_GPS],
};

const AlarmDefinition_t alarmsMap[] = {
    {
        .timeBetweenNotifications = 10000,
        .alarmIndex = SYSTEMALARMS_ALARM_GPS,
        .warnNotification = NOTIFY_SEQUENCE_ALM_WARN_GPS,
        .errorNotification = NOTIFY_SEQUENCE_ALM_ERROR_GPS,
    },
    {
        .timeBetweenNotifications = 15000,
        .alarmIndex = SYSTEMALARMS_ALARM_MAGNETOMETER,
        .warnNotification = NOTIFY_SEQUENCE_ALM_MAG,
        .errorNotification = NOTIFY_SEQUENCE_ALM_MAG,
    },
    {
        .timeBetweenNotifications = 15000,
        .alarmIndex = SYSTEMALARMS_ALARM_BATTERY,
        .warnNotification = NOTIFY_SEQUENCE_ALM_WARN_BATTERY,
        .errorNotification = NOTIFY_SEQUENCE_ALM_ERROR_BATTERY,
    },
};
const uint8_t alarmsMapSize = NELEMENTS(alarmsMap);

#endif /* SEQUENCES_H_ */
