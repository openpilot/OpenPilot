/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup ManualControl
 * @brief Interpretes the control input in ManualControlCommand
 * @{
 *
 * @file       pathfollowerhandler.c
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

#include "inc/manualcontrol.h"
#include <pathdesired.h>
#include <manualcontrolcommand.h>
#include <flightstatus.h>
#include <positionstate.h>
#include <flightmodesettings.h>


#if defined(REVOLUTION)
#include <takeofflocation.h>
// Private constants

// Private types

// Private functions

/**
 * @brief Handler to control Guided flightmodes. FlightControl is governed by PathFollower, control via PathDesired
 * @input: NONE: fully automated mode -- TODO recursively call handler for advanced stick commands
 * @output: PathDesired
 */
void pathFollowerHandler(bool newinit)
{
    if (newinit) {
        PathDesiredInitialize();
        PositionStateInitialize();
    }

    FlightStatusData flightStatus;
    FlightStatusGet(&flightStatus);

    if (newinit) {
        // After not being in this mode for a while init at current height
        PositionStateData positionState;
        PositionStateGet(&positionState);
        FlightModeSettingsData settings;
        FlightModeSettingsGet(&settings);
        PathDesiredData pathDesired;
        PathDesiredGet(&pathDesired);
        TakeOffLocationData takeoffLocation;
        TakeOffLocationGet(&takeoffLocation);
        switch (flightStatus.FlightMode) {
        case FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE:

            // Simple Return To Base mode - keep altitude the same applying configured delta, fly to takeoff position
            if (takeoffLocation.Status == TAKEOFFLOCATION_STATUS_VALID) {
                pathDesired.Start.North = takeoffLocation.North;
                pathDesired.Start.East  = takeoffLocation.East;
                pathDesired.End.North   = takeoffLocation.North;
                pathDesired.End.East    = takeoffLocation.East;
            } else {
                // in case for some bad reason takeofflocation isn't valid, fails back to home location.
                pathDesired.Start.North = 0;
                pathDesired.Start.East  = 0;
                pathDesired.End.North   = 0;
                pathDesired.End.East    = 0;
            }

            pathDesired.Start.Down       = positionState.Down - settings.ReturnToHomeAltitudeOffset;
            pathDesired.End.Down         = positionState.Down - settings.ReturnToHomeAltitudeOffset;
            pathDesired.StartingVelocity = 1;
            pathDesired.EndingVelocity   = 0;
            pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;
            break;
        default:

            pathDesired.Start.North      = positionState.North;
            pathDesired.Start.East       = positionState.East;
            pathDesired.Start.Down       = positionState.Down;
            pathDesired.End.North        = positionState.North;
            pathDesired.End.East         = positionState.East;
            pathDesired.End.Down         = positionState.Down;
            pathDesired.StartingVelocity = 1;
            pathDesired.EndingVelocity   = 0;
            pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;
            /* Disable this section, until such time as proper discussion can be had about how to implement it for all types of crafts.
               } else {
               PathDesiredData pathDesired;
               PathDesiredGet(&pathDesired);
               pathDesired.End[PATHDESIRED_END_NORTH] += dT * -cmd->Pitch;
               pathDesired.End[PATHDESIRED_END_EAST] += dT * cmd->Roll;
               pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;
               PathDesiredSet(&pathDesired);
             */
            break;
        }
        PathDesiredSet(&pathDesired);
    }

    // special handling of autoland - behaves like positon hold but with slow altitude decrease
    if (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_LAND) {
        PositionStateData positionState;
        PositionStateGet(&positionState);
        PathDesiredData pathDesired;
        PathDesiredGet(&pathDesired);
        pathDesired.End.Down = positionState.Down + 5;
        PathDesiredSet(&pathDesired);
    }
}

#else /* if defined(REVOLUTION) */
void pathFollowerHandler(__attribute__((unused)) bool newinit)
{
    AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_CRITICAL); // should not be called
}
#endif // REVOLUTION


/**
 * @}
 * @}
 */
