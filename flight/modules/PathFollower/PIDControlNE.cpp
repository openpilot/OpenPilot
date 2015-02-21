/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup PathFollower CONTROL interface class
 * @brief CONTROL interface class for pathfollower goal fsm implementations
 * @{
 *
 * @file       PIDControlNE.h
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
}
#include "PathFollowerFSM.h"
#include "PIDControlNE.h"

PIDControlNE::PIDControlNE()
    : deltaTime(0), mNECommand(0), mFSM(0), mNeutral(0), mVelocityMax(0), mMinCommand(0), mMaxCommand(0), mVelocityFeedforward(0), mActive(false)
{}

PIDControlNE::~PIDControlNE() {}

void PIDControlNE::Initialize(PathFollowerFSM *fsm)
{
    mFSM = fsm;
}

void PIDControlNE::Deactivate()
{
    mActive = false;
}

void PIDControlNE::Activate()
{
    // Do we need to initialise any loops for smooth transition
    // float currentNE;
    // StabilizationDesiredNEGet(&currentNE);
    // float u0 = currentNE - mNeutral;
    // pid2_transfer(&PID, u0);
    mActive = true;
}

void PIDControlNE::UpdateParameters(float kp, float ki, float kd, __attribute__((unused)) float ilimit, float dT, float velocityMax)
{
    // pid_configure(&PID, kp, ki, kd, ilimit);
    float Ti   = kp / ki;
    float Td   = kd / kp;
    float Tt   = (Ti + Td) / 2.0f;
    float kt   = 1.0f / Tt;
    float beta = 1.0f; // 0 to 1
    float u0   = 0.0f;
    float N    = 10.0f;
    float Tf   = Td / N;
    if (kd < 1e-6f) {
   	// PI Controller
   	Tf = Ti / N;
    }

    pid2_configure(&PIDvel[0], kp, ki, kd, Tf, kt, dT, beta, u0, 0.0f, 1.0f);
    pid2_configure(&PIDvel[1], kp, ki, kd, Tf, kt, dT, beta, u0, 0.0f, 1.0f);
    deltaTime    = dT;
    mVelocityMax = velocityMax;
}

void PIDControlNE::UpdatePositionalParameters(float kp)
{
    pid_configure(&PIDposH[0], kp, 0.0f, 0.0f, 0.0f);
    pid_configure(&PIDposH[1], kp, 0.0f, 0.0f, 0.0f);
}
void PIDControlNE::UpdatePositionSetpoint(float setpointNorth, float setpointEast)
{
    mPositionSetpointTarget[0] = setpointNorth;
    mPositionSetpointTarget[1] = setpointEast;
}
void PIDControlNE::UpdatePositionState(float pvNorth, float pvEast)
{
    mPositionState[0] = pvNorth;
    mPositionState[1] = pvEast;
}
// This is a pure position hold position control
void PIDControlNE::ControlPosition()
{
    // Current progress location relative to end
    float velNorth = 0.0f;
    float velEast  = 0.0f;

    velNorth = pid_apply(&PIDposH[0], mPositionSetpointTarget[0] - mPositionState[0], deltaTime);
    velEast  = pid_apply(&PIDposH[1], mPositionSetpointTarget[1] - mPositionState[1], deltaTime);
    UpdateVelocitySetpoint(velNorth, velEast);
}

void PIDControlNE::UpdateVelocitySetpoint(float setpointNorth, float setpointEast)
{
    // scale velocity if it is above configured maximum
    // for braking, we can not help it if initial velocity was greater
    float velH = sqrtf(setpointNorth * setpointNorth + setpointEast * setpointEast);

    if (velH > mVelocityMax) {
        setpointNorth *= mVelocityMax / velH;
        setpointEast  *= mVelocityMax / velH;
    }

    mVelocitySetpointTarget[0] = setpointNorth;
    mVelocitySetpointTarget[1] = setpointEast;
}


void PIDControlNE::UpdateBrakeVelocity(float startingVelocity, float dT, float brakeRate, float currentVelocity, float *updatedVelocity)
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

    UpdateBrakeVelocity(mVelocitySetpointTarget[0], path_time, brakeRate, pvNorth, &velocitySetpointDesired[0]);
    UpdateBrakeVelocity(mVelocitySetpointTarget[1], path_time, brakeRate, pvEast, &velocitySetpointDesired[1]);

    // Calculate the rate of change
    for (int iaxis = 0; iaxis < 2; iaxis++) {
        // RateLimit(velocitySetpointDesired[iaxis], mVelocitySetpointCurrent[iaxis], 2.0f );

        mVelocitySetpointCurrent[iaxis] = velocitySetpointDesired[iaxis];
    }
}

void PIDControlNE::UpdateVelocityState(float pvNorth, float pvEast)
{
    mVelocityState[0] = pvNorth;
    mVelocityState[1] = pvEast;

    // The FSM controls the actual descent velocity and introduces step changes as required
    float velocitySetpointDesired[2];
    velocitySetpointDesired[0] = mVelocitySetpointTarget[0];
    velocitySetpointDesired[1] = mVelocitySetpointTarget[1];

    // Calculate the rate of change
    for (int iaxis = 0; iaxis < 2; iaxis++) {
        // RateLimit(velocitySetpointDesired[iaxis], mVelocitySetpointCurrent[iaxis], 2.0f );

        mVelocitySetpointCurrent[iaxis] = velocitySetpointDesired[iaxis];
    }
}


void PIDControlNE::UpdateCommandParameters(float minCommand, float maxCommand, float velocityFeedforward)
{
    mMinCommand = minCommand;
    mMaxCommand = maxCommand;
    mVelocityFeedforward = velocityFeedforward;
}



void PIDControlNE::GetNECommand(float *northCommand, float *eastCommand)
{
    PIDvel[0].va = mVelocitySetpointCurrent[0] * mVelocityFeedforward;
    *northCommand = pid2_apply(&(PIDvel[0]), mVelocitySetpointCurrent[0], mVelocityState[0], mMinCommand, mMaxCommand);
    PIDvel[1].va = mVelocitySetpointCurrent[1] * mVelocityFeedforward;
    *eastCommand  = pid2_apply(&(PIDvel[1]), mVelocitySetpointCurrent[1], mVelocityState[1], mMinCommand, mMaxCommand);
}

void PIDControlNE::GetVelocityDesired(float *north, float *east)
{
    *north = mVelocitySetpointCurrent[0];
    *east  = mVelocitySetpointCurrent[1];
}
