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

#include "fixedwingpathfollower.h"
#include "accels.h"
#include "attitudeactual.h"
#include "pathdesired.h"	// object that will be updated by the module
#include "positiondesired.h"	// object that will be updated by the module
#include "positionactual.h"
#include "manualcontrol.h"
#include "flightstatus.h"
#include "baroairspeed.h"
#include "gpsvelocity.h"
#include "gpsposition.h"
#include "fixedwingpathfollowersettings.h"
#include "fixedwingpathfollowerstatus.h"
#include "homelocation.h"
#include "nedaccel.h"
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
#define GEE 9.81f
// Private types

// Private variables
static xTaskHandle fixedwingpathfollowerTaskHandle;
static xQueueHandle queue;

// Private functions
static void fixedwingpathfollowerTask(void *parameters);
static float bound(float val, float min, float max);

static void updateNedAccel();
static void updatePathVelocity();
static void updateVtolDesiredVelocity();
static void manualSetDesiredVelocity();
static void updateFixedDesiredAttitude();
static void updateVtolDesiredAttitude();
static void baroAirspeedUpdatedCb(UAVObjEvent * ev);

static FixedWingPathFollowerSettingsData fixedwingpathfollowerSettings;

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t FixedWingPathFollowerStart()
{
	// Start main task
	xTaskCreate(fixedwingpathfollowerTask, (signed char *)"FixedWingPathFollower", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &fixedwingpathfollowerTaskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_GUIDANCE, fixedwingpathfollowerTaskHandle);

	return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t FixedWingPathFollowerInitialize()
{
	FixedWingPathFollowerSettingsInitialize();
	FixedWingPathFollowerStatusInitialize();
	AttitudeActualInitialize();
	NedAccelInitialize();
	PathDesiredInitialize();
	PositionDesiredInitialize();
	VelocityDesiredInitialize();
	BaroAirspeedInitialize();

	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));
	
	// Listen for updates.
	AccelsConnectQueue(queue);
	BaroAirspeedConnectCallback(baroAirspeedUpdatedCb);
	
	return 0;
}
MODULE_INITCALL(FixedWingPathFollowerInitialize, FixedWingPathFollowerStart)

static float northVelIntegral = 0;
static float eastVelIntegral = 0;
static float downVelIntegral = 0;

static float northPosIntegral = 0;
static float eastPosIntegral = 0;
static float downPosIntegral = 0;

static float courseIntegral = 0;
static float speedIntegral = 0;
static float accelIntegral = 0;
static float powerIntegral = 0;
static uint8_t positionHoldLast = 0;

// correct speed by measured airspeed
static float baroAirspeedBias = 0;

/**
 * Module thread, should not return.
 */
static void fixedwingpathfollowerTask(void *parameters)
{
	SystemSettingsData systemSettings;
	FlightStatusData flightStatus;

	portTickType thisTime;
	portTickType lastUpdateTime;
	UAVObjEvent ev;
	
	// Main task loop
	lastUpdateTime = xTaskGetTickCount();
	while (1) {
		FixedWingPathFollowerSettingsGet(&fixedwingpathfollowerSettings);

		// Wait until the Accels object is updated, if a timeout then go to failsafe
		if ( xQueueReceive(queue, &ev, fixedwingpathfollowerSettings.UpdatePeriod / portTICK_RATE_MS) != pdTRUE )
		{
			AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_WARNING);
		} else {
			AlarmsClear(SYSTEMALARMS_ALARM_GUIDANCE);
		}

		// Continue collecting data if not enough time
		thisTime = xTaskGetTickCount();
		if( (thisTime - lastUpdateTime) < (fixedwingpathfollowerSettings.UpdatePeriod / portTICK_RATE_MS) )
			continue;

		// Convert the accels into the NED frame
		updateNedAccel();
		
		FlightStatusGet(&flightStatus);
		SystemSettingsGet(&systemSettings);
		FixedWingPathFollowerSettingsGet(&fixedwingpathfollowerSettings);
		
		if ((PARSE_FLIGHT_MODE(flightStatus.FlightMode) == FLIGHTMODE_GUIDANCE) &&
			((systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWING) ||
			(systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGELEVON) ||
			(systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGVTAIL) ||
			(systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_VTOL) ||
			(systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_QUADP) ||
			(systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_QUADX) ||
			(systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_HEXA) ))
		{
			if(flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD) {
				if (positionHoldLast == 0) {
					/* When enter position hold mode save current position */
					PositionDesiredData positionDesired;
					PositionActualData positionActual;
					PositionDesiredGet(&positionDesired);
					PositionActualGet(&positionActual);
					positionDesired.North = positionActual.North;
					positionDesired.East = positionActual.East;
					positionDesired.Down = positionActual.Down;
					PositionDesiredSet(&positionDesired);
					positionHoldLast = 1;
				}
			} else {
				positionHoldLast = 0;
			}
			if (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE) {
				/* Fly to home position - NED coordinates [0,0, -altitude offset] */
				PositionDesiredData positionDesired;
				PositionDesiredGet(&positionDesired);
				positionDesired.North = 0;
				positionDesired.East = 0;
				positionDesired.Down = -fixedwingpathfollowerSettings.ReturnTobaseAltitudeOffset;
				PositionDesiredSet(&positionDesired);
			} 
			
			if( flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD || flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE || flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER ) {
				if (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER && fixedwingpathfollowerSettings.PathMode != GUIDANCESETTINGS_PATHMODE_ENDPOINT) {
					updatePathVelocity();
				} else {
					updateVtolDesiredVelocity();
				}
			} else { 
				manualSetDesiredVelocity();
			}

			if ((systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWING) ||
				(systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGELEVON) ||
				(systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGVTAIL))
			{
				updateFixedDesiredAttitude();
			} else {
				updateVtolDesiredAttitude();
			}
			
		} else {
			// Be cleaner and get rid of global variables
			northVelIntegral = 0;
			eastVelIntegral = 0;
			downVelIntegral = 0;
			northPosIntegral = 0;
			eastPosIntegral = 0;
			downPosIntegral = 0;
			positionHoldLast = 0;
			courseIntegral = 0;
			speedIntegral = 0;
			accelIntegral = 0;
			powerIntegral = 0;
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
	static portTickType lastSysTime;
	portTickType thisSysTime = xTaskGetTickCount();;
	float dT = 0;
	float downCommand;

	// Check how long since last update
	if(thisSysTime > lastSysTime) // reuse dt in case of wraparound
		dT = (thisSysTime - lastSysTime) / portTICK_RATE_MS / 1000.0f;
	lastSysTime = thisSysTime;

	PathDesiredData pathDesired;
	PathDesiredGet(&pathDesired);

	PositionActualData positionActual;
	PositionActualGet(&positionActual);
	
	float cur[3] = {positionActual.North, positionActual.East, positionActual.Down};
	struct path_status progress;
	
	path_progress(pathDesired.Start, pathDesired.End, cur, &progress);
	
	float groundspeed = pathDesired.StartingVelocity + 
	    (pathDesired.EndingVelocity - pathDesired.StartingVelocity) * progress.fractional_progress;
	if(progress.fractional_progress > 1)
		groundspeed = 0;
	
	VelocityDesiredData velocityDesired;
	velocityDesired.North = progress.path_direction[0] * groundspeed;
	velocityDesired.East = progress.path_direction[1] * groundspeed;
	
	float error_speed = progress.error * fixedwingpathfollowerSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_KP];
	float correction_velocity[2] = {progress.correction_direction[0] * error_speed, 
	    progress.correction_direction[1] * error_speed};
	
	float total_vel = sqrtf(powf(correction_velocity[0],2) + powf(correction_velocity[1],2));
	float scale = 1;
	if(total_vel > fixedwingpathfollowerSettings.HorizontalVelMax)
		scale = fixedwingpathfollowerSettings.HorizontalVelMax / total_vel;

	velocityDesired.North += progress.correction_direction[0] * error_speed * scale;
	velocityDesired.East += progress.correction_direction[1] * error_speed * scale;
	
	float altitudeSetpoint = pathDesired.Start[2] + (pathDesired.End[2] - pathDesired.Start[2]) *
	    bound(progress.fractional_progress,0,1);

	float downError = altitudeSetpoint - positionActual.Down;
	downPosIntegral = bound(downPosIntegral + downError * dT * fixedwingpathfollowerSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_KI],
							-fixedwingpathfollowerSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_ILIMIT],
							fixedwingpathfollowerSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_ILIMIT]);
	downCommand = (downError * fixedwingpathfollowerSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_KP] + downPosIntegral);
	velocityDesired.Down = bound(downCommand,
								 -fixedwingpathfollowerSettings.VerticalVelMax,
								 fixedwingpathfollowerSettings.VerticalVelMax);

	VelocityDesiredSet(&velocityDesired);
}

/**
 * Compute desired velocity from the current position
 *
 * Takes in @ref PositionActual and compares it to @ref PositionDesired 
 * and computes @ref VelocityDesired
 */
void updateVtolDesiredVelocity()
{
	static portTickType lastSysTime;
	portTickType thisSysTime = xTaskGetTickCount();;
	float dT = 0;

	FixedWingPathFollowerSettingsData fixedwingpathfollowerSettings;
	PositionActualData positionActual;
	PositionDesiredData positionDesired;
	VelocityDesiredData velocityDesired;
	
	FixedWingPathFollowerSettingsGet(&fixedwingpathfollowerSettings);
	PositionActualGet(&positionActual);
	PositionDesiredGet(&positionDesired);
	VelocityDesiredGet(&velocityDesired);
	
	float northError;
	float eastError;
	float downError;
	float northCommand;
	float eastCommand;
	float downCommand;

	// Check how long since last update
	if(thisSysTime > lastSysTime) // reuse dt in case of wraparound
		dT = (thisSysTime - lastSysTime) / portTICK_RATE_MS / 1000.0f;		
	lastSysTime = thisSysTime;
	
	float northPos = 0, eastPos = 0, downPos = 0;
	switch (fixedwingpathfollowerSettings.PositionSource) {
		case GUIDANCESETTINGS_POSITIONSOURCE_EKF:
			northPos = positionActual.North;
			eastPos = positionActual.East;
			downPos = positionActual.Down;
			break;
		case GUIDANCESETTINGS_POSITIONSOURCE_GPSPOS:
		{
			NEDPositionData nedPosition;
			NEDPositionGet(&nedPosition);
			northPos = nedPosition.North;
			eastPos = nedPosition.East;
			downPos = nedPosition.Down;
		}
			break;
		default:
			PIOS_Assert(0);
			break;
	}

	// Note all distances in cm
	// Compute desired north command
	northError = positionDesired.North - northPos;
	northPosIntegral = bound(northPosIntegral + northError * dT * fixedwingpathfollowerSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_KI], 
		-fixedwingpathfollowerSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_ILIMIT],
		fixedwingpathfollowerSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_ILIMIT]);
	northCommand = (northError * fixedwingpathfollowerSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_KP] +
		northPosIntegral);
	
	eastError = positionDesired.East - eastPos;
	eastPosIntegral = bound(eastPosIntegral + eastError * dT * fixedwingpathfollowerSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_KI], 
		-fixedwingpathfollowerSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_ILIMIT],
		fixedwingpathfollowerSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_ILIMIT]);
	eastCommand = (eastError * fixedwingpathfollowerSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_KP] +
		       eastPosIntegral);
	
	
	float total_vel = sqrtf(northCommand * northCommand + eastCommand * eastCommand);
	float scale = 1.0f;
	if(total_vel > fixedwingpathfollowerSettings.HorizontalVelMax)
		scale = fixedwingpathfollowerSettings.HorizontalVelMax / total_vel;

	velocityDesired.North = northCommand * scale;
	velocityDesired.East = eastCommand * scale;

	downError = positionDesired.Down - downPos;
	downPosIntegral = bound(downPosIntegral + downError * dT * fixedwingpathfollowerSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_KI], 
		-fixedwingpathfollowerSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_ILIMIT],
		fixedwingpathfollowerSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_ILIMIT]);
	downCommand = (downError * fixedwingpathfollowerSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_KP] + downPosIntegral);
	velocityDesired.Down = bound(downCommand,
		-fixedwingpathfollowerSettings.VerticalVelMax, 
		fixedwingpathfollowerSettings.VerticalVelMax);
	
	VelocityDesiredSet(&velocityDesired);	
}

/**
 * Compute desired attitude from the desired velocity
 *
 * Takes in @ref NedActual which has the acceleration in the 
 * NED frame as the feedback term and then compares the 
 * @ref VelocityActual against the @ref VelocityDesired
 */
static void updateFixedDesiredAttitude()
{
	static portTickType lastSysTime;
	portTickType thisSysTime = xTaskGetTickCount();;
	float dT = 0;

	VelocityDesiredData velocityDesired;
	VelocityActualData velocityActual;
	StabilizationDesiredData stabDesired;
	AttitudeActualData attitudeActual;
	NedAccelData nedAccel;
	AccelsData accels;
	FixedWingPathFollowerSettingsData fixedwingpathfollowerSettings;
	StabilizationSettingsData stabSettings;
	SystemSettingsData systemSettings;
	FixedWingPathFollowerStatusData fixedwingpathfollowerStatus;

	float courseError;
	float courseCommand;

	float speedError;
	float accelCommand;

	float speedActual;
	float speedDesired;
	float accelDesired;
	float accelError;

	float powerError;
	float powerCommand;


	// Check how long since last update
	if(thisSysTime > lastSysTime) // reuse dt in case of wraparound
		dT = (thisSysTime - lastSysTime) / portTICK_RATE_MS / 1000.0f;		
	lastSysTime = thisSysTime;
	
	SystemSettingsGet(&systemSettings);
	FixedWingPathFollowerSettingsGet(&fixedwingpathfollowerSettings);

	FixedWingPathFollowerStatusGet(&fixedwingpathfollowerStatus);
	
	VelocityActualGet(&velocityActual);
	VelocityDesiredGet(&velocityDesired);
	StabilizationDesiredGet(&stabDesired);
	VelocityDesiredGet(&velocityDesired);
	AttitudeActualGet(&attitudeActual);
	AccelsGet(&accels);
	StabilizationSettingsGet(&stabSettings);
	NedAccelGet(&nedAccel);

	// current speed - lacking forward airspeed we use groundspeed :(
	speedActual = sqrtf(velocityActual.East*velocityActual.East + velocityActual.North*velocityActual.North + velocityActual.Down*velocityActual.Down ) + baroAirspeedBias;

	// Compute desired roll command
	courseError = RAD2DEG * (atan2f(velocityDesired.East,velocityDesired.North) - atan2f(velocityActual.East,velocityActual.North));
	if (courseError<-180.0f) courseError+=360.0f;
	if (courseError>180.0f) courseError-=360.0f;

	courseIntegral = bound(courseIntegral + courseError * dT * fixedwingpathfollowerSettings.CoursePI[GUIDANCESETTINGS_COURSEPI_KI], 
		-fixedwingpathfollowerSettings.CoursePI[GUIDANCESETTINGS_COURSEPI_ILIMIT],
		fixedwingpathfollowerSettings.CoursePI[GUIDANCESETTINGS_COURSEPI_ILIMIT]);
	courseCommand = (courseError * fixedwingpathfollowerSettings.CoursePI[GUIDANCESETTINGS_COURSEPI_KP] +
		courseIntegral);

	fixedwingpathfollowerStatus.E[GUIDANCESTATUS_E_COURSE] = courseError;
	fixedwingpathfollowerStatus.A[GUIDANCESTATUS_A_COURSE] = courseIntegral;
	fixedwingpathfollowerStatus.C[GUIDANCESTATUS_C_COURSE] = courseCommand;
	
	stabDesired.Roll = bound( fixedwingpathfollowerSettings.RollLimit[GUIDANCESETTINGS_ROLLLIMIT_NEUTRAL] +
		courseCommand,
		fixedwingpathfollowerSettings.RollLimit[GUIDANCESETTINGS_ROLLLIMIT_MIN],
		fixedwingpathfollowerSettings.RollLimit[GUIDANCESETTINGS_ROLLLIMIT_MAX] );

	// Compute desired yaw command
	if (speedActual>0) {
		// rate is speed dependent and roll dependent. The faster the plane, the slower it turns at a given roll angle.
		// (A "fixed roll angle level turn" is a turn at fixed G rate)
		//stabDesired.Yaw = RAD2DEG * tanf(stabDesired.Roll / RAD2DEG) * GEE / speedActual;
		// this is a global rate - translate to local since rates are always local
		//stabDesired.Yaw = stabDesired.Yaw * cosf(stabDesired.Roll / RAD2DEG);
		// tan = sin/cos - so tan*cos = sin
		stabDesired.Yaw = RAD2DEG * sinf((stabDesired.Roll-fixedwingpathfollowerSettings.RollLimit[GUIDANCESETTINGS_ROLLLIMIT_NEUTRAL]) / RAD2DEG) * GEE / speedActual;
	} else {
		stabDesired.Yaw = 0;
	}

	// Compute desired speed command  TODO: make cruise speed a variable
	speedDesired = fixedwingpathfollowerSettings.CruiseSpeed;
	speedError = speedDesired - speedActual;

	accelDesired = bound( speedError * fixedwingpathfollowerSettings.SpeedP[GUIDANCESETTINGS_SPEEDP_KP],
		-fixedwingpathfollowerSettings.SpeedP[GUIDANCESETTINGS_SPEEDP_MAX],
		fixedwingpathfollowerSettings.SpeedP[GUIDANCESETTINGS_SPEEDP_MAX]);
	
	fixedwingpathfollowerStatus.E[GUIDANCESTATUS_E_SPEED] = speedError;
	fixedwingpathfollowerStatus.A[GUIDANCESTATUS_A_SPEED] = 0.0f;
	fixedwingpathfollowerStatus.C[GUIDANCESTATUS_C_SPEED] = accelDesired;
	
	accelError = accelDesired - accels.x;
	accelIntegral = bound(accelIntegral + accelError * dT * fixedwingpathfollowerSettings.AccelPI[GUIDANCESETTINGS_ACCELPI_KI], 
		-fixedwingpathfollowerSettings.AccelPI[GUIDANCESETTINGS_ACCELPI_ILIMIT],
		fixedwingpathfollowerSettings.AccelPI[GUIDANCESETTINGS_ACCELPI_ILIMIT]);
	accelCommand = (accelError * fixedwingpathfollowerSettings.AccelPI[GUIDANCESETTINGS_ACCELPI_KP] + 
		 accelIntegral);
	
	fixedwingpathfollowerStatus.E[GUIDANCESTATUS_E_ACCEL] = accelError;
	fixedwingpathfollowerStatus.A[GUIDANCESTATUS_A_ACCEL] = accelIntegral;
	fixedwingpathfollowerStatus.C[GUIDANCESTATUS_C_ACCEL] = accelCommand;

	stabDesired.Pitch = bound(fixedwingpathfollowerSettings.PitchLimit[GUIDANCESETTINGS_PITCHLIMIT_NEUTRAL] +
		-accelCommand,
		fixedwingpathfollowerSettings.PitchLimit[GUIDANCESETTINGS_PITCHLIMIT_MIN],
		fixedwingpathfollowerSettings.PitchLimit[GUIDANCESETTINGS_PITCHLIMIT_MAX]);

	// Compute desired power command
	powerError =  -( velocityDesired.Down - velocityActual.Down ) * fixedwingpathfollowerSettings.ClimbRateBoostFactor + speedError;
	powerIntegral =	bound(powerIntegral + powerError * dT * fixedwingpathfollowerSettings.PowerPI[GUIDANCESETTINGS_POWERPI_KI], 
		-fixedwingpathfollowerSettings.PowerPI[GUIDANCESETTINGS_POWERPI_ILIMIT],
		fixedwingpathfollowerSettings.PowerPI[GUIDANCESETTINGS_POWERPI_ILIMIT]);
	powerCommand = (powerError * fixedwingpathfollowerSettings.PowerPI[GUIDANCESETTINGS_POWERPI_KP] +
		powerIntegral) + fixedwingpathfollowerSettings.ThrottleLimit[GUIDANCESETTINGS_THROTTLELIMIT_NEUTRAL];

	// prevent integral running out of bounds 
	if ( powerCommand > fixedwingpathfollowerSettings.ThrottleLimit[GUIDANCESETTINGS_THROTTLELIMIT_MAX]) {
		powerIntegral = bound(
			powerIntegral -
				( powerCommand 
				- fixedwingpathfollowerSettings.ThrottleLimit[GUIDANCESETTINGS_THROTTLELIMIT_MAX]),
			-fixedwingpathfollowerSettings.PowerPI[GUIDANCESETTINGS_POWERPI_ILIMIT],
			fixedwingpathfollowerSettings.PowerPI[GUIDANCESETTINGS_POWERPI_ILIMIT]);
		powerCommand = fixedwingpathfollowerSettings.ThrottleLimit[GUIDANCESETTINGS_THROTTLELIMIT_MAX];
	}
	if ( powerCommand < fixedwingpathfollowerSettings.ThrottleLimit[GUIDANCESETTINGS_THROTTLELIMIT_MIN]) {
		powerIntegral = bound(
			powerIntegral -
				( powerCommand
				- fixedwingpathfollowerSettings.ThrottleLimit[GUIDANCESETTINGS_THROTTLELIMIT_MIN]),
			-fixedwingpathfollowerSettings.PowerPI[GUIDANCESETTINGS_POWERPI_ILIMIT],
			fixedwingpathfollowerSettings.PowerPI[GUIDANCESETTINGS_POWERPI_ILIMIT]);
		powerCommand = fixedwingpathfollowerSettings.ThrottleLimit[GUIDANCESETTINGS_THROTTLELIMIT_MIN];
	}

	fixedwingpathfollowerStatus.E[GUIDANCESTATUS_E_POWER] = powerError;
	fixedwingpathfollowerStatus.A[GUIDANCESTATUS_A_POWER] = powerIntegral;
	fixedwingpathfollowerStatus.C[GUIDANCESTATUS_C_POWER] = powerCommand;

	// set throttle
	stabDesired.Throttle = powerCommand;

	if(fixedwingpathfollowerSettings.ThrottleControl == GUIDANCESETTINGS_THROTTLECONTROL_FALSE) {
		// For now override throttle with manual control.  Disable at your risk, quad goes to China.
		ManualControlCommandData manualControl;
		ManualControlCommandGet(&manualControl);
		stabDesired.Throttle = manualControl.Throttle;
	}
//printf("Cycle:	speed Error: %f\n	powerError: %f\n	accelCommand: %f\n	powerCommand: %f\n\n",speedError,powerError,accelCommand,powerCommand);

	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
	
	StabilizationDesiredSet(&stabDesired);

	FixedWingPathFollowerStatusSet(&fixedwingpathfollowerStatus);
}

/**
 * Compute desired attitude from the desired velocity
 *
 * Takes in @ref NedActual which has the acceleration in the 
 * NED frame as the feedback term and then compares the 
 * @ref VelocityActual against the @ref VelocityDesired
 */
static void updateVtolDesiredAttitude()
{
	static portTickType lastSysTime;
	portTickType thisSysTime = xTaskGetTickCount();;
	float dT = 0;

	VelocityDesiredData velocityDesired;
	VelocityActualData velocityActual;
	StabilizationDesiredData stabDesired;
	AttitudeActualData attitudeActual;
	NedAccelData nedAccel;
	FixedWingPathFollowerSettingsData fixedwingpathfollowerSettings;
	StabilizationSettingsData stabSettings;
	SystemSettingsData systemSettings;

	float northError;
	float northCommand;
	
	float eastError;
	float eastCommand;

	float downError;
	float downCommand;
	
	// Check how long since last update
	if(thisSysTime > lastSysTime) // reuse dt in case of wraparound
		dT = (thisSysTime - lastSysTime) / portTICK_RATE_MS / 1000.0f;		
	lastSysTime = thisSysTime;
	
	SystemSettingsGet(&systemSettings);
	FixedWingPathFollowerSettingsGet(&fixedwingpathfollowerSettings);
	
	VelocityActualGet(&velocityActual);
	VelocityDesiredGet(&velocityDesired);
	StabilizationDesiredGet(&stabDesired);
	VelocityDesiredGet(&velocityDesired);
	AttitudeActualGet(&attitudeActual);
	StabilizationSettingsGet(&stabSettings);
	NedAccelGet(&nedAccel);
	
	float northVel = 0, eastVel = 0, downVel = 0;
	switch (fixedwingpathfollowerSettings.VelocitySource) {
		case GUIDANCESETTINGS_VELOCITYSOURCE_EKF:
			northVel = velocityActual.North;
			eastVel = velocityActual.East;
			downVel = velocityActual.Down;
			break;
		case GUIDANCESETTINGS_VELOCITYSOURCE_NEDVEL:
		{
			GPSVelocityData gpsVelocity;
			GPSVelocityGet(&gpsVelocity);
			northVel = gpsVelocity.North;
			eastVel = gpsVelocity.East;
			downVel = gpsVelocity.Down;
		}
			break;
		case GUIDANCESETTINGS_VELOCITYSOURCE_GPSPOS:
		{
			GPSPositionData gpsPosition;
			GPSPositionGet(&gpsPosition);
			northVel = gpsPosition.Groundspeed * cosf(gpsPosition.Heading * F_PI / 180.0f);
			eastVel = gpsPosition.Groundspeed * sinf(gpsPosition.Heading * F_PI / 180.0f);
			downVel = velocityActual.Down;
		}
			break;
		default:
			PIOS_Assert(0);
			break;
	}
	
	// Testing code - refactor into manual control command
	ManualControlCommandData manualControlData;
	ManualControlCommandGet(&manualControlData);
	stabDesired.Yaw = stabSettings.MaximumRate[STABILIZATIONSETTINGS_MAXIMUMRATE_YAW] * manualControlData.Yaw;	
	
	// Compute desired north command
	northError = velocityDesired.North - northVel;
	northVelIntegral = bound(northVelIntegral + northError * dT * fixedwingpathfollowerSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KI], 
		-fixedwingpathfollowerSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_ILIMIT],
		fixedwingpathfollowerSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_ILIMIT]);
	northCommand = (northError * fixedwingpathfollowerSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KP] +
			northVelIntegral -
			nedAccel.North * fixedwingpathfollowerSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KD] +
			velocityDesired.North * fixedwingpathfollowerSettings.VelocityFeedforward);
	
	// Compute desired east command
	eastError = velocityDesired.East - eastVel;
	eastVelIntegral = bound(eastVelIntegral + eastError * dT * fixedwingpathfollowerSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KI], 
		-fixedwingpathfollowerSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_ILIMIT],
		fixedwingpathfollowerSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_ILIMIT]);
	eastCommand = (eastError * fixedwingpathfollowerSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KP] + 
		       eastVelIntegral - 
		       nedAccel.East * fixedwingpathfollowerSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KD] +
			   velocityDesired.East * fixedwingpathfollowerSettings.VelocityFeedforward);
	
	// Compute desired down command
	downError = velocityDesired.Down - downVel;
	// Must flip this sign 
	downError = -downError;
	downVelIntegral = bound(downVelIntegral + downError * dT * fixedwingpathfollowerSettings.VerticalVelPID[GUIDANCESETTINGS_VERTICALVELPID_KI], 
		-fixedwingpathfollowerSettings.VerticalVelPID[GUIDANCESETTINGS_VERTICALVELPID_ILIMIT],
		fixedwingpathfollowerSettings.VerticalVelPID[GUIDANCESETTINGS_VERTICALVELPID_ILIMIT]);	
	downCommand = (downError * fixedwingpathfollowerSettings.VerticalVelPID[GUIDANCESETTINGS_VERTICALVELPID_KP] +
		downVelIntegral -
		nedAccel.Down * fixedwingpathfollowerSettings.VerticalVelPID[GUIDANCESETTINGS_VERTICALVELPID_KD]);
	
	stabDesired.Throttle = bound(fixedwingpathfollowerSettings.ThrottleLimit[GUIDANCESETTINGS_THROTTLELIMIT_NEUTRAL] +
		downCommand,
		fixedwingpathfollowerSettings.ThrottleLimit[GUIDANCESETTINGS_THROTTLELIMIT_MIN],
		fixedwingpathfollowerSettings.ThrottleLimit[GUIDANCESETTINGS_THROTTLELIMIT_MAX]);
	
	// Project the north and east command signals into the pitch and roll based on yaw.  For this to behave well the
	// craft should move similarly for 5 deg roll versus 5 deg pitch
	stabDesired.Pitch = bound(fixedwingpathfollowerSettings.PitchLimit[GUIDANCESETTINGS_PITCHLIMIT_NEUTRAL] +
		(-northCommand * cosf(attitudeActual.Yaw * F_PI / 180.0f)) +
		(-eastCommand * sinf(attitudeActual.Yaw * F_PI / 180.0f)),
		fixedwingpathfollowerSettings.PitchLimit[GUIDANCESETTINGS_PITCHLIMIT_MIN],
		fixedwingpathfollowerSettings.PitchLimit[GUIDANCESETTINGS_PITCHLIMIT_MAX]);
	stabDesired.Roll = bound(fixedwingpathfollowerSettings.RollLimit[GUIDANCESETTINGS_ROLLLIMIT_NEUTRAL] +
		(-northCommand * sinf(attitudeActual.Yaw * F_PI / 180.0f)) +
		(eastCommand * cosf(attitudeActual.Yaw * F_PI / 180.0f)),
		fixedwingpathfollowerSettings.RollLimit[GUIDANCESETTINGS_ROLLLIMIT_MIN],
		fixedwingpathfollowerSettings.RollLimit[GUIDANCESETTINGS_ROLLLIMIT_MAX] );
	
	if(fixedwingpathfollowerSettings.ThrottleControl == GUIDANCESETTINGS_THROTTLECONTROL_FALSE) {
		// For now override throttle with manual control.  Disable at your risk, quad goes to China.
		ManualControlCommandData manualControl;
		ManualControlCommandGet(&manualControl);
		stabDesired.Throttle = manualControl.Throttle;
	}
	
	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
	
	StabilizationDesiredSet(&stabDesired);
}

/**
 * Keep a running filtered version of the acceleration in the NED frame
 */
static void updateNedAccel()
{
	float accel[3];
	float q[4];
	float Rbe[3][3];
	float accel_ned[3];

	// Collect downsampled attitude data
	AccelsData accels;
	AccelsGet(&accels);		
	accel[0] = accels.x;
	accel[1] = accels.y;
	accel[2] = accels.z;
	
	//rotate avg accels into earth frame and store it
	AttitudeActualData attitudeActual;
	AttitudeActualGet(&attitudeActual);
	q[0]=attitudeActual.q1;
	q[1]=attitudeActual.q2;
	q[2]=attitudeActual.q3;
	q[3]=attitudeActual.q4;
	Quaternion2R(q, Rbe);
	for (uint8_t i=0; i<3; i++){
		accel_ned[i]=0;
		for (uint8_t j=0; j<3; j++)
			accel_ned[i] += Rbe[j][i]*accel[j];
	}
	accel_ned[2] += 9.81f;
	
	NedAccelData accelData;
	NedAccelGet(&accelData);
	accelData.North = accel_ned[0];
	accelData.East = accel_ned[1];
	accelData.Down = accel_ned[2];
	NedAccelSet(&accelData);
}

/**
 * Set the desired velocity from the input sticks
 */
static void manualSetDesiredVelocity() 
{
	ManualControlCommandData cmd;
	VelocityDesiredData velocityDesired;
	
	ManualControlCommandGet(&cmd);
	VelocityDesiredGet(&velocityDesired);

	FixedWingPathFollowerSettingsData fixedwingpathfollowerSettings;
	FixedWingPathFollowerSettingsGet(&fixedwingpathfollowerSettings);
	
	velocityDesired.North = -fixedwingpathfollowerSettings.HorizontalVelMax * cmd.Pitch;
	velocityDesired.East = fixedwingpathfollowerSettings.HorizontalVelMax * cmd.Roll;
	velocityDesired.Down = 0;
	
	VelocityDesiredSet(&velocityDesired);	
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


static void baroAirspeedUpdatedCb(UAVObjEvent * ev)
{

	BaroAirspeedData baroAirspeed;
	VelocityActualData velocityActual;

	BaroAirspeedGet(&baroAirspeed);
	if (baroAirspeed.Connected != BAROAIRSPEED_CONNECTED_TRUE) {
		baroAirspeedBias = 0;
	} else {
		VelocityActualGet(&velocityActual);
		float speed = sqrtf(velocityActual.East*velocityActual.East + velocityActual.North*velocityActual.North + velocityActual.Down*velocityActual.Down );

		baroAirspeedBias = baroAirspeed.Airspeed - speed;
	}

}
