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
#ifndef PIDCONTROLNE_H
#define PIDCONTROLNE_H
extern "C" {
#include <pid.h>
}
#include "PathFollowerFSM.h"

class PIDControlNE {
public:
    PIDControlNE()
    : deltaTime(0), mNECommand(0), mFSM(0), mNeutral(0), mVelocityMax(0), mMinCommand(0), mMaxCommand(0), mVelocityFeedforward(0), mActive(false)
    {
    }

    ~PIDControlNE() {}

    void Initialize(PathFollowerFSM *fsm)
    {
        mFSM = fsm;
    }

    void Deactivate()
    {
      mActive = false;
    }

    void Activate()
    {
      // Do we need to initialise any loops for smooth transition
      //float currentNE;
      //StabilizationDesiredNEGet(&currentNE);
      //float u0 = currentNE - mNeutral;
      //pid2_transfer(&PID, u0);
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
        pid2_configure(&PIDvel[0], kp, ki, kd, Tf, kt, dT, beta, u0);
        pid2_configure(&PIDvel[1], kp, ki, kd, Tf, kt, dT, beta, u0);
        deltaTime = dT;
        mVelocityMax = velocityMax;
    }

    void UpdatePositionalParameters(float kp)
    {
         pid_configure(&PIDposH[0], kp, 0.0f, 0.0f, 0.0f);
         pid_configure(&PIDposH[1], kp, 0.0f, 0.0f, 0.0f);
    }

    void UpdateVelocitySetpoint(float setpointNorth, float setpointEast)
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

    void UpdatePositionSetpoint(float setpointNorth, float setpointEast)
    {
        mPositionSetpointTarget[0] = setpointNorth;
        mPositionSetpointTarget[1] = setpointEast;
    }

    void RateLimit(float &spDesired, float &spCurrent, float rateLimit)
     {

               float  velocity_delta = spDesired - spCurrent;
               if (fabsf(velocity_delta) < 1e-6f) {
         	  spCurrent = spDesired;
                   return;
               }

               // Calculate the rate of change
               float accelerationDesired = velocity_delta/deltaTime;

               if (fabsf(accelerationDesired) > rateLimit) {
                   accelerationDesired  *= rateLimit / accelerationDesired;
               }

               if (fabsf(accelerationDesired) < 0.1f) {
         	  spCurrent = spDesired;
               }
               else {
         	  spCurrent += accelerationDesired * deltaTime;
               }

     }

    void UpdateVelocityState(float pvNorth, float pvEast)
    {
        mVelocityState[0] = pvNorth;
        mVelocityState[1] = pvEast;

        // The FSM controls the actual descent velocity and introduces step changes as required
        float velocitySetpointDesired[2];
        velocitySetpointDesired[0] = mVelocitySetpointTarget[0];
        velocitySetpointDesired[1] = mVelocitySetpointTarget[1];

        // Calculate the rate of change
        for (int iaxis = 0; iaxis < 2; iaxis++) {
            RateLimit(velocitySetpointDesired[iaxis], mVelocitySetpointCurrent[iaxis], 2.0f );
        }

    }

    void UpdatePositionState(float pvNorth, float pvEast)
    {
        mPositionState[0] = pvNorth;
        mPositionState[1] = pvEast;
    }

    void UpdateCommandParameters(float minCommand, float maxCommand, float velocityFeedforward) {
      mMinCommand = minCommand;
      mMaxCommand = maxCommand;
      mVelocityFeedforward = velocityFeedforward;
    }

    // This is a pure position hold position control
    void ControlPosition()
    {
      // Current progress location relative to end
      float velNorth = 0.0f;
      float velEast = 0.0f;
      velNorth = pid_apply(&PIDposH[0], mPositionSetpointTarget[0] - mPositionState[0], deltaTime);
      velEast  = pid_apply(&PIDposH[1], mPositionSetpointTarget[1] - mPositionState[1], deltaTime);
      UpdateVelocitySetpoint(velNorth, velEast);
    }

    void GetNECommand(float &northCommand, float &eastCommand)
    {
        northCommand = pid2_apply(&(PIDvel[0]), mVelocitySetpointCurrent[0], mVelocityState[0], mMinCommand, mMaxCommand) + mVelocitySetpointCurrent[0] * mVelocityFeedforward;
        eastCommand = pid2_apply(&(PIDvel[1]), mVelocitySetpointCurrent[1], mVelocityState[1], mMinCommand, mMaxCommand) + mVelocitySetpointCurrent[1] * mVelocityFeedforward;
    }

    void GetVelocityDesired(float &north, float &east)
    {
        north = mVelocitySetpointCurrent[0];
        east = mVelocitySetpointCurrent[1];
    }

private:
    struct pid2 PIDvel[2]; // North, East
    struct pid PIDposH[2];
    float mVelocitySetpointTarget[2];
    float mPositionSetpointTarget[2];
    float mVelocityState[2];
    float mPositionState[2];


    float deltaTime;
    float mVelocitySetpointCurrent[2];
    float mNECommand;
    PathFollowerFSM *mFSM;
    float mNeutral;
    float mVelocityMax;
    float mMinCommand;
    float mMaxCommand;
    float mVelocityFeedforward;
    uint8_t mActive;

};

#endif // PIDCONTROLNE_H
