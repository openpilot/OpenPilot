/**
 ******************************************************************************
 * @addtogroup OpenPilot System OpenPilot System
 * @{
 * @addtogroup OpenPilot Libraries OpenPilot System Libraries
 * @{
 * @file       sanitycheck.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Utilities to validate a flight configuration
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
#include <pios_board_info.h>

// Private includes
#include "inc/sanitycheck.h"

// UAVOs
#include <manualcontrolsettings.h>
#include <flightmodesettings.h>
#include <systemsettings.h>
#include <systemalarms.h>
#include <taskinfo.h>

/****************************
* Current checks:
* 1. If a flight mode switch allows autotune and autotune module not running
* 2. If airframe is a multirotor and either manual is available or a stabilization mode uses "none"
****************************/

// ! Check a stabilization mode switch position for safety
static int32_t check_stabilization_settings(int index, bool multirotor);

/**
 * Run a preflight check over the hardware configuration
 * and currently active modules
 */
int32_t configuration_check()
{
    int32_t severity = SYSTEMALARMS_ALARM_OK;
    SystemAlarmsExtendedAlarmStatusOptions alarmstatus = SYSTEMALARMS_EXTENDEDALARMSTATUS_NONE;
    uint8_t alarmsubstatus = 0;
    // Get board type
    const struct pios_board_info *bdinfo = &pios_board_info_blob;
    bool coptercontrol     = bdinfo->board_type == 0x04;

    // Classify airframe type
    bool multirotor = true;
    uint8_t airframe_type;

    SystemSettingsAirframeTypeGet(&airframe_type);
    switch (airframe_type) {
    case SYSTEMSETTINGS_AIRFRAMETYPE_QUADX:
    case SYSTEMSETTINGS_AIRFRAMETYPE_QUADP:
    case SYSTEMSETTINGS_AIRFRAMETYPE_HEXA:
    case SYSTEMSETTINGS_AIRFRAMETYPE_OCTO:
    case SYSTEMSETTINGS_AIRFRAMETYPE_HEXAX:
    case SYSTEMSETTINGS_AIRFRAMETYPE_OCTOV:
    case SYSTEMSETTINGS_AIRFRAMETYPE_OCTOCOAXP:
    case SYSTEMSETTINGS_AIRFRAMETYPE_HEXACOAX:
    case SYSTEMSETTINGS_AIRFRAMETYPE_TRI:
    case SYSTEMSETTINGS_AIRFRAMETYPE_OCTOCOAXX:
        multirotor = true;
        break;
    default:
        multirotor = false;
    }

    // For each available flight mode position sanity check the available
    // modes
    uint8_t num_modes;
    uint8_t modes[FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_NUMELEM];
    ManualControlSettingsFlightModeNumberGet(&num_modes);
    FlightModeSettingsFlightModePositionGet(modes);

    for (uint32_t i = 0; i < num_modes; i++) {
        switch (modes[i]) {
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_MANUAL:
            if (multirotor) {
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            }
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED1:
            severity = (severity == SYSTEMALARMS_ALARM_OK) ? check_stabilization_settings(1, multirotor) : severity;
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED2:
            severity = (severity == SYSTEMALARMS_ALARM_OK) ? check_stabilization_settings(2, multirotor) : severity;
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED3:
            severity = (severity == SYSTEMALARMS_ALARM_OK) ? check_stabilization_settings(3, multirotor) : severity;
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_AUTOTUNE:
            if (!PIOS_TASK_MONITOR_IsRunning(TASKINFO_RUNNING_AUTOTUNE)) {
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            }
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_ALTITUDEHOLD:
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_ALTITUDEVARIO:
            if (coptercontrol) {
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            }
            // TODO: put check equivalent to TASK_MONITOR_IsRunning
            // here as soon as available for delayed callbacks
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_VELOCITYCONTROL:
            if (coptercontrol) {
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            } else if (!PIOS_TASK_MONITOR_IsRunning(TASKINFO_RUNNING_PATHFOLLOWER)) { // Revo supports altitude hold
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            }
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_POSITIONHOLD:
            if (coptercontrol) {
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            } else if (!PIOS_TASK_MONITOR_IsRunning(TASKINFO_RUNNING_PATHFOLLOWER)) {
                // Revo supports Position Hold
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            }
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_LAND:
            if (coptercontrol) {
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            } else if (!PIOS_TASK_MONITOR_IsRunning(TASKINFO_RUNNING_PATHFOLLOWER)) {
                // Revo supports AutoLand Mode
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            }
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_POI:
            if (coptercontrol) {
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            } else if (!PIOS_TASK_MONITOR_IsRunning(TASKINFO_RUNNING_PATHFOLLOWER)) {
                // Revo supports POI Mode
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            }
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_PATHPLANNER:
            if (coptercontrol) {
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            } else {
                // Revo supports PathPlanner and that must be OK or we are not sane
                // PathPlan alarm is uninitialized if not running
                // PathPlan alarm is warning or error if the flightplan is invalid
                SystemAlarmsAlarmData alarms;
                SystemAlarmsAlarmGet(&alarms);
                if (alarms.PathPlan != SYSTEMALARMS_ALARM_OK) {
                    severity = SYSTEMALARMS_ALARM_CRITICAL;
                }
            }
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_RETURNTOBASE:
            if (coptercontrol) {
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            } else if (!PIOS_TASK_MONITOR_IsRunning(TASKINFO_RUNNING_PATHFOLLOWER)) {
                // Revo supports ReturnToBase
                severity = SYSTEMALARMS_ALARM_CRITICAL;
            }
            break;
        default:
            // Uncovered modes are automatically an error
            severity = SYSTEMALARMS_ALARM_CRITICAL;
        }
        // mark the first encountered erroneous setting in status and substatus
        if ((severity != SYSTEMALARMS_ALARM_OK) && (alarmstatus == SYSTEMALARMS_EXTENDEDALARMSTATUS_NONE)) {
            alarmstatus    = SYSTEMALARMS_EXTENDEDALARMSTATUS_FLIGHTMODE;
            alarmsubstatus = i;
        }
    }

    // TODO: Check on a multirotor no axis supports "None"
    if (severity != SYSTEMALARMS_ALARM_OK) {
        ExtendedAlarmsSet(SYSTEMALARMS_ALARM_SYSTEMCONFIGURATION, severity, alarmstatus, alarmsubstatus);
    } else {
        AlarmsClear(SYSTEMALARMS_ALARM_SYSTEMCONFIGURATION);
    }

    return 0;
}

/**
 * Checks the stabiliation settings for a paritcular mode and makes
 * sure it is appropriate for the airframe
 * @param[in] index Which stabilization mode to check
 * @returns SYSTEMALARMS_ALARM_OK or SYSTEMALARMS_ALARM_CRITICAL
 */
static int32_t check_stabilization_settings(int index, bool multirotor)
{
    // Make sure the modes have identical sizes
    if (FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM != FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_NUMELEM ||
        FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM != FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_NUMELEM) {
        return SYSTEMALARMS_ALARM_CRITICAL;
    }

    uint8_t modes[FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM];

    // Get the different axis modes for this switch position
    switch (index) {
    case 1:
        FlightModeSettingsStabilization1SettingsArrayGet(modes);
        break;
    case 2:
        FlightModeSettingsStabilization2SettingsArrayGet(modes);
        break;
    case 3:
        FlightModeSettingsStabilization3SettingsArrayGet(modes);
        break;
    default:
        return SYSTEMALARMS_ALARM_CRITICAL;
    }

    // For multirotors verify that nothing is set to "none"
    if (multirotor) {
        for (uint32_t i = 0; i < NELEMENTS(modes); i++) {
            if (modes[i] == FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NONE) {
                return SYSTEMALARMS_ALARM_CRITICAL;
            }
        }
    }

    // Warning: This assumes that certain conditions in the XML file are met.  That
    // FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NONE has the same numeric value for each channel
    // and is the same for STABILIZATIONDESIRED_STABILIZATIONMODE_NONE

    return SYSTEMALARMS_ALARM_OK;
}
