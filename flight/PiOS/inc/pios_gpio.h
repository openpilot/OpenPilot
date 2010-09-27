/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @defgroup   PIOS_GPIO GPIO Functions
 * @brief GPIO hardware code for STM32
 * @{
 *
 * @file       pios_gpio.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      GPIO functions header.
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

#ifndef PIOS_GPIO_H
#define PIOS_GPIO_H

/* Public Functions */
extern void PIOS_GPIO_Init(void);
extern void PIOS_GPIO_Enable(uint8_t Pin);
extern void PIOS_GPIO_On(uint8_t Pin);
extern void PIOS_GPIO_Off(uint8_t Pin);
extern void PIOS_GPIO_Toggle(uint8_t Pin);

#endif /* PIOS_GPIO_H */

/**
  * @}
  * @}
  */
