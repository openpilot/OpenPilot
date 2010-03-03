/**
 ******************************************************************************
 *
 * @file       pios_servo.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RC Servo routines
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_SERVO RC Servo Functions
 * @{
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

#if !defined(PIOS_DONT_USE_SERVO)


/* Private Function Prototypes */


/* Local Variables */
static volatile uint16_t ServoPosition[PIOS_SERVO_NUM_TIMERS];


/**
* Initialise Servos
*/
void PIOS_Servo_Init(void)
{
	/* Initialise GPIOs as alternate function push/pull */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = PIOS_SERVO_GPIO_PIN_1 | PIOS_SERVO_GPIO_PIN_2 | PIOS_SERVO_GPIO_PIN_3 | PIOS_SERVO_GPIO_PIN_4;
	GPIO_Init(PIOS_SERVO_GPIO_PORT_1TO4, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIOS_SERVO_GPIO_PIN_5 | PIOS_SERVO_GPIO_PIN_6 | PIOS_SERVO_GPIO_PIN_7 | PIOS_SERVO_GPIO_PIN_8;
	GPIO_Init(PIOS_SERVO_GPIO_PORT_5TO8, &GPIO_InitStructure);
	
	/* Initialise RCC Clocks (TIM4 and TIM8) */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);

	/* Initialise Timers TIM4 and TIM8 */
	/* With a resolution of 1uS, period of 20mS (50Hz) */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = ((1000000 / PIOS_SERVO_UPDATE_HZ) - 1);
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

	/* Setup each timer separately */
	TIM_OCInitTypeDef TIM_OCInitStructure;

	/* TIM4 */
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_Pulse = PIOS_SERVOS_INITIAL_POSITION;
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC4Init(TIM4, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM4, ENABLE);
	TIM_CtrlPWMOutputs(TIM4, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
	
	/* TIM8 */
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_Pulse = PIOS_SERVOS_INITIAL_POSITION;
	TIM_OC1Init(TIM8, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
	TIM_OC2Init(TIM8, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);
	TIM_OC3Init(TIM8, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Enable);
	TIM_OC4Init(TIM8, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM8, ENABLE);
	TIM_CtrlPWMOutputs(TIM8, ENABLE);
	TIM_Cmd(TIM8, ENABLE);
}

/**
* Set the servo update rate (Max 500Hz)
* \param[in] onetofour Rate for outputs 1 to 4 (Hz)
* \param[in] fivetoeight Rate for outputs 5 to 8 (Hz)
*/
void PIOS_Servo_SetHz(uint16_t onetofour, uint16_t fivetoeight)
{
	/* (Re)-Initialise Timers TIM4 and TIM8 */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	/* Clipping */
	if(onetofour > 500) {
		onetofour = 500;
	}
	if(fivetoeight > 500) {
		fivetoeight = 500;
	}

	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;
	TIM_TimeBaseStructure.TIM_Period = ((1000000 / onetofour) - 1);
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;
	TIM_TimeBaseStructure.TIM_Period = ((1000000 / fivetoeight) - 1);
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
}

/**
* Set servo position
* \param[in] Servo Servo number (0-7)
* \param[in] Position Servo position in milliseconds
*/
void PIOS_Servo_Set(uint8_t Servo, uint16_t Position)
{
    /* Make sure servo exists */
    if (Servo < PIOS_SERVO_NUM_OUTPUTS && Servo >= 0)
    {
        /* Clip servo position */
        if(Position < Settings.Servos.PositionMin) {
            Position = Settings.Servos.PositionMin;
        }
        if(Position > Settings.Servos.PositionMax) {
            Position = Settings.Servos.PositionMax;
        }
        
        /* Update the position */
        ServoPosition[Servo] = Position;
	
        switch(Servo)
        {
			case 0:
				TIM_SetCompare1(TIM4, Position);
				break;
			case 1:
				TIM_SetCompare2(TIM4, Position);
				break;
			case 2:
				TIM_SetCompare3(TIM4, Position);
				break;
			case 3:
				TIM_SetCompare4(TIM4, Position);
				break;
			case 4:
				TIM_SetCompare1(TIM8, Position);
				break;
			case 5:
				TIM_SetCompare2(TIM8, Position);
				break;
			case 6:
				TIM_SetCompare3(TIM8, Position);
				break;
			case 7:
				TIM_SetCompare4(TIM8, Position);
				break;
        }
    }
}

#endif
