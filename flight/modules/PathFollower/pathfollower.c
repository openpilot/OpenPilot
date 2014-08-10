/**
 ******************************************************************************
 *
 * @file       pathfollower.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      This module compared @ref PositionActuatl to @ref ActiveWaypoint
 * and sets @ref AttitudeDesired.  It only does this when the FlightMode field
 * of @ref ManualControlCommand is Auto.
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

/**
 * Input object: PathDesired
 * Input object: PositionState
 * Input object: ManualControlCommand
 * Output object: StabilizationDesired
 *
 * This module acts as "autopilot" - it controls the setpoints of stabilization
 * based on current flight situation and desired flight path (PathDesired) as
 * directed by flightmode selection or pathplanner
 * This is a periodic delayed callback module
 *
 * Modules have no API, all communication to other modules is done through UAVObjects.
 * However modules may use the API exposed by shared libraries.
 * See the OpenPilot wiki for more details.
 * http://www.openpilot.org/OpenPilot_Application_Architecture
 *
 */

#include <openpilot.h>

#include <callbackinfo.h>

#include <math.h>
#include <pid.h>
#include <CoordinateConversions.h>


#include <fixedwingpathfollowersettings.h>
#include <vtolpathfollowersettings.h>
#include <flightstatus.h>
#include <pathstatus.h>


// Private constants

#define CALLBACK_PRIORITY      CALLBACK_PRIORITY_LOW
#define CBTASK_PRIORITY        CALLBACK_TASK_FLIGHTCONTROL

#define PF_IDLE_UPDATE_RATE_MS 100

#define STACK_SIZE_BYTES       2048
// Private types

// Private variables
static DelayedCallbackInfo *pathFollowerCBInfo;

static uint32_t updatePeriod = PF_IDLE_UPDATE_RATE_MS;

// Private functions
static void pathFollowerTask(void);
static void SettingsUpdatedCb(UAVObjEvent *ev);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t PathFollowerStart()
{
    // Start main task
    SettingsUpdatedCb(NULL);
    PIOS_CALLBACKSCHEDULER_Dispatch(pathFollowerCBInfo);

    return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t PathFollowerInitialize()
{
    // initialize objects
    FixedWingPathFollowerSettingsInitialize();
    VtolPathFollowerSettingsInitialize();
    FlightStatusInitialize();
    PathStatusInitialize();

    // Create object queue

    pathFollowerCBInfo = PIOS_CALLBACKSCHEDULER_Create(&pathFollowerTask, CALLBACK_PRIORITY, CBTASK_PRIORITY, CALLBACKINFO_RUNNING_ALTITUDEHOLD, STACK_SIZE_BYTES);
    FixedWingPathFollowerSettingsConnectCallback(&SettingsUpdatedCb);
    VtolPathFollowerSettingsConnectCallback(&SettingsUpdatedCb);

    return 0;
}
MODULE_INITCALL(PathFollowerInitialize, PathFollowerStart);


/**
 * Module thread, should not return.
 */
static void pathFollowerTask(void)
{
    FlightStatusData flightStatus;

    FlightStatusGet(&flightStatus);
    if (flightStatus.ControlChain.PathFollower != FLIGHTSTATUS_CONTROLCHAIN_TRUE) {
        PIOS_CALLBACKSCHEDULER_Schedule(pathFollowerCBInfo, PF_IDLE_UPDATE_RATE_MS, CALLBACK_UPDATEMODE_SOONER);
        return;
    }

    // do pathfollower things here

    PIOS_CALLBACKSCHEDULER_Schedule(pathFollowerCBInfo, updatePeriod, CALLBACK_UPDATEMODE_SOONER);
}

static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    // read in settings (TODO)
}
