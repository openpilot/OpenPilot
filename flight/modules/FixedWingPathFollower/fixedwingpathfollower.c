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
#include "manualcontrol.h"
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
static float bound(float val, float min, float max);

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
    uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];
    HwSettingsOptionalModulesGet(optionalModules);
    if (optionalModules[HWSETTINGS_OPTIONALMODULES_FIXEDWINGPATHFOLLOWER] == HWSETTINGS_OPTIONALMODULES_ENABLED) {
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
        switch (flightStatus.FlightMode) {
        case FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD:
        case FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE:
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
            break;
        case FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER:
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
            break;
        default:
            // Be cleaner and get rid of global variables
            northVelIntegral = 0;
            eastVelIntegral  = 0;
            downVelIntegral  = 0;
            courseIntegral   = 0;
            speedIntegral    = 0;
            powerIntegral    = 0;

            break;
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
                      bound(progress.fractional_progress, 0, 1);
        altitudeSetpoint = pathDesired.Start.Down + (pathDesired.End.Down - pathDesired.Start.Down) *
                           bound(progress.fractional_progress, 0, 1);
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
    // difference between path_direction and velocitystate >90 degrees  ( 4th sector, facing away from eerything )
    // fix: ignore correction, steer in path direction until the situation has become better (condition doesn't apply anymore)
    float angle1 = RAD2DEG(atan2f(progress.path_direction[1], progress.path_direction[0]) - atan2f(velocityState.East, velocityState.North));
    float angle2 = RAD2DEG(atan2f(progress.correction_direction[1], progress.correction_direction[0]) - atan2f(velocityState.East, velocityState.North));
    if (angle1 < -180.0f) {
        angle1 += 360.0f;
    }
    if (angle1 > 180.0f) {
        angle1 -= 360.0f;
    }
    if (angle2 < -180.0f) {
        angle2 += 360.0f;
    }
    if (angle2 > 180.0f) {
        angle2 -= 360.0f;
    }
    if (fabsf(angle1) >= 90.0f && fabsf(angle2) >= 90.0f) {
        error_speed = 0;
    }

    // calculate correction - can also be zero if correction vector is 0 or no error present
    velocityDesired.North += progress.correction_direction[0] * error_speed;
    velocityDesired.East  += progress.correction_direction[1] * error_speed;

    // scale to correct length
    float l = sqrtf(velocityDesired.North * velocityDesired.North + velocityDesired.East * velocityDesired.East);
    velocityDesired.North *= groundspeed / l;
    velocityDesired.East  *= groundspeed / l;

    float downError = altitudeSetpoint - positionState.Down;
    velocityDesired.Down   = downError * fixedwingpathfollowerSettings.VerticalPosP;

    // update pathstatus
    pathStatus.error = progress.error;
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
    stabDesired.Roll     = attitude[0];
    stabDesired.Pitch    = attitude[1];
    stabDesired.Yaw      = attitude[2];
    stabDesired.Throttle = attitude[3];
    stabDesired.StabilizationMode.Roll  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Yaw   = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
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

    float groundspeedState;
    float groundspeedDesired;
    float indicatedAirspeedState;
    float indicatedAirspeedDesired;
    float airspeedError;

    float pitchCommand;

    float descentspeedDesired;
    float descentspeedError;
    float powerCommand;

    float bearing;
    float heading;
    float headingError;
    float course;
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
     * Compute speed error (required for throttle and pitch)
     */

    // Current ground speed
    groundspeedState = sqrtf(velocityState.East * velocityState.East + velocityState.North * velocityState.North);
    // note that airspeedStateBias is ( calibratedAirspeed - groundSpeed ) at the time of measurement,
    // but thanks to accelerometers,  groundspeed reacts faster to changes in direction
    // than airspeed and gps sensors alone
    indicatedAirspeedState   = groundspeedState + indicatedAirspeedStateBias;

    // Desired ground speed
    groundspeedDesired       = sqrtf(velocityDesired.North * velocityDesired.North + velocityDesired.East * velocityDesired.East);
    indicatedAirspeedDesired = bound(groundspeedDesired + indicatedAirspeedStateBias,
                                     fixedwingpathfollowerSettings.HorizontalVelMin,
                                     fixedwingpathfollowerSettings.HorizontalVelMax);

    // Airspeed error
    airspeedError = indicatedAirspeedDesired - indicatedAirspeedState;

    // Vertical speed error
    descentspeedDesired = bound(
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

    if (indicatedAirspeedState < 1e-6f) {
        // prevent division by zero, abort without controlling anything. This guidance mode is not suited for takeoff or touchdown, or handling stationary planes
        // also we cannot handle planes flying backwards, lets just wait until the nose drops
        fixedwingpathfollowerStatus.Errors.Lowspeed = 1;
        return 0;
    }

    /**
     * Compute desired throttle command
     */
    // compute saturated integral error throttle response. Make integral leaky for better performance. Approximately 30s time constant.
    if (fixedwingpathfollowerSettings.PowerPI.Ki > 0) {
        powerIntegral = bound(powerIntegral + -descentspeedError * dT,
                              -fixedwingpathfollowerSettings.PowerPI.ILimit / fixedwingpathfollowerSettings.PowerPI.Ki,
                              fixedwingpathfollowerSettings.PowerPI.ILimit / fixedwingpathfollowerSettings.PowerPI.Ki
                              ) * (1.0f - 1.0f / (1.0f + 30.0f / dT));
    } else { powerIntegral = 0; }

    // Compute the cross feed from vertical speed to pitch, with saturation
    float speedErrorToPowerCommandComponent = bound(
        (airspeedError / fixedwingpathfollowerSettings.HorizontalVelMin) * fixedwingpathfollowerSettings.AirspeedToPowerCrossFeed.Kp,
        -fixedwingpathfollowerSettings.AirspeedToPowerCrossFeed.Max,
        fixedwingpathfollowerSettings.AirspeedToPowerCrossFeed.Max
        );

    // Compute final throttle response
    powerCommand = -descentspeedError * fixedwingpathfollowerSettings.PowerPI.Kp +
                   powerIntegral * fixedwingpathfollowerSettings.PowerPI.Ki +
                   speedErrorToPowerCommandComponent;

    // Output internal state to telemetry
    fixedwingpathfollowerStatus.Error.Power    = descentspeedError;
    fixedwingpathfollowerStatus.ErrorInt.Power = powerIntegral;
    fixedwingpathfollowerStatus.Command.Power  = powerCommand;

    // set throttle
    stabDesired.Throttle = bound(fixedwingpathfollowerSettings.ThrottleLimit.Neutral + powerCommand,
                                 fixedwingpathfollowerSettings.ThrottleLimit.Min,
                                 fixedwingpathfollowerSettings.ThrottleLimit.Max);

    // Error condition: plane cannot hold altitude at current speed.
    fixedwingpathfollowerStatus.Errors.Lowpower = 0;
    if (powerCommand >= fixedwingpathfollowerSettings.ThrottleLimit.Max && // throttle at maximum
        velocityState.Down > 0 && // we ARE going down
        descentspeedDesired < 0 && // we WANT to go up
        airspeedError > 0 && // we are too slow already
        fixedwingpathfollowerSettings.Safetymargins.Lowpower > 0.5f) { // alarm switched on
        fixedwingpathfollowerStatus.Errors.Lowpower = 1;
        result = 0;
    }
    // Error condition: plane keeps climbing despite minimum throttle (opposite of above)
    fixedwingpathfollowerStatus.Errors.Highpower = 0;
    if (powerCommand >= fixedwingpathfollowerSettings.ThrottleLimit.Min && // throttle at minimum
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
        airspeedErrorInt = bound(airspeedErrorInt + airspeedError * dT,
                                 -fixedwingpathfollowerSettings.SpeedPI.ILimit / fixedwingpathfollowerSettings.SpeedPI.Ki,
                                 fixedwingpathfollowerSettings.SpeedPI.ILimit / fixedwingpathfollowerSettings.SpeedPI.Ki);
    }

    // Compute the cross feed from vertical speed to pitch, with saturation
    float verticalSpeedToPitchCommandComponent = bound(-descentspeedError * fixedwingpathfollowerSettings.VerticalToPitchCrossFeed.Kp,
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

    stabDesired.Pitch = bound(fixedwingpathfollowerSettings.PitchLimit.Neutral + pitchCommand,
                              fixedwingpathfollowerSettings.PitchLimit.Min,
                              fixedwingpathfollowerSettings.PitchLimit.Max);

    // Error condition: high speed dive
    fixedwingpathfollowerStatus.Errors.Pitchcontrol = 0;
    if (pitchCommand >= fixedwingpathfollowerSettings.PitchLimit.Max && // pitch demand is full up
        velocityState.Down > 0 && // we ARE going down
        descentspeedDesired < 0 && // we WANT to go up
        airspeedError < 0 && // we are too fast already
        fixedwingpathfollowerSettings.Safetymargins.Pitchcontrol > 0.5f) { // alarm switched on
        fixedwingpathfollowerStatus.Errors.Pitchcontrol = 1;
        result = 0;
    }

    /**
     * Calculate where we are heading and why (wind issues)
     */
    bearing = attitudeState.Yaw;
    heading = RAD2DEG(atan2f(velocityState.East, velocityState.North));
    headingError = heading - bearing;
    if (headingError < -180.0f) {
        headingError += 360.0f;
    }
    if (headingError > 180.0f) {
        headingError -= 360.0f;
    }
    // Error condition: wind speed is higher than airspeed. We are forced backwards!
    fixedwingpathfollowerStatus.Errors.Wind = 0;
    if ((headingError > fixedwingpathfollowerSettings.Safetymargins.Wind ||
         headingError < -fixedwingpathfollowerSettings.Safetymargins.Wind) &&
        fixedwingpathfollowerSettings.Safetymargins.Highpower > 0.5f) { // alarm switched on
        // we are flying backwards
        fixedwingpathfollowerStatus.Errors.Wind = 1;
        result = 0;
    }

    /**
     * Compute desired roll command
     */
    if (groundspeedDesired > 1e-6f) {
        course = RAD2DEG(atan2f(velocityDesired.East, velocityDesired.North));

        courseError = course - heading;
    } else {
        // if we are not supposed to move, run in a circle
        courseError = -90.0f;
        result = 0;
    }

    if (courseError < -180.0f) {
        courseError += 360.0f;
    }
    if (courseError > 180.0f) {
        courseError -= 360.0f;
    }

    courseIntegral = bound(courseIntegral + courseError * dT * fixedwingpathfollowerSettings.CoursePI.Ki,
                           -fixedwingpathfollowerSettings.CoursePI.ILimit,
                           fixedwingpathfollowerSettings.CoursePI.ILimit);
    courseCommand  = (courseError * fixedwingpathfollowerSettings.CoursePI.Kp +
                      courseIntegral);

    fixedwingpathfollowerStatus.Error.Course    = courseError;
    fixedwingpathfollowerStatus.ErrorInt.Course = courseIntegral;
    fixedwingpathfollowerStatus.Command.Course  = courseCommand;

    stabDesired.Roll = bound(fixedwingpathfollowerSettings.RollLimit.Neutral +
                             courseCommand,
                             fixedwingpathfollowerSettings.RollLimit.Min,
                             fixedwingpathfollowerSettings.RollLimit.Max);

    // TODO: find a check to determine loss of directional control. Likely needs some check of derivative


    /**
     * Compute desired yaw command
     */
    // TODO implement raw control mode for yaw and base on Accels.Y
    stabDesired.Yaw = 0;


    stabDesired.StabilizationMode.Roll  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Yaw   = STABILIZATIONDESIRED_STABILIZATIONMODE_NONE;

    StabilizationDesiredSet(&stabDesired);

    FixedWingPathFollowerStatusSet(&fixedwingpathfollowerStatus);

    return result;
}


/**
 * Bound input value between limits
 */
static float bound(float val, float min, float max)
{
    if (val < min) {
        val = min;
    } else if (val > max) {
        val = max;
    }
    return val;
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
    float groundspeed = sqrtf(velocityState.East * velocityState.East + velocityState.North * velocityState.North);


    indicatedAirspeedStateBias = airspeedState.CalibratedAirspeed - groundspeed;
    // note - we do fly by Indicated Airspeed (== calibrated airspeed)
    // however since airspeed is updated less often than groundspeed, we use sudden changes to groundspeed to offset the airspeed by the same measurement.
}
