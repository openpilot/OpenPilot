/**
 ******************************************************************************
 *
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASH Flash device handler
 * @{
 *
 * @file       pios_flash_w25x.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Driver for talking to W25X flash chip (and most JEDEC chips)
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
#include "pios.h"

#define JEDEC_WRITE_ENABLE           0x06
#define JEDEC_WRITE_DISABLE          0x04
#define JEDEC_READ_STATUS            0x05
#define JEDEC_WRITE_STATUS           0x01
#define JEDEC_READ_DATA              0x03
#define JEDEC_FAST_READ              0x0b
#define JEDEC_DEVICE_ID              0x9F
#define JEDEC_PAGE_WRITE             0x02

#define JEDEC_STATUS_BUSY            0x01
#define JEDEC_STATUS_WRITEPROTECT    0x02
#define JEDEC_STATUS_BP0             0x04
#define JEDEC_STATUS_BP1             0x08
#define JEDEC_STATUS_BP2             0x10
#define JEDEC_STATUS_TP              0x20
#define JEDEC_STATUS_SEC             0x40
#define JEDEC_STATUS_SRP0            0x80

static uint8_t device_type;

enum pios_jedec_dev_magic {
	PIOS_JEDEC_DEV_MAGIC = 0xcb55aa55,
};

//! Device handle structure
struct jedec_flash_dev {
	uint32_t spi_id;
	uint32_t slave_num;
	bool claimed;
	uint32_t device_type;
	uint32_t capacity;
	const struct pios_flash_jedec_cfg * cfg;
#if defined(FLASH_FREERTOS)
	xSemaphoreHandle transaction_lock;
#endif
	enum pios_jedec_dev_magic magic;
};

//! Global structure for this flash device
struct jedec_flash_dev * flash_dev;

//! Private functions
static int32_t PIOS_Flash_Jedec_Validate(struct jedec_flash_dev * dev);
static struct jedec_flash_dev * PIOS_Flash_Jedec_alloc(void);
static int32_t PIOS_Flash_Jedec_ClaimBus();
static int32_t PIOS_Flash_Jedec_ReleaseBus();
static int32_t PIOS_Flash_Jedec_WriteEnable();
static int32_t PIOS_Flash_Jedec_Busy() ;


/**
 * @brief Allocate a new device
 */
static struct jedec_flash_dev * PIOS_Flash_Jedec_alloc(void)
{
	struct jedec_flash_dev * jedec_dev;
	
	jedec_dev = (struct jedec_flash_dev *)pvPortMalloc(sizeof(*jedec_dev));
	if (!jedec_dev) return (NULL);
	
	jedec_dev->claimed = false;
	jedec_dev->magic = PIOS_JEDEC_DEV_MAGIC;
#if defined(FLASH_FREERTOS)
	jedec_dev->transaction_lock = xSemaphoreCreateMutex();
#endif
	return(jedec_dev);
}

/**
 * @brief Validate the handle to the spi device
 */
static int32_t PIOS_Flash_Jedec_Validate(struct jedec_flash_dev * dev) {
	if (dev == NULL) 
		return -1;
	if (dev->magic != PIOS_JEDEC_DEV_MAGIC)
		return -2;
	if (dev->spi_id == 0)
		return -3;
	return 0;
}

/**
 * @brief Claim the SPI bus for flash use and assert CS pin
 * @return 0 for sucess, -1 for failure to get semaphore
 */
static int32_t PIOS_Flash_Jedec_ClaimBus()
{
	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	if(PIOS_SPI_ClaimBus(flash_dev->spi_id) < 0)
		return -1;
		
	PIOS_SPI_RC_PinSet(flash_dev->spi_id, flash_dev->slave_num, 0);
	flash_dev->claimed = true;
	
	return 0;
}

/**
 * @brief Release the SPI bus sempahore and ensure flash chip not using bus
 */
static int32_t PIOS_Flash_Jedec_ReleaseBus()
{
	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;
	PIOS_SPI_RC_PinSet(flash_dev->spi_id, flash_dev->slave_num, 1);
	PIOS_SPI_ReleaseBus(flash_dev->spi_id);
	flash_dev->claimed = false;
	return 0;
}

/**
 * @brief Returns if the flash chip is busy
 * @returns -1 for failure, 0 for not busy, 1 for busy
 */
static int32_t PIOS_Flash_Jedec_Busy()
{
	int32_t status = PIOS_Flash_Jedec_ReadStatus();
	if (status < 0)
		return -1;
	return status & JEDEC_STATUS_BUSY;
}

/**
 * @brief Execute the write enable instruction and returns the status
 * @returns 0 if successful, -1 if unable to claim bus
 */
static int32_t PIOS_Flash_Jedec_WriteEnable()
{
	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	uint8_t out[] = {JEDEC_WRITE_ENABLE};
	if(PIOS_Flash_Jedec_ClaimBus() != 0)
		return -1;
	PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL);
	PIOS_Flash_Jedec_ReleaseBus();
	return 0;
}

/**
 * @brief Initialize the flash device and enable write access
 */
int32_t PIOS_Flash_Jedec_Init(uint32_t spi_id, uint32_t slave_num, const struct pios_flash_jedec_cfg * cfg)
{
	flash_dev = PIOS_Flash_Jedec_alloc();
	if(flash_dev == NULL)
		return -1;

	flash_dev->spi_id = spi_id;
	flash_dev->slave_num = slave_num;
	flash_dev->cfg = cfg;

	device_type = PIOS_Flash_Jedec_ReadID();
	if(device_type == 0)
		return -1;

	return 0;
}

/**
 * @brief Grab the semaphore to perform a transaction
 * @return 0 for success, -1 for timeout
 */
int32_t PIOS_Flash_Jedec_StartTransaction()
{
#if defined(FLASH_FREERTOS)
	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	if(xSemaphoreTake(flash_dev->transaction_lock, portMAX_DELAY) != pdTRUE)
		return -1;
#endif
	return 0;
}

/**
 * @brief Release the semaphore to perform a transaction
 * @return 0 for success, -1 for timeout
 */
int32_t PIOS_Flash_Jedec_EndTransaction()
{
#if defined(FLASH_FREERTOS)
	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	if(xSemaphoreGive(flash_dev->transaction_lock) != pdTRUE)
		return -1;
#endif
	return 0;
}

/**
 * @brief Read the status register from flash chip and return it
 */
int32_t PIOS_Flash_Jedec_ReadStatus()
{
	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	uint8_t out[2] = {JEDEC_READ_STATUS, 0};
	uint8_t in[2] = {0,0};
	if(PIOS_Flash_Jedec_ClaimBus() < 0)
		return -1;
		
	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,in,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus();
		return -2;
	}
	
	PIOS_Flash_Jedec_ReleaseBus();
	return in[1];
}

/**
 * @brief Read the status register from flash chip and return it
 */
int32_t PIOS_Flash_Jedec_ReadID()
{
	uint8_t out[] = {JEDEC_DEVICE_ID};
	uint8_t in[4];
	if (PIOS_Flash_Jedec_ClaimBus() < 0) 
		return -1;
	
	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,in,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus();
		return -2;
	}
	PIOS_Flash_Jedec_ReleaseBus();
	
	flash_dev->device_type = in[1];
	flash_dev->capacity = in[3];
	return in[1];
}

/**
 * @brief Erase a sector on the flash chip
 * @param[in] add Address of flash to erase
 * @returns 0 if successful
 * @retval -1 if unable to claim bus
 * @retval
 */
int32_t PIOS_Flash_Jedec_EraseSector(uint32_t addr)
{
	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	uint8_t ret;
	uint8_t out[] = {flash_dev->cfg->sector_erase, (addr >> 16) & 0xff, (addr >> 8) & 0xff , addr & 0xff};

	if((ret = PIOS_Flash_Jedec_WriteEnable()) != 0)
		return ret;

	if(PIOS_Flash_Jedec_ClaimBus() != 0)
		return -1;
	
	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus();
		return -2;
	}
	
	PIOS_Flash_Jedec_ReleaseBus();

	// Keep polling when bus is busy too
	while(PIOS_Flash_Jedec_Busy() != 0) {
#if defined(FLASH_FREERTOS)
		vTaskDelay(1);
#endif
	}

	return 0;
}

/**
 * @brief Execute the whole chip
 * @returns 0 if successful, -1 if unable to claim bus
 */
int32_t PIOS_Flash_Jedec_EraseChip()
{
	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	uint8_t ret;
	uint8_t out[] = {flash_dev->cfg->chip_erase};

	if((ret = PIOS_Flash_Jedec_WriteEnable()) != 0)
		return ret;

	if(PIOS_Flash_Jedec_ClaimBus() != 0)
		return -1;
	
	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus();
		return -2;
	}
	
	PIOS_Flash_Jedec_ReleaseBus();

	// Keep polling when bus is busy too
	int i = 0;
	while(PIOS_Flash_Jedec_Busy() != 0) {
#if defined(FLASH_FREERTOS)
		vTaskDelay(1);
#endif
		if ((i++) % 10000 == 0)
			PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
	
}

	return 0;
}


/**
 * @brief Write one page of data (up to 256 bytes) aligned to a page start
 * @param[in] addr Address in flash to write to
 * @param[in] data Pointer to data to write to flash
 * @param[in] len Length of data to write (max 256 bytes)
 * @return Zero if success or error code
 * @retval -1 Unable to claim SPI bus
 * @retval -2 Size exceeds 256 bytes
 * @retval -3 Length to write would wrap around page boundary
 */
int32_t PIOS_Flash_Jedec_WriteData(uint32_t addr, uint8_t * data, uint16_t len)
{
	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	uint8_t ret;
	uint8_t out[4] = {JEDEC_PAGE_WRITE, (addr >> 16) & 0xff, (addr >> 8) & 0xff , addr & 0xff};

	/* Can only write one page at a time */
	if(len > 0x100)
		return -2;

	/* Ensure number of bytes fits after starting address before end of page */
	if(((addr & 0xff) + len) > 0x100)
		return -3;

	if((ret = PIOS_Flash_Jedec_WriteEnable()) != 0)
		return ret;

	/* Execute write page command and clock in address.  Keep CS asserted */
	if(PIOS_Flash_Jedec_ClaimBus() != 0)
		return -1;
	
	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus();
		return -1;
	}

	/* Clock out data to flash */
	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,data,NULL,len,NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus();
		return -1;
	}

	PIOS_Flash_Jedec_ReleaseBus();

	// Keep polling when bus is busy too
#if defined(FLASH_FREERTOS)
	while(PIOS_Flash_Jedec_Busy() != 0) {
		vTaskDelay(1);
	}
#else

	// Query status this way to prevent accel chip locking us out
	if(PIOS_Flash_Jedec_ClaimBus() < 0)
		return -1;

	PIOS_SPI_TransferByte(flash_dev->spi_id, JEDEC_READ_STATUS);
	while(PIOS_SPI_TransferByte(flash_dev->spi_id, JEDEC_READ_STATUS) & JEDEC_STATUS_BUSY);
	
	PIOS_Flash_Jedec_ReleaseBus();

#endif
	return 0;
}

/**
 * @brief Write multiple chunks of data in one transaction
 * @param[in] addr Address in flash to write to
 * @param[in] data Pointer to data to write to flash
 * @param[in] len Length of data to write (max 256 bytes)
 * @return Zero if success or error code
 * @retval -1 Unable to claim SPI bus
 * @retval -2 Size exceeds 256 bytes
 * @retval -3 Length to write would wrap around page boundary
 */
int32_t PIOS_Flash_Jedec_WriteChunks(uint32_t addr, struct pios_flash_chunk * p_chunk, uint32_t num)
{
	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;
	
	uint8_t ret;
	uint8_t out[4] = {JEDEC_PAGE_WRITE, (addr >> 16) & 0xff, (addr >> 8) & 0xff , addr & 0xff};
	
	/* Can only write one page at a time */
	uint32_t len = 0;
	for(uint32_t i = 0; i < num; i++)
		len += p_chunk[i].len;

	if(len > 0x100)
		return -2;
	
	/* Ensure number of bytes fits after starting address before end of page */
	if(((addr & 0xff) + len) > 0x100)
		return -3;
	
	if((ret = PIOS_Flash_Jedec_WriteEnable()) != 0)
		return ret;
	
	/* Execute write page command and clock in address.  Keep CS asserted */
	if(PIOS_Flash_Jedec_ClaimBus() != 0)
		return -1;
	
	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus();
		return -1;
	}
	
	for(uint32_t i = 0; i < num; i++) {
		struct pios_flash_chunk * chunk = &p_chunk[i];
		
		/* Clock out data to flash */
		if(PIOS_SPI_TransferBlock(flash_dev->spi_id,chunk->addr,NULL,chunk->len,NULL) < 0) {
			PIOS_Flash_Jedec_ReleaseBus();
			return -1;
		}

	}
	PIOS_Flash_Jedec_ReleaseBus();

	// Skip checking for busy with this to get OS running again fast

	return 0;
}

/**
 * @brief Read data from a location in flash memory
 * @param[in] addr Address in flash to write to
 * @param[in] data Pointer to data to write from flash
 * @param[in] len Length of data to write (max 256 bytes)
 * @return Zero if success or error code
 * @retval -1 Unable to claim SPI bus
 */
int32_t PIOS_Flash_Jedec_ReadData(uint32_t addr, uint8_t * data, uint16_t len)
{
	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	if(PIOS_Flash_Jedec_ClaimBus() == -1)
		return -1;

	/* Execute read command and clock in address.  Keep CS asserted */
	uint8_t out[] = {JEDEC_READ_DATA, (addr >> 16) & 0xff, (addr >> 8) & 0xff , addr & 0xff};
	
	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus();
		return -2;
	}

	/* Copy the transfer data to the buffer */
	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,NULL,data,len,NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus();
		return -3;
	}

	PIOS_Flash_Jedec_ReleaseBus();

	return 0;
}
