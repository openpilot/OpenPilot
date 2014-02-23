/**
 ******************************************************************************
 *
 * @file       pios_port_abstraction.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Allow a generic abstraction of ports from board types.
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
#include <pios_port_abstraction.h>
#include <stdlib.h>
#include <pios_debug.h>

// Number of Port allowed by current system
static uint8_t usartPortCount;
// Usart_ids associated with each registered port.
static uint32_t *usartPortList;
// Tells if a port is available (registered and not yet used)
static bool *usartPortAvailabilityList;
/**
 * Initialize port abstraction.
 * @param usartCounts number of ports handled by current system
 */
void PIOS_PORT_ABSTRACTION_Init(uint8_t usartCounts)
{
    usartPortCount = usartCounts;

    if (usartCounts != 0) {
        usartPortAvailabilityList = (bool *)malloc(sizeof(bool) * usartCounts);
        usartPortList = (uint32_t *)malloc(sizeof(uint32_t) * usartCounts);

        // initializes port list
        for (uint8_t x = 0; x < usartCounts; x++) {
            usartPortAvailabilityList[x] = true;
            usartPortList[x] = (uint32_t)0;
        }
    } else {
        // No ports available
        usartPortAvailabilityList = (bool *)0;
        usartPortList = (uint32_t *)0;
    }
}

/**
 * Register a Usart to be used as generic port
 * @param portId Port number
 * @param usartId Usart Id
 */
extern void PIOS_PORT_ABSTRACTION_RegisterUsart(uint8_t portId, uint32_t usartId)
{
    PIOS_Assert(portId < usartPortCount);
    usartPortList[portId] = usartId;
    usartPortAvailabilityList[portId] = true;
}

/**
 * Retrieve usart_id associated with portId if available. This calls flags the port as unavailable
 * @param portId Port number
 * @return Usart Id if available, 0 otherwise
 */
extern uint32_t PIOS_PORT_ABSTRACTION_PickPort(uint8_t portId)
{
    if (portId < usartPortCount && usartPortAvailabilityList[portId]) {
        // Flags port as used
        usartPortAvailabilityList[portId] = false;
        return usartPortList[portId];
    }
    return (uint32_t)0;
}

/**
 * Check port availability
 * @param portId Port Number
 * @return true if available
 */
extern bool PIOS_PORT_ABSTRACTION_IsUsartAvailable(uint8_t portId)
{
    if (portId < usartPortCount) {
        return usartPortAvailabilityList[portId];
    }
    return false;
}
