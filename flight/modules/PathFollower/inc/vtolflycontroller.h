/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup PathFollower CONTROL interface class
 * @brief vtol fly controller class definition
 * @{
 *
 * @file       vtolflycontroller.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Executes fly control for vtols
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
#ifndef VTOLFLYCONTROLLER_H
#define VTOLFLYCONTROLLER_H
#include "pathfollowercontrol.h"
#include "pidcontrolne.h"
#include "pidcontroldown.h"

class VtolFlyController : public PathFollowerControl {
private:
    static VtolFlyController *p_inst;
    VtolFlyController();


public:
    static VtolFlyController *instance()
    {
        if (!p_inst) {
            p_inst = new VtolFlyController();
        }
        return p_inst;
    }

    int32_t Initialize(VtolPathFollowerSettingsData *vtolPathFollowerSettings);


    void Activate(void);
    void Deactivate(void);
    void SettingsUpdated(void);
    void UpdateAutoPilot(void);
    void ObjectiveUpdated(void);
    uint8_t IsActive(void);
    uint8_t Mode(void);

private:
    void UpdateVelocityDesired(void);
    int8_t UpdateStabilizationDesired(bool yaw_attitude, float yaw_direction);
    void UpdateDesiredAttitudeEmergencyFallback();
    void fallback_to_hold(void);
    float updateTailInBearing();
    float updateCourseBearing();
    float updatePathBearing();
    float updatePOIBearing();
    uint8_t RunAutoPilot();

    VtolPathFollowerSettingsData *vtolPathFollowerSettings;
    PIDControlNE controlNE;
    PIDControlDown controlDown;
    uint8_t mActive;
    uint8_t mManualThrust;
    uint8_t mMode;
    float vtolEmergencyFallback;
    bool vtolEmergencyFallbackSwitch;
};

#endif // VTOLFLYCONTROLLER_H
