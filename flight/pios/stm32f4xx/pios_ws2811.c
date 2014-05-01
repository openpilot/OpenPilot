/**
 ******************************************************************************
 *
 * @file       pios_ws2811.c
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
#include <pios.h>

#ifdef PIOS_INCLUDE_WS2811

#include "pios_ws2811.h"
#include <stm32f4xx_rcc.h>
#include <pios_stm32.h>
#include "FreeRTOS.h"
#include "task.h"


// framebuffer
static ledbuf_t *fb = 0;
// bitmask with pin to be set/reset using dma
static ledbuf_t dmaSource[4];

static const struct pios_ws2811_cfg *pios_ws2811_cfg;
static const struct pios_ws2811_pin_cfg *pios_ws2811_pin_cfg;

static void setupTimer();
static void setupDMA();

// generic wrapper around corresponding SPL functions
static void genericTIM_OCxInit(TIM_TypeDef *TIMx, const TIM_OCInitTypeDef *TIM_OCInitStruct, uint8_t ch);
static void genericTIM_OCxPreloadConfig(TIM_TypeDef *TIMx, uint16_t TIM_OCPreload, uint8_t ch);

// timer creates a 1.25 uS signal, with duty cycle controlled by frame buffer values

/* Example configuration fragment for REVOLUTION

   #ifdef PIOS_INCLUDE_WS2811
   #include <pios_ws2811.h>
   #include <hwsettings.h>
   #define PIOS_WS2811_TIM_DIVIDER (PIOS_PERIPHERAL_APB2_CLOCK / (800000 * PIOS_WS2811_TIM_PERIOD))

   // interrupt vector for DMA streamCh1
   void DMA2_Stream1_IRQHandler(void) __attribute__((alias("PIOS_WS2811_irq_handler")));

   const struct pios_ws2811_pin_cfg pios_ws2811_pin_cfg[] = {
    [HWSETTINGS_WS2811LED_OUT_SERVOOUT1] = {
        .gpio     = GPIOB,
        .gpioInit =                        {
            .GPIO_Pin   = GPIO_Pin_0,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
    ....
    [HWSETTINGS_WS2811LED_OUT_FLEXIPIN4] = {
        .gpio     = GPIOB,
        .gpioInit =                        {
            .GPIO_Pin   = GPIO_Pin_13,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
   };

   const struct pios_ws2811_cfg pios_ws2811_cfg = {
    .timer     = TIM1,
    .timerInit = {
        .TIM_Prescaler         = PIOS_WS2811_TIM_DIVIDER - 1,
        .TIM_ClockDivision     = TIM_CKD_DIV1,
        .TIM_CounterMode       = TIM_CounterMode_Up,
        // period (1.25 uS per period
        .TIM_Period                            = PIOS_WS2811_TIM_PERIOD,
        .TIM_RepetitionCounter = 0x0000,
    },

    .timerCh1     = 1,
    .streamCh1    = DMA2_Stream1,
    .timerCh2     = 3,
    .streamCh2    = DMA2_Stream6,
    .streamUpdate = DMA2_Stream5,

    // DMA streamCh1, triggered by timerCh1 pwm signal.
    // if FrameBuffer indicates, reset output value early to indicate "0" bit to ws2812
    .dmaInitCh1 = PIOS_WS2811_DMA_UPDATE_CONFIG(DMA_Channel_6),
    .dmaItCh1   = DMA_IT_TEIF1 | DMA_IT_TCIF1,

    // DMA streamCh2, triggered by timerCh2 pwm signal.
    // Reset output value late to indicate "1" bit to ws2812.
    .dmaInitCh2 = PIOS_WS2811_DMA_CH1_CONFIG(DMA_Channel_6),
    .dmaItCh2   = DMA_IT_TEIF6 | DMA_IT_TCIF6,

    // DMA streamUpdate Triggered by timer update event
    // Outputs a high logic level at beginning of a cycle
    .dmaInitUpdate = PIOS_WS2811_DMA_CH2_CONFIG(DMA_Channel_6),
    .dmaItUpdate   = DMA_IT_TEIF5 | DMA_IT_TCIF5,
    .dmaSource     = TIM_DMA_CC1 | TIM_DMA_CC3 | TIM_DMA_Update,

    // DMA streamCh1 interrupt vector, used to block timer at end of framebuffer transfer
    .irq = {
        .flags = (DMA_IT_TCIF1),
        .init  = {
            .NVIC_IRQChannel    = DMA2_Stream1_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
   };

   void PIOS_WS2811_irq_handler(void)
   {
    PIOS_WS2811_DMA_irq_handler();
   }
   #endif // PIOS_INCLUDE_WS2811

 */

/*
 * How it works:
 * a timer and two channels will produce the timings events:
 * timer period will be 1.25us
 * Ch1 CC event will be raised at 0.40uS from the beginning of the cycle
 * Ch2 CC event will be raised at 0.80uS from the beginning of the cycle
 * At cycle init an Update event will be raised.
 *
 * Three dma streams will handle the output pin as following:
 * - streamUpdate dma stream, triggered by update event will produce a logic 1 on the output pin
 * - streamCh1 will bring the pin to 0 if framebuffer location is set to dmaSource value to send a "0" bit to WS281x
 * - streamCh2 will bring pin to 0 once .8us are passed to send a "1" bit to ws281x
 * Once StreamCh1 has finished to send the buffer the IRQ handler will stop the timer.
 *
 */

/**
 * @brief   Initialize WS2811 Led Driver
 * @details Initialize the Led Driver based on passed configuration
 *
 * @param[in] ws2811_cfg ws2811 driver configuration
 * @param[in] ws2811_pin_cfg pin to be used as output
 *
 */

void PIOS_WS2811_Init(const struct pios_ws2811_cfg *ws2811_cfg, const struct pios_ws2811_pin_cfg *ws2811_pin_cfg)
{
    assert_param(ws2811_cfg);
    assert_param(ws2811_pin_cfg);

    pios_ws2811_pin_cfg = ws2811_pin_cfg;
    pios_ws2811_cfg     = ws2811_cfg;
    GPIO_Init(pios_ws2811_pin_cfg->gpio, &pios_ws2811_pin_cfg->gpioInit);
    for (uint8_t i = 0; i < 4; i++) {
        dmaSource[i] = (ledbuf_t)pios_ws2811_pin_cfg->gpioInit.GPIO_Pin;
    }

    fb = (ledbuf_t *)pvPortMalloc(PIOS_WS2811_BUFFER_SIZE * sizeof(ledbuf_t));
    memset(fb, 0, PIOS_WS2811_BUFFER_SIZE * sizeof(ledbuf_t));
    Color ledoff = { 0, 0, 0 };
    for (uint8_t i = 0; i < PIOS_WS2811_NUMLEDS; i++) {
        PIOS_WS2811_setColorRGB(ledoff, i, false);
    }
    // Setup timers
    setupTimer();
    setupDMA();
}

void setupTimer()
{
    // Stop timer
    TIM_Cmd(pios_ws2811_cfg->timer, DISABLE);
    // Configure timebase and internal clock
    TIM_TimeBaseInit(pios_ws2811_cfg->timer, &pios_ws2811_cfg->timerInit);
    TIM_InternalClockConfig(pios_ws2811_cfg->timer);

    genericTIM_OCxPreloadConfig(pios_ws2811_cfg->timer, TIM_OCPreload_Enable, pios_ws2811_cfg->timerCh1);
    genericTIM_OCxPreloadConfig(pios_ws2811_cfg->timer, TIM_OCPreload_Enable, pios_ws2811_cfg->timerCh2);
    TIM_ARRPreloadConfig(pios_ws2811_cfg->timer, ENABLE);

    // enable outputs
    // TIM_CtrlPWMOutputs(pios_ws2811_cfg->timer, ENABLE);

    TIM_DMACmd(pios_ws2811_cfg->timer, pios_ws2811_cfg->dmaSource, ENABLE);

    TIM_OCInitTypeDef oc = {
        .TIM_OCMode       = TIM_OCMode_PWM1,
        .TIM_OutputState  = TIM_OutputState_Enable,
        .TIM_OutputNState = TIM_OutputNState_Disable,
        .TIM_Pulse        = 0,
        .TIM_OCPolarity   = TIM_OCPolarity_High,
        .TIM_OCNPolarity  = TIM_OCNPolarity_High,
        .TIM_OCIdleState  = TIM_OCIdleState_Reset,
        .TIM_OCNIdleState = TIM_OCNIdleState_Reset,
    };

    // (duty in ticks) / (period in ticks) * 1.25uS (period in S) = 0.40 uS
    oc.TIM_Pulse = 40 * PIOS_WS2811_TIM_PERIOD / 125;
    genericTIM_OCxInit(pios_ws2811_cfg->timer, &oc, pios_ws2811_cfg->timerCh1);
    // (duty in ticks) / (period in ticks) * 1.25uS (period in S) = 0.80 uS
    oc.TIM_Pulse = 80 * PIOS_WS2811_TIM_PERIOD / 125;
    genericTIM_OCxInit(pios_ws2811_cfg->timer, &oc, pios_ws2811_cfg->timerCh2);
}

void genericTIM_OCxInit(TIM_TypeDef *TIMx, const TIM_OCInitTypeDef *TIM_OCInitStruct, uint8_t ch)
{
    switch (ch) {
    case 1:
        TIM_OC1Init(TIMx, TIM_OCInitStruct);
        break;
    case 2:
        TIM_OC2Init(TIMx, TIM_OCInitStruct);
        break;
    case 3:
        TIM_OC3Init(TIMx, TIM_OCInitStruct);
        break;
    case 4:
        TIM_OC4Init(TIMx, TIM_OCInitStruct);
        break;
    }
}

void genericTIM_OCxPreloadConfig(TIM_TypeDef *TIMx, uint16_t TIM_OCPreload, uint8_t ch)
{
    switch (ch) {
    case 1:
        TIM_OC1PreloadConfig(TIMx, TIM_OCPreload);
        break;
    case 2:
        TIM_OC2PreloadConfig(TIMx, TIM_OCPreload);
        break;
    case 3:
        TIM_OC3PreloadConfig(TIMx, TIM_OCPreload);
        break;
    case 4:
        TIM_OC4PreloadConfig(TIMx, TIM_OCPreload);
        break;
    }
}


void setupDMA()
{
    // Configure Ch1
    DMA_Init(pios_ws2811_cfg->streamCh1, (DMA_InitTypeDef *)&pios_ws2811_cfg->dmaInitCh1);
    pios_ws2811_cfg->streamCh1->PAR  = (uint32_t)&pios_ws2811_pin_cfg->gpio->BSRRH;
    pios_ws2811_cfg->streamCh1->M0AR = (uint32_t)fb;

    NVIC_Init((NVIC_InitTypeDef *)&(pios_ws2811_cfg->irq.init));
    DMA_ITConfig(pios_ws2811_cfg->streamCh1, DMA_IT_TC, ENABLE);


    DMA_Init(pios_ws2811_cfg->streamCh2, (DMA_InitTypeDef *)&pios_ws2811_cfg->dmaInitCh2);
    pios_ws2811_cfg->streamCh2->PAR     = (uint32_t)&pios_ws2811_pin_cfg->gpio->BSRRH;
    pios_ws2811_cfg->streamCh2->M0AR    = (uint32_t)dmaSource;

    DMA_Init(pios_ws2811_cfg->streamUpdate, (DMA_InitTypeDef *)&pios_ws2811_cfg->dmaInitUpdate);
    pios_ws2811_cfg->streamUpdate->PAR  = (uint32_t)&pios_ws2811_pin_cfg->gpio->BSRRL;
    pios_ws2811_cfg->streamUpdate->M0AR = (uint32_t)dmaSource;

    DMA_ClearITPendingBit(pios_ws2811_cfg->streamCh1, pios_ws2811_cfg->dmaItCh1);
    DMA_ClearITPendingBit(pios_ws2811_cfg->streamCh2, pios_ws2811_cfg->dmaItCh2);
    DMA_ClearITPendingBit(pios_ws2811_cfg->streamUpdate, pios_ws2811_cfg->dmaItUpdate);

    DMA_Cmd(pios_ws2811_cfg->streamCh2, ENABLE);
    DMA_Cmd(pios_ws2811_cfg->streamCh1, ENABLE);
    DMA_Cmd(pios_ws2811_cfg->streamUpdate, ENABLE);
}

void setColor(uint8_t color, ledbuf_t *buf)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        buf[i] = ((color << i) & 0b10000000 ? 0x0 : dmaSource[0]);
    }
}

/**
 * Set a led color
 * @param c color
 * @param led led number
 * @param update Perform an update after changing led color
 */
void PIOS_WS2811_setColorRGB(Color c, uint8_t led, bool update)
{
    if (led >= PIOS_WS2811_NUMLEDS) {
        return;
    }
    setColor(c.G, fb + (led * 24));
    setColor(c.R, fb + 8 + (led * 24));
    setColor(c.B, fb + 16 + (led * 24));
    if (update) {
        PIOS_WS2811_Update();
    }
}

/**
 * trigger an update cycle if not already running
 */
void PIOS_WS2811_Update()
{
    // does not start if framebuffer is not allocated (init has not been called yet) or a transfer is still on going
    if (!fb || (pios_ws2811_cfg->timer->CR1 & TIM_CR1_CEN)) {
        return;
    }

    // reset counters for synchronization
    pios_ws2811_cfg->timer->CNT = PIOS_WS2811_TIM_PERIOD - 1;
    // Start a new cycle
    TIM_Cmd(pios_ws2811_cfg->timer, ENABLE);
}

/**
 * Stop timer once the complete framebuffer has been sent
 */

void PIOS_WS2811_DMA_irq_handler()
{
    TIM_Cmd(pios_ws2811_cfg->timer, DISABLE);
    DMA_ClearFlag(pios_ws2811_cfg->streamCh1, pios_ws2811_cfg->irq.flags);
}

#endif // PIOS_INCLUDE_WS2811
