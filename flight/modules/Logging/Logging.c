/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup LoggingModule Logging Module
 * @brief Features for on board logging
 * @{
 *
 * @file       Logging.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Logging module, provides features for on board logging
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

#include "openpilot.h"
#include "debuglogsettings.h"
#include "debuglogcontrol.h"
#include "debuglogstatus.h"
#include "debuglogentry.h"
#include "flightstatus.h"

// private variables
static DebugLogSettingsData settings;
static DebugLogControlData control;
static DebugLogStatusData status;
static FlightStatusData flightstatus;
static DebugLogEntryData *entry; // would be better on stack but event dispatcher stack might be insufficient

// private functions
static void SettingsUpdatedCb(UAVObjEvent *ev);
static void ControlUpdatedCb(UAVObjEvent *ev);
static void StatusUpdatedCb(UAVObjEvent *ev);
static void FlightStatusUpdatedCb(UAVObjEvent *ev);

int32_t LoggingInitialize(void)
{
    DebugLogSettingsInitialize();
    DebugLogControlInitialize();
    DebugLogStatusInitialize();
    DebugLogEntryInitialize();
    FlightStatusInitialize();
    PIOS_DEBUGLOG_Initialize();
    entry = pios_malloc(sizeof(DebugLogEntryData));
    if (!entry) {
        return -1;
    }

    return 0;
}

int32_t LoggingStart(void)
{
    DebugLogSettingsConnectCallback(SettingsUpdatedCb);
    DebugLogControlConnectCallback(ControlUpdatedCb);
    FlightStatusConnectCallback(FlightStatusUpdatedCb);
    SettingsUpdatedCb(DebugLogSettingsHandle());

    UAVObjEvent ev = {
        .obj    = DebugLogSettingsHandle(),
        .instId = 0,
        .event  = EV_UPDATED_PERIODIC,
        .lowPriority = true,
    };
    EventPeriodicCallbackCreate(&ev, StatusUpdatedCb, 1000);
    // invoke a periodic dispatcher callback - the event struct is a dummy, it could be filled with anything!
    StatusUpdatedCb(&ev);

    return 0;
}
MODULE_INITCALL(LoggingInitialize, LoggingStart);

static void StatusUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    PIOS_DEBUGLOG_Info(&status.Flight, &status.Entry);
    DebugLogStatusSet(&status);
}

static void FlightStatusUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    FlightStatusGet(&flightstatus);
    if (settings.LoggingEnabled == DEBUGLOGSETTINGS_LOGGINGENABLED_ONLYWHENARMED) {
        if (flightstatus.Armed != FLIGHTSTATUS_ARMED_ARMED) {
            PIOS_DEBUGLOG_Printf("FlightStatus Disarmed: On board Logging disabled.");
            PIOS_DEBUGLOG_Enable(0);
        } else {
            PIOS_DEBUGLOG_Enable(1);
            PIOS_DEBUGLOG_Printf("FlightStatus Armed: On board logging enabled.");
        }
    }
}

static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    DebugLogSettingsGet(&settings);
    if (settings.LoggingEnabled == DEBUGLOGSETTINGS_LOGGINGENABLED_ALWAYS) {
        PIOS_DEBUGLOG_Enable(1);
        PIOS_DEBUGLOG_Printf("On board logging enabled.");
    } else if (settings.LoggingEnabled == DEBUGLOGSETTINGS_LOGGINGENABLED_DISABLED) {
        PIOS_DEBUGLOG_Printf("On board logging disabled.");
        PIOS_DEBUGLOG_Enable(0);
    } else {
        FlightStatusUpdatedCb(NULL);
    }
}

static void ControlUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    DebugLogControlGet(&control);
    if (control.Operation == DEBUGLOGCONTROL_OPERATION_RETRIEVE) {
        memset(entry, 0, sizeof(DebugLogEntryData));
        if (PIOS_DEBUGLOG_Read(entry, control.Flight, control.Entry) != 0) {
            // reading from log failed, mark as non existent in output
            entry->Flight = control.Flight;
            entry->Entry  = control.Entry;
            entry->Type   = DEBUGLOGENTRY_TYPE_EMPTY;
        }
        DebugLogEntrySet(entry);
    } else if (control.Operation == DEBUGLOGCONTROL_OPERATION_DELETEALL) {
        uint8_t armed;
        FlightStatusArmedGet(&armed);
        if (armed == FLIGHTSTATUS_ARMED_DISARMED) {
            PIOS_DEBUGLOG_DeleteAll();
        }
    }
    StatusUpdatedCb(ev);
}


/**
 * @}
 * @}
 */
