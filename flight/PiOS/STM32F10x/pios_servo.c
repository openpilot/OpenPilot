/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_SERVO RC Servo Functions
 * @brief Code to do set RC servo output
 * @{
 *
 * @file       pios_servo.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RC Servo routines (STM32 dependent)
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

/* Project Includes */
#include "pios.h"
#include "pios_servo_priv.h"

/* Private Function Prototypes */

/**
* Initialise Servos
*/
void PIOS_Servo_Init(void)
{
#ifndef PIOS_ENABLE_DEBUG_PINS
#if defined(PIOS_INCLUDE_SERVO)
	
	
	for (uint8_t i = 0; i < pios_servo_cfg.num_channels; i++) {
		GPIO_InitTypeDef GPIO_InitStructure = pios_servo_cfg.gpio_init;
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = pios_servo_cfg.tim_base_init;
		TIM_OCInitTypeDef TIM_OCInitStructure = pios_servo_cfg.tim_oc_init;

		struct pios_servo_channel channel = pios_servo_cfg.channels[i];
		
		/* Enable appropriate clock to timer module */
		switch((int32_t) channel.timer) {
			case (int32_t)TIM1:
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
				break;
			case (int32_t)TIM2:
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
				break;
			case (int32_t)TIM3:
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
				break;
			case (int32_t)TIM4:
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
				break;
			case (int32_t)TIM5:
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
				break;
			case (int32_t)TIM6:
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
				break;
			case (int32_t)TIM7:
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
				break;
			case (int32_t)TIM8:
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
				break;
		}

		/* Enable GPIO */
		GPIO_InitStructure.GPIO_Pin = channel.pin;
		GPIO_Init(channel.port, &GPIO_InitStructure);
		
		/* Enable time base */
		TIM_TimeBaseInit(channel.timer,  &TIM_TimeBaseStructure);
		
		channel.timer->PSC = (PIOS_MASTER_CLOCK / 1000000) - 1;
				
		/* Set up for output compare function */
		switch(channel.channel) {
			case TIM_Channel_1:
				TIM_OC1Init(channel.timer, &TIM_OCInitStructure);
				TIM_OC1PreloadConfig(channel.timer, TIM_OCPreload_Enable);
				break;
			case TIM_Channel_2:
				TIM_OC2Init(channel.timer, &TIM_OCInitStructure);
				TIM_OC2PreloadConfig(channel.timer, TIM_OCPreload_Enable);
				break;
			case TIM_Channel_3:
				TIM_OC3Init(channel.timer, &TIM_OCInitStructure);
				TIM_OC3PreloadConfig(channel.timer, TIM_OCPreload_Enable);
				break;
			case TIM_Channel_4:
				TIM_OC4Init(channel.timer, &TIM_OCInitStructure);
				TIM_OC4PreloadConfig(channel.timer, TIM_OCPreload_Enable);
				break;
		}
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		TIM_ARRPreloadConfig(channel.timer, ENABLE);
		TIM_CtrlPWMOutputs(channel.timer, ENABLE);
		TIM_Cmd(channel.timer, ENABLE);		
		
	}	
	
	if(pios_servo_cfg.remap) {
		/* Warning, I don't think this will work for multiple remaps at once */
		GPIO_PinRemapConfig(pios_servo_cfg.remap, ENABLE);
	}
	

#endif // PIOS_INCLUDE_SERVO
#endif // PIOS_ENABLE_DEBUG_PINS
}

/**
* Set the servo update rate (Max 500Hz)
* \param[in] array of rates in Hz
* \param[in] maximum number of banks
*/
void PIOS_Servo_SetHz(uint16_t * speeds, uint8_t banks)
{
#ifndef PIOS_ENABLE_DEBUG_PINS
#if defined(PIOS_INCLUDE_SERVO)
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = pios_servo_cfg.tim_base_init;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;
	
	uint8_t set = 0;
	
	for(uint8_t i = 0; (i < pios_servo_cfg.num_channels) && (set < banks); i++) {		
		bool new = true;
		struct pios_servo_channel channel = pios_servo_cfg.channels[i];
		
		/* See if any previous channels use that same timer */
		for(uint8_t j = 0; (j < i) && new; j++) 
			new &= channel.timer != pios_servo_cfg.channels[j].timer;
		
		if(new) {
			TIM_TimeBaseStructure.TIM_Period = ((1000000 / speeds[set]) - 1);		
			TIM_TimeBaseInit(channel.timer,  &TIM_TimeBaseStructure);
			set++;
		}
	}
#endif // PIOS_INCLUDE_SERVO
#endif // PIOS_ENABLE_DEBUG_PINS
}

/**
* Set servo position
* \param[in] Servo Servo number (0-7)
* \param[in] Position Servo position in milliseconds
*/
void PIOS_Servo_Set(uint8_t Servo, uint16_t Position)
{
#ifndef PIOS_ENABLE_DEBUG_PINS
#if defined(PIOS_INCLUDE_SERVO)
	/* Make sure servo exists */
	if (Servo < pios_servo_cfg.num_channels && Servo >= 0) {
		/* Update the position */

		switch(pios_servo_cfg.channels[Servo].channel) {
			case TIM_Channel_1:
				TIM_SetCompare1(pios_servo_cfg.channels[Servo].timer, Position);
				break;
			case TIM_Channel_2:
				TIM_SetCompare2(pios_servo_cfg.channels[Servo].timer, Position);
				break;
			case TIM_Channel_3:
				TIM_SetCompare3(pios_servo_cfg.channels[Servo].timer, Position);
				break;
			case TIM_Channel_4:
				TIM_SetCompare4(pios_servo_cfg.channels[Servo].timer, Position);
				break;
		}	 	
	}
#endif // PIOS_INCLUDE_SERVO
#endif // PIOS_ENABLE_DEBUG_PINS
}
