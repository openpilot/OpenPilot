/**
 ******************************************************************************
 *
 * @file       pios_port_abstraction.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Allow a generic abstraction of ports from board types
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
#ifndef PIOS_PORT_ABSTRACTION_H
#define PIOS_PORT_ABSTRACTION_H

#include <stdint.h>
#include <stdbool.h>

extern void PIOS_PORT_ABSTRACTION_Init(uint8_t usartCounts);

extern void PIOS_PORT_ABSTRACTION_RegisterUsart(uint8_t portId, uint32_t usart_id);
extern uint32_t PIOS_PORT_ABSTRACTION_PickPort(uint8_t portId);
extern bool PIOS_PORT_ABSTRACTION_IsUsartAvailable(uint8_t portId);

#endif /* PIOS_PORT_ABSTRACTION_H */
