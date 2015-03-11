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
#ifndef VTOLLANDFSM_H
#define VTOLLANDFSM_H

extern "C" {
#include "statusvtolland.h"
}
#include "pathfollowerfsm.h"

// Landing state machine
typedef enum {
    LAND_STATE_INACTIVE = 0, // Inactive state is the initialised state on startup
    LAND_STATE_INIT_ALTHOLD, // Initiate altitude hold before starting descent
    LAND_STATE_WTG_FOR_DESCENTRATE, // Waiting for attainment of landing descent rate
    LAND_STATE_AT_DESCENTRATE, // Landing descent rate achieved
    LAND_STATE_WTG_FOR_GROUNDEFFECT, // Waiting for group effect to be detected
    LAND_STATE_GROUNDEFFECT, // Ground effect detected
    LAND_STATE_THRUSTDOWN, // Thrust down sequence
    LAND_STATE_THRUSTOFF, // Thrust is now off
    LAND_STATE_DISARMED, // Disarmed
    LAND_STATE_ABORT, // Abort on error triggerig fallback to hold
    LAND_STATE_SIZE
} PathFollowerFSM_LandState_T;

class VtolLandFSM : public PathFollowerFSM {
private:
    static VtolLandFSM *p_inst;
    VtolLandFSM();

public:
    static VtolLandFSM *instance()
    {
        if (!p_inst) {
            p_inst = new VtolLandFSM();
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
        StatusVtolLandData   fsmLandStatus;
        PathFollowerFSM_LandState_T currentState;
        TakeOffLocationData takeOffLocation;
        uint32_t stateRunCount;
        uint32_t stateTimeoutCount;
        float    sum1;
        float    sum2;
        float    expectedLandPositionNorth;
        float    expectedLandPositionEast;
        float    thrustLimit;
        float    boundThrustMin;
        float    boundThrustMax;
        uint8_t  observationCount;
        uint8_t  observation2Count;
        uint8_t  flZeroStabiHorizontal;
        uint8_t  flConstrainThrust;
        uint8_t  flLowAltitude;
        uint8_t  flAltitudeHold;
    } VtolLandFSMData_T;

    // FSM state structure
    typedef struct {
        void(VtolLandFSM::*setup) (void); // Called to initialise the state
        void(VtolLandFSM::*run) (uint8_t); // Run the event detection code for a state
    } PathFollowerFSM_LandStateHandler_T;

    // Private variables
    VtolLandFSMData_T *mLandData;
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
    void setState(PathFollowerFSM_LandState_T newState, StatusVtolLandStateExitReasonOptions reason);
    int32_t runState();
    int32_t runAlways();
    void updateVtolLandFSMStatus();
    void assessAltitude(void);

    void setStateTimeout(int32_t count);
    void fallback_to_hold(void);

    static PathFollowerFSM_LandStateHandler_T sLandStateTable[LAND_STATE_SIZE];
};

#endif // VTOLLANDFSM_H
