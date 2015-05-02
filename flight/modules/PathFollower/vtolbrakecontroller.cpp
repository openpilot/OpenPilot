/*
 ******************************************************************************
 *
 * @file       vtolbrakecontroller.cpp
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

#include <accelstate.h>
#include <vtolpathfollowersettings.h>
#include <flightstatus.h>
#include <flightmodesettings.h>
#include <pathstatus.h>
#include <positionstate.h>
#include <velocitystate.h>
#include <velocitydesired.h>
#include <stabilizationdesired.h>
#include <attitudestate.h>
#include <takeofflocation.h>
#include <manualcontrolcommand.h>
#include <systemsettings.h>
#include <stabilizationbank.h>
#include <stabilizationdesired.h>
#include <vtolselftuningstats.h>
#include <pathsummary.h>
}

// C++ includes
#include "vtolbrakecontroller.h"
#include "pathfollowerfsm.h"
#include "vtolbrakefsm.h"
#include "pidcontroldown.h"

// Private constants
#define BRAKE_RATE_MINIMUM 0.2f

// pointer to a singleton instance
VtolBrakeController *VtolBrakeController::p_inst = 0;

VtolBrakeController::VtolBrakeController()
    : fsm(0), vtolPathFollowerSettings(0), mActive(false), mHoldActive(false), mManualThrust(false)
{}

// Called when mode first engaged
void VtolBrakeController::Activate(void)
{
    if (!mActive) {
        mActive       = true;
        mHoldActive   = false;
        mManualThrust = false;
        SettingsUpdated();
        fsm->Activate();
        controlDown.Activate();
        controlNE.Activate();
    }
}

uint8_t VtolBrakeController::IsActive(void)
{
    return mActive;
}

uint8_t VtolBrakeController::Mode(void)
{
    return PATHDESIRED_MODE_BRAKE;
}

// Objective updated in pathdesired
void VtolBrakeController::ObjectiveUpdated(void)
{
    // Set the objective's target velocity
    controlDown.UpdateVelocitySetpoint(pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_BRAKE_STARTVELOCITYVECTOR_DOWN]);
    controlNE.UpdateVelocitySetpoint(pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_BRAKE_STARTVELOCITYVECTOR_NORTH],
                                     pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_BRAKE_STARTVELOCITYVECTOR_EAST]);
    if (pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_BRAKE_TIMEOUT] > 0.0f) {
        pathStatus->path_time = 0.0f;
    }
}
void VtolBrakeController::Deactivate(void)
{
    if (mActive) {
        mActive       = false;
        mHoldActive   = false;
        mManualThrust = false;
        fsm->Inactive();
        controlDown.Deactivate();
        controlNE.Deactivate();
    }
}


void VtolBrakeController::SettingsUpdated(void)
{
    const float dT = vtolPathFollowerSettings->UpdatePeriod / 1000.0f;

    controlNE.UpdateParameters(vtolPathFollowerSettings->BrakeHorizontalVelPID.Kp,
                               vtolPathFollowerSettings->BrakeHorizontalVelPID.Ki,
                               vtolPathFollowerSettings->BrakeHorizontalVelPID.Kd,
                               vtolPathFollowerSettings->BrakeHorizontalVelPID.ILimit,
                               dT,
                               10.0f * vtolPathFollowerSettings->HorizontalVelMax); // avoid constraining initial fast entry into brake
    controlNE.UpdatePositionalParameters(vtolPathFollowerSettings->HorizontalPosP);
    controlNE.UpdateCommandParameters(-vtolPathFollowerSettings->BrakeMaxPitch, vtolPathFollowerSettings->BrakeMaxPitch, vtolPathFollowerSettings->BrakeVelocityFeedforward);

    controlDown.UpdateParameters(vtolPathFollowerSettings->LandVerticalVelPID.Kp,
                                 vtolPathFollowerSettings->LandVerticalVelPID.Ki,
                                 vtolPathFollowerSettings->LandVerticalVelPID.Kd,
                                 vtolPathFollowerSettings->LandVerticalVelPID.Beta,
                                 dT,
                                 10.0f * vtolPathFollowerSettings->VerticalVelMax); // avoid constraining initial fast entry into brake
    controlDown.UpdatePositionalParameters(vtolPathFollowerSettings->VerticalPosP);
    controlDown.SetThrustLimits(vtolPathFollowerSettings->ThrustLimits.Min, vtolPathFollowerSettings->ThrustLimits.Max);

    VtolSelfTuningStatsData vtolSelfTuningStats;
    VtolSelfTuningStatsGet(&vtolSelfTuningStats);
    controlDown.UpdateNeutralThrust(vtolSelfTuningStats.NeutralThrustOffset + vtolPathFollowerSettings->ThrustLimits.Neutral);
    fsm->SettingsUpdated();
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t VtolBrakeController::Initialize(VtolPathFollowerSettingsData *ptr_vtolPathFollowerSettings)
{
    PIOS_Assert(ptr_vtolPathFollowerSettings);
    vtolPathFollowerSettings = ptr_vtolPathFollowerSettings;
    if (fsm == 0) {
        VtolBrakeFSM::instance()->Initialize(vtolPathFollowerSettings, pathDesired, flightStatus, pathStatus);
        fsm = (PathFollowerFSM *)VtolBrakeFSM::instance();
        controlDown.Initialize(fsm);
    }
    return 0;
}


void VtolBrakeController::UpdateVelocityDesired()
{
    VelocityStateData velocityState;

    VelocityStateGet(&velocityState);
    VelocityDesiredData velocityDesired;

    float brakeRate = vtolPathFollowerSettings->BrakeRate;
    if (brakeRate < BRAKE_RATE_MINIMUM) {
        brakeRate = BRAKE_RATE_MINIMUM; // set a minimum to avoid a divide by zero potential below
    }

    if (fsm->PositionHoldState()) {
        PositionStateData positionState;
        PositionStateGet(&positionState);

        // On first engagement set the position setpoint
        if (!mHoldActive) {
            mHoldActive = true;
            controlNE.UpdatePositionSetpoint(positionState.North, positionState.East);
            if (!mManualThrust) {
                controlDown.UpdatePositionSetpoint(positionState.Down);
            }
        }

        // Update position state and control position to create inputs to velocity control
        controlNE.UpdatePositionState(positionState.North, positionState.East);
        controlNE.ControlPosition();
        if (!mManualThrust) {
            controlDown.UpdatePositionState(positionState.Down);
            controlDown.ControlPosition();
        }

        controlNE.UpdateVelocityState(velocityState.North, velocityState.East);
        if (!mManualThrust) {
            controlDown.UpdateVelocityState(velocityState.Down);
        }
    } else {
        controlNE.UpdateVelocityStateWithBrake(velocityState.North, velocityState.East, pathStatus->path_time, brakeRate);
        if (!mManualThrust) {
            controlDown.UpdateVelocityStateWithBrake(velocityState.Down, pathStatus->path_time, brakeRate);
        }
    }

    if (!mManualThrust) {
        velocityDesired.Down = controlDown.GetVelocityDesired();
    } else { velocityDesired.Down = 0.0f; }

    float north, east;
    controlNE.GetVelocityDesired(&north, &east);
    velocityDesired.North = north;
    velocityDesired.East  = east;

    VelocityDesiredSet(&velocityDesired);

    // update pathstatus
    float cur_velocity     = velocityState.North * velocityState.North + velocityState.East * velocityState.East + velocityState.Down * velocityState.Down;
    cur_velocity      = sqrtf(cur_velocity);
    float desired_velocity = velocityDesired.North * velocityDesired.North + velocityDesired.East * velocityDesired.East + velocityDesired.Down * velocityDesired.Down;
    desired_velocity  = sqrtf(desired_velocity);
    pathStatus->error = cur_velocity - desired_velocity;
    pathStatus->fractional_progress = 1.0f;
    if (pathDesired->StartingVelocity > 0.0f) {
        pathStatus->fractional_progress = (pathDesired->StartingVelocity - cur_velocity) / pathDesired->StartingVelocity;

        // sometimes current velocity can exceed starting velocity due to initial acceleration
        if (pathStatus->fractional_progress < 0.0f) {
            pathStatus->fractional_progress = 0.0f;
        }
    }
    pathStatus->path_direction_north = velocityDesired.North;
    pathStatus->path_direction_east  = velocityDesired.East;
    pathStatus->path_direction_down  = velocityDesired.Down;

    pathStatus->correction_direction_north = velocityDesired.North - velocityState.North;
    pathStatus->correction_direction_east  = velocityDesired.East - velocityState.East;
}

int8_t VtolBrakeController::UpdateStabilizationDesired(void)
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
    float maxPitch = vtolPathFollowerSettings->BrakeMaxPitch;
    stabDesired.StabilizationMode.Pitch = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.Pitch = boundf(-northCommand * cos_angle - eastCommand * sine_angle, -maxPitch, maxPitch); // this should be in the controller
    stabDesired.StabilizationMode.Roll  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.Roll = boundf(-northCommand * sine_angle + eastCommand * cos_angle, -maxPitch, maxPitch);

    ManualControlCommandData manualControl;
    ManualControlCommandGet(&manualControl);

    stabDesired.StabilizationMode.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;
    stabDesired.Yaw = stabSettings.MaximumRate.Yaw * manualControl.Yaw;

    // default thrust mode to cruise control
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL;

    // when flight mode assist is active but in primary-thrust mode, the thrust mode must be set to the same as per the primary mode.
    if (flightStatus->FlightModeAssist == FLIGHTSTATUS_FLIGHTMODEASSIST_GPSASSIST_PRIMARYTHRUST) {
        FlightModeSettingsData settings;
        FlightModeSettingsGet(&settings);
        uint8_t thrustMode = FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_CRUISECONTROL;

        switch (flightStatus->FlightMode) {
        case FLIGHTSTATUS_FLIGHTMODE_STABILIZED1:
            thrustMode = settings.Stabilization1Settings.Thrust;
            break;
        case FLIGHTSTATUS_FLIGHTMODE_STABILIZED2:
            thrustMode = settings.Stabilization2Settings.Thrust;
            break;
        case FLIGHTSTATUS_FLIGHTMODE_STABILIZED3:
            thrustMode = settings.Stabilization3Settings.Thrust;
            break;
        case FLIGHTSTATUS_FLIGHTMODE_STABILIZED4:
            thrustMode = settings.Stabilization4Settings.Thrust;
            break;
        case FLIGHTSTATUS_FLIGHTMODE_STABILIZED5:
            thrustMode = settings.Stabilization5Settings.Thrust;
            break;
        case FLIGHTSTATUS_FLIGHTMODE_STABILIZED6:
            thrustMode = settings.Stabilization6Settings.Thrust;
            break;
        case FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD:
            // we hard code the "GPS Assisted" PostionHold/Roam to use alt-vario which
            // is a more appropriate throttle mode.  "GPSAssist" adds braking
            // and a better throttle management to the standard Position Hold.
            thrustMode = FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_ALTITUDEVARIO;
            break;
        default:
            break;
        }
        stabDesired.StabilizationMode.Thrust = (StabilizationDesiredStabilizationModeOptions)thrustMode;
    }

    // set the thrust value
    if (mManualThrust) {
        stabDesired.Thrust = manualControl.Thrust;
    } else {
        stabDesired.Thrust = controlDown.GetDownCommand();
    }

    StabilizationDesiredSet(&stabDesired);

    return result;
}


void VtolBrakeController::UpdateAutoPilot()
{
    if (pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_BRAKE_TIMEOUT] > 0.0f) {
        pathStatus->path_time += vtolPathFollowerSettings->UpdatePeriod / 1000.0f;
    }

    if (flightStatus->FlightModeAssist && flightStatus->AssistedThrottleState == FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL) {
        mManualThrust = true;
    } else {
        mManualThrust = false;
    }

    fsm->Update(); // This will run above end condition checks

    UpdateVelocityDesired();

    int8_t result = UpdateStabilizationDesired();

    if (!result) {
        fsm->Abort(); // enter PH
    }

    PathStatusSet(pathStatus);
}
