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

#ifdef PIOS_INCLUDE_VIDEO

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
        uint8_t buffer0_level[GRAPHICS_HEIGHT*GRAPHICS_WIDTH];
        uint8_t buffer0_mask[GRAPHICS_HEIGHT*GRAPHICS_WIDTH];
        uint8_t buffer1_level[GRAPHICS_HEIGHT*GRAPHICS_WIDTH];
        uint8_t buffer1_mask[GRAPHICS_HEIGHT*GRAPHICS_WIDTH];
} buffers;

// Remove the struct definition (makes it easier to write for.)
#define         buffer0_level   (buffers.buffer0_level)
#define         buffer0_mask    (buffers.buffer0_mask)
#define         buffer1_level   (buffers.buffer1_level)
#define         buffer1_mask    (buffers.buffer1_mask)

// We define pointers to each of these buffers.
uint8_t *draw_buffer_level;
uint8_t *draw_buffer_mask;
uint8_t *disp_buffer_level;
uint8_t *disp_buffer_mask;

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
        uint8_t *tmp;
        SWAP_BUFFS(tmp, disp_buffer_mask, draw_buffer_mask);
        SWAP_BUFFS(tmp, disp_buffer_level, draw_buffer_level);
}

bool PIOS_Hsync_ISR() {

	if(dev_cfg->hsync->pin.gpio->IDR & dev_cfg->hsync->pin.init.GPIO_Pin) {
		//rising
		if(gLineType == LINE_TYPE_GRAPHICS)
		{
			// Activate new line
			DMA_Cmd(dev_cfg->level.dma.tx.channel, ENABLE);
			DMA_Cmd(dev_cfg->mask.dma.tx.channel, ENABLE);
		}
	} else {
		//falling
		gLineType = LINE_TYPE_UNKNOWN; // Default case
		gActiveLine++;

		if ((gActiveLine >= GRAPHICS_LINE) && (gActiveLine < (GRAPHICS_LINE + GRAPHICS_HEIGHT))) {
			gLineType = LINE_TYPE_GRAPHICS;
			gActivePixmapLine = (gActiveLine - GRAPHICS_LINE);
			line = gActivePixmapLine*GRAPHICS_WIDTH;
		}

		if(gLineType == LINE_TYPE_GRAPHICS)
		{
			// Load new line
			DMA_Cmd(dev_cfg->mask.dma.tx.channel, DISABLE);
			DMA_Cmd(dev_cfg->level.dma.tx.channel, DISABLE);
			DMA_MemoryTargetConfig(dev_cfg->level.dma.tx.channel,(uint32_t)&disp_buffer_level[line],DMA_Memory_0);
			DMA_MemoryTargetConfig(dev_cfg->mask.dma.tx.channel,(uint32_t)&disp_buffer_mask[line],DMA_Memory_0);
			DMA_SetCurrDataCounter(dev_cfg->level.dma.tx.channel,BUFFER_LINE_LENGTH);
			DMA_SetCurrDataCounter(dev_cfg->mask.dma.tx.channel,BUFFER_LINE_LENGTH);
		}
	}

	return false;
}

bool PIOS_Vsync_ISR() {
	static portBASE_TYPE xHigherPriorityTaskWoken;
	//PIOS_LED_Toggle(LED3);

	//if(gActiveLine > 200)
    xHigherPriorityTaskWoken = pdFALSE;
	m_osdLines = gActiveLine;
	{
		gActiveLine = 0;
		Vsync_update++;
		if(Vsync_update>=2)
		{
			swap_buffers();
			Vsync_update=0;
			xHigherPriorityTaskWoken = xSemaphoreGiveFromISR(osdSemaphore, &xHigherPriorityTaskWoken);
		}
	}
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken); 	//portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);

	return xHigherPriorityTaskWoken == pdTRUE;
}

uint16_t PIOS_Video_GetOSDLines(void) {
	return m_osdLines;
}

void PIOS_Video_Init(const struct pios_video_cfg * cfg){

	dev_cfg = cfg; // store config before enabling interrupt

	if (cfg->mask.remap) {
		GPIO_PinAFConfig(cfg->mask.sclk.gpio,
				__builtin_ctz(cfg->mask.sclk.init.GPIO_Pin),
				cfg->mask.remap);
		GPIO_PinAFConfig(cfg->mask.mosi.gpio,
				__builtin_ctz(cfg->mask.mosi.init.GPIO_Pin),
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

	/* SPI3 MASTER MASKBUFFER */
	GPIO_Init(cfg->mask.sclk.gpio, (GPIO_InitTypeDef*)&(cfg->mask.sclk.init));
	GPIO_Init(cfg->mask.mosi.gpio, (GPIO_InitTypeDef*)&(cfg->mask.mosi.init));

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
	DMA_Cmd(cfg->mask.dma.tx.channel, DISABLE);
	DMA_Init(cfg->mask.dma.tx.channel, (DMA_InitTypeDef*)&(cfg->mask.dma.tx.init));

	/* Configure DMA for SPI Tx SLAVE */
	DMA_Cmd(cfg->level.dma.tx.channel, DISABLE);
	DMA_Init(cfg->level.dma.tx.channel, (DMA_InitTypeDef*)&(cfg->level.dma.tx.init));


	/* Trigger interrupt when for half conversions too to indicate double buffer */
	DMA_ITConfig(cfg->mask.dma.tx.channel, DMA_IT_TC, ENABLE);
	/*DMA_ClearFlag(cfg->mask.dma.tx.channel,DMA_FLAG_TCIF5);
	DMA_ClearITPendingBit(cfg->mask.dma.tx.channel, DMA_IT_TCIF5);

	DMA_ClearFlag(cfg->level.dma.tx.channel,DMA_FLAG_TCIF5);
	DMA_ClearITPendingBit(cfg->level.dma.tx.channel, DMA_IT_TCIF5);
*/

	/* Configure DMA interrupt */
	NVIC_Init(&cfg->level.dma.irq.init);
	NVIC_Init(&cfg->mask.dma.irq.init);

	/* Enable SPI interrupts to DMA */
	SPI_I2S_DMACmd(cfg->level.regs, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_I2S_DMACmd(cfg->mask.regs, SPI_I2S_DMAReq_Tx, ENABLE);

	/* Configure the Video Line interrupt */
	PIOS_EXTI_Init(cfg->hsync);
	PIOS_EXTI_Init(cfg->vsync);


    draw_buffer_level = buffer0_level;
    draw_buffer_mask = buffer0_mask;
    disp_buffer_level = buffer1_level;
    disp_buffer_mask = buffer1_mask;
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

void PIOS_VIDEO_DMA_Handler(void);
void DMA1_Stream7_IRQHandler(void) __attribute__ ((alias("PIOS_VIDEO_DMA_Handler")));
void DMA2_Stream5_IRQHandler(void) __attribute__ ((alias("PIOS_VIDEO_DMA_Handler")));


void PIOS_VIDEO_DMA_Handler(void)
{
	if (DMA_GetFlagStatus(DMA1_Stream7,DMA_FLAG_TCIF7)) {	// transfer completed load next line
		DMA_ClearFlag(DMA1_Stream7,DMA_FLAG_TCIF7);
		//PIOS_LED_Off(LED2);
		/*if(gLineType == LINE_TYPE_GRAPHICS)
		{
			// Load new line
			DMA_Cmd(dev_cfg->mask.dma.tx.channel, DISABLE);
			DMA_Cmd(dev_cfg->level.dma.tx.channel, DISABLE);
			DMA_MemoryTargetConfig(dev_cfg->level.dma.tx.channel,(uint32_t)&disp_buffer_level[line],DMA_Memory_0);
			DMA_MemoryTargetConfig(dev_cfg->mask.dma.tx.channel,(uint32_t)&disp_buffer_mask[line],DMA_Memory_0);
			//DMA_ClearFlag(dev_cfg->mask.dma.tx.channel,DMA_FLAG_TCIF5); // <-- TODO: HARDCODED
			//DMA_ClearFlag(dev_cfg->level.dma.tx.channel,DMA_FLAG_TCIF5); // <-- TODO: HARDCODED
			DMA_SetCurrDataCounter(dev_cfg->level.dma.tx.channel,BUFFER_LINE_LENGTH);
			DMA_SetCurrDataCounter(dev_cfg->mask.dma.tx.channel,BUFFER_LINE_LENGTH);
		}*/
		//PIOS_LED_Toggle(LED2);
	}
	else if (DMA_GetFlagStatus(DMA1_Stream7,DMA_FLAG_HTIF7)) {
		DMA_ClearFlag(DMA1_Stream7,DMA_FLAG_HTIF7);
	}
	else {

	}

	if (DMA_GetFlagStatus(DMA2_Stream5,DMA_FLAG_TCIF5)) {	// whole double buffer filled
		DMA_ClearFlag(DMA2_Stream5,DMA_FLAG_TCIF5);
		//PIOS_LED_Toggle(LED3);
	}
	else if (DMA_GetFlagStatus(DMA2_Stream5,DMA_FLAG_HTIF5)) {
		DMA_ClearFlag(DMA2_Stream5,DMA_FLAG_HTIF5);
	}
	else {

	}

}

#endif /* PIOS_INCLUDE_VIDEO */
