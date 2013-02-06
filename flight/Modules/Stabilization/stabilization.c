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
#include "relaytuning.h"
#include "relaytuningsettings.h"
#include "stabilizationdesired.h"
#include "attitudeactual.h"
#include "gyros.h"
#include "flightstatus.h"
#include "manualcontrol.h" // Just to get a macro

// Math libraries
#include "CoordinateConversions.h"
#include "pid.h"
#include "sin_lookup.h"

// Includes for various stabilization algorithms
#include "relay_tuning.h"
#include "virtualflybar.h"

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


// Private variables
static xTaskHandle taskHandle;
static StabilizationSettingsData settings;
static xQueueHandle queue;
float gyro_alpha = 0;
float axis_lock_accum[3] = {0,0,0};
uint8_t max_axis_lock = 0;
uint8_t max_axislock_rate = 0;
float weak_leveling_kp = 0;
uint8_t weak_leveling_max = 0;
bool lowThrottleZeroIntegral;
bool lowThrottleZeroAxis[MAX_AXES];
float vbar_decay = 0.991f;
struct pid pids[PID_MAX];

// Private functions
static void stabilizationTask(void* parameters);
static float bound(float val, float range);
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
#if defined(RATEDESIRED_DIAGNOSTICS)
	RateDesiredInitialize();
#endif

	// Code required for relay tuning
	sin_lookup_initalize();
	RelayTuningSettingsInitialize();
	RelayTuningInitialize();

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
#if defined(RATEDESIRED_DIAGNOSTICS)
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

		float gyro_filtered[3];
		gyro_filtered[0] = gyro_filtered[0] * gyro_alpha + gyrosData.x * (1 - gyro_alpha);
		gyro_filtered[1] = gyro_filtered[1] * gyro_alpha + gyrosData.y * (1 - gyro_alpha);
		gyro_filtered[2] = gyro_filtered[2] * gyro_alpha + gyrosData.z * (1 - gyro_alpha);

		float *attitudeDesiredAxis = &stabDesired.Roll;
		float *actuatorDesiredAxis = &actuatorDesired.Roll;
		float *rateDesiredAxis = &rateDesired.Roll;

		ActuatorDesiredGet(&actuatorDesired);

		// A flag to track which stabilization mode each axis is in
		static uint8_t previous_mode[MAX_AXES] = {255,255,255};
		bool error = false;

		//Run the selected stabilization algorithm on each axis:
		for(uint8_t i=0; i< MAX_AXES; i++)
		{
			// Check whether this axis mode needs to be reinitialized
			bool reinit = (stabDesired.StabilizationMode[i] != previous_mode[i]);
			previous_mode[i] = stabDesired.StabilizationMode[i];

			// Apply the selected control law
			switch(stabDesired.StabilizationMode[i])
			{
				case STABILIZATIONDESIRED_STABILIZATIONMODE_RATE:
					if(reinit)
						pids[PID_RATE_ROLL + i].iAccumulator = 0;

					// Store to rate desired variable for storing to UAVO
					rateDesiredAxis[i] = bound(attitudeDesiredAxis[i], settings.ManualRate[i]);

					// Compute the inner loop
					actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i],  rateDesiredAxis[i],  gyro_filtered[i], dT);
					actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i],1.0f);

					break;

				case STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE:
					if(reinit) {
						pids[PID_ROLL + i].iAccumulator = 0;
						pids[PID_RATE_ROLL + i].iAccumulator = 0;
					}

					// Compute the outer loop
					rateDesiredAxis[i] = pid_apply(&pids[PID_ROLL + i], local_error[i], dT);
					rateDesiredAxis[i] = bound(rateDesiredAxis[i], settings.MaximumRate[i]);

					// Compute the inner loop
					actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i],  rateDesiredAxis[i],  gyro_filtered[i], dT);
					actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i],1.0f);

					break;

				case STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR:

					// Store for debugging output
					rateDesiredAxis[i] = attitudeDesiredAxis[i];

					// Run a virtual flybar stabilization algorithm on this axis
					stabilization_virtual_flybar(gyro_filtered[i], rateDesiredAxis[i], &actuatorDesiredAxis[i], dT, reinit, i, &settings);

					break;
				case STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING:
				{
					if (reinit)
						pids[PID_RATE_ROLL + i].iAccumulator = 0;

					float weak_leveling = local_error[i] * weak_leveling_kp;
					weak_leveling = bound(weak_leveling, weak_leveling_max);

					// Compute desired rate as input biased towards leveling
					rateDesiredAxis[i] = attitudeDesiredAxis[i] + weak_leveling;
					actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i],  rateDesiredAxis[i],  gyro_filtered[i], dT);
					actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i],1.0f);

					break;
				}
				case STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK:
					if (reinit)
						pids[PID_RATE_ROLL + i].iAccumulator = 0;

					if(fabs(attitudeDesiredAxis[i]) > max_axislock_rate) {
						// While getting strong commands act like rate mode
						rateDesiredAxis[i] = attitudeDesiredAxis[i];
						axis_lock_accum[i] = 0;
					} else {
						// For weaker commands or no command simply attitude lock (almost) on no gyro change
						axis_lock_accum[i] += (attitudeDesiredAxis[i] - gyro_filtered[i]) * dT;
						axis_lock_accum[i] = bound(axis_lock_accum[i], max_axis_lock);
						rateDesiredAxis[i] = pid_apply(&pids[PID_ROLL + i], axis_lock_accum[i], dT);
					}

					rateDesiredAxis[i] = bound(rateDesiredAxis[i], settings.MaximumRate[i]);

					actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i],  rateDesiredAxis[i],  gyro_filtered[i], dT);
					actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i],1.0f);

					break;

				case STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE:
					// Store to rate desired variable for storing to UAVO
					rateDesiredAxis[i] = bound(attitudeDesiredAxis[i], settings.ManualRate[i]);

					// Run the relay controller which also estimates the oscillation parameters
					stabilization_relay_rate(rateDesiredAxis[i] - gyro_filtered[i], &actuatorDesiredAxis[i], i, reinit);
					actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i],1.0);

					break;

				case STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE:
					if(reinit)
						pids[PID_ROLL + i].iAccumulator = 0;

					// Compute the outer loop like attitude mode
					rateDesiredAxis[i] = pid_apply(&pids[PID_ROLL + i], local_error[i], dT);
					rateDesiredAxis[i] = bound(rateDesiredAxis[i], settings.MaximumRate[i]);

					// Run the relay controller which also estimates the oscillation parameters
					stabilization_relay_rate(rateDesiredAxis[i] - gyro_filtered[i], &actuatorDesiredAxis[i], i, reinit);
					actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i],1.0);

					break;

				case STABILIZATIONDESIRED_STABILIZATIONMODE_NONE:
					actuatorDesiredAxis[i] = bound(attitudeDesiredAxis[i],1.0f);
					break;
				default:
					error = true;
					break;
			}
		}

		if (settings.VbarPiroComp == STABILIZATIONSETTINGS_VBARPIROCOMP_TRUE)
			stabilization_virtual_flybar_pirocomp(gyro_filtered[2], dT);

#if defined(RATEDESIRED_DIAGNOSTICS)
		RateDesiredSet(&rateDesired);
#endif

		// Save dT
		actuatorDesired.UpdateTime = dT * 1000;
		actuatorDesired.Throttle = stabDesired.Throttle;

		// Suppress desired output while disarmed or throttle low, for configured axis
		if (flightStatus.Armed != FLIGHTSTATUS_ARMED_ARMED || stabDesired.Throttle < 0) {
			if (lowThrottleZeroAxis[ROLL])
				actuatorDesired.Roll = 0.0f;

			if (lowThrottleZeroAxis[PITCH])
				actuatorDesired.Pitch = 0.0f;

			if (lowThrottleZeroAxis[YAW])
				actuatorDesired.Yaw = 0.0f;
		}

		if(PARSE_FLIGHT_MODE(flightStatus.FlightMode) != FLIGHTMODE_MANUAL) {
			ActuatorDesiredSet(&actuatorDesired);
		} else {
			// Force all axes to reinitialize when engaged
			for(uint8_t i=0; i< MAX_AXES; i++)
				previous_mode[i] = 255;
		}

		if(flightStatus.Armed != FLIGHTSTATUS_ARMED_ARMED ||
		   (lowThrottleZeroIntegral && stabDesired.Throttle < 0))
		{
			// Force all axes to reinitialize when engaged
			for(uint8_t i=0; i< MAX_AXES; i++)
				previous_mode[i] = 255;
		}

		// Clear or set alarms.  Done like this to prevent toggline each cycle
		// and hammering system alarms
		if (error)
			AlarmsSet(SYSTEMALARMS_ALARM_STABILIZATION,SYSTEMALARMS_ALARM_ERROR);
		else
			AlarmsClear(SYSTEMALARMS_ALARM_STABILIZATION);
	}
}


/**
 * Clear the accumulators and derivatives for all the axes
 */
static void ZeroPids(void)
{
	for(uint32_t i = 0; i < PID_MAX; i++)
		pid_zero(&pids[i]);


	for(uint8_t i = 0; i < 3; i++)
		axis_lock_accum[i] = 0.0f;
}


/**
 * Bound input value between limits
 */
static float bound(float val, float range)
{
	if(val < -range) {
		val = -range;
	} else if(val > range) {
		val = range;
	}
	return val;
}

static void SettingsUpdatedCb(UAVObjEvent * ev)
{
	StabilizationSettingsGet(&settings);
	
	// Set the roll rate PID constants
	pid_configure(&pids[PID_RATE_ROLL], settings.RollRatePID[STABILIZATIONSETTINGS_ROLLRATEPID_KP], 
		settings.RollRatePID[STABILIZATIONSETTINGS_ROLLRATEPID_KI],
		pids[PID_RATE_ROLL].d = settings.RollRatePID[STABILIZATIONSETTINGS_ROLLRATEPID_KD],
		pids[PID_RATE_ROLL].iLim = settings.RollRatePID[STABILIZATIONSETTINGS_ROLLRATEPID_ILIMIT]);
	
	// Set the pitch rate PID constants
	pid_configure(&pids[PID_RATE_PITCH], settings.PitchRatePID[STABILIZATIONSETTINGS_PITCHRATEPID_KP], 
		pids[PID_RATE_PITCH].i = settings.PitchRatePID[STABILIZATIONSETTINGS_PITCHRATEPID_KI],
		pids[PID_RATE_PITCH].d = settings.PitchRatePID[STABILIZATIONSETTINGS_PITCHRATEPID_KD],
		pids[PID_RATE_PITCH].iLim = settings.PitchRatePID[STABILIZATIONSETTINGS_PITCHRATEPID_ILIMIT]);
	
	// Set the yaw rate PID constants
	pid_configure(&pids[PID_RATE_YAW], settings.YawRatePID[STABILIZATIONSETTINGS_YAWRATEPID_KP],
		pids[PID_RATE_YAW].i = settings.YawRatePID[STABILIZATIONSETTINGS_YAWRATEPID_KI],
		pids[PID_RATE_YAW].d = settings.YawRatePID[STABILIZATIONSETTINGS_YAWRATEPID_KD],
		pids[PID_RATE_YAW].iLim = settings.YawRatePID[STABILIZATIONSETTINGS_YAWRATEPID_ILIMIT]);
	
	// Set the roll attitude PI constants
	pid_configure(&pids[PID_ROLL], settings.RollPI[STABILIZATIONSETTINGS_ROLLPI_KP],
		settings.RollPI[STABILIZATIONSETTINGS_ROLLPI_KI], 0,
		pids[PID_ROLL].iLim = settings.RollPI[STABILIZATIONSETTINGS_ROLLPI_ILIMIT]);
	
	// Set the pitch attitude PI constants
	pid_configure(&pids[PID_PITCH], settings.PitchPI[STABILIZATIONSETTINGS_PITCHPI_KP],
		pids[PID_PITCH].i = settings.PitchPI[STABILIZATIONSETTINGS_PITCHPI_KI], 0,
		settings.PitchPI[STABILIZATIONSETTINGS_PITCHPI_ILIMIT]);
	
	// Set the yaw attitude PI constants
	pid_configure(&pids[PID_YAW], settings.YawPI[STABILIZATIONSETTINGS_YAWPI_KP],
		settings.YawPI[STABILIZATIONSETTINGS_YAWPI_KI], 0,
		settings.YawPI[STABILIZATIONSETTINGS_YAWPI_ILIMIT]);
	
	// Set up the derivative term
	pid_configure_derivative(settings.DerivativeCutoff, settings.DerivativeGamma);

	// Maximum deviation to accumulate for axis lock
	max_axis_lock = settings.MaxAxisLock;
	max_axislock_rate = settings.MaxAxisLockRate;
	
	// Settings for weak leveling
	weak_leveling_kp = settings.WeakLevelingKp;
	weak_leveling_max = settings.MaxWeakLevelingRate;
	
	// Whether to zero the PID integrals while throttle is low
	lowThrottleZeroIntegral = settings.LowThrottleZeroIntegral == STABILIZATIONSETTINGS_LOWTHROTTLEZEROINTEGRAL_TRUE;

	// Whether to suppress (zero) the StabilizationDesired output for each axis while disarmed or throttle is low
	lowThrottleZeroAxis[ROLL] = settings.LowThrottleZeroAxis[STABILIZATIONSETTINGS_LOWTHROTTLEZEROAXIS_ROLL] == STABILIZATIONSETTINGS_LOWTHROTTLEZEROAXIS_TRUE;
	lowThrottleZeroAxis[PITCH] = settings.LowThrottleZeroAxis[STABILIZATIONSETTINGS_LOWTHROTTLEZEROAXIS_PITCH] == STABILIZATIONSETTINGS_LOWTHROTTLEZEROAXIS_TRUE;
	lowThrottleZeroAxis[YAW] = settings.LowThrottleZeroAxis[STABILIZATIONSETTINGS_LOWTHROTTLEZEROAXIS_YAW] == STABILIZATIONSETTINGS_LOWTHROTTLEZEROAXIS_TRUE;

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

