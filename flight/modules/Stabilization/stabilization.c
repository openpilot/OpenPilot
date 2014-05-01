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
#include "stabilizationbank.h"
#include "stabilizationsettingsbank1.h"
#include "stabilizationsettingsbank2.h"
#include "stabilizationsettingsbank3.h"
#include "actuatordesired.h"
#include "ratedesired.h"
#include "relaytuning.h"
#include "relaytuningsettings.h"
#include "stabilizationdesired.h"
#include "attitudestate.h"
#include "airspeedstate.h"
#include "gyrostate.h"
#include "flightstatus.h"
#include "manualcontrolsettings.h"
#include "manualcontrolcommand.h"
#include "flightmodesettings.h"
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
#define UPDATE_EXPECTED     (1.0f / 666.0f)
#define UPDATE_MIN          1.0e-6f
#define UPDATE_MAX          1.0f
#define UPDATE_ALPHA        1.0e-2f

#define MAX_QUEUE_SIZE      1

#if defined(PIOS_STABILIZATION_STACK_SIZE)
#define STACK_SIZE_BYTES    PIOS_STABILIZATION_STACK_SIZE
#else
#define STACK_SIZE_BYTES    860
#endif

#define TASK_PRIORITY       (tskIDLE_PRIORITY + 3) // FLIGHT CONTROL priority
#define FAILSAFE_TIMEOUT_MS 30

// The PID_RATE_ROLL set is used by Rate mode and the rate portion of Attitude mode
// The PID_RATE set is used by the attitude portion of Attitude mode
enum { PID_RATE_ROLL, PID_RATE_PITCH, PID_RATE_YAW, PID_ROLL, PID_PITCH, PID_YAW, PID_MAX };
enum { RATE_P, RATE_I, RATE_D, RATE_LIMIT, RATE_OFFSET };
enum { ATT_P, ATT_I, ATT_LIMIT, ATT_OFFSET };

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
float vbar_decay    = 0.991f;
struct pid pids[PID_MAX];

int cur_flight_mode = -1;

static float rattitude_mode_transition_stick_position;

static float   cruise_control_min_thrust;
static float   cruise_control_max_thrust;
static float   cruise_control_thrust_difference;
static float   cruise_control_max_angle;
static float   cruise_control_max_power_factor;
static float   cruise_control_power_trim;
static float   cruise_control_max_power_factor_angle;
static float   cruise_control_half_power_delay;
static uint8_t cruise_control_flight_mode_switch_pos_enable[FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_NUMELEM];
static uint8_t cruise_control_inverted_thrust_reversing;
static uint8_t cruise_control_inverted_power_output;

// Private functions
static void stabilizationTask(void *parameters);
static float bound(float val, float range);
static void ZeroPids(void);
static void SettingsUpdatedCb(UAVObjEvent *ev);
static void BankUpdatedCb(UAVObjEvent *ev);
static void SettingsBankUpdatedCb(UAVObjEvent *ev);
static float CruiseControlAngleToFactor(float angle);
static float CruiseControlFactorToThrust(float factor, float stick_thrust);
static float CruiseControlLimitThrust(float thrust);

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

    StabilizationBankConnectCallback(BankUpdatedCb);

    StabilizationSettingsBank1ConnectCallback(SettingsBankUpdatedCb);
    StabilizationSettingsBank2ConnectCallback(SettingsBankUpdatedCb);
    StabilizationSettingsBank3ConnectCallback(SettingsBankUpdatedCb);


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
    ManualControlCommandInitialize();
    ManualControlSettingsInitialize();
    FlightStatusInitialize();
    StabilizationDesiredInitialize();
    StabilizationSettingsInitialize();
    StabilizationBankInitialize();
    StabilizationSettingsBank1Initialize();
    StabilizationSettingsBank2Initialize();
    StabilizationSettingsBank3Initialize();
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
    PiOSDeltatimeConfig timeval;

    PIOS_DELTATIME_Init(&timeval, UPDATE_EXPECTED, UPDATE_MIN, UPDATE_MAX, UPDATE_ALPHA);

    ActuatorDesiredData actuatorDesired;
    StabilizationDesiredData stabDesired;
    float throttleDesired;
    RateDesiredData rateDesired;
    AttitudeStateData attitudeState;
    GyroStateData gyroStateData;
    FlightStatusData flightStatus;
    StabilizationBankData stabBank;


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

        // Wait until the Gyro object is updated, if a timeout then go to failsafe
        if (xQueueReceive(queue, &ev, FAILSAFE_TIMEOUT_MS / portTICK_RATE_MS) != pdTRUE) {
            AlarmsSet(SYSTEMALARMS_ALARM_STABILIZATION, SYSTEMALARMS_ALARM_WARNING);
            continue;
        }

        dT = PIOS_DELTATIME_GetAverageSeconds(&timeval);
        FlightStatusGet(&flightStatus);
        StabilizationDesiredGet(&stabDesired);
        ManualControlCommandThrottleGet(&throttleDesired);
        AttitudeStateGet(&attitudeState);
        GyroStateGet(&gyroStateData);
        StabilizationBankGet(&stabBank);
#ifdef DIAG_RATEDESIRED
        RateDesiredGet(&rateDesired);
#endif
        uint8_t flight_mode_switch_position;
        ManualControlCommandFlightModeSwitchPositionGet(&flight_mode_switch_position);

        if (cur_flight_mode != flight_mode_switch_position) {
            cur_flight_mode = flight_mode_switch_position;
            SettingsBankUpdatedCb(NULL);
        }

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
        float local_error[3] = { stabDesired.Roll - attitudeState.Roll,
                                 stabDesired.Pitch - attitudeState.Pitch,
                                 stabDesired.Yaw - attitudeState.Yaw };
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
                rateDesiredAxis[i] =
                    bound(stabDesiredAxis[i], cast_struct_to_array(stabBank.ManualRate, stabBank.ManualRate.Roll)[i]);

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
                                           cast_struct_to_array(stabBank.MaximumRate, stabBank.MaximumRate.Roll)[i]);

                // Compute the inner loop
                actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i], speedScaleFactor, rateDesiredAxis[i], gyro_filtered[i], dT);
                actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);

                break;

            case STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE:
                // A parameterization from Attitude mode at center stick
                // - to Rate mode at full stick
                // This is done by parameterizing to use the rotation rate that Attitude mode
                // - would use at center stick to using the rotation rate that Rate mode
                // - would use at full stick in a weighted average sort of way.
            {
                if (reinit) {
                    pids[PID_ROLL + i].iAccumulator = 0;
                    pids[PID_RATE_ROLL + i].iAccumulator = 0;
                }

                // Compute what Rate mode would give for this stick angle's rate
                // Save Rate's rate in a temp for later merging with Attitude's rate
                float rateDesiredAxisRate;
                rateDesiredAxisRate = bound(stabDesiredAxis[i], 1.0f)
                                      * cast_struct_to_array(stabBank.ManualRate, stabBank.ManualRate.Roll)[i];

                // Compute what Attitude mode would give for this stick angle's rate

                // stabDesired for this mode is [-1.0f,+1.0f]
                // - multiply by Attitude mode max angle to get desired angle
                // - subtract off the actual angle to get the angle error
                // This is what local_error[] holds for Attitude mode
                float attitude_error = stabDesiredAxis[i]
                                       * cast_struct_to_array(stabBank.RollMax, stabBank.RollMax)[i]
                                       - cast_struct_to_array(attitudeState.Roll, attitudeState.Roll)[i];

                // Compute the outer loop just like Attitude mode does
                float rateDesiredAxisAttitude;
                rateDesiredAxisAttitude = pid_apply(&pids[PID_ROLL + i], attitude_error, dT);
                rateDesiredAxisAttitude = bound(rateDesiredAxisAttitude,
                                                cast_struct_to_array(stabBank.ManualRate,
                                                                     stabBank.ManualRate.Roll)[i]);

                // Compute the weighted average rate desired
                // Using max() rather than sqrt() for cpu speed;
                // - this makes the stick region into a square;
                // - this is a feature!
                // - hold a roll angle and add just pitch without the stick sensitivity changing
                // magnitude = sqrt(stabDesired.Roll*stabDesired.Roll + stabDesired.Pitch*stabDesired.Pitch);
                float magnitude;
                magnitude = fmaxf(fabsf(stabDesired.Roll), fabsf(stabDesired.Pitch));

                // modify magnitude to move the Att to Rate transition to the place
                // specified by the user
                // we are looking for where the stick angle == transition angle
                // and the Att rate equals the Rate rate
                // that's where Rate x (1-StickAngle) [Attitude pulling down max X Ratt proportion]
                // == Rate x StickAngle [Rate pulling up according to stick angle]
                // * StickAngle [X Ratt proportion]
                // so 1-x == x*x or x*x+x-1=0 where xE(0,1)
                // (-1+-sqrt(1+4))/2 = (-1+sqrt(5))/2
                // and quadratic formula says that is 0.618033989f
                // I tested 14.01 and came up with .61 without even remembering this number
                // I thought that moving the P,I, and maxangle terms around would change this value
                // and that I would have to take these into account, but varying
                // all P's and I's by factors of 1/2 to 2 didn't change it noticeably
                // and varying maxangle from 4 to 120 didn't either.
                // so for now I'm not taking these into account
                // while working with this, it occurred to me that Attitude mode,
                // set up with maxangle=190 would be similar to Ratt, and it is.
                #define STICK_VALUE_AT_MODE_TRANSITION 0.618033989f

                // the following assumes the transition would otherwise be at 0.618033989f
                // and THAT assumes that Att ramps up to max roll rate
                // when a small number of degrees off of where it should be

                // if below the transition angle (still in attitude mode)
                // '<=' instead of '<' keeps rattitude_mode_transition_stick_position==1.0 from causing DZ
                if (magnitude <= rattitude_mode_transition_stick_position) {
                    magnitude *= STICK_VALUE_AT_MODE_TRANSITION / rattitude_mode_transition_stick_position;
                } else {
                    magnitude = (magnitude - rattitude_mode_transition_stick_position)
                                * (1.0f - STICK_VALUE_AT_MODE_TRANSITION)
                                / (1.0f - rattitude_mode_transition_stick_position)
                                + STICK_VALUE_AT_MODE_TRANSITION;
                }
                rateDesiredAxis[i] = (1.0f - magnitude) * rateDesiredAxisAttitude
                                     + magnitude * rateDesiredAxisRate;

                // Compute the inner loop for the averaged rate
                // actuatorDesiredAxis[i] is the weighted average
                actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i], speedScaleFactor,
                                                            rateDesiredAxis[i], gyro_filtered[i], dT);
                actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);

                break;
            }

            case STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR:
                // Store for debugging output
                rateDesiredAxis[i] = stabDesiredAxis[i];

                // Run a virtual flybar stabilization algorithm on this axis
                stabilization_virtual_flybar(gyro_filtered[i], rateDesiredAxis[i], &actuatorDesiredAxis[i], dT, reinit, i, &settings);

                break;

            case STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING:
                // FIXME: local_error[] is rate - attitude for Weak Leveling
                // The only ramifications are:
                // Weak Leveling Kp is off by a factor of 3 to 12 and may need a different default in GCS
                // Changing Rate mode max rate currently requires a change to Kp
                // That would be changed to Attitude mode max angle affecting Kp
                // Also does not take dT into account
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
                                               cast_struct_to_array(stabBank.ManualRate, stabBank.ManualRate.Roll)[i]);

                actuatorDesiredAxis[i] = pid_apply_setpoint(&pids[PID_RATE_ROLL + i], speedScaleFactor, rateDesiredAxis[i], gyro_filtered[i], dT);
                actuatorDesiredAxis[i] = bound(actuatorDesiredAxis[i], 1.0f);

                break;

            case STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE:
                // Store to rate desired variable for storing to UAVO
                rateDesiredAxis[i] = bound(stabDesiredAxis[i],
                                           cast_struct_to_array(stabBank.ManualRate, stabBank.ManualRate.Roll)[i]);

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
                                           cast_struct_to_array(stabBank.MaximumRate, stabBank.MaximumRate.Roll)[i]);

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
        actuatorDesired.Thrust     = stabDesired.Thrust;

        ///////////////////////////////////////////////////////////////////////
        // Cruise Control
        // modify thrust according to 1/cos(bank angle)
        // to maintain same altitude with changing bank angle
        // but without manually adjusting thrust
        // do it here and all the various flight modes (e.g. Altitude Hold) can use it
        ///////////////////////////////////////////////////////////////////////

        // Detect if the flight mode switch has changed.  If it has, there
        // could be a time gap.  E.g. enabled, then disabled for 30 seconds
        // then enabled again.  Previous_angle will also be invalid because
        // of the time spent with Cruise Control off.
        static bool previous_time_valid; // initially false
        static uint8_t previous_flight_mode_switch_position = 250;
        if (flight_mode_switch_position != previous_flight_mode_switch_position) {
            previous_flight_mode_switch_position = flight_mode_switch_position;
            // Force calculations on this pass (usually every 8th pass),
            // but ignore rate calculations (uses time, previous_time, angle,
            // previous_angle)
            previous_time_valid = false;
        }

        if (flight_mode_switch_position < FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_NUMELEM
            && cruise_control_flight_mode_switch_pos_enable[flight_mode_switch_position] != (uint8_t)0
            && cruise_control_max_power_factor > 0.0001f) {
            static float factor;
            static float previous_angle;
            static uint32_t previous_time;
            static uint8_t calc_count;
            uint32_t time;

            // For multiple, speedy flips this mainly strives to address the
            // fact that (due to thrust delay) thrust didn't average straight
            // down, but at an angle.  For less speedy flips it acts like it
            // used to.  It can be turned off by setting power delay to 0.

            // It takes significant time for the motors of a multi-copter to
            // spin up.  It takes significant time for the collective servo of
            // a CP heli to move from one end to the other.  Both of those are
            // modeled here as linear, i.e. twice as much change takes twice
            // as long. Given a correctly configured maximum delay time this
            // code calculates how far in advance to start the control
            // transition so that half way through the physical transition it
            // is just crossing the transition angle.
            // Example: Rotation rate = 360.  Full stroke delay = 0.2
            // Transition angle 90 degrees.  Start the transition 0.1 second
            // before 90 degrees (36 degrees at 360 deg/sec) and it will be
            // complete 0.1 seconds after 90 degrees.

            // Note that this code only handles the transition to/from inverted
            // thrust.  It doesn't handle the case where thrust is changed a
            // lot in a small angle range when that range is close to 90 degrees.
            // It doesn't handle the small constant "system delay" caused by the
            // delay between reading sensors and actuators beginning to respond.
            // It also assumes that the pilot is holding the throttle constant;
            // when the pilot does change the throttle, the compensation is
            // simply recalculated.

            // This implementation of future thrust isn't perfect.  That would
            // probably require an iterative procedure for solving a
            // transcendental equation of the form linear(x) = 1/cos(x).  It's
            // shortcomings generally don't hurt anything and work better than
            // without it.  It is designed to work perfectly if the pilot is
            // using full thrust during flips and it is only activated if 70% or
            // greater thrust is used.

            time = PIOS_DELAY_GetuS();

            // Get roll and pitch angles, calculate combined angle, and begin
            // the general algorithm.
            // Example: 45 degrees roll plus 45 degrees pitch = 60 degrees
            // Do it every 8th iteration to save CPU.
            if ((time != previous_time && calc_count++ >= 8) || previous_time_valid == false) {
                float angle, angle_unmodified;

                calc_count = 0;

                // spherical right triangle
                // 0.0 <= angle <= 180.0
                angle_unmodified = angle = RAD2DEG(acosf(cos_lookup_deg(attitudeState.Roll)
                                                   * cos_lookup_deg(attitudeState.Pitch)));

                // Calculate rate as a combined (roll and pitch) bank angle
                // change; in degrees per second.  Rate is calculated over the
                // most recent 8 loops through stabilization.  We could have
                // asked the gyros.  This is probably cheaper.
                if (previous_time_valid) {
                    float rate;

                    // rate can be negative.
                    rate = (angle - previous_angle) / ((float) (time - previous_time) / 1000000.0f);

                    // Define "within range" to be those transitions that should
                    // be executing now.  Recall that each impulse transition is
                    // spread out over a range of time / angle.

                    // There is only one transition and the high power level for
                    // it is either:
                    // 1/fabsf(cos(angle)) * current thrust
                    // or max power factor * current thrust
                    // or full thrust
                    // You can cross the transition with angle either increasing
                    // or decreasing (rate positive or negative).

                    // Thrust is never boosted for negative values of
                    // actuatorDesired.Thrust (negative stick values)
                    //
                    // When the aircraft is upright, thrust is always boosted
                    // . for positive values of actuatorDesired.Thrust
                    // When the aircraft is inverted, thrust is sometimes
                    // . boosted or reversed (or combinations thereof) or zeroed
                    // . for positive values of actuatorDesired.Thrust
                    // It depends on the inverted power settings.
                    // Of course, you can set MaxPowerFactor to 1.0 to
                    // . effectively disable boost.
                    if (actuatorDesired.Thrust > 0.0f) {
                        // to enable the future thrust calculations, make sure
                        // there is a large enough transition that the result
                        // will be roughly on vs. off; without that, it can
                        // exaggerate the length of time the inverted to upright
                        // transition holds full throttle and reduce the length
                        // of time for full throttle when going upright to inverted.
                        if (actuatorDesired.Thrust > 0.7f) {
                            float thrust;

                            thrust = CruiseControlFactorToThrust(CruiseControlAngleToFactor(cruise_control_max_angle), actuatorDesired.Thrust);

                            // determine if we are in range of the transition

                            // given the thrust at max_angle and actuatorDesired.Thrust
                            // (typically close to 1.0), change variable 'thrust' to
                            // be the proportion of the largest thrust change possible
                            // that occurs when going into inverted mode.
                            // Example: 'thrust' is 0.8  A quad has min_thrust set
                            // to 0.05  The difference is 0.75.  The largest possible
                            // difference with this setup is 0.9 - 0.05 = 0.85, so
                            // the proportion is 0.75/0.85
                            // That is nearly a full throttle stroke.
                            // the 'thrust' variable is non-negative here
                            switch (cruise_control_inverted_power_output) {
                            case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_ZERO:
                                // normal multi-copter case, stroke is max to zero
                                // technically max to constant min_thrust
                                // can be used by CP
                                thrust = (thrust - CruiseControlLimitThrust(0.0f)) / cruise_control_thrust_difference;
                                break;
                            case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_NORMAL:
                                // reversed but not boosted
                                // : CP heli case, stroke is max to -stick
                                // : thrust = (thrust - CruiseControlLimitThrust(-actuatorDesired.Thrust)) / cruise_control_thrust_difference;
                                // else it is both unreversed and unboosted
                                // : simply turn off boost, stroke is max to +stick
                                // : thrust = (thrust - CruiseControlLimitThrust(actuatorDesired.Thrust)) / cruise_control_thrust_difference;
                                thrust = (thrust - CruiseControlLimitThrust(
                                    (cruise_control_inverted_thrust_reversing
                                    == STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDTHRUSTREVERSING_REVERSED)
                                    ? -actuatorDesired.Thrust
                                    : actuatorDesired.Thrust)) / cruise_control_thrust_difference;
                                break;
                            case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_BOOSTED:
                                // if boosted and reversed
                                if (cruise_control_inverted_thrust_reversing
                                    == STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDTHRUSTREVERSING_REVERSED) {
                                    // CP heli case, stroke is max to min
                                    thrust = (thrust - CruiseControlFactorToThrust(-CruiseControlAngleToFactor(cruise_control_max_angle), actuatorDesired.Thrust)) / cruise_control_thrust_difference;
                                }
                                // else it is boosted and unreversed so the throttle doesn't change
                                else {
                                    // CP heli case, no transition, so stroke is zero
                                    thrust = 0.0f;
                                }
                                break;
                            }

                            // 'thrust' is now the proportion of max stroke
                            // multiply this proportion of max stroke,
                            // times the max stroke time, to get this stroke time
                            // we only want half of this time before the transition
                            // (and half after the transition)
                            thrust *= cruise_control_half_power_delay;
                            // 'thrust' is now the length of time for this stroke
                            // multiply that times angular rate to get the lead angle
                            thrust *= fabsf(rate);
                            // if the transition is within range we use it,
                            // else we just use the current calculated thrust
                            if (cruise_control_max_angle - thrust <= angle
                                && angle <= cruise_control_max_angle + thrust) {
                                // default to a little above max angle
                                angle = cruise_control_max_angle + 0.01f;
                                // if roll direction is downward
                                // then thrust value is taken from below max angle
                                // by the code that knows about the transition angle
                                if (rate < 0.0f) {
                                    angle -= 0.02f;
                                }
                            }
                        } // if thrust > 0.7; else just use the angle we already calculated
                        factor = CruiseControlAngleToFactor(angle);
                    } else { // if thrust > 0 set factor from angle; else
                        factor = 1.0f;
                    }

                    if (angle >= cruise_control_max_angle) {
                        switch (cruise_control_inverted_power_output) {
                        case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_ZERO:
                            factor = 0.0f;
                            break;
                        case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_NORMAL:
                            factor = 1.0f;
                            break;
                        case STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDPOWEROUTPUT_BOOSTED:
                            // no change, leave factor >= 1.0 alone
                            break;
                        }
                        if (cruise_control_inverted_thrust_reversing
                            == STABILIZATIONSETTINGS_CRUISECONTROLINVERTEDTHRUSTREVERSING_REVERSED) {
                            factor = -factor;
                        }
                    }
                } // if previous_time_valid i.e. we've got a rate; else leave (angle and) factor alone
                previous_time       = time;
                previous_time_valid = true;
                previous_angle      = angle_unmodified;
            } // every 8th time

            // don't touch thrust if it's less than min_thrust
            // without that	 test, quadcopter props will spin up
            // to min thrust even at zero throttle stick
            actuatorDesired.Thrust = CruiseControlFactorToThrust(factor, actuatorDesired.Thrust);
        } // if Cruise Control is enabled on this flight switch position

        if (flightStatus.ControlChain.Stabilization == FLIGHTSTATUS_CONTROLCHAIN_TRUE) {
            ActuatorDesiredSet(&actuatorDesired);
        } else {
            // Force all axes to reinitialize when engaged
            for (uint8_t i = 0; i < MAX_AXES; i++) {
                previous_mode[i] = 255;
            }
        }

        if (flightStatus.Armed != FLIGHTSTATUS_ARMED_ARMED ||
            (lowThrottleZeroIntegral && throttleDesired < 0)) {
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
        return -range;
    } else if (val > range) {
        return range;
    }
    return val;
}


static void SettingsBankUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    if (cur_flight_mode < 0 || cur_flight_mode >= FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_NUMELEM) {
        return;
    }
    if ((ev) && ((settings.FlightModeMap[cur_flight_mode] == 0 && ev->obj != StabilizationSettingsBank1Handle()) ||
                 (settings.FlightModeMap[cur_flight_mode] == 1 && ev->obj != StabilizationSettingsBank2Handle()) ||
                 (settings.FlightModeMap[cur_flight_mode] == 2 && ev->obj != StabilizationSettingsBank3Handle()) ||
                 settings.FlightModeMap[cur_flight_mode] > 2)) {
        return;
    }

    StabilizationBankData bank;

    switch (settings.FlightModeMap[cur_flight_mode]) {
    case 0:
        StabilizationSettingsBank1Get((StabilizationSettingsBank1Data *)&bank);
        break;

    case 1:
        StabilizationSettingsBank2Get((StabilizationSettingsBank2Data *)&bank);
        break;

    case 2:
        StabilizationSettingsBank3Get((StabilizationSettingsBank3Data *)&bank);
        break;
    }
    StabilizationBankSet(&bank);
}

static void BankUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    StabilizationBankData bank;

    StabilizationBankGet(&bank);

// this code will be needed if any other modules alter stabilizationbank
/*
    StabilizationBankData curBank;
    if(flight_mode < 0) return;

    switch(cast_struct_to_array(settings.FlightModeMap, settings.FlightModeMap.Stabilized1)[flight_mode])
    {
    case 0:
        StabilizationSettingsBank1Get((StabilizationSettingsBank1Data *) &curBank);
        if(memcmp(&curBank, &bank, sizeof(StabilizationBankDataPacked)) != 0)
        {
            StabilizationSettingsBank1Set((StabilizationSettingsBank1Data *) &bank);
        }
        break;

    case 1:
        StabilizationSettingsBank2Get((StabilizationSettingsBank2Data *) &curBank);
        if(memcmp(&curBank, &bank, sizeof(StabilizationBankDataPacked)) != 0)
        {
            StabilizationSettingsBank2Set((StabilizationSettingsBank2Data *) &bank);
        }
        break;

    case 2:
        StabilizationSettingsBank3Get((StabilizationSettingsBank3Data *) &curBank);
        if(memcmp(&curBank, &bank, sizeof(StabilizationBankDataPacked)) != 0)
        {
            StabilizationSettingsBank3Set((StabilizationSettingsBank3Data *) &bank);
        }
        break;

    default:
        return; //invalid bank number
    }
 */


    // Set the roll rate PID constants
    pid_configure(&pids[PID_RATE_ROLL], bank.RollRatePID.Kp,
                  bank.RollRatePID.Ki,
                  bank.RollRatePID.Kd,
                  bank.RollRatePID.ILimit);

    // Set the pitch rate PID constants
    pid_configure(&pids[PID_RATE_PITCH], bank.PitchRatePID.Kp,
                  bank.PitchRatePID.Ki,
                  bank.PitchRatePID.Kd,
                  bank.PitchRatePID.ILimit);

    // Set the yaw rate PID constants
    pid_configure(&pids[PID_RATE_YAW], bank.YawRatePID.Kp,
                  bank.YawRatePID.Ki,
                  bank.YawRatePID.Kd,
                  bank.YawRatePID.ILimit);

    // Set the roll attitude PI constants
    pid_configure(&pids[PID_ROLL], bank.RollPI.Kp,
                  bank.RollPI.Ki,
                  0,
                  bank.RollPI.ILimit);

    // Set the pitch attitude PI constants
    pid_configure(&pids[PID_PITCH], bank.PitchPI.Kp,
                  bank.PitchPI.Ki,
                  0,
                  bank.PitchPI.ILimit);

    // Set the yaw attitude PI constants
    pid_configure(&pids[PID_YAW], bank.YawPI.Kp,
                  bank.YawPI.Ki,
                  0,
                  bank.YawPI.ILimit);
}


static float CruiseControlAngleToFactor(float angle)
{
    float factor;
    // avoid singularity
    if (angle > 89.999f && angle < 90.001f) {
        factor = cruise_control_max_power_factor;
    } else {
        // the simple bank angle boost calculation that Cruise Control revolves around
        factor = 1.0f / fabsf(cos_lookup_deg(angle));
        // factor in the power trim, no effect at 1.0, linear effect increases with factor
        factor = (factor - 1.0f) * cruise_control_power_trim + 1.0f;
        // limit to user specified max power multiplier
        if (factor > cruise_control_max_power_factor) {
            factor = cruise_control_max_power_factor;
        }
    }
    return (factor);
}


// assumes 1.0 <= factor <= 100.0
// a factor of less than 1.0 could make it return a value less than cruise_control_min_thrust
// CP helis need to have min_thrust=-1
//
// multicopters need to have min_thrust=0.05 or so
// values below that will not be subject to max / min limiting
// that means thrust can be less than min
// that means multicopter motors stop spinning at low stick
static float CruiseControlFactorToThrust(float factor, float thrust)
{
    // don't touch if below min_thrust so we don't limit to min of min_thrust
    // e.g. multicopter motors always spin
    if (thrust > cruise_control_min_thrust) {
        thrust = CruiseControlLimitThrust(thrust * factor);
    }
    return (thrust);
}


static float CruiseControlLimitThrust(float thrust)
{
    // limit to user specified absolute max thrust
    if (thrust > cruise_control_max_thrust) {
        thrust = cruise_control_max_thrust;
    } else if (thrust < cruise_control_min_thrust) {
        thrust = cruise_control_min_thrust;
    }
    return (thrust);
}


static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    StabilizationSettingsGet(&settings);

    // Set up the derivative term
    pid_configure_derivative(settings.DerivativeCutoff, settings.DerivativeGamma);

    // Maximum deviation to accumulate for axis lock
    max_axis_lock     = settings.MaxAxisLock;
    max_axislock_rate = settings.MaxAxisLockRate;

    // Settings for weak leveling
    weak_leveling_kp  = settings.WeakLevelingKp;
    weak_leveling_max = settings.MaxWeakLevelingRate;

    // Whether to zero the PID integrals while thrust is low
    lowThrottleZeroIntegral = settings.LowThrottleZeroIntegral == STABILIZATIONSETTINGS_LOWTHROTTLEZEROINTEGRAL_TRUE;

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

    // force flight mode update
    cur_flight_mode = -1;

    // Rattitude stick angle where the attitude to rate transition happens
    if (settings.RattitudeModeTransition < (uint8_t)10) {
        rattitude_mode_transition_stick_position = 10.0f / 100.0f;
    } else {
        rattitude_mode_transition_stick_position = (float)settings.RattitudeModeTransition / 100.0f;
    }

    cruise_control_min_thrust                 = (float)settings.CruiseControlMinThrust / 100.0f;
    cruise_control_max_thrust                 = (float)settings.CruiseControlMaxThrust / 100.0f;
    cruise_control_thrust_difference          = cruise_control_max_thrust - cruise_control_min_thrust;

    cruise_control_max_angle                  = (float) settings.CruiseControlMaxAngle;
    cruise_control_max_power_factor           = settings.CruiseControlMaxPowerFactor;
    cruise_control_power_trim                 = settings.CruiseControlPowerTrim / 100.0f;
    cruise_control_half_power_delay           = settings.CruiseControlPowerDelayComp / 2.0f;
    cruise_control_max_power_factor_angle     = RAD2DEG(acosf(1.0f / settings.CruiseControlMaxPowerFactor));

    cruise_control_inverted_thrust_reversing  = settings.CruiseControlInvertedThrustReversing;
    cruise_control_inverted_power_output      = settings.CruiseControlInvertedPowerOutput;

    memcpy(
        cruise_control_flight_mode_switch_pos_enable,
        settings.CruiseControlFlightModeSwitchPosEnable,
        sizeof(cruise_control_flight_mode_switch_pos_enable));
}

/**
 * @}
 * @}
 */
