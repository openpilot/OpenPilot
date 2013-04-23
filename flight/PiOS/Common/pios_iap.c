/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_IAP IAP Functions
 * @brief Common IAP functionality
 * @{
 *
 * @file       pios_iap.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      In application programming functions
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

/****************************************************************************************
 *  Header files
 ****************************************************************************************/
#include <pios.h>
#include <pios_bkp.h>

#ifdef PIOS_INCLUDE_IAP

/****************************************************************************************
 *  Private Definitions/Macros
 ****************************************************************************************/

/* these definitions reside here for protection and privacy. */
#define IAP_MAGIC_WORD_1	0x1122
#define IAP_MAGIC_WORD_2	0xAA55

#define UPPERWORD16(lw)	(uint16_t)((uint32_t)(lw)>>16)
#define LOWERWORD16(lw)	(uint16_t)((uint32_t)(lw)&0x0000ffff)
#define UPPERBYTE(w)	(uint8_t)((w)>>8)
#define LOWERBYTE(w)	(uint8_t)((w)&0x00ff)

/****************************************************************************************
 *  Private Functions
 ****************************************************************************************/

/****************************************************************************************
 *  Private (static) Data
 ****************************************************************************************/
const uint16_t pios_iap_cmd_list[] = 
	{ 
		IAP_CMD1, 
		IAP_CMD2, 
		IAP_CMD3
	};
/****************************************************************************************
 *  Public/Global Data
 ****************************************************************************************/

/*!
 * \brief	PIOS_IAP_Init - performs required initializations for iap module.
 * \param   none.
 * \return	none.
 * \retval	none.
 *
 *	Created: Sep 8, 2010 10:10:48 PM by joe
 */
void PIOS_IAP_Init( void )
{
	PIOS_BKP_Init();
	PIOS_BKP_EnableWrite();
}

/*!
 * \brief     Determines if an In-Application-Programming request has been made.
 * \param   *comm - Which communication stream to use for the IAP (USB, Telemetry, I2C, SPI, etc)
 * \return    TRUE - if correct sequence found, along with 'comm' updated.
 * 			FALSE - Note that 'comm' will have an invalid comm identifier.
 * \retval
 *
 */
uint32_t	PIOS_IAP_CheckRequest( void )
{
	uint32_t	retval = false;
	uint16_t	reg1;
	uint16_t	reg2;

	reg1 = PIOS_BKP_ReadRegister(MAGIC_REG_1);
	reg2 = PIOS_BKP_ReadRegister(MAGIC_REG_2);

	if( reg1 == IAP_MAGIC_WORD_1 && reg2 == IAP_MAGIC_WORD_2 ) {
		// We have a match.
		retval = true;
	} else {
		retval = false;
	}
	return retval;
}



/*!
 * \brief   Sets the 1st word of the request sequence.
 * \param   n/a
 * \return  n/a
 * \retval
 */
void	PIOS_IAP_SetRequest1(void)
{
	PIOS_BKP_WriteRegister(MAGIC_REG_1, IAP_MAGIC_WORD_1);
}

void	PIOS_IAP_SetRequest2(void)
{
	PIOS_BKP_WriteRegister(MAGIC_REG_2, IAP_MAGIC_WORD_2);
}

void	PIOS_IAP_ClearRequest(void)
{
	PIOS_BKP_WriteRegister(MAGIC_REG_1, 0);
	PIOS_BKP_WriteRegister(MAGIC_REG_2, 0);
}

uint16_t PIOS_IAP_ReadBootCount(void)
{
	return PIOS_BKP_ReadRegister(IAP_BOOTCOUNT);
}

void PIOS_IAP_WriteBootCount (uint16_t boot_count)
{
	PIOS_BKP_WriteRegister(IAP_BOOTCOUNT, boot_count);
}

/**
  * @brief  Return one of the IAP command values passed from bootloader.
  * @param  number: the index of the command value (0..2).
  * @retval the selected command value.
  */
uint32_t PIOS_IAP_ReadBootCmd(uint8_t number)
{
	if(PIOS_IAP_CMD_COUNT < number)
	{
		PIOS_Assert(0);
	}
	else
	{
		return PIOS_BKP_ReadRegister(pios_iap_cmd_list[number]);
	}	
}

/**
  * @brief  Write one of the IAP command values to be passed to firmware from bootloader.
  * @param  number: the index of the command value (0..2).
  * @param  value: value to be written.
  */
void PIOS_IAP_WriteBootCmd(uint8_t number, uint32_t value)
{
	if(PIOS_IAP_CMD_COUNT < number)
	{
		PIOS_Assert(0);
	}
	else
	{
		PIOS_BKP_WriteRegister(pios_iap_cmd_list[number], value);
	}	
}

#endif /* PIOS_INCLUDE_IAP */
