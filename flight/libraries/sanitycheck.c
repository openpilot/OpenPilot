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
#include <revosettings.h>
#include <positionstate.h>
#include <taskinfo.h>

// a number of useful macros
#define ADDSEVERITY(check) severity = (severity != SYSTEMALARMS_ALARM_OK ? severity : ((check) ? SYSTEMALARMS_ALARM_OK : SYSTEMALARMS_ALARM_ERROR))


/****************************
* Current checks:
* 1. If a flight mode switch allows autotune and autotune module not running
* 2. If airframe is a multirotor and either manual is available or a stabilization mode uses "none"
****************************/

// ! Check a stabilization mode switch position for safety
static bool check_stabilization_settings(int index, bool multirotor, bool coptercontrol);

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

    // Classify navigation capability
#ifdef REVOLUTION
    RevoSettingsInitialize();
    uint8_t revoFusion;
    RevoSettingsFusionAlgorithmGet(&revoFusion);
    bool navCapableFusion;
    switch (revoFusion) {
    case REVOSETTINGS_FUSIONALGORITHM_COMPLEMENTARYMAGGPSOUTDOOR:
    case REVOSETTINGS_FUSIONALGORITHM_INS13GPSOUTDOOR:
        navCapableFusion = true;
        break;
    default:
        navCapableFusion = false;
        // check for hitl.  hitl allows to feed position and velocity state via
        // telemetry, this makes nav possible even with an unsuited algorithm
        if (PositionStateHandle()) {
            if (PositionStateReadOnly()) {
                navCapableFusion = true;
            }
        }
    }
#else
    const bool navCapableFusion = false;
#endif /* ifdef REVOLUTION */


    // Classify airframe type
    bool multirotor;
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
            ADDSEVERITY(!multirotor);
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED1:
            ADDSEVERITY(check_stabilization_settings(1, multirotor, coptercontrol));
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED2:
            ADDSEVERITY(check_stabilization_settings(2, multirotor, coptercontrol));
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED3:
            ADDSEVERITY(check_stabilization_settings(3, multirotor, coptercontrol));
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED4:
            ADDSEVERITY(check_stabilization_settings(4, multirotor, coptercontrol));
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED5:
            ADDSEVERITY(check_stabilization_settings(5, multirotor, coptercontrol));
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED6:
            ADDSEVERITY(check_stabilization_settings(6, multirotor, coptercontrol));
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_AUTOTUNE:
            ADDSEVERITY(PIOS_TASK_MONITOR_IsRunning(TASKINFO_RUNNING_AUTOTUNE));
            break;
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_PATHPLANNER:
        {
            // Revo supports PathPlanner and that must be OK or we are not sane
            // PathPlan alarm is uninitialized if not running
            // PathPlan alarm is warning or error if the flightplan is invalid
            SystemAlarmsAlarmData alarms;
            SystemAlarmsAlarmGet(&alarms);
            ADDSEVERITY(alarms.PathPlan == SYSTEMALARMS_ALARM_OK);
        }
        // intentionally no break as this also needs pathfollower
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_POSITIONHOLD:
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_POSITIONVARIOFPV:
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_POSITIONVARIOLOS:
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_POSITIONVARIONSEW:
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_LAND:
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_POI:
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_RETURNTOBASE:
        case FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_AUTOCRUISE:
            ADDSEVERITY(!coptercontrol);
            ADDSEVERITY(PIOS_TASK_MONITOR_IsRunning(TASKINFO_RUNNING_PATHFOLLOWER));
            ADDSEVERITY(navCapableFusion);
            break;
        default:
            // Uncovered modes are automatically an error
            ADDSEVERITY(false);
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
 * @returns true or false
 */
static bool check_stabilization_settings(int index, bool multirotor, bool coptercontrol)
{
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
    case 4:
        FlightModeSettingsStabilization4SettingsArrayGet(modes);
        break;
    case 5:
        FlightModeSettingsStabilization5SettingsArrayGet(modes);
        break;
    case 6:
        FlightModeSettingsStabilization6SettingsArrayGet(modes);
        break;
    default:
        return false;
    }

    // For multirotors verify that roll/pitch/yaw are not set to "none"
    // (why not? might be fun to test ones reactions ;) if you dare, set your frame to "custom"!
    if (multirotor) {
        for (uint32_t i = 0; i < FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_THRUST; i++) {
            if (modes[i] == FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_MANUAL) {
                return false;
            }
        }
    }

    // coptercontrol cannot do altitude holding
    if (coptercontrol) {
        if (modes[FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_THRUST] == FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_ALTITUDEHOLD
            || modes[FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_THRUST] == FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_ALTITUDEVARIO
            ) {
            return false;
        }
    }

    // check that thrust modes are only set to thrust axis
    for (uint32_t i = 0; i < FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_THRUST; i++) {
        if (modes[i] == FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_ALTITUDEHOLD
            || modes[i] == FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_ALTITUDEVARIO
            ) {
            return false;
        }
    }
    if (!(modes[FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_THRUST] == FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_MANUAL
          || modes[FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_THRUST] == FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_ALTITUDEHOLD
          || modes[FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_THRUST] == FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_ALTITUDEVARIO
          || modes[FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_THRUST] == FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_CRUISECONTROL
          )) {
        return false;
    }

    // Warning: This assumes that certain conditions in the XML file are met.  That
    // FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_MANUAL has the same numeric value for each channel
    // and is the same for STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL
    // (this is checked at compile time by static constraint manualcontrol.h)

    return true;
}
