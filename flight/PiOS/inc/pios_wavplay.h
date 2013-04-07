/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_WAVPLAY Code for wave audio generator
 * @brief Wave audio generator
 * @{
 *
 * @file       pios_wavplay.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      audio generator
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
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

#ifndef PIOS_WAVPLAY_H
#define PIOS_WAVPLAY_H

#include <pios_stm32.h>

#define BUFFERSIZE	512	//Defines the buffer size to hold data from the SD card.

struct pios_dac_cfg {
	TIM_TypeDef *timer;
	TIM_TimeBaseInitTypeDef time_base_init;
	struct stm32_irq irq;
	struct stm32_dma dma;
	uint32_t channel;
	DAC_InitTypeDef dac_init;
	struct stm32_gpio dac_io;
};

extern void PIOS_WavPlay_Init(const struct pios_dac_cfg * cfg);
extern uint8_t WavePlayer_Start(void);

#endif /* PIOS_WAVPLAY_H */
