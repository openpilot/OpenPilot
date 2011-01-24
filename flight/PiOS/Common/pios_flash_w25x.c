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

void PIOS_FLASH_W25X_ClaimBus() 
{
	// TODO: Semaphore to lock bus
	PIOS_FLASH_ENABLE;
}

void PIOS_FLASH_W25X_ReleaseBus() 
{
	// TODO: Release semaphore
	PIOS_FLASH_DISABLE;
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

void PIOS_FLASH_W25X_WriteData(uint32_t addr, uint8_t * data, uint16_t len)
{
}

void PIOS_FLASH_W25X_ReadData(uint32_t addr, uint8_t * data, uint16_t len)
{
}