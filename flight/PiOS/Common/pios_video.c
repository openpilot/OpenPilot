/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_VIDEO Code for OSD video generator
 * @brief OSD generator, Parts from CL-OSD and SUPEROSD project
 * @{
 *
 * @file       pios_video.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      OSD generator, Parts from CL-OSD and SUPEROSD projects
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

#include "pios.h"
#if defined(PIOS_INCLUDE_VIDEO)

// Private methods
static void configure_hsync_timers();
static void stop_hsync_timers();
static void reset_hsync_timers();
static void prepare_line(uint32_t line_num);

// Private variables
extern xSemaphoreHandle osdSemaphore;
static const struct pios_video_cfg * dev_cfg;


// Define the buffers.
// For 256x192 pixel mode:
//   buffer0_level/buffer0_mask becomes buffer_level; and
//   buffer1_level/buffer1_mask becomes buffer_mask;
// For 192x128 pixel mode, allocations are as the names are written.
// divide by 8 because two bytes to a word.
// Must be allocated in one block, so it is in a struct.
struct _buffers
{
        uint16_t buffer0_level[GRAPHICS_HEIGHT*GRAPHICS_WIDTH];
        uint16_t buffer0_mask[GRAPHICS_HEIGHT*GRAPHICS_WIDTH];
        uint16_t buffer1_level[GRAPHICS_HEIGHT*GRAPHICS_WIDTH];
        uint16_t buffer1_mask[GRAPHICS_HEIGHT*GRAPHICS_WIDTH];
} buffers;

// Remove the struct definition (makes it easier to write for.)
#define         buffer0_level   (buffers.buffer0_level)
#define         buffer0_mask    (buffers.buffer0_mask)
#define         buffer1_level   (buffers.buffer1_level)
#define         buffer1_mask    (buffers.buffer1_mask)

// We define pointers to each of these buffers.
uint16_t *draw_buffer_level;
uint16_t *draw_buffer_mask;
uint16_t *disp_buffer_level;
uint16_t *disp_buffer_mask;

volatile uint8_t gLineType = LINE_TYPE_UNKNOWN;
volatile uint16_t gActiveLine = 0;
volatile uint16_t gActivePixmapLine = 0;
volatile uint16_t line=0;
volatile uint16_t Vsync_update=0;
static int16_t m_osdLines=0;

/**
 * swap_buffers: Swaps the two buffers. Contents in the display
 * buffer is seen on the output and the display buffer becomes
 * the new draw buffer.
 */
void swap_buffers()
{
        // While we could use XOR swap this is more reliable and
        // dependable and it's only called a few times per second.
        // Many compliers should optimise these to EXCH instructions.
        uint16_t *tmp;
        SWAP_BUFFS(tmp, disp_buffer_mask, draw_buffer_mask);
        SWAP_BUFFS(tmp, disp_buffer_level, draw_buffer_level);
}

uint32_t counter_position = 0;
uint32_t hsync_count, vsync_count;

void PIOS_Hsync_ISR()
{
	hsync_count++;
	// On tenth line prepare data which will start clocking out on 11th line
	if(Vsync_update==10)
	{
		prepare_line(0);
		gActiveLine = 1;
	}
	Vsync_update++;
	counter_position = DMA_GetCurrDataCounter(dev_cfg->level.dma.tx.channel);
}

void PIOS_Vsync_ISR() {
	//GPIO_ResetBits(GPIOC, GPIO_Pin_11);
	vsync_count++;
	static portBASE_TYPE xHigherPriorityTaskWoken;
	
    xHigherPriorityTaskWoken = pdFALSE;
	m_osdLines = gActiveLine;

	// load second image buffer
	stop_hsync_timers();
	
	// Wait for previous word to clock out of each
	TIM_Cmd(dev_cfg->pixel_timer.timer, ENABLE);
	uint32_t i = 0; 
	while(SPI_I2S_GetFlagStatus(dev_cfg->level.regs ,SPI_I2S_FLAG_TXE) == RESET && i < 30000) i++;
	while(SPI_I2S_GetFlagStatus(dev_cfg->mask.regs ,SPI_I2S_FLAG_TXE) == RESET && i < 30000) i++;
	while(SPI_I2S_GetFlagStatus(dev_cfg->level.regs ,SPI_I2S_FLAG_BSY) == SET && i < 30000) i++;
	while(SPI_I2S_GetFlagStatus(dev_cfg->mask.regs ,SPI_I2S_FLAG_BSY) == SET && i < 30000) i++;
	TIM_Cmd(dev_cfg->pixel_timer.timer, DISABLE);	

	swap_buffers();

	Vsync_update=0;

	// trigger redraw
	xHigherPriorityTaskWoken = xSemaphoreGiveFromISR(osdSemaphore, &xHigherPriorityTaskWoken);

	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken); 	//portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

uint16_t PIOS_Video_GetOSDLines(void) {
	return m_osdLines;
}

/**
 * Stops the pixel clock and ensures it ignores the rising edge.  To be used after a 
 * vsync until the first line is to be displayed
 */
static void stop_hsync_timers()
{
	// This removes the slave mode configuration
	TIM_Cmd(dev_cfg->pixel_timer.timer, DISABLE);
	TIM_InternalClockConfig(dev_cfg->pixel_timer.timer);
}

const struct pios_tim_callbacks px_callback = {
	.overflow = NULL,
	.edge = NULL,
};

/**
 * Reset the timer and configure for next call.  Keeps them synced.  Ideally this won't even be needed
 * since I don't think the slave mode gets lost, and this can simply be disable timer
 */
uint32_t failcount = 0;
static void reset_hsync_timers()
{
	// Stop both timers
	TIM_Cmd(dev_cfg->pixel_timer.timer, DISABLE);

	uint32_t tim_id;
	const struct pios_tim_channel *channels = &dev_cfg->hsync_capture;

	//BUG: This is nuts this line is needed.  It simply results in allocating
	//all the memory but somehow leaving it out breaks the timer functionality.
	// I do not see how these can be related
	if (failcount == 0) {
		if(PIOS_TIM_InitChannels(&tim_id, channels, 1, &px_callback, 0) < 0)
			failcount++;
	}

	dev_cfg->pixel_timer.timer->CNT = 0;

	// Listen to Channel1 (HSYNC)
	switch(dev_cfg->hsync_capture.timer_chan) {
		case TIM_Channel_1:
			TIM_SelectInputTrigger(dev_cfg->pixel_timer.timer, TIM_TS_TI1FP1);
			break;
		case TIM_Channel_2:
			TIM_SelectInputTrigger(dev_cfg->pixel_timer.timer, TIM_TS_TI2FP2);
			break;
		default:
			PIOS_Assert(0);
	}
	TIM_SelectSlaveMode(dev_cfg->pixel_timer.timer, TIM_SlaveMode_Trigger);
}


static void configure_hsync_timers()
{
	// Stop both timers
	TIM_Cmd(dev_cfg->pixel_timer.timer, DISABLE);

	// This is overkill but used for consistency.  No interrupts used for pixel clock
	// but this function calls the GPIO_Remap
	uint32_t tim_id;
	const struct pios_tim_channel *channels;

	// Init the channel to output the pixel clock
	channels = &dev_cfg->pixel_timer;
	PIOS_TIM_InitChannels(&tim_id, channels, 1, &px_callback, 0);

	// Init the channel to capture the pulse
	channels = &dev_cfg->hsync_capture;
	PIOS_TIM_InitChannels(&tim_id, channels, 1, &px_callback, 0);

	// Configure the input capture channel
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	switch(dev_cfg->hsync_capture.timer_chan) {
		case TIM_Channel_1:
			TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
			break;
		case TIM_Channel_2:
			TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
			break;
		default:
			PIOS_Assert(0);
	}
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0;
	TIM_ICInit(dev_cfg->pixel_timer.timer, &TIM_ICInitStructure);

	// Set up the channel to output the pixel clock
	switch(dev_cfg->pixel_timer.timer_chan) {
		case TIM_Channel_1:
			TIM_OC1Init(dev_cfg->pixel_timer.timer, &dev_cfg->tim_oc_init);
			TIM_OC1PreloadConfig(dev_cfg->pixel_timer.timer, TIM_OCPreload_Enable);
			TIM_SetCompare1(dev_cfg->pixel_timer.timer, 5);
			break;
		case TIM_Channel_2:
			TIM_OC2Init(dev_cfg->pixel_timer.timer, &dev_cfg->tim_oc_init);
			TIM_OC2PreloadConfig(dev_cfg->pixel_timer.timer, TIM_OCPreload_Enable);
			TIM_SetCompare2(dev_cfg->pixel_timer.timer, 5);
			break;
		case TIM_Channel_3:
			TIM_OC3Init(dev_cfg->pixel_timer.timer, &dev_cfg->tim_oc_init);
			TIM_OC3PreloadConfig(dev_cfg->pixel_timer.timer, TIM_OCPreload_Enable);
			TIM_SetCompare3(dev_cfg->pixel_timer.timer, 5);
			break;
		case TIM_Channel_4:
			TIM_OC4Init(dev_cfg->pixel_timer.timer, &dev_cfg->tim_oc_init);
			TIM_OC4PreloadConfig(dev_cfg->pixel_timer.timer, TIM_OCPreload_Enable);
			TIM_SetCompare4(dev_cfg->pixel_timer.timer, 5);
			break;
	}
	TIM_ARRPreloadConfig(dev_cfg->pixel_timer.timer, ENABLE);
	TIM_CtrlPWMOutputs(dev_cfg->pixel_timer.timer, ENABLE);

	// This shouldn't be needed as it should come from the config struture.  Something
	// is clobbering that
	TIM_PrescalerConfig(dev_cfg->pixel_timer.timer, 0, TIM_PSCReloadMode_Immediate);
	TIM_SetAutoreload(dev_cfg->pixel_timer.timer, 11);
}

DMA_TypeDef * main_dma;
DMA_TypeDef * mask_dma;
DMA_Stream_TypeDef * main_stream;
DMA_Stream_TypeDef * mask_stream;
void PIOS_Video_Init(const struct pios_video_cfg * cfg)
{
	dev_cfg = cfg; // store config before enabling interrupt

	configure_hsync_timers();

	const GPIO_InitTypeDef initStruct = {
		.GPIO_Pin = GPIO_Pin_12,
		.GPIO_Speed = GPIO_Speed_100MHz,
		.GPIO_Mode = GPIO_Mode_IN	,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL
	};
	GPIO_Init(GPIOC, &initStruct);
	
	/* SPI3 - MASKBUFFER */
	GPIO_Init(cfg->mask.sclk.gpio, (GPIO_InitTypeDef*)&(cfg->mask.sclk.init));
	GPIO_Init(cfg->mask.miso.gpio, (GPIO_InitTypeDef*)&(cfg->mask.miso.init));

	/* SPI1 SLAVE FRAMEBUFFER */
	GPIO_Init(cfg->level.sclk.gpio, (GPIO_InitTypeDef*)&(cfg->level.sclk.init));
	GPIO_Init(cfg->level.miso.gpio, (GPIO_InitTypeDef*)&(cfg->level.miso.init));

	if (cfg->mask.remap) {
		GPIO_PinAFConfig(cfg->mask.sclk.gpio,
						 __builtin_ctz(cfg->mask.sclk.init.GPIO_Pin),
						 cfg->mask.remap);
		GPIO_PinAFConfig(cfg->mask.miso.gpio,
						 __builtin_ctz(cfg->mask.miso.init.GPIO_Pin),
						 cfg->mask.remap);
	}
	if (cfg->level.remap)
	{
		GPIO_PinAFConfig(cfg->level.sclk.gpio,
						 __builtin_ctz(cfg->level.sclk.init.GPIO_Pin),
						 cfg->level.remap);
		GPIO_PinAFConfig(cfg->level.miso.gpio,
						 __builtin_ctz(cfg->level.miso.init.GPIO_Pin),
						 cfg->level.remap);
	}

	/* Initialize the SPI block */
	SPI_Init(cfg->level.regs, (SPI_InitTypeDef*)&(cfg->level.init));
	SPI_Init(cfg->mask.regs, (SPI_InitTypeDef*)&(cfg->mask.init));
	
	/* Enable SPI */
	SPI_Cmd(cfg->level.regs, ENABLE);
	SPI_Cmd(cfg->mask.regs, ENABLE);

	/* Configure DMA for SPI Tx SLAVE Maskbuffer */
	DMA_Cmd(cfg->mask.dma.tx.channel, DISABLE);
	DMA_Init(cfg->mask.dma.tx.channel, (DMA_InitTypeDef*)&(cfg->mask.dma.tx.init));

	/* Configure DMA for SPI Tx SLAVE Framebuffer*/
	DMA_Cmd(cfg->level.dma.tx.channel, DISABLE);
	DMA_Init(cfg->level.dma.tx.channel, (DMA_InitTypeDef*)&(cfg->level.dma.tx.init));

	/* Trigger interrupt when for half conversions too to indicate double buffer */
	DMA_ITConfig(cfg->level.dma.tx.channel, DMA_IT_TC, ENABLE);

	/* Configure and clear buffers */
    draw_buffer_level = buffer0_level;
    draw_buffer_mask = buffer0_mask;
    disp_buffer_level = buffer1_level;
    disp_buffer_mask = buffer1_mask;
	memset(disp_buffer_mask, 0, GRAPHICS_WIDTH*GRAPHICS_HEIGHT);
	memset(disp_buffer_level, 0, GRAPHICS_WIDTH*GRAPHICS_HEIGHT);
	memset(draw_buffer_mask, 0, GRAPHICS_WIDTH*GRAPHICS_HEIGHT);
	memset(draw_buffer_level, 0, GRAPHICS_WIDTH*GRAPHICS_HEIGHT);

	/* Configure DMA interrupt */
	
	NVIC_Init(&cfg->level.dma.irq.init);

	/* Enable SPI interrupts to DMA */
	SPI_I2S_DMACmd(cfg->mask.regs, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_I2S_DMACmd(cfg->level.regs, SPI_I2S_DMAReq_Tx, ENABLE);

	mask_dma = DMA1;
	main_dma = DMA2;
	main_stream = cfg->level.dma.tx.channel;
	mask_stream = cfg->mask.dma.tx.channel;
	/* Configure the Video Line interrupt */
	PIOS_EXTI_Init(cfg->hsync);
	PIOS_EXTI_Init(cfg->vsync);
}

/**
 * Prepare the system to watch for a HSYNC pulse to trigger the pixel
 * clock and clock out the next line
 */
static void prepare_line(uint32_t line_num)
{
	uint32_t buf_offset = line_num * GRAPHICS_WIDTH;

	DMA_ClearFlag(dev_cfg->mask.dma.tx.channel, DMA_FLAG_TCIF7 | DMA_FLAG_HTIF7 | DMA_FLAG_FEIF7 | DMA_FLAG_TEIF7);
	DMA_ClearFlag(dev_cfg->level.dma.tx.channel, DMA_FLAG_FEIF5 | DMA_FLAG_TEIF5);
	
	// Load new line
	DMA_MemoryTargetConfig(dev_cfg->level.dma.tx.channel,(uint32_t)&disp_buffer_level[buf_offset],DMA_Memory_0);
	DMA_MemoryTargetConfig(dev_cfg->mask.dma.tx.channel,(uint32_t)&disp_buffer_mask[buf_offset],DMA_Memory_0);

	// Enable DMA, Slave first
	DMA_SetCurrDataCounter(dev_cfg->level.dma.tx.channel, BUFFER_LINE_LENGTH);
	DMA_SetCurrDataCounter(dev_cfg->mask.dma.tx.channel, BUFFER_LINE_LENGTH);

	SPI_Cmd(dev_cfg->level.regs, ENABLE);
	SPI_Cmd(dev_cfg->mask.regs, ENABLE);

	/* Enable SPI interrupts to DMA */
	SPI_I2S_DMACmd(dev_cfg->mask.regs, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_I2S_DMACmd(dev_cfg->level.regs, SPI_I2S_DMAReq_Tx, ENABLE);

	DMA_Cmd(dev_cfg->level.dma.tx.channel, ENABLE);
	DMA_Cmd(dev_cfg->mask.dma.tx.channel, ENABLE);
	
	reset_hsync_timers();
}

void PIOS_VIDEO_DMA_Handler(void);
void DMA1_Stream7_IRQHandler(void) __attribute__ ((alias("PIOS_VIDEO_DMA_Handler")));
void DMA2_Stream5_IRQHandler(void) __attribute__ ((alias("PIOS_VIDEO_DMA_Handler")));

/**
 * @brief Interrupt for half and full buffer transfer
 */
void PIOS_VIDEO_DMA_Handler(void)
{
	// Try and make sure they can't get out of sync
	DMA_Cmd(dev_cfg->mask.dma.tx.channel, DISABLE);

	// Handle flags from stream channel
	if (DMA_GetFlagStatus(dev_cfg->level.dma.tx.channel,DMA_FLAG_TCIF5)) {	// whole double buffer filled
		DMA_ClearFlag(dev_cfg->level.dma.tx.channel,DMA_FLAG_TCIF5);
		if(gActiveLine < GRAPHICS_HEIGHT-2)
		{

			// Wait for previous word to clock out of each
			if( TIM4->CR1 & 0x0001) {
				PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
/*				uint32_t i = 0;
				while(SPI_I2S_GetFlagStatus(dev_cfg->level.regs ,SPI_I2S_FLAG_TXE) == RESET && i < 30000) i++;
				while(SPI_I2S_GetFlagStatus(dev_cfg->mask.regs ,SPI_I2S_FLAG_TXE) == RESET && i < 30000) i++;
				while(SPI_I2S_GetFlagStatus(dev_cfg->level.regs ,SPI_I2S_FLAG_BSY) == SET && i < 30000) i++;
				while(SPI_I2S_GetFlagStatus(dev_cfg->mask.regs ,SPI_I2S_FLAG_BSY) == SET && i < 30000) i++; */
			}

			SPI_Cmd(dev_cfg->level.regs, DISABLE);
			SPI_Cmd(dev_cfg->mask.regs, DISABLE);

			stop_hsync_timers();
			prepare_line(gActiveLine);
		}
		else if(gActiveLine == GRAPHICS_HEIGHT-2)
		{
			// Do nothing
		}
		else if(gActiveLine >= GRAPHICS_HEIGHT-1)
		{
			// STOP DMA, master first
			DMA_Cmd(dev_cfg->mask.dma.tx.channel, DISABLE);
			DMA_Cmd(dev_cfg->level.dma.tx.channel, DISABLE);
		}
		gActiveLine++;
	}
	else if (DMA_GetFlagStatus(dev_cfg->level.dma.tx.channel,DMA_FLAG_HTIF5)) {
		DMA_ClearFlag(dev_cfg->level.dma.tx.channel,DMA_FLAG_HTIF5);
	}
	else {
	}
}


#endif


