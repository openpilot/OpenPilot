/*
 ******************************************************************************
 *
 * @file       vtollandcontroller.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Vtol landing controller loop
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
#include <statusvtolautotakeoff.h>
#include <pathsummary.h>
}

// C++ includes
#include "vtolautotakeoffcontroller.h"
#include "pathfollowerfsm.h"
#include "vtolautotakeofffsm.h"
#include "pidcontroldown.h"

// Private constants
#define AUTOTAKEOFF_TO_INCREMENTAL_HEIGHT_MIN             2.0f
#define AUTOTAKEOFF_TO_INCREMENTAL_HEIGHT_MAX             50.0f
#define AUTOTAKEOFF_INFLIGHT_THROTTLE_CHECK_LIMIT         0.2f
#define AUTOTAKEOFF_THROTTLE_LIMIT_TO_ALLOW_TAKEOFF_START 0.3f
#define AUTOTAKEOFF_THROTTLE_ABORT_LIMIT                  0.1f

// pointer to a singleton instance
VtolAutoTakeoffController *VtolAutoTakeoffController::p_inst = 0;

VtolAutoTakeoffController::VtolAutoTakeoffController()
    : fsm(0), vtolPathFollowerSettings(0), mActive(false)
{}

// Called when mode first engaged
void VtolAutoTakeoffController::Activate(void)
{
    if (!mActive) {
        mActive   = true;
        mOverride = true;
        SettingsUpdated();
        fsm->Activate();
        controlDown.Activate();
        controlNE.Activate();
        autotakeoffState = STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_WAITFORARMED;
        // We only allow takeoff if the state transition of disarmed to armed occurs
        // whilst in the autotake flight mode
        FlightStatusData flightStatus;
        FlightStatusGet(&flightStatus);
        StabilizationDesiredData stabiDesired;
        StabilizationDesiredGet(&stabiDesired);

        if (flightStatus.Armed) {
            // Are we inflight?
            if (stabiDesired.Thrust > AUTOTAKEOFF_INFLIGHT_THROTTLE_CHECK_LIMIT || flightStatus.ControlChain.PathPlanner == FLIGHTSTATUS_CONTROLCHAIN_TRUE) {
                // ok assume already in flight and just enter position hold
                // if we are not actually inflight this will just be a violent autotakeoff
                autotakeoffState = STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_POSITIONHOLD;
            } else {
                autotakeoffState = STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_REQUIREUNARMEDFIRST;
                // Note that if this mode was invoked unintentionally whilst in flight, effectively
                // all inputs get ignored and the vtol continues to fly to its previous
                // stabi command.
            }
        }
        fsm->setControlState(autotakeoffState);
    }
}

uint8_t VtolAutoTakeoffController::IsActive(void)
{
    return mActive;
}

uint8_t VtolAutoTakeoffController::Mode(void)
{
    return PATHDESIRED_MODE_AUTOTAKEOFF;
}

// Objective updated in pathdesired, e.g. same flight mode but new target velocity
void VtolAutoTakeoffController::ObjectiveUpdated(void)
{
    if (mOverride) {
        // override pathDesired from PathPLanner with current position,
        // as we deliberately don' not care about the location of the waypoints on the map
        float velocity_down;
        float autotakeoff_height;
        PositionStateData positionState;
        PositionStateGet(&positionState);
        FlightModeSettingsAutoTakeOffVelocityGet(&velocity_down);
        FlightModeSettingsAutoTakeOffHeightGet(&autotakeoff_height);
        autotakeoff_height = fabsf(autotakeoff_height);
        if (autotakeoff_height < AUTOTAKEOFF_TO_INCREMENTAL_HEIGHT_MIN) {
            autotakeoff_height = AUTOTAKEOFF_TO_INCREMENTAL_HEIGHT_MIN;
        } else if (autotakeoff_height > AUTOTAKEOFF_TO_INCREMENTAL_HEIGHT_MAX) {
            autotakeoff_height = AUTOTAKEOFF_TO_INCREMENTAL_HEIGHT_MAX;
        }
        controlDown.UpdateVelocitySetpoint(velocity_down);
        controlNE.UpdateVelocitySetpoint(0.0f, 0.0f);
        controlNE.UpdatePositionSetpoint(positionState.North, positionState.East);


        controlDown.UpdatePositionSetpoint(positionState.Down - autotakeoff_height);
        mOverride = false; // further updates always come from ManualControl and will control horizontal position

        mOverride = false;
    } else {
        // Set the objective's target velocity

        controlDown.UpdateVelocitySetpoint(pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_AUTOTAKEOFF_DOWN]);
        controlNE.UpdateVelocitySetpoint(pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_AUTOTAKEOFF_NORTH],
                                         pathDesired->ModeParameters[PATHDESIRED_MODEPARAMETER_AUTOTAKEOFF_EAST]);
        controlNE.UpdatePositionSetpoint(pathDesired->End.North, pathDesired->End.East);
        controlDown.UpdatePositionSetpoint(pathDesired->End.Down);
    }
}
void VtolAutoTakeoffController::Deactivate(void)
{
    if (mActive) {
        mActive = false;
        fsm->Inactive();
        controlDown.Deactivate();
        controlNE.Deactivate();
    }
}

// AutoTakeoff Uses different vertical velocity PID.
void VtolAutoTakeoffController::SettingsUpdated(void)
{
    const float dT = vtolPathFollowerSettings->UpdatePeriod / 1000.0f;

    controlNE.UpdateParameters(vtolPathFollowerSettings->HorizontalVelPID.Kp,
                               vtolPathFollowerSettings->HorizontalVelPID.Ki,
                               vtolPathFollowerSettings->HorizontalVelPID.Kd,
                               vtolPathFollowerSettings->HorizontalVelPID.Beta,
                               dT,
                               vtolPathFollowerSettings->HorizontalVelMax);


    controlNE.UpdatePositionalParameters(vtolPathFollowerSettings->HorizontalPosP);
    controlNE.UpdateCommandParameters(-vtolPathFollowerSettings->MaxRollPitch, vtolPathFollowerSettings->MaxRollPitch, vtolPathFollowerSettings->VelocityFeedforward);

    controlDown.UpdateParameters(vtolPathFollowerSettings->AutoTakeoffVerticalVelPID.Kp,
                                 vtolPathFollowerSettings->AutoTakeoffVerticalVelPID.Ki,
                                 vtolPathFollowerSettings->AutoTakeoffVerticalVelPID.Kd,
                                 vtolPathFollowerSettings->AutoTakeoffVerticalVelPID.Beta,
                                 dT,
                                 vtolPathFollowerSettings->VerticalVelMax);
    controlDown.UpdatePositionalParameters(vtolPathFollowerSettings->VerticalPosP);
    VtolSelfTuningStatsData vtolSelfTuningStats;
    VtolSelfTuningStatsGet(&vtolSelfTuningStats);
    controlDown.UpdateNeutralThrust(vtolSelfTuningStats.NeutralThrustOffset + vtolPathFollowerSettings->ThrustLimits.Neutral);
    controlDown.SetThrustLimits(vtolPathFollowerSettings->ThrustLimits.Min, vtolPathFollowerSettings->ThrustLimits.Max);
    fsm->SettingsUpdated();
}

// AutoTakeoff Uses a different FSM to its parent
int32_t VtolAutoTakeoffController::Initialize(VtolPathFollowerSettingsData *ptr_vtolPathFollowerSettings)
{
    PIOS_Assert(ptr_vtolPathFollowerSettings);
    vtolPathFollowerSettings = ptr_vtolPathFollowerSettings;

    if (fsm == 0) {
        fsm = VtolAutoTakeoffFSM::instance();
        VtolAutoTakeoffFSM::instance()->Initialize(vtolPathFollowerSettings, pathDesired, flightStatus);
        controlDown.Initialize(fsm);
    }
    return 0;
}


void VtolAutoTakeoffController::UpdateVelocityDesired()
{
    VelocityStateData velocityState;

    VelocityStateGet(&velocityState);
    VelocityDesiredData velocityDesired;
    PositionStateData positionState;
    PositionStateGet(&positionState);

    if (fsm->PositionHoldState()) {
        controlDown.UpdatePositionState(positionState.Down);
        controlDown.ControlPosition();
    }

    controlDown.UpdateVelocityState(velocityState.Down);
    controlNE.UpdateVelocityState(velocityState.North, velocityState.East);

    controlNE.UpdatePositionState(positionState.North, positionState.East);
    controlNE.ControlPosition();

    velocityDesired.Down  = controlDown.GetVelocityDesired();
    float north, east;
    controlNE.GetVelocityDesired(&north, &east);
    velocityDesired.North = north;
    velocityDesired.East  = east;

    // update pathstatus
    pathStatus->error     = 0.0f;
    pathStatus->fractional_progress = 0.0f;
    if (fsm->PositionHoldState()) {
        pathStatus->fractional_progress = 1.0f;
        // note if the takeoff waypoint and the launch position are significantly different
        // the above check might need to expand to assessment of north and east.
    }
    pathStatus->path_direction_north = velocityDesired.North;
    pathStatus->path_direction_east  = velocityDesired.East;
    pathStatus->path_direction_down  = velocityDesired.Down;

    pathStatus->correction_direction_north = velocityDesired.North - velocityState.North;
    pathStatus->correction_direction_east  = velocityDesired.East - velocityState.East;
    pathStatus->correction_direction_down  = velocityDesired.Down - velocityState.Down;

    VelocityDesiredSet(&velocityDesired);
}

int8_t VtolAutoTakeoffController::UpdateStabilizationDesired(bool yaw_attitude, float yaw_direction)
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
    stabDesired.Thrust = controlDown.GetDownCommand();
    switch (autotakeoffState) {
    case STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_WAITFORARMED:
    case STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_WAITFORMIDTHROTTLE:
    case STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_REQUIREUNARMEDFIRST:
    case STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_ABORT:
        stabDesired.Thrust = 0.0f;
        break;
    default:
        break;
    }

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

void VtolAutoTakeoffController::UpdateAutoPilot()
{
    // state machine updates:
    // Vtol AutoTakeoff invocation from flight mode requires the following sequence:
    // 1. Arming must be done whilst in the AutoTakeOff flight mode
    // 2. If the AutoTakeoff flight mode is selected and already armed, requires disarming first
    // 3. Wait for armed state
    // 4. Once the user increases the throttle position to above 50%, then and only then initiate auto-takeoff.
    // 5. Whilst the throttle is < 50% before takeoff, all stick inputs are being ignored.
    // 6. If during the autotakeoff sequence, at any stage, if the throttle stick position reduces to less than 10%, landing is initiated.

    switch (autotakeoffState) {
    case STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_REQUIREUNARMEDFIRST:
    {
        FlightStatusData flightStatus;
        FlightStatusGet(&flightStatus);
        if (!flightStatus.Armed) {
            autotakeoffState = STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_WAITFORARMED;
            fsm->setControlState(autotakeoffState);
        }
    }
    break;
    case STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_WAITFORARMED:
    {
        FlightStatusData flightStatus;
        FlightStatusGet(&flightStatus);
        if (flightStatus.Armed) {
            autotakeoffState = STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_WAITFORMIDTHROTTLE;
            fsm->setControlState(autotakeoffState);
        }
    }
    break;
    case STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_WAITFORMIDTHROTTLE:
    {
        ManualControlCommandData cmd;
        ManualControlCommandGet(&cmd);

        if (cmd.Throttle > AUTOTAKEOFF_THROTTLE_LIMIT_TO_ALLOW_TAKEOFF_START) {
            autotakeoffState = STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_INITIATE;
            fsm->setControlState(autotakeoffState);
        }
    }
    break;
    case STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_INITIATE:
    {
        ManualControlCommandData cmd;
        ManualControlCommandGet(&cmd);
        FlightStatusData flightStatus;
        FlightStatusGet(&flightStatus);

        // we do not do a takeoff abort in pathplanner mode
        if (flightStatus.ControlChain.PathPlanner != FLIGHTSTATUS_CONTROLCHAIN_TRUE &&
            cmd.Throttle < AUTOTAKEOFF_THROTTLE_ABORT_LIMIT) {
            autotakeoffState = STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_ABORT;
            fsm->setControlState(autotakeoffState);
        }
    }
    break;

    case STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_ABORT:
    {
        FlightStatusData flightStatus;
        FlightStatusGet(&flightStatus);
        if (!flightStatus.Armed) {
            autotakeoffState = STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_WAITFORARMED;
            fsm->setControlState(autotakeoffState);
        }
    }
    break;
    case STATUSVTOLAUTOTAKEOFF_CONTROLSTATE_POSITIONHOLD:
    // nothing to do. land has been requested. stay here for forever until mode change.
    default:
        break;
    }

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

void VtolAutoTakeoffController::setArmedIfChanged(FlightStatusArmedOptions val)
{
    if (flightStatus->Armed != val) {
        flightStatus->Armed = val;
        FlightStatusSet(flightStatus);
    }
}
