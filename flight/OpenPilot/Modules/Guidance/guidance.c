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
#include "guidancesettings.h"
#include "attituderaw.h"
#include "attitudeactual.h"
#include "attitudedesired.h"
#include "positiondesired.h"	// object that will be updated by the module
#include "positionactual.h"
#include "manualcontrolcommand.h"
#include "nedaccel.h"
#include "stabilizationsettings.h"
#include "systemsettings.h"
#include "velocitydesired.h"
#include "velocityactual.h"
#include "CoordinateConversions.h"

// Private constants
#define MAX_QUEUE_SIZE 1
#define STACK_SIZE_BYTES 824
#define TASK_PRIORITY (tskIDLE_PRIORITY+2)
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
static void positionPIDcontrol();

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t GuidanceInitialize()
{
	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));
	
	// Listen for updates.
	AttitudeRawConnectQueue(queue);
	
	// Start main task
	xTaskCreate(guidanceTask, (signed char *)"Guidance", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &guidanceTaskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_GUIDANCE, guidanceTaskHandle);

	return 0;
}

static float northIntegral = 0;
static float eastIntegral = 0;
static float downIntegral = 0;
static uint8_t positionHoldLast = 0;

/**
 * Module thread, should not return.
 */
static void guidanceTask(void *parameters)
{
	SystemSettingsData systemSettings;
	GuidanceSettingsData guidanceSettings;
	ManualControlCommandData manualControl;

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

		// Wait until the AttitudeRaw object is updated, if a timeout then go to failsafe
		if ( xQueueReceive(queue, &ev, guidanceSettings.VelUpdatePeriod / portTICK_RATE_MS) != pdTRUE )
		{
			AlarmsSet(SYSTEMALARMS_ALARM_STABILIZATION,SYSTEMALARMS_ALARM_WARNING);
			continue;
		} 
				
		// Collect downsampled attitude data
		AttitudeRawData attitudeRaw;
		AttitudeRawGet(&attitudeRaw);		
		accel[0] += attitudeRaw.gyros_filtered[0];
		accel[1] += attitudeRaw.gyros_filtered[1];
		accel[2] += attitudeRaw.gyros_filtered[2];
		accel_accum++;
		
		// Continue collecting data if not enough time
		thisTime = xTaskGetTickCount();
		if( (thisTime - lastUpdateTime) < (guidanceSettings.VelUpdatePeriod / portTICK_RATE_MS) )
			continue;
		
		lastUpdateTime = xTaskGetTickCount();
		accel[0] /= accel_accum;
		accel[1] /= accel_accum;
		accel[2] /= accel_accum;
		
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
		accel_ned[2] += 9.81;
		
		NedAccelData accelData;
		NedAccelGet(&accelData);
		// Convert from m/s to cm/s
		accelData.North = accel[0] * 100;
		accelData.East = accel[1] * 100;
		accelData.North = accel[2] * 100;
		NedAccelSet(&accelData);
		
		
		ManualControlCommandGet(&manualControl);
		SystemSettingsGet(&systemSettings);
		GuidanceSettingsGet(&guidanceSettings);
		
		if ((manualControl.FlightMode == MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO) &&
		    ((systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_VTOL) ||
		     (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_QUADP) ||
		     (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_QUADX) ||
		     (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_HEXA) ))
		{
			if(positionHoldLast == 0) {
				/* When enter position hold mode save current position */
				PositionDesiredData positionDesired;
				PositionActualData positionActual;
				PositionDesiredGet(&positionDesired);
				PositionActualGet(&positionActual);
				positionDesired.North = positionActual.North;
				positionDesired.East = positionActual.East;
				PositionDesiredSet(&positionDesired);
				positionHoldLast = 1;
			}
			
			if(guidanceSettings.GuidanceMode == GUIDANCESETTINGS_GUIDANCEMODE_POSITION_PID) {
				positionPIDcontrol();
			} else {
				if(guidanceSettings.GuidanceMode == GUIDANCESETTINGS_GUIDANCEMODE_DUAL_LOOP) 
					updateVtolDesiredVelocity();
				else
					manualSetDesiredVelocity();
				
				updateVtolDesiredAttitude();
			}
		} else {
			// Be cleaner and get rid of global variables
			northIntegral = 0;
			eastIntegral = 0;
			downIntegral = 0;
			positionHoldLast = 0;
		}
		
		accel[0] = accel[1] = accel[2] = 0;
		accel_accum = 0;
	}
}

void updateVtolDesiredVelocity()
{
	GuidanceSettingsData guidanceSettings;
	PositionActualData positionActual;
	PositionDesiredData positionDesired;
	VelocityDesiredData velocityDesired;
	
	GuidanceSettingsGet(&guidanceSettings);
	PositionActualGet(&positionActual);
	PositionDesiredGet(&positionDesired);
	VelocityDesiredGet(&velocityDesired);
	
	// Note all distances in cm
	float dNorth = positionDesired.North - positionActual.North;
	float dEast = positionDesired.East - positionActual.East;
	float distance = sqrt(pow(dNorth, 2) + pow(dEast, 2));
	float heading = atan2f(dEast, dNorth);
	float groundspeed = bound(guidanceSettings.GroundVelocityP * distance, 
				  0, guidanceSettings.MaxGroundspeed);
	
	velocityDesired.North = groundspeed * cosf(heading);
	velocityDesired.East = groundspeed * sinf(heading);
	
	float dDown = positionDesired.Down - positionActual.Down;
	velocityDesired.Down = bound(guidanceSettings.VertVelocityP * dDown,
				     -guidanceSettings.MaxVerticalSpeed, 
				     guidanceSettings.MaxVerticalSpeed);
	
	VelocityDesiredSet(&velocityDesired);	
}

/**
 * Module thread, should not return.
 */
static void updateVtolDesiredAttitude()
{
	static portTickType lastSysTime;
	portTickType thisSysTime = xTaskGetTickCount();;
	float dT;

	VelocityDesiredData velocityDesired;
	VelocityActualData velocityActual;
	AttitudeDesiredData attitudeDesired;
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
	AttitudeDesiredGet(&attitudeDesired);
	VelocityDesiredGet(&velocityDesired);
	AttitudeActualGet(&attitudeActual);
	StabilizationSettingsGet(&stabSettings);
	NedAccelGet(&nedAccel);
	
	attitudeDesired.Yaw = 0;	// try and face north
	
	// Compute desired north command
	northError = velocityDesired.North - velocityActual.North;
	northIntegral = bound(northIntegral + northError * dT, 
			      -guidanceSettings.MaxVelIntegral,
			      guidanceSettings.MaxVelIntegral);
	northCommand = (northError * guidanceSettings.VelP +
			northIntegral * guidanceSettings.VelI -
			nedAccel.North * guidanceSettings.VelD);
	
	// Compute desired east command
	eastError = velocityDesired.East - velocityActual.East;
	eastIntegral = bound(eastIntegral + eastError * dT, 
			     -guidanceSettings.MaxVelIntegral,
			     guidanceSettings.MaxVelIntegral);
	eastCommand = (eastError * guidanceSettings.VelP + 
		       eastIntegral * guidanceSettings.VelI - 
		       nedAccel.East * guidanceSettings.VelD);
	
	// Compute desired down command
	downError = velocityDesired.Down - velocityActual.Down;
	downIntegral =	bound(downIntegral + downError * guidanceSettings.VelPIDUpdatePeriod, 
			      -guidanceSettings.MaxThrottleIntegral,
			      guidanceSettings.MaxThrottleIntegral);	
	downCommand = (downError * guidanceSettings.DownP +
		       downIntegral * guidanceSettings.DownI -
		       nedAccel.Down * guidanceSettings.DownD);
	
	attitudeDesired.Throttle = bound(downError * guidanceSettings.DownP + 
					 downIntegral * guidanceSettings.DownI -
					 nedAccel.Down * guidanceSettings.DownD, 0, 1);
	
	// Project the north and east command signals into the pitch and roll based on yaw.  For this to behave well the
	// craft should move similarly for 5 deg roll versus 5 deg pitch
	attitudeDesired.Pitch = bound(-northCommand * cosf(attitudeActual.Yaw * M_PI / 180) + 
				      -eastCommand * sinf(attitudeActual.Yaw * M_PI / 180),
				      -stabSettings.PitchMax, stabSettings.PitchMax);
	attitudeDesired.Roll = bound(-northCommand * sinf(attitudeActual.Yaw * M_PI / 180) + 
				     eastCommand * cosf(attitudeActual.Yaw * M_PI / 180),
				     -stabSettings.RollMax, stabSettings.RollMax);
	
	if(guidanceSettings.ThrottleControl == GUIDANCESETTINGS_THROTTLECONTROL_FALSE) {
		// For now override throttle with manual control.  Disable at your risk, quad goes to China.
		ManualControlCommandData manualControl;
		ManualControlCommandGet(&manualControl);
		attitudeDesired.Throttle = manualControl.Throttle;
	}
	
	AttitudeDesiredSet(&attitudeDesired);
}

static void manualSetDesiredVelocity() 
{
	ManualControlCommandData cmd;
	VelocityDesiredData velocityDesired;
	
	ManualControlCommandGet(&cmd);
	VelocityDesiredGet(&velocityDesired);

	GuidanceSettingsData guidanceSettings;
	GuidanceSettingsGet(&guidanceSettings);
	
	velocityDesired.North = -guidanceSettings.MaxGroundspeed * cmd.Pitch;
	velocityDesired.East = guidanceSettings.MaxGroundspeed * cmd.Roll;
	velocityDesired.Down = 0;
	
	VelocityDesiredSet(&velocityDesired);	
}

/** 
 * Control attitude with direct PID on position error
 */
static void positionPIDcontrol() 
{
	static portTickType lastSysTime;
	portTickType thisSysTime = xTaskGetTickCount();;
	float dT;
	
	AttitudeDesiredData attitudeDesired;
	AttitudeActualData attitudeActual;
	GuidanceSettingsData guidanceSettings;
	VelocityActualData velocityActual;
	StabilizationSettingsData stabSettings;
	SystemSettingsData systemSettings;
	PositionActualData positionActual;
	PositionDesiredData positionDesired;
	
	float northError;
	float northCommand;
	
	float eastError;
	float eastCommand;
	
	// Check how long since last update
	if(thisSysTime > lastSysTime) // reuse dt in case of wraparound
		dT = (thisSysTime - lastSysTime) / portTICK_RATE_MS / 1000.0f;		
	lastSysTime = thisSysTime;
	
	SystemSettingsGet(&systemSettings);
	GuidanceSettingsGet(&guidanceSettings);
	
	AttitudeDesiredGet(&attitudeDesired);
	AttitudeActualGet(&attitudeActual);
	StabilizationSettingsGet(&stabSettings);
	PositionActualGet(&positionActual);
	PositionDesiredGet(&positionDesired);
	VelocityActualGet(&velocityActual);
	
	attitudeDesired.Yaw = 0;	// try and face north
	
	// Yaw and pitch output from ground speed PID loop
	northError = positionDesired.North - positionActual.North;
	northIntegral = bound(northIntegral + northError * dT, 
			      -guidanceSettings.MaxVelIntegral,
			      guidanceSettings.MaxVelIntegral);
	northCommand = northError * guidanceSettings.VelP + 
			northIntegral * guidanceSettings.VelI -
			velocityActual.North * guidanceSettings.VelD;
	
	eastError = positionDesired.East - positionActual.East;
	eastIntegral = bound(eastIntegral + eastError * dT, 
			     -guidanceSettings.MaxVelIntegral,
			     guidanceSettings.MaxVelIntegral);
	eastCommand = eastError * guidanceSettings.VelP + 
			eastIntegral * guidanceSettings.VelI  - 
			velocityActual.East * guidanceSettings.VelD;
	
	// Project the north and east command signals into the pitch and roll based on yaw.  For this to behave well the
	// craft should move similarly for 5 deg roll versus 5 deg pitch
	attitudeDesired.Pitch = bound(-northCommand * cosf(attitudeActual.Yaw * M_PI / 180) + 
				      eastCommand * sinf(attitudeActual.Yaw * M_PI / 180),
				      -stabSettings.PitchMax, stabSettings.PitchMax);
	attitudeDesired.Roll = bound(-northCommand * sinf(attitudeActual.Yaw * M_PI / 180) + 
				     eastCommand * cosf(attitudeActual.Yaw * M_PI / 180),
				     -stabSettings.RollMax, stabSettings.RollMax);
		
	// For now override throttle with manual control.  Disable at your risk, quad goes to China.
	ManualControlCommandData manualControl;
	ManualControlCommandGet(&manualControl);
	attitudeDesired.Throttle = manualControl.Throttle;
	
	AttitudeDesiredSet(&attitudeDesired);
	
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
