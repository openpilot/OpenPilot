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
}
#include "PathFollowerFSM.h"
class PIDControlThrust {
public:
    PIDControlThrust()
    {
        Reset();
    }
    ~PIDControlThrust() {}
    void Initialize(PathFollowerFSM *fsm)
    {
        mFSM = fsm;
    }
    void Reset()
    {
        pid_zero(&PID);
    }
    void UpdateParameters(float kp, float ki, float kd, float ilimit, float dT)
    {
        pid_configure(&PID, kp, ki, kd, ilimit);
        deltaTime = dT;
    }
    void UpdateNeutralThrust(float neutral)
    {
        mNeutral = neutral;
    }
    void UpdateSetpoint(float setpoint)
    {
        mSetPoint = setpoint;
    }
    void UpdateState(float state)
    {
        mState = state;
    }

    void Update(void)
    {
        // TODO Add positional control for hold

	// Velocity Error
        float downError = mSetPoint - mState;

        // TODO Control rate of change of setpoint? e.g. goes from zero to -0.4m/s gradually.
        // Add rates of change for velocity setpoint

        downError = -downError;
        pid_scaler local_scaler = { .p = 1.0f, .i = 1.0f, .d = 1.0f };
        mFSM->CheckPidScaler(&local_scaler);

        // change following to new pid function
        // TODO Change following to calculate accel desired
        // TODO Add limits on the accel desired

        float downCommand    = -pid_apply_setpoint(&PID, &local_scaler, mSetPoint, mState, deltaTime);

        // TODO Taking into account accel desired, accel actual, calculate down command.

        // Include limits on down command

        mThrustCommand = mFSM->BoundThrust(mNeutral + downCommand);
    }

    float GetThrustCommand(void)
    {
        return mThrustCommand;
    }

private:
    struct pid PID;
    float deltaTime;
    float mSetPoint;
    float mState;
    float mThrustCommand;
    PathFollowerFSM *mFSM;
    float mNeutral;
};

#endif // PIDCONTROLTHRUST_H
