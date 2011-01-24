/*
 *  pios_flash_w25x.h
 *  OpenPilotOSX
 *
 *  Created by James Cotton on 1/23/11.
 *  Copyright 2011 OpenPilot. All rights reserved.
 *
 */

#define W25X_WRITE_ENABLE           0x06
#define W25X_WRITE_DISABLE          0x04
#define W25X_READ_STATUS            0x05
#define W25X_WRITE_STATUS           0x01
#define W25X_READ_DATA              0x03
#define W25X_FAST_READ              0x0b
#define W25X_DEVICE_ID              0x92

void PIOS_FLASH_W25X_Init();
uint8_t PIOS_FLASH_ReadStatus();
void PIOS_FLASH_W25X_WriteData(uint32_t addr, uint8_t * data, uint16_t len);
void PIOS_FLASH_W25X_ReadData(uint32_t addr, uint8_t * data, uint16_t len);