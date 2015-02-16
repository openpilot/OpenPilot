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
    PIDControlThrust();
    ~PIDControlThrust();
    void Initialize(PathFollowerFSM *fsm);
    void Deactivate();
    void Activate();
    void UpdateParameters(float kp, float ki, float kd, float beta, float dT, float velocityMax);
    void UpdateNeutralThrust(float neutral);
    void UpdateVelocitySetpoint(float setpoint);
    void RateLimit(float *spDesired, float *spCurrent, float rateLimit);
    void UpdateVelocityState(float pv);
    float GetVelocityDesired(void);
    float GetThrustCommand(void);

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
