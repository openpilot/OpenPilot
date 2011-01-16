/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_ADC ADC Functions
 * @brief STM32 ADC PIOS interface
 * @{
 *
 * @file       pios_adc.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      Analog to Digital converstion routines
 * @see        The GNU Public License (GPL) Version 3
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

#include "pios.h"
#include <pios_adc_priv.h>

// Private functions
void PIOS_ADC_downsample_data();

/* Local Variables */
static GPIO_TypeDef *ADC_GPIO_PORT[PIOS_ADC_NUM_PINS] = PIOS_ADC_PORTS;
static const uint32_t ADC_GPIO_PIN[PIOS_ADC_NUM_PINS] = PIOS_ADC_PINS;
static const uint32_t ADC_CHANNEL[PIOS_ADC_NUM_PINS] = PIOS_ADC_CHANNELS;

static ADC_TypeDef *ADC_MAPPING[PIOS_ADC_NUM_PINS] = PIOS_ADC_MAPPING;
static const uint32_t ADC_CHANNEL_MAPPING[PIOS_ADC_NUM_PINS] = PIOS_ADC_CHANNEL_MAPPING;

/**
 * @brief Initialise the ADC Peripheral
 * @param[in] adc_oversample
 * @return 
 *  @arg 1 for success
 *  @arg 0 for failure
 * Currently ignores rates and uses hardcoded values.  Need a little logic to
 * map from sampling rates and such to ADC constants.
 */
void PIOS_ADC_Init()
{	
	pios_adc_devs[0].callback_function = NULL;
	
	ADC_DeInit(ADC1);
	ADC_DeInit(ADC2);
	
	/* Setup analog pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	
	/* Enable each ADC pin in the array */
	for (int32_t i = 0; i < PIOS_ADC_NUM_PINS; i++) {
		GPIO_InitStructure.GPIO_Pin = ADC_GPIO_PIN[i];
		GPIO_Init(ADC_GPIO_PORT[i], &GPIO_InitStructure);
	}
	
	/* Enable ADC clocks */
	PIOS_ADC_CLOCK_FUNCTION;
	
	/* Map channels to conversion slots depending on the channel selection mask */
	for (int32_t i = 0; i < PIOS_ADC_NUM_PINS; i++) {
		ADC_RegularChannelConfig(ADC_MAPPING[i], ADC_CHANNEL[i],
					 ADC_CHANNEL_MAPPING[i],
					 PIOS_ADC_SAMPLE_TIME);
	}
	
#if (PIOS_ADC_USE_TEMP_SENSOR)
	ADC_TempSensorVrefintCmd(ENABLE);
	ADC_RegularChannelConfig(PIOS_ADC_TEMP_SENSOR_ADC, ADC_Channel_16,
				 PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL,
				 PIOS_ADC_SAMPLE_TIME);
#endif
	
	// TODO: update ADC to continuous sampling, configure the sampling rate
	/* Configure ADCs */
	ADC_InitTypeDef ADC_InitStructure;
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel =
	((PIOS_ADC_NUM_CHANNELS + 1) >> 1);
	ADC_Init(ADC1, &ADC_InitStructure);
	
#if (PIOS_ADC_USE_ADC2)
	ADC_Init(ADC2, &ADC_InitStructure);
	
	/* Enable ADC2 external trigger conversion (to synch with ADC1) */
	ADC_ExternalTrigConvCmd(ADC2, ENABLE);
#endif
	
	RCC_ADCCLKConfig(PIOS_ADC_ADCCLK);
	RCC_PCLK2Config(RCC_HCLK_Div16);
	
	/* Enable ADC1->DMA request */
	ADC_DMACmd(ADC1, ENABLE);
	
	/* ADC1 calibration */
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1)) ;
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1)) ;
	
#if (PIOS_ADC_USE_ADC2)
	/* ADC2 calibration */
	ADC_Cmd(ADC2, ENABLE);
	ADC_ResetCalibration(ADC2);
	while (ADC_GetResetCalibrationStatus(ADC2)) ;
	ADC_StartCalibration(ADC2);
	while (ADC_GetCalibrationStatus(ADC2)) ;
#endif
	
	PIOS_ADC_Config(1);

	/* Enable DMA1 clock */
	RCC_AHBPeriphClockCmd(pios_adc_devs[0].cfg->dma.ahb_clk, ENABLE);
	
}

void PIOS_ADC_Config(uint32_t oversampling)
{
	pios_adc_devs[0].adc_oversample = oversampling;

	/* Disable interrupts */
	DMA_ITConfig(pios_adc_devs[0].cfg->dma.rx.channel, pios_adc_devs[0].cfg->dma.irq.flags, DISABLE);

	/* Configure DMA channel */		
	DMA_InitTypeDef dma_init = pios_adc_devs[0].cfg->dma.rx.init;	
	dma_init.DMA_MemoryBaseAddr = (uint32_t) &pios_adc_devs[0].raw_data_buffer[0];
	dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_init.DMA_BufferSize = (PIOS_ADC_NUM_CHANNELS * pios_adc_devs[0].adc_oversample * 2) >> 1;
	DMA_Init(pios_adc_devs[0].cfg->dma.rx.channel, &dma_init);
	DMA_Cmd(pios_adc_devs[0].cfg->dma.rx.channel, ENABLE);
	
	/* Trigger interrupt when for half conversions too to indicate double buffer */
	DMA_ITConfig(pios_adc_devs[0].cfg->dma.rx.channel, DMA_IT_TC, ENABLE);
        DMA_ITConfig(pios_adc_devs[0].cfg->dma.rx.channel, DMA_IT_HT, ENABLE);
	
	/* Configure DMA interrupt */
	NVIC_Init(&pios_adc_devs[0].cfg->dma.irq.init);
	
	/* Finally start initial conversion */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
	/* Use simple averaging filter for now */
	for (int32_t i = 0; i < pios_adc_devs[0].adc_oversample; i++)
		pios_adc_devs[0].fir_coeffs[i] = 1;
	pios_adc_devs[0].fir_coeffs[pios_adc_devs[0].adc_oversample] = pios_adc_devs[0].adc_oversample;	
}

/**
 * Returns value of an ADC Pin
 * \param[in] pin number
 * \return ADC pin value - resolution depends on the selected oversampling rate
 * \return -1 if pin doesn't exist
 */
int32_t PIOS_ADC_PinGet(uint32_t pin)
{
	/* Check if pin exists */
	if (pin >= PIOS_ADC_NUM_CHANNELS) {
		return -1;
	}
	
	/* Return last conversion result */
	return pios_adc_devs[0].downsampled_buffer[pin];
}

/**
 * @brief Set a callback function that is executed whenever
 * the ADC double buffer swaps 
 */
void PIOS_ADC_SetCallback(ADCCallback new_function) 
{
	pios_adc_devs[0].callback_function = new_function;
}

/**
 * @brief Return the address of the downsampled data buffer 
 */
float * PIOS_ADC_GetBuffer(void)
{
	return pios_adc_devs[0].downsampled_buffer;
}

/**
 * @brief Return the address of the raw data data buffer 
 */
int16_t * PIOS_ADC_GetRawBuffer(void)
{
	return (int16_t *) pios_adc_devs[0].valid_data_buffer;
}

/**
 * @brief Return the amount of over sampling
 */
uint8_t PIOS_ADC_GetOverSampling(void)
{
	return pios_adc_devs[0].adc_oversample;
}

/**
 * @brief Set the fir coefficients.  Takes as many samples as the 
 * current filter order plus one (normalization)
 *
 * @param new_filter Array of adc_oversampling floats plus one for the
 * filter coefficients
 */
void PIOS_ADC_SetFIRCoefficients(float * new_filter)
{
	// Less than or equal to get normalization constant
	for(int i = 0; i <= pios_adc_devs[0].adc_oversample; i++) 
		pios_adc_devs[0].fir_coeffs[i] = new_filter[i];
}

/**
 * @brief Downsample the data for each of the channels then call
 * callback function if installed
 */ 
void PIOS_ADC_downsample_data()
{
	uint16_t chan;
	uint16_t sample;
	float * downsampled_buffer = &pios_adc_devs[0].downsampled_buffer[0];
	
	for (chan = 0; chan < PIOS_ADC_NUM_CHANNELS; chan++) {
		int32_t sum = 0;
		for (sample = 0; sample < pios_adc_devs[0].adc_oversample; sample++) {
			sum += pios_adc_devs[0].valid_data_buffer[chan + sample * PIOS_ADC_NUM_CHANNELS] * pios_adc_devs[0].fir_coeffs[sample];
		}
		downsampled_buffer[chan] = (float) sum / pios_adc_devs[0].fir_coeffs[pios_adc_devs[0].adc_oversample];
	}
	
	if(pios_adc_devs[0].callback_function)
		pios_adc_devs[0].callback_function(pios_adc_devs[0].downsampled_buffer);
}

/**
 * @brief Interrupt for half and full buffer transfer
 * 
 * This interrupt handler swaps between the two halfs of the double buffer to make
 * sure the ahrs uses the most recent data.  Only swaps data when AHRS is idle, but
 * really this is a pretense of a sanity check since the DMA engine is consantly 
 * running in the background.  Keep an eye on the ekf_too_slow variable to make sure
 * it's keeping up.
 */
void PIOS_ADC_DMA_Handler(void)
{
	if (DMA_GetFlagStatus(pios_adc_devs[0].cfg->full_flag /*DMA1_IT_TC1*/)) {	// whole double buffer filled 
		pios_adc_devs[0].valid_data_buffer = &pios_adc_devs[0].raw_data_buffer[1 * PIOS_ADC_NUM_CHANNELS * pios_adc_devs[0].adc_oversample];
		DMA_ClearFlag(pios_adc_devs[0].cfg->full_flag);
		PIOS_ADC_downsample_data();
	}
	else if (DMA_GetFlagStatus(pios_adc_devs[0].cfg->half_flag /*DMA1_IT_HT1*/)) {
		pios_adc_devs[0].valid_data_buffer = &pios_adc_devs[0].raw_data_buffer[0 * PIOS_ADC_NUM_CHANNELS * pios_adc_devs[0].adc_oversample];
		DMA_ClearFlag(pios_adc_devs[0].cfg->half_flag);
		PIOS_ADC_downsample_data();
	}
	else {
		// This should not happen, probably due to transfer errors
		DMA_ClearFlag(pios_adc_devs[0].cfg->dma.irq.flags /*DMA1_FLAG_GL1*/);
	}
}

/** 
 * @}
 * @}
 */
