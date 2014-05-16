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
typedef enum {
    HANDLER_STATUS_UNSET,
    HANDLER_STATUS_PENDING,
    HANDLER_STATUS_SET,
} HandlerStatus_t;

// Private variables
static HandlerStatus_t handlerStatus;
static const uint8_t TakeOffStatusInvalid = TAKEOFFLOCATION_STATUS_INVALID;
static const uint8_t TakeOffStatusValid   = TAKEOFFLOCATION_STATUS_VALID;
// Private functions
static void SetTakeOffLocation();

/**
 * Handles TakeOffPosition location setup
 * @param newinit
 */
void takeOffLocationHandler(bool newinit)
{
    if (newinit) {
        // check whether there is a preset/valid takeoff location
        uint8_t mode;
        uint8_t status;
        TakeOffLocationModeGet(&mode);
        TakeOffLocationStatusGet(&status);
        if (mode == TAKEOFFLOCATION_MODE_PRESET && status == TAKEOFFLOCATION_STATUS_VALID) {
            handlerStatus = HANDLER_STATUS_SET;
        } else {
            handlerStatus = HANDLER_STATUS_UNSET;
        }
        return;
    }

    uint8_t armed;
    FlightStatusArmedGet(&armed);

    // Location already acquired/preset
    if (armed == FLIGHTSTATUS_ARMED_ARMED && handlerStatus == HANDLER_STATUS_SET) {
        return;
    }

    switch (armed) {
    case FLIGHTSTATUS_ARMED_ARMED:
        if (handlerStatus == HANDLER_STATUS_UNSET) {
            // this is just a safety "net", should for any reason the ARMING status is skipped
            SetTakeOffLocation();
        } else if (handlerStatus == HANDLER_STATUS_PENDING) {
            // confirms a "pending" TakeOffPosition
            uint8_t newStatus = TAKEOFFLOCATION_STATUS_VALID;
            TakeOffLocationStatusSet(&newStatus);
            handlerStatus = HANDLER_STATUS_SET;
        }
        break;

    case FLIGHTSTATUS_ARMED_DISARMED:
        // unset if location is to be acquired at each arming
        if (handlerStatus == HANDLER_STATUS_SET) {
            uint8_t mode;
            TakeOffLocationModeGet(&mode);
            if (mode == TAKEOFFLOCATION_MODE_ARMINGLOCATION) {
                handlerStatus = HANDLER_STATUS_UNSET;
                uint8_t newStatus = TAKEOFFLOCATION_STATUS_INVALID;
                TakeOffLocationStatusSet(&newStatus);
            }
            // Clear a previous "pending" flag
        } else if (handlerStatus == HANDLER_STATUS_PENDING) {
            handlerStatus = HANDLER_STATUS_UNSET;
        }
        break;

    case FLIGHTSTATUS_ARMED_ARMING:
        if (handlerStatus == HANDLER_STATUS_UNSET) {
            SetTakeOffLocation();
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
    takeOffLocation.North = positionState.North;
    takeOffLocation.East  = positionState.East;
    takeOffLocation.Down  = positionState.Down;
    handlerStatus = HANDLER_STATUS_PENDING;
}
