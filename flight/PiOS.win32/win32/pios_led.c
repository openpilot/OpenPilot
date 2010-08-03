/**
 ******************************************************************************
 *
 * @file       pios_led.c   
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      LED functions, init, toggle, on & off.
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_LED LED Functions
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

#if defined(PIOS_INCLUDE_LED)

/* Private Function Prototypes */


/* Local Variables */
//static GPIO_TypeDef* LED_GPIO_PORT[PIOS_LED_NUM] = PIOS_LED_PORTS;
//static const uint32_t LED_GPIO_PIN[PIOS_LED_NUM] = PIOS_LED_PINS;
//static const uint32_t LED_GPIO_CLK[PIOS_LED_NUM] = PIOS_LED_CLKS;
static uint8_t LED_GPIO[PIOS_LED_NUM];


static inline void PIOS_SetLED(LedTypeDef LED,uint8_t stat) {
	printf("PIOS: LED %i status %i\n",LED,stat);
	LED_GPIO[LED]=stat;
}

/**
* Initialises all the LED's
*/
void PIOS_LED_Init(void)
{
	//GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	for(int LEDNum = 0; LEDNum < PIOS_LED_NUM; LEDNum++) {
		//RCC_APB2PeriphClockCmd(LED_GPIO_CLK[LEDNum], ENABLE);
		//GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN[LEDNum];
		//GPIO_Init(LED_GPIO_PORT[LEDNum], &GPIO_InitStructure);

		/* LED's Off */
		//LED_GPIO_PORT[LEDNum]->BSRR = LED_GPIO_PIN[LEDNum];
		LED_GPIO[LEDNum]=0;
	}
}


/**
* Turn on LED
* \param[in] LED LED Name (LED1, LED2)
*/
void PIOS_LED_On(LedTypeDef LED)
{
	//LED_GPIO_PORT[LED]->BRR = LED_GPIO_PIN[LED];
	PIOS_SetLED(LED,1);
}


/**
* Turn off LED
* \param[in] LED LED Name (LED1, LED2)
*/
void PIOS_LED_Off(LedTypeDef LED)
{
	//LED_GPIO_PORT[LED]->BSRR = LED_GPIO_PIN[LED];
	PIOS_SetLED(LED,0);
}


/**
* Toggle LED on/off
* \param[in] LED LED Name (LED1, LED2)
*/
void PIOS_LED_Toggle(LedTypeDef LED)
{
	//LED_GPIO_PORT[LED]->ODR ^= LED_GPIO_PIN[LED];
	PIOS_SetLED(LED,LED_GPIO[LED]?0:1);
}

#endif
