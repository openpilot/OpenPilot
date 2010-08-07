/**
 ******************************************************************************
 *
 * @file       bootloader.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Bootloader functions header
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

/* Includes */
#include "openpilot_bl.h"

/* Local Functions */
static void Serial_PutString(uint8_t *s);
static void FLASH_DisableWriteProtectionPages(void);
static void SerialDownload(void);

/* Global Variables */
extern uint32_t FlashDestination;
extern uint8_t file_name[FILE_NAME_LENGTH];

/* Local variables */
static uint32_t UserMemoryMask = 0;

/**
 * Main bootloader function
 */
void StartBootloader(void)
{
	uint8_t key = 0;
	uint32_t BlockNbr = 0;
	bool FlashProtection = FALSE;

	/* Get the number of block (4 or 2 pages) from where the user program will be loaded */
	BlockNbr = (FlashDestination - 0x08000000) >> 12;

	/* Compute the mask to test if the Flash memory, where the user program will be loaded, is write protected */
	if(BlockNbr < 62) {
		UserMemoryMask = ((uint32_t) ~((1 << BlockNbr) - 1));
	} else {
		UserMemoryMask = ((uint32_t) 0x80000000);
	}

	/* Test if any page of Flash memory where program user will be loaded is write protected */
	if((FLASH_GetWriteProtectionOptionByte() & UserMemoryMask) != UserMemoryMask) {
		FlashProtection = TRUE;
		SerialPutString("Download Image To the STM32F10x Internal Flash ------- 1\r\n");
		SerialPutString("Execute The New Program ------------------------------ 2\r\n");
		SerialPutString("Disable the write protection ------------------------- 3\r\n");
	} else {
		FlashProtection = FALSE;
		SerialPutString("Download Image To the STM32F10x Internal Flash ------- 1\r\n");
		SerialPutString("Execute The New Program ------------------------------ 2\r\n");
	}

	/* Loop through 1mS check for specified time */
	for(int32_t i = 0; i < OPBL_MAGIC_TIMEOUT; i++) {
		uint16_t start = PIOS_DELAY_TIMER->CNT;

		/* Check for magic character for 1mS */
		while((volatile uint16_t)(PIOS_DELAY_TIMER->CNT - start) <= 1000) {
			/* Check to see if there is anything on the receiving buffer */
			if(PIOS_COM_ReceiveBufferUsed(OPBL_COM_PORT) > 0) {
				key = PIOS_COM_ReceiveBuffer(OPBL_COM_PORT);
				if(key == 0x31) {
					/* Flash unlock */
					FLASH_Unlock();

					/* Download user application in the Flash */
					SerialDownload();
					return;
				} else if ((key == 0x33) && (FlashProtection == TRUE)) {
					/* Disable the write protection of desired pages */
					FLASH_DisableWriteProtectionPages();
				}
			}
		}
	}

	return;
}

/**
 * Calculate the number of pages
 * \param[in] Size The image size
 * \return The number of pages
 */
uint32_t FLASH_PagesMask(__IO uint32_t Size)
{
	uint32_t pagenumber = 0x0;
	uint32_t size = Size;

	if((size % PAGE_SIZE) != 0) {
		pagenumber = (size / PAGE_SIZE) + 1;
	} else {
		pagenumber = size / PAGE_SIZE;
	}
	return pagenumber;

}

/**
 * Print a string on the HyperTerminal
 * \param[in] s The string to be printed
 */
static void Serial_PutString(uint8_t *s)
{
	while(*s != '\0') {
		PIOS_COM_SendChar(OPBL_COM_PORT, *s);
		s++;
	}
}

/**
 * Disable the write protection of desired pages
 */
static void FLASH_DisableWriteProtectionPages(void)
{
	uint32_t useroptionbyte = 0, WRPR = 0;
	uint16_t var1 = OB_IWDG_SW, var2 = OB_STOP_NoRST, var3 = OB_STDBY_NoRST;
	FLASH_Status status = FLASH_BUSY;

	WRPR = FLASH_GetWriteProtectionOptionByte();

	/* Test if user memory is write protected */
	if((WRPR & UserMemoryMask) != UserMemoryMask) {
		useroptionbyte = FLASH_GetUserOptionByte();

		UserMemoryMask |= WRPR;

		status = FLASH_EraseOptionBytes();

		if(UserMemoryMask != 0xFFFFFFFF) {
			status = FLASH_EnableWriteProtection(
					(uint32_t) ~UserMemoryMask);
		}

		/* Test if user Option Bytes are programmed */
		if((useroptionbyte & 0x07) != 0x07) { /* Restore user Option Bytes */
			if((useroptionbyte & 0x01) == 0x0) {
				var1 = OB_IWDG_HW;
			}
			if((useroptionbyte & 0x02) == 0x0) {
				var2 = OB_STOP_RST;
			}
			if((useroptionbyte & 0x04) == 0x0) {
				var3 = OB_STDBY_RST;
			}

			FLASH_UserOptionByteConfig(var1, var2, var3);
		}

		if(status == FLASH_COMPLETE) {
			SerialPutString("Write Protection disabled...\r\n");

			SerialPutString("...and a System Reset will be generated to re-load the new option bytes\r\n");

			/* Enable WWDG clock */
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

			/* Generate a system Reset to re-load the new option bytes: enable WWDG and set
			 counter value to 0x4F, as T6 bit is cleared this will generate a WWDG reset */
			WWDG_Enable(0x4F);
		} else {
			SerialPutString("Error: Flash write unprotection failed...\r\n");
		}
	} else {
		SerialPutString("Flash memory not write protected\r\n");
	}
}

/**
 * Download and program the binary file
 */
static void SerialDownload(void)
{
	uint8_t tab_1024[1024] = { 0 };
	int32_t Size = 0;

	SerialPutString("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
	Size = Ymodem_Receive(&tab_1024[0]);
	if(Size > 0) {
		SerialPutString("\n\n\rProgramming Completed Successfully!\n\r");
	} else if(Size == -1) {
		SerialPutString("\n\n\rThe image size is higher than the allowed space memory!\n\r");
	} else if(Size == -2) {
		SerialPutString("\n\n\rVerification failed!\n\r");
	} else if(Size == -3) {
		SerialPutString("\r\n\nAborted by user.\n\r");
	} else {
		SerialPutString("\n\rFailed to receive the file!\n\r");
	}
}
