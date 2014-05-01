/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup ManualControl
 * @brief Interpretes the control input in ManualControlCommand
 * @{
 *
 * @file       manualhandler.c
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
#include <manualcontrolcommand.h>
#include <actuatordesired.h>

// Private constants

// Private types

// Private functions


/**
 * @brief Handler to control Manual flightmode - input directly steers actuators
 * @input: ManualControlCommand
 * @output: ActuatorDesired
 */
void manualHandler(bool newinit)
{
    if (newinit) {
        ActuatorDesiredInitialize();
    }

    ManualControlCommandData cmd;
    ManualControlCommandGet(&cmd);

    ActuatorDesiredData actuator;
    ActuatorDesiredGet(&actuator);

    actuator.Roll   = cmd.Roll;
    actuator.Pitch  = cmd.Pitch;
    actuator.Yaw    = cmd.Yaw;
    actuator.Thrust = cmd.Thrust;

    ActuatorDesiredSet(&actuator);
}


/**
 * @}
 * @}
 */
