/**
 ******************************************************************************
 *
 * @file       pios_sdcard.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      Sets up basic system hardware, functions are called from Main.
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_SDCARD SDCard Functions
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

/* Private Function Prototypes */


/**
* Initializes all system peripherals
*/
void PIOS_SDCARD_Init(void)
{
	/* File system object for each logical drive */
	static FATFS Fatfs[_DRIVES];
	
	/* Initialize FatFS disk */
	if(f_mount(0, &Fatfs[0]) != FR_OK) {
		/* Failed to mount MicroSD filesystem, flash LED1 forever */
		while(1) {
			for(int i = 0; i < 100000; i++);
			PIOS_LED_Toggle(LED1);
		}
	}		
}
