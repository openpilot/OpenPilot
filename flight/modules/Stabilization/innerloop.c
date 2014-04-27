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
 * @file       innerloop.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
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
#include <pid.h>
#include <callbackinfo.h>
#include <ratedesired.h>
#include <actuatordesired.h>
#include <gyrostate.h>
#include <airspeedstate.h>
#include <stabilizationstatus.h>
#include <flightstatus.h>
#include <stabilizationbank.h>

#include <stabilization.h>
#include <relay_tuning.h>
#include <virtualflybar.h>
#include <cruisecontrol.h>

// Private constants
#define STACK_SIZE_BYTES  256

#define CBTASK_PRIORITY   CALLBACK_TASK_FLIGHTCONTROL
#define CALLBACK_PRIORITY CALLBACK_PRIORITY_CRITICAL

#define UPDATE_EXPECTED   (1.0f / 666.0f)
#define UPDATE_MIN        1.0e-6f
#define UPDATE_MAX        1.0f
#define UPDATE_ALPHA      1.0e-2f

#define AXES              4

// Private variables
static DelayedCallbackInfo *callbackHandle;
static struct pid pids[3];
static float gyro_filtered[3] = { 0, 0, 0 };
static float axis_lock_accum[3] = { 0, 0, 0 };
static uint8_t previous_mode[AXES] = { 255, 255, 255, 255 };
static PiOSDeltatimeConfig timeval;
static float speedScaleFactor = 1.0f;
static StabilizationBankData stabBank;

// Private functions
static void stabilizationInnerloopTask();
static void GyroStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev);
static void BankUpdatedCb(__attribute__((unused)) UAVObjEvent *ev);
#ifdef REVOLUTION
static void AirSpeedUpdatedCb(__attribute__((unused)) UAVObjEvent *ev);
#endif

void stabilizationInnerloopInit()
{
    RateDesiredInitialize();
    ActuatorDesiredInitialize();
    GyroStateInitialize();
    StabilizationStatusInitialize();
    FlightStatusInitialize();
    StabilizationBankInitialize();
#ifdef REVOLUTION
    AirspeedStateInitialize();
    AirspeedStateConnectCallback(AirSpeedUpdatedCb);
#endif
    PIOS_DELTATIME_Init(&timeval, UPDATE_EXPECTED, UPDATE_MIN, UPDATE_MAX, UPDATE_ALPHA);

    callbackHandle = PIOS_CALLBACKSCHEDULER_Create(&stabilizationInnerloopTask, CALLBACK_PRIORITY, CBTASK_PRIORITY, CALLBACKINFO_RUNNING_STABILIZATION1, STACK_SIZE_BYTES);
    GyroStateConnectCallback(GyroStateUpdatedCb);
    StabilizationBankConnectCallback(BankUpdatedCb);
    BankUpdatedCb(NULL);
}


/**
 * WARNING! This callback executes with critical flight control priority every
 * time a gyroscope update happens do NOT put any time consuming calculations
 * in this loop unless they really have to execute with every gyro update
 */
static void stabilizationInnerloopTask()
{
    RateDesiredData rateDesired;
    ActuatorDesiredData actuator;
    StabilizationStatusInnerLoopData enabled;
    FlightStatusControlChainData cchain;

    RateDesiredGet(&rateDesired);
    ActuatorDesiredGet(&actuator);
    StabilizationStatusInnerLoopGet(&enabled);
    FlightStatusControlChainGet(&cchain);
    float *rate = &rateDesired.Roll;
    float *actuatorDesiredAxis = &actuator.Roll;
    int t;
    float dT;
    dT = PIOS_DELTATIME_GetAverageSeconds(&timeval);

    for (t = 0; t < AXES; t++) {
        bool reinit = (cast_struct_to_array(enabled, enabled.Roll)[t] != previous_mode[t]);
        previous_mode[t] = cast_struct_to_array(enabled, enabled.Roll)[t];

        if (t < STABILIZATIONSTATUS_INNERLOOP_THRUST) {
            if (reinit) {
                pids[t].iAccumulator = 0;
            }
            switch (cast_struct_to_array(enabled, enabled.Roll)[t]) {
            case STABILIZATIONSTATUS_INNERLOOP_VIRTUALFLYBAR:
                stabilization_virtual_flybar(gyro_filtered[t], rate[t], &actuatorDesiredAxis[t], dT, reinit, t, &stabSettings.settings);
                break;
            case STABILIZATIONSTATUS_INNERLOOP_RELAYTUNING:
                rate[t] = boundf(rate[t],
                                 -cast_struct_to_array(stabBank.MaximumRate, stabBank.MaximumRate.Roll)[t],
                                 cast_struct_to_array(stabBank.MaximumRate, stabBank.MaximumRate.Roll)[t]
                                 );
                stabilization_relay_rate(rate[t] - gyro_filtered[t], &actuatorDesiredAxis[t], t, reinit);
                break;
            case STABILIZATIONSTATUS_INNERLOOP_AXISLOCK:
                if (fabsf(rate[t]) > stabSettings.settings.MaxAxisLockRate) {
                    // While getting strong commands act like rate mode
                    axis_lock_accum[t] = 0;
                } else {
                    // For weaker commands or no command simply attitude lock (almost) on no gyro change
                    axis_lock_accum[t] += (rate[t] - gyro_filtered[t]) * dT;
                    axis_lock_accum[t]  = boundf(axis_lock_accum[t], -stabSettings.settings.MaxAxisLock, stabSettings.settings.MaxAxisLock);
                    rate[t] = axis_lock_accum[t] * stabSettings.settings.AxisLockKp;
                }
            // IMPORTANT: deliberately no "break;" here, execution continues with regular RATE control loop to avoid code duplication!
            // keep order as it is, RATE must follow!
            case STABILIZATIONSTATUS_INNERLOOP_RATE:
                // limit rate to maximum configured limits (once here instead of 5 times in outer loop)
                rate[t] = boundf(rate[t],
                                 -cast_struct_to_array(stabBank.MaximumRate, stabBank.MaximumRate.Roll)[t],
                                 cast_struct_to_array(stabBank.MaximumRate, stabBank.MaximumRate.Roll)[t]
                                 );
                actuatorDesiredAxis[t] = pid_apply_setpoint(&pids[t], speedScaleFactor, rate[t], gyro_filtered[t], dT);
                break;
            case STABILIZATIONSTATUS_INNERLOOP_DIRECT:
            default:
                actuatorDesiredAxis[t] = rate[t];
                break;
            }
        } else {
            switch (cast_struct_to_array(enabled, enabled.Roll)[t]) {
            case STABILIZATIONSTATUS_INNERLOOP_CRUISECONTROL:
                actuatorDesiredAxis[t] = cruisecontrol_apply_factor(rate[t]);
                break;
            case STABILIZATIONSTATUS_INNERLOOP_DIRECT:
            default:
                actuatorDesiredAxis[t] = rate[t];
                break;
            }
        }

        actuatorDesiredAxis[t] = boundf(actuatorDesiredAxis[t], -1.0f, 1.0f);
    }

    actuator.UpdateTime = dT * 1000;

    if (cchain.Stabilization == FLIGHTSTATUS_CONTROLCHAIN_TRUE) {
        ActuatorDesiredSet(&actuator);
    } else {
        // Force all axes to reinitialize when engaged
        for (t = 0; t < AXES; t++) {
            previous_mode[t] = 255;
        }
    }
}


static void GyroStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    GyroStateData gyroState;

    GyroStateGet(&gyroState);

    gyro_filtered[0] = gyro_filtered[0] * stabSettings.gyro_alpha + gyroState.x * (1 - stabSettings.gyro_alpha);
    gyro_filtered[1] = gyro_filtered[1] * stabSettings.gyro_alpha + gyroState.y * (1 - stabSettings.gyro_alpha);
    gyro_filtered[2] = gyro_filtered[2] * stabSettings.gyro_alpha + gyroState.z * (1 - stabSettings.gyro_alpha);

    PIOS_CALLBACKSCHEDULER_Dispatch(callbackHandle);
}

static void BankUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    StabilizationBankGet(&stabBank);

    // Set the roll rate PID constants
    pid_configure(&pids[STABILIZATIONSTATUS_INNERLOOP_ROLL], stabBank.RollRatePID.Kp,
                  stabBank.RollRatePID.Ki,
                  stabBank.RollRatePID.Kd,
                  stabBank.RollRatePID.ILimit);

    // Set the pitch rate PID constants
    pid_configure(&pids[STABILIZATIONSTATUS_INNERLOOP_PITCH], stabBank.PitchRatePID.Kp,
                  stabBank.PitchRatePID.Ki,
                  stabBank.PitchRatePID.Kd,
                  stabBank.PitchRatePID.ILimit);

    // Set the yaw rate PID constants
    pid_configure(&pids[STABILIZATIONSTATUS_INNERLOOP_YAW], stabBank.YawRatePID.Kp,
                  stabBank.YawRatePID.Ki,
                  stabBank.YawRatePID.Kd,
                  stabBank.YawRatePID.ILimit);
}

#ifdef REVOLUTION
static void AirSpeedUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    // Scale PID coefficients based on current airspeed estimation - needed for fixed wing planes
    AirspeedStateData airspeedState;

    AirspeedStateGet(&airspeedState);
    if (stabSettings.settings.ScaleToAirspeed < 0.1f || airspeedState.CalibratedAirspeed < 0.1f) {
        // feature has been turned off
        speedScaleFactor = 1.0f;
    } else {
        // scale the factor to be 1.0 at the specified airspeed (for example 10m/s) but scaled by 1/speed^2
        speedScaleFactor = boundf((stabSettings.settings.ScaleToAirspeed * stabSettings.settings.ScaleToAirspeed) / (airspeedState.CalibratedAirspeed * airspeedState.CalibratedAirspeed),
                                  stabSettings.settings.ScaleToAirspeedLimits.Min,
                                  stabSettings.settings.ScaleToAirspeedLimits.Max);
    }
}
#endif

/**
 * @}
 * @}
 */
