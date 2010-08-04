/**
 ******************************************************************************
 *
 * @file       pipbee.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Main PipBee functions
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


/* OpenPilot Includes */
#include "pipbee.h"

/* Global Variables */

/* Local Variables */

/* Function Prototypes */

/**
* PipBee Main function
*/
int main()
{
	uint32_t loop_ctr = 0;
	uint32_t loop_ctr2 = 0;

	// Brings up System using CMSIS functions, enables the LEDs.
	PIOS_SYS_Init();
  
	// Delay system
	PIOS_DELAY_Init();
  
	// Communication system
	PIOS_COM_Init();
  
	// ADC system
	PIOS_ADC_Init();
  
	// SPI link to master
	PIOS_SPI_Init();
   
	// Main loop
	while (1)
	{
		if (++loop_ctr > 10000000)
		{
			loop_ctr = 0;

			if (++loop_ctr2 > 3) loop_ctr2 = 0;

			switch (loop_ctr2)
			{
				case 0:
					PIOS_LED_Toggle(LED1);
					break;
				case 1:
					PIOS_LED_Toggle(LED2);
					break;
				case 2:
					PIOS_LED_Toggle(LED3);
					break;
				case 3:
					PIOS_LED_Toggle(LED4);
					break;
				default:
					loop_ctr2 = 0;
					PIOS_LED_Toggle(LED1);
					break;
			}
		}
	}
  
	return 0;
}
