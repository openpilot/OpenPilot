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
#include "flightstatus.h"
#include "positionactual.h"
#include "positiondesired.h"
#include "waypoint.h"
#include "waypointactive.h"

// Private constants
#define STACK_SIZE_BYTES 1500
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define MAX_QUEUE_SIZE 2

// Private types

// Private variables
static xTaskHandle taskHandle;
static xQueueHandle queue;

// Private functions
static void pathPlannerTask(void *parameters);

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
	
	WaypointInitialize();
	WaypointActiveInitialize();
	
	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	return 0;
}

MODULE_INITCALL(PathPlannerInitialize, PathPlannerStart)

/**
 * Module task
 */
static void pathPlannerTask(void *parameters)
{
	FlightStatusData flightStatus;
	PositionActualData positionActual;

	WaypointActiveData waypointActive;
	WaypointData waypoint;

	const float MIN_RADIUS = 2.0f; // Radius to consider at waypoint (m)

	// Main thread loop
	while (1)
	{
		FlightStatusGet(&flightStatus);

		if (flightStatus.FlightMode != FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER)
			continue;

		PositionActualGet(&positionActual);
		WaypointActiveGet(&waypointActive);
		WaypointInstGet(waypointActive.Index, &waypoint);

		float r2 = powf(positionActual.North - waypoint.Position[WAYPOINT_POSITION_NORTH], 2) +
		     powf(positionActual.East - waypoint.Position[WAYPOINT_POSITION_EAST], 2) +
			powf(positionActual.Down - waypoint.Position[WAYPOINT_POSITION_DOWN], 2);

		// We hit this waypoint
		if (r2 < (MIN_RADIUS * MIN_RADIUS)) {
			if (waypoint.Action == WAYPOINT_ACTION_NEXT) {
				waypointActive.Index++;
				WaypointActiveSet(&waypointActive);

				if(WaypointInstGet(waypointActive.Index, &waypoint) != 0) {
					// Oh shit, tried to go to non-existant waypoint
					continue;
				}

				PositionDesiredData positionDesired;
				PositionDesiredGet(&positionDesired);
				positionDesired.North = waypoint.Position[WAYPOINT_POSITION_NORTH];
				positionDesired.East = waypoint.Position[WAYPOINT_POSITION_EAST];
				positionDesired.Down = waypoint.Position[WAYPOINT_POSITION_DOWN];
				PositionDesiredSet(&positionDesired);
			} else if (waypointAction == WAYPOINT_ACTION_RTH) {
				// Fly back to the home location but 20 m above it
				PositionDesiredData positionDesired;
				PositionDesiredGet(&positionDesired);
				positionDesired.North = 0;
				positionDesired.East = 0;
				positionDesired.Down = -20;
				PositionDesiredSet(&positionDesired);
			}
		}
		vTaskDelay(10);
	}
}

/**
  * @}
  * @}
  */
