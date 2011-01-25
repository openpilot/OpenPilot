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

void PIOS_FLASH_W25X_ClaimBus() 
{
	PIOS_SPI_ClaimBus(PIOS_SPI_FLASH);
	PIOS_ADXL_DISABLE;
	PIOS_FLASH_ENABLE;
	PIOS_DELAY_WaituS(1);
}

void PIOS_FLASH_W25X_ReleaseBus() 
{
	PIOS_ADXL_DISABLE;
	PIOS_FLASH_DISABLE;
	PIOS_SPI_ReleaseBus(PIOS_SPI_FLASH);
}

void PIOS_FLASH_W25X_Init()
{
	PIOS_FLASH_W25X_ClaimBus();
	PIOS_FLASH_W25X_ReleaseBus();
}

/**
 * @brief Read the status register from flash chip and return it 
 */
uint8_t PIOS_FLASH_ReadStatus() 
{
	PIOS_FLASH_W25X_ClaimBus();
	uint8_t out[2] = {W25X_READ_STATUS, 0};
	uint8_t in[2] = {0,0};
	PIOS_SPI_TransferBlock(PIOS_SPI_FLASH,out,in,sizeof(out),NULL);	
	PIOS_FLASH_W25X_ReleaseBus();
	return in[1];
}

/**
 * @brief Read the status register from flash chip and return it 
 */
uint8_t PIOS_FLASH_ReadID() 
{
	PIOS_FLASH_W25X_ClaimBus();
	uint8_t out[] = {W25X_DEVICE_ID, 0, 0, 0, 0, 0};
	uint8_t in[6];
	PIOS_SPI_TransferBlock(PIOS_SPI_FLASH,out,in,sizeof(out),NULL);	
	PIOS_FLASH_W25X_ReleaseBus();
	return in[5];
}

void PIOS_FLASH_W25X_WriteData(uint32_t addr, uint8_t * data, uint16_t len)
{
}

void PIOS_FLASH_W25X_ReadData(uint32_t addr, uint8_t * data, uint16_t len)
{
}