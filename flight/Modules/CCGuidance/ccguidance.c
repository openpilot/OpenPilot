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
#include "hwsettings.h"

// Private constants
#define MAX_QUEUE_SIZE 1
#define STACK_SIZE_BYTES 500
#define TASK_PRIORITY (tskIDLE_PRIORITY+2)
#define RAD2DEG (180.0/M_PI)
#define DEG2RAD (M_PI/180.0)
#define GEE 9.81
#define SAMPLE_PERIOD_MS		1050
// Private types

// Private variables
static uint8_t positionHoldLast;

// Private functions
static void ccguidanceTask(UAVObjEvent * ev);
//static float bound(float val, float min, float max);
static float sphereDistance(float lat1,float long1,float lat2,float long2);
static float sphereCourse(float lat1,float long1,float lat2,float long2, float zeta);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t CCGuidanceStart()
{
	return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t CCGuidanceInitialize()
{
	static UAVObjEvent ev;

	bool CCGuidanceEnabled;
	uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];

	HwSettingsInitialize();
	HwSettingsOptionalModulesGet(optionalModules);

	if (optionalModules[HWSETTINGS_OPTIONALMODULES_CCGUIDANCE] == HWSETTINGS_OPTIONALMODULES_ENABLED)
		CCGuidanceEnabled = true;
	else
		CCGuidanceEnabled = false;

	if (CCGuidanceEnabled) {

		CCGuidanceSettingsInitialize();
		GPSPositionInitialize();
		//HomeLocationInitialize();

		memset(&ev,0,sizeof(UAVObjEvent));	
		EventPeriodicCallbackCreate(&ev, ccguidanceTask, SAMPLE_PERIOD_MS / portTICK_RATE_MS);

		// connect to GPSPosition
		//GPSPositionConnectCallback(&ccguidanceTask);

		// indicate error - can only run correctly if GPSPosition is ever set, which will change the alarm state.
		AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_ERROR);
		return 0;
	}
	return -1;	
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
	AttitudeActualData attitudeActual;
	portTickType thisTime;

	static portTickType lastUpdateTime = 0;
	float courseTrue = 0;
	static float diffHeadingYaw, courseRelative;
	static float positionDesiredNorth = 0, positionDesiredEast = 0, positionDesiredDown = 0;
	float positionActualNorth = 0, positionActualEast = 0, DistanceToBase = 0;
	static uint16_t thisTimesRotateToCourse;
	static bool	firsRunSetCourse = TRUE, StateSaveCurrentPositionToRTB = FALSE;

	CCGuidanceSettingsGet(&ccguidanceSettings);

	if (!lastUpdateTime) lastUpdateTime = xTaskGetTickCount();

	// Continue collecting data if not enough time
	thisTime = xTaskGetTickCount();
	if( (thisTime - lastUpdateTime) < (ccguidanceSettings.UpdatePeriod / portTICK_RATE_MS) ) return;

	thisTimesRotateToCourse += (thisTime - lastUpdateTime) / portTICK_RATE_MS;
	lastUpdateTime = thisTime;

	FlightStatusGet(&flightStatus);
	SystemSettingsGet(&systemSettings);

	//Activate failsave mode, activate return to base
	if ((AlarmsGet(SYSTEMALARMS_ALARM_MANUALCONTROL) != SYSTEMALARMS_ALARM_OK) &&
		(ccguidanceSettings.FaileSaveRTB == TRUE))	{
		flightStatus.FlightMode = FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE;
		FlightStatusSet(&flightStatus);
	}

	//Checking mode is enabled Return to Base
	if ((PARSE_FLIGHT_MODE(flightStatus.FlightMode) == FLIGHTMODE_GUIDANCE) &&
		((systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWING) ||
		 (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGELEVON) ||
		 (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGVTAIL) ))
	{
		GPSPositionData positionActual;
		GPSPositionGet(&positionActual);

		if(positionHoldLast != 1 && (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ||
		  (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE && ccguidanceSettings.HomeLocationSet == FALSE)))
		  {
			/* When enter position hold mode save current position */
			positionDesiredNorth = positionActual.Latitude * 1e-7;
			positionDesiredEast = positionActual.Longitude * 1e-7;
			positionDesiredDown = positionActual.Altitude + positionActual.GeoidSeparation + 1;
			positionHoldLast = 1;
		} else if (positionHoldLast != 2 && (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE && ccguidanceSettings.HomeLocationSet == TRUE)) {
			/* When we RTB, safe home position */
			positionDesiredNorth = ccguidanceSettings.HomeLocationLatitude * 1e-7;
			positionDesiredEast = ccguidanceSettings.HomeLocationLongitude * 1e-7;
			positionDesiredDown = ccguidanceSettings.HomeLocationAltitude + ccguidanceSettings.ReturnTobaseAltitudeOffset ;
			positionHoldLast = 2;
			}

		StabilizationDesiredData stabDesired;
		StabilizationDesiredGet(&stabDesired);

		/* safety */
		if (positionActual.Status==GPSPOSITION_STATUS_FIX3D) {
			/* main position hold loop */

			// Calculation of the rate after the turn and the beginning of motion in a straight line.
			if (thisTimesRotateToCourse >= ccguidanceSettings.TimesRotateToCourse) {
				/* 1. Calculate course */
				positionActualNorth = positionActual.Latitude * 1e-7;
				positionActualEast  = positionActual.Longitude * 1e-7;
				// Calculation errors between the rate of the gyroscope and GPS at a speed not less than the minimum.
				if (positionActual.Groundspeed > ccguidanceSettings.GroundSpeedCalcCorrectHeadMin ) {
					AttitudeActualGet(&attitudeActual);
					diffHeadingYaw = attitudeActual.Yaw - positionActual.Heading;
					while (diffHeadingYaw<-180.) diffHeadingYaw+=360.;
					while (diffHeadingYaw>180.)  diffHeadingYaw-=360.;
					}

				// calculate course to target
				DistanceToBase = sphereDistance(
					positionActualNorth,
					positionActualEast,
					positionDesiredNorth,
					positionDesiredEast
					);
				// If the distance to the base less than this, then do not expect a new course.
				if ( abs(DistanceToBase * 111111) > ccguidanceSettings.RadiusBase) {
					courseTrue = sphereCourse(
						positionActualNorth,
						positionActualEast,
						positionDesiredNorth,
						positionDesiredEast,
						DistanceToBase
					);
					courseRelative = courseTrue + diffHeadingYaw;
					while (courseRelative<-180.) courseRelative+=360.;
					while (courseRelative>180.)  courseRelative-=360.;
				}
				stabDesired.Yaw = courseRelative;
				thisTimesRotateToCourse = 0;
			} else {
				// If first run to activate fixed direction Yaw
				if (firsRunSetCourse) {
					// Save current location to HomeLocation if flightStatus = DISARMED
					if (flightStatus.Armed == FLIGHTSTATUS_ARMED_DISARMED && StateSaveCurrentPositionToRTB == FALSE) {
						ccguidanceSettings.HomeLocationLatitude	= positionActual.Latitude;
						ccguidanceSettings.HomeLocationLongitude = positionActual.Longitude;
						ccguidanceSettings.HomeLocationAltitude = positionActual.Altitude;
						ccguidanceSettings.HomeLocationSet = TRUE;
						CCGuidanceSettingsSet(&ccguidanceSettings);
						positionHoldLast = 0;
						StateSaveCurrentPositionToRTB = TRUE;
						if (stabDesired.Throttle < 0) stabDesired.Throttle = 0;
					}

					stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
					stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
					stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;

					AttitudeActualGet(&attitudeActual);
					stabDesired.Yaw = attitudeActual.Yaw;
					firsRunSetCourse = FALSE;
				}
				/* 2. Altitude */
				if ((positionActual.Altitude + positionActual.GeoidSeparation) < positionDesiredDown) {
					stabDesired.Pitch = ccguidanceSettings.Pitch[CCGUIDANCESETTINGS_PITCH_CLIMB];
				} else {
					stabDesired.Pitch = ccguidanceSettings.Pitch[CCGUIDANCESETTINGS_PITCH_SINK];
				}

				/* 3. GroundSpeed */
				if (ccguidanceSettings.TrottleAutoSet == TRUE) {
					if (ccguidanceSettings.TrottleMax > 1) ccguidanceSettings.TrottleMax = 1;
					if (ccguidanceSettings.TrottleMin < 0) ccguidanceSettings.TrottleMin = 0;

					if (positionActual.Groundspeed < ccguidanceSettings.GroundSpeedMax ) {
						stabDesired.Throttle += 0.1;
						if (stabDesired.Throttle > ccguidanceSettings.TrottleMax) stabDesired.Throttle = ccguidanceSettings.TrottleMax;
					} else {
						stabDesired.Throttle -= 0.1;
						if (stabDesired.Throttle < ccguidanceSettings.TrottleMin) stabDesired.Throttle = ccguidanceSettings.TrottleMin;
					}
				} else {
					/* Throttle (manual) */
					ManualControlCommandGet(&manualControl);
					stabDesired.Throttle = manualControl.Throttle;
				}
			}
			stabDesired.Roll = ccguidanceSettings.Roll[CCGUIDANCESETTINGS_ROLL_NEUTRAL];
			AlarmsClear(SYSTEMALARMS_ALARM_GUIDANCE);

		} else {
			/* Fallback, no position data! */
			stabDesired.Yaw = ccguidanceSettings.Yaw;
			stabDesired.Pitch = ccguidanceSettings.Pitch[CCGUIDANCESETTINGS_PITCH_CLIMB];
			stabDesired.Roll = ccguidanceSettings.Roll[CCGUIDANCESETTINGS_ROLL_MAX];
			stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
			stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
			stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
			/* Throttle (manual) */
			ManualControlCommandGet(&manualControl);
			stabDesired.Throttle = manualControl.Throttle;
			AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_WARNING);
		}

		StabilizationDesiredSet(&stabDesired);
	} else {
		// reset globals...
		thisTimesRotateToCourse = 0;
		positionHoldLast = 0;
		firsRunSetCourse = TRUE;
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
static float sphereCourse(float lat1, float long1, float lat2, float long2, float zeta)
{
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

