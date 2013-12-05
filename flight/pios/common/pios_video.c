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
#include "pios_config.h"
#include "pios_video.h"
#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"


// We need direct register bit manipulations for full resolution.
// The nice but time consuming functions like e.g. DMA_GetFlagStatus etc.
// called in interrupt context cost too much time for the very short time
// we have from SPI out of the last line pixel to the next Hsync.
#define DIRECT_REGISTER_ACCESS


#ifdef PIOS_INCLUDE_VIDEO

extern xSemaphoreHandle osdSemaphore;

#ifdef PAL
const uint32_t period = 10;
const uint32_t dc     = (10 / 2);
#else
const uint32_t period = 11;
const uint32_t dc     = (11 / 2);
#endif

// Allocate buffers.
// Must be allocated in one block, so it is in a struct.
struct _buffers {
    uint8_t buffer0_level[BUFFER_HEIGHT * BUFFER_WIDTH];
    uint8_t buffer0_mask[BUFFER_HEIGHT * BUFFER_WIDTH];
    uint8_t buffer1_level[BUFFER_HEIGHT * BUFFER_WIDTH];
    uint8_t buffer1_mask[BUFFER_HEIGHT * BUFFER_WIDTH];
} buffers;

// Remove the struct definition (makes it easier to write for).
#define buffer0_level (buffers.buffer0_level)
#define buffer0_mask  (buffers.buffer0_mask)
#define buffer1_level (buffers.buffer1_level)
#define buffer1_mask  (buffers.buffer1_mask)

// Pointers to each of these buffers.
uint8_t *draw_buffer_level;
uint8_t *draw_buffer_mask;
uint8_t *disp_buffer_level;
uint8_t *disp_buffer_mask;

volatile uint16_t gActiveLine  = 0;
volatile uint16_t Hsync_update = 0;

// Private const
static const struct pios_video_cfg *dev_cfg;

// Private variables
static int16_t m_osdLines = 0;

// Private functions
static void swap_buffers();
static void prepare_line(uint32_t line_num);

#ifndef DIRECT_REGISTER_ACCESS
static void flush_spi();
static void stop_pixel_timer();
#endif


/**
 * @brief Vsync interrupt service routine
 */
bool PIOS_Vsync_ISR()
{
    static portBASE_TYPE xHigherPriorityTaskWoken;
    static uint16_t Vsync_update = 0;

    xHigherPriorityTaskWoken = pdFALSE;
    m_osdLines = gActiveLine;
    gActiveLine = 0;
    Hsync_update = 0;
    Vsync_update++;
    if (Vsync_update >= 2) {		// every second field: swap buffers and trigger redraw
        Vsync_update = 0;
        swap_buffers();
        xHigherPriorityTaskWoken = xSemaphoreGiveFromISR(osdSemaphore, &xHigherPriorityTaskWoken);
    }
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);

    return xHigherPriorityTaskWoken == pdTRUE;
}


/**
 * @brief Hsync interrupt service routine
 */
bool PIOS_Hsync_ISR()
{
    // prepare data which will start clocking out on GRAPHICS_LINE+1
    if (Hsync_update++ == GRAPHICS_LINE) {
        prepare_line(0);
    }

    return true;
}


void PIOS_VIDEO_DMA_Handler(void);
void DMA1_Stream7_IRQHandler(void) __attribute__((alias("PIOS_VIDEO_DMA_Handler")));
void DMA2_Stream5_IRQHandler(void) __attribute__((alias("PIOS_VIDEO_DMA_Handler")));

/**
 * @brief DMA transfer complete interrupt handler
 */
void PIOS_VIDEO_DMA_Handler(void)
{
#ifdef DIRECT_REGISTER_ACCESS
    // Handle flags from DMA stream channel
    if ((dev_cfg->mask_dma->HISR & DMA_FLAG_TCIF7) && (dev_cfg->level_dma->HISR & DMA_FLAG_TCIF5)) {
        dev_cfg->mask_dma->HIFCR |= DMA_FLAG_TCIF7;
        dev_cfg->level_dma->HIFCR |= DMA_FLAG_TCIF5;
        // Flush SPI, short version
        while (dev_cfg->level.regs->SR & SPI_I2S_FLAG_BSY);
        // Stop pixel timer
        dev_cfg->pixel_timer.timer->CR1 &= (uint16_t)~TIM_CR1_CEN;
        // Disable the pixel timer slave mode configuration
        dev_cfg->pixel_timer.timer->SMCR &= (uint16_t)~TIM_SMCR_SMS;
        if (gActiveLine < BUFFER_HEIGHT) {	// lines existing
            prepare_line(gActiveLine);
        } else {								// last line completed
            // Stop DMA
            dev_cfg->mask.dma.tx.channel->CR &= ~(uint32_t)DMA_SxCR_EN;
            dev_cfg->level.dma.tx.channel->CR &= ~(uint32_t)DMA_SxCR_EN;
        }
    }
#else
    // Handle flags from DMA stream channel
    if (DMA_GetFlagStatus(dev_cfg->mask.dma.tx.channel, DMA_FLAG_TCIF7) && DMA_GetFlagStatus(dev_cfg->level.dma.tx.channel, DMA_FLAG_TCIF5)) {
        DMA_ClearFlag(dev_cfg->mask.dma.tx.channel, DMA_FLAG_TCIF7);
        DMA_ClearFlag(dev_cfg->level.dma.tx.channel, DMA_FLAG_TCIF5);
        flush_spi();
        stop_pixel_timer();
        if (gActiveLine < BUFFER_HEIGHT) {	// lines existing
            prepare_line(gActiveLine);
        } else {								// last line completed
            // Stop DMA
            DMA_Cmd(dev_cfg->mask.dma.tx.channel, DISABLE);
            DMA_Cmd(dev_cfg->level.dma.tx.channel, DISABLE);
        }
    }
#endif
}


/**
 * Prepare the system to watch for a Hsync pulse to trigger the pixel clock and clock out the next line
 */
static void prepare_line(uint32_t line_num)
{
#ifdef DIRECT_REGISTER_ACCESS
    uint32_t buf_offset = line_num * BUFFER_WIDTH;

    gActiveLine++;

    // Prepare next line DMA:
    // Clear DMA flags
    dev_cfg->mask_dma->HIFCR |= DMA_FLAG_TCIF7 | DMA_FLAG_HTIF7 | DMA_FLAG_FEIF7 | DMA_FLAG_TEIF7 | DMA_FLAG_DMEIF7;
    dev_cfg->level_dma->HIFCR |= DMA_FLAG_TCIF5 | DMA_FLAG_HTIF5 | DMA_FLAG_FEIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_DMEIF5;
    // Load new line
    dev_cfg->mask.dma.tx.channel->M0AR = (uint32_t)&disp_buffer_mask[buf_offset];
    dev_cfg->level.dma.tx.channel->M0AR = (uint32_t)&disp_buffer_level[buf_offset];
    // Set length
    dev_cfg->mask.dma.tx.channel->NDTR = (uint16_t)BUFFER_WIDTH;
    dev_cfg->level.dma.tx.channel->NDTR = (uint16_t)BUFFER_WIDTH;
    // Enable SPI
    dev_cfg->mask.regs->CR1 |= SPI_CR1_SPE;
    dev_cfg->level.regs->CR1 |= SPI_CR1_SPE;
    // Enable DMA
    dev_cfg->mask.dma.tx.channel->CR |= (uint32_t)DMA_SxCR_EN;
    dev_cfg->level.dma.tx.channel->CR |= (uint32_t)DMA_SxCR_EN;

    // Stop and recharge pixel timer for next Hsync:
    // Stop pixel timer
    dev_cfg->pixel_timer.timer->CR1 &= (uint16_t)~TIM_CR1_CEN;
	// Set the prescaler value
	dev_cfg->pixel_timer.timer->PSC = 0;
	// Set to immediate
	dev_cfg->pixel_timer.timer->EGR = TIM_PSCReloadMode_Immediate;
	// Set initial line offset
    dev_cfg->pixel_timer.timer->CNT = 0xffff - dc * GRAPHICS_COLUMN;			// JR_HINT put to struct for PAL/NTSC auto detect
    // Reset the SMS bits
    dev_cfg->pixel_timer.timer->SMCR &= (uint16_t)~TIM_SMCR_SMS;
    // Select the slave mode waiting for Hsync
    dev_cfg->pixel_timer.timer->SMCR |= TIM_SlaveMode_Trigger;
#else
    uint32_t buf_offset = line_num * BUFFER_WIDTH;

    gActiveLine++;

    // Prepare next line DMA
    // Clear DMA flags
    DMA_ClearFlag(dev_cfg->mask.dma.tx.channel, DMA_FLAG_TCIF7 | DMA_FLAG_HTIF7 | DMA_FLAG_FEIF7 | DMA_FLAG_TEIF7 | DMA_FLAG_DMEIF7);
    DMA_ClearFlag(dev_cfg->level.dma.tx.channel, DMA_FLAG_TCIF5 | DMA_FLAG_HTIF5 | DMA_FLAG_FEIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_DMEIF5);
    // Load new line
    DMA_MemoryTargetConfig(dev_cfg->mask.dma.tx.channel, (uint32_t)&disp_buffer_mask[buf_offset], DMA_Memory_0);
    DMA_MemoryTargetConfig(dev_cfg->level.dma.tx.channel, (uint32_t)&disp_buffer_level[buf_offset], DMA_Memory_0);
    // Set length
    DMA_SetCurrDataCounter(dev_cfg->mask.dma.tx.channel, BUFFER_WIDTH);
    DMA_SetCurrDataCounter(dev_cfg->level.dma.tx.channel, BUFFER_WIDTH);
    // Enable SPI
    SPI_Cmd(dev_cfg->mask.regs, ENABLE);
    SPI_Cmd(dev_cfg->level.regs, ENABLE);
    // Enable DMA
    DMA_Cmd(dev_cfg->mask.dma.tx.channel, ENABLE);
    DMA_Cmd(dev_cfg->level.dma.tx.channel, ENABLE);

    // Stop and recharge pixel timer for next Hsync
    // Stop timer
    TIM_Cmd(dev_cfg->pixel_timer.timer, DISABLE);
    // Set prescaler
	TIM_PrescalerConfig(dev_cfg->pixel_timer.timer, 0, TIM_PSCReloadMode_Immediate);
	// Set initial line offset
    dev_cfg->pixel_timer.timer->CNT = 0xffff - dc * GRAPHICS_COLUMN;			// JR_HINT put to struct for PAL/NTSC auto detect
    // Set timer slave mode waiting for Hsync
    TIM_SelectSlaveMode(dev_cfg->pixel_timer.timer, TIM_SlaveMode_Trigger);
#endif
}


/**
 * swap_buffers: Swaps the two buffers. Contents in the display
 * buffer is seen on the output and the display buffer becomes
 * the new draw buffer.
 */
static void swap_buffers()
{
    // While we could use XOR swap this is more reliable and
    // dependable and it's only called a few times per second.
    // Many compilers should optimize these to EXCH instructions.
    uint8_t *tmp;

    SWAP_BUFFS(tmp, disp_buffer_mask, draw_buffer_mask);
    SWAP_BUFFS(tmp, disp_buffer_level, draw_buffer_level);
}


// JR_HINT refactor
// Weird that this is needed.
// It results in allocating all timer channel.
// But leaving it out breaks the timer functionality.
// Ideas:
// Start NVIC for TIM4 or like in pios_board.c: PIOS_TIM_InitClock(&pios_tim4_cfg); ???
static void weird_timer_stuff()
{
    uint32_t tim_id;
    const struct pios_tim_channel *channels = &dev_cfg->hsync_capture;
    const struct pios_tim_callbacks px_callback = {.overflow = NULL, .edge = NULL};
    while (PIOS_TIM_InitChannels(&tim_id, channels, 1, &px_callback, 0) >= 0);
}


/**
 * Init
 */
void PIOS_Video_Init(const struct pios_video_cfg *cfg)
{
    dev_cfg = cfg; // store config before enabling interrupt

    // SPI3 - Maskbuffer
    GPIO_Init(cfg->mask.sclk.gpio, (GPIO_InitTypeDef *)&(cfg->mask.sclk.init));
    GPIO_Init(cfg->mask.miso.gpio, (GPIO_InitTypeDef *)&(cfg->mask.miso.init));
    if (cfg->mask.remap) {
        GPIO_PinAFConfig(cfg->mask.sclk.gpio, __builtin_ctz(cfg->mask.sclk.init.GPIO_Pin), cfg->mask.remap);
        GPIO_PinAFConfig(cfg->mask.miso.gpio, __builtin_ctz(cfg->mask.miso.init.GPIO_Pin), cfg->mask.remap);
    }

    // SPI1 - Levelbuffer
    GPIO_Init(cfg->level.sclk.gpio, (GPIO_InitTypeDef *)&(cfg->level.sclk.init));
    GPIO_Init(cfg->level.miso.gpio, (GPIO_InitTypeDef *)&(cfg->level.miso.init));
    if (cfg->level.remap) {
        GPIO_PinAFConfig(cfg->level.sclk.gpio, __builtin_ctz(cfg->level.sclk.init.GPIO_Pin), cfg->level.remap);
        GPIO_PinAFConfig(cfg->level.miso.gpio, __builtin_ctz(cfg->level.miso.init.GPIO_Pin), cfg->level.remap);
    }

    GPIO_Init(cfg->hsync_capture.pin.gpio, (GPIO_InitTypeDef *)&(cfg->hsync_capture.pin.init));
    if (cfg->hsync_capture.remap) {
        GPIO_PinAFConfig(cfg->hsync_capture.pin.gpio, __builtin_ctz(cfg->hsync_capture.pin.init.GPIO_Pin), cfg->hsync_capture.remap);
    }

    GPIO_Init(cfg->pixel_timer.pin.gpio, (GPIO_InitTypeDef *)&(cfg->pixel_timer.pin.init));
    if (cfg->pixel_timer.remap) {
        GPIO_PinAFConfig(cfg->pixel_timer.pin.gpio, __builtin_ctz(cfg->pixel_timer.pin.init.GPIO_Pin), cfg->pixel_timer.remap);
    }

    TIM_OC1Init(cfg->pixel_timer.timer, &cfg->tim_oc_init);
    TIM_OC1PreloadConfig(cfg->pixel_timer.timer, TIM_OCPreload_Enable);
    TIM_SelectInputTrigger(cfg->pixel_timer.timer, TIM_TS_TI2FP2);

    TIM_SetCompare1(cfg->pixel_timer.timer, dc);								// JR_HINT put to struct for PAL/NTSC auto detect
    TIM_SetAutoreload(cfg->pixel_timer.timer, period);							// JR_HINT put to struct for PAL/NTSC auto detect

    TIM_ARRPreloadConfig(cfg->pixel_timer.timer, ENABLE);
    TIM_CtrlPWMOutputs(cfg->pixel_timer.timer, ENABLE);

    /* Initialize the SPI block */
    SPI_Init(cfg->level.regs, (SPI_InitTypeDef *)&(cfg->level.init));
    SPI_Init(cfg->mask.regs, (SPI_InitTypeDef *)&(cfg->mask.init));

    /* Configure DMA for SPI Tx Maskbuffer */
    DMA_Init(cfg->mask.dma.tx.channel, (DMA_InitTypeDef *)&(cfg->mask.dma.tx.init));

    /* Configure DMA for SPI Tx Levelbuffer*/
    DMA_Init(cfg->level.dma.tx.channel, (DMA_InitTypeDef *)&(cfg->level.dma.tx.init));

    /* Trigger interrupt when transfer complete */
    DMA_ITConfig(cfg->level.dma.tx.channel, DMA_IT_TC, ENABLE);

    /* Configure and clear buffers */
    draw_buffer_level = buffer0_level;
    draw_buffer_mask  = buffer0_mask;
    disp_buffer_level = buffer1_level;
    disp_buffer_mask  = buffer1_mask;
    memset(disp_buffer_mask, 0, BUFFER_HEIGHT * BUFFER_WIDTH);
    memset(disp_buffer_level, 0, BUFFER_HEIGHT * BUFFER_WIDTH);
    memset(draw_buffer_mask, 0, BUFFER_HEIGHT * BUFFER_WIDTH);
    memset(draw_buffer_level, 0, BUFFER_HEIGHT * BUFFER_WIDTH);

    /* Configure DMA interrupt */
    NVIC_Init(&cfg->level.dma.irq.init);

    /* Enable SPI interrupts to DMA */
    SPI_I2S_DMACmd(cfg->mask.regs, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd(cfg->level.regs, SPI_I2S_DMAReq_Tx, ENABLE);

    /* Configure the Video Line interrupt */
    PIOS_EXTI_Init(cfg->hsync);
    PIOS_EXTI_Init(cfg->vsync);

    // set pixel levels to zero
    PIOS_Servo_Set(0, 0);
    PIOS_Servo_Set(1, 0);

    // weird
    weird_timer_stuff();
}


/**
 *
 */
uint16_t PIOS_Video_GetOSDLines(void)
{
    return m_osdLines;
}


#ifndef DIRECT_REGISTER_ACCESS
/**
 * Check both SPI for the stop sequence before disabling them
 */
static void flush_spi()
{
    bool level_empty   = false;
    bool mask_empty    = false;
    bool level_stopped = false;
    bool mask_stopped  = false;

    // Can't flush if clock not running
    while ((dev_cfg->pixel_timer.timer->CR1 & TIM_CR1_CEN) && (!level_stopped | !mask_stopped)) {
        level_empty |= SPI_I2S_GetFlagStatus(dev_cfg->level.regs, SPI_I2S_FLAG_TXE) == SET;
        mask_empty  |= SPI_I2S_GetFlagStatus(dev_cfg->mask.regs, SPI_I2S_FLAG_TXE) == SET;

        if (level_empty && !level_stopped) { // && SPI_I2S_GetFlagStatus(dev_cfg->level.regs, SPI_I2S_FLAG_BSY) == RESET) {
            SPI_Cmd(dev_cfg->level.regs, DISABLE);
            level_stopped = true;
        }

        if (mask_empty && !mask_stopped) { // && SPI_I2S_GetFlagStatus(dev_cfg->mask.regs, SPI_I2S_FLAG_BSY) == RESET) {
            SPI_Cmd(dev_cfg->mask.regs, DISABLE);
            mask_stopped = true;
        }
    }
    SPI_Cmd(dev_cfg->mask.regs, DISABLE);
    SPI_Cmd(dev_cfg->level.regs, DISABLE);
}

/**
 * Stops the pixel clock and ensures it ignores the rising edge.  To be used after a
 * Vsync until the first line is to be displayed
 */
static void stop_pixel_timer()
{
    // This disables the TIM peripheral
    TIM_Cmd(dev_cfg->pixel_timer.timer, DISABLE);
    // This removes the slave mode configuration
    TIM_InternalClockConfig(dev_cfg->pixel_timer.timer);
}
#endif


#endif /* PIOS_INCLUDE_VIDEO */
