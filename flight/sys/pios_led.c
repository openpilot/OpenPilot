
/**
 * Project: OpenPilot
 *    
 * @author The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2009.
 *
 * @file pios_led.c
 * LED functions, init, toggle, on & off 
 *    
 * @see The GNU Public License (GPL)
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

GPIO_TypeDef* LED_GPIO_PORT[NUM_LED] = {LED1_GPIO_PORT, LED2_GPIO_PORT};
const uint16_t LED_GPIO_PIN[NUM_LED] = {LED1_GPIO_PIN, LED2_GPIO_PIN};
const uint32_t LED_GPIO_CLK[NUM_LED] = {LED1_GPIO_CLK, LED2_GPIO_CLK};

/* Initialises all the LED's */
void LED_INIT(void)
{
	for(int LEDNum = 0; LEDNum < NUM_LED; LEDNum++)
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd(LED_GPIO_CLK[LEDNum], ENABLE);
		
		GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN[LEDNum];
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		
		GPIO_Init(LED_GPIO_PORT[LEDNum], &GPIO_InitStructure);
	}
}

void LED_ON(Led_TypeDef LEDNum)
{
	LED_GPIO_PORT[LEDNum]->BSRR = LED_GPIO_PIN[LEDNum];
}

void LED_OFF(Led_TypeDef LEDNum)
{
	LED_GPIO_PORT[LEDNum]->BRR = LED_GPIO_PIN[LEDNum];
}

void LED_TOGGLE(Led_TypeDef LEDNum)
{
	LED_GPIO_PORT[LEDNum]->ODR ^= LED_GPIO_PIN[LEDNum];
}
