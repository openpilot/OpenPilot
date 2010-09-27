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
static xTaskHandle velocityPIDTaskHandle;

// Private functions
static void guidanceTask(void *parameters);
static void velocityPIDTask(void *parameters);
static float bound(float val, float min, float max);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t GuidanceInitialize()
{
	// Start main task
	xTaskCreate(guidanceTask, (signed char *)"Guidance", STACK_SIZE, NULL, TASK_PRIORITY, &guidanceTaskHandle);
	xTaskCreate(velocityPIDTask, (signed char *)"VelocityPID", STACK_SIZE, NULL, TASK_PRIORITY, &velocityPIDTaskHandle);

	return 0;
}

/**
 * Module thread, should not return.
 */
static void guidanceTask(void *parameters)
{
	SystemSettingsData systemSettings;
	GuidanceSettingsData guidanceSettings;
	ManualControlCommandData manualControl;
	PositionActualData positionActual;
	PositionDesiredData positionDesired;
	VelocityDesiredData velocityDesired;

	portTickType lastSysTime;

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1) {
		ManualControlCommandGet(&manualControl);
		SystemSettingsGet(&systemSettings);
		GuidanceSettingsGet(&guidanceSettings);

		if ((manualControl.FlightMode == MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO) &&
		    (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_VTOL)) {

			PositionActualGet(&positionActual);
			PositionDesiredGet(&positionDesired);

			// Note all distances in cm
			float dNorth = positionDesired.North - positionActual.North;
			float dEast = positionDesired.East - positionActual.East;
			float distance = sqrt(pow(dNorth, 2) + pow(dEast, 2));
			float groundspeed = guidanceSettings.GroundVelocityP * distance;	//bound(guidanceSettings.GroundVelocityP * distance, 0, guidanceSettings.MaxGroundspeed);
			float heading = atan2f(dEast, dNorth);

			velocityDesired.North = groundspeed * cosf(heading);
			velocityDesired.East = groundspeed * sinf(heading);

			float dDown = positionDesired.Down - positionActual.Down;
			velocityDesired.Down =
			    bound(guidanceSettings.VertVelocityP * dDown, -guidanceSettings.MaxVerticalSpeed, guidanceSettings.MaxVerticalSpeed);

			VelocityDesiredSet(&velocityDesired);

		}
		vTaskDelayUntil(&lastSysTime, guidanceSettings.VelUpdatePeriod / portTICK_RATE_MS);
	}
}

/**
 * Module thread, should not return.
 */
static void velocityPIDTask(void *parameters)
{
	portTickType lastSysTime;
	VelocityDesiredData velocityDesired;
	VelocityActualData velocityActual;
	AttitudeDesiredData attitudeDesired;
	AttitudeActualData attitudeActual;
	GuidanceSettingsData guidanceSettings;
	StabilizationSettingsData stabSettings;
	SystemSettingsData systemSettings;
	ManualControlCommandData manualControl;

	float northError;
	float northDerivative;
	float northIntegral;
	float northErrorLast = 0;
	float northCommand;
	float eastError;
	float eastDerivative;
	float eastIntegral = 0;
	float eastErrorLast = 0;
	float eastCommand;
	float downError;
	float downDerivative;
	float downIntegral = 0;
	float downErrorLast = 0;

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1) {
		ManualControlCommandGet(&manualControl);
		SystemSettingsGet(&systemSettings);
		GuidanceSettingsGet(&guidanceSettings);
		if ((manualControl.FlightMode == MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO) &&
		    (systemSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_VTOL)) {
			VelocityActualGet(&velocityActual);
			VelocityDesiredGet(&velocityDesired);
			AttitudeDesiredGet(&attitudeDesired);
			VelocityDesiredGet(&velocityDesired);
			AttitudeActualGet(&attitudeActual);
			StabilizationSettingsGet(&stabSettings);

			attitudeDesired.Yaw = 0;	// try and face north

			// Yaw and pitch output from ground speed PID loop
			northError = velocityDesired.North - velocityActual.North;
			northDerivative = (northError - northErrorLast) / guidanceSettings.VelPIDUpdatePeriod;
			northIntegral =
			    bound(northIntegral + northError * guidanceSettings.VelPIDUpdatePeriod, -guidanceSettings.MaxVelIntegral,
				  guidanceSettings.MaxVelIntegral);
			northErrorLast = northError;
			northCommand =
			    northError * guidanceSettings.VelP + northDerivative * guidanceSettings.VelD + northIntegral * guidanceSettings.VelI;

			eastError = velocityDesired.East - velocityActual.East;
			eastDerivative = (eastError - eastErrorLast) / guidanceSettings.VelPIDUpdatePeriod;
			eastIntegral =
			    bound(eastIntegral + eastError * guidanceSettings.VelPIDUpdatePeriod, -guidanceSettings.MaxVelIntegral,
				  guidanceSettings.MaxVelIntegral);
			eastErrorLast = eastError;
			eastCommand =
			    eastError * guidanceSettings.VelP + eastDerivative * guidanceSettings.VelD + eastIntegral * guidanceSettings.VelI;

			// Project the north and east command signals into the pitch and roll based on yaw.  For this to behave well the
			// craft should move similarly for 5 deg roll versus 5 deg pitch
			attitudeDesired.Pitch =
			    bound(-northCommand * cosf(attitudeActual.Yaw * M_PI / 180) + eastCommand * sinf(attitudeActual.Yaw * M_PI / 180),
				  -stabSettings.PitchMax, stabSettings.PitchMax);
			attitudeDesired.Roll =
			    bound(-northCommand * sinf(attitudeActual.Yaw * M_PI / 180) + eastCommand * cosf(attitudeActual.Yaw * M_PI / 180),
				  -stabSettings.RollMax, stabSettings.RollMax);

			downError = velocityDesired.Down - velocityActual.Down;
			downDerivative = (downError - downErrorLast) / guidanceSettings.VelPIDUpdatePeriod;
			downIntegral =
			    bound(downIntegral + downError * guidanceSettings.VelPIDUpdatePeriod, -guidanceSettings.MaxThrottleIntegral,
				  guidanceSettings.MaxThrottleIntegral);
			downErrorLast = downError;
			attitudeDesired.Throttle =
			    bound(downError * guidanceSettings.DownP + downDerivative * guidanceSettings.DownD +
				  downIntegral * guidanceSettings.DownI, 0, 1);

			AttitudeDesiredSet(&attitudeDesired);

		}
		vTaskDelayUntil(&lastSysTime, guidanceSettings.VelPIDUpdatePeriod / portTICK_RATE_MS);
	}
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
