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


//*****************************************************************************
// OpenPilot Includes

#include "pipbee.h"

//*****************************************************************************
// Global Variables



//*****************************************************************************
// Local Variables



//*****************************************************************************
// Function Prototypes



//*****************************************************************************
// Main function

int main()
{
	// *************

	// Brings up System using CMSIS functions, enables the LEDs.
	PIOS_SYS_Init();

	// Delay system
	PIOS_DELAY_Init();

	// UART communication system
	PIOS_COM_Init();

	// ADC system
	PIOS_ADC_Init();

	// SPI link to master
	PIOS_SPI_Init();

	CLEAR_RTS;

	// *************
	// do a simple visible LED flash sequence for the user

	USB_LED_OFF;
	LINK_LED_OFF;
	RX_LED_OFF;
	TX_LED_OFF;

	for (int i = 0; i < 2; i++)
	{
		USB_LED_TOGGLE;
		PIOS_DELAY_WaitmS(200);
		LINK_LED_TOGGLE;
		PIOS_DELAY_WaitmS(200);
		RX_LED_TOGGLE;
		PIOS_DELAY_WaitmS(200);
		TX_LED_TOGGLE;
		PIOS_DELAY_WaitmS(200);
	}

	// *************
	// Main executive loop

	while (1)
	{
/*		uint32_t loop_ctr;

		if (++loop_ctr >= 10000000)
		{
			loop_ctr = 0;
			PIOS_LED_Toggle(LED2);
		}
*/

		// just sequence the LED's for now
		USB_LED_TOGGLE;
		PIOS_DELAY_WaitmS(200);
		LINK_LED_TOGGLE;
		PIOS_DELAY_WaitmS(200);
		RX_LED_TOGGLE;
		PIOS_DELAY_WaitmS(200);
		TX_LED_TOGGLE;
		PIOS_DELAY_WaitmS(200);

	}

	return 0;
}

//*****************************************************************************
