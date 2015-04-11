/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_PacketRxOk PacketRxOk functions
 * @brief PIOS interface to read PacketRxOk
 * @{
 *
 * @file       pios_packetrxok_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      PacketRxOk Private structures.
 *             see http://code.google.com/p/minoposd/wiki/PacketRxOk
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

#ifndef PIOS_PACKETRXOK_PRIV_H
#define PIOS_PACKETRXOK_PRIV_H

#ifdef PIOS_INCLUDE_PACKETRXOK

#include <pios.h>
#include <pios_stm32.h>

extern int32_t PIOS_PacketRxOk_Init(uint32_t *pios_packetrxok_id, uint32_t pios_gpio_packetrxok_id, GPIO_TypeDef* GPIO, uint16_t GPIO_Pin);

#endif /* PIOS_INCLUDE_PACKETRXOK */

#endif /* PIOS_PACKETRXOK_PRIV_H */

/**
 * @}
 * @}
 */
