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
float vbar_sensitivity[3] = {1, 1, 1};
uint8_t max_axis_lock = 0;
uint8_t max_axislock_rate = 0;
float weak_leveling_kp = 0;
uint8_t weak_leveling_max = 0;
bool lowThrottleZeroIntegral;
float vbar_integral[3] = {0, 0, 0};
float vbar_decay = 0.991f;
pid_type pids[PID_MAX];
int8_t vbar_gyros_suppress;
bool vbar_piro_comp = false;

// TODO: Move this to flash
static float sin_lookup[360];

// Private functions
static void stabilizationTask(void* parameters);
static float ApplyPid(pid_type * pid, const float err, float dT);
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

	for(uint32_t i = 0; i < 360; i++)
		sin_lookup[i] = sinf((float)i * 2 * M_PI / 360.0f);

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
	RelayTuningSettingsInitialize();
	RelayTuningInitialize();
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
					actuatorDesiredAxis[i] = ApplyPid(&pids[PID_RATE_ROLL + i],  rateDesiredAxis[i] - gyro_filtered[i], dT);
					actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i],1.0f);

					break;

				case STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE:
					if(reinit) {
						pids[PID_ROLL + i].iAccumulator = 0;
						pids[PID_RATE_ROLL + i].iAccumulator = 0;
					}

					// Compute the outer loop
					rateDesiredAxis[i] = ApplyPid(&pids[PID_ROLL + i], local_error[i], dT);
					rateDesiredAxis[i] = bound(rateDesiredAxis[i], settings.MaximumRate[i]);

					// Compute the inner loop
					actuatorDesiredAxis[i] = ApplyPid(&pids[PID_RATE_ROLL + i],  rateDesiredAxis[i] - gyro_filtered[i], dT);
					actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i],1.0f);

					break;

				case STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR:
				{
					if(reinit)
						vbar_integral[i] = 0;

					rateDesiredAxis[i] = attitudeDesiredAxis[i];

					// Track the angle of the virtual flybar which includes a slow decay
					vbar_integral[i] = vbar_integral[i] * vbar_decay + gyro_filtered[i] * dT;
					vbar_integral[i] = bound(vbar_integral[i], settings.VbarMaxAngle);

					// Command signal can indicate how much to disregard the gyro feedback (fast flips)
					float gyro_gain = 1.0f;
					if (vbar_gyros_suppress > 0) {
						gyro_gain = (1.0f - fabs(rateDesiredAxis[i]) * vbar_gyros_suppress / 100.0f);
						gyro_gain = (gyro_gain < 0) ? 0 : gyro_gain;
					}

					// Command signal is composed of stick input added to the gyro and virtual flybar
					actuatorDesiredAxis[i] = rateDesiredAxis[i] * vbar_sensitivity[i] - 
					gyro_gain * (vbar_integral[i] * pids[PID_VBAR_ROLL + i].i +
								 gyro_filtered[i] * pids[PID_VBAR_ROLL + i].p);

					actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i],1.0f);

					break;
				}
				case STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING:
				{
					if (reinit)
						pids[PID_RATE_ROLL + i].iAccumulator = 0;

					float weak_leveling = local_error[i] * weak_leveling_kp;
					weak_leveling = bound(weak_leveling, weak_leveling_max);

					// Compute desired rate as input biased towards leveling
					rateDesiredAxis[i] = attitudeDesiredAxis[i] + weak_leveling;
					actuatorDesiredAxis[i] = ApplyPid(&pids[PID_RATE_ROLL + i],  rateDesiredAxis[i] - gyro_filtered[i], dT);
					actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i],1.0f);

					break;
				}
				case STABILIZATIONDESIRED_STABILIZATIONMODE_RELAY:
				case STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE:
					rateDesiredAxis[i] = ApplyPid(&pids[PID_ROLL + i], local_error[i], dT);
					
					if(rateDesiredAxis[i] > settings.MaximumRate[i])
						rateDesiredAxis[i] = settings.MaximumRate[i];
					else if(rateDesiredAxis[i] < -settings.MaximumRate[i])
						rateDesiredAxis[i] = -settings.MaximumRate[i];
					
					
					axis_lock_accum[i] = 0;
					break;

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
						rateDesiredAxis[i] = ApplyPid(&pids[PID_ROLL + i], axis_lock_accum[i], dT);
					}

					rateDesiredAxis[i] = bound(rateDesiredAxis[i], settings.MaximumRate[i]);

					actuatorDesiredAxis[i] = ApplyPid(&pids[PID_RATE_ROLL + i],  rateDesiredAxis[i] - gyro_filtered[i], dT);
					actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i],1.0f);

					break;

				case STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE:
				{
					RelayTuningData relay;
					RelayTuningGet(&relay);

					static bool rateRelayRunning[MAX_AXES];

					// On first run initialize estimates to something reasonable
					if(reinit) {
						pids[PID_ROLL + i].iAccumulator = 0;
						rateRelayRunning[i] = false;
						relay.Period[i] = 200;
						relay.Gain[i] = 0;
					}
					// Replace the rate PID with a relay to measure the critical properties of this axis
					// i.e. period and gain

					// Compute the outer loop
					rateDesiredAxis[i] = ApplyPid(&pids[PID_ROLL + i], local_error[i], dT);
					rateDesiredAxis[i] = bound(rateDesiredAxis[i], settings.MaximumRate[i]);

					// Store to rate desired variable for storing to UAVO
					rateDesiredAxis[i] = bound(attitudeDesiredAxis[i], settings.ManualRate[i]);

					RelayTuningSettingsData relaySettings;
					RelayTuningSettingsGet(&relaySettings);
					float error = rateDesiredAxis[i] - gyro_filtered[i];
					float command = error > 0 ? relaySettings.Amplitude : -relaySettings.Amplitude;
					actuatorDesiredAxis[i] = bound(command,1.0f);

					static bool high = false;
					static portTickType lastHighTime;
					static portTickType lastLowTime;
					portTickType thisTime = xTaskGetTickCount();

					static float accum_sin, accum_cos;
					static uint32_t accumulated = 0;

					const uint16_t DEGLITCH_TIME = 20; // ms
					const float AMPLITUDE_ALPHA = 0.95;
					const float PERIOD_ALPHA = 0.95;

					// Make sure the period can't go below limit
					if (relay.Period[i] < DEGLITCH_TIME)
						relay.Period[i] = DEGLITCH_TIME;

					// Project the error onto a sine and cosine of the same frequency
					// to accumulate the average amplitude
					float dT = thisTime - lastHighTime;
					uint32_t phase = 360 * dT / relay.Period[i];
					if(phase >= 360)
						phase = 1;
					accum_sin += sin_lookup[phase] * error;
					accum_cos += sin_lookup[(phase + 90) % 360] * error;
					accumulated ++;

					// Make susre we've had enough time since last transition then check for a change in the output
					bool hysteresis = (high ? (thisTime - lastHighTime) : (thisTime - lastLowTime)) > DEGLITCH_TIME;
					if ( !high && hysteresis && error > 0 ){ /* RISE DETECTED */
						float this_amplitude = 2 * sqrtf(accum_sin*accum_sin + accum_cos*accum_cos) / accumulated;
						float this_gain = this_amplitude / relaySettings.Amplitude;

						accumulated = 0;
						accum_sin = 0;
						accum_cos = 0;

						if(rateRelayRunning[i] == false) {
							rateRelayRunning[i] = true;
							relay.Period[i] = 200;
							relay.Gain[i] = 0;
						} else {
							// Low pass filter each amplitude and period
							relay.Gain[i] = relay.Gain[i] * AMPLITUDE_ALPHA + this_gain * (1 - AMPLITUDE_ALPHA);
							relay.Period[i] = relay.Period[i] * PERIOD_ALPHA + dT * (1 - PERIOD_ALPHA);
						}
						lastHighTime = thisTime;
						high = true;
						RelayTuningSet(&relay);
					} else if ( high && hysteresis && error < 0 ) { /* FALL DETECTED */
						lastLowTime = thisTime;
						high = false;
					}

					break;
				}
				case STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE:
				{
					RelayTuningData relay;
					RelayTuningGet(&relay);

					static bool rateRelayRunning[MAX_AXES];

					// On first run initialize estimates to something reasonable
					if(reinit) {
						pids[PID_ROLL + i].iAccumulator = 0;
						rateRelayRunning[i] = false;
						relay.Period[i] = 200;
						relay.Gain[i] = 0;
					}

					// Replace the rate PID with a relay to measure the critical properties of this axis
					// i.e. period and gain

					// Store to rate desired variable for storing to UAVO
					rateDesiredAxis[i] = bound(attitudeDesiredAxis[i], settings.ManualRate[i]);

					RelayTuningSettingsData relaySettings;
					RelayTuningSettingsGet(&relaySettings);
					float error = rateDesiredAxis[i] - gyro_filtered[i];
					float command = error > 0 ? relaySettings.Amplitude : -relaySettings.Amplitude;
					actuatorDesiredAxis[i] = bound(command,1.0);

					static bool high = false;
					static portTickType lastHighTime;
					static portTickType lastLowTime;
					portTickType thisTime = xTaskGetTickCount();

					static float accum_sin, accum_cos;
					static uint32_t accumulated = 0;

					const uint16_t DEGLITCH_TIME = 20; // ms
					const float AMPLITUDE_ALPHA = 0.95;
					const float PERIOD_ALPHA = 0.95;

					// Make sure the period can't go below limit
					if (relay.Period[i] < DEGLITCH_TIME)
						relay.Period[i] = DEGLITCH_TIME;

					// Project the error onto a sine and cosine of the same frequency
					// to accumulate the average amplitude
					float dT = thisTime - lastHighTime;
					uint32_t phase = 360 * dT / relay.Period[i];
					if(phase >= 360)
						phase = 1;
					accum_sin += sin_lookup[phase] * error;
					accum_cos += sin_lookup[(phase + 90) % 360] * error;
					accumulated ++;

					// Make susre we've had enough time since last transition then check for a change in the output
					bool hysteresis = (high ? (thisTime - lastHighTime) : (thisTime - lastLowTime)) > DEGLITCH_TIME;
					if ( !high && hysteresis && error > 0 ){ /* RISE DETECTED */
						float this_amplitude = 2 * sqrtf(accum_sin*accum_sin + accum_cos*accum_cos) / accumulated;
						float this_gain = this_amplitude / relaySettings.Amplitude;

						accumulated = 0;
						accum_sin = 0;
						accum_cos = 0;

						if(rateRelayRunning[i] == false) {
							rateRelayRunning[i] = true;
							relay.Period[i] = 200;
							relay.Gain[i] = 0;
						} else {
							// Low pass filter each amplitude and period
							relay.Gain[i] = relay.Gain[i] * AMPLITUDE_ALPHA + this_gain * (1 - AMPLITUDE_ALPHA);
							relay.Period[i] = relay.Period[i] * PERIOD_ALPHA + dT * (1 - PERIOD_ALPHA);
						}
						lastHighTime = thisTime;
						high = true;
						RelayTuningSet(&relay);
					} else if ( high && hysteresis && error < 0 ) { /* FALL DETECTED */
						lastLowTime = thisTime;
						high = false;
					}
				}
					break;

				case STABILIZATIONDESIRED_STABILIZATIONMODE_NONE:
					actuatorDesiredAxis[i] = bound(attitudeDesiredAxis[i],1.0f);
					break;
				default:
					error = true;
					break;
			}
		}

		// Piro compensation rotates the virtual flybar by yaw step to keep it
		// rotated to external world
		if (vbar_piro_comp) {
			const float F_PI = 3.14159f;
			float cy = cosf(gyro_filtered[2] / 180.0f * F_PI * dT);
			float sy = sinf(gyro_filtered[2] / 180.0f * F_PI * dT);

			float vbar_pitch = cy * vbar_integral[1] - sy * vbar_integral[0];
			float vbar_roll = sy * vbar_integral[1] + cy * vbar_integral[0];

			vbar_integral[1] = vbar_pitch;
			vbar_integral[0] = vbar_roll;
		}

#if defined(DIAGNOSTICS)
		RateDesiredSet(&rateDesired);
#endif

		// Save dT
		actuatorDesired.UpdateTime = dT * 1000;
		actuatorDesired.Throttle = stabDesired.Throttle;

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
 * Update one of the PID structures with the input error and timestep
 * @param pid Pointer to the PID structure
 * @param[in] err The error on for this controller
 * @param[in] dT The time step since the last update
 */
float ApplyPid(pid_type * pid, const float err, float dT)
{
	float diff = (err - pid->lastErr);
	pid->lastErr = err;
	
	// Scale up accumulator by 1000 while computing to avoid losing precision
	pid->iAccumulator += err * (pid->i * dT * 1000.0f);
	pid->iAccumulator = bound(pid->iAccumulator, pid->iLim * 1000.0f);
	return ((err * pid->p) + pid->iAccumulator / 1000.0f + (diff * pid->d / dT));
}


/**
 * Clear the accumulators and derivatives for all the axes
 */
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
	
	// Need to store the vbar sensitivity
	vbar_sensitivity[0] = settings.VbarSensitivity[0];
	vbar_sensitivity[1] = settings.VbarSensitivity[1];
	vbar_sensitivity[2] = settings.VbarSensitivity[2];
	
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
	vbar_gyros_suppress = settings.VbarGyroSuppress;
	vbar_piro_comp = settings.VbarPiroComp == STABILIZATIONSETTINGS_VBARPIROCOMP_TRUE;
}


/**
 * @}
 * @}
 */

