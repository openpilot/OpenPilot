/**
 ******************************************************************************
 *
 * @file       gpio_in.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      GPIO input functions
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





/* *****************************************************************
// Example pin definitions .. this would go into your pios_board.h file

// GPIO_Mode_AIN           Analog Input
// GPIO_Mode_IN_FLOATING   Input Floating
// GPIO_Mode_IPD           Input Pull-Down
// GPIO_Mode_IPU           Input Pull-up

// API mode line
#define GPIO_IN_0_PORT			GPIOB
#define GPIO_IN_0_PIN			GPIO_Pin_13
#define GPIO_IN_0_MODE			GPIO_Mode_IPU

// Serial port CTS line
#define GPIO_IN_1_PORT			GPIOB
#define GPIO_IN_1_PIN			GPIO_Pin_14
#define GPIO_IN_1_MODE			GPIO_Mode_IPU

#define GPIO_IN_NUM				2
#define GPIO_IN_PORTS			{ GPIO_IN_0_PORT, GPIO_IN_1_PORT     }
#define GPIO_IN_PINS			{ GPIO_IN_0_PIN,  GPIO_IN_1_PIN      }
#define GPIO_IN_MODES			{ GPIO_IN_0_MODE, GPIO_IN_1_MODE     }

#define API_MODE_PIN				0
#define SERIAL_CTS_PIN				1

*********************************************************************
Example usage ..

{



	// setup all the GPIO input pins
	GPIO_IN_Init();




	if (!GPIO_IN(API_MODE_PIN))
	{	// pin is LOW


	}
	else
	{	// pin is HIGH


	}



}

***************************************************************************** */




#include <pios.h>

#include "gpio_in.h"

// *****************************************************************************
// setup the GPIO input pins

// PORT ..
// GPIOA
// GPIOB
// GPIOC
// GPIOD
// GPIOE
// GPIOF
// GPIOG

// PIN ..
// GPIO_Pin_0
// GPIO_Pin_1
// GPIO_Pin_2
// GPIO_Pin_3
// GPIO_Pin_4
// GPIO_Pin_5
// GPIO_Pin_6
// GPIO_Pin_7
// GPIO_Pin_8
// GPIO_Pin_9
// GPIO_Pin_10
// GPIO_Pin_11
// GPIO_Pin_12
// GPIO_Pin_13
// GPIO_Pin_14
// GPIO_Pin_15

// MODE ..
// GPIO_Mode_AIN           Analog Input
// GPIO_Mode_IN_FLOATING   Input Floating
// GPIO_Mode_IPD           Input Pull-Down
// GPIO_Mode_IPU           Input Pull-up

#if defined(GPIO_IN_NUM) && defined(GPIO_IN_PORTS) && defined(GPIO_IN_PINS) && defined(GPIO_IN_MODES)
// #if defined(PIOS_INCLUDE_GPIO_IN) && defined(PIOS_GPIO_IN_NUM) && defined(PIOS_GPIO_IN_PORTS) && defined(PIOS_GPIO_IN_PINS) && defined(PIOS_GPIO_IN_MODES)

	// Local Variables
	static GPIO_TypeDef  *GPIO_IN_PORT[GPIO_IN_NUM] = GPIO_IN_PORTS;
	static const uint32_t GPIO_IN_PIN[GPIO_IN_NUM]  = GPIO_IN_PINS;
	static const uint32_t GPIO_IN_MODE[GPIO_IN_NUM] = GPIO_IN_MODES;

	/**
	* Initialises all the GPIO INPUT's
	*/
	void GPIO_IN_Init(void)
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

		for (int i = 0; i < GPIO_IN_NUM; i++)
		{
			switch ((uint32_t)GPIO_IN_PORT[i])
			{
				case (uint32_t)GPIOA: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); break;
				case (uint32_t)GPIOB: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); break;
				case (uint32_t)GPIOC: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); break;
				case (uint32_t)GPIOD: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); break;
				case (uint32_t)GPIOE: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE); break;
				case (uint32_t)GPIOF: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE); break;
				case (uint32_t)GPIOG: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE); break;
			}
			GPIO_InitStructure.GPIO_Pin = GPIO_IN_PIN[i];
			GPIO_InitStructure.GPIO_Mode = GPIO_IN_MODE[i];
			GPIO_Init(GPIO_IN_PORT[i], &GPIO_InitStructure);
		}
	}

	/**
	* Read input pin level
	* \param[num] Pin Pin Number
	*/
	bool GPIO_IN(uint8_t num)
	{	// return ..
		// FALSE if the input pin is LOW
		// TRUE if the input pin is HIGH
		if (num >= GPIO_IN_NUM) return FALSE;
		return ((GPIO_IN_PORT[num]->IDR & GPIO_IN_PIN[num]) != 0);
	}

#endif

// ***********************************************************************************

/**
  * @}
  * @}
  */
