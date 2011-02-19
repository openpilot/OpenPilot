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
#include <bl_array.h>
#define MAX_WRI_RETRYS 3
/* Prototype of PIOS_Board_Init() function */
extern void PIOS_Board_Init(void);
extern void FLASH_Download();
void error(int);

int main() {

	PIOS_SYS_Init();
	PIOS_Board_Init();
	PIOS_LED_On(LED1);
	PIOS_DELAY_WaitmS(3000);
	PIOS_LED_Off(LED1);

	/// Self overwrite check
	uint32_t base_adress = SCB->VTOR;
	if (0x08000000 + (sizeof(dataArray)) > base_adress)
		error(LED1);
	///
	FLASH_Unlock();

	/// Bootloader memory space erase
	uint32_t pageAdress;
	pageAdress = 0x08000000;
	uint8_t fail = FALSE;
	while ((pageAdress < base_adress) || (fail == TRUE)) {
		for (int retry = 0; retry < MAX_DEL_RETRYS; ++retry) {
			if (FLASH_ErasePage(pageAdress) == FLASH_COMPLETE) {
				fail = FALSE;
				break;
			} else {
				fail = TRUE;
			}
		}
#ifdef STM32F10X_HD
		pageAdress += 2048;
#elif defined (STM32F10X_MD)
		pageAdress += 1024;
#endif
	}

	if (fail == TRUE)
		error(LED1);


	///

	/// Bootloader programing
	for (int x = 0; x < sizeof(dataArray)/sizeof(uint32_t); ++x) {
		int result = 0;
		PIOS_LED_Toggle(LED1);
		for (int retry = 0; retry < MAX_WRI_RETRYS; ++retry) {
			if (result == 0) {
				result = (FLASH_ProgramWord(0x08000000 + (x * 4), dataArray[x])
						== FLASH_COMPLETE) ? 1 : 0;
			}
		}
		if (result == 0)
			error(LED1);
	}
	///
	for (int x=0;x<3;++x) {
			PIOS_LED_On(LED1);
			PIOS_DELAY_WaitmS(1000);
			PIOS_LED_Off(LED1);
			PIOS_DELAY_WaitmS(1000);
		}
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
