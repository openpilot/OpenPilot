/**
 ******************************************************************************
 * @addtogroup OpenPilotBL OpenPilot BootLoader
 * @brief These files contain the code to the OpenPilot MB Bootloader.
 *
 * @{
 * @file       main.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      This is the file with the main function of the OpenPilot BootLoader
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
/* Bootloader Includes */
#include <pios.h>
#include <stdbool.h>

#define MAX_WRI_RETRYS 3
/* Prototype of PIOS_Board_Init() function */
extern void PIOS_Board_Init(void);
extern void FLASH_Download();
void error(int);

/* The ADDRESSES of the _binary_* symbols are the important
 * data.  This is non-intuitive for _binary_size where you
 * might expect its value to hold the size but you'd be wrong.
 */
extern void _binary_start;
extern void _binary_end;
extern void _binary_size;
const uint32_t * embedded_image_start = (uint32_t *) &(_binary_start);
const uint32_t * embedded_image_end   = (uint32_t *) &(_binary_end);
const uint32_t   embedded_image_size  = (uint32_t)   &(_binary_size);

int main() {

	PIOS_SYS_Init();
	PIOS_Board_Init();
	PIOS_LED_On(LED1);
	PIOS_DELAY_WaitmS(3000);
	PIOS_LED_Off(LED1);

	/// Self overwrite check
	uint32_t base_address = SCB->VTOR;
	if ((0x08000000 + embedded_image_size) > base_address)
		error(LED1);
	///
	FLASH_Unlock();

	/// Bootloader memory space erase
	uint32_t pageAddress;
	pageAddress = 0x08000000;
	bool fail = false;
	while ((pageAddress < base_address) && (fail == false)) {
		for (int retry = 0; retry < MAX_DEL_RETRYS; ++retry) {
			if (FLASH_ErasePage(pageAddress) == FLASH_COMPLETE) {
				fail = false;
				break;
			} else {
				fail = true;
			}
		}
#ifdef STM32F10X_HD
		pageAddress += 2048;
#elif defined (STM32F10X_MD)
		pageAddress += 1024;
#endif
	}

	if (fail == true)
		error(LED1);


	///

	/// Bootloader programing
	for (uint32_t offset = 0; offset < embedded_image_size/sizeof(uint32_t); ++offset) {
		bool result = false;
		PIOS_LED_Toggle(LED1);
		for (uint8_t retry = 0; retry < MAX_WRI_RETRYS; ++retry) {
			if (result == false) {
				result = (FLASH_ProgramWord(0x08000000 + (offset * 4), embedded_image_start[offset])
						== FLASH_COMPLETE) ? true : false;
			}
		}
		if (result == false)
			error(LED1);
	}
	///
	for (uint8_t x = 0; x < 3; ++x) {
			PIOS_LED_On(LED1);
			PIOS_DELAY_WaitmS(1000);
			PIOS_LED_Off(LED1);
			PIOS_DELAY_WaitmS(1000);
	}

	/// Invalidate the bootloader updater so we won't run
	/// the update again on the next power cycle.
	FLASH_ProgramWord(base_address, 0);
	FLASH_Lock();

	for (;;) {
		PIOS_DELAY_WaitmS(1000);
	}

}

void error(int led) {
	for (;;) {
		PIOS_LED_On(led);
		PIOS_DELAY_WaitmS(500);
		PIOS_LED_Off(led);
		PIOS_DELAY_WaitmS(500);
	}
}
