/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_SERVO RC Servo Functions
 * @brief Code to do set RC servo output
 * @{
 *
 * @file       pios_buzzer.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Buzzer routines (STM32 dependent)
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
#include "pios_buzzer_priv.h"

/* Private Function Prototypes */

uint16_t current_frequency;
uint16_t intervals[12] = { 57334, 54116, 51079, 48212, 45506, 42952, 40541, 38266, 36118, 34091, 32178, 30372 };
/**
* Initialise Servos
*/
void PIOS_Buzzer_Init(void)
{
#if defined(PIOS_INCLUDE_BUZZER)
	
	GPIO_InitTypeDef GPIO_InitStructure = pios_buzzer_cfg.gpio_init;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = pios_buzzer_cfg.tim_base_init;
	TIM_OCInitTypeDef TIM_OCInitStructure = pios_buzzer_cfg.tim_oc_init;

	/* Enable GPIO */
	GPIO_Init(pios_buzzer_cfg.port, &GPIO_InitStructure);
	GPIO_PinAFConfig(pios_buzzer_cfg.port, pios_buzzer_cfg.pin_source, pios_buzzer_cfg.af);

	/* Enable time base */
	TIM_TimeBaseInit(pios_buzzer_cfg.timer,  &TIM_TimeBaseStructure);

	/* Set up for output compare function */
	switch(pios_buzzer_cfg.channel) {
		case TIM_Channel_1:
			TIM_OC1Init(pios_buzzer_cfg.timer, &TIM_OCInitStructure);
			TIM_OC1PreloadConfig(pios_buzzer_cfg.timer, TIM_OCPreload_Enable);
			break;
		case TIM_Channel_2:
			TIM_OC2Init(pios_buzzer_cfg.timer, &TIM_OCInitStructure);
			TIM_OC2PreloadConfig(pios_buzzer_cfg.timer, TIM_OCPreload_Enable);
			break;
		case TIM_Channel_3:
			TIM_OC3Init(pios_buzzer_cfg.timer, &TIM_OCInitStructure);
			TIM_OC3PreloadConfig(pios_buzzer_cfg.timer, TIM_OCPreload_Enable);
			break;
		case TIM_Channel_4:
			TIM_OC4Init(pios_buzzer_cfg.timer, &TIM_OCInitStructure);
			TIM_OC4PreloadConfig(pios_buzzer_cfg.timer, TIM_OCPreload_Enable);
			break;
	}

	TIM_ARRPreloadConfig(pios_buzzer_cfg.timer, ENABLE);

#endif // PIOS_INCLUDE_BUZZER
}

void PIOS_Buzzer_Ctrl(uint8_t enable)
{
	if (enable)
	{
		TIM_Cmd(pios_buzzer_cfg.timer, ENABLE);
	}
	else
	{
		TIM_Cmd(pios_buzzer_cfg.timer, DISABLE);
	}
}

/**
* Set the PWM freq
* \param[in] piano note in numbers (MIDI note numbers)
*/
void PIOS_Buzzer_SetNote(uint8_t note)
{
#if defined(PIOS_INCLUDE_BUZZER)

	uint16_t tim_interval = intervals[note%12] / (1 << note/12);

	TIM_SetAutoreload(pios_buzzer_cfg.timer, tim_interval);

	switch(pios_buzzer_cfg.channel) {
		case TIM_Channel_1:
			TIM_SetCompare1(pios_buzzer_cfg.timer, tim_interval/2);
			break;
		case TIM_Channel_2:
			TIM_SetCompare2(pios_buzzer_cfg.timer, tim_interval/2);
			break;
		case TIM_Channel_3:
			TIM_SetCompare3(pios_buzzer_cfg.timer, tim_interval/2);
			break;
		case TIM_Channel_4:
			TIM_SetCompare4(pios_buzzer_cfg.timer, tim_interval/2);
			break;
	}

#endif // PIOS_INCLUDE_BUZZER
}

