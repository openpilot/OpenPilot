/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_ADC ADC Functions
 * @brief STM32F4xx ADC PIOS interface
 * @{
 *
 * @file       pios_adc.c  
 * @author     Michael Smith Copyright (C) 2011.
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

/*
 * @note This is a stripped-down ADC driver intended primarily for sampling
 * voltage and current values.  Samples are averaged over the period between
 * fetches so that relatively accurate measurements can be obtained without
 * forcing higher-level logic to poll aggressively.
 *
 * @todo This module needs more work to be more generally useful.  It should
 * almost certainly grow callback support so that e.g. voltage and current readings
 * can be shipped out for coulomb counting purposes.  The F1xx interface presumes
 * use with analog sensors, but that implementation largely dominates the ADC
 * resources.  Rather than commit to a new API without a defined use case, we
 * should stick to our lightweight subset until we have a better idea of what's needed.
 */

#include "pios.h"
#include <pios_adc_priv.h>

extern struct pios_adc_cfg pios_adc_cfg;

#if defined(PIOS_INCLUDE_ADC)
static void init_pins(void);
static void init_dma(void);
static void init_timer(void);
static void init_adc(void);
#endif

struct dma_config {
	GPIO_TypeDef	*port;
	uint32_t		pin;
	uint32_t		channel;
};

struct adc_accumulator {
	uint32_t		accumulator;
	uint32_t		count;
};

#if defined(PIOS_INCLUDE_ADC)
static struct dma_config config[] = PIOS_DMA_PIN_CONFIG;
#define PIOS_ADC_NUM_PINS	(sizeof(config) / sizeof(config[0]))

static struct adc_accumulator accumulator[PIOS_ADC_NUM_PINS];

static uint16_t adc_raw_buffer[2][PIOS_ADC_MAX_SAMPLES][PIOS_ADC_NUM_PINS];
#endif

#define PIOS_ADC_TIMER		TIM3		/* might want this to come from the config */
#define PIOS_LOWRATE_ADC	ADC1

#if defined(PIOS_INCLUDE_ADC)
static void
init_pins(void)
{
	/* Setup analog pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AIN;
	
	for (int32_t i = 0; i < PIOS_ADC_NUM_PINS; i++) {
		GPIO_InitStructure.GPIO_Pin = config[i].pin;
		GPIO_Init(config[i].port, &GPIO_InitStructure);
	}
}

static void
init_dma(void)
{
	/* Disable interrupts */
	DMA_ITConfig(pios_adc_cfg.dma.rx.channel, pios_adc_cfg.dma.irq.flags, DISABLE);

	/* Configure DMA channel */
	DMA_DeInit(pios_adc_cfg.dma.rx.channel);
	DMA_InitTypeDef DMAInit = pios_adc_cfg.dma.rx.init;
	DMAInit.DMA_Memory0BaseAddr		= (uint32_t)&adc_raw_buffer[0];
	DMAInit.DMA_BufferSize			= sizeof(adc_raw_buffer[0]) / sizeof(uint16_t);
	DMAInit.DMA_DIR					= DMA_DIR_PeripheralToMemory;
	DMAInit.DMA_PeripheralInc		= DMA_PeripheralInc_Disable;
	DMAInit.DMA_MemoryInc			= DMA_MemoryInc_Enable;
	DMAInit.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_HalfWord;
	DMAInit.DMA_MemoryDataSize		= DMA_MemoryDataSize_HalfWord;
	DMAInit.DMA_Mode				= DMA_Mode_Circular;
	DMAInit.DMA_Priority			= DMA_Priority_Low;
	DMAInit.DMA_FIFOMode			= DMA_FIFOMode_Disable;
	DMAInit.DMA_FIFOThreshold		= DMA_FIFOThreshold_HalfFull;
	DMAInit.DMA_MemoryBurst			= DMA_MemoryBurst_Single;
	DMAInit.DMA_PeripheralBurst		= DMA_PeripheralBurst_Single;

	DMA_Init(pios_adc_cfg.dma.rx.channel, &DMAInit);	/* channel is actually stream ... */

	/* configure for double-buffered mode and interrupt on every buffer flip */
	DMA_DoubleBufferModeConfig(pios_adc_cfg.dma.rx.channel, (uint32_t)&adc_raw_buffer[1], DMA_Memory_0);
	DMA_DoubleBufferModeCmd(pios_adc_cfg.dma.rx.channel, ENABLE);
	DMA_ITConfig(pios_adc_cfg.dma.rx.channel, DMA_IT_TC, ENABLE);

	/* enable DMA */
	DMA_Cmd(pios_adc_cfg.dma.rx.channel, ENABLE);

	/* Configure DMA interrupt */
	NVIC_InitTypeDef NVICInit = pios_adc_cfg.dma.irq.init;
	NVICInit.NVIC_IRQChannelPreemptionPriority	= PIOS_IRQ_PRIO_LOW;
	NVICInit.NVIC_IRQChannelSubPriority			= 0;
	NVICInit.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init(&NVICInit);
}

static void
init_timer(void)
{
	RCC_ClocksTypeDef	clocks;
	TIM_TimeBaseInitTypeDef TIMInit;

	/* get clock info */
	RCC_GetClocksFreq(&clocks);

	/* reset/disable the timer */
	TIM_DeInit(PIOS_ADC_TIMER);

	/* configure for 1kHz auto-reload cycle */
	TIM_TimeBaseStructInit(&TIMInit);
	TIMInit.TIM_Prescaler							= clocks.PCLK1_Frequency / 1000000;	/* 1MHz base clock*/
	TIMInit.TIM_CounterMode							= TIM_CounterMode_Down;
	TIMInit.TIM_Period								= 1000;							/* 1kHz conversion rate */
	TIMInit.TIM_ClockDivision						= TIM_CKD_DIV1;					/* no additional divisor */
	TIM_TimeBaseInit(PIOS_ADC_TIMER, &TIMInit);

	PIOS_COM_SendFormattedString(PIOS_COM_DEBUG, "TIM_Prescaler %d\r\n",TIMInit.TIM_Prescaler);

	/* configure trigger output on reload */
	TIM_SelectOutputTrigger(PIOS_ADC_TIMER, TIM_TRGOSource_Update);
	TIM_Cmd(PIOS_ADC_TIMER, ENABLE);
}

static void
init_adc(void)
{
	ADC_DeInit();

	/* turn on VREFInt in case we need it */
	ADC_TempSensorVrefintCmd(ENABLE);

	/* Do common ADC init */
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_CommonStructInit(&ADC_CommonInitStructure);
	ADC_CommonInitStructure.ADC_Mode				= ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler			= ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode		= ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay	= ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitTypeDef ADC_InitStructure;
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Resolution				= ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode				= ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode		= DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv			= ADC_ExternalTrigConv_T3_TRGO;
	ADC_InitStructure.ADC_ExternalTrigConvEdge		= ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_DataAlign					= ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion			= ((PIOS_ADC_NUM_PINS)/* >> 1*/);
	ADC_Init(PIOS_LOWRATE_ADC, &ADC_InitStructure);

	/* Enable PIOS_LOWRATE_ADC->DMA request */
		ADC_DMACmd(PIOS_LOWRATE_ADC, ENABLE);

	/* Configure input scan */
	for (int32_t i = 0; i < PIOS_ADC_NUM_PINS; i++) {
		ADC_RegularChannelConfig(PIOS_LOWRATE_ADC,
				config[i].channel,
				i+1,
				ADC_SampleTime_56Cycles);		/* XXX this is totally arbitrary... */
	}

	ADC_DMARequestAfterLastTransferCmd(PIOS_LOWRATE_ADC, ENABLE);

	/* Finally start initial conversion */
	ADC_Cmd(PIOS_LOWRATE_ADC, ENABLE);
}
#endif

/**
 * @brief Init the ADC.
 */
void PIOS_ADC_Init()
{
#if defined(PIOS_INCLUDE_ADC)
	init_pins();
	init_dma();
	init_timer();
	init_adc();
#endif
}

/**
 * @brief Configure the ADC to run at a fixed oversampling
 * @param[in] oversampling the amount of oversampling to run at
 */
void PIOS_ADC_Config(uint32_t oversampling)
{
	/* we ignore this */
}

/**
 * Returns value of an ADC Pin
 * @param[in] pin number
 * @return ADC pin value averaged over the set of samples since the last reading.
 * @return -1 if pin doesn't exist
 */
int32_t PIOS_ADC_PinGet(uint32_t pin)
{
#if defined(PIOS_INCLUDE_ADC)
	int32_t	result;

	/* Check if pin exists */
	if (pin >= PIOS_ADC_NUM_PINS) {
		return -1;
	}

	/* return accumulated result and clear accumulator */
	result = accumulator[pin].accumulator / (accumulator[pin].count ?: 1);
	accumulator[pin].accumulator = 0;
	accumulator[pin].count = 0;

	return result;
#endif
	return -1;
}

/**
 * @brief Set a callback function that is executed whenever
 * the ADC double buffer swaps 
 * @note Not currently supported.
 */
void PIOS_ADC_SetCallback(ADCCallback new_function) 
{
	// XXX might be nice to do something here
}

#if defined(PIOS_INCLUDE_FREERTOS)
/**
 * @brief Register a queue to add data to when downsampled 
 * @note Not currently supported.
 */
void PIOS_ADC_SetQueue(xQueueHandle data_queue) 
{
	// XXX it might make sense? to do this
}
#endif

/**
 * @brief Return the address of the downsampled data buffer
 * @note Not currently supported.
 */
float * PIOS_ADC_GetBuffer(void)
{
	return NULL;
}

/**
 * @brief Return the address of the raw data data buffer 
 * @note Not currently supported.
 */
int16_t * PIOS_ADC_GetRawBuffer(void)
{
	return NULL;
}

/**
 * @brief Return the amount of over sampling
 * @note Not currently supported (always returns 1)
 */
uint8_t PIOS_ADC_GetOverSampling(void)
{
	return 1;
}

/**
 * @brief Set the fir coefficients.  Takes as many samples as the 
 * current filter order plus one (normalization)
 *
 * @param new_filter Array of adc_oversampling floats plus one for the
 * filter coefficients
 * @note Not currently supported.
 */
void PIOS_ADC_SetFIRCoefficients(float * new_filter)
{
	// not implemented
}

/**
 * @brief accumulate the data for each of the channels.
 */
void accumulate(uint16_t *buffer, uint32_t count)
{
#if defined(PIOS_INCLUDE_ADC)
	uint16_t	*sp = buffer;

	/*
	 * Accumulate sampled values.
	 */
	while (count--) {
		for (int i = 0; i < PIOS_ADC_NUM_PINS; i++) {
			accumulator[i].accumulator += *sp++;
			accumulator[i].count++;
			/*
			 * If the accumulator reaches half-full, rescale in order to
			 * make more space.
			 */
			if (accumulator[i].accumulator >= (1 << 31)) {
				accumulator[i].accumulator /= 2;
				accumulator[i].count /= 2;
			}
		}
	}
	
#if defined(PIOS_INCLUDE_FREERTOS)
	// XXX should do something with this
#if 0
	if (pios_adc_devs[0].data_queue) {
		static portBASE_TYPE xHigherPriorityTaskWoken;
		xQueueSendFromISR(pios_adc_devs[0].data_queue, pios_adc_devs[0].downsampled_buffer, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);		
	}
#endif
#endif
#endif
	// XXX callback?
}

/**
 * @brief Interrupt on buffer flip.
 *
 * The hardware is done with the 'other' buffer, so we can pass it to the accumulator.
 */
void PIOS_ADC_DMA_Handler(void)
{
#if defined(PIOS_INCLUDE_ADC)
	/* terminal count, buffer has flipped */
	if (DMA_GetITStatus(pios_adc_cfg.dma.rx.channel, pios_adc_cfg.full_flag)) {
		DMA_ClearITPendingBit(pios_adc_cfg.dma.rx.channel, pios_adc_cfg.full_flag);

		/* accumulate results from the buffer that was just completed */
		accumulate(&adc_raw_buffer[DMA_GetCurrentMemoryTarget(pios_adc_cfg.dma.rx.channel) ? 0 : 1][0][0],
				PIOS_ADC_MAX_SAMPLES);

//		static uint8_t outputcounter = 0;
//		if (outputcounter == 0)
//			{
//			PIOS_COM_SendFormattedString(PIOS_COM_DEBUG, "adc vals %d %d %d %d %d %d\r\n", adc_raw_buffer[0][0][0], adc_raw_buffer[0][0][1], adc_raw_buffer[0][0][2], adc_raw_buffer[0][0][3], adc_raw_buffer[0][0][4], adc_raw_buffer[0][0][5]);
//			}
//		outputcounter++;
	}
#endif
}

/** 
 * @}
 * @}
 */
