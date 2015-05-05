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
#include <stabilization.h>
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

// Private types

// Private variables
static DelayedCallbackInfo *altitudeHoldCBInfo;
static PIDControlDown controlDown;
static AltitudeHoldSettingsData altitudeHoldSettings;
static ThrustModeType thrustMode;
static float thrustDemand = 0.0f;


// Private functions
static void SettingsUpdatedCb(UAVObjEvent *ev);
static void altitudeHoldTask(void);
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
        thrustDemand = 0.0f;
        thrustMode   = DIRECT;
        newaltitude  = true;
    } else if (mode == ALTITUDEVARIO && setpoint > DEADBAND_HIGH) {
        // being the two band symmetrical I can divide by DEADBAND_LOW to scale it to a value betweeon 0 and 1
        // then apply an "exp" f(x,k) = (k*x*x*x + (255-k)*x) / 255
        controlDown.UpdateVelocitySetpoint(-((altitudeHoldSettings.ThrustExp * powf((setpoint - DEADBAND_HIGH) / (DEADBAND_LOW), 3.0f) + (255.0f - altitudeHoldSettings.ThrustExp) * (setpoint - DEADBAND_HIGH) / DEADBAND_LOW) / 255.0f * altitudeHoldSettings.ThrustRate));
        thrustMode  = ALTITUDEVARIO;
        newaltitude = true;
    } else if (mode == ALTITUDEVARIO && setpoint < DEADBAND_LOW) {
        controlDown.UpdateVelocitySetpoint(-(-(altitudeHoldSettings.ThrustExp * powf((DEADBAND_LOW - (setpoint < 0 ? 0 : setpoint)) / DEADBAND_LOW, 3.0f) + (255.0f - altitudeHoldSettings.ThrustExp) * (DEADBAND_LOW - setpoint) / DEADBAND_LOW) / 255.0f * altitudeHoldSettings.ThrustRate));
        thrustMode  = ALTITUDEVARIO;
        newaltitude = true;
    } else if (newaltitude == true) {
        controlDown.UpdateVelocitySetpoint(0.0f);
        PositionStateData posState;
        PositionStateGet(&posState);
        controlDown.UpdatePositionSetpoint(posState.Down);
        thrustMode  = ALTITUDEHOLD;
        newaltitude = false;
    }

    return thrustDemand;
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

    switch (thrustMode) {
    case ALTITUDEHOLD:
    {
        float positionStateDown;
        PositionStateDownGet(&positionStateDown);
        controlDown.UpdatePositionState(positionStateDown);
        controlDown.ControlPosition();
        altitudeHoldStatus.VelocityDesired = controlDown.GetVelocityDesired();
        thrustDemand = controlDown.GetDownCommand();
    }
    break;

    case ALTITUDEVARIO:
        altitudeHoldStatus.VelocityDesired = controlDown.GetVelocityDesired();
        thrustDemand = controlDown.GetDownCommand();
        break;

    case DIRECT:
        altitudeHoldStatus.VelocityDesired = 0.0f;
        break;
    }

    AltitudeHoldStatusSet(&altitudeHoldStatus);
}

static void VelocityStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    PIOS_CALLBACKSCHEDULER_Dispatch(altitudeHoldCBInfo);
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
    VtolSelfTuningStatsInitialize();

    AltitudeHoldSettingsConnectCallback(&SettingsUpdatedCb);
    VtolSelfTuningStatsConnectCallback(&SettingsUpdatedCb);
    SettingsUpdatedCb(NULL);

    altitudeHoldCBInfo = PIOS_CALLBACKSCHEDULER_Create(&altitudeHoldTask, CALLBACK_PRIORITY, CBTASK_PRIORITY, CALLBACKINFO_RUNNING_ALTITUDEHOLD, STACK_SIZE_BYTES);
    VelocityStateConnectCallback(&VelocityStateUpdatedCb);
}


static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    AltitudeHoldSettingsGet(&altitudeHoldSettings);

    controlDown.UpdateParameters(altitudeHoldSettings.VerticalVelPID.Kp,
                                 altitudeHoldSettings.VerticalVelPID.Ki,
                                 altitudeHoldSettings.VerticalVelPID.Kd,
                                 altitudeHoldSettings.VerticalVelPID.Beta,
                                 (float)(OUTERLOOP_SKIPCOUNT * UPDATE_EXPECTED),
                                 altitudeHoldSettings.ThrustRate);

    controlDown.UpdatePositionalParameters(altitudeHoldSettings.VerticalPosP);

    VtolSelfTuningStatsData vtolSelfTuningStats;
    VtolSelfTuningStatsGet(&vtolSelfTuningStats);
    controlDown.UpdateNeutralThrust(vtolSelfTuningStats.NeutralThrustOffset + altitudeHoldSettings.ThrustLimits.Neutral);

    // initialise limits on thrust but note the FSM can override.
    controlDown.SetThrustLimits(altitudeHoldSettings.ThrustLimits.Min, altitudeHoldSettings.ThrustLimits.Max);
}


#endif /* ifdef REVOLUTION */
