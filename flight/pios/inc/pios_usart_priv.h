/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USART USART Functions
 * @brief PIOS interface for USART port
 * @{
 *
 * @file       pios_usart_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      USART private definitions.
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

#ifndef PIOS_USART_PRIV_H
#define PIOS_USART_PRIV_H

#include <pios.h>
#include <pios_stm32.h>
#include "fifo_buffer.h"
#include "pios_usart.h"

extern const struct pios_com_driver pios_usart_com_driver;

struct pios_usart_cfg {
	USART_TypeDef *regs;
	uint32_t remap;		/* GPIO_Remap_* */
	USART_InitTypeDef init;
	struct stm32_gpio rx;
	struct stm32_gpio tx;
	struct stm32_irq irq;
};

extern int32_t PIOS_USART_Init(uint32_t * usart_id, const struct pios_usart_cfg * cfg);
extern const struct pios_usart_cfg * PIOS_USART_GetConfig(uint32_t usart_id);

#endif /* PIOS_USART_PRIV_H */

/**
  * @}
  * @}
  */
