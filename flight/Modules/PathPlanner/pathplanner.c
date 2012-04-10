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
#include "guidancesettings.h"
#include "pathdesired.h"
#include "positionactual.h"
#include "positiondesired.h"
#include "waypoint.h"
#include "waypointactive.h"

// Private constants
#define STACK_SIZE_BYTES 512
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define MAX_QUEUE_SIZE 2

// Private types

// Private variables
static xTaskHandle taskHandle;
static xQueueHandle queue;
static WaypointActiveData waypointActive;
static WaypointData waypoint;
static GuidanceSettingsData guidanceSettings;

// Private functions
static void pathPlannerTask(void *parameters);
static void waypointsUpdated(UAVObjEvent * ev);
static void createPath();
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
int32_t bad_inits;
int32_t bad_reads;
static void pathPlannerTask(void *parameters)
{
	WaypointConnectCallback(waypointsUpdated);
	WaypointActiveConnectCallback(waypointsUpdated);

	FlightStatusData flightStatus;
	PathDesiredData pathDesired;
	PositionActualData positionActual;
	PositionDesiredData positionDesired;
	
	createPath();
	
	const float MIN_RADIUS = 4.0f; // Radius to consider at waypoint (m)

	// Main thread loop
	bool pathplanner_active = false;
	while (1)
	{

		vTaskDelay(20);

		FlightStatusGet(&flightStatus);
		if (flightStatus.FlightMode != FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER) {
			pathplanner_active = false;
			continue;
		}

		if(pathplanner_active == false) {
			// This triggers callback to update variable
			WaypointActiveGet(&waypointActive);
			waypointActive.Index = 0;
			WaypointActiveSet(&waypointActive);

			pathplanner_active = true;
			continue;
		}

		GuidanceSettingsGet(&guidanceSettings);
		
		switch(guidanceSettings.PathMode) {
			case GUIDANCESETTINGS_PATHMODE_ENDPOINT:
				PositionActualGet(&positionActual);
				
				float r2 = powf(positionActual.North - waypoint.Position[WAYPOINT_POSITION_NORTH], 2) +
				powf(positionActual.East - waypoint.Position[WAYPOINT_POSITION_EAST], 2) +
				powf(positionActual.Down - waypoint.Position[WAYPOINT_POSITION_DOWN], 2);
				
				// We hit this waypoint
				if (r2 < (MIN_RADIUS * MIN_RADIUS)) {
					switch(waypoint.Action) {
						case WAYPOINT_ACTION_NEXT:
							waypointActive.Index++;
							WaypointActiveSet(&waypointActive);

							break;
						case WAYPOINT_ACTION_RTH:
							// Fly back to the home location but 20 m above it
							PositionDesiredGet(&positionDesired);
							positionDesired.North = 0;
							positionDesired.East = 0;
							positionDesired.Down = -20;
							PositionDesiredSet(&positionDesired);
							break;
						default:
							PIOS_DEBUG_Assert(0);
					}
				}

				break;

			case GUIDANCESETTINGS_PATHMODE_PATH:

				PathDesiredGet(&pathDesired);
				PositionActualGet(&positionActual);

				float cur[3] = {positionActual.North, positionActual.East, positionActual.Down};
				struct path_status progress;

				path_progress(pathDesired.Start, pathDesired.End, cur, &progress);

				if (progress.fractional_progress >= 1) {
					switch(waypoint.Action) {
						case WAYPOINT_ACTION_NEXT:
							waypointActive.Index++;
							WaypointActiveSet(&waypointActive);

							break;
						case WAYPOINT_ACTION_RTH:
							// Fly back to the home location but 20 m above it
							PositionDesiredGet(&positionDesired);
							positionDesired.North = 0;
							positionDesired.East = 0;
							positionDesired.Down = -20;
							PositionDesiredSet(&positionDesired);
							break;
						default:
							PIOS_DEBUG_Assert(0);
					}
				}

				break;
		}
	}
}

/**
 * On changed waypoints or active waypoint update position desired
 * if we are in charge
 */
static void waypointsUpdated(UAVObjEvent * ev)
{
	FlightStatusData flightStatus;
	FlightStatusGet(&flightStatus);
	if (flightStatus.FlightMode != FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER)
		return;
	
	WaypointActiveGet(&waypointActive);
	WaypointInstGet(waypointActive.Index, &waypoint);
	
	GuidanceSettingsGet(&guidanceSettings);

	switch(guidanceSettings.PathMode) {
		case GUIDANCESETTINGS_PATHMODE_ENDPOINT:
		{
			PositionDesiredData positionDesired;
			PositionDesiredGet(&positionDesired);
			positionDesired.North = waypoint.Position[WAYPOINT_POSITION_NORTH];
			positionDesired.East = waypoint.Position[WAYPOINT_POSITION_EAST];
			positionDesired.Down = waypoint.Position[WAYPOINT_POSITION_DOWN];
			PositionDesiredSet(&positionDesired);
		}
			break;

		case GUIDANCESETTINGS_PATHMODE_PATH:
		{
			PathDesiredData pathDesired;
			pathDesired.StartingVelocity = 2;
			pathDesired.EndingVelocity = 2;

			pathDesired.End[PATHDESIRED_END_NORTH] = waypoint.Position[WAYPOINT_POSITION_NORTH];
			pathDesired.End[PATHDESIRED_END_EAST] = waypoint.Position[WAYPOINT_POSITION_EAST];
			pathDesired.End[PATHDESIRED_END_DOWN] = waypoint.Position[WAYPOINT_POSITION_DOWN];

			if(waypointActive.Index == 0) {
				// Get current position as start point
				PositionActualData positionActual;
				PositionActualGet(&positionActual);

				pathDesired.Start[PATHDESIRED_START_NORTH] = positionActual.North;
				pathDesired.Start[PATHDESIRED_START_EAST] = positionActual.East;
				pathDesired.Start[PATHDESIRED_START_DOWN] = positionActual.Down;
			} else {
				// Get previous waypoint as start point
				WaypointData waypointPrev;
				WaypointInstGet(waypointActive.Index - 1, &waypointPrev);

				pathDesired.Start[PATHDESIRED_END_NORTH] = waypointPrev.Position[WAYPOINT_POSITION_NORTH];
				pathDesired.Start[PATHDESIRED_END_EAST] = waypointPrev.Position[WAYPOINT_POSITION_EAST];
				pathDesired.Start[PATHDESIRED_END_DOWN] = waypointPrev.Position[WAYPOINT_POSITION_DOWN];
			}

			PathDesiredSet(&pathDesired);
		}
			break;
	}
}

static void createPath()
{
	// Draw O
	WaypointData waypoint;
	for(uint32_t i = 0; i < 20; i++) {
		waypoint.Position[1] = 30 * cos(i / 19.0 * 2 * M_PI);
		waypoint.Position[0] = 50 * sin(i / 19.0 * 2 * M_PI);
		waypoint.Position[2] = -50;
		waypoint.Action = WAYPOINT_ACTION_NEXT;
		WaypointCreateInstance();
		bad_inits += (WaypointInstSet(i, &waypoint) != 0);
	}
	
	// Draw P
	for(uint32_t i = 20; i < 35; i++) {
		waypoint.Position[1] = 55 + 20 * cos(i / 10.0 * M_PI - M_PI / 2);
		waypoint.Position[0] = 25 + 25 * sin(i / 10.0 * M_PI - M_PI / 2);
		waypoint.Position[2] = -50;
		waypoint.Action = WAYPOINT_ACTION_NEXT;
		WaypointCreateInstance();
		bad_inits += (WaypointInstSet(i, &waypoint) != 0);
	}
		
	waypoint.Position[1] = 35;
	waypoint.Position[0] = -50;
	waypoint.Position[2] = -50;
	waypoint.Action = WAYPOINT_ACTION_NEXT;
	WaypointCreateInstance();
	WaypointInstSet(35, &waypoint);
	
	// Draw Box
	waypoint.Position[1] = 35;
	waypoint.Position[0] = -60;
	waypoint.Position[2] = -30;
	waypoint.Action = WAYPOINT_ACTION_NEXT;
	WaypointCreateInstance();
	WaypointInstSet(36, &waypoint);

	waypoint.Position[1] = 85;
	waypoint.Position[0] = -60;
	waypoint.Position[2] = -30;
	waypoint.Action = WAYPOINT_ACTION_NEXT;
	WaypointCreateInstance();
	WaypointInstSet(37, &waypoint);

	waypoint.Position[1] = 85;
	waypoint.Position[0] = 60;
	waypoint.Position[2] = -30;
	waypoint.Action = WAYPOINT_ACTION_NEXT;
	WaypointCreateInstance();
	WaypointInstSet(38, &waypoint);
	
	waypoint.Position[1] = -40;
	waypoint.Position[0] = 60;
	waypoint.Position[2] = -30;
	waypoint.Action = WAYPOINT_ACTION_NEXT;
	WaypointCreateInstance();
	WaypointInstSet(39, &waypoint);

	waypoint.Position[1] = -40;
	waypoint.Position[0] = -60;
	waypoint.Position[2] = -30;
	waypoint.Action = WAYPOINT_ACTION_NEXT;
	WaypointCreateInstance();
	WaypointInstSet(40, &waypoint);

	waypoint.Position[1] = 35;
	waypoint.Position[0] = -60;
	waypoint.Position[2] = -30;
	waypoint.Action = WAYPOINT_ACTION_NEXT;
	WaypointCreateInstance();
	WaypointInstSet(41, &waypoint);

}

/**
 * @}
 * @}
 */
