/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup FlightSituationModule FlightSituation Module
 * @brief Sensor Merge of all Flight data into most likely situation
 * @note This object updates the @ref FlightSituationActual UAVObject
 * @{ 
 *
 * @file       flightsituation.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Attitude stabilization module.
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
#include "flightsituation.h"
#include "flightsituationactual.h"
#include "attitudeactual.h"
#include "altitudeactual.h"
#include "headingactual.h"
#include "positionactual.h"
#include "systemsettings.h"


// Private constants
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+4)

// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void flightSituationTask(void* parameters);

/**
 * Module initialization
 */
int32_t FlightSituationInitialize()
{
	// Initialize variables

	// Start main task
	xTaskCreate(flightSituationTask, (signed char*)"FlightSituation", STACK_SIZE, NULL, TASK_PRIORITY, &taskHandle);

	return 0;
}

/**
 * Module task
 */
static void flightSituationTask(void* parameters)
{
	AttitudeActualData attitudeActual;
	AltitudeActualData altitudeActual;
	HeadingActualData headingActual;
	PositionActualData positionActual;
	SystemSettingsData systemSettings;
	FlightSituationActualData flightSituationActual;
	portTickType lastSysTime;

	// private variables
	float altitudeLast=0.0;
	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1)
	{
		// Read settings and other objects
		SystemSettingsGet(&systemSettings);
		AttitudeActualGet(&attitudeActual);
		AltitudeActualGet(&altitudeActual);
		HeadingActualGet(&headingActual);
		PositionActualGet(&positionActual);
		FlightSituationActualGet(&flightSituationActual);

		// TODO: plausibility check of GPS data,
		// innertial navigation with kalman feed-in of GPS data
		// sensor fusion
		// STUB code:
		flightSituationActual.Latitude = positionActual.Latitude;
		flightSituationActual.Longitude = positionActual.Longitude;

		
		// TODO: fuse altitude information with GPS data plus 
		// plausibility check
		// STUB Code:
		flightSituationActual.Altitude = altitudeActual.Altitude;

		// TODO: get altitude over ground from somewhere:
		// method 1: reflection sensor
		// method 2: crude database with ground hight information
		// method 3: manual setting of ground hight at start pos
		// STUB code:
		flightSituationActual.ATG = altitudeActual.Altitude;
		
		// TODO: use some more sophisticated Kalman filtering
		// and several sources (including speed and pitch!)
		// to get this one right
		flightSituationActual.Climbrate = 0.9*flightSituationActual.Climbrate + 0.1*((flightSituationActual.Altitude-altitudeLast)*10);
		// the times 10 is because timescale is 1/10th
		// of a second right now
		altitudeLast = flightSituationActual.Altitude;

		// TODO: heading: sensor fusion from AttitudeActual.yaw
		// and HeadingActual - with plausibility checks
		// BUT ??? what to do with heli- / multicopters
		// that can fly sideways?
		// Maybe the AHRS can give us a movement vector too?
		flightSituationActual.Heading = positionActual.Heading;

		// TODO: airspeed - is THE critical measure to prevent stall
		// and judge which maneuvers are safe.
		// However AFAIK we have no sensor for it yet, do we?
		// Even with moderate winds, a glider or other UAV
		// can easily fly (seemingly) backwards, so airspeed
		// and groundspeed can significantly differ!
		flightSituationActual.Airspeed = positionActual.Groundspeed;

		// TODO: this can possibly be taken from GPS
		// with just a bit of plausibility checking
		// and replacing by above if missing
		flightSituationActual.Course = positionActual.Heading;
		flightSituationActual.Groundspeed = positionActual.Groundspeed;


		FlightSituationActualSet(&flightSituationActual);

		// Clear alarms
		// TODO create a new alarm
		//AlarmsClear(SYSTEMALARMS_ALARM_STABILIZATION);

		// Wait until next update
		// TODO non-hardcoded update rate
		vTaskDelayUntil(&lastSysTime, 100 / portTICK_RATE_MS );
	}
}

/** 
  * @}
  * @}
  */
