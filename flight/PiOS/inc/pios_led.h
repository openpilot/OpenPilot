/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_LED LED Functions
 * @{
 *
 * @file       pios_led.h   
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      LED functions header.
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

#ifndef PIOS_LED_H
#define PIOS_LED_H

/* Type Definitions */
#if (PIOS_LED_NUM == 1)
typedef enum { LED1 = 0 } LedTypeDef;
#elif (PIOS_LED_NUM == 2)
typedef enum { LED1 = 0, LED2 = 1 } LedTypeDef;
#elif (PIOS_LED_NUM == 3)
typedef enum { LED1 = 0, LED2 = 1, LED3 = 2 } LedTypeDef;
#elif (PIOS_LED_NUM == 4)
typedef enum { LED1 = 0, LED2 = 1, LED3 = 2, LED4 = 3 } LedTypeDef;
#else
#error PIOS_LED_NUM not defined
#endif

/* Public Functions */
extern void PIOS_LED_Init(void);
extern void PIOS_LED_On(LedTypeDef LED);
extern void PIOS_LED_Off(LedTypeDef LED);
extern void PIOS_LED_Toggle(LedTypeDef LED);

#endif /* PIOS_LED_H */
