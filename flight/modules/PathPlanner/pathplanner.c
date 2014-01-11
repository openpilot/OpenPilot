/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup PathPlanner Path Planner Module
 * @brief Executes a series of waypoints
 * @{
 *
 * @file       pathplanner.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Executes a series of waypoints
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

#include "openpilot.h"

#include "flightplan.h"
#include "flightstatus.h"
#include "airspeedstate.h"
#include "pathaction.h"
#include "pathdesired.h"
#include "pathstatus.h"
#include "positionstate.h"
#include "velocitystate.h"
#include "waypoint.h"
#include "waypointactive.h"
#include "taskinfo.h"
#include <pios_struct_helper.h>
#include "paths.h"

// Private constants
#define STACK_SIZE_BYTES            1024
#define TASK_PRIORITY               CALLBACK_TASK_NAVIGATION
#define MAX_QUEUE_SIZE              2
#define PATH_PLANNER_UPDATE_RATE_MS 100 // can be slow, since we listen to status updates as well

// Private types

// Private functions
static void pathPlannerTask();
static void commandUpdated(UAVObjEvent *ev);
static void statusUpdated(UAVObjEvent *ev);
static void updatePathDesired();
static void setWaypoint(uint16_t num);

static uint8_t pathConditionCheck();
static uint8_t conditionNone();
static uint8_t conditionTimeOut();
static uint8_t conditionDistanceToTarget();
static uint8_t conditionLegRemaining();
static uint8_t conditionBelowError();
static uint8_t conditionAboveAltitude();
static uint8_t conditionAboveSpeed();
static uint8_t conditionPointingTowardsNext();
static uint8_t conditionPythonScript();
static uint8_t conditionImmediate();


// Private variables
static DelayedCallbackInfo *pathPlannerHandle;
static DelayedCallbackInfo *pathDesiredUpdaterHandle;
static WaypointActiveData waypointActive;
static WaypointData waypoint;
static PathActionData pathAction;
static bool pathplanner_active = false;


/**
 * Module initialization
 */
int32_t PathPlannerStart()
{
    // when the active waypoint changes, update pathDesired
    WaypointConnectCallback(commandUpdated);
    WaypointActiveConnectCallback(commandUpdated);
    PathActionConnectCallback(commandUpdated);
    PathStatusConnectCallback(statusUpdated);

    // Start main task callback
    DelayedCallbackDispatch(pathPlannerHandle);

    return 0;
}

/**
 * Module initialization
 */
int32_t PathPlannerInitialize()
{
    FlightPlanInitialize();
    PathActionInitialize();
    PathStatusInitialize();
    PathDesiredInitialize();
    PositionStateInitialize();
    AirspeedStateInitialize();
    VelocityStateInitialize();
    WaypointInitialize();
    WaypointActiveInitialize();

    pathPlannerHandle = DelayedCallbackCreate(&pathPlannerTask, CALLBACK_PRIORITY_REGULAR, TASK_PRIORITY, STACK_SIZE_BYTES);
    pathDesiredUpdaterHandle = DelayedCallbackCreate(&updatePathDesired, CALLBACK_PRIORITY_CRITICAL, TASK_PRIORITY, STACK_SIZE_BYTES);

    return 0;
}

MODULE_INITCALL(PathPlannerInitialize, PathPlannerStart);

/**
 * Module task
 */
static void pathPlannerTask()
{
    DelayedCallbackSchedule(pathPlannerHandle, PATH_PLANNER_UPDATE_RATE_MS, CALLBACK_UPDATEMODE_SOONER);

    bool endCondition = false;

    FlightStatusData flightStatus;
    FlightStatusGet(&flightStatus);
    if (flightStatus.FlightMode != FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER) {
        pathplanner_active = false;

        return;
    }

    WaypointActiveGet(&waypointActive);

    if (pathplanner_active == false) {
        pathplanner_active   = true;

        // This triggers callback to update variable
        waypointActive.Index = 0;
        WaypointActiveSet(&waypointActive);

        return;
    }

    WaypointInstGet(waypointActive.Index, &waypoint);
    PathActionInstGet(waypoint.Action, &pathAction);
    PathStatusData pathStatus;
    PathStatusGet(&pathStatus);
    PathDesiredData pathDesired;
    PathDesiredGet(&pathDesired);

    // delay next step until path follower has acknowledged the path mode
    if (pathStatus.UID != pathDesired.UID) {
        return;
    }

    // negative destinations DISABLE this feature
    if (pathStatus.Status == PATHSTATUS_STATUS_CRITICAL && waypointActive.Index != pathAction.ErrorDestination && pathAction.ErrorDestination >= 0) {
        setWaypoint(pathAction.ErrorDestination);
        return;
    }

    // check if condition has been met
    endCondition = pathConditionCheck();
    // decide what to do
    switch (pathAction.Command) {
    case PATHACTION_COMMAND_ONNOTCONDITIONNEXTWAYPOINT:
        endCondition = !endCondition;
    case PATHACTION_COMMAND_ONCONDITIONNEXTWAYPOINT:
        if (endCondition) {
            setWaypoint(waypointActive.Index + 1);
        }
        break;
    case PATHACTION_COMMAND_ONNOTCONDITIONJUMPWAYPOINT:
        endCondition = !endCondition;
    case PATHACTION_COMMAND_ONCONDITIONJUMPWAYPOINT:
        if (endCondition) {
            if (pathAction.JumpDestination < 0) {
                // waypoint ids <0 code relative jumps
                setWaypoint(waypointActive.Index - pathAction.JumpDestination);
            } else {
                setWaypoint(pathAction.JumpDestination);
            }
        }
        break;
    case PATHACTION_COMMAND_IFCONDITIONJUMPWAYPOINTELSENEXTWAYPOINT:
        if (endCondition) {
            if (pathAction.JumpDestination < 0) {
                // waypoint ids <0 code relative jumps
                setWaypoint(waypointActive.Index - pathAction.JumpDestination);
            } else {
                setWaypoint(pathAction.JumpDestination);
            }
        } else {
            setWaypoint(waypointActive.Index + 1);
        }
        break;
    }
}

// callback function when status changed, issue execution of state machine
void commandUpdated(__attribute__((unused)) UAVObjEvent *ev)
{
    DelayedCallbackDispatch(pathDesiredUpdaterHandle);
}

// callback function when waypoints changed in any way, update pathDesired
void statusUpdated(__attribute__((unused)) UAVObjEvent *ev)
{
    DelayedCallbackDispatch(pathPlannerHandle);
}


// callback function when waypoints changed in any way, update pathDesired
void updatePathDesired()
{
    // only ever touch pathDesired if pathplanner is enabled
    if (!pathplanner_active) {
        return;
    }

    PathDesiredData pathDesired;

    // find out current waypoint
    WaypointActiveGet(&waypointActive);

    WaypointInstGet(waypointActive.Index, &waypoint);
    PathActionInstGet(waypoint.Action, &pathAction);

    pathDesired.End.North = waypoint.Position.North;
    pathDesired.End.East  = waypoint.Position.East;
    pathDesired.End.Down  = waypoint.Position.Down;
    pathDesired.EndingVelocity    = waypoint.Velocity;
    pathDesired.Mode = pathAction.Mode;
    pathDesired.ModeParameters[0] = pathAction.ModeParameters[0];
    pathDesired.ModeParameters[1] = pathAction.ModeParameters[1];
    pathDesired.ModeParameters[2] = pathAction.ModeParameters[2];
    pathDesired.ModeParameters[3] = pathAction.ModeParameters[3];
    pathDesired.UID = waypointActive.Index;

    if (waypointActive.Index == 0) {
        PositionStateData positionState;
        PositionStateGet(&positionState);
        // First waypoint has itself as start point (used to be home position but that proved dangerous when looping)

        /*pathDesired.Start[PATHDESIRED_START_NORTH] =  waypoint.Position[WAYPOINT_POSITION_NORTH];
           pathDesired.Start[PATHDESIRED_START_EAST] =  waypoint.Position[WAYPOINT_POSITION_EAST];
           pathDesired.Start[PATHDESIRED_START_DOWN] =  waypoint.Position[WAYPOINT_POSITION_DOWN];*/
        pathDesired.Start.North = positionState.North;
        pathDesired.Start.East  = positionState.East;
        pathDesired.Start.Down  = positionState.Down;
        pathDesired.StartingVelocity = pathDesired.EndingVelocity;
    } else {
        // Get previous waypoint as start point
        WaypointData waypointPrev;
        WaypointInstGet(waypointActive.Index - 1, &waypointPrev);

        pathDesired.Start.North = waypointPrev.Position.North;
        pathDesired.Start.East  = waypointPrev.Position.East;
        pathDesired.Start.Down  = waypointPrev.Position.Down;
        pathDesired.StartingVelocity = waypointPrev.Velocity;
    }
    PathDesiredSet(&pathDesired);
}

// helper function to go to a specific waypoint
static void setWaypoint(uint16_t num)
{
    FlightPlanData flightPlan;

    FlightPlanGet(&flightPlan);

    // here it is assumed that the flight plan has been validated (waypoint count is consistent)
    if (num >= flightPlan.WaypointCount) {
        // path plans wrap around
        num = 0;
    }

    waypointActive.Index = num;
    WaypointActiveSet(&waypointActive);
}

// execute the appropriate condition and report result
static uint8_t pathConditionCheck()
{
    // i thought about a lookup table, but a switch is safer considering there could be invalid EndCondition ID's
    switch (pathAction.EndCondition) {
    case PATHACTION_ENDCONDITION_NONE:
        return conditionNone();

        break;
    case PATHACTION_ENDCONDITION_TIMEOUT:
        return conditionTimeOut();

        break;
    case PATHACTION_ENDCONDITION_DISTANCETOTARGET:
        return conditionDistanceToTarget();

        break;
    case PATHACTION_ENDCONDITION_LEGREMAINING:
        return conditionLegRemaining();

        break;
    case PATHACTION_ENDCONDITION_BELOWERROR:
        return conditionBelowError();

        break;
    case PATHACTION_ENDCONDITION_ABOVEALTITUDE:
        return conditionAboveAltitude();

        break;
    case PATHACTION_ENDCONDITION_ABOVESPEED:
        return conditionAboveSpeed();

        break;
    case PATHACTION_ENDCONDITION_POINTINGTOWARDSNEXT:
        return conditionPointingTowardsNext();

        break;
    case PATHACTION_ENDCONDITION_PYTHONSCRIPT:
        return conditionPythonScript();

        break;
    case PATHACTION_ENDCONDITION_IMMEDIATE:
        return conditionImmediate();

        break;
    default:
        // invalid endconditions are always true to prevent freezes
        return true;

        break;
    }
}

/* the None condition is always false */
static uint8_t conditionNone()
{
    return false;
}

/**
 * the Timeout condition measures time this waypoint is active
 * Parameter 0:  timeout in seconds
 */
static uint8_t conditionTimeOut()
{
    static uint16_t toWaypoint;
    static uint32_t toStarttime;

    // reset timer if waypoint changed
    if (waypointActive.Index != toWaypoint) {
        toWaypoint  = waypointActive.Index;
        toStarttime = PIOS_DELAY_GetRaw();
    }
    if (PIOS_DELAY_DiffuS(toStarttime) >= 1e6f * pathAction.ConditionParameters[0]) {
        // make sure we reinitialize even if the same waypoint comes twice
        toWaypoint = 0xFFFF;
        return true;
    }
    return false;
}

/**
 * the DistanceToTarget measures distance to a waypoint
 * returns true if closer
 * Parameter 0:  distance in meters
 * Parameter 1:  flag: 0=2d 1=3d
 */
static uint8_t conditionDistanceToTarget()
{
    float distance;
    PositionStateData positionState;

    PositionStateGet(&positionState);
    if (pathAction.ConditionParameters[1] > 0.5f) {
        distance = sqrtf(powf(waypoint.Position.North - positionState.North, 2)
                         + powf(waypoint.Position.East - positionState.East, 2)
                         + powf(waypoint.Position.Down - positionState.Down, 2));
    } else {
        distance = sqrtf(powf(waypoint.Position.North - positionState.North, 2)
                         + powf(waypoint.Position.East - positionState.East, 2));
    }

    if (distance <= pathAction.ConditionParameters[0]) {
        return true;
    }
    return false;
}


/**
 * the LegRemaining measures how far the pathfollower got on a linear path segment
 * returns true if closer to destination (path more complete)
 * Parameter 0:  relative distance (0= complete, 1= just starting)
 */
static uint8_t conditionLegRemaining()
{
    PathDesiredData pathDesired;
    PositionStateData positionState;

    PathDesiredGet(&pathDesired);
    PositionStateGet(&positionState);

    float cur[3] = { positionState.North, positionState.East, positionState.Down };
    struct path_status progress;

    path_progress(cast_struct_to_array(pathDesired.Start, pathDesired.Start.North),
                  cast_struct_to_array(pathDesired.End, pathDesired.End.North),
                  cur, &progress, pathDesired.Mode);
    if (progress.fractional_progress >= 1.0f - pathAction.ConditionParameters[0]) {
        return true;
    }
    return false;
}

/**
 * the BelowError measures the error on a path segment
 * returns true if error is below margin
 * Parameter 0: error margin (in m)
 */
static uint8_t conditionBelowError()
{
    PathDesiredData pathDesired;
    PositionStateData positionState;

    PathDesiredGet(&pathDesired);
    PositionStateGet(&positionState);

    float cur[3] = { positionState.North, positionState.East, positionState.Down };
    struct path_status progress;

    path_progress(cast_struct_to_array(pathDesired.Start, pathDesired.Start.North),
                  cast_struct_to_array(pathDesired.End, pathDesired.End.North),
                  cur, &progress, pathDesired.Mode);
    if (progress.error <= pathAction.ConditionParameters[0]) {
        return true;
    }
    return false;
}

/**
 * the AboveAltitude measures the flight altitude relative to home position
 * returns true if above critical altitude
 * WARNING! Altitudes are always negative (down coordinate)
 * Parameter 0:  altitude in meters (negative!)
 */
static uint8_t conditionAboveAltitude()
{
    PositionStateData positionState;

    PositionStateGet(&positionState);

    if (positionState.Down <= pathAction.ConditionParameters[0]) {
        return true;
    }
    return false;
}

/**
 * the AboveSpeed measures the movement speed (3d)
 * returns true if above critical speed
 * Parameter 0:  speed in m/s
 * Parameter 1:  flag: 0=groundspeed 1=airspeed
 */
static uint8_t conditionAboveSpeed()
{
    VelocityStateData velocityState;

    VelocityStateGet(&velocityState);
    float velocity = sqrtf(velocityState.North * velocityState.North + velocityState.East * velocityState.East + velocityState.Down * velocityState.Down);

    // use airspeed if requested and available
    if (pathAction.ConditionParameters[1] > 0.5f) {
        AirspeedStateData airspeed;
        AirspeedStateGet(&airspeed);
        velocity = airspeed.CalibratedAirspeed;
    }

    if (velocity >= pathAction.ConditionParameters[0]) {
        return true;
    }
    return false;
}


/**
 * the PointingTowardsNext measures the horizontal movement vector direction relative to the next waypoint
 * regardless whether this waypoint will ever be active (Command could jump to another one on true)
 * This is useful for curve segments where the craft should stop circling when facing a certain way (usually the next waypoint)
 * returns true if within a certain angular margin
 * Parameter 0:  degrees variation allowed
 */
static uint8_t conditionPointingTowardsNext()
{
    uint16_t nextWaypointId = waypointActive.Index + 1;

    if (nextWaypointId >= UAVObjGetNumInstances(WaypointHandle())) {
        nextWaypointId = 0;
    }
    WaypointData nextWaypoint;
    WaypointInstGet(nextWaypointId, &nextWaypoint);

    float angle1 = atan2f((nextWaypoint.Position.North - waypoint.Position.North), (nextWaypoint.Position.East - waypoint.Position.East));

    VelocityStateData velocity;
    VelocityStateGet(&velocity);
    float angle2 = atan2f(velocity.North, velocity.East);

    // calculate the absolute angular difference
    angle1 = fabsf(RAD2DEG(angle1 - angle2));
    while (angle1 > 360) {
        angle1 -= 360;
    }

    if (angle1 <= pathAction.ConditionParameters[0]) {
        return true;
    }
    return false;
}

/**
 * the PythonScript is supposed to read the output of a PyMite program running at the same time
 * and return based on its output, likely read out through some to be defined UAVObject
 * TODO XXX NOT YET IMPLEMENTED
 * returns always true until implemented
 * Parameter 0-3: defined by user script
 */
static uint8_t conditionPythonScript()
{
    return true;
}

/* the immediate condition is always true */
static uint8_t conditionImmediate()
{
    return true;
}

/**
 * @}
 * @}
 */
