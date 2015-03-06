/*
 ******************************************************************************
 *
 * @file       vtolautotakeofffsm.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      This autotakeoff state machine is a helper state machine to the
 *             VtolAutoTakeOffController.
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

VtolAutoTakeOffFSM::PathFollowerFSM_AutoTakeOffStateHandler_T VtolAutoTakeOffFSM::sAutoTakeOffStateTable[AUTOTAKEOFF_STATE_SIZE] = {
    [AUTOTAKEOFF_STATE_INACTIVE]       =       { .setup = &VtolAutoTakeOffFSM::setup_inactive,             .run = 0                                      },
    [AUTOTAKEOFF_STATE_INIT_ALTHOLD]   =       { .setup = &VtolAutoTakeOffFSM::setup_init_althold,         .run = &VtolAutoTakeOffFSM::run_init_althold         },
    [AUTOTAKEOFF_STATE_WTG_FOR_DESCENTRATE] =  { .setup = &VtolAutoTakeOffFSM::setup_wtg_for_descentrate,  .run = &VtolAutoTakeOffFSM::run_wtg_for_descentrate  },
    [AUTOTAKEOFF_STATE_AT_DESCENTRATE] =       { .setup = &VtolAutoTakeOffFSM::setup_at_descentrate,       .run = &VtolAutoTakeOffFSM::run_at_descentrate       },
    [AUTOTAKEOFF_STATE_WTG_FOR_GROUNDEFFECT] = { .setup = &VtolAutoTakeOffFSM::setup_wtg_for_groundeffect, .run = &VtolAutoTakeOffFSM::run_wtg_for_groundeffect },
    [AUTOTAKEOFF_STATE_GROUNDEFFECT]   =       { .setup = &VtolAutoTakeOffFSM::setup_groundeffect,         .run = &VtolAutoTakeOffFSM::run_groundeffect         },
    [AUTOTAKEOFF_STATE_THRUSTDOWN]     =       { .setup = &VtolAutoTakeOffFSM::setup_thrustdown,           .run = &VtolAutoTakeOffFSM::run_thrustdown           },
    [AUTOTAKEOFF_STATE_THRUSTOFF]      =       { .setup = &VtolAutoTakeOffFSM::setup_thrustoff,            .run = &VtolAutoTakeOffFSM::run_thrustoff            },
    [AUTOTAKEOFF_STATE_DISARMED]       =       { .setup = &VtolAutoTakeOffFSM::setup_disarmed,             .run = &VtolAutoTakeOffFSM::run_disarmed             },
    [AUTOTAKEOFF_STATE_ABORT] =                { .setup = &VtolAutoTakeOffFSM::setup_abort,                .run = &VtolAutoTakeOffFSM::run_abort                }
};

// pointer to a singleton instance
VtolAutoTakeOffFSM *VtolAutoTakeOffFSM::p_inst = 0;


VtolAutoTakeOffFSM::VtolAutoTakeOffFSM()
    : mAutoTakeOffData(0), vtolPathFollowerSettings(0), pathDesired(0), flightStatus(0)
{}

// Private types

// Private functions
// Public API methods
/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t VtolAutoTakeOffFSM::Initialize(VtolPathFollowerSettingsData *ptr_vtolPathFollowerSettings,
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
    FSMAutoTakeOffStatusInitialize();

    mAutoTakeOffData = (VtolAutoTakeOffFSMData_T *)pios_malloc(sizeof(VtolAutoTakeOffFSMData_T));
    PIOS_Assert(mAutoTakeOffData);
    memset(mAutoTakeOffData, sizeof(VtolAutoTakeOffFSMData_T), 0);
    vtolPathFollowerSettings = ptr_vtolPathFollowerSettings;
    pathDesired  = ptr_pathDesired;
    flightStatus = ptr_flightStatus;
    initFSM();

    return 0;
}

void VtolAutoTakeOffFSM::Inactive(void)
{
    memset(mAutoTakeOffData, sizeof(VtolAutoTakeOffFSMData_T), 0);
    initFSM();
}

// Initialise the FSM
void VtolAutoTakeOffFSM::initFSM(void)
{
    if (vtolPathFollowerSettings != 0) {
        setState(AUTOTAKEOFF_STATE_INACTIVE, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_NONE);
    } else {
        mAutoTakeOffData->currentState = AUTOTAKEOFF_STATE_INACTIVE;
    }
}

void VtolAutoTakeOffFSM::Activate()
{
    memset(mAutoTakeOffData, sizeof(VtolAutoTakeOffFSMData_T), 0);
    mAutoTakeOffData->currentState   = AUTOTAKEOFF_STATE_INACTIVE;
    mAutoTakeOffData->flLowAltitude  = false;
    mAutoTakeOffData->flAltitudeHold = false;
    mAutoTakeOffData->fsmAutoTakeOffStatus.averageDescentRate      = MIN_AUTOTAKEOFFRATE;
    mAutoTakeOffData->fsmAutoTakeOffStatus.averageDescentThrust    = vtolPathFollowerSettings->ThrustLimits.Neutral;
    mAutoTakeOffData->fsmAutoTakeOffStatus.calculatedNeutralThrust = vtolPathFollowerSettings->ThrustLimits.Neutral;
    mAutoTakeOffData->boundThrustMin = vtolPathFollowerSettings->ThrustLimits.Min;
    mAutoTakeOffData->boundThrustMax = vtolPathFollowerSettings->ThrustLimits.Max;
    TakeOffLocationGet(&(mAutoTakeOffData->takeOffLocation));
    mAutoTakeOffData->fsmAutoTakeOffStatus.AltitudeAtState[AUTOTAKEOFF_STATE_INACTIVE] = 0.0f;
    assessAltitude();

    if (pathDesired->Mode == PATHDESIRED_MODE_AUTOTAKEOFF) {
#ifndef DEBUG_GROUNDIMPACT
        setState(AUTOTAKEOFF_STATE_INIT_ALTHOLD, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_NONE);
#else
        setState(AUTOTAKEOFF_STATE_WTG_FOR_GROUNDEFFECT, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_NONE);
#endif
    } else {
        // move to error state and callback to position hold
        setState(AUTOTAKEOFF_STATE_ABORT, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_NONE);
    }
}

void VtolAutoTakeOffFSM::Abort(void)
{
    setState(AUTOTAKEOFF_STATE_ABORT, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_NONE);
}

PathFollowerFSMState_T VtolAutoTakeOffFSM::GetCurrentState(void)
{
    switch (mAutoTakeOffData->currentState) {
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

void VtolAutoTakeOffFSM::Update()
{
    runState();
    if (GetCurrentState() != PFFSM_STATE_INACTIVE) {
        runAlways();
    }
}

int32_t VtolAutoTakeOffFSM::runState(void)
{
    uint8_t flTimeout = false;

    mAutoTakeOffData->stateRunCount++;

    if (mAutoTakeOffData->stateTimeoutCount > 0 && mAutoTakeOffData->stateRunCount > mAutoTakeOffData->stateTimeoutCount) {
        flTimeout = true;
    }

    // If the current state has a static function, call it
    if (sAutoTakeOffStateTable[mAutoTakeOffData->currentState].run) {
        (this->*sAutoTakeOffStateTable[mAutoTakeOffData->currentState].run)(flTimeout);
    }
    return 0;
}

int32_t VtolAutoTakeOffFSM::runAlways(void)
{
    void assessAltitude(void);

    return 0;
}

// PathFollower implements the PID scheme and has a objective
// set by a PathDesired object.  Based on the mode, pathfollower
// uses FSM's as helper functions that manage state and event detection.
// PathFollower calls into FSM methods to alter its commands.

void VtolAutoTakeOffFSM::BoundThrust(float &ulow, float &uhigh)
{
    ulow  = mAutoTakeOffData->boundThrustMin;
    uhigh = mAutoTakeOffData->boundThrustMax;


    if (mAutoTakeOffData->flConstrainThrust) {
        uhigh = mAutoTakeOffData->thrustLimit;
    }
}

void VtolAutoTakeOffFSM::ConstrainStabiDesired(StabilizationDesiredData *stabDesired)
{
    if (mAutoTakeOffData->flZeroStabiHorizontal && stabDesired) {
        stabDesired->Pitch = 0.0f;
        stabDesired->Roll  = 0.0f;
        stabDesired->Yaw   = 0.0f;
    }
}

void VtolAutoTakeOffFSM::CheckPidScaler(pid_scaler *local_scaler)
{
    if (mAutoTakeOffData->flLowAltitude) {
        local_scaler->p = AUTOTAKEOFFING_PID_SCALAR_P;
        local_scaler->i = AUTOTAKEOFFING_PID_SCALAR_I;
    }
}


// Set the new state and perform setup for subsequent state run calls
// This is called by state run functions on event detection that drive
// state transitions.
void VtolAutoTakeOffFSM::setState(PathFollowerFSM_AutoTakeOffState_T newState, FSMAutoTakeOffStatusStateExitReasonOptions reason)
{
    mAutoTakeOffData->fsmAutoTakeOffStatus.StateExitReason[mAutoTakeOffData->currentState] = reason;

    if (mAutoTakeOffData->currentState == newState) {
        return;
    }
    mAutoTakeOffData->currentState = newState;

    if (newState != AUTOTAKEOFF_STATE_INACTIVE) {
        PositionStateData positionState;
        PositionStateGet(&positionState);
        float takeOffDown = 0.0f;
        if (mAutoTakeOffData->takeOffLocation.Status == TAKEOFFLOCATION_STATUS_VALID) {
            takeOffDown = mAutoTakeOffData->takeOffLocation.Down;
        }
        mAutoTakeOffData->fsmAutoTakeOffStatus.AltitudeAtState[newState] = positionState.Down - takeOffDown;
        assessAltitude();
    }

    // Restart state timer counter
    mAutoTakeOffData->stateRunCount     = 0;

    // Reset state timeout to disabled/zero
    mAutoTakeOffData->stateTimeoutCount = 0;

    if (sAutoTakeOffStateTable[mAutoTakeOffData->currentState].setup) {
        (this->*sAutoTakeOffStateTable[mAutoTakeOffData->currentState].setup)();
    }

    updateVtolAutoTakeOffFSMStatus();
}


// Timeout utility function for use by state init implementations
void VtolAutoTakeOffFSM::setStateTimeout(int32_t count)
{
    mAutoTakeOffData->stateTimeoutCount = count;
}

void VtolAutoTakeOffFSM::updateVtolAutoTakeOffFSMStatus()
{
    mAutoTakeOffData->fsmAutoTakeOffStatus.State = mAutoTakeOffData->currentState;
    if (mAutoTakeOffData->flLowAltitude) {
        mAutoTakeOffData->fsmAutoTakeOffStatus.AltitudeState = FSMAUTOTAKEOFFSTATUS_ALTITUDESTATE_LOW;
    } else {
        mAutoTakeOffData->fsmAutoTakeOffStatus.AltitudeState = FSMAUTOTAKEOFFSTATUS_ALTITUDESTATE_HIGH;
    }
    FSMAutoTakeOffStatusSet(&mAutoTakeOffData->fsmAutoTakeOffStatus);
}


float VtolAutoTakeOffFSM::BoundVelocityDown(float velocity_down)
{
    velocity_down = boundf(velocity_down, MIN_AUTOTAKEOFFRATE, MAX_AUTOTAKEOFFRATE);
    if (mAutoTakeOffData->flLowAltitude) {
        velocity_down *= LOW_ALT_DESCENT_REDUCTION_FACTOR;
    }
    mAutoTakeOffData->fsmAutoTakeOffStatus.targetDescentRate = velocity_down;

    if (mAutoTakeOffData->flAltitudeHold) {
        return 0.0f;
    } else {
        return velocity_down;
    }
}

void VtolAutoTakeOffFSM::assessAltitude(void)
{
    float positionDown;

    PositionStateDownGet(&positionDown);
    float takeOffDown = 0.0f;
    if (mAutoTakeOffData->takeOffLocation.Status == TAKEOFFLOCATION_STATUS_VALID) {
        takeOffDown = mAutoTakeOffData->takeOffLocation.Down;
    }
    float positionDownRelativeToTakeoff = positionDown - takeOffDown;
    if (positionDownRelativeToTakeoff < AUTOTAKEOFFING_SLOWDOWN_HEIGHT) {
        mAutoTakeOffData->flLowAltitude = false;
    } else {
        mAutoTakeOffData->flLowAltitude = true;
    }
}


// FSM Setup and Run method implementation

// State: INACTIVE
void VtolAutoTakeOffFSM::setup_inactive(void)
{
    // Re-initialise local variables
    mAutoTakeOffData->flZeroStabiHorizontal = false;
    mAutoTakeOffData->flConstrainThrust     = false;
}

// State: INIT ALTHOLD
void VtolAutoTakeOffFSM::setup_init_althold(void)
{
    setStateTimeout(TIMEOUT_INIT_ALTHOLD);
    // get target descent velocity
    mAutoTakeOffData->flZeroStabiHorizontal = false;
    mAutoTakeOffData->fsmAutoTakeOffStatus.targetDescentRate = BoundVelocityDown(pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_AUTOTAKEOFF_VELOCITYVECTOR_DOWN]);
    mAutoTakeOffData->flConstrainThrust     = false;
    mAutoTakeOffData->flAltitudeHold = true;
    mAutoTakeOffData->boundThrustMin = vtolPathFollowerSettings->ThrustLimits.Min;
    mAutoTakeOffData->boundThrustMax = vtolPathFollowerSettings->ThrustLimits.Max;
}

void VtolAutoTakeOffFSM::run_init_althold(uint8_t flTimeout)
{
    if (flTimeout) {
        mAutoTakeOffData->flAltitudeHold = false;
        setState(AUTOTAKEOFF_STATE_WTG_FOR_DESCENTRATE, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_TIMEOUT);
    }
}


// State: WAITING FOR DESCENT RATE
void VtolAutoTakeOffFSM::setup_wtg_for_descentrate(void)
{
    setStateTimeout(TIMEOUT_WTG_FOR_DESCENTRATE);
    // get target descent velocity
    mAutoTakeOffData->flZeroStabiHorizontal = false;
    mAutoTakeOffData->observationCount = 0;
    mAutoTakeOffData->observation2Count     = 0;
    mAutoTakeOffData->flConstrainThrust     = false;
    mAutoTakeOffData->flAltitudeHold = false;
    mAutoTakeOffData->boundThrustMin = vtolPathFollowerSettings->ThrustLimits.Min;
    mAutoTakeOffData->boundThrustMax = vtolPathFollowerSettings->ThrustLimits.Max;
}

void VtolAutoTakeOffFSM::run_wtg_for_descentrate(uint8_t flTimeout)
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
    if (velocityState.Down > (AUTOTAKEOFFRATE_LOWLIMIT_FACTOR * mAutoTakeOffData->fsmAutoTakeOffStatus.targetDescentRate) &&
        velocityState.Down < (AUTOTAKEOFFRATE_HILIMIT_FACTOR * mAutoTakeOffData->fsmAutoTakeOffStatus.targetDescentRate)) {
        if (mAutoTakeOffData->observationCount++ > WTG_FOR_DESCENTRATE_COUNT_LIMIT) {
            setState(AUTOTAKEOFF_STATE_AT_DESCENTRATE, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_DESCENTRATEOK);
            return;
        }
    }

    if (flTimeout) {
        setState(AUTOTAKEOFF_STATE_ABORT, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_TIMEOUT);
    }
}


// State: AT DESCENT RATE
void VtolAutoTakeOffFSM::setup_at_descentrate(void)
{
    setStateTimeout(TIMEOUT_AT_DESCENTRATE);
    mAutoTakeOffData->flZeroStabiHorizontal = false;
    mAutoTakeOffData->observationCount  = 0;
    mAutoTakeOffData->sum1 = 0.0f;
    mAutoTakeOffData->sum2 = 0.0f;
    mAutoTakeOffData->flConstrainThrust = false;
    mAutoTakeOffData->fsmAutoTakeOffStatus.averageDescentRate = MIN_AUTOTAKEOFFRATE;
    mAutoTakeOffData->fsmAutoTakeOffStatus.averageDescentThrust = vtolPathFollowerSettings->ThrustLimits.Neutral;
    mAutoTakeOffData->boundThrustMin    = vtolPathFollowerSettings->ThrustLimits.Min;
    mAutoTakeOffData->boundThrustMax    = vtolPathFollowerSettings->ThrustLimits.Max;
}

void VtolAutoTakeOffFSM::run_at_descentrate(uint8_t flTimeout)
{
    VelocityStateData velocityState;

    VelocityStateGet(&velocityState);

    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);

    mAutoTakeOffData->sum1 += velocityState.Down;
    mAutoTakeOffData->sum2 += stabDesired.Thrust;
    mAutoTakeOffData->observationCount++;
    if (flTimeout) {
        mAutoTakeOffData->fsmAutoTakeOffStatus.averageDescentRate   = boundf((mAutoTakeOffData->sum1 / (float)(mAutoTakeOffData->observationCount)), 0.5f * MIN_AUTOTAKEOFFRATE, 1.5f * MAX_AUTOTAKEOFFRATE);
        mAutoTakeOffData->fsmAutoTakeOffStatus.averageDescentThrust = boundf((mAutoTakeOffData->sum2 / (float)(mAutoTakeOffData->observationCount)), vtolPathFollowerSettings->ThrustLimits.Min, vtolPathFollowerSettings->ThrustLimits.Max);

        // We need to calculate a neutral limit to use later to constrain upper thrust range during states where we are close to the ground
        // As our battery gets flat, ThrustLimits.Neutral needs to constrain us too much and we get too fast a descent rate. We can
        // detect this by the fact that the descent rate will exceed the target and the required thrust will exceed the neutral value
        mAutoTakeOffData->fsmAutoTakeOffStatus.calculatedNeutralThrust = mAutoTakeOffData->fsmAutoTakeOffStatus.averageDescentRate / mAutoTakeOffData->fsmAutoTakeOffStatus.targetDescentRate * mAutoTakeOffData->fsmAutoTakeOffStatus.averageDescentThrust;
        mAutoTakeOffData->fsmAutoTakeOffStatus.calculatedNeutralThrust = boundf(mAutoTakeOffData->fsmAutoTakeOffStatus.calculatedNeutralThrust, vtolPathFollowerSettings->ThrustLimits.Neutral, vtolPathFollowerSettings->ThrustLimits.Max);


        setState(AUTOTAKEOFF_STATE_WTG_FOR_GROUNDEFFECT, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_DESCENTRATEOK);
    }
}


// State: WAITING FOR GROUND EFFECT
void VtolAutoTakeOffFSM::setup_wtg_for_groundeffect(void)
{
    // No timeout
    mAutoTakeOffData->flZeroStabiHorizontal = false;
    mAutoTakeOffData->observationCount = 0;
    mAutoTakeOffData->observation2Count     = 0;
    mAutoTakeOffData->sum1 = 0.0f;
    mAutoTakeOffData->sum2 = 0.0f;
    mAutoTakeOffData->flConstrainThrust     = false;
    mAutoTakeOffData->fsmAutoTakeOffStatus.WtgForGroundEffect.BounceVelocity = 0.0f;
    mAutoTakeOffData->fsmAutoTakeOffStatus.WtgForGroundEffect.BounceAccel    = 0.0f;
    mAutoTakeOffData->boundThrustMin = vtolPathFollowerSettings->ThrustLimits.Min;
    mAutoTakeOffData->boundThrustMax = vtolPathFollowerSettings->ThrustLimits.Max;
}

void VtolAutoTakeOffFSM::run_wtg_for_groundeffect(__attribute__((unused)) uint8_t flTimeout)
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
        mAutoTakeOffData->fsmAutoTakeOffStatus.WtgForGroundEffect.BounceVelocity = velocityState.Down;
    } else {
        mAutoTakeOffData->fsmAutoTakeOffStatus.WtgForGroundEffect.BounceVelocity = 0.0f;
    }

    // invert sign of accel to the standard convention of down is +ve and subtract the gravity to get
    // a relative acceleration term.
    float bounceAccel     = -accelState.z - g_e;
    uint8_t flBounceAccel = (bounceAccel < BOUNCE_ACCELERATION_TRIGGER_LIMIT);
    if (flBounceAccel) {
        mAutoTakeOffData->fsmAutoTakeOffStatus.WtgForGroundEffect.BounceAccel = bounceAccel;
    } else {
        mAutoTakeOffData->fsmAutoTakeOffStatus.WtgForGroundEffect.BounceAccel = 0.0f;
    }

    if (flBounce || flBounceAccel) {
        mAutoTakeOffData->observation2Count++;
        if (mAutoTakeOffData->observation2Count > BOUNCE_TRIGGER_COUNT) {
            setState(AUTOTAKEOFF_STATE_GROUNDEFFECT, (flBounce ? FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_BOUNCEVELOCITY : FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_BOUNCEACCEL));
            return;
        }
    } else {
        mAutoTakeOffData->observation2Count = 0;
    }

    // detect low descent rate
    uint8_t flDescentRateLow = (velocityState.Down < (GROUNDEFFECT_SLOWDOWN_FACTOR * mAutoTakeOffData->fsmAutoTakeOffStatus.averageDescentRate));
    if (flDescentRateLow) {
        mAutoTakeOffData->boundThrustMax = mAutoTakeOffData->fsmAutoTakeOffStatus.calculatedNeutralThrust;
        mAutoTakeOffData->observationCount++;
        if (mAutoTakeOffData->observationCount > GROUNDEFFECT_SLOWDOWN_COUNT) {
#ifndef DEBUG_GROUNDIMPACT
            setState(AUTOTAKEOFF_STATE_GROUNDEFFECT, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_LOWDESCENTRATE);
#endif
            return;
        }
    } else {
        mAutoTakeOffData->observationCount = 0;
    }

    updateVtolAutoTakeOffFSMStatus();
}

// STATE: GROUNDEFFET
void VtolAutoTakeOffFSM::setup_groundeffect(void)
{
    setStateTimeout(TIMEOUT_GROUNDEFFECT);
    mAutoTakeOffData->flZeroStabiHorizontal     = true;
    PositionStateData positionState;
    PositionStateGet(&positionState);
    mAutoTakeOffData->expectedAutoTakeOffPositionNorth = positionState.North;
    mAutoTakeOffData->expectedAutoTakeOffPositionEast  = positionState.East;
    mAutoTakeOffData->flConstrainThrust = false;

    // now that we have ground effect limit max thrust to neutral
    mAutoTakeOffData->boundThrustMin    = -0.1f;
    mAutoTakeOffData->boundThrustMax    = mAutoTakeOffData->fsmAutoTakeOffStatus.calculatedNeutralThrust;
}
void VtolAutoTakeOffFSM::run_groundeffect(__attribute__((unused)) uint8_t flTimeout)
{
    StabilizationDesiredData stabDesired;

    StabilizationDesiredGet(&stabDesired);
    if (stabDesired.Thrust < 0.0f) {
        setState(AUTOTAKEOFF_STATE_THRUSTOFF, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_ZEROTHRUST);
        return;
    }

    // Stay in this state until we get a low altitude flag.
    if (mAutoTakeOffData->flLowAltitude == false) {
        // worst case scenario is that we land and the pid brings thrust down to zero.
        return;
    }

    // detect broad sideways drift.  If for some reason we have a hard landing that the bounce detection misses, this will kick in
    PositionStateData positionState;
    PositionStateGet(&positionState);
    float north_error   = mAutoTakeOffData->expectedAutoTakeOffPositionNorth - positionState.North;
    float east_error    = mAutoTakeOffData->expectedAutoTakeOffPositionEast - positionState.East;
    float positionError = sqrtf(north_error * north_error + east_error * east_error);
    if (positionError > 0.3f) {
        setState(AUTOTAKEOFF_STATE_THRUSTDOWN, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_POSITIONERROR);
        return;
    }

    if (flTimeout) {
        setState(AUTOTAKEOFF_STATE_THRUSTDOWN, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_TIMEOUT);
    }
}

// STATE: THRUSTDOWN
void VtolAutoTakeOffFSM::setup_thrustdown(void)
{
    setStateTimeout(TIMEOUT_THRUSTDOWN);
    mAutoTakeOffData->flZeroStabiHorizontal = true;
    mAutoTakeOffData->flConstrainThrust     = true;
    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);
    mAutoTakeOffData->thrustLimit    = stabDesired.Thrust;
    mAutoTakeOffData->sum1 = stabDesired.Thrust / (float)TIMEOUT_THRUSTDOWN;
    mAutoTakeOffData->boundThrustMin = -0.1f;
    mAutoTakeOffData->boundThrustMax = mAutoTakeOffData->fsmAutoTakeOffStatus.calculatedNeutralThrust;
}

void VtolAutoTakeOffFSM::run_thrustdown(__attribute__((unused)) uint8_t flTimeout)
{
    // reduce thrust setpoint step by step
    mAutoTakeOffData->thrustLimit -= mAutoTakeOffData->sum1;

    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);
    if (stabDesired.Thrust < 0.0f || mAutoTakeOffData->thrustLimit < 0.0f) {
        setState(AUTOTAKEOFF_STATE_THRUSTOFF, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_ZEROTHRUST);
    }

    if (flTimeout) {
        setState(AUTOTAKEOFF_STATE_THRUSTOFF, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_TIMEOUT);
    }
}

// STATE: THRUSTOFF
void VtolAutoTakeOffFSM::setup_thrustoff(void)
{
    mAutoTakeOffData->thrustLimit       = -1.0f;
    mAutoTakeOffData->flConstrainThrust = true;
    mAutoTakeOffData->boundThrustMin    = -0.1f;
    mAutoTakeOffData->boundThrustMax    = 0.0f;
}

void VtolAutoTakeOffFSM::run_thrustoff(__attribute__((unused)) uint8_t flTimeout)
{
    setState(AUTOTAKEOFF_STATE_DISARMED, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_NONE);
}

// STATE: DISARMED
void VtolAutoTakeOffFSM::setup_disarmed(void)
{
    // nothing to do
    mAutoTakeOffData->flConstrainThrust     = false;
    mAutoTakeOffData->flZeroStabiHorizontal = false;
    mAutoTakeOffData->observationCount = 0;
    mAutoTakeOffData->boundThrustMin   = -0.1f;
    mAutoTakeOffData->boundThrustMax   = 0.0f;
}

void VtolAutoTakeOffFSM::run_disarmed(__attribute__((unused)) uint8_t flTimeout)
{
#ifdef DEBUG_GROUNDIMPACT
    if (mAutoTakeOffData->observationCount++ > 100) {
        setState(AUTOTAKEOFF_STATE_WTG_FOR_GROUNDEFFECT, FSMAUTOTAKEOFFSTATUS_STATEEXITREASON_NONE);
    }
#endif
}

void VtolAutoTakeOffFSM::fallback_to_hold(void)
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
void VtolAutoTakeOffFSM::setup_abort(void)
{
    mAutoTakeOffData->boundThrustMin        = vtolPathFollowerSettings->ThrustLimits.Min;
    mAutoTakeOffData->boundThrustMax        = vtolPathFollowerSettings->ThrustLimits.Max;
    mAutoTakeOffData->flConstrainThrust     = false;
    mAutoTakeOffData->flZeroStabiHorizontal = false;
    fallback_to_hold();
}

void VtolAutoTakeOffFSM::run_abort(__attribute__((unused)) uint8_t flTimeout)
{}
