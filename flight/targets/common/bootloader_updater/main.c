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

#include <pios.h>
#include <pios_board_info.h>
#include <stdbool.h>

#define MAX_WRI_RETRYS 3
/* Prototype of PIOS_Board_Init() function */
extern void PIOS_Board_Init(void);
extern void FLASH_Download();
void error(int, int);

/* The ADDRESSES of the _binary_* symbols are the important
 * data.  This is non-intuitive for _binary_size where you
 * might expect its value to hold the size but you'd be wrong.
 */
extern uint32_t _binary_start;
extern uint32_t _binary_end;
extern uint32_t _binary_size;
const uint32_t * embedded_image_start = (uint32_t *) &(_binary_start);
const uint32_t * embedded_image_end   = (uint32_t *) &(_binary_end);
const uint32_t   embedded_image_size  = (uint32_t)   &(_binary_size);

int main()
{

	PIOS_SYS_Init();
	PIOS_Board_Init();
	PIOS_LED_On(PIOS_LED_HEARTBEAT);
	PIOS_DELAY_WaitmS(3000);
	PIOS_LED_Off(PIOS_LED_HEARTBEAT);

	/// Self overwrite check
	uint32_t base_address = SCB->VTOR;
	if ((0x08000000 + embedded_image_size) > base_address)
		error(PIOS_LED_HEARTBEAT, 1);
	///

	/*
	 * Make sure the bootloader we're carrying is for the same
	 * board type and board revision as the one we're running on.
	 *
	 * Assume the bootloader in flash and the bootloader contained in
	 * the updater both carry a board_info_blob at the end of the image.
	 */

	/* Calculate how far the board_info_blob is from the beginning of the bootloader */
	uint32_t board_info_blob_offset = (uint32_t) &pios_board_info_blob - (uint32_t)0x08000000;

	/* Use the same offset into our embedded bootloader image */
	struct pios_board_info * new_board_info_blob = (struct pios_board_info *)
		((uint32_t)embedded_image_start + board_info_blob_offset);

	/* Compare the two board info blobs to make sure they're for the same HW revision */
	if ((pios_board_info_blob.magic != new_board_info_blob->magic) ||
		(pios_board_info_blob.board_type != new_board_info_blob->board_type) ||
		(pios_board_info_blob.board_rev != new_board_info_blob->board_rev)) {
		error(PIOS_LED_HEARTBEAT, 2);
	}

	/* Embedded bootloader looks like it's the right one for this HW, proceed... */

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
		error(PIOS_LED_HEARTBEAT, 3);


	///
	/// Bootloader programing
	for (uint32_t offset = 0; offset < embedded_image_size / sizeof(uint32_t); ++offset) {
		bool result = false;
		PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
		for (uint8_t retry = 0; retry < MAX_WRI_RETRYS; ++retry) {
			if (result == false) {
				result = (FLASH_ProgramWord(0x08000000 + (offset * 4), embedded_image_start[offset])
					== FLASH_COMPLETE) ? true : false;
			}
		}
		if (result == false)
			error(PIOS_LED_HEARTBEAT, 4);
	}
	///
	for (uint8_t x = 0; x < 3; ++x) {
		PIOS_LED_On(PIOS_LED_HEARTBEAT);
		PIOS_DELAY_WaitmS(1000);
		PIOS_LED_Off(PIOS_LED_HEARTBEAT);
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

void error(int led, int code)
{
	for (;;) {
		PIOS_DELAY_WaitmS(1000);
		for (int x = 0; x < code; x++) {
			PIOS_LED_On(led);
			PIOS_DELAY_WaitmS(200);
			PIOS_LED_Off(led);
			PIOS_DELAY_WaitmS(1000);
		}
		PIOS_DELAY_WaitmS(3000);
		}
}