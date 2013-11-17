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
#include "debuglogentry.h"

// private variables
static DebugLogSettingsData settings;
static DebugLogControlData control;
static DebugLogEntryData *entry; // would be better on stack but event dispatcher stack might be insufficient

// private functions
static void SettingsUpdatedCb(UAVObjEvent *ev);
static void ControlUpdatedCb(UAVObjEvent *ev);

int32_t LoggingInitialize(void)
{
    DebugLogSettingsInitialize();
    DebugLogControlInitialize();
    DebugLogEntryInitialize();
    PIOS_DEBUGLOG_Initialize();
    entry = pvPortMalloc(sizeof(DebugLogEntryData));
    if (!entry) {
        return -1;
    }

    return 0;
}

int32_t LoggingStart(void)
{
    DebugLogSettingsConnectCallback(SettingsUpdatedCb);
    DebugLogControlConnectCallback(ControlUpdatedCb);
    SettingsUpdatedCb(DebugLogSettingsHandle());
    return 0;
}
MODULE_INITCALL(LoggingInitialize, LoggingStart);


static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    DebugLogSettingsGet(&settings);
    PIOS_DEBUGLOG_Enable(settings.LoggingEnabled);
    PIOS_DEBUGLOG_Printf("Logging enabled");
}

static void ControlUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    static bool ignore = 0; // this little hack allows us to set our own uavobject in the callback

    if (ignore) {
        ignore = 0;
        return;
    }

    DebugLogControlGet(&control);
    if (PIOS_DEBUGLOG_Read(entry, control.Flight, control.Entry) != 0) {
        // reading from log failed, mark as non existent in output
        memset(entry, 0, sizeof(DebugLogEntryData));
        entry->Flight = control.Flight;
        entry->Entry  = control.Entry;
        entry->Type   = DEBUGLOGENTRY_TYPE_EMPTY;
    }
    PIOS_DEBUGLOG_Info(&control.Flight, &control.Entry);

    ignore = 1; // set ignore flag before setting object - creates loop otherwise!!!
    DebugLogEntrySet(entry);
    DebugLogControlSet(&control);
}


/**
 * @}
 * @}
 */
