/*
 *  pios_flash_w25x.c
 *  OpenPilotOSX
 *
 *  Created by James Cotton on 1/23/11.
 *  Copyright 2011 OpenPilot. All rights reserved.
 *
 */

#include "pios.h"
#include "pios_flash_w25x.h"
#include "pios_adxl345.h"

#define W25X_WRITE_ENABLE           0x06
#define W25X_WRITE_DISABLE          0x04
#define W25X_READ_STATUS            0x05
#define W25X_WRITE_STATUS           0x01
#define W25X_READ_DATA              0x03
#define W25X_FAST_READ              0x0b
#define W25X_DEVICE_ID              0x90
#define W25X_SECTOR_ERASE           0x20
#define W25X_PAGE_WRITE             0x02
#define W25X_CHIP_ERASE             0x60

#define W25X_STATUS_BUSY            0x01
#define W25X_STATUS_WRITEPROTECT    0x02
#define W25X_STATUS_BP0             0x04
#define W25X_STATUS_BP1             0x08
#define W25X_STATUS_BP2             0x10
#define W25X_STATUS_TP              0x20
#define W25X_STATUS_SEC             0x40
#define W25X_STATUS_SRP0            0x80

static uint8_t device_type;

//! Private functions
static int8_t PIOS_Flash_W25X_ClaimBus();
static void PIOS_Flash_W25X_ReleaseBus();
static uint8_t PIOS_Flash_W25X_WriteEnable();
static uint8_t PIOS_Flash_W25X_Busy() ;

static uint32_t PIOS_SPI_FLASH;

/**
 * @brief Claim the SPI bus for flash use and assert CS pin
 * @return 0 for sucess, -1 for failure to get semaphore
 */
static int8_t PIOS_Flash_W25X_ClaimBus()
{
	int8_t ret = PIOS_SPI_ClaimBus(PIOS_SPI_FLASH);
	PIOS_FLASH_ENABLE;
	return (ret == 0) ? 0 : -1;
}

/**
 * @brief Release the SPI bus sempahore and ensure flash chip not using bus
 */
static void PIOS_Flash_W25X_ReleaseBus()
{
	PIOS_FLASH_DISABLE;
	PIOS_SPI_ReleaseBus(PIOS_SPI_FLASH);
}

/**
 * @brief Returns if the flash chip is busy
 */
static uint8_t PIOS_Flash_W25X_Busy()
{
	return PIOS_Flash_W25X_ReadStatus() & W25X_STATUS_BUSY;
}

/**
 * @brief Execute the write enable instruction and returns the status
 * @returns 0 if successful, -1 if unable to claim bus
 */
static uint8_t PIOS_Flash_W25X_WriteEnable()
{
	uint8_t out[] = {W25X_WRITE_ENABLE};
	if(PIOS_Flash_W25X_ClaimBus() != 0)
		return -1;
	PIOS_SPI_TransferBlock(PIOS_SPI_FLASH,out,NULL,sizeof(out),NULL);
	PIOS_Flash_W25X_ReleaseBus();
	return 0;
}

/**
 * @brief Initialize the flash device and enable write access
 */
int8_t PIOS_Flash_W25X_Init(uint32_t spi_id)
{
	PIOS_SPI_FLASH = spi_id;

	PIOS_GPIO_Enable(PIOS_FLASH_CS_PIN);
	device_type = PIOS_Flash_W25X_ReadID();
	return 0;
}


/**
 * @brief Read the status register from flash chip and return it
 */
uint8_t PIOS_Flash_W25X_ReadStatus()
{
	uint8_t out[2] = {W25X_READ_STATUS, 0};
	uint8_t in[2] = {0,0};
	PIOS_Flash_W25X_ClaimBus();
	PIOS_SPI_TransferBlock(PIOS_SPI_FLASH,out,in,sizeof(out),NULL);
	PIOS_Flash_W25X_ReleaseBus();
	return in[1];
}

/**
 * @brief Read the status register from flash chip and return it
 */
uint8_t PIOS_Flash_W25X_ReadID()
{
	uint8_t out[] = {W25X_DEVICE_ID, 0, 0, 0, 0, 0};
	uint8_t in[6];
	PIOS_Flash_W25X_ClaimBus();
	PIOS_SPI_TransferBlock(PIOS_SPI_FLASH,out,in,sizeof(out),NULL);
	PIOS_Flash_W25X_ReleaseBus();
	return in[5];
}

/**
 * @brief Erase a sector on the flash chip
 * @param[in] add Address of flash to erase
 * @returns 0 if successful
 * @retval -1 if unable to claim bus
 * @retval
 */
int8_t PIOS_Flash_W25X_EraseSector(uint32_t addr)
{
	uint8_t ret;
	uint8_t out[] = {W25X_SECTOR_ERASE, (addr >> 16) & 0xff, (addr >> 8) & 0xff , addr & 0xff};

	if((ret = PIOS_Flash_W25X_WriteEnable()) != 0)
		return ret;

	if(PIOS_Flash_W25X_ClaimBus() != 0)
		return -1;
	PIOS_SPI_TransferBlock(PIOS_SPI_FLASH,out,NULL,sizeof(out),NULL);
	PIOS_Flash_W25X_ReleaseBus();

	uint32_t i = 1;
	while(PIOS_Flash_W25X_Busy()) {
		if(++i == 0)
			return -1;
	}

	return 0;
}

/**
 * @brief Execute the whole chip
 * @returns 0 if successful, -1 if unable to claim bus
 */
int8_t PIOS_Flash_W25X_EraseChip()
{
	uint8_t ret;
	uint8_t out[] = {W25X_CHIP_ERASE};

	if((ret = PIOS_Flash_W25X_WriteEnable()) != 0)
		return ret;

	if(PIOS_Flash_W25X_ClaimBus() != 0)
		return -1;
	PIOS_SPI_TransferBlock(PIOS_SPI_FLASH,out,NULL,sizeof(out),NULL);
	PIOS_Flash_W25X_ReleaseBus();

	uint32_t i = 1;
	while(PIOS_Flash_W25X_Busy()) {
		if(++i == 0)
			return -1;
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
int8_t PIOS_Flash_W25X_WriteData(uint32_t addr, uint8_t * data, uint16_t len)
{
	uint8_t ret;
	uint8_t out[4] = {W25X_PAGE_WRITE, (addr >> 16) & 0xff, (addr >> 8) & 0xff , addr & 0xff};

	/* Can only write one page at a time */
	if(len > 0x100)
		return -2;

	/* Ensure number of bytes fits after starting address before end of page */
	if(((addr & 0xff) + len) > 0x100)
		return -3;

	if((ret = PIOS_Flash_W25X_WriteEnable()) != 0)
		return ret;

	/* Execute write page command and clock in address.  Keep CS asserted */
	if(PIOS_Flash_W25X_ClaimBus() != 0)
		return -1;
	PIOS_SPI_TransferBlock(PIOS_SPI_FLASH,out,NULL,sizeof(out),NULL);

	/* Clock out data to flash */
	PIOS_SPI_TransferBlock(PIOS_SPI_FLASH,data,NULL,len,NULL);

	PIOS_Flash_W25X_ReleaseBus();

	uint32_t i = 1;
	while(PIOS_Flash_W25X_Busy()) {
		if(++i == 0)
			return -1;
	}

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
int8_t PIOS_Flash_W25X_ReadData(uint32_t addr, uint8_t * data, uint16_t len)
{
	if(PIOS_Flash_W25X_ClaimBus() == -1)
		return -1;

	/* Execute read command and clock in address.  Keep CS asserted */
	uint8_t out[] = {W25X_READ_DATA, (addr >> 16) & 0xff, (addr >> 8) & 0xff , addr & 0xff};
	PIOS_SPI_TransferBlock(PIOS_SPI_FLASH,out,NULL,sizeof(out),NULL);

	/* Copy the transfer data to the buffer */
	PIOS_SPI_TransferBlock(PIOS_SPI_FLASH,NULL,data,len,NULL);

	PIOS_Flash_W25X_ReleaseBus();

	return 0;
}
