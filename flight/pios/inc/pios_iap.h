/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_IAP In-Application-Programming Module
 * @brief In-Application-Programming Module
 * @{
 *
 * @file       pios_iap.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      IAP functions
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
#ifndef PIOS_IAP_H
#define PIOS_IAP_H



/****************************************************************************************
 *  Header files
 ****************************************************************************************/
#include <pios_bkp.h>

/*****************************************************************************************
 *	Public Definitions/Macros
 ****************************************************************************************/
#define MAGIC_REG_1     PIOS_BKP_RESERVED_1
#define MAGIC_REG_2     PIOS_BKP_RESERVED_2
#define IAP_BOOTCOUNT   PIOS_BKP_RESERVED_3
#define IAP_CMD1        PIOS_BKP_RESERVED_5
#define IAP_CMD2        PIOS_BKP_RESERVED_6
#define IAP_CMD3        PIOS_BKP_RESERVED_7

#define PIOS_IAP_CLEAR_FLASH_CMD_0 0xFA5F
#define PIOS_IAP_CLEAR_FLASH_CMD_1 0x0001
#define PIOS_IAP_CLEAR_FLASH_CMD_2 0x0000

#define PIOS_IAP_CMD_COUNT 3

/****************************************************************************************
 *  Public Functions
 ****************************************************************************************/
void		PIOS_IAP_Init(void);
uint32_t	PIOS_IAP_CheckRequest( void );
void		PIOS_IAP_SetRequest1(void);
void		PIOS_IAP_SetRequest2(void);
void		PIOS_IAP_ClearRequest(void);
uint16_t	PIOS_IAP_ReadBootCount(void);
void		PIOS_IAP_WriteBootCount(uint16_t);

/**
  * @brief  Return one of the IAP command values passed from bootloader.
  * @param  number: the index of the command value (0..2).
  * @retval the selected command value.
  */
uint32_t PIOS_IAP_ReadBootCmd(uint8_t number);

/**
  * @brief  Write one of the IAP command values to be passed to firmware from bootloader.
  * @param  number: the index of the command value (0..2).
  * @param  value: value to be written.
  */
void PIOS_IAP_WriteBootCmd(uint8_t number, uint32_t value);
/****************************************************************************************
 *  Public Data
 ****************************************************************************************/

#endif /* PIOS_IAP_H */
