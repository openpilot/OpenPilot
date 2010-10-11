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
static StabilizationSettingsData settings;
static xQueueHandle queue;
float dT = 1;
pid_type pids[PID_MAX];



// Private functions
static void stabilizationTask(void* parameters);
static float ApplyPid(pid_type * pid, const float desired, const float actual, const uint8_t angular);
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

	StabilizationSettingsConnectCallback(SettingsUpdatedCb);
	SettingsUpdatedCb(StabilizationSettingsHandle());
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

	SettingsUpdatedCb((UAVObjEvent *) NULL);

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


		float *manualAxis = &manualControl.Roll;
		float *attitudeDesiredAxis = &attitudeDesired.Roll;
		float *attitudeActualAxis = &attitudeActual.Roll;
		float *actuatorDesiredAxis = &actuatorDesired.Roll;

		//Calculate desired rate
		float rates[MAX_AXES]= {0,0,0};
		for(int8_t ct=0; ct< MAX_AXES; ct++)
		{
			switch(manualControl.StabilizationSettings[ct])
			{
			case MANUALCONTROLCOMMAND_STABILIZATIONSETTINGS_RATE:
				rates[ct] = manualAxis[ct] * settings.ManualRate[ct];
				break;

			case MANUALCONTROLCOMMAND_STABILIZATIONSETTINGS_ATTITUDE:
				rates[ct] = ApplyPid(&pids[PID_ROLL + ct],  attitudeDesiredAxis[ct],  attitudeActualAxis[ct], 1);
				break;
			}
		}

		uint8_t shouldUpdate = 0;
		ActuatorDesiredGet(&actuatorDesired);
		//Calculate desired command
		for(int8_t ct=0; ct< MAX_AXES; ct++)
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
			switch(manualControl.StabilizationSettings[ct])
			{
			case MANUALCONTROLCOMMAND_STABILIZATIONSETTINGS_RATE:
			case MANUALCONTROLCOMMAND_STABILIZATIONSETTINGS_ATTITUDE:
				{
					float command = ApplyPid(&pids[PID_RATE_ROLL + ct],  rates[ct],  attitudeRaw.gyros_filtered[ct], 0);
					actuatorDesiredAxis[ct] = bound(command);
					shouldUpdate = 1;
					break;
				}
			}
		}

		// Save dT
		actuatorDesired.UpdateTime = dT * 1000;

		if(manualControl.FlightMode == MANUALCONTROLCOMMAND_FLIGHTMODE_MANUAL)
		{
			shouldUpdate = 0;
		}


		if(shouldUpdate)
		{
			actuatorDesired.Throttle = attitudeDesired.Throttle;
			ActuatorDesiredSet(&actuatorDesired);
		}

		if(manualControl.Armed == MANUALCONTROLCOMMAND_ARMED_FALSE ||
			!shouldUpdate || (attitudeDesired.Throttle < 0))
		{
			ZeroPids();
		}

		// Clear alarms
		AlarmsClear(SYSTEMALARMS_ALARM_STABILIZATION);
	}
}

float ApplyPid(pid_type * pid, const float desired, const float actual, const uint8_t angular)
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
	for(int8_t ct = 0; ct < PID_MAX; ct++) {
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
	StabilizationSettingsGet(&settings);

	float * data = settings.RollRatePI;
	for(int8_t pid=0; pid < PID_MAX; pid++)
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
