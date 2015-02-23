/*
 ******************************************************************************
 *
 * @file       PathFollowerControlVelocityRoam.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      This landing state machine is a helper state machine to the
 *              pathfollower task/thread to implement detailed landing controls.
 *		This is to be called only from the pathfollower task.
 *		Note that initiation of the land occurs in the manual control
 *		command thread calling plans.c plan_setup_land which writes
 *		the required PathDesired VELOCITYROAM mode.
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
#include "PathFollowerControlVelocityRoam.h"
#include "PIDControlNE.h"

// Private constants

// pointer to a singleton instance
PathFollowerControlVelocityRoam *PathFollowerControlVelocityRoam::p_inst = 0;

PathFollowerControlVelocityRoam::PathFollowerControlVelocityRoam()
    : vtolPathFollowerSettings(0), pathDesired(0), pathStatus(0), mActive(false)
{}

// Called when mode first engaged
void PathFollowerControlVelocityRoam::Activate(void)
{
    if (!mActive) {
        mActive = true;
        SettingsUpdated();
        controlNE.Activate();
    }
}

uint8_t PathFollowerControlVelocityRoam::IsActive(void)
{
    return mActive;
}

uint8_t PathFollowerControlVelocityRoam::Mode(void)
{
    return PATHDESIRED_MODE_VELOCITY;
}

void PathFollowerControlVelocityRoam::ObjectiveUpdated(void)
{
    controlNE.UpdateVelocitySetpoint(pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_VELOCITY_VELOCITYVECTOR_NORTH],
                                     pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_VELOCITY_VELOCITYVECTOR_EAST]);
}

void PathFollowerControlVelocityRoam::Deactivate(void)
{
    if (mActive) {
        mActive = false;
        controlNE.Deactivate();
    }
}

void PathFollowerControlVelocityRoam::SettingsUpdated(void)
{
    const float dT = vtolPathFollowerSettings->UpdatePeriod / 1000.0f;

    controlNE.UpdateParameters(vtolPathFollowerSettings->HorizontalVelPID.Kp,
                               vtolPathFollowerSettings->HorizontalVelPID.Ki,
                               vtolPathFollowerSettings->HorizontalVelPID.Kd,
                               vtolPathFollowerSettings->HorizontalVelPID.ILimit,
                               dT,
                               vtolPathFollowerSettings->HorizontalVelMax);


    controlNE.UpdatePositionalParameters(vtolPathFollowerSettings->HorizontalPosP);
    controlNE.UpdateCommandParameters(-vtolPathFollowerSettings->MaxRollPitch, vtolPathFollowerSettings->MaxRollPitch, vtolPathFollowerSettings->VelocityFeedforward);
}

int32_t PathFollowerControlVelocityRoam::Initialize(VtolPathFollowerSettingsData *ptr_vtolPathFollowerSettings,
                                                    PathDesiredData *ptr_pathDesired,
                                                    PathStatusData *ptr_pathStatus)
{
    PIOS_Assert(ptr_vtolPathFollowerSettings);
    PIOS_Assert(ptr_pathDesired);
    PIOS_Assert(ptr_pathStatus);

    vtolPathFollowerSettings = ptr_vtolPathFollowerSettings;
    pathDesired = ptr_pathDesired;
    pathStatus  = ptr_pathStatus;
    return 0;
}


void PathFollowerControlVelocityRoam::UpdateVelocityDesired()
{
    VelocityStateData velocityState;

    VelocityStateGet(&velocityState);
    VelocityDesiredData velocityDesired;

    controlNE.UpdateVelocityState(velocityState.North, velocityState.East);

    velocityDesired.Down  = 0.0f;
    float north, east;
    controlNE.GetVelocityDesired(&north, &east);
    velocityDesired.North = north;
    velocityDesired.East  = east;

    // update pathstatus
    pathStatus->error     = 0.0f;
    pathStatus->fractional_progress  = 0.0f;
    pathStatus->path_direction_north = velocityDesired.North;
    pathStatus->path_direction_east  = velocityDesired.East;
    pathStatus->path_direction_down  = velocityDesired.Down;

    pathStatus->correction_direction_north = velocityDesired.North - velocityState.North;
    pathStatus->correction_direction_east  = velocityDesired.East - velocityState.East;
    pathStatus->correction_direction_down  = 0.0f;

    VelocityDesiredSet(&velocityDesired);
}

int8_t PathFollowerControlVelocityRoam::UpdateStabilizationDesired(__attribute__((unused)) bool yaw_attitude, __attribute__((unused)) float yaw_direction)
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

    controlNE.GetNECommand(&northCommand, &eastCommand);

    float angle_radians = DEG2RAD(attitudeState.Yaw);
    float cos_angle     = cosf(angle_radians);
    float sine_angle    = sinf(angle_radians);
    float maxPitch = vtolPathFollowerSettings->MaxRollPitch;
    stabDesired.StabilizationMode.Pitch = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.Pitch = boundf(-northCommand * cos_angle - eastCommand * sine_angle, -maxPitch, maxPitch);
    stabDesired.StabilizationMode.Roll  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.Roll = boundf(-northCommand * sine_angle + eastCommand * cos_angle, -maxPitch, maxPitch);

    ManualControlCommandData manualControl;
    ManualControlCommandGet(&manualControl);

    stabDesired.StabilizationMode.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;
    stabDesired.Yaw = stabSettings.MaximumRate.Yaw * manualControl.Yaw;

    // default thrust mode to cruise control
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEVARIO;

    StabilizationDesiredSet(&stabDesired);

    return result;
}

void PathFollowerControlVelocityRoam::UpdateAutoPilot()
{
    UpdateVelocityDesired();

    bool yaw_attitude = false;
    float yaw = 0.0f;

    int8_t result     = UpdateStabilizationDesired(yaw_attitude, yaw);

    if (!result) {
        fallback_to_hold();
    }

    PathStatusSet(pathStatus);
}

void PathFollowerControlVelocityRoam::fallback_to_hold(void)
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
    pathDesired->Mode = PATHDESIRED_MODE_FLYENDPOINT;

    PathDesiredSet(pathDesired);
}
