/*
 *  pios_flash_w25x.h
 *  OpenPilotOSX
 *
 *  Created by James Cotton on 1/23/11.
 *  Copyright 2011 OpenPilot. All rights reserved.
 *
 */

int8_t PIOS_Flash_W25X_Init();
uint8_t PIOS_Flash_W25X_ReadStatus();
uint8_t PIOS_Flash_W25X_ReadID();
int8_t PIOS_Flash_W25X_EraseChip();
int8_t PIOS_Flash_W25X_EraseSector(uint32_t add);
int8_t PIOS_Flash_W25X_WriteData(uint32_t addr, uint8_t * data, uint16_t len);
int8_t PIOS_Flash_W25X_ReadData(uint32_t addr, uint8_t * data, uint16_t len);
