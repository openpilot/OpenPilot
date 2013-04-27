/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_RTC RTC Functions
 * @{
 *
 * @file       pios_rtc.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RTC functions header.
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

#ifndef PIOS_RTC_H
#define PIOS_RTC_H

#include <stdbool.h>

/* Public Functions */
extern uint32_t PIOS_RTC_Counter();
extern float PIOS_RTC_Rate();
extern float PIOS_RTC_MsPerTick();
extern bool PIOS_RTC_RegisterTickCallback(void (*fn)(uint32_t id), uint32_t data);

#endif /* PIOS_RTC_H */

/**
 * @}
 * @}
 */
