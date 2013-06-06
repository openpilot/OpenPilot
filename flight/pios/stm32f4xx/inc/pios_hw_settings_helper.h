/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_HW_SETTINGS hardware definition helpers
 * @brief STM32F4xx PIOS hardware definition helper macros
 * @{
 *
 * @file       pios_hw_settings_helper.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      hardware definition helper macros
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


#ifndef PIOS_HW_SETTINGS_HELPER_H
#define PIOS_HW_SETTINGS_HELPER_H
/*
 * @brief define a single led item like PIOS_HW_LED_DEFINITION(GPIOA, GPIO_Pin_3)
 */
#define PIOS_HW_LED_DEFINITION(_gpio, _pin) { 		\
        .pin                =      {			\
            .gpio = (_gpio),						\
            .init =      {						\
                .GPIO_Pin   = (_pin),				\
                .GPIO_Speed = GPIO_Speed_2MHz,	\
                .GPIO_Mode  = GPIO_Mode_OUT,	\
                .GPIO_OType = GPIO_OType_PP,	\
                .GPIO_PuPd  = GPIO_PuPd_UP		\
            },									\
        },										\
    }
/*
 * @brief define a single servo channel like "PIOS_HW_SERVO_CHANNEL_DEFINITION(9, 2, GPIOA, 3)"
 */
#define PIOS_HW_SERVO_CHANNEL_DEFINITION(_timNum, _chanNum, _gpio, _pinNum)	{   \
        .timer = TIM##_timNum,							\
        .timer_chan = TIM_Channel_##_chanNum,			\
        .pin   = {										\
            .gpio = (_gpio),							\
            .init = {									\
                .GPIO_Pin   = GPIO_Pin_##_pinNum,		\
                .GPIO_Speed = GPIO_Speed_2MHz,			\
                .GPIO_Mode  = GPIO_Mode_AF,				\
                .GPIO_OType = GPIO_OType_PP,			\
                .GPIO_PuPd  = GPIO_PuPd_UP				\
            },											\
            .pin_source     = GPIO_PinSource##_pinNum,	\
        },												\
        .remap = GPIO_AF_TIM##_timNum,							\
    }


#endif /* PIOS_HW_SETTINGS_HELPER_H */
