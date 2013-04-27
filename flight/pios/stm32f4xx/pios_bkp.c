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

#include <pios.h>
#include <pios_bkp.h>
#include <stm32f4xx_rtc.h>

/****************************************************************************************
 *  Header files
 ****************************************************************************************/

/*****************************************************************************************
 *	Public Definitions/Macros
 ****************************************************************************************/

/****************************************************************************************
 *  Public Functions
 ****************************************************************************************/
const uint32_t pios_bkp_registers_map[] =
	{
		RTC_BKP_DR0,
		RTC_BKP_DR1,
		RTC_BKP_DR2,
		RTC_BKP_DR3,
		RTC_BKP_DR4,
		RTC_BKP_DR5,
		RTC_BKP_DR6,
		RTC_BKP_DR7,
		RTC_BKP_DR8,
		RTC_BKP_DR9,
		RTC_BKP_DR10,
		RTC_BKP_DR11,
		RTC_BKP_DR12,
		RTC_BKP_DR13,
		RTC_BKP_DR14,
		RTC_BKP_DR15,
		RTC_BKP_DR16,
		RTC_BKP_DR17,
		RTC_BKP_DR18,
		RTC_BKP_DR19
	};
#define PIOS_BKP_REGISTERS_COUNT NELEMENTS(pios_bkp_registers_map)

void PIOS_BKP_Init(void)
{
	/* Enable CRC clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC | RCC_AHB1Periph_BKPSRAM, ENABLE);

	/* Enable PWR and BKP clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Clear Tamper pin Event(TE) pending flag */
	RTC_ClearFlag(RTC_FLAG_TAMP1F);
}

uint16_t PIOS_BKP_ReadRegister(uint32_t regnumber)
{
	if(PIOS_BKP_REGISTERS_COUNT < regnumber)
	{
		PIOS_Assert(0);
	} else {
		return (uint16_t) RTC_ReadBackupRegister(pios_bkp_registers_map[regnumber]);
	}
}

void PIOS_BKP_WriteRegister(uint32_t regnumber,uint16_t data)
{
	if(PIOS_BKP_REGISTERS_COUNT < regnumber)
	{
		PIOS_Assert(0);
	} else {
		RTC_WriteBackupRegister(pios_bkp_registers_map[regnumber],(uint32_t)data);
	}
}

void PIOS_BKP_EnableWrite(void)
{
	/* Enable write access to Backup domain */
	PWR_BackupAccessCmd(ENABLE);
}

void PIOS_BKP_DisableWrite(void)
{
	/* Enable write access to Backup domain */
	PWR_BackupAccessCmd(DISABLE);
}



/****************************************************************************************
 *  Public Data
 ****************************************************************************************/
