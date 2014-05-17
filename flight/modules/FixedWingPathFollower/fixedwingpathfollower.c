/**
 ******************************************************************************
 *
 * @file       fixedwingpathfollower.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      This module compared @ref PositionActuatl to @ref ActiveWaypoint
 * and sets @ref AttitudeDesired.  It only does this when the FlightMode field
 * of @ref ManualControlCommand is Auto.
 *
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
 * Input object: ActiveWaypoint
 * Input object: PositionState
 * Input object: ManualControlCommand
 * Output object: AttitudeDesired
 *
 * This module will periodically update the value of the AttitudeDesired object.
 *
 * The module executes in its own thread in this example.
 *
 * Modules have no API, all communication to other modules is done through UAVObjects.
 * However modules may use the API exposed by shared libraries.
 * See the OpenPilot wiki for more details.
 * http://www.openpilot.org/OpenPilot_Application_Architecture
 *
 */

#include <openpilot.h>

#include "hwsettings.h"
#include "attitudestate.h"
#include "pathdesired.h" // object that will be updated by the module
#include "positionstate.h"
#include "flightstatus.h"
#include "pathstatus.h"
#include "airspeedstate.h"
#include "fixedwingpathfollowersettings.h"
#include "fixedwingpathfollowerstatus.h"
#include "homelocation.h"
#include "stabilizationdesired.h"
#include "stabilizationsettings.h"
#include "systemsettings.h"
#include "velocitydesired.h"
#include "velocitystate.h"
#include "taskinfo.h"
#include <pios_struct_helper.h>

#include "sin_lookup.h"
#include "paths.h"
#include "CoordinateConversions.h"

// Private constants
#define MAX_QUEUE_SIZE   4
#define STACK_SIZE_BYTES 1548
#define TASK_PRIORITY    (tskIDLE_PRIORITY + 2)

// Private variables
static bool followerEnabled = false;
static xTaskHandle pathfollowerTaskHandle;
static PathDesiredData pathDesired;
static PathStatusData pathStatus;
static FixedWingPathFollowerSettingsData fixedwingpathfollowerSettings;

// Private functions
static void pathfollowerTask(void *parameters);
static void SettingsUpdatedCb(UAVObjEvent *ev);
static void updatePathVelocity();
static uint8_t updateFixedDesiredAttitude();
static void updateFixedAttitude();
static void airspeedStateUpdatedCb(UAVObjEvent *ev);
static bool correctCourse(float *C, float *V, float *F, float s);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t FixedWingPathFollowerStart()
{
    if (followerEnabled) {
        // Start main task
        xTaskCreate(pathfollowerTask, (signed char *)"PathFollower", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &pathfollowerTaskHandle);
        PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_PATHFOLLOWER, pathfollowerTaskHandle);
    }

    return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t FixedWingPathFollowerInitialize()
{
    HwSettingsInitialize();
    HwSettingsOptionalModulesData optionalModules;
    HwSettingsOptionalModulesGet(&optionalModules);
    if (optionalModules.FixedWingPathFollower == HWSETTINGS_OPTIONALMODULES_ENABLED) {
        followerEnabled = true;
        FixedWingPathFollowerSettingsInitialize();
        FixedWingPathFollowerStatusInitialize();
        PathDesiredInitialize();
        PathStatusInitialize();
        VelocityDesiredInitialize();
        AirspeedStateInitialize();
    } else {
        followerEnabled = false;
    }
    return 0;
}
MODULE_INITCALL(FixedWingPathFollowerInitialize, FixedWingPathFollowerStart);

static float northVelIntegral = 0;
static float eastVelIntegral  = 0;
static float downVelIntegral  = 0;

static float courseIntegral   = 0;
static float speedIntegral    = 0;
static float powerIntegral    = 0;
static float airspeedErrorInt = 0;

// correct speed by measured airspeed
static float indicatedAirspeedStateBias = 0;

/**
 * Module thread, should not return.
 */
static void pathfollowerTask(__attribute__((unused)) void *parameters)
{
    SystemSettingsData systemSettings;
    FlightStatusData flightStatus;

    portTickType lastUpdateTime;

    AirspeedStateConnectCallback(airspeedStateUpdatedCb);
    FixedWingPathFollowerSettingsConnectCallback(SettingsUpdatedCb);
    PathDesiredConnectCallback(SettingsUpdatedCb);

    FixedWingPathFollowerSettingsGet(&fixedwingpathfollowerSettings);
    PathDesiredGet(&pathDesired);

    // Main task loop
    lastUpdateTime = xTaskGetTickCount();
    while (1) {
        // Conditions when this runs:
        // 1. Must have FixedWing type airframe
        // 2. Flight mode is PositionHold and PathDesired.Mode is Endpoint  OR
        // FlightMode is PathPlanner and PathDesired.Mode is Endpoint or Path

        SystemSettingsGet(&systemSettings);
        if ((systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWING) &&
            (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGELEVON) &&
            (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGVTAIL)) {
            AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_WARNING);
            vTaskDelay(1000);
            continue;
        }

        // Continue collecting data if not enough time
        vTaskDelayUntil(&lastUpdateTime, fixedwingpathfollowerSettings.UpdatePeriod / portTICK_RATE_MS);


        FlightStatusGet(&flightStatus);
        PathStatusGet(&pathStatus);

        uint8_t result;
        // Check the combinations of flightmode and pathdesired mode
        if (flightStatus.ControlChain.PathFollower == FLIGHTSTATUS_CONTROLCHAIN_TRUE) {
            if (flightStatus.ControlChain.PathPlanner == FLIGHTSTATUS_CONTROLCHAIN_FALSE) {
                if (pathDesired.Mode == PATHDESIRED_MODE_FLYENDPOINT) {
                    updatePathVelocity();
                    result = updateFixedDesiredAttitude();
                    if (result) {
                        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);
                    } else {
                        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_WARNING);
                    }
                } else {
                    AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_ERROR);
                }
            } else {
                pathStatus.UID    = pathDesired.UID;
                pathStatus.Status = PATHSTATUS_STATUS_INPROGRESS;
                switch (pathDesired.Mode) {
                case PATHDESIRED_MODE_FLYENDPOINT:
                case PATHDESIRED_MODE_FLYVECTOR:
                case PATHDESIRED_MODE_FLYCIRCLERIGHT:
                case PATHDESIRED_MODE_FLYCIRCLELEFT:
                    updatePathVelocity();
                    result = updateFixedDesiredAttitude();
                    if (result) {
                        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);
                    } else {
                        pathStatus.Status = PATHSTATUS_STATUS_CRITICAL;
                        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_WARNING);
                    }
                    break;
                case PATHDESIRED_MODE_FIXEDATTITUDE:
                    updateFixedAttitude(pathDesired.ModeParameters);
                    AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);
                    break;
                case PATHDESIRED_MODE_DISARMALARM:
                    AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_CRITICAL);
                    break;
                default:
                    pathStatus.Status = PATHSTATUS_STATUS_CRITICAL;
                    AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_ERROR);
                    break;
                }
            }
        } else {
            // Be cleaner and get rid of global variables
            northVelIntegral = 0;
            eastVelIntegral  = 0;
            downVelIntegral  = 0;
            courseIntegral   = 0;
            speedIntegral    = 0;
            powerIntegral    = 0;
        }
        PathStatusSet(&pathStatus);
    }
}

/**
 * Compute desired velocity from the current position and path
 *
 * Takes in @ref PositionState and compares it to @ref PathDesired
 * and computes @ref VelocityDesired
 */
static void updatePathVelocity()
{
    PositionStateData positionState;

    PositionStateGet(&positionState);
    VelocityStateData velocityState;
    VelocityStateGet(&velocityState);

    // look ahead fixedwingpathfollowerSettings.CourseFeedForward seconds
    float cur[3] = { positionState.North + (velocityState.North * fixedwingpathfollowerSettings.CourseFeedForward),
                     positionState.East + (velocityState.East * fixedwingpathfollowerSettings.CourseFeedForward),
                     positionState.Down + (velocityState.Down * fixedwingpathfollowerSettings.CourseFeedForward) };
    struct path_status progress;

    path_progress(cast_struct_to_array(pathDesired.Start, pathDesired.Start.North),
                  cast_struct_to_array(pathDesired.End, pathDesired.End.North),
                  cur, &progress, pathDesired.Mode);

    float groundspeed;
    float altitudeSetpoint;
    switch (pathDesired.Mode) {
    case PATHDESIRED_MODE_FLYCIRCLERIGHT:
    case PATHDESIRED_MODE_DRIVECIRCLERIGHT:
    case PATHDESIRED_MODE_FLYCIRCLELEFT:
    case PATHDESIRED_MODE_DRIVECIRCLELEFT:
        groundspeed = pathDesired.EndingVelocity;
        altitudeSetpoint = pathDesired.End.Down;
        break;
    case PATHDESIRED_MODE_FLYENDPOINT:
    case PATHDESIRED_MODE_DRIVEENDPOINT:
    case PATHDESIRED_MODE_FLYVECTOR:
    case PATHDESIRED_MODE_DRIVEVECTOR:
    default:
        groundspeed = pathDesired.StartingVelocity + (pathDesired.EndingVelocity - pathDesired.StartingVelocity) *
                      boundf(progress.fractional_progress, 0, 1);
        altitudeSetpoint = pathDesired.Start.Down + (pathDesired.End.Down - pathDesired.Start.Down) *
                           boundf(progress.fractional_progress, 0, 1);
        break;
    }
    // make sure groundspeed is not zero
    if (groundspeed < 1e-2f) {
        groundspeed = 1e-2f;
    }

    // calculate velocity - can be zero if waypoints are too close
    VelocityDesiredData velocityDesired;
    velocityDesired.North = progress.path_direction[0];
    velocityDesired.East  = progress.path_direction[1];

    float error_speed = progress.error * fixedwingpathfollowerSettings.HorizontalPosP;

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
    if ( // calculating angles < 90 degrees through dot products
        ((progress.path_direction[0] * velocityState.North + progress.path_direction[1] * velocityState.East) < 0.0f) &&
        ((progress.correction_direction[0] * velocityState.North + progress.correction_direction[1] * velocityState.East) < 0.0f)) {
        error_speed = 0;
    }

    // calculate correction - can also be zero if correction vector is 0 or no error present
    velocityDesired.North += progress.correction_direction[0] * error_speed;
    velocityDesired.East  += progress.correction_direction[1] * error_speed;

    // scale to correct length
    float l = sqrtf(velocityDesired.North * velocityDesired.North + velocityDesired.East * velocityDesired.East);
    if (l > 0.0f) {
        velocityDesired.North *= groundspeed / l;
        velocityDesired.East  *= groundspeed / l;
    }

    float downError = altitudeSetpoint - positionState.Down;
    velocityDesired.Down = downError * fixedwingpathfollowerSettings.VerticalPosP;

    // update pathstatus
    pathStatus.error     = progress.error;
    pathStatus.fractional_progress = progress.fractional_progress;

    VelocityDesiredSet(&velocityDesired);
}


/**
 * Compute desired attitude from a fixed preset
 *
 */
static void updateFixedAttitude(float *attitude)
{
    StabilizationDesiredData stabDesired;

    StabilizationDesiredGet(&stabDesired);
    stabDesired.Roll   = attitude[0];
    stabDesired.Pitch  = attitude[1];
    stabDesired.Yaw    = attitude[2];
    stabDesired.Thrust = attitude[3];
    stabDesired.StabilizationMode.Roll   = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Yaw    = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;
    StabilizationDesiredSet(&stabDesired);
}

/**
 * Compute desired attitude from the desired velocity
 *
 * Takes in @ref NedState which has the acceleration in the
 * NED frame as the feedback term and then compares the
 * @ref VelocityState against the @ref VelocityDesired
 */
static uint8_t updateFixedDesiredAttitude()
{
    uint8_t result = 1;

    float dT = fixedwingpathfollowerSettings.UpdatePeriod / 1000.0f; // Convert from [ms] to [s]

    VelocityDesiredData velocityDesired;
    VelocityStateData velocityState;
    StabilizationDesiredData stabDesired;
    AttitudeStateData attitudeState;
    StabilizationSettingsData stabSettings;
    FixedWingPathFollowerStatusData fixedwingpathfollowerStatus;
    AirspeedStateData airspeedState;
    SystemSettingsData systemSettings;

    float groundspeedProjection;
    float indicatedAirspeedState;
    float indicatedAirspeedDesired;
    float airspeedError;

    float pitchCommand;

    float descentspeedDesired;
    float descentspeedError;
    float powerCommand;

    float airspeedVector[2];
    float fluidMovement[2];
    float courseComponent[2];
    float courseError;
    float courseCommand;

    FixedWingPathFollowerStatusGet(&fixedwingpathfollowerStatus);

    VelocityStateGet(&velocityState);
    StabilizationDesiredGet(&stabDesired);
    VelocityDesiredGet(&velocityDesired);
    AttitudeStateGet(&attitudeState);
    StabilizationSettingsGet(&stabSettings);
    AirspeedStateGet(&airspeedState);
    SystemSettingsGet(&systemSettings);

    /**
     * Compute speed error and course
     */
    // missing sensors for airspeed-direction we have to assume within
    // reasonable error that measured airspeed is actually the airspeed
    // component in forward pointing direction
    // airspeedVector is normalized
    airspeedVector[0]     = cos_lookup_deg(attitudeState.Yaw);
    airspeedVector[1]     = sin_lookup_deg(attitudeState.Yaw);

    // current ground speed projected in forward direction
    groundspeedProjection = velocityState.North * airspeedVector[0] + velocityState.East * airspeedVector[1];

    // note that airspeedStateBias is ( calibratedAirspeed - groundspeedProjection ) at the time of measurement,
    // but thanks to accelerometers,  groundspeedProjection reacts faster to changes in direction
    // than airspeed and gps sensors alone
    indicatedAirspeedState = groundspeedProjection + indicatedAirspeedStateBias;

    // fluidMovement is a vector describing the aproximate movement vector of
    // the surrounding fluid in 2d space (aka wind vector)
    fluidMovement[0] = velocityState.North - (indicatedAirspeedState * airspeedVector[0]);
    fluidMovement[1] = velocityState.East - (indicatedAirspeedState * airspeedVector[1]);

    // calculate the movement vector we need to fly to reach velocityDesired -
    // taking fluidMovement into account
    courseComponent[0] = velocityDesired.North - fluidMovement[0];
    courseComponent[1] = velocityDesired.East - fluidMovement[1];

    indicatedAirspeedDesired = boundf(sqrtf(courseComponent[0] * courseComponent[0] + courseComponent[1] * courseComponent[1]),
                                      fixedwingpathfollowerSettings.HorizontalVelMin,
                                      fixedwingpathfollowerSettings.HorizontalVelMax);

    // if we could fly at arbitrary speeds, we'd just have to move towards the
    // courseComponent vector as previously calculated and we'd be fine
    // unfortunately however we are bound by min and max air speed limits, so
    // we need to recalculate the correct course to meet at least the
    // velocityDesired vector direction at our current speed
    // this overwrites courseComponent
    bool valid = correctCourse(courseComponent, (float *)&velocityDesired.North, fluidMovement, indicatedAirspeedDesired);

    // Error condition: wind speed too high, we can't go where we want anymore
    fixedwingpathfollowerStatus.Errors.Wind = 0;
    if ((!valid) &&
        fixedwingpathfollowerSettings.Safetymargins.Wind > 0.5f) { // alarm switched on
        fixedwingpathfollowerStatus.Errors.Wind = 1;
        result = 0;
    }

    // Airspeed error
    airspeedError = indicatedAirspeedDesired - indicatedAirspeedState;

    // Vertical speed error
    descentspeedDesired = boundf(
        velocityDesired.Down,
        -fixedwingpathfollowerSettings.VerticalVelMax,
        fixedwingpathfollowerSettings.VerticalVelMax);
    descentspeedError = descentspeedDesired - velocityState.Down;

    // Error condition: plane too slow or too fast
    fixedwingpathfollowerStatus.Errors.Highspeed = 0;
    fixedwingpathfollowerStatus.Errors.Lowspeed  = 0;
    if (indicatedAirspeedState > systemSettings.AirSpeedMax * fixedwingpathfollowerSettings.Safetymargins.Overspeed) {
        fixedwingpathfollowerStatus.Errors.Overspeed = 1;
        result = 0;
    }
    if (indicatedAirspeedState > fixedwingpathfollowerSettings.HorizontalVelMax * fixedwingpathfollowerSettings.Safetymargins.Highspeed) {
        fixedwingpathfollowerStatus.Errors.Highspeed = 1;
        result = 0;
    }
    if (indicatedAirspeedState < fixedwingpathfollowerSettings.HorizontalVelMin * fixedwingpathfollowerSettings.Safetymargins.Lowspeed) {
        fixedwingpathfollowerStatus.Errors.Lowspeed = 1;
        result = 0;
    }
    if (indicatedAirspeedState < systemSettings.AirSpeedMin * fixedwingpathfollowerSettings.Safetymargins.Stallspeed) {
        fixedwingpathfollowerStatus.Errors.Stallspeed = 1;
        result = 0;
    }

    /**
     * Compute desired thrust command
     */
    // compute saturated integral error thrust response. Make integral leaky for better performance. Approximately 30s time constant.
    if (fixedwingpathfollowerSettings.PowerPI.Ki > 0) {
        powerIntegral = boundf(powerIntegral + -descentspeedError * dT,
                               -fixedwingpathfollowerSettings.PowerPI.ILimit / fixedwingpathfollowerSettings.PowerPI.Ki,
                               fixedwingpathfollowerSettings.PowerPI.ILimit / fixedwingpathfollowerSettings.PowerPI.Ki
                               ) * (1.0f - 1.0f / (1.0f + 30.0f / dT));
    } else {
        powerIntegral = 0;
    }

    // Compute the cross feed from vertical speed to pitch, with saturation
    float speedErrorToPowerCommandComponent = boundf(
        (airspeedError / fixedwingpathfollowerSettings.HorizontalVelMin) * fixedwingpathfollowerSettings.AirspeedToPowerCrossFeed.Kp,
        -fixedwingpathfollowerSettings.AirspeedToPowerCrossFeed.Max,
        fixedwingpathfollowerSettings.AirspeedToPowerCrossFeed.Max
        );

    // Compute final thrust response
    powerCommand = -descentspeedError * fixedwingpathfollowerSettings.PowerPI.Kp +
                   powerIntegral * fixedwingpathfollowerSettings.PowerPI.Ki +
                   speedErrorToPowerCommandComponent;

    // Output internal state to telemetry
    fixedwingpathfollowerStatus.Error.Power    = descentspeedError;
    fixedwingpathfollowerStatus.ErrorInt.Power = powerIntegral;
    fixedwingpathfollowerStatus.Command.Power  = powerCommand;

    // set thrust
    stabDesired.Thrust = boundf(fixedwingpathfollowerSettings.ThrustLimit.Neutral + powerCommand,
                                fixedwingpathfollowerSettings.ThrustLimit.Min,
                                fixedwingpathfollowerSettings.ThrustLimit.Max);

    // Error condition: plane cannot hold altitude at current speed.
    fixedwingpathfollowerStatus.Errors.Lowpower = 0;
    if (fixedwingpathfollowerSettings.ThrustLimit.Neutral + powerCommand >= fixedwingpathfollowerSettings.ThrustLimit.Max && // thrust at maximum
        velocityState.Down > 0 && // we ARE going down
        descentspeedDesired < 0 && // we WANT to go up
        airspeedError > 0 && // we are too slow already
        fixedwingpathfollowerSettings.Safetymargins.Lowpower > 0.5f) { // alarm switched on
        fixedwingpathfollowerStatus.Errors.Lowpower = 1;
        result = 0;
    }
    // Error condition: plane keeps climbing despite minimum thrust (opposite of above)
    fixedwingpathfollowerStatus.Errors.Highpower = 0;
    if (fixedwingpathfollowerSettings.ThrustLimit.Neutral + powerCommand <= fixedwingpathfollowerSettings.ThrustLimit.Min && // thrust at minimum
        velocityState.Down < 0 && // we ARE going up
        descentspeedDesired > 0 && // we WANT to go down
        airspeedError < 0 && // we are too fast already
        fixedwingpathfollowerSettings.Safetymargins.Highpower > 0.5f) { // alarm switched on
        fixedwingpathfollowerStatus.Errors.Highpower = 1;
        result = 0;
    }


    /**
     * Compute desired pitch command
     */
    if (fixedwingpathfollowerSettings.SpeedPI.Ki > 0) {
        // Integrate with saturation
        airspeedErrorInt = boundf(airspeedErrorInt + airspeedError * dT,
                                  -fixedwingpathfollowerSettings.SpeedPI.ILimit / fixedwingpathfollowerSettings.SpeedPI.Ki,
                                  fixedwingpathfollowerSettings.SpeedPI.ILimit / fixedwingpathfollowerSettings.SpeedPI.Ki);
    }

    // Compute the cross feed from vertical speed to pitch, with saturation
    float verticalSpeedToPitchCommandComponent = boundf(-descentspeedError * fixedwingpathfollowerSettings.VerticalToPitchCrossFeed.Kp,
                                                        -fixedwingpathfollowerSettings.VerticalToPitchCrossFeed.Max,
                                                        fixedwingpathfollowerSettings.VerticalToPitchCrossFeed.Max
                                                        );

    // Compute the pitch command as err*Kp + errInt*Ki + X_feed.
    pitchCommand = -(airspeedError * fixedwingpathfollowerSettings.SpeedPI.Kp
                     + airspeedErrorInt * fixedwingpathfollowerSettings.SpeedPI.Ki
                     ) + verticalSpeedToPitchCommandComponent;

    fixedwingpathfollowerStatus.Error.Speed    = airspeedError;
    fixedwingpathfollowerStatus.ErrorInt.Speed = airspeedErrorInt;
    fixedwingpathfollowerStatus.Command.Speed  = pitchCommand;

    stabDesired.Pitch = boundf(fixedwingpathfollowerSettings.PitchLimit.Neutral + pitchCommand,
                               fixedwingpathfollowerSettings.PitchLimit.Min,
                               fixedwingpathfollowerSettings.PitchLimit.Max);

    // Error condition: high speed dive
    fixedwingpathfollowerStatus.Errors.Pitchcontrol = 0;
    if (fixedwingpathfollowerSettings.PitchLimit.Neutral + pitchCommand >= fixedwingpathfollowerSettings.PitchLimit.Max && // pitch demand is full up
        velocityState.Down > 0 && // we ARE going down
        descentspeedDesired < 0 && // we WANT to go up
        airspeedError < 0 && // we are too fast already
        fixedwingpathfollowerSettings.Safetymargins.Pitchcontrol > 0.5f) { // alarm switched on
        fixedwingpathfollowerStatus.Errors.Pitchcontrol = 1;
        result = 0;
    }

    /**
     * Compute desired roll command
     */
    courseError = RAD2DEG(atan2f(courseComponent[1], courseComponent[0])) - attitudeState.Yaw;

    if (courseError < -180.0f) {
        courseError += 360;
    }
    if (courseError > 180.0f) {
        courseError -= 360;
    }

    // overlap calculation. Theres a dead zone behind the craft where the
    // counter-yawing of some craft while rolling could render a desired right
    // turn into a desired left turn. Making the turn direction based on
    // current roll angle keeps the plane committed to a direction once chosen
    if (courseError < -180.0f + (fixedwingpathfollowerSettings.ReverseCourseOverlap * 0.5f)
        && attitudeState.Roll > 0.0f) {
        courseError += 360.0f;
    }
    if (courseError > 180.0f - (fixedwingpathfollowerSettings.ReverseCourseOverlap * 0.5f)
        && attitudeState.Roll < 0.0f) {
        courseError -= 360.0f;
    }

    courseIntegral = boundf(courseIntegral + courseError * dT * fixedwingpathfollowerSettings.CoursePI.Ki,
                            -fixedwingpathfollowerSettings.CoursePI.ILimit,
                            fixedwingpathfollowerSettings.CoursePI.ILimit);
    courseCommand  = (courseError * fixedwingpathfollowerSettings.CoursePI.Kp +
                      courseIntegral);

    fixedwingpathfollowerStatus.Error.Course    = courseError;
    fixedwingpathfollowerStatus.ErrorInt.Course = courseIntegral;
    fixedwingpathfollowerStatus.Command.Course  = courseCommand;

    stabDesired.Roll = boundf(fixedwingpathfollowerSettings.RollLimit.Neutral +
                              courseCommand,
                              fixedwingpathfollowerSettings.RollLimit.Min,
                              fixedwingpathfollowerSettings.RollLimit.Max);

    // TODO: find a check to determine loss of directional control. Likely needs some check of derivative


    /**
     * Compute desired yaw command
     */
    // TODO implement raw control mode for yaw and base on Accels.Y
    stabDesired.Yaw = 0;


    stabDesired.StabilizationMode.Roll   = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Yaw    = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;

    StabilizationDesiredSet(&stabDesired);

    FixedWingPathFollowerStatusSet(&fixedwingpathfollowerStatus);

    return result;
}

static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    FixedWingPathFollowerSettingsGet(&fixedwingpathfollowerSettings);
    PathDesiredGet(&pathDesired);
}

static void airspeedStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    AirspeedStateData airspeedState;
    VelocityStateData velocityState;

    AirspeedStateGet(&airspeedState);
    VelocityStateGet(&velocityState);
    float airspeedVector[2];
    float yaw;
    AttitudeStateYawGet(&yaw);
    airspeedVector[0] = cos_lookup_deg(yaw);
    airspeedVector[1] = sin_lookup_deg(yaw);
    // vector projection of groundspeed on airspeed vector to handle both forward and backwards movement
    float groundspeedProjection = velocityState.North * airspeedVector[0] + velocityState.East * airspeedVector[1];

    indicatedAirspeedStateBias = airspeedState.CalibratedAirspeed - groundspeedProjection;
    // note - we do fly by Indicated Airspeed (== calibrated airspeed) however
    // since airspeed is updated less often than groundspeed, we use sudden
    // changes to groundspeed to offset the airspeed by the same measurement.
    // This has a side effect that in the absence of any airspeed updates, the
    // pathfollower will fly using groundspeed.
}


/**
 * Function to calculate course vector C based on airspeed s, fluid movement F
 * and desired movement vector V
 * parameters in: V,F,s
 * parameters out: C
 * returns true if a valid solution could be found for V,F,s, false if not
 * C will be set to a best effort attempt either way
 */
static bool correctCourse(float *C, float *V, float *F, float s)
{
    // Approach:
    // Let Sc be a circle around origin marking possible movement vectors
    // of the craft with airspeed s (all possible options for C)
    // Let Vl be a line through the origin along movement vector V where fr any
    // point v on line Vl v = k * (V / |V|) = k' * V
    // Let Wl be a line parallel to Vl where for any point v on line Vl exists
    // a point w on WL with w = v - F
    // Then any intersection between circle Sc and line Wl represents course
    // vector which would result in a movement vector
    // V' = k * ( V / |V|) = k' * V
    // If there is no intersection point, S is insufficient to compensate
    // for F and we can only try to fly in direction of V (thus having wind drift
    // but at least making progress orthogonal to wind)

    s = fabsf(s);
    float f = sqrtf(F[0] * F[0] + F[1] * F[1]);

    // normalize Cn=V/|V|, |V| must be >0
    float v = sqrtf(V[0] * V[0] + V[1] * V[1]);
    if (v < 1e-6f) {
        // if |V|=0, we aren't supposed to move, turn into the wind
        // (this allows hovering)
        C[0] = -F[0];
        C[1] = -F[1];
        // if desired airspeed matches fluidmovement a hover is actually
        // intended so return true
        return fabsf(f - s) < 1e-3f;
    }
    float Vn[2] = { V[0] / v, V[1] / v };

    // project F on V
    float fp    = F[0] * Vn[0] + F[1] * Vn[1];

    // find component Fo of F that is orthogonal to V
    // (which is exactly the distance between Vl and Wl)
    float Fo[2] = { F[0] - (fp * Vn[0]), F[1] - (fp * Vn[1]) };
    float fo2   = Fo[0] * Fo[0] + Fo[1] * Fo[1];

    // find k where k * Vn = C - Fo
    // |C|=s is the hypothenuse in any rectangular triangle formed by k * Vn and Fo
    // so k^2 + fo^2 = s^2 (since |Vn|=1)
    float k2 = s * s - fo2;
    if (k2 <= -1e-3f) {
        // there is no solution, we will be drifted off either way
        // fallback: fly stupidly in direction of V and hope for the best
        C[0] = V[0];
        C[1] = V[1];
        return false;
    } else if (k2 <= 1e-3f) {
        // there is exactly one solution: -Fo
        C[0] = -Fo[0];
        C[1] = -Fo[1];
        return true;
    }
    // we have two possible solutions k positive and k negative as there are
    // two intersection points between Wl and Sc
    // which one is better? two criteria:
    // 1. we MUST move in the right direction, if any k leads to -v its invalid
    // 2. we should minimize the speed error
    float k     = sqrt(k2);
    float C1[2] = { -k * Vn[0] - Fo[0], -k * Vn[1] - Fo[1] };
    float C2[2] = { k *Vn[0] - Fo[0], k * Vn[1] - Fo[1] };
    // project C+F on Vn to find signed resulting movement vector length
    float vp1   = (C1[0] + F[0]) * Vn[0] + (C1[1] + F[1]) * Vn[1];
    float vp2   = (C2[0] + F[0]) * Vn[0] + (C2[1] + F[1]) * Vn[1];
    if (vp1 >= 0.0f && fabsf(v - vp1) < fabsf(v - vp2)) {
        // in this case the angle between course and resulting movement vector
        // is greater than 90 degrees - so we actually fly backwards
        C[0] = C1[0];
        C[1] = C1[1];
        return true;
    }
    C[0] = C2[0];
    C[1] = C2[1];
    if (vp2 >= 0.0f) {
        // in this case the angle between course and movement vector is less than
        // 90 degrees, but we do move in the right direction
        return true;
    } else {
        // in this case we actually get driven in the opposite direction of V
        // with both solutions for C
        // this might be reached in headwind stronger than maximum allowed
        // airspeed.
        return false;
    }
}
