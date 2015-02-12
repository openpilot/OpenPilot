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
#ifndef PIDCONTROLTHRUST_H
#define PIDCONTROLTHRUST_H
extern "C" {
#include <pid.h>
#include <stabilizationdesired.h>
}
#include "PathFollowerFSM.h"

class PIDControlThrust {
public:
    PIDControlThrust()
    : deltaTime(0), mVelocitySetpointTarget(0), mVelocityState(0), mThrustCommand(0.0f), mFSM(0), mNeutral(0.5f), mActive(false)
    {
        Deactivate();
    }

    ~PIDControlThrust() {}

    void Initialize(PathFollowerFSM *fsm)
    {
        mFSM = fsm;
    }

    void Deactivate()
    {
      //pid_zero(&PID);
      mActive = false;
    }

    void Activate()
    {
      float currentThrust;
      StabilizationDesiredThrustGet(&currentThrust);
      float u0 = currentThrust - mNeutral;
      pid2_transfer(&PID, u0);
      mActive = true;
    }

    void UpdateParameters(float kp, float ki, float kd, __attribute__((unused)) float ilimit, float dT, float velocityMax)
    {
        //pid_configure(&PID, kp, ki, kd, ilimit);
        float Ti = kp/ki;
        float Td = kd/kp;
        float kt = (Ti + Td)/2.0f;
        float Tf = Td/10.0f;
        float beta = 1.0f; // 0 to 1
        float u0 = 0.0f;
        pid2_configure(&PID, kp, ki, kd, Tf, kt, dT, beta, u0);
        deltaTime = dT;
        mVelocityMax = velocityMax;
    }

    void UpdateNeutralThrust(float neutral)
    {
        if (mActive) {
            // adjust neutral and achieve bumpless transfer
            PID.I += mNeutral - neutral;
        }
        mNeutral = neutral;
    }

    void UpdateVelocitySetpoint(float setpoint)
    {
        mVelocitySetpointTarget = setpoint;
        if (fabsf(mVelocitySetpointTarget) > mVelocityMax) {
             // maintain sign but set to max
             mVelocitySetpointTarget *= mVelocityMax / fabsf(mVelocitySetpointTarget);
        }
    }

    void UpdateVelocityState(float pv)
    {
        mVelocityState = pv;

        // The FSM controls the actual descent velocity and introduces step changes as required
        float velocitySetpointDesired = mFSM->BoundVelocityDown(mVelocitySetpointTarget);

        // Calculate the rate of change
        float accelerationDesired = (velocitySetpointDesired - mVelocitySetpointCurrent)/deltaTime;

        if (fabsf(accelerationDesired) > 2.0f) {
            accelerationDesired  *= 2.0f / accelerationDesired;
        }
        else  if (fabsf(accelerationDesired) < 0.1f) {
            mVelocitySetpointCurrent = velocitySetpointDesired;
        }
        else {
            mVelocitySetpointCurrent += accelerationDesired * deltaTime;
        }
    }

    float GetThrustCommand(void)
    {
        //pid_scaler local_scaler = { .p = 1.0f, .i = 1.0f, .d = 1.0f };
        //mFSM->CheckPidScaler(&local_scaler);
        //float downCommand    = -pid_apply_setpoint(&PID, &local_scaler, mVelocitySetpoint, mState, deltaTime);
        float ulow, uhigh;
        mFSM->BoundThrust(ulow, uhigh);
        float downCommand    = -pid2_apply(&PID, mVelocitySetpointCurrent, mVelocityState, ulow-mNeutral, uhigh-mNeutral);
        mThrustCommand = mNeutral + downCommand;
        return mThrustCommand;
    }

    float GetVelocityDesired(void)
    {
        return mVelocitySetpointCurrent;
    }

private:
    struct pid2 PID;
    float deltaTime;
    float mVelocitySetpointTarget;
    float mVelocitySetpointCurrent;
    float mVelocityState;
    float mThrustCommand;
    PathFollowerFSM *mFSM;
    float mNeutral;
    float mVelocityMax;
    uint8_t mActive;

};

#endif // PIDCONTROLTHRUST_H
