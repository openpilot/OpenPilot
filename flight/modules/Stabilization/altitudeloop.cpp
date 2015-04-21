/**
 ******************************************************************************
 *
 * @file       altitudeloop.cpp
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

extern "C" {
#include <openpilot.h>

#include <callbackinfo.h>

#include <pid.h>
#include <altitudeloop.h>
#include <CoordinateConversions.h>
#include <altitudeholdsettings.h>
#include <altitudeholdstatus.h>
#include <velocitystate.h>
#include <positionstate.h>
#include <vtolselftuningstats.h>
}

#include <pidcontroldown.h>

// Private constants


#ifdef REVOLUTION

#define UPDATE_EXPECTED   (1.0f / PIOS_SENSOR_RATE)
#define UPDATE_MIN        1.0e-6f
#define UPDATE_MAX        1.0f
#define UPDATE_ALPHA      1.0e-2f

#define CALLBACK_PRIORITY CALLBACK_PRIORITY_LOW
#define CBTASK_PRIORITY   CALLBACK_TASK_FLIGHTCONTROL

#define STACK_SIZE_BYTES  512
// Private types

// Private variables
static PIDControlDown controlDown;
static DelayedCallbackInfo *altitudeHoldCBInfo;
static AltitudeHoldSettingsData altitudeHoldSettings;
static struct pid pid0, pid1;
static ThrustModeType thrustMode;
static PiOSDeltatimeConfig timeval;
static float thrustSetpoint = 0.0f;
static float thrustDemand   = 0.0f;
    // this is the max speed in m/s at the extents of thrust
    static float thrustRate;
    static uint8_t thrustExp;


// Private functions
static void altitudeHoldTask(void);
static void SettingsUpdatedCb(UAVObjEvent *ev);
static void VelocityStateUpdatedCb(UAVObjEvent *ev);

/**
 * Setup mode and setpoint
 *
 * reinit: true when althold/vario mode selected over a previous alternate thrust mode
 */
float stabilizationAltitudeHold(float setpoint, ThrustModeType mode, bool reinit)
{
    static bool newaltitude = true;

    if (reinit) {
	controlDown.Activate();
        newaltitude = true;
    }

    const float DEADBAND      = 0.20f;
    const float DEADBAND_HIGH = 1.0f / 2 + DEADBAND / 2;
    const float DEADBAND_LOW  = 1.0f / 2 - DEADBAND / 2;


    if (altitudeHoldSettings.CutThrustWhenZero && setpoint <= 0) {
        // Cut thrust if desired
        controlDown.UpdateVelocitySetpoint(0.0f);
        thrustDemand   = 0.0f;
        thrustMode     = DIRECT;
        newaltitude    = true;
    } else if (mode == ALTITUDEVARIO && setpoint > DEADBAND_HIGH) {
        // being the two band symmetrical I can divide by DEADBAND_LOW to scale it to a value betweeon 0 and 1
        // then apply an "exp" f(x,k) = (k*x*x*x + (255-k)*x) / 255
	controlDown.UpdateVelocitySetpoint(-((thrustExp * powf((setpoint - DEADBAND_HIGH) / (DEADBAND_LOW), 3) + (255 - thrustExp) * (setpoint - DEADBAND_HIGH) / DEADBAND_LOW) / 255 * thrustRate));
        thrustMode     = ALTITUDEVARIO;
        newaltitude    = true;
    } else if (mode == ALTITUDEVARIO && setpoint < DEADBAND_LOW) {
	controlDown.UpdateVelocitySetpoint(-(-(thrustExp * powf((DEADBAND_LOW - (setpoint < 0 ? 0 : setpoint)) / DEADBAND_LOW, 3) + (255 - thrustExp) * (DEADBAND_LOW - setpoint) / DEADBAND_LOW) / 255 * thrustRate));
        thrustMode     = ALTITUDEVARIO;
        newaltitude    = true;
    } else if (newaltitude == true) {
        controlDown.UpdateVelocitySetpoint(0.0f);
        PositionStateData posState;
        PositionStateGet(&posState);
        controlDown.UpdatePositionSetpoint(posState.Down);
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

    float velocityStateDown;
    VelocityStateDownGet(&velocityStateDown);
    controlDown.UpdateVelocityState(velocityStateDown);

    float dT;
    dT = PIOS_DELTATIME_GetAverageSeconds(&timeval);

    switch (thrustMode) {
    case ALTITUDEHOLD:
    {
        float positionStateDown;
        PositionStateDownGet(&positionStateDown);
        controlDown.UpdatePositionState(positionStateDown);
        controlDown.ControlPosition();
        altitudeHoldStatus.VelocityDesired  = controlDown.GetVelocityDesired();
    }
    break;
    case ALTITUDEVARIO:
        altitudeHoldStatus.VelocityDesired  = controlDown.GetVelocityDesired();
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
    {
        thrustDemand = controlDown.GetDownCommand();
    }
    break;
    }
}

static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    AltitudeHoldSettingsGet(&altitudeHoldSettings);
    //pid_configure(&pid0, altitudeHoldSettings.AltitudePI.Kp, altitudeHoldSettings.AltitudePI.Ki, 0, altitudeHoldSettings.AltitudePI.Ilimit);
    //pid_zero(&pid0);
    //pid_configure(&pid1, altitudeHoldSettings.VelocityPI.Kp, altitudeHoldSettings.VelocityPI.Ki, 0, altitudeHoldSettings.VelocityPI.Ilimit);
    //pid_zero(&pid1);

    controlDown.UpdateParameters(vtolPathFollowerSettings->LandVerticalVelPID.Kp,
                                    vtolPathFollowerSettings->LandVerticalVelPID.Ki,
                                    vtolPathFollowerSettings->LandVerticalVelPID.Kd,
                                    vtolPathFollowerSettings->LandVerticalVelPID.Beta,
                                    UPDATE_EXPECTED,
                                    vtolPathFollowerSettings->VerticalVelMax);

    // The following is not currently used in the landing control.
    controlDown.UpdatePositionalParameters(vtolPathFollowerSettings->VerticalPosP);

    VtolSelfTuningStatsData vtolSelfTuningStats;
    VtolSelfTuningStatsGet(&vtolSelfTuningStats);
    controlDown.UpdateNeutralThrust(vtolSelfTuningStats.NeutralThrustOffset + vtolPathFollowerSettings->ThrustLimits.Neutral);

    // initialise limits on thrust but note the FSM can override.
    controlDown.SetThrustLimits(vtolPathFollowerSettings->ThrustLimits.Min, vtolPathFollowerSettings->ThrustLimits.Max);

    AltitudeHoldSettingsThrustExpGet(&thrustExp);
    AltitudeHoldSettingsThrustRateGet(&thrustRate);
}

static void VelocityStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    PIOS_CALLBACKSCHEDULER_Dispatch(altitudeHoldCBInfo);
}


#endif /* ifdef REVOLUTION */
