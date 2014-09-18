/**
 ******************************************************************************
 *
 * @file       altitudeloop.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
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

#include <openpilot.h>

#include <callbackinfo.h>

#include <pid.h>
#include <altitudeloop.h>
#include <CoordinateConversions.h>
#include <altitudeholdsettings.h>
#include <altitudeholdstatus.h>
#include <velocitystate.h>
#include <positionstate.h>
// Private constants


#ifdef REVOLUTION

#define UPDATE_EXPECTED   (1.0f / 666.0f)
#define UPDATE_MIN        1.0e-6f
#define UPDATE_MAX        1.0f
#define UPDATE_ALPHA      1.0e-2f

#define CALLBACK_PRIORITY CALLBACK_PRIORITY_LOW
#define CBTASK_PRIORITY   CALLBACK_TASK_FLIGHTCONTROL

#define STACK_SIZE_BYTES  512
// Private types

// Private variables
static DelayedCallbackInfo *altitudeHoldCBInfo;
static AltitudeHoldSettingsData altitudeHoldSettings;
static struct pid pid0, pid1;
static ThrustModeType thrustMode;
static PiOSDeltatimeConfig timeval;
static float thrustSetpoint = 0.0f;
static float thrustDemand   = 0.0f;
static float startThrust    = 0.5f;


// Private functions
static void altitudeHoldTask(void);
static void SettingsUpdatedCb(UAVObjEvent *ev);
static void VelocityStateUpdatedCb(UAVObjEvent *ev);

/**
 * Setup mode and setpoint
 */
float stabilizationAltitudeHold(float setpoint, ThrustModeType mode, bool reinit)
{
    static bool newaltitude = true;

    if (reinit) {
        startThrust = setpoint;
        pid_zero(&pid0);
        pid_zero(&pid1);
        newaltitude = true;
    }

    const float DEADBAND      = 0.20f;
    const float DEADBAND_HIGH = 1.0f / 2 + DEADBAND / 2;
    const float DEADBAND_LOW  = 1.0f / 2 - DEADBAND / 2;

    // this is the max speed in m/s at the extents of thrust
    float thrustRate;
    uint8_t thrustExp;

    AltitudeHoldSettingsThrustExpGet(&thrustExp);
    AltitudeHoldSettingsThrustRateGet(&thrustRate);

    PositionStateData posState;
    PositionStateGet(&posState);

    if (altitudeHoldSettings.CutThrustWhenZero && setpoint <= 0) {
        // Cut thrust if desired
        thrustSetpoint = 0.0f;
        thrustDemand   = 0.0f;
        thrustMode     = DIRECT;
        newaltitude    = true;
    } else if (mode == ALTITUDEVARIO && setpoint > DEADBAND_HIGH) {
        // being the two band symmetrical I can divide by DEADBAND_LOW to scale it to a value betweeon 0 and 1
        // then apply an "exp" f(x,k) = (k*x*x*x + (255-k)*x) / 255
        thrustSetpoint = -((thrustExp * powf((setpoint - DEADBAND_HIGH) / (DEADBAND_LOW), 3) + (255 - thrustExp) * (setpoint - DEADBAND_HIGH) / DEADBAND_LOW) / 255 * thrustRate);
        thrustMode     = ALTITUDEVARIO;
        newaltitude    = true;
    } else if (mode == ALTITUDEVARIO && setpoint < DEADBAND_LOW) {
        thrustSetpoint = -(-(thrustExp * powf((DEADBAND_LOW - (setpoint < 0 ? 0 : setpoint)) / DEADBAND_LOW, 3) + (255 - thrustExp) * (DEADBAND_LOW - setpoint) / DEADBAND_LOW) / 255 * thrustRate);
        thrustMode     = ALTITUDEVARIO;
        newaltitude    = true;
    } else if (newaltitude == true) {
        thrustSetpoint = posState.Down;
        thrustMode     = ALTITUDEHOLD;
        newaltitude    = false;
    }

    return thrustDemand;
}

/**
 * Initialise the module, called on startup
 */
void stabilizationAltitudeloopInit()
{
    AltitudeHoldSettingsInitialize();
    AltitudeHoldStatusInitialize();
    PositionStateInitialize();
    VelocityStateInitialize();

    PIOS_DELTATIME_Init(&timeval, UPDATE_EXPECTED, UPDATE_MIN, UPDATE_MAX, UPDATE_ALPHA);
    // Create object queue

    altitudeHoldCBInfo = PIOS_CALLBACKSCHEDULER_Create(&altitudeHoldTask, CALLBACK_PRIORITY, CBTASK_PRIORITY, CALLBACKINFO_RUNNING_ALTITUDEHOLD, STACK_SIZE_BYTES);
    AltitudeHoldSettingsConnectCallback(&SettingsUpdatedCb);
    VelocityStateConnectCallback(&VelocityStateUpdatedCb);

    // Start main task
    SettingsUpdatedCb(NULL);
}


/**
 * Module thread, should not return.
 */
static void altitudeHoldTask(void)
{
    AltitudeHoldStatusData altitudeHoldStatus;

    AltitudeHoldStatusGet(&altitudeHoldStatus);

    // do the actual control loop(s)
    float positionStateDown;
    PositionStateDownGet(&positionStateDown);
    float velocityStateDown;
    VelocityStateDownGet(&velocityStateDown);

    float dT;
    dT = PIOS_DELTATIME_GetAverageSeconds(&timeval);
    switch (thrustMode) {
    case ALTITUDEHOLD:
        // altitude control loop
        // No scaling.
        pid_scaler scaler = { .p = 1.0f, .i = 1.0f, .d = 1.0f };
        altitudeHoldStatus.VelocityDesired = pid_apply_setpoint(&pid0, &scaler, thrustSetpoint, positionStateDown, dT);
        break;
    case ALTITUDEVARIO:
        altitudeHoldStatus.VelocityDesired = thrustSetpoint;
        break;
    default:
        altitudeHoldStatus.VelocityDesired = 0;
        break;
    }

    AltitudeHoldStatusSet(&altitudeHoldStatus);

    switch (thrustMode) {
    case DIRECT:
        thrustDemand = thrustSetpoint;
        break;
    default:
        // velocity control loop
        // No scaling.
        pid_scaler scaler = { .p = 1.0f, .i = 1.0f, .d = 1.0f };
        thrustDemand = startThrust - pid_apply_setpoint(&pid1, &scaler, altitudeHoldStatus.VelocityDesired, velocityStateDown, dT);

        break;
    }
}

static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    AltitudeHoldSettingsGet(&altitudeHoldSettings);
    pid_configure(&pid0, altitudeHoldSettings.AltitudePI.Kp, altitudeHoldSettings.AltitudePI.Ki, 0, altitudeHoldSettings.AltitudePI.Ilimit);
    pid_zero(&pid0);
    pid_configure(&pid1, altitudeHoldSettings.VelocityPI.Kp, altitudeHoldSettings.VelocityPI.Ki, 0, altitudeHoldSettings.VelocityPI.Ilimit);
    pid_zero(&pid1);
}

static void VelocityStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    PIOS_CALLBACKSCHEDULER_Dispatch(altitudeHoldCBInfo);
}


#endif /* ifdef REVOLUTION */
