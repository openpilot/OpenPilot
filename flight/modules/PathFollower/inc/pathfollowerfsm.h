/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup PathFollower FSM interface class
 * @brief FSM interface class for pathfollower goal fsm implementations
 * @{
 *
 * @file       pathfollowerfsm.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Interface class for PathFollower FSMs
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
#ifndef PATHFOLLOWERFSM_H
#define PATHFOLLOWERFSM_H

extern "C" {
#include <stabilizationdesired.h>
}
#include <pidcontroldowncallback.h>

typedef enum {
    PFFSM_STATE_INACTIVE = 0, // Inactive state is the initialised state on startup
    PFFSM_STATE_ACTIVE,
    PFFSM_STATE_DISARMED,
    PFFSM_STATE_ABORT // Abort on error
} PathFollowerFSMState_T;

class PathFollowerFSM : public PIDControlDownCallback {
public:
    // PathFollowerFSM() {};
    virtual void Inactive(void) {}
    virtual void Activate(void) {}
    virtual void Update(void) {}
    virtual void SettingsUpdated(void) {}
    virtual void BoundThrust(__attribute__((unused)) float &ulow, __attribute__((unused)) float &uhigh) {}
    virtual PathFollowerFSMState_T GetCurrentState(void)
    {
        return PFFSM_STATE_INACTIVE;
    }
    virtual void ConstrainStabiDesired(__attribute__((unused)) StabilizationDesiredData *stabDesired) {}
    virtual float BoundVelocityDown(float velocity)
    {
        return velocity;
    }
    virtual void CheckPidScaler(__attribute__((unused)) pid_scaler *scaler) {}
    virtual void GetYaw(bool &yaw_attitude, float &yaw_direction)
    {
        yaw_attitude = false; yaw_direction = 0.0f;
    }
    virtual void Abort(void) {}
    virtual uint8_t ManualThrust(void)
    {
        return false;
    }
    virtual uint8_t PositionHoldState(void)
    {
        return false;
    }

    // virtual ~PathFollowerFSM() = 0;
};

#endif // PATHFOLLOWERFSM_H
