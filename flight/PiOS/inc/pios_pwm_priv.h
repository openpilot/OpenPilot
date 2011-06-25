/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_SERVO Servo Functions
 * @brief PIOS interface to read and write from servo PWM ports
 * @{
 *
 * @file       pios_servo_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Servo private structures.
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

#ifndef PIOS_PWM_PRIV_H
#define PIOS_PWM_PRIV_H

#include <pios.h>
#include <pios_stm32.h>

struct pios_pwm_channel {
	TIM_TypeDef * timer;
	GPIO_TypeDef * port;
	uint16_t ccr;
	uint8_t channel;
	uint16_t pin;
};

struct pios_pwm_cfg {
	TIM_TimeBaseInitTypeDef tim_base_init;
	TIM_ICInitTypeDef tim_ic_init;
	GPIO_InitTypeDef gpio_init;
	uint32_t remap;		/* GPIO_Remap_* */
	struct stm32_irq irq;
	const struct pios_pwm_channel *const channels;
	uint8_t num_channels;
};

extern void PIOS_PWM_irq_handler(TIM_TypeDef * timer);

extern uint8_t pios_pwm_num_channels;
extern const struct pios_pwm_cfg pios_pwm_cfg;

extern const struct pios_rcvr_driver pios_pwm_rcvr_driver;

extern void PIOS_PWM_Init(void);

#endif /* PIOS_PWM_PRIV_H */

/**
 * @}
 * @}
 */
