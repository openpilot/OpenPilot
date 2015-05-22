/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup FixedWing CONTROL interface class
 * @brief CONTROL interface class for pathfollower fixed wing fly controller
 * @{
 *
 * @file       FixedWingCONTROL.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Executes CONTROL for fixed wing fly objectives
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
#ifndef FIXEDWINGAUTOTAKEOFFCONTROLLER_H
#define FIXEDWINGAUTOTAKEOFFCONTROLLER_H
#include "fixedwingflycontroller.h"

// AutoTakeoff state machine
typedef enum {
    FW_AUTOTAKEOFF_STATE_INACTIVE = 0,
    FW_AUTOTAKEOFF_STATE_LAUNCH,
    FW_AUTOTAKEOFF_STATE_CLIMB,
    FW_AUTOTAKEOFF_STATE_HOLD,
    FW_AUTOTAKEOFF_STATE_ABORT,
    FW_AUTOTAKEOFF_STATE_SIZE
} FixedWingAutoTakeoffControllerState_T;

class FixedWingAutoTakeoffController : public FixedWingFlyController {
public:
    void Activate(void);
    void UpdateAutoPilot(void);

private:
    // variables
    FixedWingAutoTakeoffControllerState_T state;
    float initYaw;
    float maxVelocity;

    // functions
    void setState(FixedWingAutoTakeoffControllerState_T setstate);
    void setAttitude(bool unsafe);
    float getAirspeed(void);
    bool isUnsafe(void);
    void run_inactive(void);
    void run_launch(void);
    void run_climb(void);
    void run_hold(void);
    void run_abort(void);
    void init_inactive(void);
    void init_launch(void);
    void init_climb(void);
    void init_hold(void);
    void init_abort(void);
    void(FixedWingAutoTakeoffController::*const runFunctionTable[FW_AUTOTAKEOFF_STATE_SIZE]) (void) = {
        &FixedWingAutoTakeoffController::run_inactive,
        &FixedWingAutoTakeoffController::run_launch,
        &FixedWingAutoTakeoffController::run_climb,
        &FixedWingAutoTakeoffController::run_hold,
        &FixedWingAutoTakeoffController::run_abort
    };
    void(FixedWingAutoTakeoffController::*const initFunctionTable[FW_AUTOTAKEOFF_STATE_SIZE]) (void) = {
        &FixedWingAutoTakeoffController::init_inactive,
        &FixedWingAutoTakeoffController::init_launch,
        &FixedWingAutoTakeoffController::init_climb,
        &FixedWingAutoTakeoffController::init_hold,
        &FixedWingAutoTakeoffController::init_abort
    };
};

#endif // FIXEDWINGAUTOTAKEOFFCONTROLLER_H
