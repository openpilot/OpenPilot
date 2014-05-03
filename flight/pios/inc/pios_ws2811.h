/**
 ******************************************************************************
 *
 * @file       pios_ws2811.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      A driver for ws2811 rgb led controller.
 *             this is a plain PiOS port of the very clever solution
 *             implemented by Omri Iluz in the chibios driver here:
 *             https://github.com/omriiluz/WS2812B-LED-Driver-ChibiOS
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
#ifndef PIOS_WS2811_H_
#define PIOS_WS2811_H_

#include <stdint.h>
#include <pios_gpio.h>
#include <pios_stm32.h>
#include <pios_gpio_priv.h>
#include <stm32f4xx.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_dma.h>


#define sign(x) ((x > 0) - (x < 0))
#define PIOS_WS2811_NUMLEDS            2
#define PIOS_WS2811_BUFFER_SIZE        (((PIOS_WS2811_NUMLEDS) * 24))
#define PIOS_WS2811_MEMORYDATASIZE     DMA_MemoryDataSize_HalfWord
#define PIOS_WS2811_PERIPHERALDATASIZE DMA_PeripheralDataSize_HalfWord
#define PIOS_WS2811_TIM_PERIOD         20

#define PIOS_WS2811_DMA_CH1_CONFIG(channel) \
    { \
        .DMA_BufferSize         = PIOS_WS2811_BUFFER_SIZE, \
        .DMA_Channel            = channel, \
        .DMA_DIR = DMA_DIR_MemoryToPeripheral, \
        .DMA_FIFOMode           = DMA_FIFOMode_Enable, \
        .DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull, \
        .DMA_Memory0BaseAddr    = 0, \
        .DMA_MemoryBurst        = DMA_MemoryBurst_INC4, \
        .DMA_MemoryDataSize     = PIOS_WS2811_MEMORYDATASIZE, \
        .DMA_MemoryInc          = DMA_MemoryInc_Enable, \
        .DMA_Mode = DMA_Mode_Circular, \
        .DMA_PeripheralBaseAddr = 0, \
        .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single, \
        .DMA_PeripheralDataSize = PIOS_WS2811_PERIPHERALDATASIZE, \
        .DMA_PeripheralInc      = DMA_PeripheralInc_Disable, \
        .DMA_Priority           = DMA_Priority_VeryHigh, }

#define PIOS_WS2811_DMA_CH2_CONFIG(channel) \
    { \
        .DMA_BufferSize         = 4, \
        .DMA_Channel            = channel, \
        .DMA_DIR = DMA_DIR_MemoryToPeripheral, \
        .DMA_FIFOMode           = DMA_FIFOMode_Enable, \
        .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full, \
        .DMA_Memory0BaseAddr    = 0, \
        .DMA_MemoryBurst        = DMA_MemoryBurst_INC4, \
        .DMA_MemoryDataSize     = PIOS_WS2811_MEMORYDATASIZE, \
        .DMA_MemoryInc          = DMA_MemoryInc_Enable, \
        .DMA_Mode = DMA_Mode_Circular, \
        .DMA_PeripheralBaseAddr = 0, \
        .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single, \
        .DMA_PeripheralDataSize = PIOS_WS2811_PERIPHERALDATASIZE, \
        .DMA_PeripheralInc      = DMA_PeripheralInc_Disable, \
        .DMA_Priority           = DMA_Priority_VeryHigh, }

#define PIOS_WS2811_DMA_UPDATE_CONFIG(channel) \
    { \
        .DMA_BufferSize         = 4, \
        .DMA_Channel            = channel, \
        .DMA_DIR = DMA_DIR_MemoryToPeripheral, \
        .DMA_FIFOMode           = DMA_FIFOMode_Enable, \
        .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full, \
        .DMA_Memory0BaseAddr    = 0, \
        .DMA_MemoryBurst        = DMA_MemoryBurst_INC4, \
        .DMA_MemoryDataSize     = PIOS_WS2811_MEMORYDATASIZE, \
        .DMA_MemoryInc          = DMA_MemoryInc_Enable, \
        .DMA_Mode = DMA_Mode_Circular, \
        .DMA_PeripheralBaseAddr = 0, \
        .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single, \
        .DMA_PeripheralDataSize = PIOS_WS2811_PERIPHERALDATASIZE, \
        .DMA_PeripheralInc      = DMA_PeripheralInc_Disable, \
        .DMA_Priority           = DMA_Priority_High }


typedef uint16_t ledbuf_t;

typedef struct Color Color;
struct Color {
    uint8_t R;
    uint8_t G;
    uint8_t B;
};
struct pios_ws2811_pin_cfg {
    GPIO_TypeDef     *gpio;
    GPIO_InitTypeDef gpioInit;
};
struct pios_ws2811_cfg {
    TIM_TimeBaseInitTypeDef timerInit;
    TIM_TypeDef        *timer;
    uint8_t            timerCh1;
    uint8_t            timerCh2;

    DMA_InitTypeDef    dmaInitCh1;
    DMA_Stream_TypeDef *streamCh1;
    uint32_t           dmaItCh1;

    DMA_InitTypeDef    dmaInitCh2;
    DMA_Stream_TypeDef *streamCh2;
    uint32_t           dmaItCh2;

    DMA_InitTypeDef    dmaInitUpdate;
    DMA_Stream_TypeDef *streamUpdate;
    uint32_t           dmaItUpdate;
    uint16_t           dmaSource;
    struct stm32_irq   irq;
};

void PIOS_WS2811_Init(const struct pios_ws2811_cfg *ws2811_cfg, const struct pios_ws2811_pin_cfg *ws2811_pin_cfg);

void PIOS_WS2811_setColorRGB(Color c, uint8_t led, bool update);
void PIOS_WS2811_Update();

void PIOS_WS2811_DMA_irq_handler();

#endif /* PIOS_WS2811_H_ */
