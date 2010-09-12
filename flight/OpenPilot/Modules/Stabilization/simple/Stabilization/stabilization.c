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
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+4)

// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void stabilizationTask(void* parameters);
static float bound(float val, float min, float max);

/**
 * Module initialization
 */
int32_t StabilizationInitialize()
{
	// Initialize variables

	// Start main task
	xTaskCreate(stabilizationTask, (signed char*)"Stabilization", STACK_SIZE, NULL, TASK_PRIORITY, &taskHandle);

	return 0;
}

/**
 * Module task
 */
static void stabilizationTask(void* parameters)
{
	StabilizationSettingsData stabSettings;
	ActuatorDesiredData actuatorDesired;
	AttitudeDesiredData attitudeDesired;
	AttitudeActualData attitudeActual;
	ManualControlCommandData manualControl;
	SystemSettingsData systemSettings;
	portTickType lastSysTime;
	float pitchError, pitchErrorLast;
	float rollError, rollErrorLast;
	float yawError, yawErrorLast;
	float pitchDerivative;
	float rollDerivative;
	float yawDerivative;
	float pitchIntegral;
	float rollIntegral;
	float yawIntegral;

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
		// Read settings and other objects
		StabilizationSettingsGet(&stabSettings);
		SystemSettingsGet(&systemSettings);
		ManualControlCommandGet(&manualControl);
		AttitudeDesiredGet(&attitudeDesired);
		AttitudeActualGet(&attitudeActual);

		// Pitch stabilization control loop
		pitchError = bound(attitudeDesired.Pitch, -stabSettings.PitchMax, stabSettings.PitchMax) - attitudeActual.Pitch;
		pitchDerivative = (pitchError - pitchErrorLast) / stabSettings.UpdatePeriod;
		pitchIntegral = bound(pitchIntegral+pitchError*stabSettings.UpdatePeriod, -stabSettings.PitchIntegralLimit, stabSettings.PitchIntegralLimit);
		actuatorDesired.Pitch = stabSettings.PitchKp*pitchError + stabSettings.PitchKi*pitchIntegral + stabSettings.PitchKd*pitchDerivative;
		actuatorDesired.Pitch = bound(actuatorDesired.Pitch, -1.0, 1.0);
		pitchErrorLast = pitchError;

		// Roll stabilization control loop
		rollError = bound(attitudeDesired.Roll, -stabSettings.RollMax, stabSettings.RollMax) - attitudeActual.Roll;
		rollDerivative = (rollError - rollErrorLast) / stabSettings.UpdatePeriod;
		rollIntegral = bound(rollIntegral+rollError*stabSettings.UpdatePeriod, -stabSettings.RollIntegralLimit, stabSettings.RollIntegralLimit);
		actuatorDesired.Roll = stabSettings.RollKp*rollError + stabSettings.RollKi*rollIntegral + stabSettings.RollKd*rollDerivative;
		actuatorDesired.Roll = bound(actuatorDesired.Roll, -1.0, 1.0);
		rollErrorLast = rollError;

		// Yaw stabilization control loop (only enabled on VTOL airframes)
		if (( systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_VTOL )||( systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_HELICP))
		{
			yawError = attitudeDesired.Yaw - attitudeActual.Yaw;
			//this should make it take the quickest path to reach the desired yaw
			if (yawError>180.0)yawError -= 360;
			if (yawError<-180.0)yawError += 360;
			yawDerivative = (yawError - yawErrorLast) / stabSettings.UpdatePeriod;
			yawIntegral = bound(yawIntegral+yawError*stabSettings.UpdatePeriod, -stabSettings.YawIntegralLimit, stabSettings.YawIntegralLimit);
			actuatorDesired.Yaw = stabSettings.YawKp*yawError + stabSettings.YawKi*yawIntegral + stabSettings.YawKd*yawDerivative;;
			actuatorDesired.Yaw = bound(actuatorDesired.Yaw, -1.0, 1.0);
			yawErrorLast = yawError;
		}
		else
		{
			actuatorDesired.Yaw = 0.0;
		}

		// Setup throttle
		actuatorDesired.Throttle = bound(attitudeDesired.Throttle, 0.0, stabSettings.ThrottleMax);

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

		// Wait until next update
		vTaskDelayUntil(&lastSysTime, stabSettings.UpdatePeriod / portTICK_RATE_MS );
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
