/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup PathFollower FSM Land
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
#ifndef FSM_LAND_H
#define FSM_LAND_H

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

// forward declarations
struct VelocityDesiredData;
struct StabilizationDesiredData;

// public initialisations go here
int32_t FSMLandInitialize(VtolPathFollowerSettingsData *vtolPathFollowerSettings,
                          PathDesiredData *pathDesired,
                          FlightStatusData *flightStatus);
void FSMLandInactive(void);
void FSMLandActivate(void);
void FSMLandUpdate(void);
void FSMLandSettingsUpdated(void);
float FSMLandBoundThrust(float thrustDesired);
PathFollowerFSM_LandState_T FSMGetCurrentState(void);
void FSMLandConstrainStabiDesired(StabilizationDesiredData *stabDesired);
float FSMLandBoundVelocityDown(float);
void FSMLandCheckPidScalar(pid_scaler *scaler);

#endif // FSM_LAND_H
