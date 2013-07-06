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

#include "pios_tim_priv.h"

static const TIM_TimeBaseInitTypeDef tim_1_2_3_4_time_base = {
    .TIM_Prescaler         = (PIOS_MASTER_CLOCK / 1000000) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = ((1000000 / PIOS_SERVO_UPDATE_HZ) - 1),
    .TIM_RepetitionCounter = 0x0000,
};

static const struct pios_tim_clock_cfg tim_1_cfg = {
    .timer = TIM1,
    .time_base_init                            = &tim_1_2_3_4_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM1_CC_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_2_cfg = {
    .timer = TIM2,
    .time_base_init                            = &tim_1_2_3_4_time_base,
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
    .time_base_init                            = &tim_1_2_3_4_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM3_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_4_cfg = {
    .timer = TIM4,
    .time_base_init                            = &tim_1_2_3_4_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM4_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_channel pios_tim_rcvrport_all_channels[] = {
        /*                                tim, ch, GPIOX,pin) */
         PIOS_HW_INPUT_CHANNEL_DEFINITION       (  4,  1, GPIOB,  6), // 0
         PIOS_HW_INPUT_CHANNEL_DEFINITION_REMAP (  3,  2, GPIOB,  5, GPIO_PartialRemap_TIM3), //1
         PIOS_HW_INPUT_CHANNEL_DEFINITION       (  3,  3, GPIOB,  0), // 2
         PIOS_HW_INPUT_CHANNEL_DEFINITION       (  3,  4, GPIOB,  1), // 3
         PIOS_HW_INPUT_CHANNEL_DEFINITION       (  2,  1, GPIOA,  0), // 4
         PIOS_HW_INPUT_CHANNEL_DEFINITION       (  2,  2, GPIOA,  1), // 5
};

static const struct pios_tim_channel pios_tim_servoport_all_pins[] = {
         PIOS_HW_SERVO_CHANNEL_DEFINITION       (  4,  4, GPIOB,  9),
         PIOS_HW_SERVO_CHANNEL_DEFINITION       (  4,  3, GPIOB,  8),
         PIOS_HW_SERVO_CHANNEL_DEFINITION       (  4,  2, GPIOB,  7),
         PIOS_HW_SERVO_CHANNEL_DEFINITION       (  1,  1, GPIOA,  8),
         PIOS_HW_SERVO_CHANNEL_DEFINITION_REMAP (  3,  1, GPIOB,  4, GPIO_PartialRemap_TIM3),
         PIOS_HW_SERVO_CHANNEL_DEFINITION       (  2,  3, GPIOA,  2),
};


static const struct pios_tim_channel pios_tim_servoport_rcvrport_pins[] = {
        PIOS_HW_SERVO_CHANNEL_DEFINITION        (  4,  4, GPIOB,  9),
        PIOS_HW_SERVO_CHANNEL_DEFINITION        (  4,  3, GPIOB,  8),
        PIOS_HW_SERVO_CHANNEL_DEFINITION        (  4,  2, GPIOB,  7),
        PIOS_HW_SERVO_CHANNEL_DEFINITION        (  1,  1, GPIOA,  8),
        PIOS_HW_SERVO_CHANNEL_DEFINITION_REMAP  (  3,  1, GPIOB,  4, GPIO_PartialRemap_TIM3),
        PIOS_HW_SERVO_CHANNEL_DEFINITION        (  2,  3, GPIOA,  2),
    // Receiver port pins
    // S3-S6 inputs are used as outputs in this case
        PIOS_HW_SERVO_CHANNEL_DEFINITION        (  3,  3, GPIOB,  0),
        PIOS_HW_SERVO_CHANNEL_DEFINITION        (  3,  4, GPIOB,  1),
        PIOS_HW_SERVO_CHANNEL_DEFINITION        (  2,  1, GPIOA,  0),
        PIOS_HW_SERVO_CHANNEL_DEFINITION        (  2,  2, GPIOA,  1),
};

static const struct pios_tim_channel pios_tim_ppm_flexi_port =
PIOS_HW_INPUT_CHANNEL_DEFINITION_REMAP  (  2,  4, GPIOB, 11, GPIO_PartialRemap2_TIM2);

#endif /* SERVO_IO_HW_DEFS_H_ */
