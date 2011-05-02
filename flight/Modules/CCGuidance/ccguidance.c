/**
 ******************************************************************************
 *
 * @file       ccguidance.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      CCGuidance for CopterControl. Fixed wing only.
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
 * Input object: GPSPosition
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

#include "openpilot.h"
#include "ccguidancesettings.h"
#include "gpsposition.h"
#include "positiondesired.h"	// object that will be updated by the module
#include "manualcontrol.h"
#include "manualcontrolcommand.h"
#include "stabilizationdesired.h"
#include "systemsettings.h"

// Private constants
#define MAX_QUEUE_SIZE 1
#define STACK_SIZE_BYTES 256
#define TASK_PRIORITY (tskIDLE_PRIORITY+2)
#define RAD2DEG (180.0/M_PI)
#define GEE 9.81
// Private types

// Private variables
static xTaskHandle ccguidanceTaskHandle;
static xQueueHandle queue;
static uint8_t positionHoldLast = 0;


// Private functions
static void ccguidanceTask(void *parameters);
static float bound(float val, float min, float max);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t CCGuidanceInitialize()
{
	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));
	
	// Listen for updates.
	GPSPositionConnectQueue(queue);
	
	// Start main task
	xTaskCreate(ccguidanceTask, (signed char *)"Guidance", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &ccguidanceTaskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_GUIDANCE, ccguidanceTaskHandle);

	return 0;
}

/**
 * Module thread, should not return.
 */
static void ccguidanceTask(void *parameters)
{
	SystemSettingsData systemSettings;
	CCGuidanceSettingsData ccguidanceSettings;
	ManualControlCommandData manualControl;

	portTickType thisTime;
	portTickType lastUpdateTime;
	UAVObjEvent ev;

	float altitudeError;
	float courseError;
	
	// Main task loop
	lastUpdateTime = xTaskGetTickCount();
	while (1) {
		CCGuidanceSettingsGet(&ccguidanceSettings);

		// Wait until the GPSPosition object is updated, if a timeout then go to failsafe
		if ( xQueueReceive(queue, &ev, ccguidanceSettings.UpdatePeriod / portTICK_RATE_MS) != pdTRUE )
		{
			AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_WARNING);
		} else {
			AlarmsClear(SYSTEMALARMS_ALARM_GUIDANCE);
		}
				
		// Continue collecting data if not enough time
		thisTime = xTaskGetTickCount();
		if( (thisTime - lastUpdateTime) < (ccguidanceSettings.UpdatePeriod / portTICK_RATE_MS) )
			continue;
		
		lastUpdateTime = xTaskGetTickCount();
		
		ManualControlCommandGet(&manualControl);
		SystemSettingsGet(&systemSettings);
		
		if ((PARSE_FLIGHT_MODE(manualControl.FlightMode) == FLIGHTMODE_GUIDANCE) &&
		    ((systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWING) ||
		     (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGELEVON) ||
		     (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGVTAIL) ))
		{
			StabilizationDesiredData stabDesired;
			StabilizationDesiredGet(&stabDesired);
			PositionDesiredData positionDesired;
			PositionDesiredGet(&positionDesired);
			GPSPositionData positionActual;
			GPSPositionGet(&positionActual);

			if(positionHoldLast == 0) {
				/* When enter position hold mode save current position */
				positionDesired.North = positionActual.Latitude;
				positionDesired.East = positionActual.Longitude;
				positionDesired.Down = positionActual.Altitude;
				PositionDesiredSet(&positionDesired);
				positionHoldLast = 1;
			}

			/* safety */
			if (positionActual.Status==GPSPOSITION_STATUS_FIX3D) {
						
				/* main position hold loop */
				/* 1. Altitude */

				altitudeError = positionDesired.Down - positionActual.Altitude;
				stabDesired.Pitch = bound(
					ccguidanceSettings.Pitch[CCGUIDANCESETTINGS_PITCH_NEUTRAL] +
					ccguidanceSettings.Pitch[CCGUIDANCESETTINGS_PITCH_KP] * altitudeError,
					ccguidanceSettings.Pitch[CCGUIDANCESETTINGS_PITCH_MIN],
					ccguidanceSettings.Pitch[CCGUIDANCESETTINGS_PITCH_MAX]);

				/* 2. Heading */

				courseError = RAD2DEG * atan2f(positionDesired.East-positionActual.Longitude,positionDesired.North-positionActual.Latitude) - positionActual.Heading;
				if (courseError<-180.) courseError+=360.;
				if (courseError>180.) courseError-=360.;

				stabDesired.Roll = bound(
					ccguidanceSettings.Roll[CCGUIDANCESETTINGS_ROLL_NEUTRAL] +
					ccguidanceSettings.Roll[CCGUIDANCESETTINGS_ROLL_KP] * courseError,
					-ccguidanceSettings.Roll[CCGUIDANCESETTINGS_ROLL_MAX],
					ccguidanceSettings.Roll[CCGUIDANCESETTINGS_ROLL_MAX]);

				if (positionActual.Groundspeed>0) {

					stabDesired.Yaw = RAD2DEG * sinf(
						( stabDesired.Roll - ccguidanceSettings.Roll[CCGUIDANCESETTINGS_ROLL_NEUTRAL]
						) / RAD2DEG
						) * GEE / positionActual.Groundspeed;
				} else {
					stabDesired.Yaw = 0;
				}

			} else {
				/* Fallback, no position data! */
				stabDesired.Yaw = 0;
				stabDesired.Pitch = ccguidanceSettings.Pitch[CCGUIDANCESETTINGS_PITCH_NEUTRAL];
				stabDesired.Roll = ccguidanceSettings.Roll[CCGUIDANCESETTINGS_ROLL_MAX];
			}

			/* 3. Throttle (manual) */

			stabDesired.Throttle = manualControl.Throttle;
			stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
			stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
			stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
			
			StabilizationDesiredSet(&stabDesired);


		} else {
			// reset globals...
			positionHoldLast = 0;
		}
		
	}
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
