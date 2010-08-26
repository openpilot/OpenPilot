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
#define PITCH_INTEGRAL_LIMIT 0.5
#define ROLL_INTEGRAL_LIMIT 0.5
#define YAW_INTEGRAL_LIMIT 0.5
#define DEG2RAD ( M_PI / 180.0 )

// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void stabilizationTask(void* parameters);
static float bound(float val, float min, float max);
static float angleDifference(float val);

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
	float pitchErrorGlobal, pitchErrorLastGlobal;
	float yawErrorGlobal, yawErrorLastGlobal;
	float pitchError, pitchErrorLast;
	float yawError, yawErrorLast;
	float rollError, rollErrorLast;
	float pitchDerivative;
	float yawDerivative;
	float rollDerivative;
	float pitchIntegral, pitchIntegralLimit;
	float yawIntegral, yawIntegralLimit;
	float rollIntegral, rollIntegralLimit;

	// Initialize
	pitchIntegral = 0.0;
	yawIntegral = 0.0;
	rollIntegral = 0.0;
	pitchErrorLastGlobal = 0.0;
	yawErrorLastGlobal = 0.0;
	rollErrorLast = 0.0;

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

		// For all three axis, calculate Error and ErrorLast - translating from global to local reference frame.

		// global pitch error
		if ( manualControl.FlightMode != MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO )
		{
			pitchErrorGlobal = angleDifference(
					bound(attitudeDesired.Pitch, -stabSettings.PitchMax, stabSettings.PitchMax) - attitudeActual.Pitch
				);
		} 
		else
		{
			// in AUTO mode, Stabilization is used to steer the plane freely,
			// while Navigation dictates the flight path, including maneuvers outside stable limits.
			pitchErrorGlobal = angleDifference(attitudeDesired.Pitch - attitudeActual.Pitch);
		}

		// global yaw error
		if ( systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_VTOL || manualControl.FlightMode == MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO )
		{
			// VTOLS consider yaw. AUTO mode considers YAW, too.
			yawErrorGlobal = angleDifference(attitudeDesired.Yaw - attitudeActual.Yaw);
		} else {
			// FIXED WING STABILIZATION however does not.
			yawErrorGlobal = 0;
		}
	
		// local pitch error
		pitchError = cos(DEG2RAD * attitudeActual.Roll) * pitchErrorGlobal + sin(DEG2RAD * attitudeActual.Roll) * yawErrorGlobal;
		// local roll error (no translation needed - always local)
		if ( manualControl.FlightMode != MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO )
		{
			rollError = angleDifference(
					bound(attitudeDesired.Roll, -stabSettings.RollMax, stabSettings.RollMax) - attitudeActual.Roll
				);
		}
		else
		{
			// in AUTO mode, Stabilization is used to steer the plane freely,
			// while Navigation dictates the flight path, including maneuvers outside stable limits.
			rollError = angleDifference(attitudeDesired.Roll - attitudeActual.Roll);
		}
		// local yaw error
		yawError = cos(DEG2RAD * attitudeActual.Roll) * yawErrorGlobal + sin(DEG2RAD * attitudeActual.Roll) * pitchErrorGlobal;
		
		// for the derivative, the local last errors are needed. Therefore global lasts are translated into local lasts

		// pitch last
		pitchErrorLast = cos(DEG2RAD * attitudeActual.Roll) * pitchErrorLastGlobal + sin(DEG2RAD * attitudeActual.Roll) * yawErrorLastGlobal;

		// yaw last
		yawErrorLast = cos(DEG2RAD * attitudeActual.Roll) * yawErrorLastGlobal + sin(DEG2RAD * attitudeActual.Roll) * pitchErrorLastGlobal;
		
		// global last variables are no longer needed 
		pitchErrorLastGlobal = pitchErrorGlobal;
		yawErrorLastGlobal = yawErrorGlobal;
		
		// local Pitch stabilization control loop
		pitchDerivative = pitchError - pitchErrorLast;
		pitchIntegralLimit = PITCH_INTEGRAL_LIMIT / stabSettings.PitchKi;
		pitchIntegral = bound(pitchIntegral+pitchError*stabSettings.UpdatePeriod, -pitchIntegralLimit, pitchIntegralLimit);
		actuatorDesired.Pitch = stabSettings.PitchKp*pitchError + stabSettings.PitchKi*pitchIntegral + stabSettings.PitchKd*pitchDerivative;
		actuatorDesired.Pitch = bound(actuatorDesired.Pitch, -1.0, 1.0);

		// local Roll stabilization control loop
		rollDerivative = rollError - rollErrorLast;
		rollIntegralLimit = ROLL_INTEGRAL_LIMIT / stabSettings.RollKi;
		rollIntegral = bound(rollIntegral+rollError*stabSettings.UpdatePeriod, -rollIntegralLimit, rollIntegralLimit);
		actuatorDesired.Roll = stabSettings.RollKp*rollError + stabSettings.RollKi*rollIntegral + stabSettings.RollKd*rollDerivative;
		actuatorDesired.Roll = bound(actuatorDesired.Roll, -1.0, 1.0);
		rollErrorLast = rollError;


		// local Yaw stabilization control loop (only enabled on VTOL airframes)
		if (( systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_VTOL )||( systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_HELICP))
		{
			yawDerivative = yawError - yawErrorLast;
			yawIntegralLimit = YAW_INTEGRAL_LIMIT / stabSettings.YawKi;
			yawIntegral = bound(yawIntegral+yawError*stabSettings.UpdatePeriod, -yawIntegralLimit, yawIntegralLimit);
			actuatorDesired.Yaw = stabSettings.YawKp*yawError + stabSettings.YawKi*yawIntegral + stabSettings.YawKd*yawDerivative;;
			actuatorDesired.Yaw = bound(actuatorDesired.Yaw, -1.0, 1.0);
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
			yawIntegral = 0.0;
			rollIntegral = 0.0;
			pitchErrorLastGlobal = 0.0;
			yawErrorLastGlobal = 0.0;
			rollErrorLast = 0.0;
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
 * Fix result of angular differences
 */
static float angleDifference(float val)
{
	while ( val < -180.0 )
	{
		val += 360.0;
	}
	while ( val > 180.0 )
	{
		val -= 360.0;
	}
	return val;
}

/** 
  * @}
  * @}
  */
