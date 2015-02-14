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
#include <pathsummary.h>
}

// C++ includes
#include "PathFollowerControlLanding.h"
#include "PathFollowerFSM.h"
#include "PIDControlThrust.h"

// Private constants

// pointer to a singleton instance
PathFollowerControlLanding *PathFollowerControlLanding::p_inst = 0;

PathFollowerControlLanding::PathFollowerControlLanding()
    : fsm(0), vtolPathFollowerSettings(0), pathDesired(0), flightStatus(0), pathStatus(0)
{
}

// Called when mode first engaged
void PathFollowerControlLanding::Activate(void)
{
    fsm->Activate();
    controlThrust.Activate();
    controlNE.Activate();
}

// Objective updated in pathdesired, e.g. same flight mode but new target velocity
void PathFollowerControlLanding::ObjectiveUpdated(void)
{
    // Set the objective's target velocity
    controlThrust.UpdateVelocitySetpoint(pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_VELOCITY_VELOCITYVECTOR_DOWN]);
    controlNE.UpdateVelocitySetpoint(pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_VELOCITY_VELOCITYVECTOR_NORTH],
                                     pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_VELOCITY_VELOCITYVECTOR_EAST]);
    controlNE.UpdatePositionSetpoint(pathDesired->End.North, pathDesired->End.East);
}
void PathFollowerControlLanding::Deactivate(void)
{
    controlThrust.Deactivate();
    controlNE.Deactivate();
    fsm->Inactive();
}


void PathFollowerControlLanding::SettingsUpdated(void)
{
    const float dT = vtolPathFollowerSettings->UpdatePeriod / 1000.0f;

    controlNE.UpdateParameters( vtolPathFollowerSettings->HorizontalVelPID.Kp,
                                vtolPathFollowerSettings->HorizontalVelPID.Ki,
                                vtolPathFollowerSettings->HorizontalVelPID.Kd,
                                vtolPathFollowerSettings->HorizontalVelPID.ILimit,
                                dT,
                                vtolPathFollowerSettings->HorizontalVelMax);


    controlNE.UpdatePositionalParameters(  vtolPathFollowerSettings->HorizontalPosP );
    controlNE.UpdateCommandParameters( -vtolPathFollowerSettings->MaxRollPitch, vtolPathFollowerSettings->MaxRollPitch, vtolPathFollowerSettings->VelocityFeedforward);

    controlThrust.UpdateParameters(vtolPathFollowerSettings->VerticalVelPID.Kp,
                                   vtolPathFollowerSettings->VerticalVelPID.Ki,
                                   vtolPathFollowerSettings->VerticalVelPID.Kd,
                                   vtolPathFollowerSettings->VerticalVelPID.ILimit,
                                   dT,
                                   vtolPathFollowerSettings->VerticalVelMax);
    // TODO Add trigger for this
    VtolSelfTuningStatsData vtolSelfTuningStats;
    VtolSelfTuningStatsGet(&vtolSelfTuningStats);
    controlThrust.UpdateNeutralThrust(vtolSelfTuningStats.NeutralThrustOffset + vtolPathFollowerSettings->ThrustLimits.Neutral);
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
    pathStatus   = ptr_pathStatus;
    controlThrust.Initialize(fsm);

    return 0;
}


void PathFollowerControlLanding::UpdateVelocityDesired()
{
    VelocityStateData velocityState;
    VelocityStateGet(&velocityState);
    VelocityDesiredData velocityDesired;

    controlThrust.UpdateVelocityState(velocityState.Down);
    controlNE.UpdateVelocityState(velocityState.North, velocityState.East);

    // Implement optional horizontal position hold.
    if (((uint8_t)pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_LAND_OPTIONS]) == PATHDESIRED_MODEPARAMETER_LAND_OPTION_HORIZONTAL_PH) {

	// landing flight mode has stored original horizontal position in pathdesired
	PositionStateData positionState;
	PositionStateGet(&positionState);
        controlNE.UpdatePositionState(positionState.North, positionState.East);
        controlNE.ControlPosition();
    }

    velocityDesired.Down = controlThrust.GetVelocityDesired();
    float north, east;
    controlNE.GetVelocityDesired(north, east);
    velocityDesired.North = north;
    velocityDesired.East = east;

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

int8_t PathFollowerControlLanding::UpdateStabilizationDesired(bool yaw_attitude, float yaw_direction)
{
    uint8_t result = 1;
    StabilizationDesiredData stabDesired;
    AttitudeStateData attitudeState;
    StabilizationBankData stabSettings;
    float northCommand;
    float eastCommand;

    StabilizationDesiredGet(&stabDesired);
    AttitudeStateGet(&attitudeState);
    StabilizationBankGet(&stabSettings);

    controlNE.GetNECommand(northCommand, eastCommand);
    stabDesired.Thrust = controlThrust.GetThrustCommand();

    float angle_radians = DEG2RAD(attitudeState.Yaw);
    float cos_angle     = cosf(angle_radians);
    float sine_angle    = sinf(angle_radians);
    float maxPitch = vtolPathFollowerSettings->MaxRollPitch;
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

    fsm->ConstrainStabiDesired(&stabDesired); // excludes thrust
    StabilizationDesiredSet(&stabDesired);

    return result;
}

void PathFollowerControlLanding::UpdateAutoPilot()
{
    fsm->Update();

    UpdateVelocityDesired();

    // yaw behaviour is configurable in vtolpathfollower, select yaw control algorithm
    bool yaw_attitude = true;
    float yaw = 0.0f;

    fsm->GetYaw(yaw_attitude, yaw);

    int8_t result = UpdateStabilizationDesired(yaw_attitude, yaw);

    if (!result) {
        fsm->Abort();
    }

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
