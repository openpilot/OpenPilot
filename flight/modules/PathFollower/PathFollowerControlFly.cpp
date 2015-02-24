/*
 ******************************************************************************
 *
 * @file       PathFollowerControlFly.c
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
#include "PathFollowerControlFly.h"
#include "PathFollowerFSM.h"
#include "PIDControlThrust.h"

// Private constants
#define DEADBAND_HIGH          0.10f
#define DEADBAND_LOW           -0.10f

// pointer to a singleton instance
PathFollowerControlFly *PathFollowerControlFly::p_inst = 0;

PathFollowerControlFly::PathFollowerControlFly()
    : vtolPathFollowerSettings(0), pathDesired(0), pathStatus(0), mActive(false), mManualThrust(false)
{}

// Called when mode first engaged
void PathFollowerControlFly::Activate(void)
{
    if (!mActive) {
        mActive = true;
        mManualThrust = false;
        SettingsUpdated();
        controlThrust.Activate();
        controlNE.Activate();
        mMode = pathDesired->Mode;
    }
}

uint8_t PathFollowerControlFly::IsActive(void)
{
    return mActive;
}

uint8_t PathFollowerControlFly::Mode(void)
{
    return mMode;
}

// Objective updated in pathdesired
void PathFollowerControlFly::ObjectiveUpdated(void)
{}

void PathFollowerControlFly::Deactivate(void)
{
    if (mActive) {
        mActive = false;
        mManualThrust = false;
        controlThrust.Deactivate();
        controlNE.Deactivate();
    }
}


void PathFollowerControlFly::SettingsUpdated(void)
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

    controlThrust.UpdateParameters(vtolPathFollowerSettings->VerticalVelPID.Kp,
                                   vtolPathFollowerSettings->VerticalVelPID.Ki,
                                   vtolPathFollowerSettings->VerticalVelPID.Kd,
                                   vtolPathFollowerSettings->VerticalVelPID.ILimit, // TODO Change to BETA
                                   dT,
                                   vtolPathFollowerSettings->VerticalVelMax);
    controlNE.UpdatePositionalParameters(vtolPathFollowerSettings->VerticalPosP);

    // TODO Add trigger for this
    VtolSelfTuningStatsData vtolSelfTuningStats;
    VtolSelfTuningStatsGet(&vtolSelfTuningStats);
    controlThrust.UpdateNeutralThrust(vtolSelfTuningStats.NeutralThrustOffset + vtolPathFollowerSettings->ThrustLimits.Neutral);
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t PathFollowerControlFly::Initialize(VtolPathFollowerSettingsData *ptr_vtolPathFollowerSettings,
                                           PathDesiredData *ptr_pathDesired,
                                           PathStatusData *ptr_pathStatus)
{
    PIOS_Assert(ptr_vtolPathFollowerSettings);
    PIOS_Assert(ptr_pathDesired);
    PIOS_Assert(ptr_pathStatus);

    vtolPathFollowerSettings = ptr_vtolPathFollowerSettings;
    pathDesired = ptr_pathDesired;
    pathStatus  = ptr_pathStatus;
    controlThrust.Initialize(0);

    return 0;
}


/**
 * Compute desired velocity from the current position and path
 */
void PathFollowerControlFly::UpdateVelocityDesired()
{
    PositionStateData positionState;

    PositionStateGet(&positionState);

    VelocityStateData velocityState;
    VelocityStateGet(&velocityState);

    VelocityDesiredData velocityDesired;

    // look ahead kFF seconds
    float cur[3]   = { positionState.North + (velocityState.North * vtolPathFollowerSettings->CourseFeedForward),
                       positionState.East + (velocityState.East * vtolPathFollowerSettings->CourseFeedForward),
                       positionState.Down + (velocityState.Down * vtolPathFollowerSettings->CourseFeedForward) };
    struct path_status progress;
    path_progress(pathDesired, cur, &progress);

    controlNE.ControlPositionWithPath(&progress);
    if (!mManualThrust) {
        controlThrust.ControlPositionWithPath(&progress);
    }

    float north, east;
    controlNE.GetVelocityDesired(&north, &east);
    velocityDesired.North = north;
    velocityDesired.East  = east;
    if (!mManualThrust) {
        velocityDesired.Down = controlThrust.GetVelocityDesired();
    } else { velocityDesired.Down = 0.0f; }

    // update pathstatus
    pathStatus->error = progress.error;
    pathStatus->fractional_progress  = progress.fractional_progress;
    pathStatus->path_direction_north = progress.path_vector[0];
    pathStatus->path_direction_east  = progress.path_vector[1];
    pathStatus->path_direction_down  = progress.path_vector[2];

    pathStatus->correction_direction_north = progress.correction_vector[0];
    pathStatus->correction_direction_east  = progress.correction_vector[1];
    pathStatus->correction_direction_down  = progress.correction_vector[2];

    VelocityDesiredSet(&velocityDesired);
}


int8_t PathFollowerControlFly::UpdateStabilizationDesired(bool yaw_attitude, float yaw_direction)
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
    stabDesired.Pitch = boundf(-northCommand * cos_angle - eastCommand * sine_angle, -maxPitch, maxPitch); // this should be in the controller
    stabDesired.StabilizationMode.Roll  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.Roll = boundf(-northCommand * sine_angle + eastCommand * cos_angle, -maxPitch, maxPitch);

    ManualControlCommandData manualControl;
    ManualControlCommandGet(&manualControl);

#if 0
    // DEBUG HACK: allow user to skew compass on purpose to see if emergency failsafe kicks in
    if (vtolPathFollowerSettings->FlyawayEmergencyFallback == VTOLPATHFOLLOWERSETTINGS_FLYAWAYEMERGENCYFALLBACK_DEBUGTEST) {
        attitudeState.Yaw += 120.0f;
        if (attitudeState.Yaw > 180.0f) {
            attitudeState.Yaw -= 360.0f;
        }
    }


    if ( // emergency flyaway detection
        ( // integral already at its limit
            vtolPathFollowerSettings.HorizontalVelPID.ILimit - fabsf(global.PIDvel[0].iAccumulator) < 1e-6f ||
            vtolPathFollowerSettings.HorizontalVelPID.ILimit - fabsf(global.PIDvel[1].iAccumulator) < 1e-6f
        ) &&
        // angle between desired and actual velocity >90 degrees (by dot product)
        (velocityDesired.North * velocityState.North + velocityDesired.East * velocityState.East < 0.0f) &&
        // quad is moving at significant speed (during flyaway it would keep speeding up)
        squaref(velocityState.North) + squaref(velocityState.East) > 1.0f
        ) {
        vtolEmergencyFallback += dT;
        if (vtolEmergencyFallback >= vtolPathFollowerSettings->FlyawayEmergencyFallbackTriggerTime) {
            // after emergency timeout, trigger alarm - everything else is handled by callers
            // (switch to emergency algorithm, switch to emergency waypoint in pathplanner, alarms, ...)
            result = 0;
        }
    } else {
        vtolEmergencyFallback = 0.0f;
    }
#endif // if 0

    if (yaw_attitude) {
        stabDesired.StabilizationMode.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
        stabDesired.Yaw = yaw_direction;
    } else {
        stabDesired.StabilizationMode.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;
        stabDesired.Yaw = stabSettings.MaximumRate.Yaw * manualControl.Yaw;
    }

    // default thrust mode to cruise control
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL;

    if (mManualThrust) {
        stabDesired.Thrust = manualControl.Thrust;
    } else {
        stabDesired.Thrust = controlThrust.GetThrustCommand();
    }

    StabilizationDesiredSet(&stabDesired);

    return result;
}

/**
 * Compute desired attitude for vtols - emergency fallback
 */
void PathFollowerControlFly::UpdateDesiredAttitudeEmergencyFallback()
{
    VelocityDesiredData velocityDesired;
    VelocityStateData velocityState;
    StabilizationDesiredData stabDesired;

    float courseError;
    float courseCommand;

    VelocityStateGet(&velocityState);
    VelocityDesiredGet(&velocityDesired);

    ManualControlCommandData manualControlData;
    ManualControlCommandGet(&manualControlData);

    courseError = RAD2DEG(atan2f(velocityDesired.East, velocityDesired.North) - atan2f(velocityState.East, velocityState.North));

    if (courseError < -180.0f) {
        courseError += 360.0f;
    }
    if (courseError > 180.0f) {
        courseError -= 360.0f;
    }


    courseCommand   = (courseError * vtolPathFollowerSettings->EmergencyFallbackYawRate.kP);
    stabDesired.Yaw = boundf(courseCommand, -vtolPathFollowerSettings->EmergencyFallbackYawRate.Max, vtolPathFollowerSettings->EmergencyFallbackYawRate.Max);

    controlThrust.UpdateVelocitySetpoint(velocityDesired.Down);
    controlThrust.UpdateVelocityState(velocityState.Down);
    stabDesired.Thrust = controlThrust.GetThrustCommand();


    stabDesired.Roll   = vtolPathFollowerSettings->EmergencyFallbackAttitude.Roll;
    stabDesired.Pitch  = vtolPathFollowerSettings->EmergencyFallbackAttitude.Pitch;

    if (vtolPathFollowerSettings->ThrustControl == VTOLPATHFOLLOWERSETTINGS_THRUSTCONTROL_MANUAL) {
        stabDesired.Thrust = manualControlData.Thrust;
    }

    stabDesired.StabilizationMode.Roll   = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Yaw    = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL;
    StabilizationDesiredSet(&stabDesired);
}


void PathFollowerControlFly::UpdateAutoPilot()
{
    if (vtolPathFollowerSettings->ThrustControl == VTOLPATHFOLLOWERSETTINGS_THRUSTCONTROL_MANUAL) {
        mManualThrust = true;
    }

    uint8_t result = RunAutoPilot();

    if (result) {
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);
    } else {
        pathStatus->Status = PATHSTATUS_STATUS_CRITICAL;
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_WARNING);
    }

    PathStatusSet(pathStatus);
}
/**
 * vtol autopilot
 * use hover capable algorithm with unlimeted movement calculation. if that fails (flyaway situation due to compass failure)
 * fall back to emergency fallback autopilot to keep minimum amount of flight control
 */
uint8_t PathFollowerControlFly::RunAutoPilot()
{
    enum { RETURN_0 = 0, RETURN_1 = 1, RETURN_RESULT } returnmode;
    enum { FOLLOWER_REGULAR, FOLLOWER_FALLBACK } followermode;
    uint8_t result = 0;

    // decide on behaviour based on settings and system state
    if (vtolEmergencyFallbackSwitch) {
        returnmode   = RETURN_0;
        followermode = FOLLOWER_FALLBACK;
    } else {
        if (vtolPathFollowerSettings->FlyawayEmergencyFallback == VTOLPATHFOLLOWERSETTINGS_FLYAWAYEMERGENCYFALLBACK_ALWAYS) {
            returnmode   = RETURN_1;
            followermode = FOLLOWER_FALLBACK;
        } else {
            returnmode   = RETURN_RESULT;
            followermode = FOLLOWER_REGULAR;
        }
    }

    switch (followermode) {
    case FOLLOWER_REGULAR:
    {
        // horizontal position control PID loop works according to settings in regular mode, allowing integral terms
        UpdateVelocityDesired();

        // yaw behaviour is configurable in vtolpathfollower, select yaw control algorithm
        bool yaw_attitude = true;
        float yaw = 0.0f;

        switch (vtolPathFollowerSettings->YawControl) {
        case VTOLPATHFOLLOWERSETTINGS_YAWCONTROL_MANUAL:
            yaw_attitude = false;
            break;
        case VTOLPATHFOLLOWERSETTINGS_YAWCONTROL_TAILIN:
            yaw = updateTailInBearing();
            break;
        case VTOLPATHFOLLOWERSETTINGS_YAWCONTROL_MOVEMENTDIRECTION:
            yaw = updateCourseBearing();
            break;
        case VTOLPATHFOLLOWERSETTINGS_YAWCONTROL_PATHDIRECTION:
            yaw = updatePathBearing();
            break;
        case VTOLPATHFOLLOWERSETTINGS_YAWCONTROL_POI:
            yaw = updatePOIBearing();
            break;
        }

        result = UpdateStabilizationDesired(yaw_attitude, yaw);

        if (!result) {
            if (vtolPathFollowerSettings->FlyawayEmergencyFallback != VTOLPATHFOLLOWERSETTINGS_FLYAWAYEMERGENCYFALLBACK_DISABLED) {
                // switch to emergency follower if follower indicates problems
                vtolEmergencyFallbackSwitch = true;
            }
        }
    }
    break;
    case FOLLOWER_FALLBACK:
    {
        // fallback loop only cares about intended horizontal flight direction, simplify control behaviour accordingly
        controlNE.UpdatePositionalParameters(1.0f);
        UpdateVelocityDesired();

        // emergency follower has no return value
        UpdateDesiredAttitudeEmergencyFallback();
    }
    break;
    }

    switch (returnmode) {
    case RETURN_RESULT:
        return result;

    default:
        // returns either 0 or 1 according to enum definition above
        return returnmode;
    }
}


/**
 * Compute bearing of current takeoff location
 */
float PathFollowerControlFly::updateTailInBearing()
{
    PositionStateData p;

    PositionStateGet(&p);
    TakeOffLocationData t;
    TakeOffLocationGet(&t);
    // atan2f always returns in between + and - 180 degrees
    return RAD2DEG(atan2f(p.East - t.East, p.North - t.North));
}


/**
 * Compute bearing of current movement direction
 */
float PathFollowerControlFly::updateCourseBearing()
{
    VelocityStateData v;

    VelocityStateGet(&v);
    // atan2f always returns in between + and - 180 degrees
    return RAD2DEG(atan2f(v.East, v.North));
}


/**
 * Compute bearing of current path direction
 */
float PathFollowerControlFly::updatePathBearing()
{
    PositionStateData positionState;

    PositionStateGet(&positionState);

    float cur[3] = { positionState.North,
                     positionState.East,
                     positionState.Down };
    struct path_status progress;

    path_progress(pathDesired, cur, &progress);

    // atan2f always returns in between + and - 180 degrees
    return RAD2DEG(atan2f(progress.path_vector[1], progress.path_vector[0]));
}


/**
 * Compute bearing between current position and POI
 */
float PathFollowerControlFly::updatePOIBearing()
{
    PoiLocationData poi;

    PoiLocationGet(&poi);
    PositionStateData positionState;
    PositionStateGet(&positionState);

    const float dT = vtolPathFollowerSettings->UpdatePeriod / 1000.0f;
    float dLoc[3];
    float yaw = 0;
    /*float elevation = 0;*/

    dLoc[0] = positionState.North - poi.North;
    dLoc[1] = positionState.East - poi.East;
    dLoc[2] = positionState.Down - poi.Down;

    if (dLoc[1] < 0) {
        yaw = RAD2DEG(atan2f(dLoc[1], dLoc[0])) + 180.0f;
    } else {
        yaw = RAD2DEG(atan2f(dLoc[1], dLoc[0])) - 180.0f;
    }
    ManualControlCommandData manualControlData;
    ManualControlCommandGet(&manualControlData);

    float pathAngle = 0;
    if (manualControlData.Roll > DEADBAND_HIGH) {
        pathAngle = -(manualControlData.Roll - DEADBAND_HIGH) * dT * 300.0f;
    } else if (manualControlData.Roll < DEADBAND_LOW) {
        pathAngle = -(manualControlData.Roll - DEADBAND_LOW) * dT * 300.0f;
    }

    return yaw + (pathAngle / 2.0f);
}
