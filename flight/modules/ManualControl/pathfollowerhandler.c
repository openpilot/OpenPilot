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
    uint8_t positionRoamFlightMode;
    FlightStatusFlightModeGet(&flightMode);
    FlightStatusPositionRoamStateSet(&positionRoamFlightMode);

    if (newinit) {
        // After not being in this mode for a while init at current height
        switch (flightMode) {
        case FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE:
            plan_setup_returnToBase();
            break;

        case FLIGHTSTATUS_FLIGHTMODE_POSITIONROAM:
            if (positionRoamFlightMode == FLIGHTSTATUS_POSITIONROAMSTATE_BRAKING) {
        	// Just initiated braking after returning from stabi control
                plan_setup_brake();
            }
            else if (positionRoamFlightMode == FLIGHTSTATUS_POSITIONROAMSTATE_POSITIONHOLD) {
                plan_setup_positionHold(); // this probably won't occur, it will be called
					   // directly at end of braking.
            }
	    break;
        case FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD:
            plan_setup_positionHold();
            break;
        case FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIOFPV:
            plan_setup_PositionVarioFPV();
            break;
        case FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIOLOS:
            plan_setup_PositionVarioLOS();
            break;
        case FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIONSEW:
            plan_setup_PositionVarioNSEW();
            break;

        case FLIGHTSTATUS_FLIGHTMODE_LAND:
            plan_setup_land();
            break;
        case FLIGHTSTATUS_FLIGHTMODE_AUTOCRUISE:
            plan_setup_AutoCruise();
            break;

        default:
            plan_setup_positionHold();
            break;
        }
    }

    switch (flightMode) {
    case FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIOFPV:
        plan_run_PositionVarioFPV();
        break;
    case FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIOLOS:
        plan_run_PositionVarioLOS();
        break;
    case FLIGHTSTATUS_FLIGHTMODE_POSITIONVARIONSEW:
        plan_run_PositionVarioNSEW();
        break;
    case FLIGHTSTATUS_FLIGHTMODE_LAND:
        plan_run_land();
        break;
    case FLIGHTSTATUS_FLIGHTMODE_AUTOCRUISE:
        plan_run_AutoCruise();
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
