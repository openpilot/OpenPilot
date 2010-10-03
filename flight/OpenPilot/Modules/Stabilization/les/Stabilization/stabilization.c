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
#include "lesstabilizationsettings.h"
#include "actuatordesired.h"
#include "attitudedesired.h"
#include "attitudeactual.h"
#include "attituderaw.h"
#include "manualcontrolcommand.h"
#include "systemsettings.h"
#include "ahrssettings.h"


// Private constants
#define MAX_QUEUE_SIZE 2
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+4)
#define FAILSAFE_TIMEOUT_MS 100

enum {PID_RATE_ROLL, PID_RATE_PITCH, PID_RATE_YAW, PID_ROLL, PID_PITCH, PID_YAW, PID_MAX};

enum {ROLL,PITCH,YAW,MAX_AXES};


// Private types
typedef struct {
	float p;
	float i;
	float d;
	float iLim;
	float iAccumulator;
	float lastErr;
} pid_type;

// Private variables
static xTaskHandle taskHandle;
static LesStabilizationSettingsData settings;
static xQueueHandle queue;
float dT = 1;
pid_type pids[PID_MAX];



// Private functions
static void stabilizationTask(void* parameters);
static float ApplyPid(pid_type * pid, const float desired, const float actual, const bool angular);
static float bound(float val);
static void ZeroPids(void);
static void SettingsUpdatedCb(UAVObjEvent * ev);

/**
 * Module initialization
 */
int32_t StabilizationInitialize()
{
	// Initialize variables

	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	// Listen for updates.
	AttitudeActualConnectQueue(queue);
	AttitudeRawConnectQueue(queue);

	LesStabilizationSettingsConnectCallback(SettingsUpdatedCb);
	SettingsUpdatedCb(LesStabilizationSettingsHandle());
	// Start main task
	xTaskCreate(stabilizationTask, (signed char*)"Stabilization", STACK_SIZE, NULL, TASK_PRIORITY, &taskHandle);

	return 0;
}

/**
 * Module task
 */
static void stabilizationTask(void* parameters)
{
	portTickType lastSysTime;
	portTickType thisSysTime;
	UAVObjEvent ev;


	ActuatorDesiredData actuatorDesired;
	AttitudeDesiredData attitudeDesired;
	AttitudeActualData attitudeActual;
	AttitudeRawData attitudeRaw;
	SystemSettingsData systemSettings;
	ManualControlCommandData manualControl;

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	ZeroPids();
	while(1) {
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

		ManualControlCommandGet(&manualControl);
		AttitudeDesiredGet(&attitudeDesired);
		AttitudeActualGet(&attitudeActual);
		AttitudeRawGet(&attitudeRaw);
		SystemSettingsGet(&systemSettings);


		float rates[MAX_AXES]= {0,0,0};
		rates[ROLL]  = ApplyPid(&pids[PID_ROLL],  attitudeDesired.Roll,  attitudeActual.Roll ,false);
		rates[PITCH] = ApplyPid(&pids[PID_PITCH], attitudeDesired.Pitch, attitudeActual.Pitch,false);
		if(settings.YawMode == LESSTABILIZATIONSETTINGS_YAWMODE_RATE) {  // rate stabilization on yaw
			rates[YAW] = manualControl.Yaw * settings.ManualYawRate;
		}else{
			rates[YAW] = ApplyPid(&pids[PID_YAW], attitudeDesired.Yaw, attitudeActual.Yaw,true);
		}

		for(int ct=0; ct< MAX_AXES; ct++)
		{
			if(fabs(rates[ct]) > settings.MaximumRate[ct])
			{
				if(rates[ct] > 0)
				{
					rates[ct] = settings.MaximumRate[ct];
				}else
				{
					rates[ct] = -settings.MaximumRate[ct];
				}

			}
		}

		float commands[MAX_AXES];
		commands[ROLL] = ApplyPid(&pids[PID_RATE_ROLL], attitudeRaw.gyros_filtered[ROLL], rates[ROLL],false);
		commands[PITCH] = ApplyPid(&pids[PID_RATE_PITCH], attitudeRaw.gyros_filtered[PITCH], rates[PITCH],false);
		commands[YAW] = ApplyPid(&pids[PID_RATE_YAW], attitudeRaw.gyros_filtered[YAW], rates[YAW],false);


		// On fixed wing we don't try to stabilizew yaw
		if ( systemSettings.AirframeType < SYSTEMSETTINGS_AIRFRAMETYPE_VTOL)
		{
			commands[YAW] = -manualControl.Yaw;
		}

		actuatorDesired.Pitch = bound(-commands[PITCH]);
		actuatorDesired.Roll = bound(-commands[ROLL]);
		actuatorDesired.Yaw = bound(-commands[YAW]);
		// Setup throttle
		actuatorDesired.Throttle = attitudeDesired.Throttle;

		// Save dT
		actuatorDesired.UpdateTime = dT * 1000;

		// Write actuator desired (if not in manual mode)
		if ( manualControl.FlightMode != MANUALCONTROLCOMMAND_FLIGHTMODE_MANUAL )
		{
			ActuatorDesiredSet(&actuatorDesired);
		}
		else
		{
			ZeroPids();
		}

		// Clear alarms
		AlarmsClear(SYSTEMALARMS_ALARM_STABILIZATION);
	}
}

float ApplyPid(pid_type * pid, const float desired, const float actual, const bool angular)
{
	float err = desired - actual;
	if(angular) //take shortest route to desired position
	{
		if(err > 180)
		{
			err -= 360;
		}
		if(err < -180)
		{
			err += 360;
		}
	}

	float diff = (err - pid->lastErr);
	pid->lastErr = err;
	pid->iAccumulator += err * pid->i * dT;
	if(fabs(pid->iAccumulator) > pid->iLim) {
		if(pid->iAccumulator >0) {
			pid->iAccumulator = pid->iLim;
		} else {
			pid->iAccumulator = -pid->iLim;
		}
	}
	return ((err * pid->p) + pid->iAccumulator + (diff * pid->d / dT));
}


static void ZeroPids(void)
{
	for(int ct = 0; ct < PID_MAX; ct++) {
		pids[ct].iAccumulator = 0;
		pids[ct].lastErr = 0;
	}
}


/**
 * Bound input value between limits
 */
static float bound(float val)
{
	if(val < -1) {
		val = -1;
	} else if(val > 1) {
		val = 1;
	}
	return val;
}


static void SettingsUpdatedCb(UAVObjEvent * ev)
{
	memset(pids,0,sizeof (pid_type) * PID_MAX);
	LesStabilizationSettingsGet(&settings);
	pids[PID_RATE_ROLL].p = settings.RollRateP;
	pids[PID_RATE_PITCH].p = settings.PitchRateP;
	pids[PID_RATE_YAW].p = settings.YawRatePI[LESSTABILIZATIONSETTINGS_YAWRATEPI_KP];
	pids[PID_RATE_YAW].i = settings.YawRatePI[LESSTABILIZATIONSETTINGS_YAWRATEPI_KI];
	pids[PID_RATE_YAW].iLim = settings.YawRatePI[LESSTABILIZATIONSETTINGS_YAWRATEPI_ILIMIT];


	float * data = settings.RollPI;
	for(int pid=PID_ROLL; pid < PID_MAX; pid++)
	{
		pids[pid].p = *data++;
		pids[pid].i = *data++;
		pids[pid].iLim = *data++;
	}
}


/**
  * @}
  * @}
  */
