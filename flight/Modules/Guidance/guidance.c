/**
 ******************************************************************************
 *
 * @file       guidance.c
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
#include "guidance.h"
#include "accels.h"
#include "attitudeactual.h"
#include "positiondesired.h"	// object that will be updated by the module
#include "positionactual.h"
#include "manualcontrol.h"
#include "flightstatus.h"
#include "gpsvelocity.h"
#include "gpsposition.h"
#include "guidancesettings.h"
#include "nedaccel.h"
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
// Private types

// Private variables
static xTaskHandle guidanceTaskHandle;
static xQueueHandle queue;

// Private functions
static void guidanceTask(void *parameters);
static float bound(float val, float min, float max);

static void updateVtolDesiredVelocity();
static void manualSetDesiredVelocity();
static void updateVtolDesiredAttitude();

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t GuidanceStart()
{
	// Start main task
	xTaskCreate(guidanceTask, (signed char *)"Guidance", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &guidanceTaskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_GUIDANCE, guidanceTaskHandle);

	return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t GuidanceInitialize()
{
	GuidanceSettingsInitialize();
	PositionDesiredInitialize();
	NedAccelInitialize();
	VelocityDesiredInitialize();

	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));
	
	// Listen for updates.
	AccelsConnectQueue(queue);
	
	return 0;
}
MODULE_INITCALL(GuidanceInitialize, GuidanceStart)

static float northVelIntegral = 0;
static float eastVelIntegral = 0;
static float downVelIntegral = 0;

static float northPosIntegral = 0;
static float eastPosIntegral = 0;
static float downPosIntegral = 0;

static uint8_t positionHoldLast = 0;

/**
 * Module thread, should not return.
 */
static void guidanceTask(void *parameters)
{
	SystemSettingsData systemSettings;
	GuidanceSettingsData guidanceSettings;
	FlightStatusData flightStatus;

	portTickType thisTime;
	portTickType lastUpdateTime;
	UAVObjEvent ev;
	
	float accel[3] = {0,0,0};
	uint32_t accel_accum = 0;
	
	float q[4];
	float Rbe[3][3];
	float accel_ned[3];
	
	// Main task loop
	lastUpdateTime = xTaskGetTickCount();
	while (1) {
		GuidanceSettingsGet(&guidanceSettings);

		// Wait until the AttitudeRaw object is updated, if a timeout then go to failsafe
		if ( xQueueReceive(queue, &ev, guidanceSettings.UpdatePeriod / portTICK_RATE_MS) != pdTRUE )
		{
			AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE,SYSTEMALARMS_ALARM_WARNING);
		} else {
			AlarmsClear(SYSTEMALARMS_ALARM_GUIDANCE);
		}

		// Collect downsampled attitude data
		AccelsData accels;
		AccelsGet(&accels);		
		accel[0] += accels.x;
		accel[1] += accels.y;
		accel[2] += accels.z;
		accel_accum++;

		// Continue collecting data if not enough time
		thisTime = xTaskGetTickCount();
		if( (thisTime - lastUpdateTime) < (guidanceSettings.UpdatePeriod / portTICK_RATE_MS) )
			continue;
		
		lastUpdateTime = xTaskGetTickCount();
		accel[0] /= accel_accum;
		accel[1] /= accel_accum;
		accel[2] /= accel_accum;
		accel[0] = accel[1] = accel[2] = 0;
		accel_accum = 0;
		
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
		
		FlightStatusGet(&flightStatus);
		SystemSettingsGet(&systemSettings);
		GuidanceSettingsGet(&guidanceSettings);
		
		if ((flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ||
			 flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER) &&
		    ((systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_VTOL) ||
		     (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_QUADP) ||
		     (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_QUADX) ||
		     (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_HEXA) ))
		{
			if( flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ||
			   flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER) 
				updateVtolDesiredVelocity();
			else 
				manualSetDesiredVelocity();			
			updateVtolDesiredAttitude();
		} else {
			// Be cleaner and get rid of global variables
			northVelIntegral = 0;
			eastVelIntegral = 0;
			downVelIntegral = 0;
			northPosIntegral = 0;
			eastPosIntegral = 0;
			downPosIntegral = 0;
			positionHoldLast = 0;
		}
	}
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

	GuidanceSettingsData guidanceSettings;
	PositionActualData positionActual;
	PositionDesiredData positionDesired;
	VelocityDesiredData velocityDesired;
	
	GuidanceSettingsGet(&guidanceSettings);
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
	
	// Note all distances in cm
	// Compute desired north command
	northError = positionDesired.North - positionActual.North;
	northPosIntegral = bound(northPosIntegral + northError * dT * guidanceSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_KI], 
			      -guidanceSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_ILIMIT],
			      guidanceSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_ILIMIT]);
	northCommand = (northError * guidanceSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_KP] +
			northPosIntegral);
	
	eastError = positionDesired.East - positionActual.East;
	eastPosIntegral = bound(eastPosIntegral + eastError * dT * guidanceSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_KI], 
				 -guidanceSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_ILIMIT],
				 guidanceSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_ILIMIT]);
	eastCommand = (eastError * guidanceSettings.HorizontalPosPI[GUIDANCESETTINGS_HORIZONTALPOSPI_KP] +
		       eastPosIntegral);
	
	
	float total_vel = sqrtf(powf(northCommand,2) + powf(eastCommand,2));
	float scale = 1;
	if(total_vel > guidanceSettings.HorizontalVelMax)
		scale = guidanceSettings.HorizontalVelMax / total_vel;

	velocityDesired.North = northCommand * scale;
	velocityDesired.East = eastCommand * scale;

	downError = positionDesired.Down - positionActual.Down;
	downPosIntegral = bound(downPosIntegral + downError * dT * guidanceSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_KI], 
				-guidanceSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_ILIMIT],
				guidanceSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_ILIMIT]);
	downCommand = (downError * guidanceSettings.VerticalPosPI[GUIDANCESETTINGS_VERTICALPOSPI_KP] + downPosIntegral);
	velocityDesired.Down = bound(downCommand,
				     -guidanceSettings.VerticalVelMax, 
				     guidanceSettings.VerticalVelMax);
	
	VelocityDesiredSet(&velocityDesired);	
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
	GuidanceSettingsData guidanceSettings;
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
	GuidanceSettingsGet(&guidanceSettings);
	
	VelocityActualGet(&velocityActual);
	VelocityDesiredGet(&velocityDesired);
	StabilizationDesiredGet(&stabDesired);
	VelocityDesiredGet(&velocityDesired);
	AttitudeActualGet(&attitudeActual);
	StabilizationSettingsGet(&stabSettings);
	NedAccelGet(&nedAccel);
	
	float northVel, eastVel, downVel;
	switch (guidanceSettings.VelocitySource) {
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
		case GUIDANCESETTINGS_VELOCITYSOURCE_GPSPOS:
		{
			GPSPositionData gpsPosition;
			GPSPositionGet(&gpsPosition);
			northVel = gpsPosition.Groundspeed * cosf(gpsPosition.Heading * F_PI / 180.0f);
			eastVel = gpsPosition.Groundspeed * sinf(gpsPosition.Heading * F_PI / 180.0f);
			downVel = velocityActual.Down;
		}
		default:
			break;
	}
	
	// Testing code - refactor into manual control command
	ManualControlCommandData manualControlData;
	ManualControlCommandGet(&manualControlData);
	stabDesired.Yaw = stabSettings.MaximumRate[STABILIZATIONSETTINGS_MAXIMUMRATE_YAW] * manualControlData.Yaw;	
	
	// Compute desired north command
	northError = velocityDesired.North - northVel;
	northVelIntegral = bound(northVelIntegral + northError * dT * guidanceSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KI], 
			      -guidanceSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_ILIMIT],
			      guidanceSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_ILIMIT]);
	northCommand = (northError * guidanceSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KP] +
			northVelIntegral -
			nedAccel.North * guidanceSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KD]);
	
	// Compute desired east command
	eastError = velocityDesired.East - eastVel;
	eastVelIntegral = bound(eastVelIntegral + eastError * dT * guidanceSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KI], 
			     -guidanceSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_ILIMIT],
			     guidanceSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_ILIMIT]);
	eastCommand = (eastError * guidanceSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KP] + 
		       eastVelIntegral - 
		       nedAccel.East * guidanceSettings.HorizontalVelPID[GUIDANCESETTINGS_HORIZONTALVELPID_KD]);
	
	// Compute desired down command
	downError = velocityDesired.Down - downVel;
	// Must flip this sign 
	downError = -downError;
	downVelIntegral = bound(downVelIntegral + downError * dT * guidanceSettings.VerticalVelPID[GUIDANCESETTINGS_VERTICALVELPID_KI], 
			      -guidanceSettings.VerticalVelPID[GUIDANCESETTINGS_VERTICALVELPID_ILIMIT],
			      guidanceSettings.VerticalVelPID[GUIDANCESETTINGS_VERTICALVELPID_ILIMIT]);	
	downCommand = (downError * guidanceSettings.VerticalVelPID[GUIDANCESETTINGS_VERTICALVELPID_KP] +
		       downVelIntegral -
		       nedAccel.Down * guidanceSettings.VerticalVelPID[GUIDANCESETTINGS_VERTICALVELPID_KD]);
	
	stabDesired.Throttle = bound(downCommand, 0, 1);
	
	// Project the north and east command signals into the pitch and roll based on yaw.  For this to behave well the
	// craft should move similarly for 5 deg roll versus 5 deg pitch
	stabDesired.Pitch = bound(-northCommand * cosf(attitudeActual.Yaw * M_PI / 180) + 
				      -eastCommand * sinf(attitudeActual.Yaw * M_PI / 180),
				      -guidanceSettings.MaxRollPitch, guidanceSettings.MaxRollPitch);
	stabDesired.Roll = bound(-northCommand * sinf(attitudeActual.Yaw * M_PI / 180) + 
				     eastCommand * cosf(attitudeActual.Yaw * M_PI / 180),
				     -guidanceSettings.MaxRollPitch, guidanceSettings.MaxRollPitch);
	
	if(guidanceSettings.ThrottleControl == GUIDANCESETTINGS_THROTTLECONTROL_FALSE) {
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
 * Set the desired velocity from the input sticks
 */
static void manualSetDesiredVelocity() 
{
	ManualControlCommandData cmd;
	VelocityDesiredData velocityDesired;
	
	ManualControlCommandGet(&cmd);
	VelocityDesiredGet(&velocityDesired);

	GuidanceSettingsData guidanceSettings;
	GuidanceSettingsGet(&guidanceSettings);
	
	velocityDesired.North = -guidanceSettings.HorizontalVelMax * cmd.Pitch;
	velocityDesired.East = guidanceSettings.HorizontalVelMax * cmd.Roll;
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
