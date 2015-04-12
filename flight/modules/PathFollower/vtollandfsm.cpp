/*
 ******************************************************************************
 *
 * @file       vtollandfsm.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      This landing state machine is a helper state machine to the
 *             VtolLandController.
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
#include <statusvtolland.h>
#include <pathsummary.h>
}

// C++ includes
#include <vtollandfsm.h>


// Private constants
#define TIMER_COUNT_PER_SECOND            (1000 / vtolPathFollowerSettings->UpdatePeriod)
#define MIN_LANDRATE                      0.1f
#define MAX_LANDRATE                      0.6f
#define LOW_ALT_DESCENT_REDUCTION_FACTOR  0.7f  // TODO Need to make the transition smooth
#define LANDRATE_LOWLIMIT_FACTOR          0.5f
#define LANDRATE_HILIMIT_FACTOR           1.5f
#define TIMEOUT_INIT_ALTHOLD              (3 * TIMER_COUNT_PER_SECOND)
#define TIMEOUT_WTG_FOR_DESCENTRATE       (10 * TIMER_COUNT_PER_SECOND)
#define WTG_FOR_DESCENTRATE_COUNT_LIMIT   10
#define TIMEOUT_AT_DESCENTRATE            10
#define TIMEOUT_GROUNDEFFECT              (1 * TIMER_COUNT_PER_SECOND)
#define TIMEOUT_THRUSTDOWN                (2 * TIMER_COUNT_PER_SECOND)
#define LANDING_PID_SCALAR_P              2.0f
#define LANDING_PID_SCALAR_I              10.0f
#define LANDING_SLOWDOWN_HEIGHT           -5.0f
#define BOUNCE_VELOCITY_TRIGGER_LIMIT     -0.3f
#define BOUNCE_ACCELERATION_TRIGGER_LIMIT -6.0f
#define BOUNCE_TRIGGER_COUNT              4
#define GROUNDEFFECT_SLOWDOWN_FACTOR      0.3f
#define GROUNDEFFECT_SLOWDOWN_COUNT       4

VtolLandFSM::PathFollowerFSM_LandStateHandler_T VtolLandFSM::sLandStateTable[LAND_STATE_SIZE] = {
    [LAND_STATE_INACTIVE]       =       { .setup = &VtolLandFSM::setup_inactive,             .run = 0                                      },
    [LAND_STATE_INIT_ALTHOLD]   =       { .setup = &VtolLandFSM::setup_init_althold,         .run = &VtolLandFSM::run_init_althold         },
    [LAND_STATE_WTG_FOR_DESCENTRATE] =  { .setup = &VtolLandFSM::setup_wtg_for_descentrate,  .run = &VtolLandFSM::run_wtg_for_descentrate  },
    [LAND_STATE_AT_DESCENTRATE] =       { .setup = &VtolLandFSM::setup_at_descentrate,       .run = &VtolLandFSM::run_at_descentrate       },
    [LAND_STATE_WTG_FOR_GROUNDEFFECT] = { .setup = &VtolLandFSM::setup_wtg_for_groundeffect, .run = &VtolLandFSM::run_wtg_for_groundeffect },
    [LAND_STATE_GROUNDEFFECT]   =       { .setup = &VtolLandFSM::setup_groundeffect,         .run = &VtolLandFSM::run_groundeffect         },
    [LAND_STATE_THRUSTDOWN]     =       { .setup = &VtolLandFSM::setup_thrustdown,           .run = &VtolLandFSM::run_thrustdown           },
    [LAND_STATE_THRUSTOFF]      =       { .setup = &VtolLandFSM::setup_thrustoff,            .run = &VtolLandFSM::run_thrustoff            },
    [LAND_STATE_DISARMED]       =       { .setup = &VtolLandFSM::setup_disarmed,             .run = &VtolLandFSM::run_disarmed             },
    [LAND_STATE_ABORT] =                { .setup = &VtolLandFSM::setup_abort,                .run = &VtolLandFSM::run_abort                }
};

// pointer to a singleton instance
VtolLandFSM *VtolLandFSM::p_inst = 0;


VtolLandFSM::VtolLandFSM()
    : mLandData(0), vtolPathFollowerSettings(0), pathDesired(0), flightStatus(0)
{}

// Private types

// Private functions
// Public API methods
/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t VtolLandFSM::Initialize(VtolPathFollowerSettingsData *ptr_vtolPathFollowerSettings,
                                PathDesiredData *ptr_pathDesired,
                                FlightStatusData *ptr_flightStatus)
{
    PIOS_Assert(ptr_vtolPathFollowerSettings);
    PIOS_Assert(ptr_pathDesired);
    PIOS_Assert(ptr_flightStatus);

    if (mLandData == 0) {
        mLandData = (VtolLandFSMData_T *)pios_malloc(sizeof(VtolLandFSMData_T));
        PIOS_Assert(mLandData);
    }
    memset(mLandData, 0, sizeof(VtolLandFSMData_T));
    vtolPathFollowerSettings = ptr_vtolPathFollowerSettings;
    pathDesired  = ptr_pathDesired;
    flightStatus = ptr_flightStatus;
    initFSM();

    return 0;
}

void VtolLandFSM::Inactive(void)
{
    memset(mLandData, 0, sizeof(VtolLandFSMData_T));
    initFSM();
}

// Initialise the FSM
void VtolLandFSM::initFSM(void)
{
    if (vtolPathFollowerSettings != 0) {
        setState(STATUSVTOLLAND_STATE_INACTIVE, STATUSVTOLLAND_STATEEXITREASON_NONE);
    } else {
        mLandData->currentState = STATUSVTOLLAND_STATE_INACTIVE;
    }
}

void VtolLandFSM::Activate()
{
    memset(mLandData, 0, sizeof(VtolLandFSMData_T));
    mLandData->currentState   = STATUSVTOLLAND_STATE_INACTIVE;
    mLandData->flLowAltitude  = false;
    mLandData->flAltitudeHold = false;
    mLandData->fsmLandStatus.averageDescentRate      = MIN_LANDRATE;
    mLandData->fsmLandStatus.averageDescentThrust    = vtolPathFollowerSettings->ThrustLimits.Neutral;
    mLandData->fsmLandStatus.calculatedNeutralThrust = vtolPathFollowerSettings->ThrustLimits.Neutral;
    mLandData->boundThrustMin = vtolPathFollowerSettings->ThrustLimits.Min;
    mLandData->boundThrustMax = vtolPathFollowerSettings->ThrustLimits.Max;
    TakeOffLocationGet(&(mLandData->takeOffLocation));
    mLandData->fsmLandStatus.AltitudeAtState[STATUSVTOLLAND_STATE_INACTIVE] = 0.0f;
    assessAltitude();

    if (pathDesired->Mode == PATHDESIRED_MODE_LAND) {
#ifndef DEBUG_GROUNDIMPACT
        setState(STATUSVTOLLAND_STATE_INITALTHOLD, STATUSVTOLLAND_STATEEXITREASON_NONE);
#else
        setState(STATUSVTOLLAND_STATE_WTGFORGROUNDEFFECT, STATUSVTOLLAND_STATEEXITREASON_NONE);
#endif
    } else {
        // move to error state and callback to position hold
        setState(STATUSVTOLLAND_STATE_ABORT, STATUSVTOLLAND_STATEEXITREASON_NONE);
    }
}

void VtolLandFSM::Abort(void)
{
    setState(STATUSVTOLLAND_STATE_ABORT, STATUSVTOLLAND_STATEEXITREASON_NONE);
}

PathFollowerFSMState_T VtolLandFSM::GetCurrentState(void)
{
    switch (mLandData->currentState) {
    case STATUSVTOLLAND_STATE_INACTIVE:
        return PFFSM_STATE_INACTIVE;

        break;
    case STATUSVTOLLAND_STATE_ABORT:
        return PFFSM_STATE_ABORT;

        break;
    case STATUSVTOLLAND_STATE_DISARMED:
        return PFFSM_STATE_DISARMED;

        break;
    default:
        return PFFSM_STATE_ACTIVE;

        break;
    }
}

void VtolLandFSM::Update()
{
    runState();
    if (GetCurrentState() != PFFSM_STATE_INACTIVE) {
        runAlways();
    }
}

int32_t VtolLandFSM::runState(void)
{
    uint8_t flTimeout = false;

    mLandData->stateRunCount++;

    if (mLandData->stateTimeoutCount > 0 && mLandData->stateRunCount > mLandData->stateTimeoutCount) {
        flTimeout = true;
    }

    // If the current state has a static function, call it
    if (sLandStateTable[mLandData->currentState].run) {
        (this->*sLandStateTable[mLandData->currentState].run)(flTimeout);
    }
    return 0;
}

int32_t VtolLandFSM::runAlways(void)
{
    void assessAltitude(void);

    return 0;
}

// PathFollower implements the PID scheme and has a objective
// set by a PathDesired object.  Based on the mode, pathfollower
// uses FSM's as helper functions that manage state and event detection.
// PathFollower calls into FSM methods to alter its commands.

void VtolLandFSM::BoundThrust(float &ulow, float &uhigh)
{
    ulow  = mLandData->boundThrustMin;
    uhigh = mLandData->boundThrustMax;


    if (mLandData->flConstrainThrust) {
        uhigh = mLandData->thrustLimit;
    }
}

void VtolLandFSM::ConstrainStabiDesired(StabilizationDesiredData *stabDesired)
{
    if (mLandData->flZeroStabiHorizontal && stabDesired) {
        stabDesired->Pitch = 0.0f;
        stabDesired->Roll  = 0.0f;
        stabDesired->Yaw   = 0.0f;
    }
}

void VtolLandFSM::CheckPidScaler(pid_scaler *local_scaler)
{
    if (mLandData->flLowAltitude) {
        local_scaler->p = LANDING_PID_SCALAR_P;
        local_scaler->i = LANDING_PID_SCALAR_I;
    }
}


// Set the new state and perform setup for subsequent state run calls
// This is called by state run functions on event detection that drive
// state transitions.
void VtolLandFSM::setState(StatusVtolLandStateOptions newState, StatusVtolLandStateExitReasonOptions reason)
{
    mLandData->fsmLandStatus.StateExitReason[mLandData->currentState] = reason;

    if (mLandData->currentState == newState) {
        return;
    }
    mLandData->currentState = newState;

    if (newState != STATUSVTOLLAND_STATE_INACTIVE) {
        PositionStateData positionState;
        PositionStateGet(&positionState);
        float takeOffDown = 0.0f;
        if (mLandData->takeOffLocation.Status == TAKEOFFLOCATION_STATUS_VALID) {
            takeOffDown = mLandData->takeOffLocation.Down;
        }
        mLandData->fsmLandStatus.AltitudeAtState[newState] = positionState.Down - takeOffDown;
        assessAltitude();
    }

    // Restart state timer counter
    mLandData->stateRunCount     = 0;

    // Reset state timeout to disabled/zero
    mLandData->stateTimeoutCount = 0;

    if (sLandStateTable[mLandData->currentState].setup) {
        (this->*sLandStateTable[mLandData->currentState].setup)();
    }

    updateVtolLandFSMStatus();
}


// Timeout utility function for use by state init implementations
void VtolLandFSM::setStateTimeout(int32_t count)
{
    mLandData->stateTimeoutCount = count;
}

void VtolLandFSM::updateVtolLandFSMStatus()
{
    mLandData->fsmLandStatus.State = mLandData->currentState;
    if (mLandData->flLowAltitude) {
        mLandData->fsmLandStatus.AltitudeState = STATUSVTOLLAND_ALTITUDESTATE_LOW;
    } else {
        mLandData->fsmLandStatus.AltitudeState = STATUSVTOLLAND_ALTITUDESTATE_HIGH;
    }
    StatusVtolLandSet(&mLandData->fsmLandStatus);
}


float VtolLandFSM::BoundVelocityDown(float velocity_down)
{
    velocity_down = boundf(velocity_down, MIN_LANDRATE, MAX_LANDRATE);
    if (mLandData->flLowAltitude) {
        velocity_down *= LOW_ALT_DESCENT_REDUCTION_FACTOR;
    }
    mLandData->fsmLandStatus.targetDescentRate = velocity_down;

    if (mLandData->flAltitudeHold) {
        return 0.0f;
    } else {
        return velocity_down;
    }
}

void VtolLandFSM::assessAltitude(void)
{
    float positionDown;

    PositionStateDownGet(&positionDown);
    float takeOffDown = 0.0f;
    if (mLandData->takeOffLocation.Status == TAKEOFFLOCATION_STATUS_VALID) {
        takeOffDown = mLandData->takeOffLocation.Down;
    }
    float positionDownRelativeToTakeoff = positionDown - takeOffDown;
    if (positionDownRelativeToTakeoff < LANDING_SLOWDOWN_HEIGHT) {
        mLandData->flLowAltitude = false;
    } else {
        mLandData->flLowAltitude = true;
    }
}


// FSM Setup and Run method implementation

// State: INACTIVE
void VtolLandFSM::setup_inactive(void)
{
    // Re-initialise local variables
    mLandData->flZeroStabiHorizontal = false;
    mLandData->flConstrainThrust     = false;
}

// State: INIT ALTHOLD
void VtolLandFSM::setup_init_althold(void)
{
    setStateTimeout(TIMEOUT_INIT_ALTHOLD);
    // get target descent velocity
    mLandData->flZeroStabiHorizontal = false;
    mLandData->fsmLandStatus.targetDescentRate = BoundVelocityDown(pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_LAND_VELOCITYVECTOR_DOWN]);
    mLandData->flConstrainThrust     = false;
    mLandData->flAltitudeHold = true;
    mLandData->boundThrustMin = vtolPathFollowerSettings->ThrustLimits.Min;
    mLandData->boundThrustMax = vtolPathFollowerSettings->ThrustLimits.Max;
}

void VtolLandFSM::run_init_althold(uint8_t flTimeout)
{
    if (flTimeout) {
        mLandData->flAltitudeHold = false;
        setState(STATUSVTOLLAND_STATE_WTGFORDESCENTRATE, STATUSVTOLLAND_STATEEXITREASON_TIMEOUT);
    }
}


// State: WAITING FOR DESCENT RATE
void VtolLandFSM::setup_wtg_for_descentrate(void)
{
    setStateTimeout(TIMEOUT_WTG_FOR_DESCENTRATE);
    // get target descent velocity
    mLandData->flZeroStabiHorizontal = false;
    mLandData->observationCount = 0;
    mLandData->observation2Count     = 0;
    mLandData->flConstrainThrust     = false;
    mLandData->flAltitudeHold = false;
    mLandData->boundThrustMin = vtolPathFollowerSettings->ThrustLimits.Min;
    mLandData->boundThrustMax = vtolPathFollowerSettings->ThrustLimits.Max;
}

void VtolLandFSM::run_wtg_for_descentrate(uint8_t flTimeout)
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
    if (velocityState.Down > (LANDRATE_LOWLIMIT_FACTOR * mLandData->fsmLandStatus.targetDescentRate) &&
        velocityState.Down < (LANDRATE_HILIMIT_FACTOR * mLandData->fsmLandStatus.targetDescentRate)) {
        if (mLandData->observationCount++ > WTG_FOR_DESCENTRATE_COUNT_LIMIT) {
            setState(STATUSVTOLLAND_STATE_ATDESCENTRATE, STATUSVTOLLAND_STATEEXITREASON_DESCENTRATEOK);
            return;
        }
    }

    if (flTimeout) {
        setState(STATUSVTOLLAND_STATE_ABORT, STATUSVTOLLAND_STATEEXITREASON_TIMEOUT);
    }
}


// State: AT DESCENT RATE
void VtolLandFSM::setup_at_descentrate(void)
{
    setStateTimeout(TIMEOUT_AT_DESCENTRATE);
    mLandData->flZeroStabiHorizontal = false;
    mLandData->observationCount  = 0;
    mLandData->sum1 = 0.0f;
    mLandData->sum2 = 0.0f;
    mLandData->flConstrainThrust = false;
    mLandData->fsmLandStatus.averageDescentRate = MIN_LANDRATE;
    mLandData->fsmLandStatus.averageDescentThrust = vtolPathFollowerSettings->ThrustLimits.Neutral;
    mLandData->boundThrustMin    = vtolPathFollowerSettings->ThrustLimits.Min;
    mLandData->boundThrustMax    = vtolPathFollowerSettings->ThrustLimits.Max;
}

void VtolLandFSM::run_at_descentrate(uint8_t flTimeout)
{
    VelocityStateData velocityState;

    VelocityStateGet(&velocityState);

    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);

    mLandData->sum1 += velocityState.Down;
    mLandData->sum2 += stabDesired.Thrust;
    mLandData->observationCount++;
    if (flTimeout) {
        mLandData->fsmLandStatus.averageDescentRate   = boundf((mLandData->sum1 / (float)(mLandData->observationCount)), 0.5f * MIN_LANDRATE, 1.5f * MAX_LANDRATE);
        mLandData->fsmLandStatus.averageDescentThrust = boundf((mLandData->sum2 / (float)(mLandData->observationCount)), vtolPathFollowerSettings->ThrustLimits.Min, vtolPathFollowerSettings->ThrustLimits.Max);

        // We need to calculate a neutral limit to use later to constrain upper thrust range during states where we are close to the ground
        // As our battery gets flat, ThrustLimits.Neutral needs to constrain us too much and we get too fast a descent rate. We can
        // detect this by the fact that the descent rate will exceed the target and the required thrust will exceed the neutral value
        mLandData->fsmLandStatus.calculatedNeutralThrust = mLandData->fsmLandStatus.averageDescentRate / mLandData->fsmLandStatus.targetDescentRate * mLandData->fsmLandStatus.averageDescentThrust;
        mLandData->fsmLandStatus.calculatedNeutralThrust = boundf(mLandData->fsmLandStatus.calculatedNeutralThrust, vtolPathFollowerSettings->ThrustLimits.Neutral, vtolPathFollowerSettings->ThrustLimits.Max);


        setState(STATUSVTOLLAND_STATE_WTGFORGROUNDEFFECT, STATUSVTOLLAND_STATEEXITREASON_DESCENTRATEOK);
    }
}


// State: WAITING FOR GROUND EFFECT
void VtolLandFSM::setup_wtg_for_groundeffect(void)
{
    // No timeout
    mLandData->flZeroStabiHorizontal = false;
    mLandData->observationCount = 0;
    mLandData->observation2Count     = 0;
    mLandData->sum1 = 0.0f;
    mLandData->sum2 = 0.0f;
    mLandData->flConstrainThrust     = false;
    mLandData->fsmLandStatus.WtgForGroundEffect.BounceVelocity = 0.0f;
    mLandData->fsmLandStatus.WtgForGroundEffect.BounceAccel    = 0.0f;
    mLandData->boundThrustMin = vtolPathFollowerSettings->ThrustLimits.Min;
    mLandData->boundThrustMax = vtolPathFollowerSettings->ThrustLimits.Max;
}

void VtolLandFSM::run_wtg_for_groundeffect(__attribute__((unused)) uint8_t flTimeout)
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
        mLandData->fsmLandStatus.WtgForGroundEffect.BounceVelocity = velocityState.Down;
    } else {
        mLandData->fsmLandStatus.WtgForGroundEffect.BounceVelocity = 0.0f;
    }

    // invert sign of accel to the standard convention of down is +ve and subtract the gravity to get
    // a relative acceleration term.
    float bounceAccel     = -accelState.z - g_e;
    uint8_t flBounceAccel = (bounceAccel < BOUNCE_ACCELERATION_TRIGGER_LIMIT);
    if (flBounceAccel) {
        mLandData->fsmLandStatus.WtgForGroundEffect.BounceAccel = bounceAccel;
    } else {
        mLandData->fsmLandStatus.WtgForGroundEffect.BounceAccel = 0.0f;
    }

    if (flBounce || flBounceAccel) {
        mLandData->observation2Count++;
        if (mLandData->observation2Count > BOUNCE_TRIGGER_COUNT) {
            setState(STATUSVTOLLAND_STATE_GROUNDEFFECT, (flBounce ? STATUSVTOLLAND_STATEEXITREASON_BOUNCEVELOCITY : STATUSVTOLLAND_STATEEXITREASON_BOUNCEACCEL));
            return;
        }
    } else {
        mLandData->observation2Count = 0;
    }

    // detect low descent rate
    uint8_t flDescentRateLow = (velocityState.Down < (GROUNDEFFECT_SLOWDOWN_FACTOR * mLandData->fsmLandStatus.averageDescentRate));
    if (flDescentRateLow) {
        mLandData->boundThrustMax = mLandData->fsmLandStatus.calculatedNeutralThrust;
        mLandData->observationCount++;
        if (mLandData->observationCount > GROUNDEFFECT_SLOWDOWN_COUNT) {
#ifndef DEBUG_GROUNDIMPACT
            setState(STATUSVTOLLAND_STATE_GROUNDEFFECT, STATUSVTOLLAND_STATEEXITREASON_LOWDESCENTRATE);
#endif
            return;
        }
    } else {
        mLandData->observationCount = 0;
    }

    updateVtolLandFSMStatus();
}

// STATE: GROUNDEFFET
void VtolLandFSM::setup_groundeffect(void)
{
    setStateTimeout(TIMEOUT_GROUNDEFFECT);
    mLandData->flZeroStabiHorizontal     = true;
    PositionStateData positionState;
    PositionStateGet(&positionState);
    mLandData->expectedLandPositionNorth = positionState.North;
    mLandData->expectedLandPositionEast  = positionState.East;
    mLandData->flConstrainThrust = false;

    // now that we have ground effect limit max thrust to neutral
    mLandData->boundThrustMin    = -0.1f;
    mLandData->boundThrustMax    = mLandData->fsmLandStatus.calculatedNeutralThrust;
}
void VtolLandFSM::run_groundeffect(__attribute__((unused)) uint8_t flTimeout)
{
    StabilizationDesiredData stabDesired;

    StabilizationDesiredGet(&stabDesired);
    if (stabDesired.Thrust < 0.0f) {
        setState(STATUSVTOLLAND_STATE_THRUSTOFF, STATUSVTOLLAND_STATEEXITREASON_ZEROTHRUST);
        return;
    }

    // Stay in this state until we get a low altitude flag.
    if (mLandData->flLowAltitude == false) {
        // worst case scenario is that we land and the pid brings thrust down to zero.
        return;
    }

    // detect broad sideways drift.  If for some reason we have a hard landing that the bounce detection misses, this will kick in
    PositionStateData positionState;
    PositionStateGet(&positionState);
    float north_error   = mLandData->expectedLandPositionNorth - positionState.North;
    float east_error    = mLandData->expectedLandPositionEast - positionState.East;
    float positionError = sqrtf(north_error * north_error + east_error * east_error);
    if (positionError > 0.3f) {
        setState(STATUSVTOLLAND_STATE_THRUSTDOWN, STATUSVTOLLAND_STATEEXITREASON_POSITIONERROR);
        return;
    }

    if (flTimeout) {
        setState(STATUSVTOLLAND_STATE_THRUSTDOWN, STATUSVTOLLAND_STATEEXITREASON_TIMEOUT);
    }
}

// STATE: THRUSTDOWN
void VtolLandFSM::setup_thrustdown(void)
{
    setStateTimeout(TIMEOUT_THRUSTDOWN);
    mLandData->flZeroStabiHorizontal = true;
    mLandData->flConstrainThrust     = true;
    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);
    mLandData->thrustLimit    = stabDesired.Thrust;
    mLandData->sum1 = stabDesired.Thrust / (float)TIMEOUT_THRUSTDOWN;
    mLandData->boundThrustMin = -0.1f;
    mLandData->boundThrustMax = mLandData->fsmLandStatus.calculatedNeutralThrust;
}

void VtolLandFSM::run_thrustdown(__attribute__((unused)) uint8_t flTimeout)
{
    // reduce thrust setpoint step by step
    mLandData->thrustLimit -= mLandData->sum1;

    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);
    if (stabDesired.Thrust < 0.0f || mLandData->thrustLimit < 0.0f) {
        setState(STATUSVTOLLAND_STATE_THRUSTOFF, STATUSVTOLLAND_STATEEXITREASON_ZEROTHRUST);
    }

    if (flTimeout) {
        setState(STATUSVTOLLAND_STATE_THRUSTOFF, STATUSVTOLLAND_STATEEXITREASON_TIMEOUT);
    }
}

// STATE: THRUSTOFF
void VtolLandFSM::setup_thrustoff(void)
{
    mLandData->thrustLimit       = -1.0f;
    mLandData->flConstrainThrust = true;
    mLandData->boundThrustMin    = -0.1f;
    mLandData->boundThrustMax    = 0.0f;
}

void VtolLandFSM::run_thrustoff(__attribute__((unused)) uint8_t flTimeout)
{
    setState(STATUSVTOLLAND_STATE_DISARMED, STATUSVTOLLAND_STATEEXITREASON_NONE);
}

// STATE: DISARMED
void VtolLandFSM::setup_disarmed(void)
{
    // nothing to do
    mLandData->flConstrainThrust     = false;
    mLandData->flZeroStabiHorizontal = false;
    mLandData->observationCount = 0;
    mLandData->boundThrustMin   = -0.1f;
    mLandData->boundThrustMax   = 0.0f;
}

void VtolLandFSM::run_disarmed(__attribute__((unused)) uint8_t flTimeout)
{
#ifdef DEBUG_GROUNDIMPACT
    if (mLandData->observationCount++ > 100) {
        setState(STATUSVTOLLAND_STATE_WTGFORGROUNDEFFECT, STATUSVTOLLAND_STATEEXITREASON_NONE);
    }
#endif
}

void VtolLandFSM::fallback_to_hold(void)
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
void VtolLandFSM::setup_abort(void)
{
    mLandData->boundThrustMin        = vtolPathFollowerSettings->ThrustLimits.Min;
    mLandData->boundThrustMax        = vtolPathFollowerSettings->ThrustLimits.Max;
    mLandData->flConstrainThrust     = false;
    mLandData->flZeroStabiHorizontal = false;
    fallback_to_hold();
}

void VtolLandFSM::run_abort(__attribute__((unused)) uint8_t flTimeout)
{}
