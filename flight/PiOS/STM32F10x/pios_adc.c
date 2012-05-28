/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_ADC ADC Functions
 * @brief STM32 ADC PIOS interface
 * @{
 *
 * @file       pios_adc.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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

// Private types

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
};

#if defined(PIOS_INCLUDE_FREERTOS)
struct pios_adc_dev * pios_adc_dev;
#endif

// Private functions
void PIOS_ADC_downsample_data();
static struct pios_adc_dev * PIOS_ADC_Allocate();
static bool PIOS_ADC_validate(struct pios_adc_dev *);

/* Local Variables */
static GPIO_TypeDef *ADC_GPIO_PORT[PIOS_ADC_NUM_PINS] = PIOS_ADC_PORTS;
static const uint32_t ADC_GPIO_PIN[PIOS_ADC_NUM_PINS] = PIOS_ADC_PINS;
static const uint32_t ADC_CHANNEL[PIOS_ADC_NUM_PINS] = PIOS_ADC_CHANNELS;

static ADC_TypeDef *ADC_MAPPING[PIOS_ADC_NUM_PINS] = PIOS_ADC_MAPPING;
static const uint32_t ADC_CHANNEL_MAPPING[PIOS_ADC_NUM_PINS] = PIOS_ADC_CHANNEL_MAPPING;

static bool PIOS_ADC_validate(struct pios_adc_dev * dev)
{
	if (dev == NULL)
		return false;

	return (dev->magic == PIOS_ADC_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_adc_dev * PIOS_ADC_Allocate()
{
	struct pios_adc_dev * adc_dev;
	
	adc_dev = (struct pios_adc_dev *)pvPortMalloc(sizeof(*adc_dev));
	if (!adc_dev) return (NULL);
	
	adc_dev->magic = PIOS_ADC_DEV_MAGIC;
	return(adc_dev);
}
#else
#error Not implemented
#endif

/**
 * @brief Initialise the ADC Peripheral, configure to run at the max oversampling
 */
int32_t PIOS_ADC_Init(const struct pios_adc_cfg * cfg)
{
	pios_adc_dev = PIOS_ADC_Allocate();
	if (pios_adc_dev == NULL)
		return -1;

	pios_adc_dev->cfg = cfg;
	pios_adc_dev->callback_function = NULL;
	
#if defined(PIOS_INCLUDE_FREERTOS)
	pios_adc_dev->data_queue = NULL;
#endif
	
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

	PIOS_ADC_Config(PIOS_ADC_MAX_OVERSAMPLING);	
	
	return 0;
}

/**
 * @brief Configure the ADC to run at a fixed oversampling
 * @param[in] oversampling the amount of oversampling to run at
 */
void PIOS_ADC_Config(uint32_t oversampling)
{	
	pios_adc_dev->adc_oversample = (oversampling > PIOS_ADC_MAX_OVERSAMPLING) ? PIOS_ADC_MAX_OVERSAMPLING : oversampling;

	ADC_DeInit(ADC1);
	ADC_DeInit(ADC2);
	
	/* Disable interrupts */
	DMA_ITConfig(pios_adc_dev->cfg->dma.rx.channel, pios_adc_dev->cfg->dma.irq.flags, DISABLE);
	
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
	// return	
	/* Configure ADCs */
	ADC_InitTypeDef ADC_InitStructure;
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = ((PIOS_ADC_NUM_CHANNELS + 1) >> 1);
	ADC_Init(ADC1, &ADC_InitStructure);
	
#if (PIOS_ADC_USE_ADC2)
	ADC_Init(ADC2, &ADC_InitStructure);
	
	/* Enable ADC2 external trigger conversion (to synch with ADC1) */
	ADC_ExternalTrigConvCmd(ADC2, ENABLE);
#endif
	
	RCC_ADCCLKConfig(PIOS_ADC_ADCCLK);
		
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
	
	/* This makes sure we have an even number of transfers if using ADC2 */
	pios_adc_dev->dma_block_size = ((PIOS_ADC_NUM_CHANNELS + PIOS_ADC_USE_ADC2) >> PIOS_ADC_USE_ADC2) << PIOS_ADC_USE_ADC2;
	pios_adc_dev->dma_half_buffer_size = pios_adc_dev->dma_block_size * pios_adc_dev->adc_oversample;

	/* Configure DMA channel */		
	DMA_InitTypeDef dma_init = pios_adc_dev->cfg->dma.rx.init;	
	dma_init.DMA_MemoryBaseAddr = (uint32_t) &pios_adc_dev->raw_data_buffer[0];
	dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_init.DMA_BufferSize = pios_adc_dev->dma_half_buffer_size; /* x2 for double buffer /2 for 32-bit xfr */
	DMA_Init(pios_adc_dev->cfg->dma.rx.channel, &dma_init);
	DMA_Cmd(pios_adc_dev->cfg->dma.rx.channel, ENABLE);
	
	/* Trigger interrupt when for half conversions too to indicate double buffer */
	DMA_ITConfig(pios_adc_dev->cfg->dma.rx.channel, DMA_IT_TC, ENABLE);
        DMA_ITConfig(pios_adc_dev->cfg->dma.rx.channel, DMA_IT_HT, ENABLE);
	
	/* Configure DMA interrupt */
	NVIC_Init(&pios_adc_dev->cfg->dma.irq.init);
	
	/* Finally start initial conversion */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
	/* Use simple averaging filter for now */
	for (int32_t i = 0; i < pios_adc_dev->adc_oversample; i++)
		pios_adc_dev->fir_coeffs[i] = 1;
	pios_adc_dev->fir_coeffs[pios_adc_dev->adc_oversample] = pios_adc_dev->adc_oversample;	
	
	/* Enable DMA1 clock */
	RCC_AHBPeriphClockCmd(pios_adc_dev->cfg->dma.ahb_clk, ENABLE);
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
	return pios_adc_dev->downsampled_buffer[pin];
}

/**
 * @brief Set a callback function that is executed whenever
 * the ADC double buffer swaps 
 */
void PIOS_ADC_SetCallback(ADCCallback new_function) 
{
	pios_adc_dev->callback_function = new_function;
}

#if defined(PIOS_INCLUDE_FREERTOS)
/**
 * @brief Register a queue to add data to when downsampled 
 */
void PIOS_ADC_SetQueue(xQueueHandle data_queue) 
{
	pios_adc_dev->data_queue = data_queue;
}
#endif

/**
 * @brief Return the address of the downsampled data buffer 
 */
float * PIOS_ADC_GetBuffer(void)
{
	return pios_adc_dev->downsampled_buffer;
}

/**
 * @brief Return the address of the raw data data buffer 
 */
int16_t * PIOS_ADC_GetRawBuffer(void)
{
	return (int16_t *) pios_adc_dev->valid_data_buffer;
}

/**
 * @brief Return the amount of over sampling
 */
uint8_t PIOS_ADC_GetOverSampling(void)
{
	return pios_adc_dev->adc_oversample;
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
	for(int i = 0; i <= pios_adc_dev->adc_oversample; i++) 
		pios_adc_dev->fir_coeffs[i] = new_filter[i];
}

/**
 * @brief Downsample the data for each of the channels then call
 * callback function if installed
 */ 
void PIOS_ADC_downsample_data()
{
	uint16_t chan;
	uint16_t sample;
	float * downsampled_buffer = &pios_adc_dev->downsampled_buffer[0];
	
	for (chan = 0; chan < PIOS_ADC_NUM_CHANNELS; chan++) {
		int32_t sum = 0;
		for (sample = 0; sample < pios_adc_dev->adc_oversample; sample++) {
			sum += pios_adc_dev->valid_data_buffer[chan + sample * pios_adc_dev->dma_block_size] * pios_adc_dev->fir_coeffs[sample];
		}
		downsampled_buffer[chan] = (float) sum / pios_adc_dev->fir_coeffs[pios_adc_dev->adc_oversample];
	}
	
#if defined(PIOS_INCLUDE_FREERTOS)
	if(pios_adc_dev->data_queue) {
		static portBASE_TYPE xHigherPriorityTaskWoken;
		xQueueSendFromISR(pios_adc_dev->data_queue, pios_adc_dev->downsampled_buffer, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);		
	}
#endif
	if(pios_adc_dev->callback_function)
		pios_adc_dev->callback_function(pios_adc_dev->downsampled_buffer);
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
	if (!PIOS_ADC_validate(pios_adc_dev))
		return;

	if (DMA_GetFlagStatus(pios_adc_dev->cfg->full_flag /*DMA1_IT_TC1*/)) {	// whole double buffer filled 
		pios_adc_dev->valid_data_buffer = &pios_adc_dev->raw_data_buffer[pios_adc_dev->dma_half_buffer_size];
		DMA_ClearFlag(pios_adc_dev->cfg->full_flag);
		PIOS_ADC_downsample_data();
	}
	else if (DMA_GetFlagStatus(pios_adc_dev->cfg->half_flag /*DMA1_IT_HT1*/)) {
		pios_adc_dev->valid_data_buffer = &pios_adc_dev->raw_data_buffer[0];
		DMA_ClearFlag(pios_adc_dev->cfg->half_flag);
		PIOS_ADC_downsample_data();
	}
	else {
		// This should not happen, probably due to transfer errors
		DMA_ClearFlag(pios_adc_dev->cfg->dma.irq.flags /*DMA1_FLAG_GL1*/);
	}
}

/** 
 * @}
 * @}
 */
