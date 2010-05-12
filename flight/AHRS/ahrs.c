/**
 ******************************************************************************
 *
 * @file       ahrs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Main AHRS functions
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
#include "ahrs.h"

/* Global Variables */

/* Local Variables */

/* Function Prototypes */

/**
* AHRS Main function
*/
int main()
{
	/* Brings up System using CMSIS functions, enables the LEDs. */
	PIOS_SYS_Init();

	/* Delay system */
	PIOS_DELAY_Init();

	/* Communication system */
	PIOS_COM_Init();

	/* ADC system */
	PIOS_ADC_Init();

	/* Magnetic sensor system */
	#if 0
	PIOS_I2C_Init();
	PIOS_HMC5843_Init();
	#endif

	/* Toggle LED's forever */
	PIOS_LED_On(LED1);

	uint8_t buffer[3] = {0};
	int32_t result;

	for(;;) {
		PIOS_LED_Toggle(LED1);

		// Test ADC
		PIOS_COM_SendFormattedString(COM_USART1, "%d,%d,%d,%d,%d,%d\r", PIOS_ADC_PinGet(0), PIOS_ADC_PinGet(1), PIOS_ADC_PinGet(2), PIOS_ADC_PinGet(3), PIOS_ADC_PinGet(4), PIOS_ADC_PinGet(5));

		#if 0
		result = PIOS_HMC5843_Read(0x0A, buffer, 3);
		PIOS_COM_SendFormattedString(COM_USART1, "Result: %d\r", result);
		PIOS_COM_SendFormattedString(COM_USART1, "Ident: %s\r", buffer);
		#endif

		PIOS_DELAY_WaitmS(25);
	}


	return 0;
}
