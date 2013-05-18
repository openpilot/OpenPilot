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
#include <stm32f10x.h>
#include <stm32f10x_bkp.h>
#include <stm32f10x_pwr.h>

/****************************************************************************************
*  Header files
****************************************************************************************/

/*****************************************************************************************
 *	Public Definitions/Macros
 ****************************************************************************************/

/****************************************************************************************
*  Public Functions
****************************************************************************************/
const uint32_t pios_bkp_registers_map[] = {
    BKP_DR1,
    BKP_DR2,
    BKP_DR3,
    BKP_DR4,
    BKP_DR5,
    BKP_DR6,
    BKP_DR7,
    BKP_DR8,
    BKP_DR9,
    BKP_DR10,
    BKP_DR11,
    BKP_DR12,
    BKP_DR13,
    BKP_DR14,
    BKP_DR15,
    BKP_DR16,
    BKP_DR17,
    BKP_DR18,
    BKP_DR19,

#if FALSE /*  Not enabled as stm32f4 needs some modifications to
           *  accomodate more than 20 registers (like storing 2 uint16_t
           *  regs in one uint32_t bkp location)
           */
    BKP_DR20,
    BKP_DR21,
    BKP_DR22,
    BKP_DR23,
    BKP_DR24,
    BKP_DR25,
    BKP_DR26,
    BKP_DR27,
    BKP_DR28,
    BKP_DR29,
    BKP_DR30,
    BKP_DR32,
    BKP_DR33,
    BKP_DR34,
    BKP_DR35,
    BKP_DR36,
    BKP_DR37,
    BKP_DR38,
    BKP_DR39,
    BKP_DR40,
    BKP_DR41,
    BKP_DR42,
#endif /* if FALSE */
};
#define PIOS_BKP_REGISTERS_COUNT NELEMENTS(pios_bkp_registers_map)

void PIOS_BKP_Init(void)
{
    /* Enable CRC clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);

    /* Enable PWR and BKP clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* Clear Tamper pin Event(TE) pending flag */
    BKP_ClearFlag();
}

uint16_t PIOS_BKP_ReadRegister(uint32_t regnumber)
{
    if (PIOS_BKP_REGISTERS_COUNT < regnumber) {
        PIOS_Assert(0);
    } else {
        return (uint16_t)BKP_ReadBackupRegister(pios_bkp_registers_map[regnumber]);
    }
}

void PIOS_BKP_WriteRegister(uint32_t regnumber, uint16_t data)
{
    if (PIOS_BKP_REGISTERS_COUNT < regnumber) {
        PIOS_Assert(0);
    } else {
        BKP_WriteBackupRegister(pios_bkp_registers_map[regnumber], (uint32_t)data);
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
