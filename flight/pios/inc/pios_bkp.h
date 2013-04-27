/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_BKP Backup SRAM functions
 * @brief Hardware abstraction layer for backup sram
 * @{
 *
 * @file       pios_bkp.c  
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
#ifndef PIOS_BKP_H_
#define PIOS_BKP_H_


/****************************************************************************************
 *  Header files
 ****************************************************************************************/

/*****************************************************************************************
 *	Public Definitions/Macros
 ****************************************************************************************/
// Backup registers definitions
// registers reserved for PIOS usage
#define PIOS_BKP_RESERVED_1   0  // IAP_MAGIC_REG_1
#define PIOS_BKP_RESERVED_2   1  // IAP_MAGIC_REG_2
#define PIOS_BKP_RESERVED_3   2  // IAP_BOOTCOUNT
#define PIOS_BKP_RESERVED_4   3  // PIOS_WDG_REGISTER
#define PIOS_BKP_RESERVED_5   4  // IAP_CMD1
#define PIOS_BKP_RESERVED_6   5  // IAP_CMD2
#define PIOS_BKP_RESERVED_7   6  // IAP_CMD3
#define PIOS_BKP_RESERVED_8   7
#define PIOS_BKP_RESERVED_9   8
#define PIOS_BKP_RESERVED_10  9
// registers reserved for BOARD specific usage
#define PIOS_BKP_BOARD_RESERVED_1 10
#define PIOS_BKP_BOARD_RESERVED_2 11
#define PIOS_BKP_BOARD_RESERVED_3 12
// registers reserved for APP usage
#define PIOS_BKP_APP_RESERVED_1 13
#define PIOS_BKP_APP_RESERVED_2 14
#define PIOS_BKP_APP_RESERVED_3 15
#define PIOS_BKP_APP_RESERVED_4 16

/****************************************************************************************
 *  Public Functions
 ****************************************************************************************/
/** @defgroup PIOS_BKP_Public_Functions
  * @{
  */

/**
  * @brief  Initialize the Backup Register hardware
  * @param  None
  * @retval None
  */
void		PIOS_BKP_Init(void);

/**
  * @brief  Reads data from the specified Backup Register.
  * @param  regnumber: specifies the Backup Register.
  * @retval The content of the specified Data Backup Register
  */
uint16_t	PIOS_BKP_ReadRegister(uint32_t regnumber);

/**
  * @brief  Writes user data to the specified Backup Register.
  * @param  regnumber: specifies the Data Backup Register.
  * @param  data: data to write
  * @retval None
  */
void		PIOS_BKP_WriteRegister(uint32_t regnumber,uint16_t data);

/**
  * @brief  Enable Backup registers write access
  * @param  None
  * @retval None
  */
void		PIOS_BKP_EnableWrite(void);

/**
  * @brief  Disable Backup registers write access
  * @param  None
  * @retval None
  */
void		PIOS_BKP_DisableWrite(void);

/**
  * @}
  */



/****************************************************************************************
 *  Public Data
 ****************************************************************************************/

#endif /* PIOS_BKP_H_ */