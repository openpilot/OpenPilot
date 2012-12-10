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
#include "paths.h"

#include "flightstatus.h"
#include "baroairspeed.h" // TODO: make baroairspeed optional
#include "pathaction.h"
#include "pathdesired.h"
#include "pathstatus.h"
#include "positionactual.h"
#include "velocityactual.h"
#include "waypoint.h"
#include "waypointactive.h"

// Private constants
#define STACK_SIZE_BYTES 1024
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define MAX_QUEUE_SIZE 2
#define F_PI 3.141526535897932f
#define RAD2DEG (180.0f/F_PI)
#define PATH_PLANNER_UPDATE_RATE_MS 20

// Private types

// Private functions
static void pathPlannerTask(void *parameters);
static void updatePathDesired(UAVObjEvent * ev);
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
static xTaskHandle taskHandle;
static WaypointActiveData waypointActive;
static WaypointData waypoint;
static PathActionData pathAction;
static bool pathplanner_active = false;


/**
 * Module initialization
 */
int32_t PathPlannerStart()
{
	taskHandle = NULL;

	// Start VM thread
	xTaskCreate(pathPlannerTask, (signed char *)"PathPlanner", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_PATHPLANNER, taskHandle);

	return 0;
}

/**
 * Module initialization
 */
int32_t PathPlannerInitialize()
{
	taskHandle = NULL;

	PathActionInitialize();
	PathStatusInitialize();
	PathDesiredInitialize();
	PositionActualInitialize();
	BaroAirspeedInitialize();
	VelocityActualInitialize();
	WaypointInitialize();
	WaypointActiveInitialize();
	
	return 0;
}

MODULE_INITCALL(PathPlannerInitialize, PathPlannerStart)

/**
 * Module task
 */
static void pathPlannerTask(void *parameters)
{
	// when the active waypoint changes, update pathDesired
	WaypointConnectCallback(updatePathDesired);
	WaypointActiveConnectCallback(updatePathDesired);
	PathActionConnectCallback(updatePathDesired);

	FlightStatusData flightStatus;
	PathDesiredData pathDesired;
	PathStatusData pathStatus;
	
	// Main thread loop
	bool endCondition = false;
	while (1)
	{

		vTaskDelay(PATH_PLANNER_UPDATE_RATE_MS);

		FlightStatusGet(&flightStatus);
		if (flightStatus.FlightMode != FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER) {
			pathplanner_active = false;
			continue;
		}

		WaypointActiveGet(&waypointActive);

		if(pathplanner_active == false) {

			pathplanner_active = true;

			// This triggers callback to update variable
			waypointActive.Index = 0;
			WaypointActiveSet(&waypointActive);

			continue;
		}

		WaypointInstGet(waypointActive.Index,&waypoint);
		PathActionInstGet(waypoint.Action, &pathAction);
		PathStatusGet(&pathStatus);
		PathDesiredGet(&pathDesired);

		// delay next step until path follower has acknowledged the path mode
		if (pathStatus.UID != pathDesired.UID) {
			continue;
		}

		// negative destinations DISABLE this feature
		if (pathStatus.Status == PATHSTATUS_STATUS_CRITICAL && waypointActive.Index != pathAction.ErrorDestination && pathAction.ErrorDestination >= 0) {
			setWaypoint(pathAction.ErrorDestination);
			continue;
		}

		// check if condition has been met
		endCondition = pathConditionCheck();

		// decide what to do
		switch (pathAction.Command) {
			case PATHACTION_COMMAND_ONNOTCONDITIONNEXTWAYPOINT:
				endCondition = !endCondition;
			case PATHACTION_COMMAND_ONCONDITIONNEXTWAYPOINT:
				if (endCondition) setWaypoint(waypointActive.Index+1);
				break;
			case PATHACTION_COMMAND_ONNOTCONDITIONJUMPWAYPOINT:
				endCondition = !endCondition;
			case PATHACTION_COMMAND_ONCONDITIONJUMPWAYPOINT:
				if (endCondition) {
					if (pathAction.JumpDestination<0) {
						// waypoint ids <0 code relative jumps
						setWaypoint(waypointActive.Index - pathAction.JumpDestination );
					} else {
						setWaypoint(pathAction.JumpDestination);
					}
				}
				break;
			case PATHACTION_COMMAND_IFCONDITIONJUMPWAYPOINTELSENEXTWAYPOINT:
				if (endCondition) {
					if (pathAction.JumpDestination<0) {
						// waypoint ids <0 code relative jumps
						setWaypoint(waypointActive.Index - pathAction.JumpDestination );
					} else {
						setWaypoint(pathAction.JumpDestination);
					}
				} else {
					setWaypoint(waypointActive.Index+1);
				}
				break;
		}
	}
}

// callback function when waypoints changed in any way, update pathDesired
void updatePathDesired(UAVObjEvent * ev) {

	// only ever touch pathDesired if pathplanner is enabled
	if (!pathplanner_active) return;

	// use local variables, dont use stack since this is huge and a callback,
	// dont use the globals because we cant use mutexes here
	static WaypointActiveData waypointActive;
	static PathActionData pathAction;
	static WaypointData waypoint;
	static PathDesiredData pathDesired;

	// find out current waypoint
	WaypointActiveGet(&waypointActive);

	WaypointInstGet(waypointActive.Index,&waypoint);
	PathActionInstGet(waypoint.Action, &pathAction);

	pathDesired.End[PATHDESIRED_END_NORTH] = waypoint.Position[WAYPOINT_POSITION_NORTH];
	pathDesired.End[PATHDESIRED_END_EAST]  = waypoint.Position[WAYPOINT_POSITION_EAST];
	pathDesired.End[PATHDESIRED_END_DOWN]  = waypoint.Position[WAYPOINT_POSITION_DOWN];
	pathDesired.EndingVelocity = waypoint.Velocity;
	pathDesired.Mode = pathAction.Mode;
	pathDesired.ModeParameters[0] = pathAction.ModeParameters[0]; 
	pathDesired.ModeParameters[1] = pathAction.ModeParameters[1]; 
	pathDesired.ModeParameters[2] = pathAction.ModeParameters[2]; 
	pathDesired.ModeParameters[3] = pathAction.ModeParameters[3]; 
	pathDesired.UID = waypointActive.Index;

	if(waypointActive.Index == 0) {
		PositionActualData positionActual;
		PositionActualGet(&positionActual);
		// First waypoint has itself as start point (used to be home position but that proved dangerous when looping)

		/*pathDesired.Start[PATHDESIRED_START_NORTH] =  waypoint.Position[WAYPOINT_POSITION_NORTH];
		pathDesired.Start[PATHDESIRED_START_EAST] =  waypoint.Position[WAYPOINT_POSITION_EAST];
		pathDesired.Start[PATHDESIRED_START_DOWN] =  waypoint.Position[WAYPOINT_POSITION_DOWN];*/
		pathDesired.Start[PATHDESIRED_START_NORTH] = positionActual.North;
		pathDesired.Start[PATHDESIRED_START_EAST] = positionActual.East;
		pathDesired.Start[PATHDESIRED_START_DOWN] = positionActual.Down;
		pathDesired.StartingVelocity = pathDesired.EndingVelocity;
	} else {
		// Get previous waypoint as start point
		WaypointData waypointPrev;
		WaypointInstGet(waypointActive.Index - 1, &waypointPrev);

		pathDesired.Start[PATHDESIRED_START_NORTH] = waypointPrev.Position[WAYPOINT_POSITION_NORTH];
		pathDesired.Start[PATHDESIRED_START_EAST] = waypointPrev.Position[WAYPOINT_POSITION_EAST];
		pathDesired.Start[PATHDESIRED_START_DOWN] = waypointPrev.Position[WAYPOINT_POSITION_DOWN];
		pathDesired.StartingVelocity = waypointPrev.Velocity;
	}
	PathDesiredSet(&pathDesired);

}

// helper function to go to a specific waypoint
static void setWaypoint(uint16_t num) {
	// path plans wrap around
	if (num>=UAVObjGetNumInstances(WaypointHandle())) {
		num = 0;
	}

	waypointActive.Index = num;
	WaypointActiveSet(&waypointActive);

}

// execute the apropriate condition and report result
static uint8_t pathConditionCheck() {
	// i thought about a lookup table, but a switch is saver considering there could be invalid EndCondition ID's
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
static uint8_t conditionNone() {
	return false;
}

/**
 * the Timeout condition measures time this waypoint is active
 * Parameter 0:  timeout in seconds
 */
static uint8_t conditionTimeOut() {
	static uint16_t toWaypoint;
	static uint32_t toStarttime;

	// reset timer if waypoint changed
	if (waypointActive.Index!=toWaypoint) {
		toWaypoint = waypointActive.Index;
		toStarttime = PIOS_DELAY_GetRaw();
	}
	if (PIOS_DELAY_DiffuS(toStarttime) >= 1e6 * pathAction.ConditionParameters[0]) {
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
static uint8_t conditionDistanceToTarget() {
	float distance;
	PositionActualData positionActual;

	PositionActualGet(&positionActual);
	if (pathAction.ConditionParameters[1]>0.5f) {
		distance=sqrtf(powf( waypoint.Position[0]-positionActual.North ,2)
				+powf( waypoint.Position[1]-positionActual.East ,2)
				+powf( waypoint.Position[1]-positionActual.Down ,2));
	} else {
		distance=sqrtf(powf( waypoint.Position[0]-positionActual.North ,2)
				+powf( waypoint.Position[1]-positionActual.East ,2));
	}

	if (distance<=pathAction.ConditionParameters[0]) {
		return true;
	}
	return false;
}


/**
 * the LegRemaining measures how far the pathfollower got on a linear path segment 
 * returns true if closer to destination (path more complete)
 * Parameter 0:  relative distance (0= complete, 1= just starting)
 */
static uint8_t conditionLegRemaining() {

	PathDesiredData pathDesired;
	PositionActualData positionActual;
	PathDesiredGet(&pathDesired);
	PositionActualGet(&positionActual);

	float cur[3] = {positionActual.North, positionActual.East, positionActual.Down};
	struct path_status progress;

	path_progress(pathDesired.Start, pathDesired.End, cur, &progress, pathDesired.Mode);
	if ( progress.fractional_progress >= 1.0f - pathAction.ConditionParameters[0] ) {
		return true;
	}
	return false;
}

/**
 * the BelowError measures the error on a path segment 
 * returns true if error is below margin
 * Parameter 0: error margin (in m)
 */
static uint8_t conditionBelowError() {

	PathDesiredData pathDesired;
	PositionActualData positionActual;
	PathDesiredGet(&pathDesired);
	PositionActualGet(&positionActual);

	float cur[3] = {positionActual.North, positionActual.East, positionActual.Down};
	struct path_status progress;

	path_progress(pathDesired.Start, pathDesired.End, cur, &progress, pathDesired.Mode);
	if ( progress.error <= pathAction.ConditionParameters[0] ) {
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
static uint8_t conditionAboveAltitude() {
	PositionActualData positionActual;
	PositionActualGet(&positionActual);
	
	if (positionActual.Down <= pathAction.ConditionParameters[0]) {
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
static uint8_t conditionAboveSpeed() {

	VelocityActualData velocityActual;
	VelocityActualGet(&velocityActual);
	float velocity = sqrtf( velocityActual.North*velocityActual.North + velocityActual.East*velocityActual.East + velocityActual.Down*velocityActual.Down );

	// use airspeed if requested and available
	if (pathAction.ConditionParameters[1]>0.5f) {
		BaroAirspeedData baroAirspeed;
		BaroAirspeedGet (&baroAirspeed);
		if (baroAirspeed.BaroConnected == BAROAIRSPEED_BAROCONNECTED_TRUE) {
			velocity = baroAirspeed.TrueAirspeed;
		}
	}

	if ( velocity >= pathAction.ConditionParameters[0]) {
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
static uint8_t conditionPointingTowardsNext() {
	uint16_t nextWaypointId = waypointActive.Index+1;
	if (nextWaypointId>=UAVObjGetNumInstances(WaypointHandle())) {
		nextWaypointId = 0;
	}
	WaypointData nextWaypoint;
	WaypointInstGet(nextWaypointId,&nextWaypoint);

	float angle1 = atan2f((nextWaypoint.Position[0]-waypoint.Position[0]),(nextWaypoint.Position[1]-waypoint.Position[1]));

	VelocityActualData velocity;
	VelocityActualGet (&velocity);
	float angle2 = atan2f(velocity.North,velocity.East);

	// calculate the absolute angular difference
	angle1 = fabsf(RAD2DEG * (angle1 - angle2));
	while (angle1>360) angle1-=360;

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
static uint8_t conditionPythonScript() {
	return true;
}

/* the immediate condition is always true */
static uint8_t conditionImmediate() {
	return true;
}

/**
 * @}
 * @}
 */
