/**
 ******************************************************************************
 *
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_EXTI External Interrupt Handlers
 * @{
 *
 * @file       pios_exti.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      External Interrupts Handlers header.
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

#ifndef PIOS_EXTI_H
#define PIOS_EXTI_H

/* Public Functions */

#include <pios_stm32.h>

struct pios_exti_cfg {
	bool (* vector)(void);
	uint32_t line;		/* use EXTI_LineN macros */
	struct stm32_gpio pin;
	struct stm32_irq irq;
	struct stm32_exti exti;
};

/* must be added to any pios_exti_cfg definition for it to be valid */
#define __exti_config	__attribute__((section("_exti")))

extern int32_t PIOS_EXTI_Init(const struct pios_exti_cfg * cfg);

#endif /* PIOS_EXTI_H */

/**
  * @}
  * @}
  */
