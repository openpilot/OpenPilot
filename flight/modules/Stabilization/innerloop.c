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
#include <pid.h>
#include <callbackinfo.h>
#include <ratedesired.h>
#include <actuatordesired.h>
#include <gyrostate.h>
#include <airspeedstate.h>
#include <stabilizationstatus.h>
#include <flightstatus.h>
#include <manualcontrolcommand.h>
#include <stabilizationbank.h>
#include <stabilizationdesired.h>
#include <actuatordesired.h>

#include <stabilization.h>
#include <relay_tuning.h>
#include <virtualflybar.h>
#include <cruisecontrol.h>

// Private constants

#define CALLBACK_PRIORITY CALLBACK_PRIORITY_CRITICAL

#define UPDATE_EXPECTED   (1.0f / PIOS_SENSOR_RATE)
#define UPDATE_MIN        1.0e-6f
#define UPDATE_MAX        1.0f
#define UPDATE_ALPHA      1.0e-2f

// Private variables
static DelayedCallbackInfo *callbackHandle;
static float gyro_filtered[3] = { 0, 0, 0 };
static float axis_lock_accum[3] = { 0, 0, 0 };
static uint8_t previous_mode[AXES] = { 255, 255, 255, 255 };
static PiOSDeltatimeConfig timeval;
static float speedScaleFactor = 1.0f;

// Private functions
static void stabilizationInnerloopTask();
static void GyroStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev);
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
    ManualControlCommandInitialize();
    StabilizationDesiredInitialize();
    ActuatorDesiredInitialize();
#ifdef REVOLUTION
    AirspeedStateInitialize();
    AirspeedStateConnectCallback(AirSpeedUpdatedCb);
#endif
    PIOS_DELTATIME_Init(&timeval, UPDATE_EXPECTED, UPDATE_MIN, UPDATE_MAX, UPDATE_ALPHA);

    callbackHandle = PIOS_CALLBACKSCHEDULER_Create(&stabilizationInnerloopTask, CALLBACK_PRIORITY, CBTASK_PRIORITY, CALLBACKINFO_RUNNING_STABILIZATION1, STACK_SIZE_BYTES);
    GyroStateConnectCallback(GyroStateUpdatedCb);

    // schedule dead calls every FAILSAFE_TIMEOUT_MS to have the watchdog cleared
    PIOS_CALLBACKSCHEDULER_Schedule(callbackHandle, FAILSAFE_TIMEOUT_MS, CALLBACK_UPDATEMODE_LATER);
}

static float get_pid_scale_source_value()
{
    float value;

    switch (stabSettings.stabBank.ThrustPIDScaleSource) {
    case STABILIZATIONBANK_THRUSTPIDSCALESOURCE_MANUALCONTROLTHROTTLE:
        ManualControlCommandThrottleGet(&value);
        break;
    case STABILIZATIONBANK_THRUSTPIDSCALESOURCE_STABILIZATIONDESIREDTHRUST:
        StabilizationDesiredThrustGet(&value);
        break;
    case STABILIZATIONBANK_THRUSTPIDSCALESOURCE_ACTUATORDESIREDTHRUST:
        ActuatorDesiredThrustGet(&value);
        break;
    default:
        ActuatorDesiredThrustGet(&value);
        break;
    }

    if (value < 0) {
        value = 0.0f;
    }

    return value;
}

typedef struct pid_curve_scaler {
    float  x;
    pointf points[5];
} pid_curve_scaler;

static float pid_curve_value(const pid_curve_scaler *scaler)
{
    float y = y_on_curve(scaler->x, scaler->points, sizeof(scaler->points) / sizeof(scaler->points[0]));

    return 1.0f + (IS_REAL(y) ? y : 0.0f);
}

static pid_scaler create_pid_scaler(int axis)
{
    pid_scaler scaler;

    // Always scaled with the this.
    scaler.p = scaler.i = scaler.d = speedScaleFactor;

    if (stabSettings.thrust_pid_scaling_enabled[axis][0]
        || stabSettings.thrust_pid_scaling_enabled[axis][1]
        || stabSettings.thrust_pid_scaling_enabled[axis][2]) {
        const pid_curve_scaler curve_scaler = {
            .x      = get_pid_scale_source_value(),
            .points = {
                { 0.00f, stabSettings.stabBank.ThrustPIDScaleCurve[0] },
                { 0.25f, stabSettings.stabBank.ThrustPIDScaleCurve[1] },
                { 0.50f, stabSettings.stabBank.ThrustPIDScaleCurve[2] },
                { 0.75f, stabSettings.stabBank.ThrustPIDScaleCurve[3] },
                { 1.00f, stabSettings.stabBank.ThrustPIDScaleCurve[4] }
            }
        };

        float curve_value = pid_curve_value(&curve_scaler);

        if (stabSettings.thrust_pid_scaling_enabled[axis][0]) {
            scaler.p *= curve_value;
        }
        if (stabSettings.thrust_pid_scaling_enabled[axis][1]) {
            scaler.i *= curve_value;
        }
        if (stabSettings.thrust_pid_scaling_enabled[axis][2]) {
            scaler.d *= curve_value;
        }
    }

    return scaler;
}

/**
 * WARNING! This callback executes with critical flight control priority every
 * time a gyroscope update happens do NOT put any time consuming calculations
 * in this loop unless they really have to execute with every gyro update
 */
static void stabilizationInnerloopTask()
{
    // watchdog and error handling
    {
#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_STABILIZATION);
#endif
        bool warn  = false;
        bool error = false;
        bool crit  = false;
        // check if outer loop keeps executing
        if (stabSettings.monitor.rateupdates > -64) {
            stabSettings.monitor.rateupdates--;
        }
        if (stabSettings.monitor.rateupdates < -(2 * OUTERLOOP_SKIPCOUNT)) {
            // warning if rate loop skipped more than 2 execution
            warn = true;
        }
        if (stabSettings.monitor.rateupdates < -(4 * OUTERLOOP_SKIPCOUNT)) {
            // critical if rate loop skipped more than 4 executions
            crit = true;
        }
        // check if gyro keeps updating
        if (stabSettings.monitor.gyroupdates < 1) {
            // error if gyro didn't update at all!
            error = true;
        }
        if (stabSettings.monitor.gyroupdates > 1) {
            // warning if we missed a gyro update
            warn = true;
        }
        if (stabSettings.monitor.gyroupdates > 3) {
            // critical if we missed 3 gyro updates
            crit = true;
        }
        stabSettings.monitor.gyroupdates = 0;

        if (crit) {
            AlarmsSet(SYSTEMALARMS_ALARM_STABILIZATION, SYSTEMALARMS_ALARM_CRITICAL);
        } else if (error) {
            AlarmsSet(SYSTEMALARMS_ALARM_STABILIZATION, SYSTEMALARMS_ALARM_ERROR);
        } else if (warn) {
            AlarmsSet(SYSTEMALARMS_ALARM_STABILIZATION, SYSTEMALARMS_ALARM_WARNING);
        } else {
            AlarmsClear(SYSTEMALARMS_ALARM_STABILIZATION);
        }
    }


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
        bool reinit = (StabilizationStatusInnerLoopToArray(enabled)[t] != previous_mode[t]);
        previous_mode[t] = StabilizationStatusInnerLoopToArray(enabled)[t];

        if (t < STABILIZATIONSTATUS_INNERLOOP_THRUST) {
            if (reinit) {
                stabSettings.innerPids[t].iAccumulator = 0;
            }
            switch (StabilizationStatusInnerLoopToArray(enabled)[t]) {
            case STABILIZATIONSTATUS_INNERLOOP_VIRTUALFLYBAR:
                stabilization_virtual_flybar(gyro_filtered[t], rate[t], &actuatorDesiredAxis[t], dT, reinit, t, &stabSettings.settings);
                break;
            case STABILIZATIONSTATUS_INNERLOOP_RELAYTUNING:
                rate[t] = boundf(rate[t],
                                 -StabilizationBankMaximumRateToArray(stabSettings.stabBank.MaximumRate)[t],
                                 StabilizationBankMaximumRateToArray(stabSettings.stabBank.MaximumRate)[t]
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
                                 -StabilizationBankMaximumRateToArray(stabSettings.stabBank.MaximumRate)[t],
                                 StabilizationBankMaximumRateToArray(stabSettings.stabBank.MaximumRate)[t]
                                 );
                pid_scaler scaler = create_pid_scaler(t);
                actuatorDesiredAxis[t] = pid_apply_setpoint(&stabSettings.innerPids[t], &scaler, rate[t], gyro_filtered[t], dT);
                break;
            case STABILIZATIONSTATUS_INNERLOOP_DIRECT:
            default:
                actuatorDesiredAxis[t] = rate[t];
                break;
            }
        } else {
            switch (StabilizationStatusInnerLoopToArray(enabled)[t]) {
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
    {
        uint8_t armed;
        FlightStatusArmedGet(&armed);
        float throttleDesired;
        ManualControlCommandThrottleGet(&throttleDesired);
        if (armed != FLIGHTSTATUS_ARMED_ARMED ||
            ((stabSettings.settings.LowThrottleZeroIntegral == STABILIZATIONSETTINGS_LOWTHROTTLEZEROINTEGRAL_TRUE) && throttleDesired < 0)) {
            // Force all axes to reinitialize when engaged
            for (t = 0; t < AXES; t++) {
                previous_mode[t] = 255;
            }
        }
    }
    PIOS_CALLBACKSCHEDULER_Schedule(callbackHandle, FAILSAFE_TIMEOUT_MS, CALLBACK_UPDATEMODE_LATER);
}


static void GyroStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    GyroStateData gyroState;

    GyroStateGet(&gyroState);

    gyro_filtered[0] = gyro_filtered[0] * stabSettings.gyro_alpha + gyroState.x * (1 - stabSettings.gyro_alpha);
    gyro_filtered[1] = gyro_filtered[1] * stabSettings.gyro_alpha + gyroState.y * (1 - stabSettings.gyro_alpha);
    gyro_filtered[2] = gyro_filtered[2] * stabSettings.gyro_alpha + gyroState.z * (1 - stabSettings.gyro_alpha);

    PIOS_CALLBACKSCHEDULER_Dispatch(callbackHandle);
    stabSettings.monitor.gyroupdates++;
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
