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
#ifndef FIXEDWINGFLYCONTROLLER_H
#define FIXEDWINGFLYCONTROLLER_H
#include "pathfollowercontrol.h"

class FixedWingFlyController : public PathFollowerControl {
private:
    static FixedWingFlyController *p_inst;
    FixedWingFlyController();


public:
    static FixedWingFlyController *instance()
    {
        if (!p_inst) {
            p_inst = new FixedWingFlyController();
        }
        return p_inst;
    }

    int32_t Initialize(FixedWingPathFollowerSettingsData *fixedWingSettings,
                       PathDesiredData *pathDesired,
                       PathStatusData *pathStatus);


    void Activate(void);
    void Deactivate(void);
    void SettingsUpdated(void);
    void UpdateAutoPilot(void);
    void ObjectiveUpdated(void);
    uint8_t IsActive(void);
    uint8_t Mode(void);
    void AirspeedStateUpdatedCb(__attribute__((unused)) UAVObjEvent * ev);

private:
    void resetGlobals();
    uint8_t updateAutoPilotFixedWing();
    void updatePathVelocity(float kFF, bool limited);
    uint8_t updateFixedDesiredAttitude();
    bool correctCourse(float *C, float *V, float *F, float s);

    FixedWingPathFollowerSettingsData *fixedWingSettings;
    PathDesiredData *pathDesired;
    PathStatusData *pathStatus;
    uint8_t mActive;
    uint8_t mMode;

    struct pid PIDposH[2];
    struct pid PIDposV;
    struct pid PIDcourse;
    struct pid PIDspeed;
    struct pid PIDpower;
    // correct speed by measured airspeed
    float indicatedAirspeedStateBias;
};

#endif // FIXEDWINGFLYCONTROLLER_H
