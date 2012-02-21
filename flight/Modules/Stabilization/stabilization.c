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
#include "gyros.h"
#include "flightstatus.h"
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

enum {PID_RATE_ROLL, PID_RATE_PITCH, PID_RATE_YAW, PID_ROLL, PID_PITCH, PID_YAW, PID_VBAR_ROLL, PID_VBAR_PITCH, PID_VBAR_YAW, PID_MAX};

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
float gyro_alpha = 0;
float gyro_filtered[3] = {0,0,0};
float axis_lock_accum[3] = {0,0,0};
uint8_t max_axis_lock = 0;
uint8_t max_axislock_rate = 0;
float weak_leveling_kp = 0;
uint8_t weak_leveling_max = 0;
bool lowThrottleZeroIntegral;
float vbar_integral[3] = {0, 0, 0};
float vbar_decay = 0.991f;
pid_type pids[PID_MAX];

// Private functions
static void stabilizationTask(void* parameters);
static float ApplyPid(pid_type * pid, const float err, float dT);
static float bound(float val);
static void ZeroPids(void);
static void SettingsUpdatedCb(UAVObjEvent * ev);

/**
 * Module initialization
 */
int32_t StabilizationStart()
{
	// Initialize variables
	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	// Listen for updates.
	//	AttitudeActualConnectQueue(queue);
	GyrosConnectQueue(queue);

	StabilizationSettingsConnectCallback(SettingsUpdatedCb);
	SettingsUpdatedCb(StabilizationSettingsHandle());

	// Start main task
	xTaskCreate(stabilizationTask, (signed char*)"Stabilization", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_STABILIZATION, taskHandle);
	PIOS_WDG_RegisterFlag(PIOS_WDG_STABILIZATION);
	return 0;
}

/**
 * Module initialization
 */
int32_t StabilizationInitialize()
{
	// Initialize variables
	StabilizationSettingsInitialize();
	ActuatorDesiredInitialize();
#if defined(DIAGNOSTICS)
	RateDesiredInitialize();
#endif

	return 0;
}

MODULE_INITCALL(StabilizationInitialize, StabilizationStart)

/**
 * Module task
 */
static void stabilizationTask(void* parameters)
{
	UAVObjEvent ev;

	uint32_t timeval = PIOS_DELAY_GetRaw();

	ActuatorDesiredData actuatorDesired;
	StabilizationDesiredData stabDesired;
	RateDesiredData rateDesired;
	AttitudeActualData attitudeActual;
	GyrosData gyrosData;
	FlightStatusData flightStatus;

	SettingsUpdatedCb((UAVObjEvent *) NULL);

	// Main task loop
	ZeroPids();
	while(1) {
		float dT;

		PIOS_WDG_UpdateFlag(PIOS_WDG_STABILIZATION);

		// Wait until the AttitudeRaw object is updated, if a timeout then go to failsafe
		if ( xQueueReceive(queue, &ev, FAILSAFE_TIMEOUT_MS / portTICK_RATE_MS) != pdTRUE )
		{
			AlarmsSet(SYSTEMALARMS_ALARM_STABILIZATION,SYSTEMALARMS_ALARM_WARNING);
			continue;
		}

		dT = PIOS_DELAY_DiffuS(timeval) * 1.0e-6f;
		timeval = PIOS_DELAY_GetRaw();

		FlightStatusGet(&flightStatus);
		StabilizationDesiredGet(&stabDesired);
		AttitudeActualGet(&attitudeActual);
		GyrosGet(&gyrosData);

#if defined(DIAGNOSTICS)
		RateDesiredGet(&rateDesired);
#endif

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
		local_error[2] = fmodf(local_error[2] + 180, 360) - 180;
#endif


		gyro_filtered[0] = gyro_filtered[0] * gyro_alpha + gyrosData.x * (1 - gyro_alpha);
		gyro_filtered[1] = gyro_filtered[1] * gyro_alpha + gyrosData.y * (1 - gyro_alpha);
		gyro_filtered[2] = gyro_filtered[2] * gyro_alpha + gyrosData.z * (1 - gyro_alpha);

		float *attitudeDesiredAxis = &stabDesired.Roll;
		float *actuatorDesiredAxis = &actuatorDesired.Roll;
		float *rateDesiredAxis = &rateDesired.Roll;

		//Calculate desired rate
		for(uint8_t i=0; i< MAX_AXES; i++)
		{
			switch(stabDesired.StabilizationMode[i])
			{
				case STABILIZATIONDESIRED_STABILIZATIONMODE_RATE:
				case STABILIZATIONDESIRED_STABILIZATIONMODE_VBAR:
					rateDesiredAxis[i] = attitudeDesiredAxis[i];

					// Zero attitude and axis lock accumulators
					pids[PID_ROLL + i].iAccumulator = 0;
					axis_lock_accum[i] = 0;
					break;

				case STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING:
				{
					float weak_leveling = local_error[i] * weak_leveling_kp;

					if(weak_leveling > weak_leveling_max)
						weak_leveling = weak_leveling_max;
					if(weak_leveling < -weak_leveling_max)
						weak_leveling = -weak_leveling_max;

					rateDesiredAxis[i] = attitudeDesiredAxis[i] + weak_leveling;

					// Zero attitude and axis lock accumulators
					pids[PID_ROLL + i].iAccumulator = 0;
					axis_lock_accum[i] = 0;
					break;
				}
				case STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE:
					rateDesiredAxis[i] = ApplyPid(&pids[PID_ROLL + i], local_error[i], dT);
					
					if(rateDesiredAxis[i] > settings.MaximumRate[i])
						rateDesiredAxis[i] = settings.MaximumRate[i];
					else if(rateDesiredAxis[i] < -settings.MaximumRate[i])
						rateDesiredAxis[i] = -settings.MaximumRate[i];
					
					
					axis_lock_accum[i] = 0;
					break;

				case STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK:
					if(fabs(attitudeDesiredAxis[i]) > max_axislock_rate) {
						// While getting strong commands act like rate mode
						rateDesiredAxis[i] = attitudeDesiredAxis[i];
						axis_lock_accum[i] = 0;
					} else {
						// For weaker commands or no command simply attitude lock (almost) on no gyro change
						axis_lock_accum[i] += (attitudeDesiredAxis[i] - gyro_filtered[i]) * dT;
						if(axis_lock_accum[i] > max_axis_lock)
							axis_lock_accum[i] = max_axis_lock;
						else if(axis_lock_accum[i] < -max_axis_lock)
							axis_lock_accum[i] = -max_axis_lock;

						rateDesiredAxis[i] = ApplyPid(&pids[PID_ROLL + i], axis_lock_accum[i], dT);
					}
					
					if(rateDesiredAxis[i] > settings.MaximumRate[i])
						rateDesiredAxis[i] = settings.MaximumRate[i];
					else if(rateDesiredAxis[i] < -settings.MaximumRate[i])
						rateDesiredAxis[i] = -settings.MaximumRate[i];

					break;
			}
		}

		uint8_t shouldUpdate = 1;
#if defined(DIAGNOSTICS)
		RateDesiredSet(&rateDesired);
#endif
		ActuatorDesiredGet(&actuatorDesired);
		//Calculate desired command
		for(uint32_t i = 0; i < MAX_AXES; i++)
		{
			switch(stabDesired.StabilizationMode[i])
			{
				case STABILIZATIONDESIRED_STABILIZATIONMODE_RATE:
				case STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE:
				case STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK:
				case STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING:
				{
					float command = ApplyPid(&pids[PID_RATE_ROLL + i],  rateDesiredAxis[i] - gyro_filtered[i], dT);
					actuatorDesiredAxis[i] = bound(command);
					break;
				}
				case STABILIZATIONDESIRED_STABILIZATIONMODE_VBAR:
				{
					// Track the angle of the virtual flybar which includes a slow decay
					vbar_integral[i] = vbar_integral[i] * vbar_decay + gyro_filtered[i] * dT;

					// Command signal is composed of stick input added to the gyro and virtual flybar
					float command = rateDesiredAxis[i] - vbar_integral[i] * pids[PID_VBAR_ROLL + i].i - gyro_filtered[i] * pids[PID_VBAR_ROLL + i].p;

					actuatorDesiredAxis[i] = bound(command);
					break;
				}
				case STABILIZATIONDESIRED_STABILIZATIONMODE_NONE:
					switch (i)
				{
					case ROLL:
						actuatorDesiredAxis[i] = bound(attitudeDesiredAxis[i]);
						shouldUpdate = 1;
						pids[PID_RATE_ROLL].iAccumulator = 0;
						pids[PID_ROLL].iAccumulator = 0;
						break;
					case PITCH:
						actuatorDesiredAxis[i] = bound(attitudeDesiredAxis[i]);
						shouldUpdate = 1;
						pids[PID_RATE_PITCH].iAccumulator = 0;
						pids[PID_PITCH].iAccumulator = 0;
						break;
					case YAW:
						actuatorDesiredAxis[i] = bound(attitudeDesiredAxis[i]);
						shouldUpdate = 1;
						pids[PID_RATE_YAW].iAccumulator = 0;
						pids[PID_YAW].iAccumulator = 0;
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
		   (lowThrottleZeroIntegral && stabDesired.Throttle < 0) ||
		   !shouldUpdate)
		{
			ZeroPids();
		}


		// Clear alarms
		AlarmsClear(SYSTEMALARMS_ALARM_STABILIZATION);
	}
}

float ApplyPid(pid_type * pid, const float err, float dT)
{
	float diff = (err - pid->lastErr);
	pid->lastErr = err;

	// Scale up accumulator by 1000 while computing to avoid losing precision
	pid->iAccumulator += err * (pid->i * dT * 1000.0f);
	if(pid->iAccumulator > (pid->iLim * 1000.0f)) {
		pid->iAccumulator = pid->iLim * 1000.0f;
	} else if (pid->iAccumulator < -(pid->iLim * 1000.0f)) {
		pid->iAccumulator = -pid->iLim * 1000.0f;
	}
	return ((err * pid->p) + pid->iAccumulator / 1000.0f + (diff * pid->d / dT));
}


static void ZeroPids(void)
{
	for(int8_t ct = 0; ct < PID_MAX; ct++) {
		pids[ct].iAccumulator = 0.0f;
		pids[ct].lastErr = 0.0f;
	}
	for(uint8_t i = 0; i < 3; i++)
		axis_lock_accum[i] = 0.0f;
}


/**
 * Bound input value between limits
 */
static float bound(float val)
{
	if(val < -1.0f) {
		val = -1.0f;
	} else if(val > 1.0f) {
		val = 1.0f;
	}
	return val;
}

static void SettingsUpdatedCb(UAVObjEvent * ev)
{
	memset(pids,0,sizeof (pid_type) * PID_MAX);
	StabilizationSettingsGet(&settings);

	// Set the roll rate PID constants
	pids[PID_RATE_ROLL].p = settings.RollRatePID[STABILIZATIONSETTINGS_ROLLRATEPID_KP];
	pids[PID_RATE_ROLL].i = settings.RollRatePID[STABILIZATIONSETTINGS_ROLLRATEPID_KI];
	pids[PID_RATE_ROLL].d = settings.RollRatePID[STABILIZATIONSETTINGS_ROLLRATEPID_KD];
	pids[PID_RATE_ROLL].iLim = settings.RollRatePID[STABILIZATIONSETTINGS_ROLLRATEPID_ILIMIT];

	// Set the pitch rate PID constants
	pids[PID_RATE_PITCH].p = settings.PitchRatePID[STABILIZATIONSETTINGS_PITCHRATEPID_KP];
	pids[PID_RATE_PITCH].i = settings.PitchRatePID[STABILIZATIONSETTINGS_PITCHRATEPID_KI];
	pids[PID_RATE_PITCH].d = settings.PitchRatePID[STABILIZATIONSETTINGS_PITCHRATEPID_KD];
	pids[PID_RATE_PITCH].iLim = settings.PitchRatePID[STABILIZATIONSETTINGS_PITCHRATEPID_ILIMIT];

	// Set the yaw rate PID constants
	pids[PID_RATE_YAW].p = settings.YawRatePID[STABILIZATIONSETTINGS_YAWRATEPID_KP];
	pids[PID_RATE_YAW].i = settings.YawRatePID[STABILIZATIONSETTINGS_YAWRATEPID_KI];
	pids[PID_RATE_YAW].d = settings.YawRatePID[STABILIZATIONSETTINGS_YAWRATEPID_KD];
	pids[PID_RATE_YAW].iLim = settings.YawRatePID[STABILIZATIONSETTINGS_YAWRATEPID_ILIMIT];

	// Set the roll attitude PI constants
	pids[PID_ROLL].p = settings.RollPI[STABILIZATIONSETTINGS_ROLLPI_KP];
	pids[PID_ROLL].i = settings.RollPI[STABILIZATIONSETTINGS_ROLLPI_KI];
	pids[PID_ROLL].iLim = settings.RollPI[STABILIZATIONSETTINGS_ROLLPI_ILIMIT];

	// Set the pitch attitude PI constants
	pids[PID_PITCH].p = settings.PitchPI[STABILIZATIONSETTINGS_PITCHPI_KP];
	pids[PID_PITCH].i = settings.PitchPI[STABILIZATIONSETTINGS_PITCHPI_KI];
	pids[PID_PITCH].iLim = settings.PitchPI[STABILIZATIONSETTINGS_PITCHPI_ILIMIT];

	// Set the yaw attitude PI constants
	pids[PID_YAW].p = settings.YawPI[STABILIZATIONSETTINGS_YAWPI_KP];
	pids[PID_YAW].i = settings.YawPI[STABILIZATIONSETTINGS_YAWPI_KI];
	pids[PID_YAW].iLim = settings.YawPI[STABILIZATIONSETTINGS_YAWPI_ILIMIT];

	// Set the roll attitude PI constants
	pids[PID_VBAR_ROLL].p = settings.VbarRollPI[STABILIZATIONSETTINGS_VBARROLLPI_KP];
	pids[PID_VBAR_ROLL].i = settings.VbarRollPI[STABILIZATIONSETTINGS_VBARROLLPI_KI];
	
	// Set the pitch attitude PI constants
	pids[PID_VBAR_PITCH].p = settings.VbarPitchPI[STABILIZATIONSETTINGS_VBARPITCHPI_KP];
	pids[PID_VBAR_PITCH].i = settings.VbarPitchPI[STABILIZATIONSETTINGS_VBARPITCHPI_KI];
	
	// Set the yaw attitude PI constants
	pids[PID_VBAR_YAW].p = settings.VbarYawPI[STABILIZATIONSETTINGS_VBARYAWPI_KP];
	pids[PID_VBAR_YAW].i = settings.VbarYawPI[STABILIZATIONSETTINGS_VBARYAWPI_KI];

	// Maximum deviation to accumulate for axis lock
	max_axis_lock = settings.MaxAxisLock;
	max_axislock_rate = settings.MaxAxisLockRate;

	// Settings for weak leveling
	weak_leveling_kp = settings.WeakLevelingKp;
	weak_leveling_max = settings.MaxWeakLevelingRate;

	// Whether to zero the PID integrals while throttle is low
	lowThrottleZeroIntegral = settings.LowThrottleZeroIntegral == STABILIZATIONSETTINGS_LOWTHROTTLEZEROINTEGRAL_TRUE;

	// The dT has some jitter iteration to iteration that we don't want to
	// make thie result unpredictable.  Still, it's nicer to specify the constant
	// based on a time (in ms) rather than a fixed multiplier.  The error between
	// update rates on OP (~300 Hz) and CC (~475 Hz) is negligible for this
	// calculation
	const float fakeDt = 0.0025;
	if(settings.GyroTau < 0.0001)
		gyro_alpha = 0;   // not trusting this to resolve to 0
	else
		gyro_alpha = expf(-fakeDt  / settings.GyroTau);

	// Compute time constant for vbar decay term based on a tau
	vbar_decay = expf(-fakeDt / settings.VbarTau);
}


/**
 * @}
 * @}
 */
