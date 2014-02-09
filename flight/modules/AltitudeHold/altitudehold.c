/**
 ******************************************************************************
 *
 * @file       guidance.c
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
 * Input object: ActiveWaypoint
 * Input object: PositionState
 * Input object: ManualControlCommand
 * Output object: AttitudeDesired
 *
 * This module will periodically update the value of the AttitudeDesired object.
 *
 * The module executes in its own thread in this example.
 *
 * Modules have no API, all communication to other modules is done through UAVObjects.
 * However modules may use the API exposed by shared libraries.
 * See the OpenPilot wiki for more details.
 * http://www.openpilot.org/OpenPilot_Application_Architecture
 *
 */

#include <openpilot.h>

#include <math.h>
#include <pid.h>
#include <CoordinateConversions.h>
#include <altitudeholdsettings.h>
#include <altitudeholddesired.h> // object that will be updated by the module
#include <altitudeholdstatus.h>
#include <flightstatus.h>
#include <stabilizationdesired.h>
#include <accelstate.h>
#include <pios_constants.h>
#include <velocitystate.h>
#include <positionstate.h>
// Private constants

#define CALLBACK_PRIORITY      CALLBACK_PRIORITY_LOW
#define CBTASK_PRIORITY        CALLBACK_TASK_FLIGHTCONTROL

#define STACK_SIZE_BYTES       1024
#define DESIRED_UPDATE_RATE_MS 100 // milliseconds
// Private types

// Private variables
static DelayedCallbackInfo *altitudeHoldCBInfo;
static AltitudeHoldSettingsData altitudeHoldSettings;
static struct pid pid0, pid1;


// Private functions
static void altitudeHoldTask(void);
static void SettingsUpdatedCb(UAVObjEvent *ev);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AltitudeHoldStart()
{
    // Start main task
    SettingsUpdatedCb(NULL);
    DelayedCallbackDispatch(altitudeHoldCBInfo);

    return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AltitudeHoldInitialize()
{
    AltitudeHoldSettingsInitialize();
    AltitudeHoldDesiredInitialize();
    AltitudeHoldStatusInitialize();

    // Create object queue

    altitudeHoldCBInfo = DelayedCallbackCreate(&altitudeHoldTask, CALLBACK_PRIORITY, CBTASK_PRIORITY, STACK_SIZE_BYTES);
    AltitudeHoldSettingsConnectCallback(&SettingsUpdatedCb);

    return 0;
}
MODULE_INITCALL(AltitudeHoldInitialize, AltitudeHoldStart);


/**
 * Module thread, should not return.
 */
static void altitudeHoldTask(void)
{
    static float startThrust = 0.5f;

    // make sure we run only when we are supposed to run
    FlightStatusData flightStatus;

    FlightStatusGet(&flightStatus);
    switch (flightStatus.FlightMode) {
    case FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD:
    case FLIGHTSTATUS_FLIGHTMODE_ALTITUDEVARIO:
        break;
    default:
        pid_zero(&pid0);
        pid_zero(&pid1);
        StabilizationDesiredThrustGet(&startThrust);
        DelayedCallbackSchedule(altitudeHoldCBInfo, DESIRED_UPDATE_RATE_MS, CALLBACK_UPDATEMODE_SOONER);
        return;

        break;
    }

    AltitudeHoldStatusData altitudeHoldStatus;
    AltitudeHoldStatusGet(&altitudeHoldStatus);

    // do the actual control loop(s)
    AltitudeHoldDesiredData altitudeHoldDesired;
    AltitudeHoldDesiredGet(&altitudeHoldDesired);
    float positionStateDown;
    PositionStateDownGet(&positionStateDown);
    float velocityStateDown;
    VelocityStateDownGet(&velocityStateDown);

    switch (altitudeHoldDesired.ControlMode) {
    case ALTITUDEHOLDDESIRED_CONTROLMODE_ALTITUDE:
        // altitude control loop
        altitudeHoldStatus.VelocityDesired = pid_apply_setpoint(&pid0, 1.0f, altitudeHoldDesired.SetPoint, positionStateDown, 1000.0f / DESIRED_UPDATE_RATE_MS);
        break;
    case ALTITUDEHOLDDESIRED_CONTROLMODE_VELOCITY:
        altitudeHoldStatus.VelocityDesired = altitudeHoldDesired.SetPoint;
        break;
    default:
        altitudeHoldStatus.VelocityDesired = 0;
        break;
    }

    AltitudeHoldStatusSet(&altitudeHoldStatus);

    float thrust;
    switch (altitudeHoldDesired.ControlMode) {
    case ALTITUDEHOLDDESIRED_CONTROLMODE_THRUST:
        thrust = altitudeHoldDesired.SetPoint;
        break;
    default:
        // velocity control loop
        thrust = startThrust - pid_apply_setpoint(&pid1, 1.0f, altitudeHoldStatus.VelocityDesired, velocityStateDown, 1000.0f / DESIRED_UPDATE_RATE_MS);

        if (thrust >= 1.0f) {
            thrust = 1.0f;
        }
        if (thrust <= 0.0f) {
            thrust = 0.0f;
        }
        break;
    }

    StabilizationDesiredData stab;
    StabilizationDesiredGet(&stab);
    stab.Roll   = altitudeHoldDesired.Roll;
    stab.Pitch  = altitudeHoldDesired.Pitch;
    stab.Yaw    = altitudeHoldDesired.Yaw;
    stab.Thrust = thrust;
    stab.StabilizationMode.Roll  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stab.StabilizationMode.Pitch = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stab.StabilizationMode.Yaw   = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;

    StabilizationDesiredSet(&stab);

    DelayedCallbackSchedule(altitudeHoldCBInfo, DESIRED_UPDATE_RATE_MS, CALLBACK_UPDATEMODE_SOONER);
}

static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    AltitudeHoldSettingsGet(&altitudeHoldSettings);
    pid_configure(&pid0, altitudeHoldSettings.AltitudePI.Kp, altitudeHoldSettings.AltitudePI.Ki, 0, altitudeHoldSettings.AltitudePI.Ilimit);
    pid_zero(&pid0);
    pid_configure(&pid1, altitudeHoldSettings.VelocityPI.Kp, altitudeHoldSettings.VelocityPI.Ki, 0, altitudeHoldSettings.VelocityPI.Ilimit);
    pid_zero(&pid1);
}
