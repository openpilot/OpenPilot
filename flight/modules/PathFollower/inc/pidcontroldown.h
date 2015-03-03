/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup PathFollower PID Control implementation
 * @brief PID Controller for down direction
 * @{
 *
 * @file       PIDControlDown.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Executes control loop for down direction
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
#ifndef PIDCONTROLDOWN_H
#define PIDCONTROLDOWN_H
extern "C" {
#include <pid.h>
#include <stabilizationdesired.h>
}
#include "pathfollowerfsm.h"

class PIDControlDown {
public:
    PIDControlDown();
    ~PIDControlDown();
    void Initialize(PathFollowerFSM *fsm);
    void Deactivate();
    void Activate();
    void UpdateParameters(float kp, float ki, float kd, float beta, float dT, float velocityMax);
    void UpdateNeutralThrust(float neutral);
    void UpdateVelocitySetpoint(float setpoint);
    void RateLimit(float *spDesired, float *spCurrent, float rateLimit);
    void UpdateVelocityState(float pv);
    float GetVelocityDesired(void);
    float GetDownCommand(void);
    void UpdatePositionalParameters(float kp);
    void UpdatePositionState(float pvDown);
    void UpdatePositionSetpoint(float setpointDown);
    void ControlPosition();
    void ControlPositionWithPath(struct path_status *progress);
    void UpdateBrakeVelocity(float startingVelocity, float dT, float brakeRate, float currentVelocity, float *updatedVelocity);
    void UpdateVelocityStateWithBrake(float pvDown, float path_time, float brakeRate);

private:
    void setup_neutralThrustCalc();
    void run_neutralThrustCalc();

    struct pid2 PID;
    float deltaTime;
    float mVelocitySetpointTarget;
    float mVelocitySetpointCurrent;
    float mVelocityState;
    float mDownCommand;
    PathFollowerFSM *mFSM;
    float mNeutral;
    float mVelocityMax;
    struct pid PIDpos;
    float mPositionSetpointTarget;
    float mPositionState;
    uint8_t mActive;

    struct NeutralThrustEstimation {
        uint32_t count;
        float    sum;
        float    average;
        float    correction;
        float    algo_erro_check;
        float    min;
        float    max;
        bool     start_sampling;
        bool     have_correction;
    };
    struct NeutralThrustEstimation neutralThrustEst;
};

#endif // PIDCONTROLDOWN_H
