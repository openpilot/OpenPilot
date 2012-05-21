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
static void reset_hsync_timers();

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

void PIOS_Hsync_ISR()
{
	if(Vsync_update==11 && (DMA_GetCmdStatus(dev_cfg->level.dma.tx.channel)!=ENABLE))
	{
		// load first line of data
		DMA_MemoryTargetConfig(dev_cfg->mask.dma.tx.channel,(uint32_t)&disp_buffer_mask[0],DMA_Memory_0);
		DMA_MemoryTargetConfig(dev_cfg->level.dma.tx.channel,(uint32_t)&disp_buffer_level[0],DMA_Memory_0);

		// load second line of data
		DMA_MemoryTargetConfig(dev_cfg->mask.dma.tx.channel,(uint32_t)&disp_buffer_mask[GRAPHICS_WIDTH],DMA_Memory_1);
		DMA_MemoryTargetConfig(dev_cfg->level.dma.tx.channel,(uint32_t)&disp_buffer_level[GRAPHICS_WIDTH],DMA_Memory_1);

		// Enable DMA, Slave first
		DMA_SetCurrDataCounter(dev_cfg->level.dma.tx.channel,BUFFER_LINE_LENGTH);
		DMA_SetCurrDataCounter(dev_cfg->mask.dma.tx.channel,BUFFER_LINE_LENGTH);
		DMA_Cmd(dev_cfg->level.dma.tx.channel, ENABLE);
		DMA_Cmd(dev_cfg->mask.dma.tx.channel, ENABLE);

		reset_hsync_timers();
	}
	Vsync_update++;
}

void PIOS_Vsync_ISR() {
	static portBASE_TYPE xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;
	m_osdLines = gActiveLine;

	// load second image buffer
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
 * Reset the timer and configure for next call.  Keeps them synced.
 */
static void reset_hsync_timers()
{
	// Stop both timers
	TIM_Cmd(dev_cfg->pixel_timer.timer, DISABLE);

	// Listen to Channel1 (HSYNC)
	TIM_SelectInputTrigger(dev_cfg->pixel_timer.timer, TIM_TS_TI2FP2);
	TIM_SelectSlaveMode(dev_cfg->pixel_timer.timer, TIM_SlaveMode_Trigger);
	TIM_SelectMasterSlaveMode(dev_cfg->pixel_timer.timer, TIM_MasterSlaveMode_Enable);
}
/**
 * Called when the last pixel is clocked.
 */
static void line_overflow(uint32_t tim_id, uint32_t context, uint8_t chan_idx, uint16_t count)
{
	reset_hsync_timers();
}

const struct pios_tim_callbacks px_callback = {
	.overflow = NULL,
	.edge = NULL,
};

static void configure_hsync_timers()
{
	// Stop both timers
	TIM_Cmd(dev_cfg->pixel_timer.timer, DISABLE);
	TIM_Cmd(dev_cfg->line_timer, DISABLE);

	TIM_DeInit(dev_cfg->pixel_timer.timer);

	// This is overkill but used for consistency.  No interrupts used for pixel clock
	// but this function calls the GPIO_Remap
	uint32_t tim_id;
	const struct pios_tim_channel *channels = &dev_cfg->pixel_timer;
	PIOS_TIM_InitChannels(&tim_id, channels, 1, &px_callback, 0);

	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* TIM4_CH1 pin (PC.06) configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, dev_cfg->pixel_timer.remap);
	
	/* TIM4_CH2 pin (PB0.7) configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, dev_cfg->pixel_timer.remap);
	
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0;
	
	TIM_ICInit(dev_cfg->pixel_timer.timer, &TIM_ICInitStructure);


	// Set up the channel to output the pixel clock
	switch(dev_cfg->pixel_timer.timer_chan) {
		case TIM_Channel_1:
			TIM_OC1Init(dev_cfg->pixel_timer.timer, &dev_cfg->tim_oc_init);
			TIM_OC1PreloadConfig(dev_cfg->pixel_timer.timer, TIM_OCPreload_Enable);
			TIM_SetCompare1(dev_cfg->pixel_timer.timer, 7);
			break;
		case TIM_Channel_2:
			TIM_OC2Init(dev_cfg->pixel_timer.timer, &dev_cfg->tim_oc_init);
			TIM_OC2PreloadConfig(dev_cfg->pixel_timer.timer, TIM_OCPreload_Enable);
			TIM_SetCompare2(dev_cfg->pixel_timer.timer, 7);
			break;
		case TIM_Channel_3:
			TIM_OC3Init(dev_cfg->pixel_timer.timer, &dev_cfg->tim_oc_init);
			TIM_OC3PreloadConfig(dev_cfg->pixel_timer.timer, TIM_OCPreload_Enable);
			TIM_SetCompare3(dev_cfg->pixel_timer.timer, 7);
			break;
		case TIM_Channel_4:
			TIM_OC4Init(dev_cfg->pixel_timer.timer, &dev_cfg->tim_oc_init);
			TIM_OC4PreloadConfig(dev_cfg->pixel_timer.timer, TIM_OCPreload_Enable);
			TIM_SetCompare4(dev_cfg->pixel_timer.timer, 7);
			break;
	}
	TIM_ARRPreloadConfig(dev_cfg->pixel_timer.timer, ENABLE);
	TIM_CtrlPWMOutputs(dev_cfg->pixel_timer.timer, ENABLE);
	
	// This shouldn't be needed as it should come from the config struture.  Something
	// is clobbering that
	TIM_PrescalerConfig(dev_cfg->pixel_timer.timer, 0, TIM_PSCReloadMode_Immediate);
	TIM_SetAutoreload(dev_cfg->pixel_timer.timer, 15);

	// Set up the timer to run the pixel clock on the next hsync
	reset_hsync_timers();
}

void PIOS_Video_Init(const struct pios_video_cfg * cfg)
{

	dev_cfg = cfg; // store config before enabling interrupt

	configure_hsync_timers();

//	if (cfg->mask.remap) {
//		GPIO_PinAFConfig(cfg->mask.sclk.gpio,
//				__builtin_ctz(cfg->mask.sclk.init.GPIO_Pin),
//				cfg->mask.remap);
//		GPIO_PinAFConfig(cfg->mask.mosi.gpio,
//				__builtin_ctz(cfg->mask.mosi.init.GPIO_Pin),
//				cfg->mask.remap);
//	}
	if (cfg->level.remap)
	{
		GPIO_PinAFConfig(cfg->level.sclk.gpio,
				GPIO_PinSource3,
				//__builtin_ctz(cfg->mask.sclk.init.GPIO_Pin),
				cfg->level.remap);
		GPIO_PinAFConfig(cfg->level.miso.gpio,
				GPIO_PinSource4,
				//__builtin_ctz(cfg->level.miso.init.GPIO_Pin),
				cfg->level.remap);
	}

	/* SPI3 MASTER MASKBUFFER */
//	GPIO_Init(cfg->mask.sclk.gpio, (GPIO_InitTypeDef*)&(cfg->mask.sclk.init));
//	GPIO_Init(cfg->mask.mosi.gpio, (GPIO_InitTypeDef*)&(cfg->mask.mosi.init));

	/* SPI1 SLAVE FRAMEBUFFER */
	GPIO_Init(cfg->level.sclk.gpio, (GPIO_InitTypeDef*)&(cfg->level.sclk.init));
	GPIO_Init(cfg->level.miso.gpio, (GPIO_InitTypeDef*)&(cfg->level.miso.init));

	/* Initialize the SPI block */
	SPI_Init(cfg->level.regs, (SPI_InitTypeDef*)&(cfg->level.init));
	SPI_Init(cfg->mask.regs, (SPI_InitTypeDef*)&(cfg->mask.init));

	/* Enable SPI */
	SPI_Cmd(cfg->level.regs, ENABLE);
	SPI_Cmd(cfg->mask.regs, ENABLE);

	/* Configure DMA for SPI Tx MASTER */
//	DMA_Cmd(cfg->mask.dma.tx.channel, DISABLE);
//	DMA_Init(cfg->mask.dma.tx.channel, (DMA_InitTypeDef*)&(cfg->mask.dma.tx.init));

	/* Configure DMA for SPI Tx SLAVE */
	DMA_Cmd(cfg->level.dma.tx.channel, DISABLE);
	DMA_Init(cfg->level.dma.tx.channel, (DMA_InitTypeDef*)&(cfg->level.dma.tx.init));


	/* Trigger interrupt when for half conversions too to indicate double buffer */
	DMA_ITConfig(cfg->level.dma.tx.channel, DMA_IT_TC, ENABLE);

    draw_buffer_level = buffer0_level;
    draw_buffer_mask = buffer0_mask;
    disp_buffer_level = buffer1_level;
    disp_buffer_mask = buffer1_mask;

	/* Configure DMA interrupt */
//	NVIC_Init(&cfg->mask.dma.irq.init);
	NVIC_Init(&cfg->level.dma.irq.init);

	/* double buffer config */
	for (uint16_t x = 0; x < GRAPHICS_WIDTH*GRAPHICS_HEIGHT; x++) {
		  disp_buffer_level[x] = 0;
		  disp_buffer_mask[x] = 0;
	}
	for (uint16_t x = 0; x < GRAPHICS_WIDTH*GRAPHICS_HEIGHT; x++) {
		  draw_buffer_level[x] = 0;
		  draw_buffer_mask[x] = 0;
	}

//	DMA_DoubleBufferModeConfig(cfg->mask.dma.tx.channel,(uint32_t)&disp_buffer_mask[GRAPHICS_WIDTH],DMA_Memory_0);
	DMA_DoubleBufferModeConfig(cfg->level.dma.tx.channel,(uint32_t)&disp_buffer_level[GRAPHICS_WIDTH],DMA_Memory_0);

//	DMA_MemoryTargetConfig(dev_cfg->mask.dma.tx.channel,(uint32_t)&disp_buffer_mask[0],DMA_Memory_0);
	DMA_MemoryTargetConfig(dev_cfg->level.dma.tx.channel,(uint32_t)&disp_buffer_level[0],DMA_Memory_0);

	/* Enable double buffering */
//	DMA_DoubleBufferModeCmd(cfg->mask.dma.tx.channel,ENABLE);
	DMA_DoubleBufferModeCmd(cfg->level.dma.tx.channel,ENABLE);

	/* Enable SPI interrupts to DMA */
//	SPI_I2S_DMACmd(cfg->mask.regs, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_I2S_DMACmd(cfg->level.regs, SPI_I2S_DMAReq_Tx, ENABLE);

	/* Configure the Video Line interrupt */
	PIOS_EXTI_Init(cfg->hsync);
	PIOS_EXTI_Init(cfg->vsync);
}


void PIOS_VIDEO_DMA_Handler(void);
void DMA1_Stream7_IRQHandler(void) __attribute__ ((alias("PIOS_VIDEO_DMA_Handler")));
void DMA2_Stream5_IRQHandler(void) __attribute__ ((alias("PIOS_VIDEO_DMA_Handler")));

/**
 * @brief Interrupt for half and full buffer transfer
 *
 * This interrupt handler swaps between the two halfs of the double buffer to make
 * sure the ahrs uses the most recent data.  Only swaps data when AHRS is idle, but
 * really this is a pretense of a sanity check since the DMA engine is consantly
 * running in the background.  Keep an eye on the ekf_too_slow variable to make sure
 * it's keeping up.
 */
void PIOS_VIDEO_DMA_Handler(void)
{
	PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);

	reset_hsync_timers();

	if (DMA_GetFlagStatus(DMA1_Stream7,DMA_FLAG_TCIF7)) {	// whole double buffer filled

		DMA_ClearFlag(DMA1_Stream7,DMA_FLAG_TCIF7);
		//PIOS_LED_Toggle(LED2);
	}
	else if (DMA_GetFlagStatus(DMA1_Stream7,DMA_FLAG_HTIF7)) {
		DMA_ClearFlag(DMA1_Stream7,DMA_FLAG_HTIF7);
	}
	else {

	}

	if (DMA_GetFlagStatus(dev_cfg->level.dma.tx.channel,DMA_FLAG_TCIF5)) {	// whole double buffer filled

		line = gActiveLine*GRAPHICS_WIDTH;
		if(gActiveLine < GRAPHICS_HEIGHT-2)
		{
			// Load new line
			switch(DMA_GetCurrentMemoryTarget(dev_cfg->mask.dma.tx.channel))
			{
			case 0:
				DMA_MemoryTargetConfig(dev_cfg->level.dma.tx.channel,(uint32_t)&disp_buffer_level[line],DMA_Memory_1);
				DMA_MemoryTargetConfig(dev_cfg->mask.dma.tx.channel,(uint32_t)&disp_buffer_mask[line],DMA_Memory_1);
				break;
			case 1:
				DMA_MemoryTargetConfig(dev_cfg->level.dma.tx.channel,(uint32_t)&disp_buffer_level[line],DMA_Memory_0);
				DMA_MemoryTargetConfig(dev_cfg->mask.dma.tx.channel,(uint32_t)&disp_buffer_mask[line],DMA_Memory_0);
				break;
			}
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
			gActiveLine = 0;
		}
		gActiveLine++;


		DMA_ClearFlag(dev_cfg->level.dma.tx.channel,DMA_FLAG_TCIF5);
		//PIOS_LED_Toggle(LED3);
	}
	else if (DMA_GetFlagStatus(dev_cfg->level.dma.tx.channel,DMA_FLAG_HTIF5)) {
		DMA_ClearFlag(dev_cfg->level.dma.tx.channel,DMA_FLAG_HTIF5);
	}
	else {

	}

}


#endif


