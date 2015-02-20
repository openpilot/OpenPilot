/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup PathFollower CONTROL interface class
 * @brief CONTROL interface class for pathfollower goal fsm implementations
 * @{
 *
 * @file       PIDControlThrust.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Executes CONTROL for landing sequence
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
extern "C" {
#include <openpilot.h>

#include <callbackinfo.h>

#include <math.h>
#include <pid.h>
#include <CoordinateConversions.h>
#include <sin_lookup.h>
#include <pathdesired.h>
#include <paths.h>
#include "plans.h"
#include <stabilizationdesired.h>
#include <pidstatus.h>
}

#include "PathFollowerFSM.h"
#include "PIDControlThrust.h"

PIDControlThrust::PIDControlThrust()
    : deltaTime(0), mVelocitySetpointTarget(0), mVelocityState(0), mThrustCommand(0.0f), mFSM(0), mNeutral(0.5f), mActive(false)
{
    Deactivate();
}

PIDControlThrust::~PIDControlThrust() {}

void PIDControlThrust::Initialize(PathFollowerFSM *fsm)
{
    mFSM = fsm;
}

void PIDControlThrust::Deactivate()
{
    // pid_zero(&PID);
    mActive = false;
}

void PIDControlThrust::Activate()
{
    float currentThrust;

    StabilizationDesiredThrustGet(&currentThrust);
    float u0 = currentThrust - mNeutral;
    pid2_transfer(&PID, u0);
    mActive = true;
}

void PIDControlThrust::UpdateParameters(float kp, float ki, float kd, float beta, float dT, float velocityMax)
{
    // pid_configure(&PID, kp, ki, kd, ilimit);
    float Ti   = kp / ki;
    float Td   = kd / kp;
    float Tt   = (Ti + Td) / 2.0f;
    float kt   = 1.0f / Tt;
    float N    = 10.0f;
    float Tf   = Td / N;
    if (kd < 1e-6f) {
	// PI Controller
	Tf = Ti / N;
    }

    pid2_configure(&PID, kp, ki, kd, Tf, kt, dT, beta, mNeutral, mNeutral, -1.0f);
    deltaTime    = dT;
    mVelocityMax = velocityMax;
}

void PIDControlThrust::UpdateNeutralThrust(float neutral)
{
    if (mActive) {
        // adjust neutral and achieve bumpless transfer
        PID.va = neutral;
        pid2_transfer(&PID, mThrustCommand);
    }
    mNeutral = neutral;
}

void PIDControlThrust::UpdateVelocitySetpoint(float setpoint)
{
    mVelocitySetpointTarget = setpoint;
    if (fabsf(mVelocitySetpointTarget) > mVelocityMax) {
        // maintain sign but set to max
        mVelocitySetpointTarget *= mVelocityMax / fabsf(mVelocitySetpointTarget);
    }
}

void PIDControlThrust::RateLimit(float *spDesired, float *spCurrent, float rateLimit)
{
    float velocity_delta = *spDesired - *spCurrent;

    if (fabsf(velocity_delta) < 1e-6f) {
        *spCurrent = *spDesired;
        return;
    }

    // Calculate the rate of change
    float accelerationDesired = velocity_delta / deltaTime;

    if (fabsf(accelerationDesired) > rateLimit) {
        accelerationDesired *= rateLimit / accelerationDesired;
    }

    if (fabsf(accelerationDesired) < 0.1f) {
        *spCurrent = *spDesired;
    } else {
        *spCurrent += accelerationDesired * deltaTime;
    }
}

void PIDControlThrust::UpdateBrakeVelocity(float startingVelocity, float dT, float brakeRate, float currentVelocity, float *updatedVelocity)
{
    if (startingVelocity >= 0.0f) {
        *updatedVelocity = startingVelocity - dT * brakeRate;
        if (*updatedVelocity > currentVelocity) {
            *updatedVelocity = currentVelocity;
        }
        if (*updatedVelocity < 0.0f) {
            *updatedVelocity = 0.0f;
        }
    } else {
        *updatedVelocity = startingVelocity + dT * brakeRate;
        if (*updatedVelocity < currentVelocity) {
            *updatedVelocity = currentVelocity;
        }
        if (*updatedVelocity > 0.0f) {
            *updatedVelocity = 0.0f;
        }
    }
}

void PIDControlNE::UpdateVelocityStateWithBrake(float pvNorth, float pvEast, float path_time, float brakeRate)
{
    mVelocityState[0] = pvNorth;
    mVelocityState[1] = pvEast;

    float velocitySetpointDesired[2];

    updateBrakeVelocity(mVelocitySetpointTarget[0], path_time, brakeRate, pvNorth, &velocitySetpointDesired[0]);
    updateBrakeVelocity(mVelocitySetpointTarget[1], path_time, brakeRate, pvEast, &velocitySetpointDesired[1]);

    // Calculate the rate of change
    for (int iaxis = 0; iaxis < 2; iaxis++) {
        // RateLimit(velocitySetpointDesired[iaxis], mVelocitySetpointCurrent[iaxis], 2.0f );

        mVelocitySetpointCurrent[iaxis] = velocitySetpointDesired[iaxis];
    }
}


// Update velocity state called per dT. Also update current
// desired velocity
void PIDControlThrust::UpdateVelocityState(float pv)
{
    mVelocityState = pv;

    // The FSM controls the actual descent velocity and introduces step changes as required
    float velocitySetpointDesired = mFSM->BoundVelocityDown(mVelocitySetpointTarget);
    // RateLimit(velocitySetpointDesired, mVelocitySetpointCurrent, 2.0f );
    mVelocitySetpointCurrent = velocitySetpointDesired;
}

float PIDControlThrust::GetVelocityDesired(void)
{
    return mVelocitySetpointCurrent;
}

float PIDControlThrust::GetThrustCommand(void)
{
    PIDStatusData pidStatus;
    // pid_scaler local_scaler = { .p = 1.0f, .i = 1.0f, .d = 1.0f };
    // mFSM->CheckPidScaler(&local_scaler);
    // float downCommand    = -pid_apply_setpoint(&PID, &local_scaler, mVelocitySetpoint, mState, deltaTime);
    float ulow, uhigh;

    mFSM->BoundThrust(ulow, uhigh);
    float downCommand = pid2_apply(&PID, mVelocitySetpointCurrent, mVelocityState, ulow, uhigh);
    pidStatus.setpoint = mVelocitySetpointCurrent;
    pidStatus.actual = mVelocityState;
    pidStatus.error = mVelocitySetpointCurrent - mVelocityState;
    pidStatus.setpoint = mVelocitySetpointCurrent;
    pidStatus.ulow = ulow;
    pidStatus.uhigh = uhigh;
    pidStatus.command = downCommand;
    pidStatus.P = PID.P;
    pidStatus.I = PID.I;
    pidStatus.D = PID.D;
    PIDStatusSet(&pidStatus);
    mThrustCommand = downCommand;
    return mThrustCommand;
}
