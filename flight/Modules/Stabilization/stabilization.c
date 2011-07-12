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
#include "ratedesired.h"
#include "stabilizationdesired.h"
#include "attitudeactual.h"
#include "attituderaw.h"
#include "flightstatus.h"
#include "systemsettings.h"
#include "ahrssettings.h"
#include "manualcontrol.h" // Just to get a macro
#include "CoordinateConversions.h"

// Private constants
#define MAX_QUEUE_SIZE 1

#if defined(PIOS_STABILIZATION_STACK_SIZE)
#define STACK_SIZE_BYTES PIOS_STABILIZATION_STACK_SIZE
#else
#define STACK_SIZE_BYTES 724
#endif

#define TASK_PRIORITY (tskIDLE_PRIORITY+4)
#define FAILSAFE_TIMEOUT_MS 30

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
static float ApplyPid(pid_type * pid, const float err);
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
	//	AttitudeActualConnectQueue(queue);
	AttitudeRawConnectQueue(queue);
	
	StabilizationSettingsConnectCallback(SettingsUpdatedCb);
	SettingsUpdatedCb(StabilizationSettingsHandle());
	// Start main task
	xTaskCreate(stabilizationTask, (signed char*)"Stabilization", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_STABILIZATION, taskHandle);
	PIOS_WDG_RegisterFlag(PIOS_WDG_STABILIZATION);
	
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
	StabilizationDesiredData stabDesired;
	RateDesiredData rateDesired;
	AttitudeActualData attitudeActual;
	AttitudeRawData attitudeRaw;
	SystemSettingsData systemSettings;
	FlightStatusData flightStatus;
	
	SettingsUpdatedCb((UAVObjEvent *) NULL);
	
	// Main task loop
	lastSysTime = xTaskGetTickCount();
	ZeroPids();
	while(1) {
		PIOS_WDG_UpdateFlag(PIOS_WDG_STABILIZATION);
		
		// Wait until the AttitudeRaw object is updated, if a timeout then go to failsafe
		if ( xQueueReceive(queue, &ev, FAILSAFE_TIMEOUT_MS / portTICK_RATE_MS) != pdTRUE )
		{
			AlarmsSet(SYSTEMALARMS_ALARM_STABILIZATION,SYSTEMALARMS_ALARM_WARNING);
			continue;
		} 
		
		// Check how long since last update
		thisSysTime = xTaskGetTickCount();
		if(thisSysTime > lastSysTime) // reuse dt in case of wraparound
			dT = (thisSysTime - lastSysTime) / portTICK_RATE_MS / 1000.0f;
		lastSysTime = thisSysTime;
		
		FlightStatusGet(&flightStatus);
		StabilizationDesiredGet(&stabDesired);
		AttitudeActualGet(&attitudeActual);
		AttitudeRawGet(&attitudeRaw);
		RateDesiredGet(&rateDesired);
		SystemSettingsGet(&systemSettings);

#if defined(PIOS_QUATERNION_STABILIZATION)
		// Quaternion calculation of error in each axis.  Uses more memory.
		float rpy_desired[3];
		float q_desired[4];
		float q_error[4];
		float local_error[3];
		
		// Essentially zero errors for anything in rate or none
		if(stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE)
			rpy_desired[0] = stabDesired.Roll;		
		else 
			rpy_desired[0] = attitudeActual.Roll;

		if(stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE)
			rpy_desired[1] = stabDesired.Pitch;
		else
			rpy_desired[1] = attitudeActual.Pitch;

		if(stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE)
			rpy_desired[2] = stabDesired.Yaw;
		else
			rpy_desired[2] = attitudeActual.Yaw;
		
		RPY2Quaternion(rpy_desired, q_desired);
		quat_inverse(q_desired);
		quat_mult(q_desired, &attitudeActual.q1, q_error);
		quat_inverse(q_error);
		Quaternion2RPY(q_error, local_error);
		
#else
		// Simpler algorithm for CC, less memory
		float local_error[3] = {stabDesired.Roll - attitudeActual.Roll,
			stabDesired.Pitch - attitudeActual.Pitch,
			stabDesired.Yaw - attitudeActual.Yaw};
		local_error[2] = fmod(local_error[2] + 180, 360) - 180;
#endif
			
		float *attitudeDesiredAxis = &stabDesired.Roll;
		float *actuatorDesiredAxis = &actuatorDesired.Roll;
		float *rateDesiredAxis = &rateDesired.Roll;
		
		//Calculate desired rate
		for(int8_t ct=0; ct< MAX_AXES; ct++)
		{
			switch(stabDesired.StabilizationMode[ct])
			{
				case STABILIZATIONDESIRED_STABILIZATIONMODE_RATE:
					rateDesiredAxis[ct] = attitudeDesiredAxis[ct];
					break;
					
				case STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE:
					rateDesiredAxis[ct] = ApplyPid(&pids[PID_ROLL + ct], local_error[ct]);
					break;
			}
		}
		
		uint8_t shouldUpdate = 1;
		RateDesiredSet(&rateDesired);
		ActuatorDesiredGet(&actuatorDesired);
		//Calculate desired command
		for(int8_t ct=0; ct< MAX_AXES; ct++)
		{
			if(fabs(rateDesiredAxis[ct]) > settings.MaximumRate[ct])
			{
				if(rateDesiredAxis[ct] > 0)
				{
					rateDesiredAxis[ct] = settings.MaximumRate[ct];
				}else
				{
					rateDesiredAxis[ct] = -settings.MaximumRate[ct];
				}
				
			}
			switch(stabDesired.StabilizationMode[ct])
			{
				case STABILIZATIONDESIRED_STABILIZATIONMODE_RATE:
				case STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE:
				{
					float command = ApplyPid(&pids[PID_RATE_ROLL + ct],  rateDesiredAxis[ct]-attitudeRaw.gyros[ct]);
					actuatorDesiredAxis[ct] = bound(command);
					break;
				}
				case STABILIZATIONDESIRED_STABILIZATIONMODE_NONE:
					switch (ct)
				{
					case ROLL:
						actuatorDesiredAxis[ct] = bound(attitudeDesiredAxis[ct]);
						shouldUpdate = 1;
						break;
					case PITCH:
						actuatorDesiredAxis[ct] = bound(attitudeDesiredAxis[ct]);
						shouldUpdate = 1;
						break;
					case YAW:
						actuatorDesiredAxis[ct] = bound(attitudeDesiredAxis[ct]);
						shouldUpdate = 1;
						break;
				}
					break;
					
			}
		}
		
		// Save dT
		actuatorDesired.UpdateTime = dT * 1000;
		
		if(PARSE_FLIGHT_MODE(flightStatus.FlightMode) == FLIGHTMODE_MANUAL)
			shouldUpdate = 0;
		
		if(shouldUpdate)
		{
			actuatorDesired.Throttle = stabDesired.Throttle;
			if(dT > 15)
				actuatorDesired.NumLongUpdates++;
			ActuatorDesiredSet(&actuatorDesired);
		}
		
		if(flightStatus.Armed != FLIGHTSTATUS_ARMED_ARMED ||
		   !shouldUpdate || (stabDesired.Throttle < 0))
		{
			ZeroPids();
		}
		
		
		// Clear alarms
		AlarmsClear(SYSTEMALARMS_ALARM_STABILIZATION);				
	}
}

float ApplyPid(pid_type * pid, const float err)
{
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
