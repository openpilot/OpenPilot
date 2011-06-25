/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_PPM PPM Functions
 * @brief PIOS interface to read and write from ppm port
 * @{
 *
 * @file       pios_ppm_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      ppm private structures.
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

#ifndef PIOS_PPM_PRIV_H
#define PIOS_PPM_PRIV_H

#include <pios.h>
#include <pios_stm32.h>

struct pios_ppmsv_cfg {
	TIM_TimeBaseInitTypeDef tim_base_init;
	struct stm32_irq irq;
	TIM_TypeDef * timer;
	uint16_t ccr;
};

struct pios_ppm_cfg {
	TIM_TimeBaseInitTypeDef tim_base_init;
	TIM_ICInitTypeDef tim_ic_init;
	GPIO_InitTypeDef gpio_init;
	uint32_t remap;		/* GPIO_Remap_* */
	struct stm32_irq irq;
	TIM_TypeDef * timer;
	GPIO_TypeDef * port;
	uint16_t ccr;
};

extern void PIOS_PPM_irq_handler();
extern void PIOS_PPMSV_irq_handler();

extern uint8_t pios_ppm_num_channels;
extern const struct pios_ppm_cfg pios_ppm_cfg;
extern const struct pios_ppmsv_cfg pios_ppmsv_cfg;

extern const struct pios_rcvr_driver pios_ppm_rcvr_driver;

extern void PIOS_PPM_Init(void);

#endif /* PIOS_PPM_PRIV_H */

/**
 * @}
 * @}
 */
