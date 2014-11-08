/**
 ******************************************************************************
 * @addtogroup OpenPilotLibraries OpenPilot Libraries
 * @{
 * @addtogroup Navigation
 * @brief setups RTH/PH and other pathfollower/pathplanner status
 * @{
 *
 * @file       plan.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************/
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

#include <plans.h>
#include <openpilot.h>
#include <attitudesettings.h>
#include <takeofflocation.h>
#include <pathdesired.h>
#include <positionstate.h>
#include <flightmodesettings.h>
#include <manualcontrolcommand.h>
#include <attitudestate.h>
#include <sin_lookup.h>

#define UPDATE_EXPECTED 0.02f
#define UPDATE_MIN      1.0e-6f
#define UPDATE_MAX      1.0f
#define UPDATE_ALPHA    1.0e-2f

/**
 * @brief initialize UAVOs and structs used by this library
 */
void plan_initialize()
{
    TakeOffLocationInitialize();
    PositionStateInitialize();
    PathDesiredInitialize();
    FlightModeSettingsInitialize();
    AttitudeStateInitialize();
    ManualControlCommandInitialize();
}

/**
 * @brief setup pathplanner/pathfollower for positionhold
 */
void plan_setup_positionHold()
{
    PositionStateData positionState;

    PositionStateGet(&positionState);
    PathDesiredData pathDesired;
    PathDesiredGet(&pathDesired);

    FlightModeSettingsPositionHoldOffsetData offset;
    FlightModeSettingsPositionHoldOffsetGet(&offset);

    pathDesired.End.North        = positionState.North;
    pathDesired.End.East         = positionState.East;
    pathDesired.End.Down         = positionState.Down;
    pathDesired.Start.North      = positionState.North + offset.Horizontal; // in FlyEndPoint the direction of this vector does not matter
    pathDesired.Start.East       = positionState.East;
    pathDesired.Start.Down       = positionState.Down;
    pathDesired.StartingVelocity = 0.0f;
    pathDesired.EndingVelocity   = 0.0f;
    pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;

    PathDesiredSet(&pathDesired);
}

/**
 * @brief setup pathplanner/pathfollower for return to base
 */
void plan_setup_returnToBase()
{
    // Simple Return To Base mode - keep altitude the same applying configured delta, fly to takeoff position
    float positionStateDown;

    PositionStateDownGet(&positionStateDown);

    PathDesiredData pathDesired;
    PathDesiredGet(&pathDesired);

    TakeOffLocationData takeoffLocation;
    TakeOffLocationGet(&takeoffLocation);

    // TODO: right now VTOLPF does fly straight to destination altitude.
    // For a safer RTB destination altitude will be the higher between takeofflocation and current position (corrected with safety margin)

    float destDown;
    FlightModeSettingsReturnToBaseAltitudeOffsetGet(&destDown);
    destDown = MIN(positionStateDown, takeoffLocation.Down) - destDown;
    FlightModeSettingsPositionHoldOffsetData offset;
    FlightModeSettingsPositionHoldOffsetGet(&offset);

    pathDesired.End.North        = takeoffLocation.North;
    pathDesired.End.East         = takeoffLocation.East;
    pathDesired.End.Down         = destDown;

    pathDesired.Start.North      = takeoffLocation.North + offset.Horizontal; // in FlyEndPoint the direction of this vector does not matter
    pathDesired.Start.East       = takeoffLocation.East;
    pathDesired.Start.Down       = destDown;

    pathDesired.StartingVelocity = 0.0f;
    pathDesired.EndingVelocity   = 0.0f;
    pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;

    PathDesiredSet(&pathDesired);
}

static PiOSDeltatimeConfig landdT;
void plan_setup_land()
{
    float descendspeed;

    plan_setup_positionHold();

    FlightModeSettingsLandingVelocityGet(&descendspeed);
    PathDesiredData pathDesired;
    PathDesiredGet(&pathDesired);
    pathDesired.StartingVelocity = descendspeed;
    pathDesired.EndingVelocity   = descendspeed;
    PathDesiredSet(&pathDesired);
    PIOS_DELTATIME_Init(&landdT, UPDATE_EXPECTED, UPDATE_MIN, UPDATE_MAX, UPDATE_ALPHA);
}

/**
 * @brief execute land
 */
void plan_run_land()
{
    float downPos, descendspeed;
    PathDesiredEndData pathDesiredEnd;

    PositionStateDownGet(&downPos); // current down position
    PathDesiredEndGet(&pathDesiredEnd); // desired position
    PathDesiredEndingVelocityGet(&descendspeed);

    // desired position is updated to match the desired descend speed but don't run ahead
    // too far if the current position can't keep up. This normaly means we have landed.
    if (pathDesiredEnd.Down - downPos < 10) {
        pathDesiredEnd.Down += descendspeed * PIOS_DELTATIME_GetAverageSeconds(&landdT);
    }

    PathDesiredEndSet(&pathDesiredEnd);
}


/**
 * @brief positionvario functionality
 */
static bool vario_hold    = true;
static float hold_position[3];
static float vario_control_lowpass[4];
static float vario_course = 0.0f;

static void plan_setup_PositionVario()
{
    vario_hold = true;
    vario_control_lowpass[0] = 0.0f;
    vario_control_lowpass[1] = 0.0f;
    vario_control_lowpass[2] = 0.0f;
    vario_control_lowpass[3] = 0.0f;
    AttitudeStateYawGet(&vario_course);
    plan_setup_positionHold();
}

void plan_setup_CourseLock()
{
    plan_setup_PositionVario();
}

void plan_setup_MagicRoam()
{
    plan_setup_PositionVario();
}

void plan_setup_MagicLeash()
{
    plan_setup_PositionVario();
}

void plan_setup_AbsolutePosition()
{
    plan_setup_PositionVario();
}


#define DEADBAND 0.1f
static bool normalizeDeadband(float controlVector[4])
{
    bool moving = false;

    // roll, pitch, yaw between -1 and +1
    // thrust between 0 and 1 mapped to -1 to +1
    controlVector[3] = (2.0f * controlVector[3]) - 1.0f;
    int t;

    for (t = 0; t < 4; t++) {
        if (controlVector[t] < -DEADBAND) {
            moving = true;
            controlVector[t] += DEADBAND;
        } else if (controlVector[t] > DEADBAND) {
            moving = true;
            controlVector[t] -= DEADBAND;
        } else {
            controlVector[t] = 0.0f;
        }
        // deadband has been cut out, scale value back to [-1,+1]
        controlVector[t] *= (1.0f / (1.0f - DEADBAND));
        controlVector[t]  = boundf(controlVector[t], -1.0f, 1.0f);
    }

    return moving;
}

typedef enum { COURSE, FPV, LOS, NSEW } vario_type;

static void getVector(float controlVector[4], vario_type type)
{
    FlightModeSettingsPositionHoldOffsetData offset;

    FlightModeSettingsPositionHoldOffsetGet(&offset);

    // scale controlVector[3] (thrust) by vertical/horizontal to have vertical plane less sensitive
    controlVector[3] *= offset.Vertical / offset.Horizontal;

    float length = sqrtf(controlVector[0] * controlVector[0] + controlVector[1] * controlVector[1] + controlVector[3] * controlVector[3]);

    if (length <= 1e-9f) {
        length = 1.0f; // should never happen as getVector is not called if control within deadband
    }
    {
        float direction[3] = {
            controlVector[1] / length, // pitch is north
            controlVector[0] / length, // roll is east
            controlVector[3] / length // thrust is down
        };
        controlVector[0] = direction[0];
        controlVector[1] = direction[1];
        controlVector[2] = direction[2];
    }
    controlVector[3] = length * offset.Horizontal;

    // rotate north and east - rotation angle based on type
    float angle;
    switch (type) {
    case COURSE:
        angle = vario_course;
        break;
    case NSEW:
        angle = 0.0f;
        // NSEW no rotation takes place
        break;
    case FPV:
        // local rotation, using current yaw
        AttitudeStateYawGet(&angle);
        break;
    case LOS:
        // determine location based on vector from takeoff to current location
    {
        PositionStateData positionState;
        PositionStateGet(&positionState);
        TakeOffLocationData takeoffLocation;
        TakeOffLocationGet(&takeoffLocation);
        angle = RAD2DEG(atan2f(positionState.East - takeoffLocation.East, positionState.North - takeoffLocation.North));
    }
    break;
    }
    // rotate horizontally by angle
    {
        float rotated[2] = {
            controlVector[0] * cos_lookup_deg(angle) - controlVector[1] * sin_lookup_deg(angle),
            controlVector[0] * sin_lookup_deg(angle) + controlVector[1] * cos_lookup_deg(angle)
        };
        controlVector[0] = rotated[0];
        controlVector[1] = rotated[1];
    }
}


static void plan_run_PositionVario(vario_type type)
{
    float controlVector[4];
    float alpha;
    PathDesiredData pathDesired;

    PathDesiredGet(&pathDesired);
    FlightModeSettingsPositionHoldOffsetData offset;
    FlightModeSettingsPositionHoldOffsetGet(&offset);


    ManualControlCommandRollGet(&controlVector[0]);
    ManualControlCommandPitchGet(&controlVector[1]);
    ManualControlCommandYawGet(&controlVector[2]);
    ManualControlCommandThrustGet(&controlVector[3]);


    FlightModeSettingsVarioControlLowPassAlphaGet(&alpha);
    vario_control_lowpass[0] = alpha * vario_control_lowpass[0] + (1.0f - alpha) * controlVector[0];
    vario_control_lowpass[1] = alpha * vario_control_lowpass[1] + (1.0f - alpha) * controlVector[1];
    vario_control_lowpass[2] = alpha * vario_control_lowpass[2] + (1.0f - alpha) * controlVector[2];
    vario_control_lowpass[3] = alpha * vario_control_lowpass[3] + (1.0f - alpha) * controlVector[3];
    controlVector[0] = vario_control_lowpass[0];
    controlVector[1] = vario_control_lowpass[1];
    controlVector[2] = vario_control_lowpass[2];
    controlVector[3] = vario_control_lowpass[3];

    // check if movement is desired
    if (normalizeDeadband(controlVector) == false) {
        // no movement desired, re-enter positionHold at current start-position
        if (!vario_hold) {
            vario_hold = true;

            // new hold position is the position that was previously the start position
            pathDesired.End.North   = hold_position[0];
            pathDesired.End.East    = hold_position[1];
            pathDesired.End.Down    = hold_position[2];
            // while the new start position has the same offset as in position hold
            pathDesired.Start.North = pathDesired.End.North + offset.Horizontal; // in FlyEndPoint the direction of this vector does not matter
            pathDesired.Start.East  = pathDesired.End.East;
            pathDesired.Start.Down  = pathDesired.End.Down;
            PathDesiredSet(&pathDesired);
        }
    } else {
        PositionStateData positionState;
        PositionStateGet(&positionState);

        // flip pitch to have pitch down (away) point north
        controlVector[1] = -controlVector[1];
        getVector(controlVector, type);

        // layout of control Vector : unitVector in movement direction {0,1,2} vector length {3} velocity {4}
        if (vario_hold) {
            // start position is the position that was previously the hold position
            vario_hold = false;
            hold_position[0] = pathDesired.End.North;
            hold_position[1] = pathDesired.End.East;
            hold_position[2] = pathDesired.End.Down;
        } else {
            // start position is advanced according to movement - in the direction of ControlVector only
            // projection using scalar product
            float kp = (positionState.North - hold_position[0]) * controlVector[0]
                       + (positionState.East - hold_position[1]) * controlVector[1]
                       + (positionState.Down - hold_position[2]) * -controlVector[2];
            if (kp > 0.0f) {
                hold_position[0] += kp * controlVector[0];
                hold_position[1] += kp * controlVector[1];
                hold_position[2] += kp * -controlVector[2];
            }
        }
        // new destination position is advanced based on controlVector
        pathDesired.End.North   = hold_position[0] + controlVector[0] * controlVector[3];
        pathDesired.End.East    = hold_position[1] + controlVector[1] * controlVector[3];
        pathDesired.End.Down    = hold_position[2] - controlVector[2] * controlVector[3];
        // the new start position has the same offset as in position hold
        pathDesired.Start.North = pathDesired.End.North + offset.Horizontal; // in FlyEndPoint the direction of this vector does not matter
        pathDesired.Start.East  = pathDesired.End.East;
        pathDesired.Start.Down  = pathDesired.End.Down;
        PathDesiredSet(&pathDesired);
    }
}

void plan_run_CourseLock()
{
    plan_run_PositionVario(COURSE);
}

void plan_run_MagicRoam()
{
    plan_run_PositionVario(FPV);
}

void plan_run_MagicLeash()
{
    plan_run_PositionVario(LOS);
}

void plan_run_AbsolutePosition()
{
    plan_run_PositionVario(NSEW);
}


/**
 * @brief setup pathplanner/pathfollower for AutoCruise
 */
static PiOSDeltatimeConfig actimeval;
void plan_setup_AutoCruise()
{
    PositionStateData positionState;

    PositionStateGet(&positionState);
    PathDesiredData pathDesired;
    PathDesiredGet(&pathDesired);

    FlightModeSettingsPositionHoldOffsetData offset;
    FlightModeSettingsPositionHoldOffsetGet(&offset);

    // initialization is flight in direction of the nose.
    // the velocity is not relevant, as it will be reset by the run function even during first call
    float angle;
    AttitudeStateYawGet(&angle);
    float vector[2] = {
        cos_lookup_deg(angle),
        sin_lookup_deg(angle)
    };
    hold_position[0]             = positionState.North;
    hold_position[1]             = positionState.East;
    hold_position[2]             = positionState.Down;
    pathDesired.End.North        = hold_position[0] + vector[0];
    pathDesired.End.East         = hold_position[1] + vector[1];
    pathDesired.End.Down         = hold_position[2];
    // start position has the same offset as in position hold
    pathDesired.Start.North      = pathDesired.End.North + offset.Horizontal; // in FlyEndPoint the direction of this vector does not matter
    pathDesired.Start.East       = pathDesired.End.East;
    pathDesired.Start.Down       = pathDesired.End.Down;
    pathDesired.StartingVelocity = 0.0f;
    pathDesired.EndingVelocity   = 0.0f;
    pathDesired.Mode             = PATHDESIRED_MODE_FLYENDPOINT;

    PathDesiredSet(&pathDesired);

    // re-iniztializing deltatime is valid and also good practice here since
    // getAverageSeconds() has not been called/updated in a long time if we were in a different flightmode.
    PIOS_DELTATIME_Init(&actimeval, UPDATE_EXPECTED, UPDATE_MIN, UPDATE_MAX, UPDATE_ALPHA);
}

/**
 * @brief execute autocruise
 */
void plan_run_AutoCruise()
{
    PositionStateData positionState;

    PositionStateGet(&positionState);
    PathDesiredData pathDesired;
    PathDesiredGet(&pathDesired);
    FlightModeSettingsPositionHoldOffsetData offset;
    FlightModeSettingsPositionHoldOffsetGet(&offset);

    float controlVector[4];
    ManualControlCommandRollGet(&controlVector[0]);
    ManualControlCommandPitchGet(&controlVector[1]);
    ManualControlCommandYawGet(&controlVector[2]);
    controlVector[3] = 0.5f; // dummy, thrust is normalized separately
    normalizeDeadband(controlVector); // return value ignored
    ManualControlCommandThrustGet(&controlVector[3]); // no deadband as we are using thrust for velocity
    controlVector[3] = boundf(controlVector[3], 1e-6f, 1.0f); // bound to above zero, to prevent loss of vector direction

    // normalize old desired movement vector
    float vector[3] = { pathDesired.End.North - hold_position[0],
                        pathDesired.End.East - hold_position[1],
                        pathDesired.End.Down - hold_position[2] };
    float length    = sqrtf(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
    if (length < 1e-9f) {
        length = 1.0f; // should not happen since initialized properly in setup()
    }
    vector[0] /= length;
    vector[1] /= length;
    vector[2] /= length;

    // start position is advanced according to actual movement - in the direction of desired vector only
    // projection using scalar product
    float kp = (positionState.North - hold_position[0]) * vector[0]
               + (positionState.East - hold_position[1]) * vector[1]
               + (positionState.Down - hold_position[2]) * vector[2];
    if (kp > 0.0f) {
        hold_position[0] += kp * vector[0];
        hold_position[1] += kp * vector[1];
        hold_position[2] += kp * vector[2];
    }

    // new angle is equal to old angle plus offset depending on yaw input and time
    // (controlVector is normalized with a deadband, change is zero within deadband)
    float angle = RAD2DEG(atan2f(vector[1], vector[0]));
    float dT    = PIOS_DELTATIME_GetAverageSeconds(&actimeval);
    angle    += 10.0f * controlVector[2] * dT; // TODO magic value could eventually end up in a to be created settings

    // resulting movement vector is scaled by velocity demand in controlvector[3] [0.0-1.0]
    vector[0] = cosf(DEG2RAD(angle)) * offset.Horizontal * controlVector[3];
    vector[1] = sinf(DEG2RAD(angle)) * offset.Horizontal * controlVector[3];
    vector[2] = -controlVector[1] * offset.Vertical * controlVector[3];

    pathDesired.End.North   = hold_position[0] + vector[0];
    pathDesired.End.East    = hold_position[1] + vector[1];
    pathDesired.End.Down    = hold_position[2] + vector[2];
    // start position has the same offset as in position hold
    pathDesired.Start.North = pathDesired.End.North + offset.Horizontal; // in FlyEndPoint the direction of this vector does not matter
    pathDesired.Start.East  = pathDesired.End.East;
    pathDesired.Start.Down  = pathDesired.End.Down;
    PathDesiredSet(&pathDesired);
}
