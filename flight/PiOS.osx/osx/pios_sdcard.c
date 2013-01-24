/**
 ******************************************************************************
 *
 * @file       pios_sdcard.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
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

#if defined(PIOS_INCLUDE_SDCARD)


/**
* Initialises SPI pins and peripheral to access MMC/SD Card
* \param[in] mode currently only mode 0 supported
* \return < 0 if initialisation failed
*/
int32_t PIOS_SDCARD_Init(void)
{
	/* No error */
	return 0;
}


/**
* Connects to SD Card
* \return < 0 if initialisation sequence failed
*/
int32_t PIOS_SDCARD_PowerOn(void)
{
	return 0; /* Status should be 0 if nothing went wrong! */
}


/**
* Disconnects from SD Card
* \return < 0 on errors
*/
int32_t PIOS_SDCARD_PowerOff(void)
{
	return 0; // no error
}


/**
* If SD card was previously available: Checks if the SD Card is still
* available by sending the STATUS command.<BR>
* This takes ca. 10 uS
*
* If SD card was previously not available: Checks if the SD Card is
* available by sending the IDLE command at low speed.<BR>
* This takes ca. 500 uS!<BR>
* Once we got a positive response, SDCARD_PowerOn() will be
* called by this function to initialize the card completely.
*
* Example for Connection/Disconnection detection:
* \code
* // this function is called each second from a low-priority task
* // If multiple tasks are accessing the SD card, add a semaphore/mutex
* //  to avoid IO access collisions with other tasks!
* u8 sdcard_available;
* int32_t CheckSDCard(void)
* {
*   // check if SD card is available
*   // High speed access if the card was previously available
*   u8 prev_sdcard_available = sdcard_available;
*   sdcard_available = PIOS_SDCARD_CheckAvailable(prev_sdcard_available);
*
*   if(sdcard_available && !prev_sdcard_available) {
*     // SD Card has been connected
*
*     // now it's possible to read/write sectors
*
*   } else if( !sdcard_available && prev_sdcard_available ) {
*     // SD Card has been disconnected
*
*     // here you can notify your application about this state
*   }
*
*   return 0; // no error
* }
* \endcode
* \param[in] was_available should only be set if the SD card was previously available
* \return 0 if no response from SD Card
* \return 1 if SD card is accessible
*/
int32_t PIOS_SDCARD_CheckAvailable(uint8_t was_available)
{
	return 1; /* 1 = available, 0 = not available. */
}


/**
* Sends command to SD card
* \param[in] cmd SD card command
* \param[in] addr 32bit address
* \param[in] crc precalculated CRC
* \return >= 0x00 if command has been sent successfully (contains received byte)
* \return -1 if no response from SD Card (timeout)
*/
int32_t PIOS_SDCARD_SendSDCCmd(uint8_t cmd, uint32_t addr, uint8_t crc)
{
	return -1;
}


/**
* Reads 512 bytes from selected sector
* \param[in] sector 32bit sector
* \param[in] *buffer pointer to 512 byte buffer
* \return 0 if whole sector has been successfully read
* \return -error if error occured during read operation:<BR>
* <UL>
*   <LI>Bit 0              - In idle state if 1
*   <LI>Bit 1              - Erase Reset if 1
*   <LI>Bit 2              - Illgal Command if 1
*   <LI>Bit 3              - Com CRC Error if 1
*   <LI>Bit 4              - Erase Sequence Error if 1
*   <LI>Bit 5              - Address Error if 1
*   <LI>Bit 6              - Parameter Error if 1
*   <LI>Bit 7              - Not used, always '0'
* </UL>
* \return -256 if timeout during command has been sent
* \return -257 if timeout while waiting for start token
*/
int32_t PIOS_SDCARD_SectorRead(uint32_t sector, uint8_t *buffer)
{
	return -256;
}


/**
* Writes 512 bytes into selected sector
* \param[in] sector 32bit sector
* \param[in] *buffer pointer to 512 byte buffer
* \return 0 if whole sector has been successfully read
* \return -error if error occured during read operation:<BR>
* <UL>
*   <LI>Bit 0              - In idle state if 1
*   <LI>Bit 1              - Erase Reset if 1
*   <LI>Bit 2              - Illgal Command if 1
*   <LI>Bit 3              - Com CRC Error if 1
*   <LI>Bit 4              - Erase Sequence Error if 1
*   <LI>Bit 5              - Address Error if 1
*   <LI>Bit 6              - Parameter Error if 1
*   <LI>Bit 7              - Not used, always '0'
* </UL>
* \return -256 if timeout during command has been sent
* \return -257 if write operation not accepted
* \return -258 if timeout during write operation
*/
int32_t PIOS_SDCARD_SectorWrite(uint32_t sector, uint8_t *buffer)
{
	return -256;
}


/**
* Reads the CID informations from SD Card
* \param[in] *cid pointer to buffer which holds the CID informations
* \return 0 if the informations haven been successfully read
* \return -error if error occured during read operation
* \return -256 if timeout during command has been sent
* \return -257 if timeout while waiting for start token
*/
int32_t PIOS_SDCARD_CIDRead(SDCARDCidTypeDef *cid)
{
	return -256;
}

/**
* Reads the CSD informations from SD Card
* \param[in] *csd pointer to buffer which holds the CSD informations
* \return 0 if the informations haven been successfully read
* \return -error if error occured during read operation
* \return -256 if timeout during command has been sent
* \return -257 if timeout while waiting for start token
*/
int32_t PIOS_SDCARD_CSDRead(SDCARDCsdTypeDef *csd)
{
	return -256;
}

/**
* Attempts to write a startup log to the SDCard
* return 0 No errors
* return -1 Error deleting file
* return -2 Error opening file
* return -3 Error writing file
*/
int32_t PIOS_SDCARD_StartupLog(void)
{
	return -3;
}

/**
 * Check if the SD card has been mounted
 * @return 0 if no
 * @return 1 if yes
 */
int32_t PIOS_SDCARD_IsMounted()
{
	return 1;
}

/**
* Mounts the file system
* param[in] CreateStartupLog 1 = True, 0 = False
* return 0 No errors
* return -1 SDCard not available
* return -2 Cannot find first partition
* return -3 No volume information
* return -4 Error writing startup log file
*/
int32_t PIOS_SDCARD_MountFS(uint32_t CreateStartupLog)
{
	return 0;
}

/**
* Mounts the file system
* return Amount of free bytes
*/
int32_t PIOS_SDCARD_GetFree(void)
{
	return 10240;
}

/**
* Read from file
* return 0 No error
* return -1 DFS_ReadFile failed
* return -2 Less bytes read than expected
*/
//int32_t PIOS_SDCARD_ReadBuffer(PFILEINFO fileinfo, uint8_t *buffer, uint32_t len)
//{
	/* No error */
//	return -1;
//}

/**
* Read a line from file
* returns Number of bytes read
*/
//int32_t PIOS_SDCARD_ReadLine(PFILEINFO fileinfo, uint8_t *buffer, uint32_t max_len)
//{
//	return -1;
//}

/**
* Copy a file
* WARNING: This will overwrite the destination file even if it exists
* param[in] *Source Path to file to copy
* param[in] *Destination Path to destination file
* return 0 No errors
* return -1 Source file doesn't exist
* return -2 Failed to create destination file
* return -3 DFS_ReadFile failed
* return -4 DFS_WriteFile failed
*/
int32_t PIOS_SDCARD_FileCopy(char *Source, char *Destination)
{
	return -2;
}

/**
* Delete a file
* param[in] *Filename File to delete
* return 0 No errors
* return -1 Error deleting file
*/
int32_t PIOS_SDCARD_FileDelete(char *Filename)
{
	return -1;
}

#endif
