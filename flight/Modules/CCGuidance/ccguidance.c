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
//#include "positiondesired.h"	// object that will be updated by the module
#include "manualcontrol.h"
#include "manualcontrolcommand.h"
#include "flightstatus.h"
//#include "homelocation.h"
#include "stabilizationdesired.h"
#include "systemsettings.h"
#include "attitudeactual.h"

// Private constants
#define MAX_QUEUE_SIZE 1
#define STACK_SIZE_BYTES 500
#define TASK_PRIORITY (tskIDLE_PRIORITY+2)
#define RAD2DEG (180.0/M_PI)
#define DEG2RAD (M_PI/180.0)
#define GEE 9.81
// Private types

// Private variables
static uint8_t positionHoldLast = 0;
float DistanceToTarget = 0;

// Private functions
static void ccguidanceTask(UAVObjEvent * ev);
//static float bound(float val, float min, float max);
static float sphereDistance(float lat1,float long1,float lat2,float long2);
static float sphereCourse(float lat1,float long1,float lat2,float long2);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t CCGuidanceStart()
{
	// indicate error - can only run correctly if GPSPosition is ever set, which will change the alarm state.
	AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_ERROR);

	return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t CCGuidanceInitialize()
{
	CCGuidanceSettingsInitialize();
	GPSPositionInitialize();
	//HomeLocationInitialize();

	// connect to GPSPosition
	GPSPositionConnectCallback(&ccguidanceTask);
	
	return 0;
}
// no macro - optional module
MODULE_INITCALL(CCGuidanceInitialize, CCGuidanceStart)

/**
 * Main module callback.
 */
static void ccguidanceTask(UAVObjEvent * ev)
{
	SystemSettingsData systemSettings;
	CCGuidanceSettingsData ccguidanceSettings;
	ManualControlCommandData manualControl;
	FlightStatusData flightStatus;
	//HomeLocationData homeLocation;

	portTickType thisTime;

	static portTickType lastUpdateTime = 0;
	if (!lastUpdateTime) lastUpdateTime = xTaskGetTickCount();

	float courseTrue, courseRelative;
	//float actHeading = 0;
	static float diffHeadingYaw/*, oldHeading*/;
	AttitudeActualData attitudeActual;

	CCGuidanceSettingsGet(&ccguidanceSettings);

	// Continue collecting data if not enough time
	thisTime = xTaskGetTickCount();
	if( (thisTime - lastUpdateTime) < (ccguidanceSettings.UpdatePeriod / portTICK_RATE_MS) )
		return;

	// dT = (thisTime - lastUpdateTime) / portTICK_RATE_MS / 1000.0f;

	lastUpdateTime = thisTime;
	
	ManualControlCommandGet(&manualControl);
	FlightStatusGet(&flightStatus);
	SystemSettingsGet(&systemSettings);
	//HomeLocationGet(&homeLocation);
	
	//Activate failsave mode, activate return to base
//	if (AlarmsGet(SYSTEMALARMS_ALARM_MANUALCONTROL) != SYSTEMALARMS_ALARM_OK) {
//		flightStatus.FlightMode = FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE;
//	}
	
	if ((PARSE_FLIGHT_MODE(flightStatus.FlightMode) == FLIGHTMODE_GUIDANCE) &&
		((systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWING) ||
		 (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGELEVON) ||
		 (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGVTAIL) ))
	{
		StabilizationDesiredData stabDesired;
		StabilizationDesiredGet(&stabDesired);
		static int positionDesiredNorth, positionDesiredEast, positionDesiredDown = 0;
		
		GPSPositionData positionActual;
		GPSPositionGet(&positionActual);

		if(positionHoldLast == 0 && (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ||
		  (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE )))//&& homeLocation.Set == HOMELOCATION_SET_FALSE)))
		  {
			/* When enter position hold mode save current position */
			positionDesiredNorth = positionActual.Latitude;
			positionDesiredEast = positionActual.Longitude;
			positionDesiredDown = positionActual.Altitude + positionActual.GeoidSeparation + 1;
			//PositionDesiredSet(&positionDesired);
			positionHoldLast = 1;
		} else if (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE) {// && homeLocation.Set == HOMELOCATION_SET_TRUE) {
			/* When we RTB, safe home position */
			positionHoldLast = 0;
			positionDesiredNorth = 23; //homeLocation.Latitude;
			positionDesiredEast = 24; //homeLocation.Longitude;
			positionDesiredDown = 25; //homeLocation.Altitude + ccguidanceSettings.ReturnTobaseAltitudeOffset ;
			
			}

		stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
		stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
		stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;

		/* safety */
		if (positionActual.Status==GPSPOSITION_STATUS_FIX3D) {
			/* main position hold loop */
			/* 1. Altitude */

			if ((positionActual.Altitude + positionActual.GeoidSeparation) < positionDesiredDown) {
				stabDesired.Pitch = ccguidanceSettings.Pitch[CCGUIDANCESETTINGS_PITCH_CLIMB];
			} else {
				stabDesired.Pitch = ccguidanceSettings.Pitch[CCGUIDANCESETTINGS_PITCH_SINK];
			}

			/* 2. Heading */
			
			// calculate course to target
			courseTrue = sphereCourse(
				positionActual.Latitude * 1e-7,
				positionActual.Longitude * 1e-7,
				positionDesiredNorth * 1e-7,
				positionDesiredEast * 1e-7
				);

			if (positionActual.Groundspeed > 2 ) {
				/*if (positionActual.Heading<-180.) actHeading = positionActual.Heading + 360.;
				if (positionActual.Heading>180.)  actHeading = positionActual.Heading - 360.;
				if (abs(oldHeading - actHeading) < 10)*/ {
					AttitudeActualGet(&attitudeActual);
					diffHeadingYaw = attitudeActual.Yaw - positionActual.Heading;
					while (diffHeadingYaw<-180.) diffHeadingYaw+=360.;
					while (diffHeadingYaw>180.)  diffHeadingYaw-=360.;
				}
				//oldHeading = actHeading;
			} //else oldHeading = 400;	//any number > 0-360 degree
			
			if ( abs(DistanceToTarget) > 8) {
				courseRelative = courseTrue + diffHeadingYaw;
				while (courseRelative<-180.) courseRelative+=360.;
				while (courseRelative>180.)  courseRelative-=360.;
				stabDesired.Yaw = courseRelative;			
			} /*else {
				stabDesired.Yaw = 0;
				stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
			  }*/
			
			AlarmsClear(SYSTEMALARMS_ALARM_GUIDANCE);

		} else {
			/* Fallback, no position data! */
			stabDesired.Yaw = 0;
			stabDesired.Pitch = ccguidanceSettings.Pitch[CCGUIDANCESETTINGS_PITCH_CLIMB];
			stabDesired.Roll = ccguidanceSettings.Roll[CCGUIDANCESETTINGS_ROLL_MAX];
			stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
			AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_WARNING);
		}

		/* 3. Throttle (manual) */
		stabDesired.Throttle = manualControl.Throttle;
		
		StabilizationDesiredSet(&stabDesired);

	} else{
		// reset globals...
		positionHoldLast = 0;
		AlarmsClear(SYSTEMALARMS_ALARM_GUIDANCE);
	}
	
}


/**
 * Bound input value between limits
 */
/*
static float bound(float val, float min, float max)
{
	if (val < min) {
		val = min;
	} else if (val > max) {
		val = max;
	}
	return val;
}
*/

/**
 * calculate spherical distance and course between two coordinate pairs
 * see http://de.wikipedia.org/wiki/Orthodrome for details
 */
 static float sphereDistance(float lat1, float long1, float lat2, float long2)
{
	float zeta=(RAD2DEG * acos (
		sin(DEG2RAD*lat1) * sin(DEG2RAD*lat2)
		+ cos(DEG2RAD*lat1) * cos(DEG2RAD*lat2) * cos(DEG2RAD*(long2-long1))
	));
	if (isnan(zeta)) {
		zeta=0;
	}
	return zeta;

}
static float sphereCourse(float lat1, float long1, float lat2, float long2)
{
	float zeta = sphereDistance(lat1,long1,lat2,long2);
	DistanceToTarget = 111111 * zeta;
	float angle = RAD2DEG * acos(
			( sin(DEG2RAD*lat2) - sin(DEG2RAD*lat1) * cos(DEG2RAD*zeta) )
			/ ( cos(DEG2RAD*lat1) * sin(DEG2RAD*zeta) )
		);
	if (isnan(angle)) angle=0;
	float diff=long2-long1;
	if (diff>180) diff-=360;
	if (diff<-180) diff+=360;
	if (diff>=0) {
		return angle;
	} else {
		return -angle;
	}
}

