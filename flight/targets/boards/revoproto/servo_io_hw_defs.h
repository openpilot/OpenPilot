/**
 ******************************************************************************
 *
 * @file       servo_io_hw_defs.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      brief goes here. 
 *             --
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

#ifndef SERVO_IO_HW_DEFS_H
#define SERVO_IO_HW_DEFS_H


#include "pios_tim_priv.h"

static const TIM_TimeBaseInitTypeDef tim_2_3_5_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB1_CLOCK / 1000000) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = ((1000000 / PIOS_SERVO_UPDATE_HZ) - 1),
    .TIM_RepetitionCounter = 0x0000,
};

static const TIM_TimeBaseInitTypeDef tim_9_10_11_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB2_CLOCK / 1000000) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = ((1000000 / PIOS_SERVO_UPDATE_HZ) - 1),
    .TIM_RepetitionCounter = 0x0000,
};

// Set up timers that only have inputs on APB2
static const TIM_TimeBaseInitTypeDef tim_1_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB2_CLOCK / 1000000) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = 0xFFFF,
    .TIM_RepetitionCounter = 0x0000,
};

static const TIM_TimeBaseInitTypeDef tim_8_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB2_CLOCK / 1000000) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = 0xFFFF,
    .TIM_RepetitionCounter = 0x0000,
};

// Set up timers that only have inputs on APB1
static const TIM_TimeBaseInitTypeDef tim_4_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB1_CLOCK / 1000000) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = 0xFFFF,
    .TIM_RepetitionCounter = 0x0000,
};


static const struct pios_tim_clock_cfg tim_2_cfg = {
    .timer = TIM2,
    .time_base_init                            = &tim_2_3_5_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM2_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};


static const struct pios_tim_clock_cfg tim_3_cfg = {
    .timer = TIM3,
    .time_base_init                            = &tim_2_3_5_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM3_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};


static const struct pios_tim_clock_cfg tim_5_cfg = {
    .timer = TIM5,
    .time_base_init                            = &tim_2_3_5_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM5_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_8_cfg = {
    .timer = TIM8,
    .time_base_init                            = &tim_8_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM8_CC_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_9_cfg = {
    .timer = TIM9,
    .time_base_init                            = &tim_9_10_11_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM1_BRK_TIM9_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_10_cfg = {
    .timer = TIM10,
    .time_base_init                            = &tim_9_10_11_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM1_UP_TIM10_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_11_cfg = {
    .timer = TIM11,
    .time_base_init                            = &tim_9_10_11_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM1_TRG_COM_TIM11_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_1_cfg = {
    .timer = TIM1,
    .time_base_init                            = &tim_1_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM1_CC_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_4_cfg = {
    .timer = TIM4,
    .time_base_init                            = &tim_4_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM4_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

/**
 * Pios servo configuration structures
 */
#include <pios_servo_priv.h>
static const struct pios_tim_channel pios_tim_servoport_all_pins[] = {
        /*                                tim, ch, GPIOX,pin) */
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  9,  1, GPIOE,  5), // 0
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  9,  2, GPIOE,  6), // 1
        PIOS_HW_SERVO_CHANNEL_DEFINITION ( 11,  1, GPIOB,  9), // 2
        PIOS_HW_SERVO_CHANNEL_DEFINITION ( 10,  1, GPIOB,  8), // 3
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  5,  3, GPIOA,  2), // 4
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  5,  4, GPIOA,  3), // 5
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  3,  3, GPIOB,  0), // 6
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  3,  4, GPIOB,  1), // 7

        PIOS_HW_SERVO_CHANNEL_DEFINITION (  2,  2, GPIOB,  3), // LED1
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  3,  1, GPIOB,  4), // LED2
};

const struct pios_servo_cfg pios_servo_cfg = {
    .tim_oc_init          = {
        .TIM_OCMode       = TIM_OCMode_PWM1,
        .TIM_OutputState  = TIM_OutputState_Enable,
        .TIM_OutputNState = TIM_OutputNState_Disable,
        .TIM_Pulse        = PIOS_SERVOS_INITIAL_POSITION,
        .TIM_OCPolarity   = TIM_OCPolarity_High,
        .TIM_OCNPolarity  = TIM_OCPolarity_High,
        .TIM_OCIdleState  = TIM_OCIdleState_Reset,
        .TIM_OCNIdleState = TIM_OCNIdleState_Reset,
    },
    .channels     = pios_tim_servoport_all_pins,
    .num_channels = NELEMENTS(pios_tim_servoport_all_pins),
};


/*
 * PWM Inputs
 */
#if defined(PIOS_INCLUDE_PWM) || defined(PIOS_INCLUDE_PPM)
#include <pios_pwm_priv.h>
static const struct pios_tim_channel pios_tim_rcvrport_all_channels[] = {
        /*                                tim, ch, GPIOX,pin) */
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  4,  4, GPIOD, 15), // 0
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  4,  3, GPIOD, 14), // 1
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  4,  2, GPIOD, 13), // 2
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  4,  1, GPIOD, 12), // 3
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  1,  4, GPIOE, 14), // 4
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  1,  3, GPIOE, 13), // 5
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  1,  2, GPIOE, 11), // 6
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  1,  1, GPIOE,  9), // 7
};

const struct pios_pwm_cfg pios_pwm_cfg = {
    .tim_ic_init         = {
        .TIM_ICPolarity  = TIM_ICPolarity_Rising,
        .TIM_ICSelection = TIM_ICSelection_DirectTI,
        .TIM_ICPrescaler = TIM_ICPSC_DIV1,
        .TIM_ICFilter    = 0x0,
    },
    .channels     = pios_tim_rcvrport_all_channels,
    .num_channels = NELEMENTS(pios_tim_rcvrport_all_channels),
};
#endif /* if defined(PIOS_INCLUDE_PWM) || defined(PIOS_INCLUDE_PPM) */

/*
 * PPM Input
 */
#if defined(PIOS_INCLUDE_PPM)
#include <pios_ppm_priv.h>
static const struct pios_ppm_cfg pios_ppm_cfg = {
    .tim_ic_init         = {
        .TIM_ICPolarity  = TIM_ICPolarity_Rising,
        .TIM_ICSelection = TIM_ICSelection_DirectTI,
        .TIM_ICPrescaler = TIM_ICPSC_DIV1,
        .TIM_ICFilter    = 0x0,
        .TIM_Channel     = TIM_Channel_2,
    },
    /* Use only the first channel for ppm */
    .channels     = &pios_tim_rcvrport_all_channels[0],
    .num_channels = 1,
};

#endif // PPM

#if defined(PIOS_INCLUDE_GCSRCVR)
#include "pios_gcsrcvr_priv.h"
#endif /* PIOS_INCLUDE_GCSRCVR */

#if defined(PIOS_INCLUDE_RCVR)
#include "pios_rcvr_priv.h"
#endif /* PIOS_INCLUDE_RCVR */


/*
 * SONAR Inputs
 */
#if defined(PIOS_INCLUDE_HCSR04)
#include <pios_hcsr04_priv.h>

static const struct pios_tim_channel pios_tim_hcsr04_port_all_channels[] = {
        /*                                tim, ch, GPIOX,pin) */
        PIOS_HW_SERVO_CHANNEL_DEFINITION (  8,  3, GPIOC,  8), // 0
};

const struct pios_hcsr04_cfg pios_hcsr04_cfg = {
    .tim_ic_init         = {
        .TIM_ICPolarity  = TIM_ICPolarity_Rising,
        .TIM_ICSelection = TIM_ICSelection_DirectTI,
        .TIM_ICPrescaler = TIM_ICPSC_DIV1,
        .TIM_ICFilter    = 0x0,
    },
    .channels     = pios_tim_hcsr04_port_all_channels,
    .num_channels = NELEMENTS(pios_tim_hcsr04_port_all_channels),
    .trigger             = {
        .gpio = GPIOD,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP,
            .GPIO_Speed = GPIO_Speed_2MHz,
        },
    },
};
#endif /* if defined(PIOS_INCLUDE_HCSR04) */



#endif /* SERVO_IO_HW_DEFS_H */
