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
 * Input object: PositionActual
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
#include "paths.h"

#include "accels.h"
#include "hwsettings.h"
#include "attitudeactual.h"
#include "pathdesired.h"	// object that will be updated by the module
#include "positionactual.h"
#include "manualcontrol.h"
#include "flightstatus.h"
#include "baroairspeed.h"
#include "gpsvelocity.h"
#include "gpsposition.h"
#include "fixedwingpathfollowersettings.h"
#include "fixedwingpathfollowerstatus.h"
#include "homelocation.h"
#include "nedposition.h"
#include "stabilizationdesired.h"
#include "stabilizationsettings.h"
#include "systemsettings.h"
#include "velocitydesired.h"
#include "velocityactual.h"
#include "CoordinateConversions.h"

// Private constants
#define MAX_QUEUE_SIZE 4
#define STACK_SIZE_BYTES 1548
#define TASK_PRIORITY (tskIDLE_PRIORITY+2)
#define F_PI 3.14159265358979323846f
#define RAD2DEG (180.0f/F_PI)
#define DEG2RAD (F_PI/180.0f)
#define GEE 9.81f
// Private types

// Private variables
static bool followerEnabled = false;
static xTaskHandle pathfollowerTaskHandle;
static PathDesiredData pathDesired;
static FixedWingPathFollowerSettingsData fixedwingpathfollowerSettings;

// Private functions
static void pathfollowerTask(void *parameters);
static void SettingsUpdatedCb(UAVObjEvent * ev);
static void updatePathVelocity();
static uint8_t updateFixedDesiredAttitude();
static void baroAirspeedUpdatedCb(UAVObjEvent * ev);
static float bound(float val, float min, float max);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t FixedWingPathFollowerStart()
{
	if (followerEnabled) {
		// Start main task
		xTaskCreate(pathfollowerTask, (signed char *)"PathFollower", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &pathfollowerTaskHandle);
		TaskMonitorAdd(TASKINFO_RUNNING_PATHFOLLOWER, pathfollowerTaskHandle);
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
		VelocityDesiredInitialize();
		BaroAirspeedInitialize();
	} else {
		followerEnabled = false;
	}
	return 0;
}
MODULE_INITCALL(FixedWingPathFollowerInitialize, FixedWingPathFollowerStart)

static float northVelIntegral = 0;
static float eastVelIntegral = 0;
static float downVelIntegral = 0;

static float courseIntegral = 0;
static float speedIntegral = 0;
static float accelIntegral = 0;
static float powerIntegral = 0;

// correct speed by measured airspeed
static float baroAirspeedBias = 0;

/**
 * Module thread, should not return.
 */
static void pathfollowerTask(void *parameters)
{
	SystemSettingsData systemSettings;
	FlightStatusData flightStatus;
	
	portTickType lastUpdateTime;
	
	BaroAirspeedConnectCallback(baroAirspeedUpdatedCb);
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
		//    FlightMode is PathPlanner and PathDesired.Mode is Endpoint or Path

		SystemSettingsGet(&systemSettings);
		if ( (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWING) &&
			(systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGELEVON) &&
			(systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGVTAIL) )
		{
			AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_WARNING);
			vTaskDelay(1000);
			continue;
		}

		// Continue collecting data if not enough time
		vTaskDelayUntil(&lastUpdateTime, fixedwingpathfollowerSettings.UpdatePeriod / portTICK_RATE_MS);

		FlightStatusGet(&flightStatus);
	
		uint8_t result;
		// Check the combinations of flightmode and pathdesired mode
		switch(flightStatus.FlightMode) {
			case FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD:
			case FLIGHTSTATUS_FLIGHTMODE_RTH:
				if (pathDesired.Mode == PATHDESIRED_MODE_ENDPOINT) {
					updatePathVelocity();
					result = updateFixedDesiredAttitude();
					if (result) {
						AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_OK);
					} else {
						AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_WARNING);
					}
				} else {
					AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_ERROR);
				}
				break;
			case FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER:
				switch(pathDesired.Mode) {
					case PATHDESIRED_MODE_PATH:
					case PATHDESIRED_MODE_ENDPOINT:
						updatePathVelocity();
						result = updateFixedDesiredAttitude();
						if (result) {
							AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_OK);
						} else {
							AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_WARNING);
						}
						break;
					default:
						AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_ERROR);
						break;
				}
				break;
			default:
				// Be cleaner and get rid of global variables
				northVelIntegral = 0;
				eastVelIntegral = 0;
				downVelIntegral = 0;
				courseIntegral = 0;
				speedIntegral = 0;
				accelIntegral = 0;
				powerIntegral = 0;

				break;
		}
	}
}

/**
 * Compute desired velocity from the current position and path
 *
 * Takes in @ref PositionActual and compares it to @ref PathDesired 
 * and computes @ref VelocityDesired
 */
static void updatePathVelocity()
{
	PositionActualData positionActual;
	PositionActualGet(&positionActual);
	VelocityActualData velocityActual;
	VelocityActualGet(&velocityActual);

	// look ahead fixedwingpathfollowerSettings.CourseFeedForward seconds
	float cur[3] = {positionActual.North + (velocityActual.North * fixedwingpathfollowerSettings.CourseFeedForward),
					positionActual.East + (velocityActual.East * fixedwingpathfollowerSettings.CourseFeedForward),
					positionActual.Down + (velocityActual.Down * fixedwingpathfollowerSettings.CourseFeedForward)
					};
	struct path_status progress;

	path_progress(pathDesired.Start, pathDesired.End, cur, &progress);
	
	float groundspeed;
	float altitudeSetpoint;
	switch (pathDesired.Mode) {
		case PATHDESIRED_MODE_PATH:
		case PATHDESIRED_MODE_ENDPOINT:
		default:
			groundspeed = pathDesired.StartingVelocity + (pathDesired.EndingVelocity - pathDesired.StartingVelocity) *
			bound(progress.fractional_progress,0,1);
			altitudeSetpoint = pathDesired.Start[2] + (pathDesired.End[2] - pathDesired.Start[2]) *
			bound(progress.fractional_progress,0,1);
			break;
	}
	
	// calculate velocity - can be zero if waypoints are too close
	VelocityDesiredData velocityDesired;
	velocityDesired.North = progress.path_direction[0] * bound(groundspeed,1e-6,groundspeed);
	velocityDesired.East = progress.path_direction[1] * bound(groundspeed,1e-6,groundspeed);
	
	float error_speed = progress.error * fixedwingpathfollowerSettings.HorizontalPosP;

	// calculate correction - can also be zero if correction vector is 0 or no error present
	velocityDesired.North += progress.correction_direction[0] * error_speed;
	velocityDesired.East += progress.correction_direction[1] * error_speed;
	
	float downError = altitudeSetpoint - positionActual.Down;
	velocityDesired.Down = downError * fixedwingpathfollowerSettings.VerticalPosP;

	VelocityDesiredSet(&velocityDesired);
}

/**
 * Compute desired attitude from the desired velocity
 *
 * Takes in @ref NedActual which has the acceleration in the 
 * NED frame as the feedback term and then compares the 
 * @ref VelocityActual against the @ref VelocityDesired
 */
static uint8_t updateFixedDesiredAttitude()
{

	uint8_t result = 1;

	float dT = fixedwingpathfollowerSettings.UpdatePeriod / 1000.0f;

	VelocityDesiredData velocityDesired;
	VelocityActualData velocityActual;
	StabilizationDesiredData stabDesired;
	AttitudeActualData attitudeActual;
	AccelsData accels;
	FixedWingPathFollowerSettingsData fixedwingpathfollowerSettings;
	StabilizationSettingsData stabSettings;
	FixedWingPathFollowerStatusData fixedwingpathfollowerStatus;

	float groundspeedActual;
	float groundspeedDesired;
	float airspeedActual;
	float airspeedDesired;
	float speedError;
	float accelActual;
	float accelDesired;
	float accelError;
	float accelCommand;
	
	float pitchCommand;

	float climbspeedDesired;
	float climbspeedError;
	float powerError;
	float powerCommand;

	float courseError;
	float courseCommand;

	FixedWingPathFollowerSettingsGet(&fixedwingpathfollowerSettings);

	FixedWingPathFollowerStatusGet(&fixedwingpathfollowerStatus);
	
	VelocityActualGet(&velocityActual);
	VelocityDesiredGet(&velocityDesired);
	StabilizationDesiredGet(&stabDesired);
	VelocityDesiredGet(&velocityDesired);
	AttitudeActualGet(&attitudeActual);
	AccelsGet(&accels);
	StabilizationSettingsGet(&stabSettings);


	/**
	 * Compute speed error (required for throttle and pitch)
	 */

	// Current ground speed
	groundspeedActual  = sqrtf( velocityActual.East*velocityActual.East + velocityActual.North*velocityActual.North );
	airspeedActual     = groundspeedActual + baroAirspeedBias;

	// Desired ground speed
	groundspeedDesired = sqrtf(velocityDesired.North*velocityDesired.North + velocityDesired.East*velocityDesired.East);
	airspeedDesired    = bound( groundspeedDesired + baroAirspeedBias,
							fixedwingpathfollowerSettings.AirSpeedMin,
							fixedwingpathfollowerSettings.AirSpeedMax);

	// Airspeed error
	speedError = airspeedDesired - ( airspeedActual );
	// Vertical error
	climbspeedDesired = bound (
						velocityDesired.Down,
						-fixedwingpathfollowerSettings.VerticalVelMax,
						fixedwingpathfollowerSettings.VerticalVelMax);
	climbspeedError = climbspeedDesired - velocityActual.Down;

	// Error condition: wind speed is higher than maximum allowed speed. We are forced backwards!
	fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_WIND] = 0;
	if (groundspeedDesired - baroAirspeedBias <= 0 ) {
		fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_WIND] = 1;
		result = 0;
	}
	// Error condition: plane too slow or too fast
	fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_HIGHSPEED] = 0;
	fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_LOWSPEED] = 0;
	if ( airspeedActual >  fixedwingpathfollowerSettings.AirSpeedMax * 1.2f) {
		fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_HIGHSPEED] = 1;
		result = 0;
	}
	if (airspeedActual < fixedwingpathfollowerSettings.AirSpeedMin * 0.8f) {
		fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_LOWSPEED] = 1;
		result = 0;
	}

	if (airspeedActual<1e-6) {
		// prevent division by zero, abort without controlling anything. This guidance mode is not suited for takeoff or touchdown, or handling stationary planes
		// also we cannot handle planes flying backwards, lets just wait until the nose drops
		fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_LOWSPEED] = 1;
		return 0;
	}

	/**
	 * Compute desired throttle command
	 */
	// compute desired power
	powerError = -climbspeedError +
				bound (
					(speedError/airspeedActual) * fixedwingpathfollowerSettings.AirspeedToVerticalCrossFeed[FIXEDWINGPATHFOLLOWERSETTINGS_AIRSPEEDTOVERTICALCROSSFEED_KP],
					-fixedwingpathfollowerSettings.AirspeedToVerticalCrossFeed[FIXEDWINGPATHFOLLOWERSETTINGS_AIRSPEEDTOVERTICALCROSSFEED_MAX],
					fixedwingpathfollowerSettings.AirspeedToVerticalCrossFeed[FIXEDWINGPATHFOLLOWERSETTINGS_AIRSPEEDTOVERTICALCROSSFEED_MAX]
				);
	powerIntegral =	bound(powerIntegral + powerError * dT * fixedwingpathfollowerSettings.PowerPI[FIXEDWINGPATHFOLLOWERSETTINGS_POWERPI_KI], 
		-fixedwingpathfollowerSettings.PowerPI[FIXEDWINGPATHFOLLOWERSETTINGS_POWERPI_ILIMIT],
		fixedwingpathfollowerSettings.PowerPI[FIXEDWINGPATHFOLLOWERSETTINGS_POWERPI_ILIMIT]);
	powerCommand = (powerError * fixedwingpathfollowerSettings.PowerPI[FIXEDWINGPATHFOLLOWERSETTINGS_POWERPI_KP] +
		powerIntegral) + fixedwingpathfollowerSettings.ThrottleLimit[FIXEDWINGPATHFOLLOWERSETTINGS_THROTTLELIMIT_NEUTRAL];

	// prevent integral running out of bounds 
	if ( powerCommand > fixedwingpathfollowerSettings.ThrottleLimit[FIXEDWINGPATHFOLLOWERSETTINGS_THROTTLELIMIT_MAX]) {
		powerIntegral = bound(
			powerIntegral -
				( powerCommand 
				- fixedwingpathfollowerSettings.ThrottleLimit[FIXEDWINGPATHFOLLOWERSETTINGS_THROTTLELIMIT_MAX]),
			-fixedwingpathfollowerSettings.PowerPI[FIXEDWINGPATHFOLLOWERSETTINGS_POWERPI_ILIMIT],
			fixedwingpathfollowerSettings.PowerPI[FIXEDWINGPATHFOLLOWERSETTINGS_POWERPI_ILIMIT]);
		powerCommand = fixedwingpathfollowerSettings.ThrottleLimit[FIXEDWINGPATHFOLLOWERSETTINGS_THROTTLELIMIT_MAX];
	}
	if ( powerCommand < fixedwingpathfollowerSettings.ThrottleLimit[FIXEDWINGPATHFOLLOWERSETTINGS_THROTTLELIMIT_MIN]) {
		powerIntegral = bound(
			powerIntegral -
				( powerCommand
				- fixedwingpathfollowerSettings.ThrottleLimit[FIXEDWINGPATHFOLLOWERSETTINGS_THROTTLELIMIT_MIN]),
			-fixedwingpathfollowerSettings.PowerPI[FIXEDWINGPATHFOLLOWERSETTINGS_POWERPI_ILIMIT],
			fixedwingpathfollowerSettings.PowerPI[FIXEDWINGPATHFOLLOWERSETTINGS_POWERPI_ILIMIT]);
		powerCommand = fixedwingpathfollowerSettings.ThrottleLimit[FIXEDWINGPATHFOLLOWERSETTINGS_THROTTLELIMIT_MIN];
	}

	fixedwingpathfollowerStatus.E[FIXEDWINGPATHFOLLOWERSTATUS_E_POWER] = powerError;
	fixedwingpathfollowerStatus.A[FIXEDWINGPATHFOLLOWERSTATUS_A_POWER] = powerIntegral;
	fixedwingpathfollowerStatus.C[FIXEDWINGPATHFOLLOWERSTATUS_C_POWER] = powerCommand;

	// set throttle
	stabDesired.Throttle = powerCommand;

	// Error condition: plane cannot hold altitude at current speed.
	fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_LOWPOWER] = 0;
	if (
		powerCommand == fixedwingpathfollowerSettings.ThrottleLimit[FIXEDWINGPATHFOLLOWERSETTINGS_THROTTLELIMIT_MAX] // throttle at maximum
		&& velocityActual.Down > 0 // we ARE going down
		&& climbspeedDesired < 0 // we WANT to go up
		&& speedError > 0 // we are too slow already
		) {
		fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_LOWPOWER] = 1;
		result = 0;
	}
	// Error condition: plane keeps climbing despite minimum throttle (opposite of above)
	fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_HIGHPOWER] = 0;
	if (
		powerCommand == fixedwingpathfollowerSettings.ThrottleLimit[FIXEDWINGPATHFOLLOWERSETTINGS_THROTTLELIMIT_MIN] // throttle at minimum
		&& velocityActual.Down < 0 // we ARE going up
		&& climbspeedDesired > 0 // we WANT to go down
		&& speedError < 0 // we are too fast already
		) {
		fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_HIGHPOWER] = 1;
		result = 0;
	}


	/**
	 * Compute desired pitch command
	 */
	// compute desired acceleration
	accelDesired = bound( (speedError/airspeedActual) * fixedwingpathfollowerSettings.SpeedP[FIXEDWINGPATHFOLLOWERSETTINGS_SPEEDP_KP],
		-fixedwingpathfollowerSettings.SpeedP[FIXEDWINGPATHFOLLOWERSETTINGS_SPEEDP_MAX],
		fixedwingpathfollowerSettings.SpeedP[FIXEDWINGPATHFOLLOWERSETTINGS_SPEEDP_MAX]);
	
	fixedwingpathfollowerStatus.E[FIXEDWINGPATHFOLLOWERSTATUS_E_SPEED] = (speedError/airspeedActual);
	fixedwingpathfollowerStatus.A[FIXEDWINGPATHFOLLOWERSTATUS_A_SPEED] = 0.0f;
	fixedwingpathfollowerStatus.C[FIXEDWINGPATHFOLLOWERSTATUS_C_SPEED] = accelDesired;

	// exclude gravity from acceleration. If the aicraft is flying at high pitch, it fights gravity without getting faster
	accelActual = accels.x - (sinf( DEG2RAD * attitudeActual.Pitch) * GEE);

	accelError = accelDesired - accelActual;
	accelIntegral = bound(accelIntegral + accelError * dT * fixedwingpathfollowerSettings.AccelPI[FIXEDWINGPATHFOLLOWERSETTINGS_ACCELPI_KI], 
		-fixedwingpathfollowerSettings.AccelPI[FIXEDWINGPATHFOLLOWERSETTINGS_ACCELPI_ILIMIT],
		fixedwingpathfollowerSettings.AccelPI[FIXEDWINGPATHFOLLOWERSETTINGS_ACCELPI_ILIMIT]);
	accelCommand = (accelError * fixedwingpathfollowerSettings.AccelPI[FIXEDWINGPATHFOLLOWERSETTINGS_ACCELPI_KP] + 
		 accelIntegral);
	
	fixedwingpathfollowerStatus.E[FIXEDWINGPATHFOLLOWERSTATUS_E_ACCEL] = accelError;
	fixedwingpathfollowerStatus.A[FIXEDWINGPATHFOLLOWERSTATUS_A_ACCEL] = accelIntegral;
	fixedwingpathfollowerStatus.C[FIXEDWINGPATHFOLLOWERSTATUS_C_ACCEL] = accelCommand;

	// compute desired pitch
	pitchCommand= -accelCommand + bound ( (-climbspeedError/airspeedActual) * fixedwingpathfollowerSettings.VerticalToPitchCrossFeed[FIXEDWINGPATHFOLLOWERSETTINGS_VERTICALTOPITCHCROSSFEED_KP],
					-fixedwingpathfollowerSettings.VerticalToPitchCrossFeed[FIXEDWINGPATHFOLLOWERSETTINGS_VERTICALTOPITCHCROSSFEED_MAX],
					fixedwingpathfollowerSettings.VerticalToPitchCrossFeed[FIXEDWINGPATHFOLLOWERSETTINGS_VERTICALTOPITCHCROSSFEED_MAX]
					);
	stabDesired.Pitch = bound(fixedwingpathfollowerSettings.PitchLimit[FIXEDWINGPATHFOLLOWERSETTINGS_PITCHLIMIT_NEUTRAL] +
		pitchCommand,
		fixedwingpathfollowerSettings.PitchLimit[FIXEDWINGPATHFOLLOWERSETTINGS_PITCHLIMIT_MIN],
		fixedwingpathfollowerSettings.PitchLimit[FIXEDWINGPATHFOLLOWERSETTINGS_PITCHLIMIT_MAX]);

	// Error condition: high speed dive
	fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_PITCHCONTROL] = 0;
	if (
		pitchCommand == fixedwingpathfollowerSettings.PitchLimit[FIXEDWINGPATHFOLLOWERSETTINGS_PITCHLIMIT_MAX] // pitch demand is full up
		&& velocityActual.Down > 0 // we ARE going down
		&& climbspeedDesired < 0 // we WANT to go up
		&& speedError < 0 // we are too fast already
		) {
		fixedwingpathfollowerStatus.Errors[FIXEDWINGPATHFOLLOWERSTATUS_ERRORS_PITCHCONTROL] = 1;
		result = 0;
	}


	/**
	 * Compute desired roll command
	 */
	if (groundspeedDesired> 1e-6) {
		courseError = RAD2DEG * (atan2f(velocityDesired.East,velocityDesired.North) - atan2f(velocityActual.East,velocityActual.North));
	} else {
		// if we are not supposed to move, keep going wherever we are now. Don't make things worse by changing direction.
		courseError = 0;
	}
	
	if (courseError<-180.0f) courseError+=360.0f;
	if (courseError>180.0f) courseError-=360.0f;

	courseIntegral = bound(courseIntegral + courseError * dT * fixedwingpathfollowerSettings.CoursePI[FIXEDWINGPATHFOLLOWERSETTINGS_COURSEPI_KI], 
		-fixedwingpathfollowerSettings.CoursePI[FIXEDWINGPATHFOLLOWERSETTINGS_COURSEPI_ILIMIT],
		fixedwingpathfollowerSettings.CoursePI[FIXEDWINGPATHFOLLOWERSETTINGS_COURSEPI_ILIMIT]);
	courseCommand = (courseError * fixedwingpathfollowerSettings.CoursePI[FIXEDWINGPATHFOLLOWERSETTINGS_COURSEPI_KP] +
		courseIntegral);

	fixedwingpathfollowerStatus.E[FIXEDWINGPATHFOLLOWERSTATUS_E_COURSE] = courseError;
	fixedwingpathfollowerStatus.A[FIXEDWINGPATHFOLLOWERSTATUS_A_COURSE] = courseIntegral;
	fixedwingpathfollowerStatus.C[FIXEDWINGPATHFOLLOWERSTATUS_C_COURSE] = courseCommand;
	
	stabDesired.Roll = bound( fixedwingpathfollowerSettings.RollLimit[FIXEDWINGPATHFOLLOWERSETTINGS_ROLLLIMIT_NEUTRAL] +
		courseCommand,
		fixedwingpathfollowerSettings.RollLimit[FIXEDWINGPATHFOLLOWERSETTINGS_ROLLLIMIT_MIN],
		fixedwingpathfollowerSettings.RollLimit[FIXEDWINGPATHFOLLOWERSETTINGS_ROLLLIMIT_MAX] );

	// TODO: find a check to determine loss of directional control. Likely needs some check of derivative


	/**
	 * Compute desired yaw command
	 */
	// TODO implement raw control mode for yaw and base on Accels.X
	stabDesired.Yaw = 0;


	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] = STABILIZATIONDESIRED_STABILIZATIONMODE_NONE;
	
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

static void SettingsUpdatedCb(UAVObjEvent * ev)
{
	FixedWingPathFollowerSettingsGet(&fixedwingpathfollowerSettings);
	PathDesiredGet(&pathDesired);
}

static void baroAirspeedUpdatedCb(UAVObjEvent * ev)
{

	BaroAirspeedData baroAirspeed;
	VelocityActualData velocityActual;

	BaroAirspeedGet(&baroAirspeed);
	if (baroAirspeed.Connected != BAROAIRSPEED_CONNECTED_TRUE) {
		baroAirspeedBias = 0;
	} else {
		VelocityActualGet(&velocityActual);
		float groundspeed = sqrtf(velocityActual.East*velocityActual.East + velocityActual.North*velocityActual.North );

		baroAirspeedBias = baroAirspeed.Airspeed - groundspeed;
	}

}
