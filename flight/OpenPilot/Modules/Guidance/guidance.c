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
#include "attitudeactual.h"
#include "attitudedesired.h"
#include "positiondesired.h"	// object that will be updated by the module
#include "positionactual.h"
#include "manualcontrolcommand.h"
#include "stabilizationsettings.h"
#include "systemsettings.h"
#include "velocitydesired.h"
#include "velocityactual.h"

// Private constants
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
// Private types

// Private variables
static xTaskHandle guidanceTaskHandle;

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
	// Start main task
	xTaskCreate(guidanceTask, (signed char *)"Guidance", STACK_SIZE, NULL, TASK_PRIORITY, &guidanceTaskHandle);

	return 0;
}

static float northIntegral = 0;
static float northErrorLast = 0;
static float eastIntegral = 0;
static float eastErrorLast = 0;
static float downIntegral = 0;
static float downErrorLast = 0;

/**
 * Module thread, should not return.
 */
static void guidanceTask(void *parameters)
{
	SystemSettingsData systemSettings;
	GuidanceSettingsData guidanceSettings;
	ManualControlCommandData manualControl;

	portTickType lastSysTime;

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1) {
		ManualControlCommandGet(&manualControl);
		SystemSettingsGet(&systemSettings);
		GuidanceSettingsGet(&guidanceSettings);
		
		if ((manualControl.FlightMode == MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO) &&
		    ((systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_VTOL) ||
		     (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_QUADP) ||
		     (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_QUADX)))
		{
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
			northErrorLast = 0;
			eastIntegral = 0;
			eastErrorLast = 0;
			downIntegral = 0;
			downErrorLast = 0;
		}			
		vTaskDelayUntil(&lastSysTime, guidanceSettings.VelUpdatePeriod / portTICK_RATE_MS);
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
	GuidanceSettingsData guidanceSettings;
	StabilizationSettingsData stabSettings;
	SystemSettingsData systemSettings;

	float northError;
	float northDerivative;
	float northCommand;
	
	float eastError;
	float eastDerivative;
	float eastCommand;

	float downError;
	float downDerivative;
	
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
	
	attitudeDesired.Yaw = 0;	// try and face north
	
	// Yaw and pitch output from ground speed PID loop
	northError = velocityDesired.North - velocityActual.North;
	northDerivative = (northError - northErrorLast) / dT;
	northIntegral =
	bound(northIntegral + northError * dT, -guidanceSettings.MaxVelIntegral,
	      guidanceSettings.MaxVelIntegral);
	northErrorLast = northError;
	northCommand =
	northError * guidanceSettings.VelP + northDerivative * guidanceSettings.VelD + northIntegral * guidanceSettings.VelI;
	
	eastError = velocityDesired.East - velocityActual.East;
	eastDerivative = (eastError - eastErrorLast) / dT;
	eastIntegral = bound(eastIntegral + eastError * dT, 
			     -guidanceSettings.MaxVelIntegral,
			     guidanceSettings.MaxVelIntegral);
	eastErrorLast = eastError;
	eastCommand = eastError * guidanceSettings.VelP + eastDerivative * guidanceSettings.VelD + eastIntegral * guidanceSettings.VelI;
	
	// Project the north and east command signals into the pitch and roll based on yaw.  For this to behave well the
	// craft should move similarly for 5 deg roll versus 5 deg pitch
	attitudeDesired.Pitch = bound(-northCommand * cosf(attitudeActual.Yaw * M_PI / 180) + 
				      eastCommand * sinf(attitudeActual.Yaw * M_PI / 180),
				      -stabSettings.PitchMax, stabSettings.PitchMax);
	attitudeDesired.Roll = bound(-northCommand * sinf(attitudeActual.Yaw * M_PI / 180) + 
				     eastCommand * cosf(attitudeActual.Yaw * M_PI / 180),
				     -stabSettings.RollMax, stabSettings.RollMax);
	
	downError = velocityDesired.Down - velocityActual.Down;
	downDerivative = (downError - downErrorLast) / guidanceSettings.VelPIDUpdatePeriod;
	downIntegral =	bound(downIntegral + downError * guidanceSettings.VelPIDUpdatePeriod, 
			      -guidanceSettings.MaxThrottleIntegral,
			      guidanceSettings.MaxThrottleIntegral);
	downErrorLast = downError;
	attitudeDesired.Throttle = bound(downError * guidanceSettings.DownP + downDerivative * guidanceSettings.DownD +
					 downIntegral * guidanceSettings.DownI, 0, 1);
	
	// For now override throttle with manual control.  Disable at your risk, quad goes to China.
	ManualControlCommandData manualControl;
	ManualControlCommandGet(&manualControl);
	attitudeDesired.Throttle = manualControl.Throttle;
	
	AttitudeDesiredSet(&attitudeDesired);
}

static void manualSetDesiredVelocity() 
{
	ManualControlCommandData cmd;
	VelocityDesiredData velocityDesired;
	
	ManualControlCommandGet(&cmd);
	VelocityDesiredGet(&velocityDesired);
	
	velocityDesired.North = -200 * cmd.Pitch;
	velocityDesired.East = 200 * cmd.Roll;
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
	StabilizationSettingsData stabSettings;
	SystemSettingsData systemSettings;
	PositionActualData positionActual;
	PositionDesiredData positionDesired;
	
	float northError;
	float northDerivative;
	float northCommand;
	
	float eastError;
	float eastDerivative;
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
	
	attitudeDesired.Yaw = 0;	// try and face north
	
	// Yaw and pitch output from ground speed PID loop
	northError = positionDesired.North - positionActual.North;
	northDerivative = (northError - northErrorLast) / dT;
	northIntegral =
	bound(northIntegral + northError * dT, -guidanceSettings.MaxVelIntegral,
	      guidanceSettings.MaxVelIntegral);
	northErrorLast = northError;
	northCommand =
	northError * guidanceSettings.VelP + northDerivative * guidanceSettings.VelD + northIntegral * guidanceSettings.VelI;
	
	eastError = positionDesired.East - positionActual.East;
	eastDerivative = (eastError - eastErrorLast) / dT;
	eastIntegral = bound(eastIntegral + eastError * dT, 
			     -guidanceSettings.MaxVelIntegral,
			     guidanceSettings.MaxVelIntegral);
	eastErrorLast = eastError;
	eastCommand = eastError * guidanceSettings.VelP + eastDerivative * guidanceSettings.VelD + eastIntegral * guidanceSettings.VelI;
	
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
