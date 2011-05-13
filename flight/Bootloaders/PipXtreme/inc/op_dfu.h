/**
 ******************************************************************************
 *
 * @file       op_dfu.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OP_DFU_H
#define __OP_DFU_H
#include "common.h"
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct {
	uint8_t programmingType;
	uint8_t readWriteFlags;
	uint32_t startOfUserCode;
	uint32_t sizeOfCode;
	uint8_t sizeOfDescription;
	uint8_t BL_Version;
	uint16_t devID;
	DeviceType devType;
	uint32_t FW_Crc;
} Device;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define COMMAND	0
#define COUNT	1
#define DATA	5

/* Exported functions ------------------------------------------------------- */
void processComand(uint8_t *Receive_Buffer);
uint32_t baseOfAdressType(uint8_t type);
uint8_t isBiggerThanAvailable(uint8_t type, uint32_t size);
void OPDfuIni(uint8_t discover);
void DataDownload(DownloadAction);
bool flash_read(uint8_t * buffer, uint32_t adr, DFUProgType type);
#endif /* __OP_DFU_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
