/*
 ******************************************************************************
 *
 * @file       fsm_brake.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      This brakeing state machine is a helper state machine to the
 *              pathfollower task/thread to implement detailed brakeing controls.
 *		This is to be called only from the pathfollower task.
 *		Note that initiation of the brake occurs in the manual control
 *		command thread calling plans.c plan_setup_brake which writes
 *		the required PathDesired BRAKE mode.
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
 * Input object: TODO Update when completed
 * Input object:
 * Input object:
 * Output object:
 *
 * This module acts as a brakeing FSM "autopilot"
 * This is a periodic delayed callback module
 *
 * Modules have no API, all communication to other modules is done through UAVObjects.
 * However modules may use the API exposed by shared libraries.
 * See the OpenPilot wiki for more details.
 * http://www.openpilot.org/OpenPilot_Application_Architecture
 *
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

// TODO Remove unused
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
#include <fsmbrakestatus.h>
#include <pathsummary.h>
}

// C++ includes
#include <FSMBrake.h>


// Private constants
#define TIMER_COUNT_PER_SECOND            (1000 / vtolPathFollowerSettings->UpdatePeriod)

FSMBrake::PathFollowerFSM_BrakeStateHandler_T FSMBrake::sBrakeStateTable[BRAKE_STATE_SIZE] = {
    [BRAKE_STATE_INACTIVE]       	=  { .setup = &FSMBrake::setup_inactive,.run = 0      },
    [BRAKE_STATE_BRAKE]   		=  { .setup = &FSMBrake::setup_brake,   .run = &FSMBrake::run_brake         },
    [BRAKE_STATE_HOLD] 			=  { .setup = &FSMBrake::setup_hold,    .run = &FSMBrake::run_hold  }
};

// pointer to a singleton instance
FSMBrake *FSMBrake::p_inst = 0;


FSMBrake::FSMBrake()
    : mBrakeData(0), vtolPathFollowerSettings(0), pathDesired(0), flightStatus(0)
{}

// Private types

// Private functions
// Public API methods
/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t FSMBrake::Initialize(VtolPathFollowerSettingsData *ptr_vtolPathFollowerSettings,
                            PathDesiredData *ptr_pathDesired,
                            FlightStatusData *ptr_flightStatus)
{
    PIOS_Assert(ptr_vtolPathFollowerSettings);
    PIOS_Assert(ptr_pathDesired);
    PIOS_Assert(ptr_flightStatus);

    mBrakeData = (FSMBrakeData_T *)pios_malloc(sizeof(FSMBrakeData_T));
    PIOS_Assert(mBrakeData);
    memset(mBrakeData, sizeof(FSMBrakeData_T), 0);
    vtolPathFollowerSettings = ptr_vtolPathFollowerSettings;
    pathDesired  = ptr_pathDesired;
    flightStatus = ptr_flightStatus;
    initFSM();

    return 0;
}

void FSMBrake::Inactive(void)
{
    memset(mBrakeData, sizeof(FSMBrakeData_T), 0);
    initFSM();
}

// Initialise the FSM
void FSMBrake::initFSM(void)
{
    if (vtolPathFollowerSettings != 0) {
        setState(BRAKE_STATE_INACTIVE, FSMBRAKESTATUS_STATEEXITREASON_NONE);
    } else {
        mBrakeData->currentState = BRAKE_STATE_INACTIVE;
    }
}

void FSMBrake::Activate()
{
    memset(mBrakeData, sizeof(FSMBrakeData_T), 0);
    mBrakeData->currentState   = BRAKE_STATE_INACTIVE;
    setState(BRAKE_STATE_BRAKE, FSMBRAKESTATUS_STATEEXITREASON_NONE);
}

void FSMBrake::Abort(void)
{
    setState(BRAKE_STATE_HOLD, FSMBRAKESTATUS_STATEEXITREASON_NONE);
}

PathFollowerFSMState_T FSMBrake::GetCurrentState(void)
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

void FSMBrake::Update()
{
    runState();
    if (GetCurrentState() != PFFSM_STATE_INACTIVE) {
        runAlways();
    }
}

int32_t FSMBrake::runState(void)
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

int32_t FSMBrake::runAlways(void)
{
    return 0;
}

// Set the new state and perform setup for subsequent state run calls
// This is called by state run functions on event detection that drive
// state transitions.
void FSMBrake::setState(PathFollowerFSM_BrakeState_T newState, FSMBrakeStatusStateExitReasonOptions reason)
{
    mBrakeData->fsmBrakeStatus.StateExitReason[mBrakeData->currentState] = reason;

    if (mBrakeData->currentState == newState) {
        return;
    }
    mBrakeData->currentState = newState;

    // Restart state timer counter
    mBrakeData->stateRunCount     = 0;

    // Reset state timeout to disabled/zero
    mBrakeData->stateTimeoutCount = 0;

    if (sBrakeStateTable[mBrakeData->currentState].setup) {
        (this->*sBrakeStateTable[mBrakeData->currentState].setup)();
    }

    updateFSMBrakeStatus();
}


// Timeout utility function for use by state init implementations
void FSMBrake::setStateTimeout(int32_t count)
{
    mBrakeData->stateTimeoutCount = count;
}

void FSMBrake::updateFSMBrakeStatus()
{
    mBrakeData->fsmBrakeStatus.State = mBrakeData->currentState;
    if (mBrakeData->flLowAltitude) {
        mBrakeData->fsmBrakeStatus.AltitudeState = FSMBRAKESTATUS_ALTITUDESTATE_LOW;
    } else {
        mBrakeData->fsmBrakeStatus.AltitudeState = FSMBRAKESTATUS_ALTITUDESTATE_HIGH;
    }
    FSMBrakeStatusSet(&mBrakeData->fsmBrakeStatus);
}


// FSM Setup and Run method implementation

// State: INACTIVE
void FSMBrake::setup_inactive(void)
{
    // Re-initialise local variables
}


// State: WAITING FOR DESCENT RATE
void FSMBrake::setup_brake(void)
{
    setStateTimeout(TIMEOUT_WTG_FOR_DESCENTRATE);
    mBrakeData->observationCount = 0;
    mBrakeData->observation2Count     = 0;
}

void FSMBrake::run_brake(uint8_t flTimeout)
{
// Brake mode end condition checks
     bool exit_brake = false;
     VelocityStateData velocityState;
     if (pathStatus->path_time > pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_BRAKE_TIMEOUT]) { // enter hold on timeout
         pathSummary->brake_exit_reason = PATHSUMMARY_BRAKE_EXIT_REASON_TIMEOUT;
         exit_brake = true;
     } else if (pathStatus->fractional_progress > BRAKE_FRACTIONALPROGRESS_STARTVELOCITYCHECK) {
         VelocityStateGet(&velocityState);
         if (fabsf(velocityState.East) < BRAKE_EXIT_VELOCITY_LIMIT && fabsf(velocityState.North) < BRAKE_EXIT_VELOCITY_LIMIT) {
             pathSummary->brake_exit_reason = PATHSUMMARY_BRAKE_EXIT_REASON_PATHCOMPLETED;
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
         pathSummary->brake_distance_offset = sqrtf(north_offset * north_offset + east_offset * east_offset + down_offset * down_offset);
         pathSummary->time_remaining = pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_BRAKE_TIMEOUT] - pathStatus->path_time;
         pathSummary->fractional_progress   = pathStatus->fractional_progress;
         float cur_velocity = velocityState.North * velocityState.North + velocityState.East * velocityState.East + velocityState.Down * velocityState.Down;
         cur_velocity = sqrtf(cur_velocity);
         pathSummary->decelrate = (pathDesired->StartingVelocity - cur_velocity) / pathStatus->path_time;
         pathSummary->brakeRateActualDesiredRatio = pathSummary->decelrate / vtolPathFollowerSettings->BrakeRate;
         pathSummary->velocityIntoHold = cur_velocity;
         pathSummary->Mode = PATHSUMMARY_MODE_BRAKE;
         pathSummary->UID  = pathStatus->UID;
         PathSummarySet(&pathSummary);

         plan_setup_assistedcontrol(true); // braking timeout true
         // having re-entered hold allow reassessment of neutral thrust
         neutralThrustEst.have_correction = false;
     }
 }
#endif


    // Look at current actual thrust...are we already shutdown??
    VelocityStateData velocityState;

    VelocityStateGet(&velocityState);
}


void FSMBrake::fallback_to_hold(void)
{
  // TODO SHould I simply add position controls to my pid loops herein
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
    pathDesired->Mode = PATHDESIRED_MODE_FLYENDPOINT;

    PathDesiredSet(pathDesired);
}

// abort repeatedly overwrites pathfollower's objective on a brakeing abort and
// continues to do so until a flight mode change.
void FSMBrake::setup_hold(void)
{
    fallback_to_hold();
}

void FSMBrake::run_hold(__attribute__((unused)) uint8_t flTimeout)
{
}
