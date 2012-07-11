/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_STM32 STM32 HAL
 * @brief STM32 specific global data structures 
 * @{
 *
 * @file       pios_stm32.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Types that are specific to the STM32 peripherals
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

#ifndef PIOS_STM32_H
#define PIOS_STM32_H

struct stm32_irq {
	void (*handler) (uint32_t);
	uint32_t flags;
	NVIC_InitTypeDef init;
};

struct stm32_exti {
	EXTI_InitTypeDef init;
};

struct stm32_dma_chan {
#if defined(STM32F2XX) || defined(STM32F4XX)
	DMA_Stream_TypeDef *channel;
#else
	DMA_Channel_TypeDef *channel;
#endif
	DMA_InitTypeDef init;
};

struct stm32_dma {
	uint32_t ahb_clk;			/* ignored on STM32F2XX */
	struct stm32_irq irq;
	struct stm32_dma_chan rx;
	struct stm32_dma_chan tx;
};

struct stm32_gpio {
	GPIO_TypeDef *gpio;
	GPIO_InitTypeDef init;
	uint8_t pin_source;
};

/**
  * @}
  * @}
  */

#endif /* PIOS_STM32_H */
