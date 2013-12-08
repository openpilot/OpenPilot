/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup StabilizationModule Stabilization Module
 * @brief Stabilization PID loops in an airframe type independent manner
 * @note This object updates the @ref ActuatorDesired "Actuator Desired" based on the
 * PID loops on the @ref AttitudeDesired "Attitude Desired" and @ref AttitudeState "Attitude State"
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

#include <openpilot.h>
#include <pios_struct_helper.h>
#include "stabilization.h"
#include "stabilizationsettings.h"
#include "actuatordesired.h"
#include "ratedesired.h"
#include "relaytuning.h"
#include "relaytuningsettings.h"
#include "stabilizationdesired.h"
#include "attitudestate.h"
#include "airspeedstate.h"
#include "gyrostate.h"
#include "flightstatus.h"
#include "manualcontrol.h" // Just to get a macro
#include "taskinfo.h"

// Math libraries
#include "CoordinateConversions.h"
#include "pid.h"
#include "sin_lookup.h"

// Includes for various stabilization algorithms
#include "relay_tuning.h"
#include "virtualflybar.h"

// Includes for various stabilization algorithms
#include "relay_tuning.h"

// Private constants
#define MAX_QUEUE_SIZE      1

#if defined(PIOS_STABILIZATION_STACK_SIZE)
#define STACK_SIZE_BYTES    PIOS_STABILIZATION_STACK_SIZE
#else
#define STACK_SIZE_BYTES    724
#endif

#define TASK_PRIORITY       (tskIDLE_PRIORITY + 4)
#define FAILSAFE_TIMEOUT_MS 30

enum { PID_RATE_ROLL, PID_RATE_PITCH, PID_RATE_YAW, PID_ROLL, PID_PITCH, PID_YAW, PID_MAX };


// Private variables
static xTaskHandle taskHandle;
static StabilizationSettingsData settings;
static xQueueHandle queue;
float gyro_alpha = 0;
float axis_lock_accum[3] = { 0, 0, 0 };
uint8_t max_axis_lock     = 0;
uint8_t max_axislock_rate = 0;
float weak_leveling_kp    = 0;
uint8_t weak_leveling_max = 0;
bool lowThrottleZeroIntegral;
bool lowThrottleZeroAxis[MAX_AXES];
float vbar_decay = 0.991f;
struct pid pids[PID_MAX];
static float horizon_begin2, horizon_begin3;

// Private functions
static void stabilizationTask(void *parameters);
static float bound(float val, float range);
static void ZeroPids(void);
static void SettingsUpdatedCb(UAVObjEvent *ev);

/**
 * Module initialization
 */
int32_t StabilizationStart()
{
    // Initialize variables
    // Create object queue
    queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

    // Listen for updates.
    // AttitudeStateConnectQueue(queue);
    GyroStateConnectQueue(queue);

    StabilizationSettingsConnectCallback(SettingsUpdatedCb);
    SettingsUpdatedCb(StabilizationSettingsHandle());

    // Start main task
    xTaskCreate(stabilizationTask, (signed char *)"Stabilization", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &taskHandle);
    PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_STABILIZATION, taskHandle);
#ifdef PIOS_INCLUDE_WDG
    PIOS_WDG_RegisterFlag(PIOS_WDG_STABILIZATION);
#endif
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
#ifdef DIAG_RATEDESIRED
    RateDesiredInitialize();
#endif
#ifdef REVOLUTION
    AirspeedStateInitialize();
#endif
    // Code required for relay tuning
    sin_lookup_initalize();
    RelayTuningSettingsInitialize();
    RelayTuningInitialize();

    return 0;
}

MODULE_INITCALL(StabilizationInitialize, StabilizationStart);

/**
 * Module task
 */
static void stabilizationTask(__attribute__((unused)) void *parameters)
{
    UAVObjEvent ev;

    uint32_t timeval = PIOS_DELAY_GetRaw();

    ActuatorDesiredData actuatorDesired;
    StabilizationDesiredData stabDesired;
    RateDesiredData rateDesired;
    AttitudeStateData attitudeState;
    GyroStateData gyroStateData;
    FlightStatusData flightStatus;

#ifdef REVOLUTION
    AirspeedStateData airspeedState;
#endif

    SettingsUpdatedCb((UAVObjEvent *)NULL);

    // Main task loop
    ZeroPids();
    while (1) {
        float dT;

#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_STABILIZATION);
#endif

        // Wait until the AttitudeRaw object is updated, if a timeout then go to failsafe
        if (xQueueReceive(queue, &ev, FAILSAFE_TIMEOUT_MS / portTICK_RATE_MS) != pdTRUE) {
            AlarmsSet(SYSTEMALARMS_ALARM_STABILIZATION, SYSTEMALARMS_ALARM_WARNING);
            continue;
        }

        dT = PIOS_DELAY_DiffuS(timeval) * 1.0e-6f;
        timeval = PIOS_DELAY_GetRaw();

        FlightStatusGet(&flightStatus);
        StabilizationDesiredGet(&stabDesired);
        AttitudeStateGet(&attitudeState);
        GyroStateGet(&gyroStateData);
#ifdef DIAG_RATEDESIRED
        RateDesiredGet(&rateDesired);
#endif
#ifdef REVOLUTION
        float speedScaleFactor;
        // Scale PID coefficients based on current airspeed estimation - needed for fixed wing planes
        AirspeedStateGet(&airspeedState);
        if (settings.ScaleToAirspeed < 0.1f || airspeedState.CalibratedAirspeed < 0.1f) {
            // feature has been turned off
            speedScaleFactor = 1.0f;
        } else {
            // scale the factor to be 1.0 at the specified airspeed (for example 10m/s) but scaled by 1/speed^2
            speedScaleFactor = (settings.ScaleToAirspeed * settings.ScaleToAirspeed) / (airspeedState.CalibratedAirspeed * airspeedState.CalibratedAirspeed);
            if (speedScaleFactor < settings.ScaleToAirspeedLimits.Min) {
                speedScaleFactor = settings.ScaleToAirspeedLimits.Min;
            }
            if (speedScaleFactor > settings.ScaleToAirspeedLimits.Max) {
                speedScaleFactor = settings.ScaleToAirspeedLimits.Max;
            }
        }
#else
        const float speedScaleFactor = 1.0f;
#endif

#if defined(PIOS_QUATERNION_STABILIZATION)
        // Quaternion calculation of error in each axis.  Uses more memory.
        float rpy_desired[3];
        float q_desired[4];
        float q_error[4];
        float local_error[3];

        // Essentially zero errors for anything in rate or none
        if (stabDesired.StabilizationMode.Roll == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) {
            rpy_desired[0] = stabDesired.Roll;
        } else {
            rpy_desired[0] = attitudeState.Roll;
        }

        if (stabDesired.StabilizationMode.Pitch == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) {
            rpy_desired[1] = stabDesired.Pitch;
        } else {
            rpy_desired[1] = attitudeState.Pitch;
        }

        if (stabDesired.StabilizationMode.Yaw == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) {
            rpy_desired[2] = stabDesired.Yaw;
        } else {
            rpy_desired[2] = attitudeState.Yaw;
        }

        RPY2Quaternion(rpy_desired, q_desired);
        quat_inverse(q_desired);
        quat_mult(q_desired, &attitudeState.q1, q_error);
        quat_inverse(q_error);
        Quaternion2RPY(q_error, local_error);

#else /* if defined(PIOS_QUATERNION_STABILIZATION) */
        // Simpler algorithm for CC, less memory
        float local_error[3] = { stabDesired.Roll  - attitudeState.Roll,
                                 stabDesired.Pitch - attitudeState.Pitch,
                                 stabDesired.Yaw   - attitudeState.Yaw };
        // find shortest way
        float modulo = fmodf(local_error[2] + 180.0f, 360.0f);
        if (modulo < 0) {
            local_error[2] = modulo + 180.0f;
        } else {
            local_error[2] = modulo - 180.0f;
        }
#endif /* if defined(PIOS_QUATERNION_STABILIZATION) */

        float gyro_filtered[3];
        gyro_filtered[0] = gyro_filtered[0] * gyro_alpha + gyroStateData.x * (1 - gyro_alpha);
        gyro_filtered[1] = gyro_filtered[1] * gyro_alpha + gyroStateData.y * (1 - gyro_alpha);
        gyro_filtered[2] = gyro_filtered[2] * gyro_alpha + gyroStateData.z * (1 - gyro_alpha);

        float *stabDesiredAxis     = &stabDesired.Roll;
        float *actuatorDesiredAxis = &actuatorDesired.Roll;
        float *rateDesiredAxis     = &rateDesired.Roll;

        ActuatorDesiredGet(&actuatorDesired);

        // A flag to track which stabilization mode each axis is in
        static uint8_t previous_mode[MAX_AXES] = { 255, 255, 255 };
        bool error = false;

        // Run the selected stabilization algorithm on each axis:
        for (uint8_t i = 0; i < MAX_AXES; i++) {
            // Check whether this axis mode needs to be reinitialized
            bool reinit = (cast_struct_to_array(stabDesired.StabilizationMode, stabDesired.StabilizationMode.Roll)[i] != previous_mode[i]);
            previous_mode[i] = cast_struct_to_array(stabDesired.StabilizationMode, stabDesired.StabilizationMode.Roll)[i];

            // Apply the selected control law
            switch (cast_struct_to_array(stabDesired.StabilizationMode, stabDesired.StabilizationMode.Roll)[i]) {
            case STABILIZATIONDESIRED_STABILIZATIONMODE_RATE:
                if (reinit) {
                    pids[PID_RATE_ROLL + i].iAccumulator = 0;
                }

                // Store to rate desired variable for storing to UAVO
                // this bound() seems unnecessary
                rateDesiredAxis[i] =
                    bound(stabDesiredAxis[i], cast_struct_to_array(settings.ManualRate, settings.ManualRate.Roll)[i]);

                // Compute the inner loop
                actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i], speedScaleFactor, rateDesiredAxis[i], gyro_filtered[i], dT);
                actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);

                break;

            case STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE:
                if (reinit) {
                    pids[PID_ROLL + i].iAccumulator = 0;
                    pids[PID_RATE_ROLL + i].iAccumulator = 0;
                }

                // Compute the outer loop
                rateDesiredAxis[i] = pid_apply(&pids[PID_ROLL + i], local_error[i], dT);
                rateDesiredAxis[i] = bound(rateDesiredAxis[i],
                                           cast_struct_to_array(settings.MaximumRate, settings.MaximumRate.Roll)[i]);

                // Compute the inner loop
                actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i], speedScaleFactor, rateDesiredAxis[i], gyro_filtered[i], dT);
                actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);

                break;

            case STABILIZATIONDESIRED_STABILIZATIONMODE_MULTIWIIHORIZON:
            {
                if (reinit) {
                    pids[PID_ROLL + i].iAccumulator = 0;
                    pids[PID_RATE_ROLL + i].iAccumulator = 0;
                }

                // Taper the Attitude mode PID's
                // - from full configured value at center stick
                // - to zero at max stick
                // Otherwise this is just Attitude mode

                // Or just calculate both the Rate actuator and the Attitude actuator
                // - and parameterize a weighted average of the two

                // Compute what Rate mode would give for this stick angle

                // Store to rate desired variable for storing to UAVO
                // this bound() seems unnecessary both here and in Rate mode where this came from
                float rateDesiredAxisRate;
                rateDesiredAxisRate = bound(stabDesiredAxis[i], 1.0f)
                    * cast_struct_to_array(settings.ManualRate, settings.ManualRate.Roll)[i];

                // Compute the inner loop
                //actuatorDesiredAxisRate = pid_apply_setpoint(&pids[PID_RATE_ROLL + i], speedScaleFactor, rateDesiredAxisRate, gyro_filtered[i], dT);
                //actuatorDesiredAxisRate = bound(actuatorDesiredAxisRate, 1.0f);

                // Compute what Attitude mode would give for this stick angle

                // stabDesired for this mode is [-1.0f,+1.0f]
                // - multiply by Attitude mode max angle to get desired angle
                // - subtract off the actual angle to get the angle error
                local_error[0] = stabDesired.Roll  * settings.RollMax  - attitudeState.Roll;
                local_error[1] = stabDesired.Pitch * settings.PitchMax - attitudeState.Pitch;
                local_error[2] = stabDesired.Yaw   * settings.YawMax   - attitudeState.Yaw;
                // find shortest way
                modulo = fmodf(local_error[2] + 180.0f, 360.0f);
                if (modulo < 0) {
                    local_error[2] = modulo + 180.0f;
                } else {
                    local_error[2] = modulo - 180.0f;
                }

                // Compute the outer loop
                float rateDesiredAxisAttitude;
                rateDesiredAxisAttitude = pid_apply(&pids[PID_ROLL + i], local_error[i], dT);
                rateDesiredAxisAttitude = bound(rateDesiredAxisAttitude,
                                           cast_struct_to_array(settings.MaximumRate, settings.MaximumRate.Roll)[i]);

                // Using max() rather than sqrt() for cpu speed;
                // - this makes the stick region into a square;
                // - this is a feature!
                // - hold a roll angle and add just pitch without it jumping into rate mode
                // magnitude = sqrt(cmd->Roll*cmd->Roll + cmd->Pitch*cmd->Pitch);
                float magnitude;
                magnitude = fmaxf(fabsf(stabDesired.Roll), fabsf(stabDesired.Pitch));
                rateDesiredAxis[i] = (1.0f-magnitude) * rateDesiredAxisAttitude + magnitude * rateDesiredAxisRate;

                // Compute the inner loop
                actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i], speedScaleFactor, rateDesiredAxis[i], gyro_filtered[i], dT);
                actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);

/*
need a way of keeping both iAccumulator's from winding up
do we need an anti-windup alpha setting? (factor)
  
it should be parametric with magnitude
*/

// At magnitudes close to one,  the Attitude accumulator gets zeroed
pids[PID_ROLL      + i].iAccumulator *= (1.0f-magnitude); // * factor;
// At magnitudes close to zero, the Rate     accumulator gets zeroed
pids[PID_RATE_ROLL + i].iAccumulator *= magnitude;        // * factor;

// TODO: put a factor in?

// TODO: fix weak leveling?

                break;
            }

            case STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR:

                // Store for debugging output
                rateDesiredAxis[i] = stabDesiredAxis[i];

                // Run a virtual flybar stabilization algorithm on this axis
                stabilization_virtual_flybar(gyro_filtered[i], rateDesiredAxis[i], &actuatorDesiredAxis[i], dT, reinit, i, &settings);

                break;

            case STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING:
            {
                if (reinit) {
                    pids[PID_RATE_ROLL + i].iAccumulator = 0;
                }

                float weak_leveling = local_error[i] * weak_leveling_kp;
                weak_leveling = bound(weak_leveling, weak_leveling_max);

                // Compute desired rate as input biased towards leveling
                rateDesiredAxis[i]     = stabDesiredAxis[i] + weak_leveling;
                actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i], speedScaleFactor, rateDesiredAxis[i], gyro_filtered[i], dT);
                actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);

                break;
            }

/*
perhaps I just need to run both rate and attitude in parallel
and have magnitude parameterize between the two actuator values

or I need to do a weak leveling
with a full attitude PID

local_error is wrong here.  it is rate - angle

I could have stabilization.c send an angle so that local_error is correct
no, just use +-1.0f and scale it here (into rate or attitude) according to magnitude

what will it act like if the max angle is very small?
*/

            case STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK:
                if (reinit) {
                    pids[PID_RATE_ROLL + i].iAccumulator = 0;
                }

                if (fabsf(stabDesiredAxis[i]) > max_axislock_rate) {
                    // While getting strong commands act like rate mode
                    rateDesiredAxis[i] = stabDesiredAxis[i];
                    axis_lock_accum[i] = 0;
                } else {
                    // For weaker commands or no command simply attitude lock (almost) on no gyro change
                    axis_lock_accum[i] += (stabDesiredAxis[i] - gyro_filtered[i]) * dT;
                    axis_lock_accum[i]  = bound(axis_lock_accum[i], max_axis_lock);
                    rateDesiredAxis[i]  = pid_apply(&pids[PID_ROLL + i], axis_lock_accum[i], dT);
                }

                rateDesiredAxis[i]     = bound(rateDesiredAxis[i],
                                               cast_struct_to_array(settings.ManualRate, settings.ManualRate.Roll)[i]);

                actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i], speedScaleFactor, rateDesiredAxis[i], gyro_filtered[i], dT);
                actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);

                break;

            case STABILIZATIONDESIRED_STABILIZATIONMODE_HORIZON:
            case STABILIZATIONDESIRED_STABILIZATIONMODE_HORIZON2:
            {
                // Require Horizon mode to be configured on both roll and pitch, or neither;
                // - never just one of them
                // If Attitude mode were only on roll, you could do a pitch and a yaw
                // - to get an effective roll without even moving the roll stick

                // Magnitude of stick angle [0,1]
                float magnitude;
                // old_magnitude is used to detect region changes to zero iAccumulator
                static float old_magnitude; // = 0.0f
                // Using max() rather than sqrt() for cpu speed;
                // - this makes the stick region into a square;
                // - this is a feature!
                // - hold a roll angle and add just pitch without it jumping into rate mode
                // magnitude = sqrt(cmd->Roll*cmd->Roll + cmd->Pitch*cmd->Pitch);
                magnitude = fmaxf(fabsf(stabDesired.Roll), fabsf(stabDesired.Pitch));

                if (reinit) {
                    pids[PID_ROLL + i].iAccumulator = 0;
                    pids[PID_RATE_ROLL + i].iAccumulator = 0;
                    // Keep iAcc from getting zeroed twice in case someone worries about that
                    // - it wouldn't hurt, but it would not be deterministic and clean
                    old_magnitude = magnitude;
                }

/*
make sure begin2 and 3 are [0,1]
don't modify stabDesiredAxis[i], put it in temps instead
  two cases (fix begin3 too)
better yet, scope factor out into the open, default it to 1.0f
  and use it in calculating local_error or when using local_error
problem: with this, stabDesired artificially grows to 1 at begin3-eps
  so local_error jumps down at begin3 because stabDesired jumps down from
  an inflated 1 at begin3-eps to an uninflated 2/3 at begin3
avoiding the inflation means it won't go to Attitude max angle at 2/3'rds stick
right this is OK
  there is a switch to rate mode at begin3
  the code thinks it went from full stick in attitude mode
  to 2/3rds stick in rate mode
  and that is OK
*/

                // If in either region 1 or 2 we use Attitude mode
                if (magnitude < horizon_begin3) {
                    // If there was an Rate -> Attitude mode change
                    if (old_magnitude >= horizon_begin3) {
                        pids[PID_ROLL + i].iAccumulator = 0;
                        pids[PID_RATE_ROLL + i].iAccumulator = 0;
                    }
                    // In region 1, parametric stays zero to force use of default rate
                    // In region 2, parametric goes from 0 -> 1
                    // - as stick angle (magnitude, any direction) goes from begin2 to begin3
                    float parametric = 0.0f;

                    // factor is the amount that the stick angle needs to be increased
                    // - so we get only normal attitude mode stick sensitivity at begin2
                    // - but we get full max attitude angle at begin3 (less than full stick)
                    // The stick is passed on as a full = 1.0f at begin3
                    // In region 1, factor stays at 1.0f
                    // In region 2, factor grows from 1 to a factor that increases the stick
                    // - to 1.0f by begin3
                    // - for the default config, that is 3/2 because default begin3 is 2/3
                    float factor = 1.0f;

                    // if in region 1 we use the commanded stick angle and configured rates
                    // - first region is unmodified attitude mode
                    // If in region 2 we tweak the stick position and the aircraft rotation rate
                    // Second region is increased sensitivity attitude mode
                    // Max rotation rate is still taken from attitude mode and tapered from full to zero
                    // - over the range begin2 -> begin3
                    // At the beginning of this region, the unmodified stick value is used as is so the transition is smooth
                    // At the end of this region, the stick value is (+-) 1.0
                    // This means that the end of this region acts like attitude mode max angle
                    // - so this region acts like a more sensitive attitude mode
                    // Note that if region 2 length is zero it can't get in here
                    // - that is important to avoid divide by zero
                    // - and also a big bump in the stick when region 2 is very small and close to center stick
                    if (magnitude >= horizon_begin2) {
                        parametric = (magnitude - horizon_begin2) / (horizon_begin3 - horizon_begin2);

                        // Increase the stick angle for region 2
                        factor = (parametric * ((1.0f/horizon_begin3)-1.0f) + 1.0f);
                    }

                    // for HORIZON, use the initial (full) rate for the whole of region2
                    // this is thought to be not good leave it here to compare HORIZON and HORIZON2
                    if (cast_struct_to_array(stabDesired.StabilizationMode, stabDesired.StabilizationMode.Roll)[i] == STABILIZATIONDESIRED_STABILIZATIONMODE_HORIZON) {
                        // Leave the rotation rate at maximum for HORIZON (not HORIZON2)
                        parametric = 0.0f;
                    }

                    // stabDesired for this mode is [-1.0f,+1.0f]
                    // - multiply by Attitude mode max angle to get desired angle
                    // - subtract off the actual angle to get the angle error
                    local_error[0] = stabDesired.Roll  * settings.RollMax  * factor - attitudeState.Roll;
                    local_error[1] = stabDesired.Pitch * settings.PitchMax * factor - attitudeState.Pitch;
                    local_error[2] = stabDesired.Yaw   * settings.YawMax   * factor - attitudeState.Yaw;
                    // find shortest way
                    modulo = fmodf(local_error[2] + 180.0f, 360.0f);
                    if (modulo < 0) {
                        local_error[2] = modulo + 180.0f;
                    } else {
                        local_error[2] = modulo - 180.0f;
                    }

                    // Compute the inner loop
                    rateDesiredAxis[i] = pid_apply(&pids[PID_ROLL + i], local_error[i], dT);
                    rateDesiredAxis[i] = bound(rateDesiredAxis[i],
                                               cast_struct_to_array(settings.MaximumRate, settings.MaximumRate.Roll)[i] * (1.0f-parametric));

                    // Compute the outer loop
                    actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i], speedScaleFactor, rateDesiredAxis[i], gyro_filtered[i], dT);
                    actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);
                }

                // Third region is a 'compressed' Rate mode
                // The beginning of this region has rate 0, the end has max Rate mode rate
                // That means that the very beginning of this region doesn't rotate much
                else {
                    // If there was an Attitude -> Rate mode change
                    if (old_magnitude < horizon_begin3) {
                        pids[PID_RATE_ROLL + i].iAccumulator = 0;
                    }
                    // parametric goes from 0 -> 1
                    // - as stick angle (magnitude, any direction) goes from begin3 to 'max stick'
                    float parametric;
                    parametric = (magnitude - horizon_begin3) / (1.0f - horizon_begin3);

                    // Scale the stick commands from magnitude [begin3, 1.0f] to [0, 1.0f]
                    // We see as input, a large stick angle (in the last region) [begin3, 1.0f]
                    // - but pass that on with magnitude [0, 1.0f] and let the following code see that

                    // Store to rate desired variable for storing to UAVO
                    // this bound() seems unnecessary, here and in Rate mode where this came from
                    // stabDesiredAxis[i] comes here unscaled ([-1.0f,+1.0f])
                    // - so we have to scale it to degrees per second
                    // - multiply it by Rate mode max rate
                    rateDesiredAxis[i] =
                        bound(stabDesiredAxis[i], 1.0f)
                            * cast_struct_to_array(settings.ManualRate, settings.ManualRate.Roll)[i]
                            * parametric;

                    // Compute the inner loop
                    actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i], speedScaleFactor, rateDesiredAxis[i], gyro_filtered[i], dT);
                    actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);
                }
                // track mode changes
                old_magnitude = magnitude;

                break;
            }

            case STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE:
                // Store to rate desired variable for storing to UAVO
                rateDesiredAxis[i] = bound(stabDesiredAxis[i],
                                           cast_struct_to_array(settings.ManualRate, settings.ManualRate.Roll)[i]);

                // Run the relay controller which also estimates the oscillation parameters
                stabilization_relay_rate(rateDesiredAxis[i] - gyro_filtered[i], &actuatorDesiredAxis[i], i, reinit);
                actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);

                break;

            case STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE:
                if (reinit) {
                    pids[PID_ROLL + i].iAccumulator = 0;
                }

                // Compute the outer loop like attitude mode
                rateDesiredAxis[i] = pid_apply(&pids[PID_ROLL + i], local_error[i], dT);
                rateDesiredAxis[i] = bound(rateDesiredAxis[i],
                                           cast_struct_to_array(settings.MaximumRate, settings.MaximumRate.Roll)[i]);

                // Run the relay controller which also estimates the oscillation parameters
                stabilization_relay_rate(rateDesiredAxis[i] - gyro_filtered[i], &actuatorDesiredAxis[i], i, reinit);
                actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);

                break;

            case STABILIZATIONDESIRED_STABILIZATIONMODE_NONE:
                actuatorDesiredAxis[i] = bound(stabDesiredAxis[i], 1.0f);
                break;
            default:
                error = true;
                break;
            }
        }

        if (settings.VbarPiroComp == STABILIZATIONSETTINGS_VBARPIROCOMP_TRUE) {
            stabilization_virtual_flybar_pirocomp(gyro_filtered[2], dT);
        }

#ifdef DIAG_RATEDESIRED
        RateDesiredSet(&rateDesired);
#endif

        // Save dT
        actuatorDesired.UpdateTime = dT * 1000;
        actuatorDesired.Throttle   = stabDesired.Throttle;

        // Suppress desired output while disarmed or throttle low, for configured axis
        if (flightStatus.Armed != FLIGHTSTATUS_ARMED_ARMED || stabDesired.Throttle < 0) {
            if (lowThrottleZeroAxis[ROLL]) {
                actuatorDesired.Roll = 0.0f;
            }

            if (lowThrottleZeroAxis[PITCH]) {
                actuatorDesired.Pitch = 0.0f;
            }

            if (lowThrottleZeroAxis[YAW]) {
                actuatorDesired.Yaw = 0.0f;
            }
        }

        if (PARSE_FLIGHT_MODE(flightStatus.FlightMode) != FLIGHTMODE_MANUAL) {
            ActuatorDesiredSet(&actuatorDesired);
        } else {
            // Force all axes to reinitialize when engaged
            for (uint8_t i = 0; i < MAX_AXES; i++) {
                previous_mode[i] = 255;
            }
        }

        if (flightStatus.Armed != FLIGHTSTATUS_ARMED_ARMED ||
            (lowThrottleZeroIntegral && stabDesired.Throttle < 0)) {
            // Force all axes to reinitialize when engaged
            for (uint8_t i = 0; i < MAX_AXES; i++) {
                previous_mode[i] = 255;
            }
        }

        // Clear or set alarms.  Done like this to prevent toggline each cycle
        // and hammering system alarms
        if (error) {
            AlarmsSet(SYSTEMALARMS_ALARM_STABILIZATION, SYSTEMALARMS_ALARM_ERROR);
        } else {
            AlarmsClear(SYSTEMALARMS_ALARM_STABILIZATION);
        }
    }
}


/**
 * Clear the accumulators and derivatives for all the axes
 */
static void ZeroPids(void)
{
    for (uint32_t i = 0; i < PID_MAX; i++) {
        pid_zero(&pids[i]);
    }


    for (uint8_t i = 0; i < 3; i++) {
        axis_lock_accum[i] = 0.0f;
    }
}


/**
 * Bound input value between limits
 */
static float bound(float val, float range)
{
    if (val < -range) {
        val = -range;
    } else if (val > range) {
        val = range;
    }
    return val;
}

static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    StabilizationSettingsGet(&settings);

    // Set the roll rate PID constants
    pid_configure(&pids[PID_RATE_ROLL], settings.RollRatePID.Kp,
                  settings.RollRatePID.Ki,
                  pids[PID_RATE_ROLL].d = settings.RollRatePID.Kd,
                  pids[PID_RATE_ROLL].iLim = settings.RollRatePID.ILimit);

    // Set the pitch rate PID constants
    pid_configure(&pids[PID_RATE_PITCH], settings.PitchRatePID.Kp,
                  pids[PID_RATE_PITCH].i = settings.PitchRatePID.Ki,
                  pids[PID_RATE_PITCH].d = settings.PitchRatePID.Kd,
                  pids[PID_RATE_PITCH].iLim = settings.PitchRatePID.ILimit);

    // Set the yaw rate PID constants
    pid_configure(&pids[PID_RATE_YAW], settings.YawRatePID.Kp,
                  pids[PID_RATE_YAW].i = settings.YawRatePID.Ki,
                  pids[PID_RATE_YAW].d = settings.YawRatePID.Kd,
                  pids[PID_RATE_YAW].iLim = settings.YawRatePID.ILimit);

    // Set the roll attitude PI constants
    pid_configure(&pids[PID_ROLL], settings.RollPI.Kp,
                  settings.RollPI.Ki, 0,
                  pids[PID_ROLL].iLim = settings.RollPI.ILimit);

    // Set the pitch attitude PI constants
    pid_configure(&pids[PID_PITCH], settings.PitchPI.Kp,
                  pids[PID_PITCH].i = settings.PitchPI.Ki, 0,
                  settings.PitchPI.ILimit);

    // Set the yaw attitude PI constants
    pid_configure(&pids[PID_YAW], settings.YawPI.Kp,
                  settings.YawPI.Ki, 0,
                  settings.YawPI.ILimit);

    // Set up the derivative term
    pid_configure_derivative(settings.DerivativeCutoff, settings.DerivativeGamma);

    // Maximum deviation to accumulate for axis lock
    max_axis_lock     = settings.MaxAxisLock;
    max_axislock_rate = settings.MaxAxisLockRate;

    // Settings for weak leveling
    weak_leveling_kp  = settings.WeakLevelingKp;
    weak_leveling_max = settings.MaxWeakLevelingRate;

    // Whether to zero the PID integrals while throttle is low
    lowThrottleZeroIntegral    = settings.LowThrottleZeroIntegral == STABILIZATIONSETTINGS_LOWTHROTTLEZEROINTEGRAL_TRUE;

    // Whether to suppress (zero) the StabilizationDesired output for each axis while disarmed or throttle is low
    lowThrottleZeroAxis[ROLL]  = settings.LowThrottleZeroAxis.Roll == STABILIZATIONSETTINGS_LOWTHROTTLEZEROAXIS_TRUE;
    lowThrottleZeroAxis[PITCH] = settings.LowThrottleZeroAxis.Pitch == STABILIZATIONSETTINGS_LOWTHROTTLEZEROAXIS_TRUE;
    lowThrottleZeroAxis[YAW]   = settings.LowThrottleZeroAxis.Yaw == STABILIZATIONSETTINGS_LOWTHROTTLEZEROAXIS_TRUE;

    // The dT has some jitter iteration to iteration that we don't want to
    // make thie result unpredictable.  Still, it's nicer to specify the constant
    // based on a time (in ms) rather than a fixed multiplier.  The error between
    // update rates on OP (~300 Hz) and CC (~475 Hz) is negligible for this
    // calculation
    const float fakeDt = 0.0025f;
    if (settings.GyroTau < 0.0001f) {
        gyro_alpha = 0; // not trusting this to resolve to 0
    } else {
        gyro_alpha = expf(-fakeDt / settings.GyroTau);
    }

    // Compute time constant for vbar decay term based on a tau
    vbar_decay = expf(-fakeDt / settings.VbarTau);

    // The first Horizon Mode region begins at (stick position) 0.0f (center),
    // - other regions are 0.0f<=begin<=1.0f
    // Beginning of region 2 is limited to [0,99] by GCS
    // Length    of region 2 is limited to [0,99] by GCS
    // Uavo's HorizonModeRegionBegin and HorizonModeRegionLength are currently integers
    // - representing percent of max stick deflection
    horizon_begin2 = settings.HorizonModeRegionBegin / 100.0f;
    // This is done so that it will never enter region 2 if region 2 length is zero
    // It shouldn't, because (float)a + (0/100.0f) should exactly equal (float)a
    // - but that 0/100.0f might be epsilon if somebody converts the length to float in the future
    if (settings.HorizonModeRegionLength > 0) {
        horizon_begin3 = horizon_begin2 + settings.HorizonModeRegionLength / 100.0f;
        // this bounding could be moved into the GCS
        if (horizon_begin3 > 0.99f) {
            horizon_begin3 = 0.99f;
        }
        if (horizon_begin2 > horizon_begin3-0.005f) {
            horizon_begin2 = horizon_begin3;
        }
    }
    else {
        horizon_begin3 = horizon_begin2;
    }
    // To avoid division by zero...
    // - we have guarenteed that begin3 is not zero
    // - we have guarenteed that begin3 is not one
    // begin3-begin2 close to zero but not zero is another thing to be careful of
}


/**
 * @}
 * @}
 */
