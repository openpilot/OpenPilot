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
 * @file       outerloop.c
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
#include <stabilizationdesired.h>
// #include <gyrostate.h>
// #include <airspeedstate.h>
#include <stabilizationstatus.h>
// #include <flightstatus.h>
#include <stabilizationbank.h>

#include <stabilization.h>
#include <cruisecontrol.h>

// Private constants
#define STACK_SIZE_BYTES  256

#define CBTASK_PRIORITY   CALLBACK_TASK_FLIGHTCONTROL
#define CALLBACK_PRIORITY CALLBACK_PRIORITY_REGULAR

#define UPDATE_EXPECTED   (1.0f / 666.0f)
#define UPDATE_MIN        1.0e-6f
#define UPDATE_MAX        1.0f
#define UPDATE_ALPHA      1.0e-2f

#define AXES              4

// Private variables
static DelayedCallbackInfo *callbackHandle;
static struct pid pids[3];
static AttitudeStateData attitude;
static uint8_t previous_mode[AXES] = { 255, 255, 255, 255 };
static PiOSDeltatimeConfig timeval;

// Private functions
static void stabilizationOuterloopTask();
static void AttitudeStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev);
static void BankUpdatedCb(__attribute__((unused)) UAVObjEvent *ev);

void stabilizationOuterloopInit()
{
    RateDesiredInitialize();
    StabilizationDesiredInitialize();
    AttitudeStateInitialize();
    StabilizationStatusInitialize();
    StabilizationBankInitialize();
    PIOS_DELTATIME_Init(&timeval, UPDATE_EXPECTED, UPDATE_MIN, UPDATE_MAX, UPDATE_ALPHA);

    callbackHandle = PIOS_CALLBACKSCHEDULER_Create(&stabilizationOuterloopTask, CALLBACK_PRIORITY, CBTASK_PRIORITY, CALLBACKINFO_RUNNING_STABILIZATION0, STACK_SIZE_BYTES);
    AttitudeStateConnectCallback(AttitudeStateUpdatedCb);
    StabilizationBankConnectCallback(BankUpdatedCb);
    BankUpdatedCb(NULL);
}


/**
 * WARNING! This callback executes with critical flight control priority every
 * time a gyroscope update happens do NOT put any time consuming calculations
 * in this loop unless they really have to execute with every gyro update
 */
static void stabilizationOuterloopTask()
{
    RateDesiredData rateDesired;
    StabilizationDesiredData stabilizationDesired;
    StabilizationStatusOuterLoopData enabled;

    StabilizationDesiredGet(&stabilizationDesired);
    RateDesiredGet(&rateDesired);
    StabilizationStatusOuterLoopGet(&enabled);
    float *stabilizationDesiredAxis = &stabilizationDesired.Roll;
    float *rateDesiredAxis = &rateDesired.Roll;
    int t;
    // float dT = PIOS_DELTATIME_GetAverageSeconds(&timeval);

    for (t = 0; t < AXES; t++) {
        bool reinit = (cast_struct_to_array(enabled, enabled.Roll)[t] != previous_mode[t]);
        previous_mode[t] = cast_struct_to_array(enabled, enabled.Roll)[t];

        if (t < STABILIZATIONSTATUS_OUTERLOOP_THRUST) {
            if (reinit) {
                pids[t].iAccumulator = 0;
            }
            switch (cast_struct_to_array(enabled, enabled.Roll)[t]) {
            case STABILIZATIONSTATUS_OUTERLOOP_MANUAL:
            default:
                rateDesiredAxis[t] = stabilizationDesiredAxis[t];
                break;
            }
        } else {
            switch (cast_struct_to_array(enabled, enabled.Roll)[t]) {
            default:
                rateDesiredAxis[t] = stabilizationDesiredAxis[t];
                break;
            }
        }
    }

    RateDesiredSet(&rateDesired);
}


static void AttitudeStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    // this does not need mutex protection as both eventdispatcher and stabi run in same callback task!
    AttitudeStateGet(&attitude);

    PIOS_CALLBACKSCHEDULER_Dispatch(callbackHandle);
}

static void BankUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    StabilizationBankData bank;

    StabilizationBankGet(&bank);

    // Set the roll rate PID constants
    pid_configure(&pids[STABILIZATIONSTATUS_OUTERLOOP_ROLL], bank.RollRatePID.Kp,
                  bank.RollRatePID.Ki,
                  bank.RollRatePID.Kd,
                  bank.RollRatePID.ILimit);

    // Set the pitch rate PID constants
    pid_configure(&pids[STABILIZATIONSTATUS_OUTERLOOP_PITCH], bank.PitchRatePID.Kp,
                  bank.PitchRatePID.Ki,
                  bank.PitchRatePID.Kd,
                  bank.PitchRatePID.ILimit);

    // Set the yaw rate PID constants
    pid_configure(&pids[STABILIZATIONSTATUS_OUTERLOOP_YAW], bank.YawRatePID.Kp,
                  bank.YawRatePID.Ki,
                  bank.YawRatePID.Kd,
                  bank.YawRatePID.ILimit);
}

/**
 * @}
 * @}
 */
