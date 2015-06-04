/*
 ******************************************************************************
 *
 * @file       FixedWingLandController.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Fixed wing fly controller implementation
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

extern "C" {
#include <openpilot.h>

#include <pathdesired.h>
#include <fixedwingpathfollowersettings.h>
#include <flightstatus.h>
#include <pathstatus.h>
#include <stabilizationdesired.h>
}

// C++ includes
#include "fixedwinglandcontroller.h"

// Private constants

// pointer to a singleton instance
FixedWingLandController *FixedWingLandController::p_inst = 0;

FixedWingLandController::FixedWingLandController()
    : fixedWingSettings(NULL), mActive(false), mMode(0)
{}

// Called when mode first engaged
void FixedWingLandController::Activate(void)
{
    if (!mActive) {
        mActive = true;
        SettingsUpdated();
        resetGlobals();
        mMode   = pathDesired->Mode;
    }
}

uint8_t FixedWingLandController::IsActive(void)
{
    return mActive;
}

uint8_t FixedWingLandController::Mode(void)
{
    return mMode;
}

// Objective updated in pathdesired
void FixedWingLandController::ObjectiveUpdated(void)
{}

void FixedWingLandController::Deactivate(void)
{
    if (mActive) {
        mActive = false;
        resetGlobals();
    }
}


void FixedWingLandController::SettingsUpdated(void)
{}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t FixedWingLandController::Initialize(FixedWingPathFollowerSettingsData *ptr_fixedWingSettings)
{
    PIOS_Assert(ptr_fixedWingSettings);

    fixedWingSettings = ptr_fixedWingSettings;

    resetGlobals();

    return 0;
}

/**
 * reset integrals
 */
void FixedWingLandController::resetGlobals()
{
    pathStatus->path_time = 0.0f;
    pathStatus->path_direction_north = 0.0f;
    pathStatus->path_direction_east  = 0.0f;
    pathStatus->path_direction_down  = 0.0f;
    pathStatus->correction_direction_north = 0.0f;
    pathStatus->correction_direction_east  = 0.0f;
    pathStatus->correction_direction_down  = 0.0f;
    pathStatus->error = 0.0f;
    pathStatus->fractional_progress = 0.0f;
}

/**
 * fixed wing autopilot
 * use fixed attitude heading towards destination waypoint
 */
void FixedWingLandController::UpdateAutoPilot()
{
    StabilizationDesiredData stabDesired;

    stabDesired.Roll   = 0.0f;
    stabDesired.Pitch  = fixedWingSettings->LandingPitch;
    stabDesired.Thrust = 0.0f;
    stabDesired.StabilizationMode.Roll   = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;

    // find out vector direction of *runway* (if any)
    // and align, otherwise just stay straight ahead
    if (fabsf(pathDesired->Start.North - pathDesired->End.North) < 1e-3f &&
        fabsf(pathDesired->Start.East - pathDesired->End.East) < 1e-3f) {
        stabDesired.Yaw = 0.0f;
        stabDesired.StabilizationMode.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
    } else {
        stabDesired.Yaw = RAD2DEG(atan2f(pathDesired->End.East - pathDesired->Start.East, pathDesired->End.North - pathDesired->Start.North));
        if (stabDesired.Yaw < -180.0f) {
            stabDesired.Yaw += 360.0f;
        }
        if (stabDesired.Yaw > 180.0f) {
            stabDesired.Yaw -= 360.0f;
        }
        stabDesired.StabilizationMode.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    }
    StabilizationDesiredSet(&stabDesired);
    AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);

    PathStatusSet(pathStatus);
}
