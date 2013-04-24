/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_HCSR04 HCSR04 Functions
 * @brief Hardware functions to deal with the altitude sonar sensor
 * @{
 *
 * @file       pios_hcsr04.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      HCSR04 functions header.
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

#ifndef PIOS_HCSR04_H
#define PIOS_HCSR04_H

/* Public Functions */
extern void PIOS_HCSR04_Init(void);
extern int32_t PIOS_HCSR04_Get(void);
extern int32_t PIOS_HCSR04_Completed(void);
extern void PIOS_HCSR04_Trigger(void);

#endif /* PIOS_HCSR04_H */

/** 
  * @}
  * @}
  */
