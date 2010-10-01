/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup StabilizationModule Stabilization Module
 * @brief Stabilization PID loops in an airframe type independent manner
 * @note This object updates the @ref ActuatorDesired "Actuator Desired" based on the 
 * PID loops on the @ref AttitudeDesired "Attitude Desired" and @ref AttitudeActual "Attitude Actual"
 * @{ 
 *
 * @file       stabilization.c
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
#include "stabilization.h"
#include "stabilizationsettings.h"
#include "actuatordesired.h"
#include "attitudedesired.h"
#include "attitudeactual.h"
#include "manualcontrolcommand.h"
#include "systemsettings.h"

// Private constants
#define MAX_QUEUE_SIZE 2
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+4)
#define FAILSAFE_TIMEOUT_MS 100

// Private types

// Private variables
static xQueueHandle queue;
static xTaskHandle taskHandle;

// Private functions
static void stabilizationTask(void* parameters);
static float bound(float val, float min, float max);

/**
 * Module initialization
 */
int32_t StabilizationInitialize()
{
	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));
	
	// Listen for AttitudeActual updates.
	AttitudeActualConnectQueue(queue);
	
	// Start main task
	xTaskCreate(stabilizationTask, (signed char*)"Stabilization", STACK_SIZE, NULL, TASK_PRIORITY, &taskHandle);

	return 0;
}
float dT = 1;

/**
 * Module task
 */
static void stabilizationTask(void* parameters)
{
	UAVObjEvent ev;

	StabilizationSettingsData stabSettings;
	ActuatorDesiredData actuatorDesired;
	AttitudeDesiredData attitudeDesired;
	AttitudeActualData attitudeActual;
	ManualControlCommandData manualControl;
	SystemSettingsData systemSettings;
	portTickType lastSysTime;
	portTickType thisSysTime;
	float pitchError, pitchErrorLast;
	float rollError, rollErrorLast;
	float yawError, yawErrorLast;
	float pitchDerivative;
	float rollDerivative;
	float yawDerivative;
	float pitchIntegral;
	float rollIntegral;
	float yawIntegral;
	float yawPrevious;
	float yawChange;

	// Initialize
	pitchIntegral = 0.0;
	rollIntegral = 0.0;
	yawIntegral = 0.0;
	pitchErrorLast = 0.0;
	rollErrorLast = 0.0;
	yawErrorLast = 0.0;

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1)
	{
		// Wait until the ActuatorDesired object is updated, if a timeout then go to failsafe
                if ( xQueueReceive(queue, &ev, FAILSAFE_TIMEOUT_MS / portTICK_RATE_MS) != pdTRUE )
                {
                        AlarmsSet(SYSTEMALARMS_ALARM_STABILIZATION,SYSTEMALARMS_ALARM_WARNING);
                }
		
		// Check how long since last update
		thisSysTime = xTaskGetTickCount();
		if(thisSysTime > lastSysTime) // reuse dt in case of wraparound
			dT = (thisSysTime - lastSysTime) / portTICK_RATE_MS / 1000.0f;		
		lastSysTime = thisSysTime;
		
		// Read settings and other objects
		StabilizationSettingsGet(&stabSettings);
		SystemSettingsGet(&systemSettings);
		ManualControlCommandGet(&manualControl);
		AttitudeDesiredGet(&attitudeDesired);
		AttitudeActualGet(&attitudeActual);

		// Pitch stabilization control loop
		pitchError = attitudeDesired.Pitch - attitudeActual.Pitch;
		pitchDerivative = (pitchError - pitchErrorLast) / dT;
		pitchIntegral = bound(pitchIntegral + pitchError * dT, -stabSettings.PitchIntegralLimit, stabSettings.PitchIntegralLimit);
		actuatorDesired.Pitch = stabSettings.PitchKp*pitchError + stabSettings.PitchKi*pitchIntegral + stabSettings.PitchKd*pitchDerivative;
		actuatorDesired.Pitch = bound(actuatorDesired.Pitch, -1.0, 1.0);
		pitchErrorLast = pitchError;

		// Roll stabilization control loop
		rollError = attitudeDesired.Roll - attitudeActual.Roll;
		rollDerivative = (rollError - rollErrorLast) / dT;
		rollIntegral = bound(rollIntegral + rollError * dT, -stabSettings.RollIntegralLimit, stabSettings.RollIntegralLimit);
		actuatorDesired.Roll = stabSettings.RollKp*rollError + stabSettings.RollKi*rollIntegral + stabSettings.RollKd*rollDerivative;
		actuatorDesired.Roll = bound(actuatorDesired.Roll, -1.0, 1.0);
		rollErrorLast = rollError;

		// Yaw stabilization control loop (only enabled on VTOL airframes)
		if (( systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_VTOL )||( systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_HELICP))
		{
			if(stabSettings.YawMode == STABILIZATIONSETTINGS_YAWMODE_RATE) {  // rate stabilization on yaw
				yawChange = (attitudeActual.Yaw - yawPrevious) / dT;
				yawPrevious = attitudeActual.Yaw;
				yawError = bound(attitudeDesired.Yaw, -stabSettings.YawMax, stabSettings.YawMax) - yawChange;
			} else { // heading stabilization
				yawError = attitudeDesired.Yaw - attitudeActual.Yaw;
			}

			//this should make it take the quickest path to reach the desired yaw
			if (yawError>180.0)yawError -= 360;
			if (yawError<-180.0)yawError += 360;
			yawDerivative = (yawError - yawErrorLast) / dT;
			yawIntegral = bound(yawIntegral + yawError * dT, -stabSettings.YawIntegralLimit, stabSettings.YawIntegralLimit);
			actuatorDesired.Yaw = stabSettings.YawKp*yawError + stabSettings.YawKi*yawIntegral + stabSettings.YawKd*yawDerivative;;
			actuatorDesired.Yaw = bound(actuatorDesired.Yaw, -1.0, 1.0);
			yawErrorLast = yawError;
		}
		else
		{
			actuatorDesired.Yaw = 0.0;
		}

		// Setup throttle
		actuatorDesired.Throttle = bound(attitudeDesired.Throttle, stabSettings.ThrottleMin, stabSettings.ThrottleMax);

		// Write actuator desired (if not in manual mode)
		if ( manualControl.FlightMode != MANUALCONTROLCOMMAND_FLIGHTMODE_MANUAL )
		{
			ActuatorDesiredSet(&actuatorDesired);
		}
		else
		{
			pitchIntegral = 0.0;
			rollIntegral = 0.0;
			yawIntegral = 0.0;
			pitchErrorLast = 0.0;
			rollErrorLast = 0.0;
			yawErrorLast = 0.0;
		}

		// Clear alarms
		AlarmsClear(SYSTEMALARMS_ALARM_STABILIZATION);
	}
}

/**
 * Bound input value between limits
 */
static float bound(float val, float min, float max)
{
	if ( val < min )
	{
		val = min;
	}
	else if ( val > max )
	{
		val = max;
	}
	return val;
}

/** 
  * @}
  * @}
  */
