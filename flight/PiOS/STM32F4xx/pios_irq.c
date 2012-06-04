/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_IRQ IRQ Setup Functions
 * @brief STM32 Hardware code to enable and disable interrupts
 * @{
 *
 * @file       pios_irq.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012
 * @brief      IRQ Enable/Disable routines
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

/*
 * @todo This should be shared with the F1xx code.
 */

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_IRQ)

/* Private Function Prototypes */

/* Local Variables */
/* The nesting counter ensures, that interrupts won't be enabled so long nested functions disable them */
static uint32_t nested_ctr;

/* Stored priority level before IRQ has been disabled (important for co-existence with vPortEnterCritical) */
static uint32_t prev_primask;

/**
* Disables all interrupts (nested)
* \return < 0 On errors
*/
int32_t PIOS_IRQ_Disable(void)
{
	/* Get current priority if nested level == 0 */
	if (!nested_ctr) {
		__asm volatile ("   mrs %0, primask\n":"=r" (prev_primask)
		    );
	}

	/* Disable interrupts */
	__asm volatile ("     mov r0, #1     \n" "     msr primask, r0\n":::"r0");

	++nested_ctr;

	/* No error */
	return 0;
}

/**
* Enables all interrupts (nested)
* \return < 0 on errors
* \return -1 on nesting errors (PIOS_IRQ_Disable() hasn't been called before)
*/
int32_t PIOS_IRQ_Enable(void)
{
	/* Check for nesting error */
	if (nested_ctr == 0) {
		/* Nesting error */
		return -1;
	}

	/* Decrease nesting level */
	--nested_ctr;

	/* Set back previous priority once nested level reached 0 again */
	if (nested_ctr == 0) {
		__asm volatile ("   msr primask, %0\n"::"r" (prev_primask)
		    );
	}

	/* No error */
	return 0;
}

#endif

/**
  * @}
  * @}
  */
