/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup PathFollower FSM Brake
 * @brief Executes FSM for landing sequence
 * @{
 *
 * @file       fsm_land.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Executes FSM for landing sequence
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
#ifndef FSM_BRAKE_H
#define FSM_BRAKE_H

extern "C" {
#include "fsmlandstatus.h"
}
#include "PathFollowerFSM.h"

// Brakeing state machine
typedef enum {
    BRAKE_STATE_INACTIVE = 0, // Inactive state is the initialised state on startup
    BRAKE_STATE_BRAKE, // Initiate altitude hold before starting descent
    BRAKE_STATE_HOLD, // Waiting for attainment of landing descent rate
    BRAKE_STATE_SIZE
} PathFollowerFSM_BrakeState_T;

class FSMBrake : PathFollowerFSM {
private:
    static FSMBrake *p_inst;
    FSMBrake();

public:
    static FSMBrake *instance()
    {
        if (!p_inst) {
            p_inst = new FSMBrake();
        }
        return p_inst;
    }
    int32_t Initialize(VtolPathFollowerSettingsData *vtolPathFollowerSettings,
                       PathDesiredData *pathDesired,
                       FlightStatusData *flightStatus);
    void Inactive(void);
    void Activate(void);
    void Update(void);
    PathFollowerFSMState_T GetCurrentState(void);
    void Abort(void);

protected:

    // FSM instance data type
    typedef struct {
        PathFollowerFSM_BrakeState_T currentState;
        uint32_t stateRunCount;
        uint32_t stateTimeoutCount;
        float    sum1;
        float    sum2;
        uint8_t  observationCount;
        uint8_t  observation2Count;
    } FSMBrakeData_T;

    // FSM state structure
    typedef struct {
        void(FSMBrake::*setup) (void); // Called to initialise the state
        void(FSMBrake::*run) (uint8_t); // Run the event detection code for a state
    } PathFollowerFSM_BrakeStateHandler_T;

    // Private variables
    FSMBrakeData_T *mBrakeData;
    VtolPathFollowerSettingsData *vtolPathFollowerSettings;
    PathDesiredData *pathDesired;
    FlightStatusData *flightStatus;

    void setup_inactive(void);
    void setup_brake(void);
    void setup_hold(void);
    void run_brake(uint8_t);
    void run_hold(uint8_t);
    void initFSM(void);
    void setState(PathFollowerFSM_BrakeState_T newState, FSMBrakeStatusStateExitReasonOptions reason);
    int32_t runState();
    int32_t runAlways();
    void updateFSMBrakeStatus();

    void setStateTimeout(int32_t count);
    void fallback_to_hold(void);

    static PathFollowerFSM_BrakeStateHandler_T sBrakeStateTable[BRAKE_STATE_SIZE];
};

#endif // FSM_BRAKE_H
