/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_ADC ADC Functions
 * @brief STM32F4xx ADC PIOS interface
 * @{
 *
 * @file       pios_adc.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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

#ifdef PIOS_INCLUDE_ADC

#include <pios_adc_priv.h>


#if !defined(PIOS_ADC_MAX_SAMPLES)
#define PIOS_ADC_MAX_SAMPLES 0
#endif

#if !defined(PIOS_ADC_MAX_OVERSAMPLING)
#define PIOS_ADC_MAX_OVERSAMPLING 0
#endif

#if !defined(PIOS_ADC_USE_ADC2)
#define PIOS_ADC_USE_ADC2 0
#endif

#if !defined(PIOS_ADC_NUM_CHANNELS)
#define PIOS_ADC_NUM_CHANNELS 0
#endif

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
//	int16_t fir_coeffs[PIOS_ADC_MAX_SAMPLES+1]  __attribute__ ((aligned(4)));
//	volatile int16_t raw_data_buffer[PIOS_ADC_MAX_SAMPLES]  __attribute__ ((aligned(4)));
//	float downsampled_buffer[PIOS_ADC_NUM_CHANNELS]  __attribute__ ((aligned(4)));
	enum pios_adc_dev_magic magic;
};

struct pios_adc_dev * pios_adc_dev;

// Private functions
void PIOS_ADC_downsample_data();
static struct pios_adc_dev * PIOS_ADC_Allocate();
static bool PIOS_ADC_validate(struct pios_adc_dev *);

#if defined(PIOS_INCLUDE_ADC)
static void init_pins(void);
static void init_dma(void);
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
static const struct dma_config config[] = PIOS_DMA_PIN_CONFIG;
#define PIOS_ADC_NUM_PINS	(sizeof(config) / sizeof(config[0]))

static struct adc_accumulator accumulator[PIOS_ADC_NUM_PINS];

// Two buffers here for double buffering
static uint16_t adc_raw_buffer[2][PIOS_ADC_MAX_SAMPLES][PIOS_ADC_NUM_PINS];
#endif

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
		if (config[i].port == NULL)
			continue;
		GPIO_InitStructure.GPIO_Pin = config[i].pin;
		GPIO_Init(config[i].port, &GPIO_InitStructure);
	}
}

static void
init_dma(void)
{
	/* Disable interrupts */
	DMA_ITConfig(pios_adc_dev->cfg->dma.rx.channel, pios_adc_dev->cfg->dma.irq.flags, DISABLE);

	/* Configure DMA channel */
	DMA_DeInit(pios_adc_dev->cfg->dma.rx.channel);
	DMA_InitTypeDef DMAInit = pios_adc_dev->cfg->dma.rx.init;
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

	DMA_Init(pios_adc_dev->cfg->dma.rx.channel, &DMAInit);	/* channel is actually stream ... */

	/* configure for double-buffered mode and interrupt on every buffer flip */
	DMA_DoubleBufferModeConfig(pios_adc_dev->cfg->dma.rx.channel, (uint32_t)&adc_raw_buffer[1], DMA_Memory_0);
	DMA_DoubleBufferModeCmd(pios_adc_dev->cfg->dma.rx.channel, ENABLE);
	DMA_ITConfig(pios_adc_dev->cfg->dma.rx.channel, DMA_IT_TC, ENABLE);
	//DMA_ITConfig(pios_adc_dev->cfg->dma.rx.channel, DMA_IT_HT, ENABLE);

	/* enable DMA */
	DMA_Cmd(pios_adc_dev->cfg->dma.rx.channel, ENABLE);

	/* Configure DMA interrupt */
	NVIC_InitTypeDef NVICInit = pios_adc_dev->cfg->dma.irq.init;
	NVIC_Init(&NVICInit);
}

static void
init_adc(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	ADC_DeInit();

	/* turn on VREFInt in case we need it */
	ADC_TempSensorVrefintCmd(ENABLE);

	/* Do common ADC init */
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_CommonStructInit(&ADC_CommonInitStructure);
	ADC_CommonInitStructure.ADC_Mode				= ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler			= ADC_Prescaler_Div8;
	ADC_CommonInitStructure.ADC_DMAAccessMode		= ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay	= ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitTypeDef ADC_InitStructure;
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Resolution				= ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode				= ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode		= ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge		= ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign					= ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion			= ((PIOS_ADC_NUM_PINS)/* >> 1*/);
	ADC_Init(pios_adc_dev->cfg->adc_dev, &ADC_InitStructure);

	/* Enable DMA request */
	ADC_DMACmd(pios_adc_dev->cfg->adc_dev, ENABLE);

	/* Configure input scan */
	for (int32_t i = 0; i < PIOS_ADC_NUM_PINS; i++) {
		ADC_RegularChannelConfig(pios_adc_dev->cfg->adc_dev,
				config[i].channel,
				i+1,
				ADC_SampleTime_56Cycles);		/* XXX this is totally arbitrary... */
	}

	ADC_DMARequestAfterLastTransferCmd(pios_adc_dev->cfg->adc_dev, ENABLE);

	/* Finally start initial conversion */
	ADC_Cmd(pios_adc_dev->cfg->adc_dev, ENABLE);
	ADC_ContinuousModeCmd(pios_adc_dev->cfg->adc_dev, ENABLE);
	ADC_SoftwareStartConv(pios_adc_dev->cfg->adc_dev);
}
#endif

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
#if defined(PIOS_INCLUDE_ADC)
#error Not implemented
#endif
static struct pios_adc_dev * PIOS_ADC_Allocate()
{
	return (struct pios_adc_dev *) NULL;
}
#endif

/**
 * @brief Init the ADC.
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

#if defined(PIOS_INCLUDE_ADC)
	init_pins();
	init_dma();
	init_adc();
#endif

	return 0;
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
 * @return -2 if no data acquired since last read
 */
int32_t last_conv_value;
int32_t PIOS_ADC_PinGet(uint32_t pin)
{
#if defined(PIOS_INCLUDE_ADC)
	int32_t	result;
	
	/* Check if pin exists */
	if (pin >= PIOS_ADC_NUM_PINS) {
		return -1;
	}
	
	if (accumulator[pin].accumulator <= 0)
		return -2;

	/* return accumulated result and clear accumulator */
	result = accumulator[pin].accumulator / (accumulator[pin].count ?: 1);
	accumulator[pin].accumulator = result;
	accumulator[pin].count = 1;

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
	pios_adc_dev->callback_function = new_function;
}

#if defined(PIOS_INCLUDE_FREERTOS)
/**
 * @brief Register a queue to add data to when downsampled 
 * @note Not currently supported.
 */
void PIOS_ADC_SetQueue(xQueueHandle data_queue) 
{
	pios_adc_dev->data_queue = data_queue;
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
	if (pios_adc_dev->data_queue) {
		static portBASE_TYPE xHigherPriorityTaskWoken;
//		xQueueSendFromISR(pios_adc_dev->data_queue, pios_adc_dev->downsampled_buffer, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);		
	}

#endif
#endif

//	if(pios_adc_dev->callback_function)
//		pios_adc_dev->callback_function(pios_adc_dev->downsampled_buffer);

}

/**
 * @brief Interrupt on buffer flip.
 *
 * The hardware is done with the 'other' buffer, so we can pass it to the accumulator.
 */
void PIOS_ADC_DMA_Handler(void)
{
	if (!PIOS_ADC_validate(pios_adc_dev))
		return;

#if defined(PIOS_INCLUDE_ADC)
	/* terminal count, buffer has flipped */
	if (DMA_GetITStatus(pios_adc_dev->cfg->dma.rx.channel, pios_adc_dev->cfg->full_flag)) {
		DMA_ClearITPendingBit(pios_adc_dev->cfg->dma.rx.channel, pios_adc_dev->cfg->full_flag);

		/* accumulate results from the buffer that was just completed */
		accumulate(&adc_raw_buffer[DMA_GetCurrentMemoryTarget(pios_adc_dev->cfg->dma.rx.channel) ? 0 : 1][0][0],
				PIOS_ADC_MAX_SAMPLES);

	}
#endif
}

#endif /* PIOS_INCLUDE_ADC */

/** 
 * @}
 * @}
 */
