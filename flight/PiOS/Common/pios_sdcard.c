/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SDCARD SDCard Functions
 * @brief Code to deal with reading and writing to flash cards
 * @{
 *
 * @file       pios_sdcard.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      Sets up basic system hardware, functions are called from Main.
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_SDCARD)

/* Global Variables */
VOLINFO PIOS_SDCARD_VolInfo;
uint8_t PIOS_SDCARD_Sector[SECTOR_SIZE];

/* Local Definitions */
#if !defined(SDCARD_MUTEX_TAKE)
#define SDCARD_MUTEX_TAKE {}
#define SDCARD_MUTEX_GIVE {}
#endif

/* Definitions for MMC/SDC command */
#define SDCMD_GO_IDLE_STATE				(0x40+0)
#define SDCMD_GO_IDLE_STATE_CRC			0x95

#define SDCMD_SEND_OP_COND				(0x40+1)
#define SDCMD_SEND_OP_COND_CRC			0xf9

#define SDCMD_SEND_OP_COND_SDC			(0xC0+41)
#define SDCMD_SEND_OP_COND_SDC_CRC		0xff

#define SDCMD_READ_OCR					(0x40+58)
#define SDCMD_READ_OCR_CRC				0xff

#define SDCMD_APP_CMD					(0x40+55)
#define SDCMD_APP_CMD_CRC				0xff

#define SDCMD_SEND_IF_COND				(0x40+8)
#define SDCMD_SEND_IF_COND_CRC			0x87

#define SDCMD_SEND_CSD					(0x40+9)
#define SDCMD_SEND_CSD_CRC				0xff

#define SDCMD_SEND_CID					(0x40+10)
#define SDCMD_SEND_CID_CRC				0xff

#define SDCMD_SEND_STATUS				(0x40+13)
#define SDCMD_SEND_STATUS_CRC			0xaf

#define SDCMD_READ_SINGLE_BLOCK			(0x40+17)
#define SDCMD_READ_SINGLE_BLOCK_CRC		0xff

#define SDCMD_SET_BLOCKLEN				(0x40+16)
#define SDCMD_SET_BLOCKLEN_CRC			0xff

#define SDCMD_WRITE_SINGLE_BLOCK		(0x40+24)
#define SDCMD_WRITE_SINGLE_BLOCK_CRC	0xff

/* Card type flags (CardType) */
#define CT_MMC                          0x01
#define CT_SD1                          0x02
#define CT_SD2                          0x04
#define CT_SDC                          (CT_SD1|CT_SD2)
#define CT_BLOCK                        0x08
static uint8_t CardType;
static int32_t sdcard_mounted;

static uint32_t PIOS_SDCARD_SPI;

/**
* Initialises SPI pins and peripheral to access MMC/SD Card
* \param[in] mode currently only mode 0 supported
* \return < 0 if initialisation failed
*/
int32_t PIOS_SDCARD_Init(uint32_t spi_id)
{
	SDCARD_MUTEX_TAKE;

	sdcard_mounted = 0;

	PIOS_SDCARD_SPI = spi_id;

	/* Init SPI port for slow frequency access (ca. 0.3 MBit/s) */
	PIOS_SPI_SetClockSpeed(PIOS_SDCARD_SPI, PIOS_SPI_PRESCALER_256);

	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xFF);
	SDCARD_MUTEX_GIVE;

	/* No error */
	return 0;
}

/**
* Connects to SD Card
* \return < 0 if initialisation sequence failed
*/
int32_t PIOS_SDCARD_PowerOn(void)
{
	int32_t status;
	int i;

	SDCARD_MUTEX_TAKE;
	/* Ensure that chip select line deactivated */
	PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 1);	/* spi, pin_value */

	/* Init SPI port for slow frequency access (ca. 0.3 MBit/s) */
	PIOS_SPI_SetClockSpeed(PIOS_SDCARD_SPI, PIOS_SPI_PRESCALER_256);

	/* Send 80 clock cycles to start up */
	for (i = 0; i < 10; ++i) {
		PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
	}

	/* Activate chip select */
	PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 0);	/* spi, pin_value */

	/* wait for 1 mS */
	PIOS_DELAY_WaituS(1000);

	/* Send CMD0 to reset the media */
	if ((status = PIOS_SDCARD_SendSDCCmd(SDCMD_GO_IDLE_STATE, 0, SDCMD_GO_IDLE_STATE_CRC)) < 0) {
		goto error;
	}

	CardType = 0;

	/* A card is detected, what type is it? Use SEND_IF_COND (CMD8) to find out */
	if ((status = PIOS_SDCARD_SendSDCCmd(SDCMD_SEND_IF_COND, 0x1AA, SDCMD_SEND_IF_COND_CRC)) == 0x01aa) {
		/* SDHC Card Detected. */

		/* We now check to see if we should use block mode or byte mode. Command is SEND_OP_COND_SDC (ACMD41) with HCS (bit 30) set */
		for (i = 0; i < 16384; i++)
			if ((status = PIOS_SDCARD_SendSDCCmd(SDCMD_SEND_OP_COND_SDC, 0x01 << 30, SDCMD_SEND_OP_COND_SDC_CRC)) == 0)
				break;

		if (i < 16384) {
			status = PIOS_SDCARD_SendSDCCmd(SDCMD_READ_OCR, 0, SDCMD_READ_OCR_CRC);
			CardType = ((status >> 24) & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
			status = 0;
		} else {
			/* We waited long enough! */
			status = -2;
		}
	} else {
		/* Card is SDv1 or MMC. */

		/* MMC will accept ACMD41 (SDv1 won't and requires CMD1) so send ACMD41 first and then CMD1 if that fails */
		if ((status = PIOS_SDCARD_SendSDCCmd(SDCMD_SEND_OP_COND_SDC, 0, SDCMD_SEND_OP_COND_SDC_CRC)) <= 1) {
			CardType = CT_SD1;
		} else {
			CardType = CT_MMC;
		}

		for (i = 0; i < 16384; i++) {
			if (CardType == CT_SD1) {
				status = PIOS_SDCARD_SendSDCCmd(SDCMD_SEND_OP_COND_SDC, 0, SDCMD_SEND_OP_COND_SDC_CRC);
			} else {
				status = PIOS_SDCARD_SendSDCCmd(SDCMD_SEND_OP_COND, 0, SDCMD_SEND_OP_COND_CRC);
			}

			if (status < 0) {
				break;
			}
			if (status == 0) {
				break;
			}
		}

		/* The block size should already be 512 bytes but re-initialize just in case (ignore if it fails) */
		PIOS_SDCARD_SendSDCCmd(SDCMD_SET_BLOCKLEN, 512, SDCMD_SEND_OP_COND_CRC);
	}

	if (i == 16384 || CardType == 0) {
		/* The last loop timed out or the cardtype was not detected... */
		status = -2;
	}

error:
	/* Deactivate chip select */
	PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 1);	/* spi, pin_value */

	/* Send dummy byte once deactivated to drop cards DO */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
	SDCARD_MUTEX_GIVE;
	return status;		/* Status should be 0 if nothing went wrong! */
}

/**
* Disconnects from SD Card
* \return < 0 on errors
*/
int32_t PIOS_SDCARD_PowerOff(void)
{
	return 0;		// no error
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
	int32_t ret;
	SDCARD_MUTEX_TAKE;

	if (was_available) {
		/* Init SPI port for fast frequency access (ca. 18 MBit/s) */
		/* This is required for the case that the SPI port is shared with other devices */
		PIOS_SPI_SetClockSpeed(PIOS_SDCARD_SPI, PIOS_SPI_PRESCALER_4);

		/* Activate chip select */
		PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 0);	/* spi, pin_value */

		/* Send STATUS command to check if media is available */
		ret = PIOS_SDCARD_SendSDCCmd(SDCMD_SEND_STATUS, 0, SDCMD_SEND_STATUS_CRC);
	} else {
		/* Ensure that SPI interface is clocked at low speed */
		PIOS_SPI_SetClockSpeed(PIOS_SDCARD_SPI, PIOS_SPI_PRESCALER_256);

		/* Deactivate chip select */
		PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 1);	/* spi, pin_value */
		/* send 80 clock cycles to start up */
		uint8_t i;
		for (i = 0; i < 10; ++i) {
			PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
		}

		/* Activate chip select */
		PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 0);	/* spi, pin_value */

		/* Send CMD0 to reset the media */
		if ((ret = (PIOS_SDCARD_SendSDCCmd(SDCMD_GO_IDLE_STATE, 0, SDCMD_GO_IDLE_STATE_CRC))) < 0) {
			goto not_available;
		}

		/* Deactivate chip select */
		PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 1);	/* spi, pin_value */

		SDCARD_MUTEX_GIVE;
		/* Run power-on sequence (negative return = not available) */
		ret = PIOS_SDCARD_PowerOn();
	}

not_available:
	/* Deactivate chip select */
	PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 1);	/* spi, pin_value */
	/* Send dummy byte once deactivated to drop cards DO */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
	SDCARD_MUTEX_GIVE;

	return (ret == 0) ? 1 : 0;	/* 1 = available, 0 = not available. */
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
	int i;
	int32_t ret;

	if (cmd & 0x80) {	/* ACMD<n> is the command sequence of CMD55-CMD<n> */
		cmd &= 0x7F;
		ret = PIOS_SDCARD_SendSDCCmd(SDCMD_APP_CMD, 0, SDCMD_APP_CMD_CRC);
		if (ret > 1) {
			return ret;
		}
	}

	/* Activate chip select */
	PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 0);	/* spi, pin_value */

	/* Transfer to card */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, (uint8_t) cmd);
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, (addr >> 24) & 0xff);
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, (addr >> 16) & 0xff);
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, (addr >> 8) & 0xff);
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, (addr >> 0) & 0xff);
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, crc);

	uint8_t timeout = 0;

	if (cmd == SDCMD_SEND_STATUS) {
		/* One dummy read */
		PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

		/* Read two bytes (only last value will be returned) */
		ret = PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
		ret = PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

		/* All-one read: we expect that SD card is not connected: notify timeout! */
		if (ret == 0xff) {
			timeout = 1;
		}
	} else {
		/* Wait for standard R1 response */
		for (i = 0; i < 8; ++i) {
			if (!((ret = PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff)) & 0x80)) {
				break;
			}
		}
		if (i == 8) {
			timeout = 1;
		}
	}

	if ((cmd == SDCMD_SEND_IF_COND || cmd == SDCMD_READ_OCR) && timeout != 1) {
		/* This is a 4 byte R3 or R7 response. */
		ret = (PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff) << 24);
		ret |= (PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff) << 16);
		ret |= (PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff) << 8);
		ret |= (PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff) << 0);
	}

	/* Required clocking (see spec) */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

	/* Deactivate chip-select on timeout, and return error code */
	if (timeout) {
		PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 1);	/* spi, pin_value */

		/* Send dummy byte once deactivated to drop cards DO */
		PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
		return -1;
	}

	/* Else return received value - don't deactivate chip select or mutex (for continuous access) */
	return ret;
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
int32_t PIOS_SDCARD_SectorRead(uint32_t sector, uint8_t * buffer)
{
	int32_t status;
	int i;
	if (!(CardType & CT_BLOCK)) {
		sector *= 512;
	}

	SDCARD_MUTEX_TAKE;

	/* Init SPI port for fast frequency access (ca. 18 MBit/s) */
	/* this is required for the case that the SPI port is shared with other devices */
	PIOS_SPI_SetClockSpeed(PIOS_SDCARD_SPI, PIOS_SPI_PRESCALER_4);

	if ((status = PIOS_SDCARD_SendSDCCmd(SDCMD_READ_SINGLE_BLOCK, sector, SDCMD_READ_SINGLE_BLOCK_CRC))) {
		status = (status < 0) ? -256 : status;	/* return timeout indicator or error flags */
		goto error;
	}

	/* Wait for start token of the data block */
	for (i = 0; i < 65536; ++i) {	// TODO: check if sufficient
		uint8_t ret = PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
		if (ret != 0xff)
			break;
	}

	if (i == 65536) {
		status = -257;
		goto error;
	}

	/* Read 512 bytes via DMA */
	PIOS_SPI_TransferBlock(PIOS_SDCARD_SPI, NULL, buffer, 512, NULL);

	/* Read (and ignore) CRC */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

	/* Required for clocking (see spec) */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

error:
	/* Deactivate chip select */
	PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 1);	// spi, pin_value

	/* Send dummy byte once deactivated to drop cards DO */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
	SDCARD_MUTEX_GIVE;
	return status;
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
int32_t PIOS_SDCARD_SectorWrite(uint32_t sector, uint8_t * buffer)
{
	int32_t status;
	int i;

	SDCARD_MUTEX_TAKE;

	if (!(CardType & CT_BLOCK)) {
		sector *= 512;
	}

	/* Init SPI port for fast frequency access (ca. 18 MBit/s) */
	/* This is required for the case that the SPI port is shared with other devices */
	PIOS_SPI_SetClockSpeed(PIOS_SDCARD_SPI, PIOS_SPI_PRESCALER_4);

	if ((status = PIOS_SDCARD_SendSDCCmd(SDCMD_WRITE_SINGLE_BLOCK, sector, SDCMD_WRITE_SINGLE_BLOCK_CRC))) {
		status = (status < 0) ? -256 : status;	/* Return timeout indicator or error flags */
		goto error;
	}

	/* Send start token */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xfe);

	/* Send 512 bytes of data via DMA */
	PIOS_SPI_TransferBlock(PIOS_SDCARD_SPI, buffer, NULL, 512, NULL);

	/* Send CRC */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

	/* Read response */
	uint8_t response = PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
	if ((response & 0x0f) != 0x5) {
		status = -257;
		goto error;
	}

	/* Wait for write completion */
	for (i = 0; i < 32 * 65536; ++i) {	/* TODO: check if sufficient */
		uint8_t ret = PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
		if (ret != 0x00) {
			break;
		}
	}
	if (i == 32 * 65536) {
		status = -258;
		goto error;
	}

	/* Required for clocking (see spec) */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

error:
	/* Deactivate chip select */
	PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 1);	/* spi, pin_value */
	/* Send dummy byte once deactivated to drop cards DO */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

	SDCARD_MUTEX_GIVE;

	return status;
}

/**
* Reads the CID informations from SD Card
* \param[in] *cid pointer to buffer which holds the CID informations
* \return 0 if the informations haven been successfully read
* \return -error if error occured during read operation
* \return -256 if timeout during command has been sent
* \return -257 if timeout while waiting for start token
*/
int32_t PIOS_SDCARD_CIDRead(SDCARDCidTypeDef * cid)
{
	int32_t status;
	int i;

	/* Init SPI port for fast frequency access (ca. 18 MBit/s) */
	/* This is required for the case that the SPI port is shared with other devices */
	SDCARD_MUTEX_TAKE;

	PIOS_SPI_SetClockSpeed(PIOS_SDCARD_SPI, PIOS_SPI_PRESCALER_4);

	if ((status = PIOS_SDCARD_SendSDCCmd(SDCMD_SEND_CID, 0, SDCMD_SEND_CID_CRC))) {
		status = (status < 0) ? -256 : status;	/* return timeout indicator or error flags */
		goto error;
	}

	/* Wait for start token of the data block */
	for (i = 0; i < 65536; ++i) {	/* TODO: check if sufficient */
		uint8_t ret = PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
		if (ret != 0xff) {
			break;
		}
	}
	if (i == 65536) {
		status = -257;
	}

	/* Read 16 bytes via DMA */
	uint8_t cid_buffer[16];
	PIOS_SPI_TransferBlock(PIOS_SDCARD_SPI, NULL, cid_buffer, 16, NULL);

	/* Read (and ignore) CRC */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

	/* Required for clocking (see spec) */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

	/* Sort returned informations into CID structure */
	/* From STM Mass Storage example */
	/* Byte 0 */
	cid->ManufacturerID = cid_buffer[0];
	/* Byte 1 */
	cid->OEM_AppliID = cid_buffer[1] << 8;
	/* Byte 2 */
	cid->OEM_AppliID |= cid_buffer[2];
	/* Byte 3..7 */
	for (i = 0; i < 5; ++i)
		cid->ProdName[i] = cid_buffer[3 + i];
	cid->ProdName[5] = 0;	/* string terminator */
	/* Byte 8 */
	cid->ProdRev = cid_buffer[8];
	/* Byte 9 */
	cid->ProdSN = cid_buffer[9] << 24;
	/* Byte 10 */
	cid->ProdSN |= cid_buffer[10] << 16;
	/* Byte 11 */
	cid->ProdSN |= cid_buffer[11] << 8;
	/* Byte 12 */
	cid->ProdSN |= cid_buffer[12];
	/* Byte 13 */
	cid->Reserved1 |= (cid_buffer[13] & 0xF0) >> 4;
	/* Byte 14 */
	cid->ManufactDate = (cid_buffer[13] & 0x0F) << 8;
	/* Byte 15 */
	cid->ManufactDate |= cid_buffer[14];
	/* Byte 16 */
	cid->msd_CRC = (cid_buffer[15] & 0xFE) >> 1;
	cid->Reserved2 = 1;

error:
	/* deactivate chip select */
	PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 1);	/* spi, pin_value */
	/* Send dummy byte once deactivated to drop cards DO */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
	SDCARD_MUTEX_GIVE;

	return status;
}

/**
* Reads the CSD informations from SD Card
* \param[in] *csd pointer to buffer which holds the CSD informations
* \return 0 if the informations haven been successfully read
* \return -error if error occured during read operation
* \return -256 if timeout during command has been sent
* \return -257 if timeout while waiting for start token
*/
int32_t PIOS_SDCARD_CSDRead(SDCARDCsdTypeDef * csd)
{
	int32_t status;
	int i;

	SDCARD_MUTEX_TAKE;

	/* Init SPI port for fast frequency access (ca. 18 MBit/s) */
	/* This is required for the case that the SPI port is shared with other devices */
	PIOS_SPI_SetClockSpeed(PIOS_SDCARD_SPI, PIOS_SPI_PRESCALER_4);

	if ((status = PIOS_SDCARD_SendSDCCmd(SDCMD_SEND_CSD, 0, SDCMD_SEND_CSD_CRC))) {
		status = (status < 0) ? -256 : status;	/* Return timeout indicator or error flags */
		goto error;
	}
	// wait for start token of the data block
	for (i = 0; i < 65536; ++i) {	/* TODO: check if sufficient */
		uint8_t ret = PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
		if (ret != 0xff) {
			break;
		}
	}
	if (i == 65536) {
		status = -257;
		goto error;
	}

	/* Read 16 bytes via DMA */
	uint8_t csd_buffer[16];
	PIOS_SPI_TransferBlock(PIOS_SDCARD_SPI, NULL, csd_buffer, 16, NULL);

	/* Read (and ignore) CRC */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

	/* Required for clocking (see spec) */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);

	/* Sort returned informations into CSD structure */
	/* from STM Mass Storage example */
	/* Byte 0 */
	csd->CSDStruct = (csd_buffer[0] & 0xC0) >> 6;
	csd->SysSpecVersion = (csd_buffer[0] & 0x3C) >> 2;
	csd->Reserved1 = csd_buffer[0] & 0x03;
	/* Byte 1 */
	csd->TAAC = csd_buffer[1];
	/* Byte 2 */
	csd->NSAC = csd_buffer[2];
	/* Byte 3 */
	csd->MaxBusClkFrec = csd_buffer[3];
	/* Byte 4 */
	csd->CardComdClasses = csd_buffer[4] << 4;
	/* Byte 5 */
	csd->CardComdClasses |= (csd_buffer[5] & 0xF0) >> 4;
	csd->RdBlockLen = csd_buffer[5] & 0x0F;
	/* Byte 6 */
	csd->PartBlockRead = (csd_buffer[6] & 0x80) >> 7;
	csd->WrBlockMisalign = (csd_buffer[6] & 0x40) >> 6;
	csd->RdBlockMisalign = (csd_buffer[6] & 0x20) >> 5;
	csd->DSRImpl = (csd_buffer[6] & 0x10) >> 4;
	csd->Reserved2 = 0;	/* Reserved */
	csd->DeviceSize = (csd_buffer[6] & 0x03) << 10;
	/* Byte 7 */
	csd->DeviceSize |= (csd_buffer[7]) << 2;
	/* Byte 8 */
	csd->DeviceSize |= (csd_buffer[8] & 0xC0) >> 6;
	csd->MaxRdCurrentVDDMin = (csd_buffer[8] & 0x38) >> 3;
	csd->MaxRdCurrentVDDMax = (csd_buffer[8] & 0x07);
	/* Byte 9 */
	csd->MaxWrCurrentVDDMin = (csd_buffer[9] & 0xE0) >> 5;
	csd->MaxWrCurrentVDDMax = (csd_buffer[9] & 0x1C) >> 2;
	csd->DeviceSizeMul = (csd_buffer[9] & 0x03) << 1;
	/* Byte 10 */
	csd->DeviceSizeMul |= (csd_buffer[10] & 0x80) >> 7;
	csd->EraseGrSize = (csd_buffer[10] & 0x7C) >> 2;
	csd->EraseGrMul = (csd_buffer[10] & 0x03) << 3;
	/* Byte 11 */
	csd->EraseGrMul |= (csd_buffer[11] & 0xE0) >> 5;
	csd->WrProtectGrSize = (csd_buffer[11] & 0x1F);
	/* Byte 12 */
	csd->WrProtectGrEnable = (csd_buffer[12] & 0x80) >> 7;
	csd->ManDeflECC = (csd_buffer[12] & 0x60) >> 5;
	csd->WrSpeedFact = (csd_buffer[12] & 0x1C) >> 2;
	csd->MaxWrBlockLen = (csd_buffer[12] & 0x03) << 2;
	/* Byte 13 */
	csd->MaxWrBlockLen |= (csd_buffer[13] & 0xc0) >> 6;
	csd->WriteBlockPaPartial = (csd_buffer[13] & 0x20) >> 5;
	csd->Reserved3 = 0;
	csd->ContentProtectAppli = (csd_buffer[13] & 0x01);
	/* Byte 14 */
	csd->FileFormatGrouop = (csd_buffer[14] & 0x80) >> 7;
	csd->CopyFlag = (csd_buffer[14] & 0x40) >> 6;
	csd->PermWrProtect = (csd_buffer[14] & 0x20) >> 5;
	csd->TempWrProtect = (csd_buffer[14] & 0x10) >> 4;
	csd->FileFormat = (csd_buffer[14] & 0x0C) >> 2;
	csd->ECC = (csd_buffer[14] & 0x03);
	/* Byte 15 */
	csd->msd_CRC = (csd_buffer[15] & 0xFE) >> 1;
	csd->Reserved4 = 1;

error:
	/* Deactivate chip select */
	PIOS_SPI_RC_PinSet(PIOS_SDCARD_SPI, 1);	/* spi, pin_value */
	/* Send dummy byte once deactivated to drop cards DO */
	PIOS_SPI_TransferByte(PIOS_SDCARD_SPI, 0xff);
	SDCARD_MUTEX_GIVE;

	return status;
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
	FILEINFO File;
	char Buffer[1024];
	uint32_t Cache;

	/* Delete the file if it exists - ignore errors */
	DFS_UnlinkFile(&PIOS_SDCARD_VolInfo, (uint8_t *) LOG_FILENAME, PIOS_SDCARD_Sector);

	if (DFS_OpenFile(&PIOS_SDCARD_VolInfo, (uint8_t *) LOG_FILENAME, DFS_WRITE, PIOS_SDCARD_Sector, &File)) {
		/* Error opening file */
		return -2;
	}

	sprintf(Buffer, "PiOS Startup Log\r\n\r\nLog file creation completed.\r\n");
	if (DFS_WriteFile(&File, PIOS_SDCARD_Sector, (uint8_t *) Buffer, &Cache, strlen(Buffer))) {
		/* Error writing to file */
		return -3;
	}

	sprintf(Buffer, "------------------------------\r\nSD Card Information\r\n------------------------------\r\n");
	if (DFS_WriteFile(&File, PIOS_SDCARD_Sector, (uint8_t *) Buffer, &Cache, strlen(Buffer))) {
		/* Error writing to file */
		return -2;
	}

	/* Disabled because it takes ca. 7 seconds with my 2GB card */
	/* sprintf(Buffer, "Total Space: %u MB\r\nFree Space: %u MB\r\n", (uint16_t)((VolInfo.numclusters * (VolInfo.secperclus / 2)) / 1024), (uint16_t)(PIOS_SDCARD_GetFree() / 1048576)); */

	sprintf(Buffer, "Total Space: %u MB\r\n\r\n", (uint16_t) ((PIOS_SDCARD_VolInfo.numclusters * (PIOS_SDCARD_VolInfo.secperclus / 2)) / 1024));
	if (DFS_WriteFile(&File, PIOS_SDCARD_Sector, (uint8_t *) Buffer, &Cache, strlen(Buffer))) {
		/* Error writing to file */
		return -2;
	}

	/* No errors */
	return 0;
}

/**
 * Check if the SD card has been mounted
 * @return 0 if no
 * @return 1 if yes
 */
int32_t PIOS_SDCARD_IsMounted()
{
	return sdcard_mounted;
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
	uint32_t pstart, psize;
	uint8_t pactive, ptype;
	uint8_t sdcard_available = 0;

	sdcard_available = PIOS_SDCARD_CheckAvailable(0);

	if (!sdcard_available) {
		/* Disconnected */
		return -1;
	}

	pstart = DFS_GetPtnStart(0, PIOS_SDCARD_Sector, 0, &pactive, &ptype, &psize);
	if (pstart == 0xffffffff) {
		/* Cannot find first partition */
		return -2;
	}

	if (DFS_GetVolInfo(0, PIOS_SDCARD_Sector, pstart, &PIOS_SDCARD_VolInfo) != DFS_OK) {
		/* No volume information */
		return -3;
	}

	if (CreateStartupLog == 1) {
		if (PIOS_SDCARD_StartupLog()) {
			/* Error writing startup log file */
			return -4;
		}
	}

	/* No errors */
	sdcard_mounted = 1;
	return 0;
}

/**
* Mounts the file system
* return Amount of free bytes
*/
int32_t PIOS_SDCARD_GetFree(void)
{
	uint32_t VolFreeBytes = 0xffffffff;
	uint32_t ScratchCache = 0;

	/* This takes very long, since it scans ALL clusters */
	/* It takes ca. 7 seconds to determine free clusters out of ~47000 (2Gb) */

	/* Scan all the clusters */
	for (uint32_t i = 2; i < PIOS_SDCARD_VolInfo.numclusters; ++i) {
		if (!DFS_GetFAT(&PIOS_SDCARD_VolInfo, PIOS_SDCARD_Sector, &ScratchCache, i)) {
			VolFreeBytes += PIOS_SDCARD_VolInfo.secperclus * SECTOR_SIZE;
		}
	}

	return VolFreeBytes;
}

/**
* Read from file
* return 0 No error
* return -1 DFS_ReadFile failed
* return -2 Less bytes read than expected
*/
int32_t PIOS_SDCARD_ReadBuffer(PFILEINFO fileinfo, uint8_t * buffer, uint32_t len)
{
	uint32_t SuccessCount;

	if (DFS_ReadFile(fileinfo, PIOS_SDCARD_Sector, buffer, &SuccessCount, len)) {
		/* DFS_ReadFile failed */
		return -1;
	}
	if (SuccessCount != len) {
		/* Less bytes read than expected */
		return -2;
	}

	/* No error */
	return 0;
}

/**
* Read a line from file
* returns Number of bytes read
*/
int32_t PIOS_SDCARD_ReadLine(PFILEINFO fileinfo, uint8_t * buffer, uint32_t max_len)
{
	int32_t status;
	uint32_t num_read = 0;

	while (fileinfo->pointer < fileinfo->filelen) {
		status = PIOS_SDCARD_ReadBuffer(fileinfo, buffer, 1);

		if (status < 0) {
			return status;
		}

		++num_read;

		if (*buffer == '\n' || *buffer == '\r') {
			break;
		}

		if (num_read < max_len) {
			++buffer;
		}
	}

	/* Replace newline by terminator */
	*buffer = 0;

	return num_read;
}

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
	FILEINFO SourceFile, DestFile;

	/* Disable caching to avoid file inconsistencies while using different sector buffers! */
	DFS_CachingEnabledSet(0);

	if (DFS_OpenFile(&PIOS_SDCARD_VolInfo, (uint8_t *) Source, DFS_READ, PIOS_SDCARD_Sector, &SourceFile)) {
		/* Source file doesn't exist */
		return -1;
	} else {
		/* Delete destination file if it already exists - ignore errors */
		DFS_UnlinkFile(&PIOS_SDCARD_VolInfo, (uint8_t *) Destination, PIOS_SDCARD_Sector);

		if (DFS_OpenFile(&PIOS_SDCARD_VolInfo, (uint8_t *) Destination, DFS_WRITE, PIOS_SDCARD_Sector, &DestFile)) {
			/* Failed to create destination file */
			return -2;
		}
	}

	/* Copy operation */
	uint8_t WriteBuffer[SECTOR_SIZE];
	uint32_t SuccessCountRead;
	uint32_t SuccessCountWrite;
	do {
		if (DFS_ReadFile(&SourceFile, PIOS_SDCARD_Sector, WriteBuffer, &SuccessCountRead, SECTOR_SIZE)) {
			/* DFS_ReadFile failed */
			return -3;
		} else if (DFS_WriteFile(&DestFile, PIOS_SDCARD_Sector, WriteBuffer, &SuccessCountWrite, SuccessCountRead)) {
			/* DFS_WriteFile failed */
			return -4;
		}
	} while (SuccessCountRead > 0);

	/* No errors */
	return 0;
}

/**
* Delete a file
* param[in] *Filename File to delete
* return 0 No errors
* return -1 Error deleting file
*/
int32_t PIOS_SDCARD_FileDelete(char *Filename)
{
	if (DFS_UnlinkFile(&PIOS_SDCARD_VolInfo, (uint8_t *) Filename, PIOS_SDCARD_Sector)) {
		/* Error deleting file */
		return -1;
	}

	/* No errors */
	return 0;
}

#endif

/**
  * @}
  * @}
  */
