/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_SBUS S.Bus Functions
 * @brief PIOS interface to read and write from Futaba S.Bus port
 * @{
 *
 * @file       pios_sbus_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Futaba S.Bus Private structures.
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

#ifndef PIOS_SBUS_PRIV_H
#define PIOS_SBUS_PRIV_H

#include <pios.h>
#include <pios_stm32.h>
#include <pios_usart_priv.h>

struct pios_sbus_cfg {
	const struct pios_usart_cfg *pios_usart_sbus_cfg;
	GPIO_InitTypeDef gpio_init;
	uint32_t remap;		/* GPIO_Remap_* */
	struct stm32_irq irq;
	GPIO_TypeDef *port;
	uint16_t pin;
};

extern void PIOS_SBUS_irq_handler();

extern uint8_t pios_sbus_num_channels;
extern const struct pios_sbus_cfg pios_sbus_cfg;

#endif /* PIOS_SBUS_PRIV_H */

/**
 * @}
 * @}
 */
