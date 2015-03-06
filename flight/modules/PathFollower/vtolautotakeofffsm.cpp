/*
 ******************************************************************************
 *
 * @file       vtolautotakeofffsm.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      This autotakeoff state machine is a helper state machine to the
 *             VtolAutoTakeoffController.
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

#include <math.h>
#include <pid.h>
#include <CoordinateConversions.h>
#include <sin_lookup.h>
#include <pathdesired.h>
#include <paths.h>
#include "plans.h"
#include <sanitycheck.h>

#include <homelocation.h>
#include <accelstate.h>
#include <fixedwingpathfollowersettings.h>
#include <fixedwingpathfollowerstatus.h>
#include <vtolpathfollowersettings.h>
#include <flightstatus.h>
#include <flightmodesettings.h>
#include <pathstatus.h>
#include <positionstate.h>
#include <velocitystate.h>
#include <velocitydesired.h>
#include <stabilizationdesired.h>
#include <airspeedstate.h>
#include <attitudestate.h>
#include <takeofflocation.h>
#include <poilocation.h>
#include <manualcontrolcommand.h>
#include <systemsettings.h>
#include <stabilizationbank.h>
#include <stabilizationdesired.h>
#include <vtolselftuningstats.h>
#include <fsmlandstatus.h>
#include <pathsummary.h>
}

// C++ includes
#include <vtolautotakeofffsm.h>


// Private constants
#define TIMER_COUNT_PER_SECOND            (1000 / vtolPathFollowerSettings->UpdatePeriod)
#define MIN_AUTOTAKEOFFRATE                      0.1f
#define MAX_AUTOTAKEOFFRATE                      0.6f
#define LOW_ALT_DESCENT_REDUCTION_FACTOR  0.7f  // TODO Need to make the transition smooth
#define AUTOTAKEOFFRATE_LOWLIMIT_FACTOR          0.5f
#define AUTOTAKEOFFRATE_HILIMIT_FACTOR           1.5f
#define TIMEOUT_INIT_ALTHOLD              (3 * TIMER_COUNT_PER_SECOND)
#define TIMEOUT_WTG_FOR_DESCENTRATE       (10 * TIMER_COUNT_PER_SECOND)
#define WTG_FOR_DESCENTRATE_COUNT_LIMIT   10
#define TIMEOUT_AT_DESCENTRATE            10
#define TIMEOUT_GROUNDEFFECT              (1 * TIMER_COUNT_PER_SECOND)
#define TIMEOUT_THRUSTDOWN                (2 * TIMER_COUNT_PER_SECOND)
#define AUTOTAKEOFFING_PID_SCALAR_P              2.0f
#define AUTOTAKEOFFING_PID_SCALAR_I              10.0f
#define AUTOTAKEOFFING_SLOWDOWN_HEIGHT           -5.0f
#define BOUNCE_VELOCITY_TRIGGER_LIMIT     -0.3f
#define BOUNCE_ACCELERATION_TRIGGER_LIMIT -6.0f
#define BOUNCE_TRIGGER_COUNT              4
#define GROUNDEFFECT_SLOWDOWN_FACTOR      0.3f
#define GROUNDEFFECT_SLOWDOWN_COUNT       4

VtolAutoTakeoffFSM::PathFollowerFSM_AutoTakeoffStateHandler_T VtolAutoTakeoffFSM::sAutoTakeoffStateTable[AUTOTAKEOFF_STATE_SIZE] = {
    [AUTOTAKEOFF_STATE_INACTIVE]       =        { .setup = &VtolAutoTakeoffFSM::setup_inactive,             .run = 0                                      },
    [AUTOTAKEOFF_STATE_CHECKSTATE]   =       	{ .setup = &VtolAutoTakeoffFSM::setup_checkstate,         .run = &VtolAutoTakeoffFSM::run_checkstate         },
    [AUTOTAKEOFF_STATE_SLOWSTART] =  		{ .setup = &VtolAutoTakeoffFSM::setup_slowstart,  .run = &VtolAutoTakeoffFSM::run_slowstart  },
    [AUTOTAKEOFF_STATE_THRUSTUP] =       	{ .setup = &VtolAutoTakeoffFSM::setup_thrustup,       .run = &VtolAutoTakeoffFSM::run_thrustup       },
    [AUTOTAKEOFF_STATE_ASCEND] = 		{ .setup = &VtolAutoTakeoffFSM::setup_ascend, .run = &VtolAutoTakeoffFSM::run_ascend },
    [AUTOTAKEOFF_STATE_ALTHOLD]   =    		{ .setup = &VtolAutoTakeoffFSM::setup_althold,         .run = &VtolAutoTakeoffFSM::run_althold         },
    [AUTOTAKEOFF_STATE_THRUSTDOWN]     =       { .setup = &VtolAutoTakeoffFSM::setup_thrustdown,           .run = &VtolAutoTakeoffFSM::run_thrustdown           },
    [AUTOTAKEOFF_STATE_THRUSTOFF]      =       { .setup = &VtolAutoTakeoffFSM::setup_thrustoff,            .run = &VtolAutoTakeoffFSM::run_thrustoff            },
    [AUTOTAKEOFF_STATE_DISARMED]       =       { .setup = &VtolAutoTakeoffFSM::setup_disarmed,             .run = &VtolAutoTakeoffFSM::run_disarmed             },
    [AUTOTAKEOFF_STATE_ABORT] =                { .setup = &VtolAutoTakeoffFSM::setup_abort,                .run = &VtolAutoTakeoffFSM::run_abort                }
};

// pointer to a singleton instance
VtolAutoTakeoffFSM *VtolAutoTakeoffFSM::p_inst = 0;


VtolAutoTakeoffFSM::VtolAutoTakeoffFSM()
    : mAutoTakeoffData(0), vtolPathFollowerSettings(0), pathDesired(0), flightStatus(0)
{}

// Private types

// Private functions
// Public API methods
/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t VtolAutoTakeoffFSM::Initialize(VtolPathFollowerSettingsData *ptr_vtolPathFollowerSettings,
                                PathDesiredData *ptr_pathDesired,
                                FlightStatusData *ptr_flightStatus)
{
    PIOS_Assert(ptr_vtolPathFollowerSettings);
    PIOS_Assert(ptr_pathDesired);
    PIOS_Assert(ptr_flightStatus);

    HomeLocationInitialize();
    AccelStateInitialize();
    VtolPathFollowerSettingsInitialize();
    FlightStatusInitialize();
    FlightModeSettingsInitialize();
    PathStatusInitialize();
    PathSummaryInitialize();
    PathDesiredInitialize();
    PositionStateInitialize();
    VelocityStateInitialize();
    VelocityDesiredInitialize();
    StabilizationDesiredInitialize();
    TakeOffLocationInitialize();
    ManualControlCommandInitialize();
    StatusVtolAutoTakeoffInitialize();

    mAutoTakeoffData = (VtolAutoTakeoffFSMData_T *)pios_malloc(sizeof(VtolAutoTakeoffFSMData_T));
    PIOS_Assert(mAutoTakeoffData);
    memset(mAutoTakeoffData, sizeof(VtolAutoTakeoffFSMData_T), 0);
    vtolPathFollowerSettings = ptr_vtolPathFollowerSettings;
    pathDesired  = ptr_pathDesired;
    flightStatus = ptr_flightStatus;
    initFSM();

    return 0;
}

void VtolAutoTakeoffFSM::Inactive(void)
{
    memset(mAutoTakeoffData, sizeof(VtolAutoTakeoffFSMData_T), 0);
    initFSM();
}

// Initialise the FSM
void VtolAutoTakeoffFSM::initFSM(void)
{
    if (vtolPathFollowerSettings != 0) {
        setState(AUTOTAKEOFF_STATE_INACTIVE, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_NONE);
    } else {
        mAutoTakeoffData->currentState = AUTOTAKEOFF_STATE_INACTIVE;
    }
}

void VtolAutoTakeoffFSM::Activate()
{
    memset(mAutoTakeoffData, sizeof(VtolAutoTakeoffFSMData_T), 0);
    mAutoTakeoffData->currentState   = AUTOTAKEOFF_STATE_INACTIVE;
#if 0
    mAutoTakeoffData->flLowAltitude  = false;
    mAutoTakeoffData->flAltitudeHold = false;
    mAutoTakeoffData->fsmAutoTakeoffStatus.averageDescentRate      = MIN_AUTOTAKEOFFRATE;
    mAutoTakeoffData->fsmAutoTakeoffStatus.averageDescentThrust    = vtolPathFollowerSettings->ThrustLimits.Neutral;
    mAutoTakeoffData->fsmAutoTakeoffStatus.calculatedNeutralThrust = vtolPathFollowerSettings->ThrustLimits.Neutral;
#endif
    mAutoTakeoffData->boundThrustMin = vtolPathFollowerSettings->ThrustLimits.Min;
    mAutoTakeoffData->boundThrustMax = vtolPathFollowerSettings->ThrustLimits.Max;
    TakeOffLocationGet(&(mAutoTakeoffData->takeOffLocation));
    mAutoTakeoffData->fsmAutoTakeoffStatus.AltitudeAtState[AUTOTAKEOFF_STATE_INACTIVE] = 0.0f;
    //assessAltitude();

    if (pathDesired->Mode == PATHDESIRED_MODE_AUTOTAKEOFF) {
        setState(AUTOTAKEOFF_STATE_CHECKSTATE, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_NONE);
    } else {
        // move to error state and callback to position hold
        setState(AUTOTAKEOFF_STATE_ABORT, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_NONE);
    }
}

void VtolAutoTakeoffFSM::Abort(void)
{
    setState(AUTOTAKEOFF_STATE_ABORT, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_NONE);
}

PathFollowerFSMState_T VtolAutoTakeoffFSM::GetCurrentState(void)
{
    switch (mAutoTakeoffData->currentState) {
    case AUTOTAKEOFF_STATE_INACTIVE:
        return PFFSM_STATE_INACTIVE;

        break;
    case AUTOTAKEOFF_STATE_ABORT:
        return PFFSM_STATE_ABORT;

        break;
    case AUTOTAKEOFF_STATE_DISARMED:
        return PFFSM_STATE_DISARMED;

        break;
    default:
        return PFFSM_STATE_ACTIVE;

        break;
    }
}

void VtolAutoTakeoffFSM::Update()
{
    runState();
    if (GetCurrentState() != PFFSM_STATE_INACTIVE) {
        runAlways();
    }
}

int32_t VtolAutoTakeoffFSM::runState(void)
{
    uint8_t flTimeout = false;

    mAutoTakeoffData->stateRunCount++;

    if (mAutoTakeoffData->stateTimeoutCount > 0 && mAutoTakeoffData->stateRunCount > mAutoTakeoffData->stateTimeoutCount) {
        flTimeout = true;
    }

    // If the current state has a static function, call it
    if (sAutoTakeoffStateTable[mAutoTakeoffData->currentState].run) {
        (this->*sAutoTakeoffStateTable[mAutoTakeoffData->currentState].run)(flTimeout);
    }
    return 0;
}

int32_t VtolAutoTakeoffFSM::runAlways(void)
{
    //void assessAltitude(void);

    return 0;
}

// PathFollower implements the PID scheme and has a objective
// set by a PathDesired object.  Based on the mode, pathfollower
// uses FSM's as helper functions that manage state and event detection.
// PathFollower calls into FSM methods to alter its commands.

void VtolAutoTakeoffFSM::BoundThrust(float &ulow, float &uhigh)
{
    ulow  = mAutoTakeoffData->boundThrustMin;
    uhigh = mAutoTakeoffData->boundThrustMax;


    if (mAutoTakeoffData->flConstrainThrust) {
        uhigh = mAutoTakeoffData->thrustLimit;
    }
}

void VtolAutoTakeoffFSM::ConstrainStabiDesired(StabilizationDesiredData *stabDesired)
{
    if (mAutoTakeoffData->flZeroStabiHorizontal && stabDesired) {
        stabDesired->Pitch = 0.0f;
        stabDesired->Roll  = 0.0f;
        stabDesired->Yaw   = 0.0f;
    }
}

// Set the new state and perform setup for subsequent state run calls
// This is called by state run functions on event detection that drive
// state transitions.
void VtolAutoTakeoffFSM::setState(PathFollowerFSM_AutoTakeoffState_T newState, StatusVtolAutoTakeoffStateExitReasonOptions reason)
{
    mAutoTakeoffData->fsmAutoTakeoffStatus.StateExitReason[mAutoTakeoffData->currentState] = reason;

    if (mAutoTakeoffData->currentState == newState) {
        return;
    }
    mAutoTakeoffData->currentState = newState;

    if (newState != AUTOTAKEOFF_STATE_INACTIVE) {
        PositionStateData positionState;
        PositionStateGet(&positionState);
        float takeOffDown = 0.0f;
        if (mAutoTakeoffData->takeOffLocation.Status == TAKEOFFLOCATION_STATUS_VALID) {
            takeOffDown = mAutoTakeoffData->takeOffLocation.Down;
        }
        mAutoTakeoffData->fsmAutoTakeoffStatus.AltitudeAtState[newState] = positionState.Down - takeOffDown;
        assessAltitude();
    }

    // Restart state timer counter
    mAutoTakeoffData->stateRunCount     = 0;

    // Reset state timeout to disabled/zero
    mAutoTakeoffData->stateTimeoutCount = 0;

    if (sAutoTakeoffStateTable[mAutoTakeoffData->currentState].setup) {
        (this->*sAutoTakeoffStateTable[mAutoTakeoffData->currentState].setup)();
    }

    updateVtolAutoTakeoffFSMStatus();
}


// Timeout utility function for use by state init implementations
void VtolAutoTakeoffFSM::setStateTimeout(int32_t count)
{
    mAutoTakeoffData->stateTimeoutCount = count;
}

void VtolAutoTakeoffFSM::updateVtolAutoTakeoffFSMStatus()
{
    mAutoTakeoffData->fsmAutoTakeoffStatus.State = mAutoTakeoffData->currentState;
    if (mAutoTakeoffData->flLowAltitude) {
        mAutoTakeoffData->fsmAutoTakeoffStatus.AltitudeState = STATUSVTOLAUTOTAKEOFF_ALTITUDESTATE_LOW;
    } else {
        mAutoTakeoffData->fsmAutoTakeoffStatus.AltitudeState = STATUSVTOLAUTOTAKEOFF_ALTITUDESTATE_HIGH;
    }
    StatusVtolAutoTakeoffSet(&mAutoTakeoffData->fsmAutoTakeoffStatus);
}


float VtolAutoTakeoffFSM::BoundVelocityDown(float velocity_down)
{
    velocity_down = boundf(velocity_down, MIN_AUTOTAKEOFFRATE, MAX_AUTOTAKEOFFRATE);
    if (mAutoTakeoffData->flLowAltitude) {
        velocity_down *= LOW_ALT_DESCENT_REDUCTION_FACTOR;
    }
    mAutoTakeoffData->fsmAutoTakeoffStatus.targetDescentRate = velocity_down;

    if (mAutoTakeoffData->flAltitudeHold) {
        return 0.0f;
    } else {
        return velocity_down;
    }
}

void VtolAutoTakeoffFSM::assessAltitude(void)
{
    float positionDown;

    PositionStateDownGet(&positionDown);
    float takeOffDown = 0.0f;
    if (mAutoTakeoffData->takeOffLocation.Status == TAKEOFFLOCATION_STATUS_VALID) {
        takeOffDown = mAutoTakeoffData->takeOffLocation.Down;
    }
    float positionDownRelativeToTakeoff = positionDown - takeOffDown;
    if (positionDownRelativeToTakeoff < AUTOTAKEOFFING_SLOWDOWN_HEIGHT) {
        mAutoTakeoffData->flLowAltitude = false;
    } else {
        mAutoTakeoffData->flLowAltitude = true;
    }
}


// State: INACTIVE
void VtolAutoTakeoffFSM::setup_inactive(void)
{
    // Re-initialise local variables
    mAutoTakeoffData->flZeroStabiHorizontal = false;
    mAutoTakeoffData->flConstrainThrust     = false;
}

// State: CHECKSTATE
void VtolAutoTakeoffFSM::setup_checkstate(void)
{
    // Assumptions that do not need to be checked if flight mode is AUTOTAKEOFF
    // 1. Already armed
    // 2. Not in flight. This was checked in plans.c
    // 3. User has placed throttle position to more than 50% to allow autotakeoff

    // If pathplanner, we may need additional checks???
    // E.g. if inflight, this mode is just positon hold??

    // Start from a enforced thrust off condition
    mAutoTakeoffData->thrustLimit       = -1.0f;
    mAutoTakeoffData->flConstrainThrust = true;
    mAutoTakeoffData->boundThrustMin    = -0.1f;
    mAutoTakeoffData->boundThrustMax    = 0.0f;

    setState(AUTOTAKEOFF_STATE_SLOWSTART, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_TIMEOUT);
}

// STATE: SLOWSTART
void VtolAutoTakeoffFSM::setup_slowstart(void)
{
    setStateTimeout(TIMEOUT_SLOWSTART);
    mAutoTakeoffData->flZeroStabiHorizontal = true;
    mAutoTakeoffData->flConstrainThrust     = true;
    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);
    mAutoTakeoffData->thrustLimit    = 0.0f;
    mAutoTakeoffData->sum1 = 0.2f / (float)TIMEOUT_SLOWSTART;
    mAutoTakeoffData->boundThrustMin = 0.05f;
    mAutoTakeoffData->boundThrustMax = 0.05f;
}

void VtolAutoTakeoffFSM::run_slowstart(__attribute__((unused)) uint8_t flTimeout)
{
    // increase thrust setpoint step by step
    mAutoTakeoffData->thrustLimit += mAutoTakeoffData->sum1;

    if (flTimeout) {
        setState(AUTOTAKEOFF_STATE_THRUSTUP, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_TIMEOUT);
    }
}



// State: WAITING FOR DESCENT RATE
void VtolAutoTakeoffFSM::setup_wtg_for_descentrate(void)
{
    setStateTimeout(TIMEOUT_WTG_FOR_DESCENTRATE);
    // get target descent velocity
    mAutoTakeoffData->flZeroStabiHorizontal = false;
    mAutoTakeoffData->observationCount = 0;
    mAutoTakeoffData->observation2Count     = 0;
    mAutoTakeoffData->flConstrainThrust     = false;
    mAutoTakeoffData->flAltitudeHold = false;
    mAutoTakeoffData->boundThrustMin = vtolPathFollowerSettings->ThrustLimits.Min;
    mAutoTakeoffData->boundThrustMax = vtolPathFollowerSettings->ThrustLimits.Max;
}

void VtolAutoTakeoffFSM::run_wtg_for_descentrate(uint8_t flTimeout)
{
    // Look at current actual thrust...are we already shutdown??
    VelocityStateData velocityState;

    VelocityStateGet(&velocityState);
    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);

    // We don't expect PID to get exactly the target descent rate, so have a lower
    // water mark but need to see 5 observations to be confident that we have semi-stable
    // descent achieved

    // we need to see velocity down within a range of control before we proceed, without which we
    // really don't have confidence to allow later states to run.
    if (velocityState.Down > (AUTOTAKEOFFRATE_LOWLIMIT_FACTOR * mAutoTakeoffData->fsmAutoTakeoffStatus.targetDescentRate) &&
        velocityState.Down < (AUTOTAKEOFFRATE_HILIMIT_FACTOR * mAutoTakeoffData->fsmAutoTakeoffStatus.targetDescentRate)) {
        if (mAutoTakeoffData->observationCount++ > WTG_FOR_DESCENTRATE_COUNT_LIMIT) {
            setState(AUTOTAKEOFF_STATE_AT_DESCENTRATE, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_DESCENTRATEOK);
            return;
        }
    }

    if (flTimeout) {
        setState(AUTOTAKEOFF_STATE_ABORT, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_TIMEOUT);
    }
}


// State: AT DESCENT RATE
void VtolAutoTakeoffFSM::setup_at_descentrate(void)
{
    setStateTimeout(TIMEOUT_AT_DESCENTRATE);
    mAutoTakeoffData->flZeroStabiHorizontal = false;
    mAutoTakeoffData->observationCount  = 0;
    mAutoTakeoffData->sum1 = 0.0f;
    mAutoTakeoffData->sum2 = 0.0f;
    mAutoTakeoffData->flConstrainThrust = false;
    mAutoTakeoffData->fsmAutoTakeoffStatus.averageDescentRate = MIN_AUTOTAKEOFFRATE;
    mAutoTakeoffData->fsmAutoTakeoffStatus.averageDescentThrust = vtolPathFollowerSettings->ThrustLimits.Neutral;
    mAutoTakeoffData->boundThrustMin    = vtolPathFollowerSettings->ThrustLimits.Min;
    mAutoTakeoffData->boundThrustMax    = vtolPathFollowerSettings->ThrustLimits.Max;
}

void VtolAutoTakeoffFSM::run_at_descentrate(uint8_t flTimeout)
{
    VelocityStateData velocityState;

    VelocityStateGet(&velocityState);

    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);

    mAutoTakeoffData->sum1 += velocityState.Down;
    mAutoTakeoffData->sum2 += stabDesired.Thrust;
    mAutoTakeoffData->observationCount++;
    if (flTimeout) {
        mAutoTakeoffData->fsmAutoTakeoffStatus.averageDescentRate   = boundf((mAutoTakeoffData->sum1 / (float)(mAutoTakeoffData->observationCount)), 0.5f * MIN_AUTOTAKEOFFRATE, 1.5f * MAX_AUTOTAKEOFFRATE);
        mAutoTakeoffData->fsmAutoTakeoffStatus.averageDescentThrust = boundf((mAutoTakeoffData->sum2 / (float)(mAutoTakeoffData->observationCount)), vtolPathFollowerSettings->ThrustLimits.Min, vtolPathFollowerSettings->ThrustLimits.Max);

        // We need to calculate a neutral limit to use later to constrain upper thrust range during states where we are close to the ground
        // As our battery gets flat, ThrustLimits.Neutral needs to constrain us too much and we get too fast a descent rate. We can
        // detect this by the fact that the descent rate will exceed the target and the required thrust will exceed the neutral value
        mAutoTakeoffData->fsmAutoTakeoffStatus.calculatedNeutralThrust = mAutoTakeoffData->fsmAutoTakeoffStatus.averageDescentRate / mAutoTakeoffData->fsmAutoTakeoffStatus.targetDescentRate * mAutoTakeoffData->fsmAutoTakeoffStatus.averageDescentThrust;
        mAutoTakeoffData->fsmAutoTakeoffStatus.calculatedNeutralThrust = boundf(mAutoTakeoffData->fsmAutoTakeoffStatus.calculatedNeutralThrust, vtolPathFollowerSettings->ThrustLimits.Neutral, vtolPathFollowerSettings->ThrustLimits.Max);


        setState(AUTOTAKEOFF_STATE_WTG_FOR_GROUNDEFFECT, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_DESCENTRATEOK);
    }
}


// State: WAITING FOR GROUND EFFECT
void VtolAutoTakeoffFSM::setup_wtg_for_groundeffect(void)
{
    // No timeout
    mAutoTakeoffData->flZeroStabiHorizontal = false;
    mAutoTakeoffData->observationCount = 0;
    mAutoTakeoffData->observation2Count     = 0;
    mAutoTakeoffData->sum1 = 0.0f;
    mAutoTakeoffData->sum2 = 0.0f;
    mAutoTakeoffData->flConstrainThrust     = false;
    mAutoTakeoffData->fsmAutoTakeoffStatus.WtgForGroundEffect.BounceVelocity = 0.0f;
    mAutoTakeoffData->fsmAutoTakeoffStatus.WtgForGroundEffect.BounceAccel    = 0.0f;
    mAutoTakeoffData->boundThrustMin = vtolPathFollowerSettings->ThrustLimits.Min;
    mAutoTakeoffData->boundThrustMax = vtolPathFollowerSettings->ThrustLimits.Max;
}

void VtolAutoTakeoffFSM::run_wtg_for_groundeffect(__attribute__((unused)) uint8_t flTimeout)
{
    // detect material downrating in thrust for 1 second.
    VelocityStateData velocityState;

    VelocityStateGet(&velocityState);
    AccelStateData accelState;
    AccelStateGet(&accelState);

    // +ve 9.8 expected
    float g_e;
    HomeLocationg_eGet(&g_e);

    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);

    // detect bounce
    uint8_t flBounce = (velocityState.Down < BOUNCE_VELOCITY_TRIGGER_LIMIT);
    if (flBounce) {
        mAutoTakeoffData->fsmAutoTakeoffStatus.WtgForGroundEffect.BounceVelocity = velocityState.Down;
    } else {
        mAutoTakeoffData->fsmAutoTakeoffStatus.WtgForGroundEffect.BounceVelocity = 0.0f;
    }

    // invert sign of accel to the standard convention of down is +ve and subtract the gravity to get
    // a relative acceleration term.
    float bounceAccel     = -accelState.z - g_e;
    uint8_t flBounceAccel = (bounceAccel < BOUNCE_ACCELERATION_TRIGGER_LIMIT);
    if (flBounceAccel) {
        mAutoTakeoffData->fsmAutoTakeoffStatus.WtgForGroundEffect.BounceAccel = bounceAccel;
    } else {
        mAutoTakeoffData->fsmAutoTakeoffStatus.WtgForGroundEffect.BounceAccel = 0.0f;
    }

    if (flBounce || flBounceAccel) {
        mAutoTakeoffData->observation2Count++;
        if (mAutoTakeoffData->observation2Count > BOUNCE_TRIGGER_COUNT) {
            setState(AUTOTAKEOFF_STATE_GROUNDEFFECT, (flBounce ? STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_BOUNCEVELOCITY : STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_BOUNCEACCEL));
            return;
        }
    } else {
        mAutoTakeoffData->observation2Count = 0;
    }

    // detect low descent rate
    uint8_t flDescentRateLow = (velocityState.Down < (GROUNDEFFECT_SLOWDOWN_FACTOR * mAutoTakeoffData->fsmAutoTakeoffStatus.averageDescentRate));
    if (flDescentRateLow) {
        mAutoTakeoffData->boundThrustMax = mAutoTakeoffData->fsmAutoTakeoffStatus.calculatedNeutralThrust;
        mAutoTakeoffData->observationCount++;
        if (mAutoTakeoffData->observationCount > GROUNDEFFECT_SLOWDOWN_COUNT) {
#ifndef DEBUG_GROUNDIMPACT
            setState(AUTOTAKEOFF_STATE_GROUNDEFFECT, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_LOWDESCENTRATE);
#endif
            return;
        }
    } else {
        mAutoTakeoffData->observationCount = 0;
    }

    updateVtolAutoTakeoffFSMStatus();
}

// STATE: GROUNDEFFET
void VtolAutoTakeoffFSM::setup_groundeffect(void)
{
    setStateTimeout(TIMEOUT_GROUNDEFFECT);
    mAutoTakeoffData->flZeroStabiHorizontal     = true;
    PositionStateData positionState;
    PositionStateGet(&positionState);
    mAutoTakeoffData->expectedAutoTakeoffPositionNorth = positionState.North;
    mAutoTakeoffData->expectedAutoTakeoffPositionEast  = positionState.East;
    mAutoTakeoffData->flConstrainThrust = false;

    // now that we have ground effect limit max thrust to neutral
    mAutoTakeoffData->boundThrustMin    = -0.1f;
    mAutoTakeoffData->boundThrustMax    = mAutoTakeoffData->fsmAutoTakeoffStatus.calculatedNeutralThrust;
}
void VtolAutoTakeoffFSM::run_groundeffect(__attribute__((unused)) uint8_t flTimeout)
{
    StabilizationDesiredData stabDesired;

    StabilizationDesiredGet(&stabDesired);
    if (stabDesired.Thrust < 0.0f) {
        setState(AUTOTAKEOFF_STATE_THRUSTOFF, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_ZEROTHRUST);
        return;
    }

    // Stay in this state until we get a low altitude flag.
    if (mAutoTakeoffData->flLowAltitude == false) {
        // worst case scenario is that we land and the pid brings thrust down to zero.
        return;
    }

    // detect broad sideways drift.  If for some reason we have a hard landing that the bounce detection misses, this will kick in
    PositionStateData positionState;
    PositionStateGet(&positionState);
    float north_error   = mAutoTakeoffData->expectedAutoTakeoffPositionNorth - positionState.North;
    float east_error    = mAutoTakeoffData->expectedAutoTakeoffPositionEast - positionState.East;
    float positionError = sqrtf(north_error * north_error + east_error * east_error);
    if (positionError > 0.3f) {
        setState(AUTOTAKEOFF_STATE_THRUSTDOWN, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_POSITIONERROR);
        return;
    }

    if (flTimeout) {
        setState(AUTOTAKEOFF_STATE_THRUSTDOWN, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_TIMEOUT);
    }
}

// STATE: THRUSTDOWN
void VtolAutoTakeoffFSM::setup_thrustdown(void)
{
    setStateTimeout(TIMEOUT_THRUSTDOWN);
    mAutoTakeoffData->flZeroStabiHorizontal = true;
    mAutoTakeoffData->flConstrainThrust     = true;
    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);
    mAutoTakeoffData->thrustLimit    = stabDesired.Thrust;
    mAutoTakeoffData->sum1 = stabDesired.Thrust / (float)TIMEOUT_THRUSTDOWN;
    mAutoTakeoffData->boundThrustMin = -0.1f;
    mAutoTakeoffData->boundThrustMax = mAutoTakeoffData->fsmAutoTakeoffStatus.calculatedNeutralThrust;
}

void VtolAutoTakeoffFSM::run_thrustdown(__attribute__((unused)) uint8_t flTimeout)
{
    // reduce thrust setpoint step by step
    mAutoTakeoffData->thrustLimit -= mAutoTakeoffData->sum1;

    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);
    if (stabDesired.Thrust < 0.0f || mAutoTakeoffData->thrustLimit < 0.0f) {
        setState(AUTOTAKEOFF_STATE_THRUSTOFF, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_ZEROTHRUST);
    }

    if (flTimeout) {
        setState(AUTOTAKEOFF_STATE_THRUSTOFF, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_TIMEOUT);
    }
}

// STATE: THRUSTOFF
void VtolAutoTakeoffFSM::setup_thrustoff(void)
{
    mAutoTakeoffData->thrustLimit       = -1.0f;
    mAutoTakeoffData->flConstrainThrust = true;
    mAutoTakeoffData->boundThrustMin    = -0.1f;
    mAutoTakeoffData->boundThrustMax    = 0.0f;
}

void VtolAutoTakeoffFSM::run_thrustoff(__attribute__((unused)) uint8_t flTimeout)
{
    setState(AUTOTAKEOFF_STATE_DISARMED, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_NONE);
}

// STATE: DISARMED
void VtolAutoTakeoffFSM::setup_disarmed(void)
{
    // nothing to do
    mAutoTakeoffData->flConstrainThrust     = false;
    mAutoTakeoffData->flZeroStabiHorizontal = false;
    mAutoTakeoffData->observationCount = 0;
    mAutoTakeoffData->boundThrustMin   = -0.1f;
    mAutoTakeoffData->boundThrustMax   = 0.0f;
}

void VtolAutoTakeoffFSM::run_disarmed(__attribute__((unused)) uint8_t flTimeout)
{
#ifdef DEBUG_GROUNDIMPACT
    if (mAutoTakeoffData->observationCount++ > 100) {
        setState(AUTOTAKEOFF_STATE_WTG_FOR_GROUNDEFFECT, STATUSVTOLAUTOTAKEOFF_STATEEXITREASON_NONE);
    }
#endif
}

void VtolAutoTakeoffFSM::fallback_to_hold(void)
{
    PositionStateData positionState;

    PositionStateGet(&positionState);
    pathDesired->End.North        = positionState.North;
    pathDesired->End.East         = positionState.East;
    pathDesired->End.Down         = positionState.Down;
    pathDesired->Start.North      = positionState.North;
    pathDesired->Start.East       = positionState.East;
    pathDesired->Start.Down       = positionState.Down;
    pathDesired->StartingVelocity = 0.0f;
    pathDesired->EndingVelocity   = 0.0f;
    pathDesired->Mode = PATHDESIRED_MODE_GOTOENDPOINT;

    PathDesiredSet(pathDesired);
}

// abort repeatedly overwrites pathfollower's objective on a landing abort and
// continues to do so until a flight mode change.
void VtolAutoTakeoffFSM::setup_abort(void)
{
    mAutoTakeoffData->boundThrustMin        = vtolPathFollowerSettings->ThrustLimits.Min;
    mAutoTakeoffData->boundThrustMax        = vtolPathFollowerSettings->ThrustLimits.Max;
    mAutoTakeoffData->flConstrainThrust     = false;
    mAutoTakeoffData->flZeroStabiHorizontal = false;
    fallback_to_hold();
}

void VtolAutoTakeoffFSM::run_abort(__attribute__((unused)) uint8_t flTimeout)
{}
