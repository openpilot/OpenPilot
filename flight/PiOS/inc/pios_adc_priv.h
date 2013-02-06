/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_ADC ADC Functions
 * @brief PIOS interface for USART port
 * @{
 *
 * @file       pios_adc_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      ADC private definitions.
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

#ifndef PIOS_ADC_PRIV_H
#define PIOS_ADC_PRIV_H

#include <pios.h>
#include <pios_stm32.h>
#include <pios_adc.h>
#include <pios_adc_rcvr.h>
#include <fifo_buffer.h>

struct pios_adc_cfg {
	ADC_TypeDef* adc_dev;
	struct stm32_dma dma;
	uint32_t half_flag;
	uint32_t full_flag;
	uint16_t max_downsample;
};


enum pios_adc_dev_magic {
	PIOS_ADC_DEV_MAGIC = 0x58375124,
};

struct pios_adc_dev {
	const struct pios_adc_cfg * cfg;	
	ADCCallback callback_function;
#if defined(PIOS_INCLUDE_FREERTOS)
	xQueueHandle data_queue;
#endif
	volatile int16_t *valid_data_buffer;
	volatile uint8_t adc_oversample;
	uint8_t dma_block_size;
	uint16_t dma_half_buffer_size;
#if defined(PIOS_INCLUDE_ADC)
	int16_t fir_coeffs[PIOS_ADC_MAX_SAMPLES+1]  __attribute__ ((aligned(4)));
	volatile int16_t raw_data_buffer[PIOS_ADC_MAX_SAMPLES]  __attribute__ ((aligned(4)));	// Double buffer that DMA just used
	float downsampled_buffer[PIOS_ADC_NUM_CHANNELS]  __attribute__ ((aligned(4)));
#endif
	enum pios_adc_dev_magic magic;

#if defined(PIOS_INCLUDE_AD7998)
	// The I2C bus information if using AD7998
	uint32_t i2c_id;
	xTaskHandle TaskHandle;
#endif
	
	// The PPM channel values
	uint16_t ppm_channel[PIOS_ADC_RCVR_MAX_CHANNELS];
	uint8_t ppm_supv_timer;
	bool ppm_fresh;
};

int32_t PIOS_ADC_Init(const struct pios_adc_cfg * cfg);

#endif /* PIOS_ADC_PRIV_H */

/**
 * @}
 * @}
 */

