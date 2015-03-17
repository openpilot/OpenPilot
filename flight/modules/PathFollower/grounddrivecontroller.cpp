/*
 ******************************************************************************
 *
 * @file       grounddrivecontroller.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Ground drive controller
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
#include <groundpathfollowersettings.h>
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
#include <pathsummary.h>
#include <statusgrounddrive.h>
}

// C++ includes
#include "grounddrivecontroller.h"

// Private constants

// pointer to a singleton instance
GroundDriveController *GroundDriveController::p_inst = 0;

GroundDriveController::GroundDriveController()
    : groundSettings(0), mActive(false)
{}

// Called when mode first engaged
void GroundDriveController::Activate(void)
{
    if (!mActive) {
        mActive = true;
        SettingsUpdated();
        controlNE.Activate();
        resetGlobals();
        mMode = pathDesired->Mode;
    }
}

uint8_t GroundDriveController::IsActive(void)
{
    return mActive;
}

uint8_t GroundDriveController::Mode(void)
{
    return mMode;
}

// Objective updated in pathdesired
void GroundDriveController::ObjectiveUpdated(void)
{}

void GroundDriveController::Deactivate(void)
{
    if (mActive) {
        mActive = false;
        resetGlobals();
        controlNE.Deactivate();
    }
}


void GroundDriveController::SettingsUpdated(void)
{
    const float dT = groundSettings->UpdatePeriod / 1000.0f;

    controlNE.UpdatePositionalParameters(groundSettings->HorizontalPosP);
    controlNE.UpdateParameters(groundSettings->SpeedPI.Kp,
                               groundSettings->SpeedPI.Ki,
                               groundSettings->SpeedPI.Kd,
                               groundSettings->SpeedPI.Beta,
                               dT,
                               groundSettings->HorizontalVelMax);


    // max/min are NE command values equivalent to thrust but must be symmetrical as this is NE not forward/reverse.
    controlNE.UpdateCommandParameters(-groundSettings->ThrustLimit.Max, groundSettings->ThrustLimit.Max, groundSettings->VelocityFeedForward);
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t GroundDriveController::Initialize(GroundPathFollowerSettingsData *ptr_groundSettings)
{
    PIOS_Assert(ptr_groundSettings);

    groundSettings = ptr_groundSettings;

    resetGlobals();

    return 0;
}

/**
 * reset integrals
 */
void GroundDriveController::resetGlobals()
{
    pathStatus->path_time = 0.0f;
}

void GroundDriveController::UpdateAutoPilot()
{
    uint8_t result = updateAutoPilotGround();

    if (result) {
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);
    } else {
        pathStatus->Status = PATHSTATUS_STATUS_CRITICAL;
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_WARNING);
    }

    PathStatusSet(pathStatus);
}

/**
 * fixed wing autopilot:
 * straight forward:
 * 1. update path velocity for limited motion crafts
 * 2. update attitude according to default fixed wing pathfollower algorithm
 */
uint8_t GroundDriveController::updateAutoPilotGround()
{
    updatePathVelocity(groundSettings->CourseFeedForward);
    return updateGroundDesiredAttitude();
}

/**
 * Compute desired velocity from the current position and path
 */
void GroundDriveController::updatePathVelocity(float kFF)
{
    PositionStateData positionState;

    PositionStateGet(&positionState);
    VelocityStateData velocityState;
    VelocityStateGet(&velocityState);
    VelocityDesiredData velocityDesired;
    controlNE.UpdateVelocityState(velocityState.North, velocityState.East);

    // look ahead kFF seconds
    float cur[3] = { positionState.North + (velocityState.North * kFF),
                     positionState.East + (velocityState.East * kFF),
                     positionState.Down + (velocityState.Down * kFF) };
    struct path_status progress;
    path_progress(pathDesired, cur, &progress, false);

    // GOTOENDPOINT: correction_vector is distance array to endpoint, path_vector is velocity vector
    // FOLLOWVECTOR:  correct_vector is distance to vector path, path_vector is the desired velocity vector

    // Calculate the desired velocity from the lateral vector path errors (correct_vector) and the desired velocity vector (path_vector)
    controlNE.ControlPositionWithPath(&progress);
    float north, east;
    controlNE.GetVelocityDesired(&north, &east);
    velocityDesired.North = north;
    velocityDesired.East  = east;
    velocityDesired.Down  = 0.0f;

#if 0
    if (limited &&
        // if a plane is crossing its desired flightpath facing the wrong way (away from flight direction)
        // it would turn towards the flightpath to get on its desired course. This however would reverse the correction vector
        // once it crosses the flightpath again, which would make it again turn towards the flightpath (but away from its desired heading)
        // leading to an S-shape snake course the wrong way
        // this only happens especially if HorizontalPosP is too high, as otherwise the angle between velocity desired and path_direction won't
        // turn steep unless there is enough space complete the turn before crossing the flightpath
        // in this case the plane effectively needs to be turned around
        // indicators:
        // difference between correction_direction and velocitystate >90 degrees and
        // difference between path_direction and velocitystate >90 degrees  ( 4th sector, facing away from everything )
        // fix: ignore correction, steer in path direction until the situation has become better (condition doesn't apply anymore)
        // calculating angles < 90 degrees through dot products
        (vector_lengthf(progress.path_vector, 2) > 1e-6f) &&
        ((progress.path_vector[0] * velocityState.North + progress.path_vector[1] * velocityState.East) < 0.0f) &&
        ((progress.correction_vector[0] * velocityState.North + progress.correction_vector[1] * velocityState.East) < 0.0f)) {
        ;
    }
#endif


    // update pathstatus
    pathStatus->error = progress.error;
    pathStatus->fractional_progress  = progress.fractional_progress;
    // FOLLOWVECTOR: desired velocity vector
    pathStatus->path_direction_north = progress.path_vector[0];
    pathStatus->path_direction_east  = progress.path_vector[1];
    pathStatus->path_direction_down  = progress.path_vector[2];

    // FOLLOWVECTOR: correction distance to vector path
    pathStatus->correction_direction_north = progress.correction_vector[0];
    pathStatus->correction_direction_east  = progress.correction_vector[1];
    pathStatus->correction_direction_down  = progress.correction_vector[2];

    VelocityDesiredSet(&velocityDesired);
}

/**
 * Compute desired attitude for ground vehicles
 */
uint8_t GroundDriveController::updateGroundDesiredAttitude()
{
    StatusGroundDriveData statusGround;
    VelocityStateData velocityState;

    VelocityStateGet(&velocityState);
    AttitudeStateData attitudeState;
    AttitudeStateGet(&attitudeState);
    statusGround.State.Yaw = attitudeState.Yaw;
    statusGround.State.Velocity = sqrtf(velocityState.North * velocityState.North + velocityState.East * velocityState.East);

    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);

    // estimate a north/east command value to control the velocity error.
    // ThrustLimits.Max(+-) limits the range.  Think of this as a command unit vector
    // of the ultimate direction to reduce lateral error and achieve the target direction (desired angle).
    float northCommand, eastCommand;
    controlNE.GetNECommand(&northCommand, &eastCommand);

    // Get current vehicle orientation
    float angle_radians  = DEG2RAD(attitudeState.Yaw); // (+-pi)
    float cos_angle      = cosf(angle_radians);
    float sine_angle     = sinf(angle_radians);

    float courseCommand  = 0.0f;
    float speedCommand   = 0.0f;
    float lateralCommand = boundf(-northCommand * sine_angle + eastCommand * cos_angle, -groundSettings->ThrustLimit.Max, groundSettings->ThrustLimit.Max);
    float forwardCommand = boundf(northCommand * cos_angle + eastCommand * sine_angle, -groundSettings->ThrustLimit.Max, groundSettings->ThrustLimit.Max);
    // +ve facing correct direction, lateral command should just correct angle,
    if (forwardCommand >= 0.1f) {
        // if +ve forward command, -+ lateralCommand drives steering to manage lateral error and angular error

        courseCommand = boundf(lateralCommand, -1.0f, 1.0f);
        speedCommand  = boundf(forwardCommand, groundSettings->ThrustLimit.SlowForward, groundSettings->ThrustLimit.Max);

        // reinstate max thrust
        controlNE.UpdateCommandParameters(-groundSettings->ThrustLimit.Max, groundSettings->ThrustLimit.Max, groundSettings->VelocityFeedForward);

        statusGround.ControlState = STATUSGROUNDDRIVE_CONTROLSTATE_ONTRACK;
    } else {
        // -ve facing opposite direction, lateral command irrelevant, need to turn to change direction and do so slowly.

        // Reduce steering angle based on current velocity
        float steeringReductionFactor = 1.0f;
        if (stabDesired.Thrust > 0.3f) {
            steeringReductionFactor = (groundSettings->HorizontalVelMax - statusGround.State.Velocity) / groundSettings->HorizontalVelMax;
            steeringReductionFactor = boundf(steeringReductionFactor, 0.05f, 1.0f);
        }

        // should we turn left or right?
        if (lateralCommand >= 0.1f) {
            courseCommand = 1.0f * steeringReductionFactor;
            statusGround.ControlState = STATUSGROUNDDRIVE_CONTROLSTATE_TURNAROUNDRIGHT;
        } else {
            courseCommand = -1.0f * steeringReductionFactor;
            statusGround.ControlState = STATUSGROUNDDRIVE_CONTROLSTATE_TURNAROUNDLEFT;
        }

        // Impose limits to slow down.
        controlNE.UpdateCommandParameters(-groundSettings->ThrustLimit.SlowForward, groundSettings->ThrustLimit.SlowForward, 0.0f);

        speedCommand = groundSettings->ThrustLimit.SlowForward;
    }

    stabDesired.Roll   = 0.0f;
    stabDesired.Pitch  = 0.0f;
    stabDesired.Yaw    = courseCommand;

    // Mix yaw into thrust limit TODO
    stabDesired.Thrust = boundf(speedCommand, groundSettings->ThrustLimit.Min, groundSettings->ThrustLimit.Max);

    stabDesired.StabilizationMode.Roll   = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;
    stabDesired.StabilizationMode.Pitch  = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;
    stabDesired.StabilizationMode.Yaw    = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;
    StabilizationDesiredSet(&stabDesired);

    statusGround.NECommand.North       = northCommand;
    statusGround.NECommand.East        = eastCommand;
    statusGround.State.Thrust          = stabDesired.Thrust;
    statusGround.BodyCommand.Forward   = forwardCommand;
    statusGround.BodyCommand.Right     = lateralCommand;
    statusGround.ControlCommand.Course = courseCommand;
    statusGround.ControlCommand.Speed  = speedCommand;
    StatusGroundDriveSet(&statusGround);

    return 1;
}
