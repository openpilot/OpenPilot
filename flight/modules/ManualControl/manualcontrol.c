/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup ManualControlModule Manual Control Module
 * @brief Provide manual control or allow it alter flight mode.
 * @{
 *
 * Reads in the ManualControlCommand FlightMode setting from receiver then either
 * pass the settings straght to ActuatorDesired object (manual mode) or to
 * AttitudeDesired object (stabilized mode)
 *
 * @file       manualcontrol.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      ManualControl module. Handles safety R/C link and flight mode.
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

#include "inc/manualcontrol.h"
#include <sanitycheck.h>
#include <manualcontrolsettings.h>
#include <manualcontrolcommand.h>
#include <flightmodesettings.h>
#include <flightstatus.h>
#include <systemsettings.h>
#include <stabilizationdesired.h>
#include <callbackinfo.h>
#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
#include <stabilizationsettings.h>
#include <vtolpathfollowersettings.h>
#endif

// Private constants
#if defined(PIOS_MANUAL_STACK_SIZE)
#define STACK_SIZE_BYTES  PIOS_MANUAL_STACK_SIZE
#else
#define STACK_SIZE_BYTES  1152
#endif

#define CALLBACK_PRIORITY CALLBACK_PRIORITY_REGULAR
#define CBTASK_PRIORITY   CALLBACK_TASK_FLIGHTCONTROL


// defined handlers

static const controlHandler handler_MANUAL = {
    .controlChain      = {
        .Stabilization = false,
        .PathFollower  = false,
        .PathPlanner   = false,
    },
    .handler           = &manualHandler,
};
static const controlHandler handler_STABILIZED = {
    .controlChain      = {
        .Stabilization = true,
        .PathFollower  = false,
        .PathPlanner   = false,
    },
    .handler           = &stabilizedHandler,
};


static const controlHandler handler_AUTOTUNE = {
    .controlChain      = {
        .Stabilization = false,
        .PathFollower  = false,
        .PathPlanner   = false,
    },
    .handler           = NULL,
};

#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
static const controlHandler handler_PATHFOLLOWER = {
    .controlChain      = {
        .Stabilization = true,
        .PathFollower  = true,
        .PathPlanner   = false,
    },
    .handler           = &pathFollowerHandler,
};

static const controlHandler handler_PATHPLANNER = {
    .controlChain      = {
        .Stabilization = true,
        .PathFollower  = true,
        .PathPlanner   = true,
    },
    .handler           = &pathPlannerHandler,
};
static uint32_t thrustAtBrakeStart = 0.0f;

#endif /* ifndef PIOS_EXCLUDE_ADVANCED_FEATURES */
// Private variables
static DelayedCallbackInfo *callbackHandle;

// Private functions
static void configurationUpdatedCb(UAVObjEvent *ev);
static void commandUpdatedCb(UAVObjEvent *ev);
static void manualControlTask(void);
#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
static uint8_t isAssistedFlightMode( uint8_t position);
#endif

#define assumptions (assumptions1 && assumptions2 && assumptions3 && assumptions4 && assumptions5 && assumptions6 && assumptions7 && assumptions_flightmode)

/**
 * Module starting
 */
int32_t ManualControlStart()
{
    // Run this initially to make sure the configuration is checked
    configuration_check();

    // Whenever the configuration changes, make sure it is safe to fly
    SystemSettingsConnectCallback(configurationUpdatedCb);
    ManualControlSettingsConnectCallback(configurationUpdatedCb);
    ManualControlCommandConnectCallback(commandUpdatedCb);

    // clear alarms
    AlarmsClear(SYSTEMALARMS_ALARM_MANUALCONTROL);

    // Make sure unarmed on power up
    armHandler(true);

#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
    takeOffLocationHandlerInit();
#endif
    // Start main task
    PIOS_CALLBACKSCHEDULER_Dispatch(callbackHandle);

    return 0;
}

/**
 * Module initialization
 */
int32_t ManualControlInitialize()
{
    /* Check the assumptions about uavobject enum's are correct */
    PIOS_STATIC_ASSERT(assumptions);

    ManualControlCommandInitialize();
    FlightStatusInitialize();
    ManualControlSettingsInitialize();
    FlightModeSettingsInitialize();
    SystemSettingsInitialize();
#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
    StabilizationSettingsInitialize();
    VtolPathFollowerSettingsInitialize();
#endif
    callbackHandle = PIOS_CALLBACKSCHEDULER_Create(&manualControlTask, CALLBACK_PRIORITY, CBTASK_PRIORITY, CALLBACKINFO_RUNNING_MANUALCONTROL, STACK_SIZE_BYTES);

    return 0;
}
MODULE_INITCALL(ManualControlInitialize, ManualControlStart);

/**
 * Module task
 */
static void manualControlTask(void)
{
    // Process Arming
    armHandler(false);
#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
    takeOffLocationHandler();
#endif
    // Process flight mode
    FlightStatusData flightStatus;

    FlightStatusGet(&flightStatus);
    ManualControlCommandData cmd;
    ManualControlCommandGet(&cmd);
#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
    VtolPathFollowerSettingsData vtolPathFollowerSettings;
    VtolPathFollowerSettingsGet(&vtolPathFollowerSettings);
#endif

    FlightModeSettingsData modeSettings;
    FlightModeSettingsGet(&modeSettings);

    uint8_t position = cmd.FlightModeSwitchPosition;
    uint8_t newMode  = flightStatus.FlightMode;
    uint8_t newFlightModeAssist = flightStatus.FlightModeAssist;
    uint8_t newAssistedControlState  = flightStatus.AssistedControlState;
    uint8_t newAssistedThrottleState = flightStatus.AssistedThrottleState;
    if (position < FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_NUMELEM) {
        newMode = modeSettings.FlightModePosition[position];
    }

    // Depending on the mode update the Stabilization or Actuator objects
    const controlHandler *handler = &handler_MANUAL;
    switch (newMode) {
    case FLIGHTSTATUS_FLIGHTMODE_MANUAL:
        handler = &handler_MANUAL;
        break;
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED1:
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED2:
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED3:
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED4:
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED5:
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED6:
        handler = &handler_STABILIZED;

#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
        newFlightModeAssist = isAssistedFlightMode( position );
        if (newFlightModeAssist) {

            // assess roll/pitch state
            bool flagRollPitchHasInput = (fabsf(cmd.Roll) > 0.0f || fabsf(cmd.Pitch) > 0.0f);

            // assess throttle state
            bool throttleNeutral = false;
            // TODO Add in adjustments
	    float throttleRangeDelta = vtolPathFollowerSettings.ThrustLimits.Neutral * 0.35f;
	    float throttleNeutralLow = vtolPathFollowerSettings.ThrustLimits.Neutral - throttleRangeDelta;
	    float throttleNeutralHi = vtolPathFollowerSettings.ThrustLimits.Neutral + throttleRangeDelta;
	    if (cmd.Thrust > throttleNeutralLow && cmd.Thrust < throttleNeutralHi) throttleNeutral = true;

	    switch (flightStatus.AssistedControlState) {
		  case  FLIGHTSTATUS_ASSISTEDCONTROLSTATE_PRIMARY:
		    if (!flagRollPitchHasInput) {
			newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_BRAKE;

			uint8_t pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTO;
			if (newFlightModeAssist == FLIGHTSTATUS_FLIGHTMODEASSIST_GPSASSISTMANUALTHRUST) {
			    pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
			}

			newAssistedThrottleState = pathfollowerthrustmode;
			handler = &handler_PATHFOLLOWER;
			thrustAtBrakeStart = cmd.Thrust;
		    }
		    // otherwise stabi handler and manual thrust
		    break;

		  case FLIGHTSTATUS_ASSISTEDCONTROLSTATE_BRAKE:
		    if (flagRollPitchHasInput) { // sticks can easily override braking
			newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_PRIMARY;
			newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
			// and stabi handler
		    }
		    else {
			uint8_t pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTO;
			if (newFlightModeAssist == FLIGHTSTATUS_FLIGHTMODEASSIST_GPSASSISTMANUALTHRUST) {
			    pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
			}
			// when braking always auto throttle for this mode
			newAssistedThrottleState = pathfollowerthrustmode;
			handler = &handler_PATHFOLLOWER;
			// except if user adjusts thrust in which case revert to manual
			float thrustLo = 0.95f * thrustAtBrakeStart;
			float thrustHi = 1.05f * thrustAtBrakeStart;
			if (cmd.Thrust < thrustLo || cmd.Thrust > thrustHi) newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
		    }
		    break;

		    // transition from braking to positionhold will retain throttle state in auto
		  case FLIGHTSTATUS_ASSISTEDCONTROLSTATE_HOLD:

		    if (newFlightModeAssist == FLIGHTSTATUS_FLIGHTMODEASSIST_GPSASSISTMANUALTHRUST) {

			if (flagRollPitchHasInput) { // need neutral to break from PH
			    newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_PRIMARY;
			    newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
			    // and stabi handler
		        }
		        else {
		            newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
		            handler = &handler_PATHFOLLOWER;
			}
		    }
		    else {

			if (throttleNeutral && flagRollPitchHasInput) { // need neutral to break from PH
			    newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_PRIMARY;
			    newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
			    // and stabi handler
			}
			else {
			    uint8_t pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTO;
			    switch (flightStatus.AssistedThrottleState) {
			      case FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTO:
				if (throttleNeutral) pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTOOVERRIDE;
				break;
			      case FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTOOVERRIDE:
				if (!throttleNeutral) pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
				break;
			    }

			    newAssistedThrottleState = pathfollowerthrustmode;
			    handler = &handler_PATHFOLLOWER;
			}

		    }


		    break;
	    }

        }
#endif
        break;
#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES

    case FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD:
        newFlightModeAssist = isAssistedFlightMode( position );
        if (newFlightModeAssist) {
            newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_BRAKE;
            if (newFlightModeAssist == FLIGHTSTATUS_FLIGHTMODEASSIST_GPSASSISTMANUALTHRUST) {
	        newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
            }
            else {
	        newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTO; // still gets overridden by vtol settings
            }
        }
    case FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIOFPV:
    case FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIOLOS:
    case FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIONSEW:
    case FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE:
    case FLIGHTSTATUS_FLIGHTMODE_LAND:
    case FLIGHTSTATUS_FLIGHTMODE_POI:
    case FLIGHTSTATUS_FLIGHTMODE_AUTOCRUISE:
        handler = &handler_PATHFOLLOWER;
        break;
    case FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER:
        handler = &handler_PATHPLANNER;
        break;
#endif
    case FLIGHTSTATUS_FLIGHTMODE_AUTOTUNE:
        handler = &handler_AUTOTUNE;
        break;
        // There is no default, so if a flightmode is forgotten the compiler can throw a warning!
    }

    bool newinit = false;

    // FlightMode needs to be set correctly on first run (otherwise ControlChain is invalid)
    static bool firstRun = true;

    if (flightStatus.FlightMode != newMode || firstRun ||
	newAssistedControlState != flightStatus.AssistedControlState ||
	flightStatus.AssistedThrottleState != newAssistedThrottleState) {
        firstRun = false;
        flightStatus.ControlChain = handler->controlChain;
        flightStatus.FlightMode   = newMode;
        flightStatus.FlightModeAssist = newFlightModeAssist;
        flightStatus.AssistedControlState = newAssistedControlState;
        flightStatus.AssistedThrottleState = newAssistedThrottleState;
        FlightStatusSet(&flightStatus);
        newinit = true;
    }
    if (handler->handler) {
        handler->handler(newinit);
    }
}

/**
 * Called whenever a critical configuration component changes
 */
static void configurationUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    configuration_check();
}

/**
 * Called whenever a critical configuration component changes
 */
static void commandUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    PIOS_CALLBACKSCHEDULER_Dispatch(callbackHandle);
}


/**
 * Check and set modes for gps assisted stablised flight modes
 */
#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
static uint8_t isAssistedFlightMode( uint8_t position)
{

    uint8_t isAssistedFlag = STABILIZATIONSETTINGS_FLIGHTMODEASSISTMAP_NONE;
    uint8_t FlightModeAssistMap[STABILIZATIONSETTINGS_FLIGHTMODEASSISTMAP_NUMELEM];

    StabilizationSettingsFlightModeAssistMapGet(FlightModeAssistMap);

    if (position < STABILIZATIONSETTINGS_FLIGHTMODEASSISTMAP_NUMELEM) {
        isAssistedFlag = FlightModeAssistMap[position];
    }

    return isAssistedFlag;
}
#endif

/**
 * @}
 * @}
 */
