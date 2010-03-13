/**
 ******************************************************************************
 *
 * @file       openpilot_bl.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Main OpenPilot Bootloader Function
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
#include "openpilot_bl.h"

/* Global Variables */
static pFunction Jump_To_Application;
static uint32_t JumpAddress;


/**
* OpenPilot Bootloader Main function
*/
int main()
{
	/* Brings up System using CMSIS functions, enables the LEDs. */
	PIOS_SYS_Init();

	/* Initialise LED's */
	PIOS_LED_Off(LED1);
	PIOS_LED_Off(LED2);

	/* Only go into bootloader when the USB cable is connected */
	if(PIOS_USB_CableConnected()) {
		/* Delay system */
		PIOS_DELAY_Init();

		if(PIOS_USB_IsInitialized()) {
			/* Initialise the USB system */
			PIOS_USB_Init(0);
		}

		/* Initialise COM Ports */
		PIOS_COM_Init();

		/* Execute the IAP driver in order to re-program the Flash */
		StartBootloader();
	}

	/* Test if user code is programmed starting from address "ApplicationAddress" */
	if(((*(volatile uint32_t*) ApplicationAddress) & 0x2FFE0000) == 0x20000000) {
		/* Jump to user application */
		JumpAddress = *(volatile uint32_t*) (ApplicationAddress + 4);
		Jump_To_Application = (pFunction) JumpAddress;

		/* Initialise user application's Stack Pointer */
		__set_MSP(*(volatile uint32_t*) ApplicationAddress);
		Jump_To_Application();
	}

	/* Loop for ever if no application code is not programmed */
	PIOS_LED_Off(LED1);
	PIOS_LED_Off(LED2);
	for(;;) {
		PIOS_LED_Toggle(LED1);
		PIOS_LED_Toggle(LED2);
		PIOS_DELAY_WaitmS(50);
	}

	return 0;
}

