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
#include <attitudestate.h>
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
static struct pid accelpid;
static float accelStateDown;


// Private functions
static void altitudeHoldTask(void);
static void SettingsUpdatedCb(UAVObjEvent *ev);
static void AccelStateUpdatedCb(UAVObjEvent *ev);

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
    AccelStateConnectCallback(&AccelStateUpdatedCb);

    return 0;
}
MODULE_INITCALL(AltitudeHoldInitialize, AltitudeHoldStart);


/**
 * Module thread, should not return.
 */
static void altitudeHoldTask(void)
{
    static float startThrottle = 0.5f;

    // make sure we run only when we are supposed to run
    FlightStatusData flightStatus;

    FlightStatusGet(&flightStatus);
    switch (flightStatus.FlightMode) {
    case FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD:
    case FLIGHTSTATUS_FLIGHTMODE_ALTITUDEVARIO:
        break;
    default:
        pid_zero(&accelpid);
        StabilizationDesiredThrottleGet(&startThrottle);
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

    // altitude control loop
    altitudeHoldStatus.VelocityDesired     = altitudeHoldSettings.AltitudeP * (positionStateDown - altitudeHoldDesired.Altitude) + altitudeHoldDesired.Velocity;

    // velocity control loop
    altitudeHoldStatus.AccelerationDesired = altitudeHoldSettings.VelocityP * (velocityStateDown - altitudeHoldStatus.VelocityDesired) - 9.81f;

    altitudeHoldStatus.AccelerationFiltered = accelStateDown;

    AltitudeHoldStatusSet(&altitudeHoldStatus);

    // compensate acceleration by rotation
    // explanation: Rbe[2][2] is the Down component of a 0,0,1 vector rotated by Attitude.Q
    // It is 1.0 for no rotation, 0.0 for a 90 degrees roll or pitch and -1.0 for a 180 degrees flipped rotation
    // multiplying with 1/Rbe[2][2] therefore is the acceleration/thrust required to overcome gravity and achieve the wanted vertical
    // acceleration at the current tilt angle.
    // around 90 degrees rotation this is infinite (since no possible acceleration would get us up or down) so we set the error to zero to keep
    // integrals from winding in any direction

    AttitudeStateData attitudeState;
    AttitudeStateGet(&attitudeState);
    float Rbe[3][3];
    Quaternion2R(&attitudeState.q1, Rbe);

    float rotatedAccelDesired = altitudeHoldStatus.AccelerationDesired;
#if 0
    if (fabsf(Rbe[2][2]) > 1e-3f) {
        rotatedAccelDesired /= Rbe[2][2];
    } else {
        rotatedAccelDesired = accelStateDown;
    }
#endif

    // acceleration control loop
    float throttle = startThrottle - pid_apply_setpoint(&accelpid, 1.0f, rotatedAccelDesired, accelStateDown, 1000 / DESIRED_UPDATE_RATE_MS);

    if (throttle >= 1.0f) {
        throttle = 1.0f;
    }
    if (throttle <= 0.0f) {
        throttle = 0.0f;
    }
    StabilizationDesiredData stab;
    StabilizationDesiredGet(&stab);
    stab.Roll     = altitudeHoldDesired.Roll;
    stab.Pitch    = altitudeHoldDesired.Pitch;
    stab.Yaw      = altitudeHoldDesired.Yaw;
    stab.Throttle = throttle;
    stab.StabilizationMode.Roll  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stab.StabilizationMode.Pitch = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stab.StabilizationMode.Yaw   = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;

    StabilizationDesiredSet(&stab);

    DelayedCallbackSchedule(altitudeHoldCBInfo, DESIRED_UPDATE_RATE_MS, CALLBACK_UPDATEMODE_SOONER);
}

static void AccelStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    float down;

    AccelStatezGet(&down);
    accelStateDown = down * altitudeHoldSettings.AccelAlpha + accelStateDown * (1.0f - altitudeHoldSettings.AccelAlpha);
}

static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    AltitudeHoldSettingsGet(&altitudeHoldSettings);
    pid_configure(&accelpid, altitudeHoldSettings.AccelPI.Kp, altitudeHoldSettings.AccelPI.Kp, 0, altitudeHoldSettings.AccelPI.Ilimit);
    pid_zero(&accelpid);
    accelStateDown = 0.0f;
}
