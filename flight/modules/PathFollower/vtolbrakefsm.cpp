/*
 ******************************************************************************
 *
 * @file       vtolbrakefsm.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Vtol brake finate state machine to regulate behaviour of the
 *              brake controller.
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

#include <vtolpathfollowersettings.h>
#include <flightstatus.h>
#include <flightmodesettings.h>
#include <pathstatus.h>
#include <positionstate.h>
#include <velocitystate.h>
#include <velocitydesired.h>
#include <stabilizationdesired.h>
#include <attitudestate.h>
#include <manualcontrolcommand.h>
#include <systemsettings.h>
#include <stabilizationbank.h>
#include <stabilizationdesired.h>
#include <vtolselftuningstats.h>
#include <pathsummary.h>
}

// C++ includes
#include <vtolbrakefsm.h>


// Private constants
#define TIMER_COUNT_PER_SECOND                      (1000 / vtolPathFollowerSettings->UpdatePeriod)
#define BRAKE_FRACTIONALPROGRESS_STARTVELOCITYCHECK 0.95f
#define BRAKE_EXIT_VELOCITY_LIMIT                   0.2f

VtolBrakeFSM::PathFollowerFSM_BrakeStateHandler_T VtolBrakeFSM::sBrakeStateTable[BRAKE_STATE_SIZE] = {
    [BRAKE_STATE_INACTIVE] = { .setup = 0,                          .run = 0                        },
    [BRAKE_STATE_BRAKE]    = { .setup = &VtolBrakeFSM::setup_brake, .run = &VtolBrakeFSM::run_brake },
    [BRAKE_STATE_HOLD]     = { .setup = 0,                          .run = 0                        }
};

// pointer to a singleton instance
VtolBrakeFSM *VtolBrakeFSM::p_inst = 0;


VtolBrakeFSM::VtolBrakeFSM()
    : mBrakeData(0), vtolPathFollowerSettings(0), pathDesired(0), flightStatus(0)
{}


// Private types

// Private functions
// Public API methods
/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t VtolBrakeFSM::Initialize(VtolPathFollowerSettingsData *ptr_vtolPathFollowerSettings,
                                 PathDesiredData *ptr_pathDesired,
                                 FlightStatusData *ptr_flightStatus,
                                 PathStatusData *ptr_pathStatus)
{
    PIOS_Assert(ptr_vtolPathFollowerSettings);
    PIOS_Assert(ptr_pathDesired);
    PIOS_Assert(ptr_flightStatus);

    // allow for Initialize being called more than once.
    if (!mBrakeData) {
        mBrakeData = (VtolBrakeFSMData_T *)pios_malloc(sizeof(VtolBrakeFSMData_T));
        PIOS_Assert(mBrakeData);
    }
    memset(mBrakeData, sizeof(VtolBrakeFSMData_T), 0);
    vtolPathFollowerSettings = ptr_vtolPathFollowerSettings;
    pathDesired  = ptr_pathDesired;
    flightStatus = ptr_flightStatus;
    pathStatus   = ptr_pathStatus;
    initFSM();

    return 0;
}

void VtolBrakeFSM::Inactive(void)
{
    memset(mBrakeData, sizeof(VtolBrakeFSMData_T), 0);
    initFSM();
}

// Initialise the FSM
void VtolBrakeFSM::initFSM(void)
{
    mBrakeData->currentState = BRAKE_STATE_INACTIVE;
}

void VtolBrakeFSM::Activate()
{
    memset(mBrakeData, sizeof(VtolBrakeFSMData_T), 0);
    mBrakeData->currentState = BRAKE_STATE_INACTIVE;
    setState(BRAKE_STATE_BRAKE, FSMBRAKESTATUS_STATEEXITREASON_NONE);
}

void VtolBrakeFSM::Abort(void)
{
    setState(BRAKE_STATE_HOLD, FSMBRAKESTATUS_STATEEXITREASON_NONE);
}

PathFollowerFSMState_T VtolBrakeFSM::GetCurrentState(void)
{
    switch (mBrakeData->currentState) {
    case BRAKE_STATE_INACTIVE:
        return PFFSM_STATE_INACTIVE;

        break;
    default:
        return PFFSM_STATE_ACTIVE;

        break;
    }
}

void VtolBrakeFSM::Update()
{
    runState();
}

int32_t VtolBrakeFSM::runState(void)
{
    uint8_t flTimeout = false;

    mBrakeData->stateRunCount++;

    if (mBrakeData->stateTimeoutCount > 0 && mBrakeData->stateRunCount > mBrakeData->stateTimeoutCount) {
        flTimeout = true;
    }

    // If the current state has a static function, call it
    if (sBrakeStateTable[mBrakeData->currentState].run) {
        (this->*sBrakeStateTable[mBrakeData->currentState].run)(flTimeout);
    }
    return 0;
}

// Set the new state and perform setup for subsequent state run calls
// This is called by state run functions on event detection that drive
// state transitions.
void VtolBrakeFSM::setState(PathFollowerFSM_BrakeState_T newState, __attribute__((unused)) VtolBrakeFSMStatusStateExitReasonOptions reason)
{
    // mBrakeData->fsmBrakeStatus.StateExitReason[mBrakeData->currentState] = reason;

    if (mBrakeData->currentState == newState) {
        return;
    }
    mBrakeData->currentState      = newState;

    // Restart state timer counter
    mBrakeData->stateRunCount     = 0;

    // Reset state timeout to disabled/zero
    mBrakeData->stateTimeoutCount = 0;

    if (sBrakeStateTable[mBrakeData->currentState].setup) {
        (this->*sBrakeStateTable[mBrakeData->currentState].setup)();
    }

}


// Timeout utility function for use by state init implementations
void VtolBrakeFSM::setStateTimeout(int32_t count)
{
    mBrakeData->stateTimeoutCount = count;
}

// FSM Setup and Run method implementation

// State: WAITING FOR DESCENT RATE
void VtolBrakeFSM::setup_brake(void)
{
    setStateTimeout(TIMER_COUNT_PER_SECOND * pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_BRAKE_TIMEOUT]);
    mBrakeData->observationCount  = 0;
    mBrakeData->observation2Count = 0;
}


void VtolBrakeFSM::run_brake(uint8_t flTimeout)
{
    // Brake mode end condition checks
    bool exit_brake = false;
    VelocityStateData velocityState;
    PathSummaryData pathSummary;

    if (flTimeout) {
        pathSummary.brake_exit_reason = PATHSUMMARY_BRAKE_EXIT_REASON_TIMEOUT;
        exit_brake = true;
    } else if (pathStatus->fractional_progress > BRAKE_FRACTIONALPROGRESS_STARTVELOCITYCHECK) {
        VelocityStateGet(&velocityState);
        if (fabsf(velocityState.East) < BRAKE_EXIT_VELOCITY_LIMIT && fabsf(velocityState.North) < BRAKE_EXIT_VELOCITY_LIMIT) {
            pathSummary.brake_exit_reason = PATHSUMMARY_BRAKE_EXIT_REASON_PATHCOMPLETED;
            exit_brake = true;
        }
    }

    if (exit_brake) {
        // Calculate the distance error between the originally desired
        // stopping point and the actual brake-exit point.

        PositionStateData p;
        PositionStateGet(&p);
        float north_offset = pathDesired->End.North - p.North;
        float east_offset  = pathDesired->End.East - p.East;
        float down_offset  = pathDesired->End.Down - p.Down;
        pathSummary.brake_distance_offset = sqrtf(north_offset * north_offset + east_offset * east_offset + down_offset * down_offset);
        pathSummary.time_remaining = pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_BRAKE_TIMEOUT] - pathStatus->path_time;
        pathSummary.fractional_progress   = pathStatus->fractional_progress;
        float cur_velocity = velocityState.North * velocityState.North + velocityState.East * velocityState.East + velocityState.Down * velocityState.Down;
        cur_velocity = sqrtf(cur_velocity);
        pathSummary.decelrate = (pathDesired->StartingVelocity - cur_velocity) / pathStatus->path_time;
        pathSummary.brakeRateActualDesiredRatio = pathSummary.decelrate / vtolPathFollowerSettings->BrakeRate;
        pathSummary.velocityIntoHold = cur_velocity;
        pathSummary.Mode = PATHSUMMARY_MODE_BRAKE;
        pathSummary.UID  = pathStatus->UID;
        PathSummarySet(&pathSummary);

        setState(BRAKE_STATE_HOLD, FSMBRAKESTATUS_STATEEXITREASON_NONE);
    }
}

uint8_t VtolBrakeFSM::PositionHoldState(void)
{
    return mBrakeData->currentState == BRAKE_STATE_HOLD;
}
