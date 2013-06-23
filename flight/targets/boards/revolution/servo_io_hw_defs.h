/**
 ******************************************************************************
 * @file       servo_io_hw_defs.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 * @brief Defines board specific servo and timer I/O pins.
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
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB1_CLOCK / PIOS_SERVO_TIMER_CLOCK) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = ((PIOS_SERVO_TIMER_CLOCK / PIOS_SERVO_UPDATE_HZ) - 1),
    .TIM_RepetitionCounter = 0x0000,
};
static const TIM_TimeBaseInitTypeDef tim_9_10_11_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB2_CLOCK / PIOS_SERVO_TIMER_CLOCK) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = ((PIOS_SERVO_TIMER_CLOCK / PIOS_SERVO_UPDATE_HZ) - 1),
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

// Set up timers that only have inputs on APB1
// TIM2,3,4,5,6,7,12,13,14
static const TIM_TimeBaseInitTypeDef tim_apb1_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB1_CLOCK / PIOS_SERVO_TIMER_CLOCK) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = 0xFFFF,
    .TIM_RepetitionCounter = 0x0000,
};


// Set up timers that only have inputs on APB2
// TIM1,8,9,10,11
static const TIM_TimeBaseInitTypeDef tim_apb2_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB2_CLOCK / PIOS_SERVO_TIMER_CLOCK) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = 0xFFFF,
    .TIM_RepetitionCounter = 0x0000,
};

static const struct pios_tim_clock_cfg tim_1_cfg = {
    .timer = TIM1,
    .time_base_init                            = &tim_apb2_time_base,
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
    .time_base_init                            = &tim_apb1_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM4_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};
static const struct pios_tim_clock_cfg tim_8_cfg = {
    .timer = TIM8,
    .time_base_init                            = &tim_apb2_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM8_CC_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_12_cfg = {
    .timer = TIM12,
    .time_base_init                            = &tim_apb1_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM8_BRK_TIM12_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};


/**
 * Pios servo configuration structures
 * Using TIM3, TIM9, TIM2, TIM5
 */
#include <pios_servo_priv.h>
static const struct pios_tim_channel pios_tim_servoport_all_pins[] = {
    /*                                tim, ch, GPIOX,pin) */
    PIOS_HW_SERVO_CHANNEL_DEFINITION (  3,  3, GPIOB,  0), // 0
    PIOS_HW_SERVO_CHANNEL_DEFINITION (  3,  4, GPIOB,  1), // 1
    PIOS_HW_SERVO_CHANNEL_DEFINITION (  9,  2, GPIOA,  3), // 2
    PIOS_HW_SERVO_CHANNEL_DEFINITION (  2,  3, GPIOA,  2), // 3
    PIOS_HW_SERVO_CHANNEL_DEFINITION (  5,  2, GPIOA,  1), // 4
    PIOS_HW_SERVO_CHANNEL_DEFINITION (  5,  1, GPIOA,  0), // 5

    // PWM pins on FlexiIO(receiver) port
    // *  5: PB14 = SPI2 MISO, TIM12 CH1, USART3 RTS
    PIOS_HW_SERVO_CHANNEL_DEFINITION ( 12,  1, GPIOB, 14), // 6
    // *  6: PB15 = SPI2 MOSI, TIM12 CH2
    PIOS_HW_SERVO_CHANNEL_DEFINITION ( 12,  2, GPIOB, 15), // 7
    // *  7: PC6 = TIM8 CH1, USART6 TX
    PIOS_HW_SERVO_CHANNEL_DEFINITION (  8,  1, GPIOC,  6), // 8
    // *  8: PC7 = TIM8 CH2, USART6 RX
    PIOS_HW_SERVO_CHANNEL_DEFINITION (  8,  2, GPIOC,  7), // 9
    // *  9: PC8 = TIM8 CH3
    PIOS_HW_SERVO_CHANNEL_DEFINITION (  8,  3, GPIOC,  8), //10
    // * 10: PC9 = TIM8 CH4
    PIOS_HW_SERVO_CHANNEL_DEFINITION (  8,  4, GPIOC,  9), //11
};

#define PIOS_SERVOPORT_ALL_PINS_COUNT NELEMENTS(pios_tim_servoport_all_pins)
// Defines the masks used to enable each single input output from pios_tim_servoport_all_pins array
//                                                             31    25   20   15   10    5    0
//                                                              |     |    |    |    |    |    |
#define PIOS_SERVOPORT_ALL_TIMERS_ENABLE_MASK                 0b00000000000000000000111111111111
#define PIOS_SERVOPORT_PPM_IN_ENABLE_MASK                     0b00000000000000000000000001000000
#define PIOS_SERVOPORT_PWM_IN_ENABLE_MASK                     0b00000000000000000000111111000000
#define PIOS_SERVOPORT_PWM_OUTPUTS_ENABLE_MASK                0b00000000000000000000000000111111
#define PIOS_PPMIN_PWM_ALL_OUTPUTS_ENABLE_MASK      (PIOS_SERVOPORT_ALL_TIMERS_ENABLE_MASK & ~PIOS_SERVOPORT_PPM_IN_ENABLE_MASK)

#define PIOS_SERVOPORT_FIRST_INPUT 6
// pin defined as PPM in
#define PIOS_SERVOPORT_PPM_IN &pios_tim_servoport_all_pins[PIOS_SERVOPORT_FIRST_INPUT]
#define PIOS_SERVOPORT_PWM_IN &pios_tim_servoport_all_pins[PIOS_SERVOPORT_FIRST_INPUT]

#include <pios_pwm_priv.h>
const struct pios_servo_cfg pios_servo_cfg_out = {
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
    .num_channels = PIOS_SERVOPORT_ALL_PINS_COUNT,
};

const struct pios_pwm_cfg pios_pwm_cfg = {
    .tim_ic_init         = {
        .TIM_ICPolarity  = TIM_ICPolarity_Rising,
        .TIM_ICSelection = TIM_ICSelection_DirectTI,
        .TIM_ICPrescaler = TIM_ICPSC_DIV1,
        .TIM_ICFilter    = 0x0,
    },
    .channels     = PIOS_SERVOPORT_PWM_IN,
    .num_channels = PIOS_SERVOPORT_ALL_PINS_COUNT - PIOS_SERVOPORT_FIRST_INPUT,
};

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
        .TIM_Channel     = TIM_Channel_1,
    },
    /* Use only the first channel for ppm */
    .channels     = PIOS_SERVOPORT_PPM_IN,
    .num_channels = 1,
};

#endif // PPM

#if defined(PIOS_INCLUDE_GCSRCVR)
#include "pios_gcsrcvr_priv.h"
#endif /* PIOS_INCLUDE_GCSRCVR */

#if defined(PIOS_INCLUDE_RCVR)
#include "pios_rcvr_priv.h"
#endif /* PIOS_INCLUDE_RCVR */

#endif /* SERVO_IO_HW_DEFS_H */
