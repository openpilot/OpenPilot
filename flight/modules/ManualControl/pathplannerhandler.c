/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup ManualControl
 * @brief Interpretes the control input in ManualControlCommand
 * @{
 *
 * @file       pathplannerhandler.c
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

// Private constants

// Private types

// Private functions


/**
 * @brief Handler to control Navigated flightmodes. FlightControl is governed by PathFollower, controlled indirectly via PathPlanner
 * @input: NONE: fully automated mode -- TODO recursively call handler for advanced stick commands to affect navigation
 * @output: NONE
 */
void pathPlannerHandler(__attribute__((unused)) bool newinit)
{
    /**
     *
     * TODO: In fully autonomous mode, commands to the navigation facility
     * can be encoded through standard input cmd channels, as the pathplanner
     * pathfollower generally ignores them
     *
     * this should be provided by a separate handler invoked here, as the
     * handler for pathFollower should likely invoke them as well!!!
     * (inputs also ignored)
     *
     */
}


/**
 * @}
 * @}
 */
