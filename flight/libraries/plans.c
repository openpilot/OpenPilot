/**
 ******************************************************************************
 * @addtogroup OpenPilotLibraries OpenPilot Libraries
 * @{
 * @addtogroup Navigation
 * @brief setups RTH/PH and other pathfollower/pathplanner status
 * @{
 *
 * @file       plan.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************/
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

#include <plans.h>
#include <openpilot.h>
#include <attitudesettings.h>
#include <takeofflocation.h>
#include <pathdesired.h>
#include <positionstate.h>
#include <flightmodesettings.h>

/**
 * @brief initialize UAVOs and structs used by this library
 */
void plan_initialize()
{
    TakeOffLocationInitialize();
    PositionStateInitialize();
    PathDesiredInitialize();
    FlightModeSettingsInitialize();
}

/**
 * @brief setup pathplanner/pathfollower for positionhold
 */
void plan_setup_positionHold()
{
    PositionStateData positionState;

    PositionStateGet(&positionState);
    PathDesiredData pathDesired;
    PathDesiredGet(&pathDesired);

    pathDesired.Start.North      = positionState.North;
    pathDesired.Start.East       = positionState.East;
    pathDesired.Start.Down       = positionState.Down;
    pathDesired.End.North        = positionState.North;
    pathDesired.End.East         = positionState.East;
    pathDesired.End.Down         = positionState.Down;
    pathDesired.StartingVelocity = 1;
    pathDesired.EndingVelocity   = 0;
    pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;

    PathDesiredSet(&pathDesired);
}

/**
 * @brief setup pathplanner/pathfollower for return to base
 */
void plan_setup_returnToBase()
{
    // Simple Return To Base mode - keep altitude the same applying configured delta, fly to takeoff position
    float positionStateDown;

    PositionStateDownGet(&positionStateDown);

    PathDesiredData pathDesired;
    PathDesiredGet(&pathDesired);

    TakeOffLocationData takeoffLocation;
    TakeOffLocationGet(&takeoffLocation);

    // TODO: right now VTOLPF does fly straight to destination altitude.
    // For a safer RTB destination altitude will be the higher between takeofflocation and current position (corrected with safety margin)

    float destDown;
    FlightModeSettingsReturnToBaseAltitudeOffsetGet(&destDown);
    destDown = MIN(positionStateDown, takeoffLocation.Down) - destDown;

    pathDesired.Start.North      = takeoffLocation.North;
    pathDesired.Start.East       = takeoffLocation.East;
    pathDesired.Start.Down       = destDown;

    pathDesired.End.North        = takeoffLocation.North;
    pathDesired.End.East         = takeoffLocation.East;
    pathDesired.End.Down         = destDown;

    pathDesired.StartingVelocity = 1;
    pathDesired.EndingVelocity   = 0;
    pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;

    PathDesiredSet(&pathDesired);
}

void plan_setup_land()
{
    plan_setup_positionHold();
}

/**
 * @brief execute land
 */
void plan_run_land()
{
    PathDesiredEndData pathDesiredEnd;

    PathDesiredEndGet(&pathDesiredEnd);

    PositionStateDownGet(&pathDesiredEnd.Down);
    pathDesiredEnd.Down += 5;

    PathDesiredEndSet(&pathDesiredEnd);
}
