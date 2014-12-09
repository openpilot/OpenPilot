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
#include <adjustments.h>
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
static float thrustAtBrakeStart = 0.0f;
static float thrustLo = 0.0f;
static float thrustHi = 0.0f;

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
    AdjustmentsInitialize();
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
            float neutralThrustOffset = 0.0f;
            AdjustmentsNeutralThrustOffsetGet(&neutralThrustOffset);
	    float throttleRangeDelta = (vtolPathFollowerSettings.ThrustLimits.Neutral + neutralThrustOffset) * 0.2f;
	    float throttleNeutralLow = (vtolPathFollowerSettings.ThrustLimits.Neutral + neutralThrustOffset) - throttleRangeDelta;
	    float throttleNeutralHi = (vtolPathFollowerSettings.ThrustLimits.Neutral + neutralThrustOffset) + throttleRangeDelta;
	    if (cmd.Thrust > throttleNeutralLow && cmd.Thrust < throttleNeutralHi) throttleNeutral = true;

	    // determine default thrust mode for hold/brake states
	    uint8_t pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
	    if (newFlightModeAssist == FLIGHTSTATUS_FLIGHTMODEASSIST_GPSASSIST) {
		pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTO;
	    }

	    switch (newAssistedControlState) {

		  case  FLIGHTSTATUS_ASSISTEDCONTROLSTATE_PRIMARY:
		    if (!flagRollPitchHasInput) {
			// no stick input on roll/pitch so enter brake state
			newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_BRAKE;
			newAssistedThrottleState = pathfollowerthrustmode;
			handler = &handler_PATHFOLLOWER;
			// retain thrust cmd for later comparison with actual in braking
			thrustAtBrakeStart = cmd.Thrust;
			thrustLo = 0.95f * thrustAtBrakeStart;
			thrustHi = 1.05f * thrustAtBrakeStart;
		    }
		    else {
			// stick input so stay in primary mode control state
			// newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_PRIMARY;
			newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL; // Effectively None
		    }
		    break;

		  case FLIGHTSTATUS_ASSISTEDCONTROLSTATE_BRAKE:
		    if (flagRollPitchHasInput) {
			// stick input during brake sequence allows immediate resumption of control
			newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_PRIMARY;
			newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL; // Effectively None
		    }
		    else {
			// no stick input, stay in brake mode
			newAssistedThrottleState = pathfollowerthrustmode;
			handler = &handler_PATHFOLLOWER;
			// if user adjusts thrust in which case revert to manual
			if (cmd.Thrust < thrustLo || cmd.Thrust > thrustHi) newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
		    }
		    break;

		  case FLIGHTSTATUS_ASSISTEDCONTROLSTATE_HOLD:

		    if (newFlightModeAssist == FLIGHTSTATUS_FLIGHTMODEASSIST_GPSASSISTMANUALTHRUST ||
			(newFlightModeAssist == FLIGHTSTATUS_FLIGHTMODEASSIST_GPSASSIST &&
			    newAssistedThrottleState == FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL) ) {

			// for manual throttle states/modes, stick control immediately reverts to primary mode control
			if (flagRollPitchHasInput) {
			    newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_PRIMARY;
			    newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL; // Effectively None
		        }
		        else {
		            // otherwise stay in the hold state
			    //newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_HOLD;
		            newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
		            handler = &handler_PATHFOLLOWER;
			}
		    }
		    else if (newFlightModeAssist == FLIGHTSTATUS_FLIGHTMODEASSIST_GPSASSIST &&
			     newAssistedThrottleState != FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL) {
			// ok auto thrust mode applies in hold unless overridden

			if (throttleNeutral && flagRollPitchHasInput) {
			    // throttle is neutral approximately and stick input present so revert to primary mode control
			    newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_PRIMARY;
			    newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL; // Effectively None
			}
			else {
			    // otherwise hold, autothrust, no stick input...we watch throttle for need to change mode
			    switch (newAssistedThrottleState) {
			      case FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTO:
				// whilst in auto, watch for neutral range, from which we allow override
				if (throttleNeutral) pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTOOVERRIDE;
				else pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTO;
				break;
			      case FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTOOVERRIDE:
				// previously user has set throttle to neutral range, apply a deadband and revert to manual
				// if moving out of deadband.  This allows for landing in hold state.
				if (!throttleNeutral) pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
				else pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTOOVERRIDE;
				break;
			      case FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL:
				pathfollowerthrustmode = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
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

	    switch (newFlightModeAssist) {
	      case FLIGHTSTATUS_FLIGHTMODEASSIST_GPSASSISTMANUALTHRUST:
		    newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL;
		break;
	      case FLIGHTSTATUS_FLIGHTMODEASSIST_GPSASSIST:
		    newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_AUTO;
		break;
	      case FLIGHTSTATUS_FLIGHTMODEASSIST_NONE:
	      default:
		    newAssistedThrottleState = FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL; // Effectively None
		break;
	    }

	    switch (newAssistedControlState) {
		  case FLIGHTSTATUS_ASSISTEDCONTROLSTATE_BRAKE:
			newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_BRAKE;
			break;
		  case  FLIGHTSTATUS_ASSISTEDCONTROLSTATE_PRIMARY:
			newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_BRAKE;
			break;
		  case FLIGHTSTATUS_ASSISTEDCONTROLSTATE_HOLD:
			newAssistedControlState = FLIGHTSTATUS_ASSISTEDCONTROLSTATE_HOLD;
			break;
	    }
        }
        handler = &handler_PATHFOLLOWER;
        break;



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
