/**
 ******************************************************************************
 *
 * @file       pios_adc.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      Analog to Digital converstion routines
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_ADC ADC Functions
 * @{
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


/* Project Includes */
#include "pios.h"

#if !defined(PIOS_DONT_USE_ADC)

/* Local Variables */
/* following two arrays are word aligned, so that DMA can transfer two hwords at once */
static uint16_t adc_conversion_values[NUM_ADC_PINS] __attribute__((aligned(4)));
static uint16_t adc_conversion_values_sum[NUM_ADC_PINS] __attribute__((aligned(4)));

static uint16_t adc_pin_values[NUM_ADC_PINS];
static uint32_t adc_pin_changed[NUM_ADC_PINS];


/**
* Initialise the ADC Peripheral
*/
void PIOS_ADC_Init(void)
{
	int32_t i;

	/* Clear arrays and variables */
	for(i=0; i < NUM_ADC_PINS; ++i) {
		adc_conversion_values[i] = 0;
		adc_conversion_values_sum[i] = 0;
	}
	for(i=0; i < NUM_ADC_PINS; ++i) {
		adc_pin_values[i] = 0;
	}
	for(i=0; i < NUM_ADC_PINS; ++i) {
		adc_pin_changed[i] = 1;
	}

	/* Setup analog pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;

	GPIO_InitStructure.GPIO_Pin = ADC_Z_PIN | ADC_A_PIN | ADC_B_PIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Enable ADC1/2 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);

	/* Map channels to conversion slots depending on the channel selection mask */
	/* Distribute this over the three ADCs, so that channels can be converted in parallel */
	/* Sample time: */
	/* With an ADCCLK = 14 MHz and a sampling time of 293.5 cycles: */
	/* Tconv = 239.5 + 12.5 = 252 cycles = 18µs */
	/* To be pedantic, we take A and B simultaneously, and Z and Temp simultaneously */
	ADC_RegularChannelConfig(ADC1, ADC_A_CHANNEL, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_B_CHANNEL, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_Z_CHANNEL, 2, ADC_SampleTime_239Cycles5);
	

	/* Configure ADCs */
	ADC_InitTypeDef ADC_InitStructure;
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 2;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Init(ADC2, &ADC_InitStructure);

	/* Enable ADC2 external trigger conversion (to synch with ADC1) */
	ADC_ExternalTrigConvCmd(ADC2, ENABLE);

	/* Enable ADC1->DMA request */
	ADC_DMACmd(ADC1, ENABLE);

	/* ADC1 calibration */
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));

	/* ADC2 calibration */
	ADC_Cmd(ADC2, ENABLE);
	ADC_ResetCalibration(ADC2);
	while(ADC_GetResetCalibrationStatus(ADC2));
	ADC_StartCalibration(ADC2);
	while(ADC_GetCalibrationStatus(ADC2));

	/* Enable DMA1 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* Configure DMA1 channel 1 to fetch data from ADC result register */
	DMA_InitTypeDef DMA_InitStructure;
	DMA_StructInit(&DMA_InitStructure);
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_conversion_values;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 2; /* Number of conversions depends on number of used channels */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);

	/* Trigger interrupt when all conversion values have been fetched */
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

	/* Configure and enable DMA interrupt */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ADC_IRQ_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Finally start initial conversion */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
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
	if(pin >= NUM_ADC_PINS) {
		return -1;
	}

	/* Return last conversion result */
	return adc_pin_values[pin];
}


/**
* Checks for pin changes, and calls given callback function with following parameters on pin changes:
* \code
*   void ADCNotifyChanged(uint32_t pin, uint16_t value)
* \endcode
* \param[in] _callback pointer to callback function
* \return < 0 on errors
*/
int32_t PIOS_ADC_Handler(void *_callback)
{
	/* No callback function? */
	if(_callback == NULL) {
		return -1;
	}

	int pin;
	void (*callback)(int32_t pin, uint32_t value) = _callback;

	/* Check for changed ADC conversion values */
	for(pin = 0; pin < NUM_ADC_PINS; pin++) {		
		PIOS_IRQ_Disable();
		uint32_t pin_value = adc_pin_values[pin];
		PIOS_IRQ_Enable();
		
		/* Call application hook */
		/* Note that due to dual conversion approach, we have to convert the pin number */
		/* If an uneven number of channels selected */
		if(adc_pin_changed[pin] == 1) {
			callback(pin, pin_value);
		}
	}

	/* Start next scan */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	/* No error */
	return 0;
}


/**
* DMA channel interrupt is triggered when all ADC channels have been converted
* \note shouldn't be called directly from application
*/
void DMA1_Channel1_IRQHandler(void)
{
	int32_t i;
	uint16_t *src_ptr, *dst_ptr, *changed_ptr;

	/* Clear the pending flag(s) */
	DMA_ClearFlag(DMA1_FLAG_TC1 | DMA1_FLAG_TE1 | DMA1_FLAG_HT1 | DMA1_FLAG_GL1);

	/* Copy conversion values to adc_pin_values */
//	src_ptr = (uint16_t *)adc_conversion_values;
//	dst_ptr = (uint16_t *)&adc_pin_values[NUM_ADC_PINS];
//	changed_ptr = (uint16_t *)&adc_pin_changed[NUM_ADC_PINS];
//
//	for(i = 0; i < NUM_ADC_PINS; ++i) {
//		/* Takeover new value */
//		if(*dst_ptr != *src_ptr) {
//			*dst_ptr = *src_ptr;
//			*changed_ptr = 1;
//		} else {
//			*changed_ptr = 0;
//		}
//
//		/* Switch to next results */
//		++dst_ptr;
//		++src_ptr;
//		++changed_ptr;
//	}
	
	src_ptr = (uint16_t *)adc_conversion_values;

	for(i = 0; i < NUM_ADC_PINS; ++i) {
		/* Takeover new value */
		if(adc_pin_values[i] != *src_ptr) {
			adc_pin_values[i] = *src_ptr;
			adc_pin_changed[i] = 1;
		} else {
			adc_pin_changed[i] = 0;
		}

		++src_ptr;
	}

	/* Request next conversion */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

#endif
