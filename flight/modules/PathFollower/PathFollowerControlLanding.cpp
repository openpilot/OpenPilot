/*
 ******************************************************************************
 *
 * @file       PathFollowerControlLanding.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      This landing state machine is a helper state machine to the
 *              pathfollower task/thread to implement detailed landing controls.
 *		This is to be called only from the pathfollower task.
 *		Note that initiation of the land occurs in the manual control
 *		command thread calling plans.c plan_setup_land which writes
 *		the required PathDesired LAND mode.
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
 * This module acts as a landing FSM "autopilot"
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
#include <fsmlandstatus.h>
#include <pathsummary.h>
}

// C++ includes
#include "PathFollowerControlLanding.h"
#include "PathFollowerFSM.h"


// Private constants

// pointer to a singleton instance
PathFollowerControlLanding *PathFollowerControlLanding::p_inst = 0;

PathFollowerControlLanding::PathFollowerControlLanding()
: fsm(0), vtolPathFollowerSettings(0), pathDesired(0), flightStatus(0), pathStatus(0)
{}

// Private types

// Private functions
// Public API methods
void PathFollowerControlLanding::Activate(void) {
  fsm->Activate();
}
void PathFollowerControlLanding::Deactivate(void) {
    pid_zero(&PIDvel[0]);
    pid_zero(&PIDvel[1]);
    pid_zero(&PIDvel[2]);
    fsm->Inactive();

}
void PathFollowerControlLanding::SettingsUpdated(void) {
   pid_configure(&PIDvel[0], vtolPathFollowerSettings->HorizontalVelPID.Kp, vtolPathFollowerSettings->HorizontalVelPID.Ki, vtolPathFollowerSettings->HorizontalVelPID.Kd, vtolPathFollowerSettings->HorizontalVelPID.ILimit);
   pid_configure(&PIDvel[1], vtolPathFollowerSettings->HorizontalVelPID.Kp, vtolPathFollowerSettings->HorizontalVelPID.Ki, vtolPathFollowerSettings->HorizontalVelPID.Kd, vtolPathFollowerSettings->HorizontalVelPID.ILimit);
   pid_configure(&PIDvel[2], vtolPathFollowerSettings->VerticalVelPID.Kp, vtolPathFollowerSettings->VerticalVelPID.Ki, vtolPathFollowerSettings->VerticalVelPID.Kd, vtolPathFollowerSettings->VerticalVelPID.ILimit);
   pid_configure(&PIDposH[0], vtolPathFollowerSettings->HorizontalPosP, 0.0f, 0.0f, 0.0f);
   pid_configure(&PIDposH[1], vtolPathFollowerSettings->HorizontalPosP, 0.0f, 0.0f, 0.0f);
   fsm->SettingsUpdated();
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t PathFollowerControlLanding::Initialize(PathFollowerFSM *fsm_ptr,
                                               VtolPathFollowerSettingsData *ptr_vtolPathFollowerSettings,
                            PathDesiredData *ptr_pathDesired,
                            FlightStatusData *ptr_flightStatus,
                            PathStatusData *ptr_pathStatus)
{
    PIOS_Assert(ptr_vtolPathFollowerSettings);
    PIOS_Assert(ptr_pathDesired);
    PIOS_Assert(ptr_flightStatus);
    PIOS_Assert(ptr_pathStatus);
    PIOS_Assert(fsm_ptr);

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

    fsm = fsm_ptr;
    vtolPathFollowerSettings = ptr_vtolPathFollowerSettings;
    pathDesired  = ptr_pathDesired;
    flightStatus = ptr_flightStatus;
    pathStatus = ptr_pathStatus;

    return 0;
}


void PathFollowerControlLanding::UpdateVelocityDesired() {
  PositionStateData positionState;

  PositionStateGet(&positionState);
  VelocityStateData velocityState;
  VelocityStateGet(&velocityState);
  VelocityDesiredData velocityDesired;

  const float dT = vtolPathFollowerSettings->UpdatePeriod / 1000.0f;

  velocityDesired.North = pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_VELOCITY_VELOCITYVECTOR_NORTH];
  velocityDesired.East  = pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_VELOCITY_VELOCITYVECTOR_EAST];
  velocityDesired.Down  = pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_VELOCITY_VELOCITYVECTOR_DOWN];


  velocityDesired.Down = fsm->BoundVelocityDown(velocityDesired.Down);

  if (((uint8_t)pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_LAND_OPTIONS]) == PATHDESIRED_MODEPARAMETER_LAND_OPTION_HORIZONTAL_PH) {
	  float cur[3] = { positionState.North, positionState.East, positionState.Down };
	  struct path_status progress;
	  path_progress(pathDesired, cur, &progress);

	  // calculate correction
	  velocityDesired.North += pid_apply(&PIDposH[0], progress.correction_vector[0], dT);
	  velocityDesired.East  += pid_apply(&PIDposH[1], progress.correction_vector[1], dT);
  }

  // update pathstatus
  pathStatus->error = 0.0f;
  pathStatus->fractional_progress  = 0.0f;
  pathStatus->path_direction_north = velocityDesired.North;
  pathStatus->path_direction_east  = velocityDesired.East;
  pathStatus->path_direction_down  = velocityDesired.Down;

  pathStatus->correction_direction_north = velocityDesired.North - velocityState.North;
  pathStatus->correction_direction_east  = velocityDesired.East - velocityState.East;
  pathStatus->correction_direction_down  = velocityDesired.Down - velocityState.Down;


  VelocityDesiredSet(&velocityDesired);

}

int8_t PathFollowerControlLanding::UpdateStabilizationDesired(bool yaw_attitude, float yaw_direction) {
    const float dT     = vtolPathFollowerSettings->UpdatePeriod / 1000.0f;
    uint8_t result     = 1;

    VelocityDesiredData velocityDesired;
    VelocityStateData velocityState;
    StabilizationDesiredData stabDesired;
    AttitudeStateData attitudeState;
    StabilizationBankData stabSettings;
    SystemSettingsData systemSettings;
    VtolSelfTuningStatsData vtolSelfTuningStats;

    float northError;
    float northCommand;

    float eastError;
    float eastCommand;

    float downError;
    float downCommand;

    SystemSettingsGet(&systemSettings);
    VelocityStateGet(&velocityState);
    VelocityDesiredGet(&velocityDesired);
    StabilizationDesiredGet(&stabDesired);
    VelocityDesiredGet(&velocityDesired);
    AttitudeStateGet(&attitudeState);
    StabilizationBankGet(&stabSettings);
    VtolSelfTuningStatsGet(&vtolSelfTuningStats);


        // scale velocity if it is above configured maximum
        // for braking, we can not help it if initial velocity was greater
        float velH = sqrtf(velocityDesired.North * velocityDesired.North + velocityDesired.East * velocityDesired.East);
        if (velH > vtolPathFollowerSettings->HorizontalVelMax) {
            velocityDesired.North *= vtolPathFollowerSettings->HorizontalVelMax / velH;
            velocityDesired.East  *= vtolPathFollowerSettings->HorizontalVelMax / velH;
        }
        if (fabsf(velocityDesired.Down) > vtolPathFollowerSettings->VerticalVelMax) {
            velocityDesired.Down *= vtolPathFollowerSettings->VerticalVelMax / fabsf(velocityDesired.Down);
        }

    // calculate the velocity errors between desired and actual
    northError = velocityDesired.North - velocityState.North;
    eastError  = velocityDesired.East - velocityState.East;
    downError  = velocityDesired.Down - velocityState.Down;

    // Must flip this sign
    downError  = -downError;

        northCommand = pid_apply(&PIDvel[0], northError, dT) + velocityDesired.North * vtolPathFollowerSettings->VelocityFeedforward;
        eastCommand  = pid_apply(&PIDvel[1], eastError, dT) + velocityDesired.East * vtolPathFollowerSettings->VelocityFeedforward;

    pid_scaler local_scaler = { .p = 1.0f, .i = 1.0f, .d = 1.0f };
    fsm->CheckPidScalar(&local_scaler);
    downCommand = -pid_apply_setpoint(&PIDvel[2], &local_scaler, velocityDesired.Down, velocityState.Down, dT);


    stabDesired.Thrust = fsm->BoundThrust(vtolSelfTuningStats.NeutralThrustOffset + downCommand + vtolPathFollowerSettings->ThrustLimits.Neutral);


    // Project the north and east command signals into the pitch and roll based on yaw.  For this to behave well the
    // craft should move similarly for 5 deg roll versus 5 deg pitch

    float maxPitch = vtolPathFollowerSettings->MaxRollPitch;

    float angle_radians = DEG2RAD(attitudeState.Yaw);
    float cos_angle     = cosf(angle_radians);
    float sine_angle    = sinf(angle_radians);

    stabDesired.Pitch = boundf(-northCommand * cos_angle - eastCommand * sine_angle, -maxPitch, maxPitch);
    stabDesired.Roll  = boundf(-northCommand * sine_angle + eastCommand * cos_angle, -maxPitch, maxPitch);

    ManualControlCommandData manualControl;
    ManualControlCommandGet(&manualControl);


    stabDesired.StabilizationMode.Roll  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    if (yaw_attitude) {
        stabDesired.StabilizationMode.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
        stabDesired.Yaw = yaw_direction;
    } else {
        stabDesired.StabilizationMode.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;
        stabDesired.Yaw = stabSettings.MaximumRate.Yaw * manualControl.Yaw;
    }

    // default thrust mode to cruise control
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL;

    fsm->ConstrainStabiDesired(&stabDesired);
    StabilizationDesiredSet(&stabDesired);

    return result;
}

void PathFollowerControlLanding::UpdateAutoPilot()
{
        UpdateVelocityDesired();

        // yaw behaviour is configurable in vtolpathfollower, select yaw control algorithm
        bool yaw_attitude = true;
        float yaw = 0.0f;

        fsm->GetYaw(yaw_attitude, yaw);

        int8_t result = UpdateStabilizationDesired(yaw_attitude, yaw);

        if (!result) {
        	fsm->Abort();
        }

        fsm->Update();
        if (fsm->GetCurrentState() == PFFSM_STATE_DISARMED) {
            setArmedIfChanged(FLIGHTSTATUS_ARMED_DISARMED);
        }


    PathStatusSet(pathStatus);

}

void PathFollowerControlLanding::setArmedIfChanged(uint8_t val)
{
    if (flightStatus->Armed != val) {
        flightStatus->Armed = val;
        FlightStatusSet(flightStatus);
    }
}


