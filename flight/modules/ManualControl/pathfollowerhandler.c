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
#include <plans.h>

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
        plan_initialize();
    }

    uint8_t flightMode;
    FlightStatusFlightModeGet(&flightMode);

    if (newinit) {
        // After not being in this mode for a while init at current height
        switch (flightMode) {
        case FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE:
            plan_setup_returnToBase();
            break;

        case FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD:
            plan_setup_positionHold();
            break;

        case FLIGHTSTATUS_FLIGHTMODE_LAND:
            plan_setup_land();
            break;

        default:
            plan_setup_positionHold();

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
    }

    switch (flightMode) {
    // special handling of autoland - behaves like positon hold but with slow altitude decrease
    case FLIGHTSTATUS_FLIGHTMODE_LAND:
        plan_run_land();
        break;
    default:
        break;
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
