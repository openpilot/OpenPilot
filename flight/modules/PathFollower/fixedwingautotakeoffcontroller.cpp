/*
 ******************************************************************************
 *
 * @file       FixedWingAutoTakeoffController.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Fixed wing fly controller implementation
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

#include <pid.h>
#include <sin_lookup.h>
#include <pathdesired.h>
#include <fixedwingpathfollowersettings.h>
#include <flightstatus.h>
#include <pathstatus.h>
#include <stabilizationdesired.h>
#include <velocitystate.h>
#include <positionstate.h>
#include <attitudestate.h>
}

// C++ includes
#include "fixedwingautotakeoffcontroller.h"

// Private constants

// pointer to a singleton instance
FixedWingAutoTakeoffController *FixedWingAutoTakeoffController::p_inst = 0;


// Called when mode first engaged
void FixedWingAutoTakeoffController::Activate(void)
{
    if (!mActive) {
        setState(FW_AUTOTAKEOFF_STATE_LAUNCH);
    }
    FixedWingFlyController::Activate();
}

/**
 * fixed wing autopilot
 * use fixed attitude heading towards destination waypoint
 */
void FixedWingAutoTakeoffController::UpdateAutoPilot()
{
    if (state < FW_AUTOTAKEOFF_STATE_SIZE) {
        (this->*runFunctionTable[state])();
    } else {
        setState(FW_AUTOTAKEOFF_STATE_LAUNCH);
    }
}

/**
 * getAirspeed helper function
 */
float FixedWingAutoTakeoffController::getAirspeed(void)
{
    VelocityStateData v;
    float yaw;

    VelocityStateGet(&v);
    AttitudeStateYawGet(&yaw);

    // current ground speed projected in forward direction
    float groundspeedProjection = v.North * cos_lookup_deg(yaw) + v.East * sin_lookup_deg(yaw);

    // note that airspeedStateBias is ( calibratedAirspeed - groundspeedProjection ) at the time of measurement,
    // but thanks to accelerometers,  groundspeedProjection reacts faster to changes in direction
    // than airspeed and gps sensors alone
    return groundspeedProjection + indicatedAirspeedStateBias;
}


/**
 * setState - state transition including initialization
 */
void FixedWingAutoTakeoffController::setState(FixedWingAutoTakeoffControllerState_T setstate)
{
    if (state < FW_AUTOTAKEOFF_STATE_SIZE && setstate != state) {
        state = setstate;
        (this->*initFunctionTable[state])();
    }
}

/**
 * setAttitude - output function to steer plane
 */
void FixedWingAutoTakeoffController::setAttitude(bool unsafe)
{
    StabilizationDesiredData stabDesired;

    stabDesired.Roll = 0.0f;
    stabDesired.Yaw  = initYaw;
    if (unsafe) {
        stabDesired.Pitch  = fixedWingSettings->LandingPitch;
        stabDesired.Thrust = 0.0f;
    } else {
        stabDesired.Pitch  = fixedWingSettings->TakeOffPitch;
        stabDesired.Thrust = fixedWingSettings->ThrustLimit.Max;
    }
    stabDesired.StabilizationMode.Roll   = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Yaw    = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;

    StabilizationDesiredSet(&stabDesired);
    if (unsafe) {
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_WARNING);
        pathStatus->Status = PATHSTATUS_STATUS_CRITICAL;
    } else {
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);
    }

    // calculate fractional progress based on altitude
    float downPos;
    PositionStateDownGet(&downPos);
    if (fabsf(pathDesired->End.Down - pathDesired->Start.Down) < 1e-3f) {
        pathStatus->fractional_progress = 1.0f;
        pathStatus->error = 0.0f;
    } else {
        pathStatus->fractional_progress = (downPos - pathDesired->Start.Down) / (pathDesired->End.Down - pathDesired->Start.Down);
    }
    pathStatus->error = fabsf(downPos - pathDesired->End.Down);

    PathStatusSet(pathStatus);
}

/**
 * check if situation is unsafe
 */
bool FixedWingAutoTakeoffController::isUnsafe(void)
{
    bool abort  = false;
    float speed = getAirspeed();

    if (speed > maxVelocity) {
        maxVelocity = speed;
    }
    // too much total deceleration (crash, insufficient climbing power, ...)
    if (speed < maxVelocity - fixedWingSettings->SafetyCutoffLimits.MaxDecelerationDeltaMPS) {
        abort = true;
    }
    AttitudeStateData attitude;
    AttitudeStateGet(&attitude);
    // too much bank angle
    if (fabsf(attitude.Roll) > fixedWingSettings->SafetyCutoffLimits.RollDeg) {
        abort = true;
    }
    if (fabsf(attitude.Pitch - fixedWingSettings->TakeOffPitch) > fixedWingSettings->SafetyCutoffLimits.PitchDeg) {
        abort = true;
    }
    float deltayaw = attitude.Yaw - initYaw;
    if (deltayaw > 180.0f) {
        deltayaw -= 360.0f;
    }
    if (deltayaw < -180.0f) {
        deltayaw += 360.0f;
    }
    if (fabsf(deltayaw) > fixedWingSettings->SafetyCutoffLimits.YawDeg) {
        abort = true;
    }
    return abort;
}


// init inactive does nothing
void FixedWingAutoTakeoffController::init_inactive(void) {}

// init launch resets private variables to start values
void FixedWingAutoTakeoffController::init_launch(void)
{
    // find out vector direction of *runway* (if any)
    // and align, otherwise just stay straight ahead
    pathStatus->path_direction_north = 0.0f;
    pathStatus->path_direction_east  = 0.0f;
    pathStatus->path_direction_down  = 0.0f;
    pathStatus->correction_direction_north = 0.0f;
    pathStatus->correction_direction_east  = 0.0f;
    pathStatus->correction_direction_down  = 0.0f;
    if (fabsf(pathDesired->Start.North - pathDesired->End.North) < 1e-3f &&
        fabsf(pathDesired->Start.East - pathDesired->End.East) < 1e-3f) {
        AttitudeStateYawGet(&initYaw);
    } else {
        initYaw = RAD2DEG(atan2f(pathDesired->End.East - pathDesired->Start.East, pathDesired->End.North - pathDesired->Start.North));
        if (initYaw < -180.0f) {
            initYaw += 360.0f;
        }
        if (initYaw > 180.0f) {
            initYaw -= 360.0f;
        }
    }

    maxVelocity = getAirspeed();
}

// init climb does nothing
void FixedWingAutoTakeoffController::init_climb(void) {}

// init hold does nothing
void FixedWingAutoTakeoffController::init_hold(void) {}

// init abort does nothing
void FixedWingAutoTakeoffController::init_abort(void) {}


// run inactive does nothing
// no state transitions
void FixedWingAutoTakeoffController::run_inactive(void) {}

// run launch tries to takeoff - indicates safe situation with engine power (for hand launch)
// run launch checks for:
// 1. min velocity for climb
void FixedWingAutoTakeoffController::run_launch(void)
{
    // state transition
    if (maxVelocity > fixedWingSettings->SafetyCutoffLimits.MaxDecelerationDeltaMPS) {
        setState(FW_AUTOTAKEOFF_STATE_CLIMB);
    }

    setAttitude(isUnsafe());
}

// run climb climbs with max power
// run climb checks for:
// 1. min altitude for hold
// 2. critical situation for abort (different than launch)
void FixedWingAutoTakeoffController::run_climb(void)
{
    bool unsafe = isUnsafe();
    float downPos;

    PositionStateDownGet(&downPos);

    if (unsafe) {
        // state transition 2
        setState(FW_AUTOTAKEOFF_STATE_ABORT);
    } else if (downPos < pathDesired->End.Down) {
        // state transition 1
        setState(FW_AUTOTAKEOFF_STATE_HOLD);
    }

    setAttitude(unsafe);
}

// run hold loiters like in position hold
// no state transitions (FlyController does exception handling)
void FixedWingAutoTakeoffController::run_hold(void)
{
    // parent controller will do perfect position hold in autotakeoff mode
    FixedWingFlyController::UpdateAutoPilot();
}

// run abort descends with wings level, engine off (like land)
// no state transitions
void FixedWingAutoTakeoffController::run_abort(void)
{
    setAttitude(true);
}
