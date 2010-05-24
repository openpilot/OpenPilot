/**
 ******************************************************************************
 *
 * @file       pios_opahrs.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      IRQ Enable/Disable routines
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_OPAHRS OPAHRS Functions
 * @{
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
#include "pios.h"

#if defined(PIOS_INCLUDE_OPAHRS)

/**
* Initialise the OpenPilot AHRS
*/
void PIOS_OPAHRS_Init(void)
{
}

void PIOS_OPAHRS_ReadAttitude(void)
{
}

int32_t PIOS_OPAHRS_Read(uint8_t address, uint8_t *buffer, uint8_t len)
{
	return 0;
}

int32_t PIOS_OPAHRS_Write(uint8_t address, uint8_t buffer)
{
	return 0;
}

#endif /* PIOS_INCLUDE_OPAHRS */
