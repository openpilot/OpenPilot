/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup PathFollower FSM
 * @brief Executes landing sequence via an FSM
 * @{
 *
 * @file       vtollandfsm.h
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
#ifndef VTOLAUTOTAKEOFFFSM_H
#define VTOLAUTOTAKEOFFFSM_H

extern "C" {
#include "statusvtolautotakeoff.h"
}
#include "pathfollowerfsm.h"

// AutoTakeoffing state machine
typedef enum {
    AUTOTAKEOFF_STATE_INACTIVE = 0, // Inactive state is the initialised state on startup
    AUTOTAKEOFF_STATE_INIT_ALTHOLD, // Initiate altitude hold before starting descent
    AUTOTAKEOFF_STATE_WTG_FOR_DESCENTRATE, // Waiting for attainment of landing descent rate
    AUTOTAKEOFF_STATE_AT_DESCENTRATE, // AutoTakeoffing descent rate achieved
    AUTOTAKEOFF_STATE_WTG_FOR_GROUNDEFFECT, // Waiting for group effect to be detected
    AUTOTAKEOFF_STATE_GROUNDEFFECT, // Ground effect detected
    AUTOTAKEOFF_STATE_THRUSTDOWN, // Thrust down sequence
    AUTOTAKEOFF_STATE_THRUSTOFF, // Thrust is now off
    AUTOTAKEOFF_STATE_DISARMED, // Disarmed
    AUTOTAKEOFF_STATE_ABORT, // Abort on error triggerig fallback to hold
    AUTOTAKEOFF_STATE_SIZE
} PathFollowerFSM_AutoTakeoffState_T;

class VtolAutoTakeoffFSM : public PathFollowerFSM {
private:
    static VtolAutoTakeoffFSM *p_inst;
    VtolAutoTakeoffFSM();

public:
    static VtolAutoTakeoffFSM *instance()
    {
        if (!p_inst) {
            p_inst = new VtolAutoTakeoffFSM();
        }
        return p_inst;
    }
    int32_t Initialize(VtolPathFollowerSettingsData *vtolPathFollowerSettings,
                       PathDesiredData *pathDesired,
                       FlightStatusData *flightStatus);
    void Inactive(void);
    void Activate(void);
    void Update(void);
    void BoundThrust(float &ulow, float &uhigh);
    PathFollowerFSMState_T GetCurrentState(void);
    void ConstrainStabiDesired(StabilizationDesiredData *stabDesired);
    float BoundVelocityDown(float);
    void CheckPidScaler(pid_scaler *scaler);
    void Abort(void);

protected:

    // FSM instance data type
    typedef struct {
      StatusVtolAutoTakeoffData   fsmAutoTakeoffStatus;
        PathFollowerFSM_AutoTakeoffState_T currentState;
        TakeOffLocationData takeOffLocation;
        uint32_t stateRunCount;
        uint32_t stateTimeoutCount;
        float    sum1;
        float    sum2;
        float    expectedAutoTakeoffPositionNorth;
        float    expectedAutoTakeoffPositionEast;
        float    thrustLimit;
        float    boundThrustMin;
        float    boundThrustMax;
        uint8_t  observationCount;
        uint8_t  observation2Count;
        uint8_t  flZeroStabiHorizontal;
        uint8_t  flConstrainThrust;
        uint8_t  flLowAltitude;
        uint8_t  flAltitudeHold;
    } VtolAutoTakeoffFSMData_T;

    // FSM state structure
    typedef struct {
        void(VtolAutoTakeoffFSM::*setup) (void); // Called to initialise the state
        void(VtolAutoTakeoffFSM::*run) (uint8_t); // Run the event detection code for a state
    } PathFollowerFSM_AutoTakeoffStateHandler_T;

    // Private variables
    VtolAutoTakeoffFSMData_T *mAutoTakeoffData;
    VtolPathFollowerSettingsData *vtolPathFollowerSettings;
    PathDesiredData *pathDesired;
    FlightStatusData *flightStatus;

    void setup_inactive(void);
    void setup_init_althold(void);
    void setup_wtg_for_descentrate(void);
    void setup_at_descentrate(void);
    void setup_wtg_for_groundeffect(void);
    void run_init_althold(uint8_t);
    void run_wtg_for_descentrate(uint8_t);
    void run_at_descentrate(uint8_t);
    void run_wtg_for_groundeffect(uint8_t);
    void setup_groundeffect(void);
    void run_groundeffect(uint8_t);
    void setup_thrustdown(void);
    void run_thrustdown(uint8_t);
    void setup_thrustoff(void);
    void run_thrustoff(uint8_t);
    void setup_disarmed(void);
    void run_disarmed(uint8_t);
    void setup_abort(void);
    void run_abort(uint8_t);
    void initFSM(void);
    void setState(PathFollowerFSM_AutoTakeoffState_T newState, StatusVtolAutoTakeoffStateExitReasonOptions reason);
    int32_t runState();
    int32_t runAlways();
    void updateVtolAutoTakeoffFSMStatus();
    void assessAltitude(void);

    void setStateTimeout(int32_t count);
    void fallback_to_hold(void);

    static PathFollowerFSM_AutoTakeoffStateHandler_T sAutoTakeoffStateTable[AUTOTAKEOFF_STATE_SIZE];
};

#endif // VTOLAUTOTAKEOFFFSM_H
