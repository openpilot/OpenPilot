/**
 ******************************************************************************
 *
 * @file       takeofflocationhandler.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      handles TakeOffLocation
 *             --
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
#include "inc/manualcontrol.h"
#include <stdint.h>
#include <flightstatus.h>
#include <takeofflocation.h>
#include <positionstate.h>

// Private constants

// Private types

// Private variables
static bool locationSet;
// Private functions
static void SetTakeOffLocation();

void takeOffLocationHandlerInit()
{
    TakeOffLocationInitialize();
    // check whether there is a preset/valid takeoff location
    uint8_t mode;
    uint8_t status;
    TakeOffLocationModeGet(&mode);
    TakeOffLocationStatusGet(&status);
    // preset with invalid location will actually behave like FirstTakeoff
    if (mode == TAKEOFFLOCATION_MODE_PRESET && status == TAKEOFFLOCATION_STATUS_VALID) {
        locationSet = true;
    } else {
        locationSet = false;
        status = TAKEOFFLOCATION_STATUS_INVALID;
        TakeOffLocationStatusSet(&status);
    }
}
/**
 * Handles TakeOffPosition location setup
 * @param newinit
 */
void takeOffLocationHandler()
{
    uint8_t armed;
    uint8_t status;

    FlightStatusArmedGet(&armed);

    // Location already acquired/preset
    if (armed == FLIGHTSTATUS_ARMED_ARMED && locationSet) {
        return;
    }

    TakeOffLocationStatusGet(&status);

    switch (armed) {
    case FLIGHTSTATUS_ARMED_ARMING:
    case FLIGHTSTATUS_ARMED_ARMED:
        if (!locationSet || status != TAKEOFFLOCATION_STATUS_VALID) {
            uint8_t mode;
            TakeOffLocationModeGet(&mode);

            if ((mode != TAKEOFFLOCATION_MODE_PRESET) || (status == TAKEOFFLOCATION_STATUS_INVALID)) {
                SetTakeOffLocation();
            } else {
                locationSet = true;
            }
        }
        break;

    case FLIGHTSTATUS_ARMED_DISARMED:
        // unset if location is to be acquired at each arming
        if (locationSet) {
            uint8_t mode;
            TakeOffLocationModeGet(&mode);
            if (mode == TAKEOFFLOCATION_MODE_ARMINGLOCATION) {
                locationSet = false;
                status = TAKEOFFLOCATION_STATUS_INVALID;
                TakeOffLocationStatusSet(&status);
            }
        }
        break;
    }
}

/**
 * Retrieve TakeOffLocation from current PositionStatus
 */
void SetTakeOffLocation()
{
    TakeOffLocationData takeOffLocation;

    TakeOffLocationGet(&takeOffLocation);

    PositionStateData positionState;
    PositionStateGet(&positionState);

    takeOffLocation.North  = positionState.North;
    takeOffLocation.East   = positionState.East;
    takeOffLocation.Down   = positionState.Down;
    takeOffLocation.Status = TAKEOFFLOCATION_STATUS_VALID;

    TakeOffLocationSet(&takeOffLocation);
    locationSet = true;
}
