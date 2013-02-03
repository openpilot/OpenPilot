/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SYS System Functions
 * @brief PIOS System Initialization code
 * @{
 *
 * @file       pios_sys.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      System and hardware Init functions header.
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

#ifndef PIOS_SYS_H
#define PIOS_SYS_H

#define PIOS_SYS_SERIAL_NUM_BINARY_LEN 12
#define PIOS_SYS_SERIAL_NUM_ASCII_LEN (PIOS_SYS_SERIAL_NUM_BINARY_LEN * 2)

/* Public Functions */
extern void PIOS_SYS_Init(void);
extern int32_t PIOS_SYS_Reset(void);
extern uint32_t PIOS_SYS_getCPUFlashSize(void);
extern int32_t PIOS_SYS_SerialNumberGetBinary(uint8_t array[PIOS_SYS_SERIAL_NUM_BINARY_LEN]);
extern int32_t PIOS_SYS_SerialNumberGet(char str[PIOS_SYS_SERIAL_NUM_ASCII_LEN+1]);

#endif /* PIOS_SYS_H */

/**
  * @}
  * @}
  */
