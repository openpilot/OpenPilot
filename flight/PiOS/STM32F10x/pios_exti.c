/**
 ******************************************************************************
 *
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_EXTI External Interrupt Handlers
 * @brief External interrupt handler functions
 * @note Currently deals with BMP085 readings
 * @{
 *
 * @file       pios_exti.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      External Interrupt Handlers
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_EXTI)

/**
* Handle external lines 15 to 10 interrupt requests
*/
void EXTI15_10_IRQHandler(void)
{
#if defined(PIOS_INCLUDE_FREERTOS)
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
#endif

#if defined(PIOS_INCLUDE_BMP085)
	if (EXTI_GetITStatus(PIOS_BMP085_EOC_EXTI_LINE) != RESET) {
		/* Read the ADC Value */
#if defined(PIOS_INCLUDE_FREERTOS)
		xSemaphoreGiveFromISR(PIOS_BMP085_EOC, &xHigherPriorityTaskWoken);
#else
		PIOS_BMP085_EOC=1;
#endif

		/* Clear the EXTI line pending bit */
		EXTI_ClearITPendingBit(PIOS_BMP085_EOC_EXTI_LINE);
	}
#endif

#if defined(PIOS_INCLUDE_FREERTOS)
	/* Yield From ISR if needed */
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
#endif
}

/**
* Handle external lines 9 to 5 interrupt requests
*/
extern void PIOS_HMC5843_IRQHandler(void);
void EXTI9_5_IRQHandler(void)
{
#if defined(PIOS_INCLUDE_HMC5843)
	if (EXTI_GetITStatus(PIOS_HMC5843_DRDY_EXTI_LINE) != RESET) {
		PIOS_HMC5843_IRQHandler();
		EXTI_ClearITPendingBit(PIOS_HMC5843_DRDY_EXTI_LINE);
	}
#endif
}

/**
* Handle external line 4 interrupt requests
*/
#if defined(PIOS_INCLUDE_USB)
void EXTI4_IRQHandler(void)
{
	if (EXTI_GetITStatus(PIOS_USB_DETECT_EXTI_LINE) != RESET) {
		/* Clear the EXTI line pending bit */
		EXTI_ClearITPendingBit(PIOS_USB_DETECT_EXTI_LINE);
	}
}
#endif
#endif

/**
  * @}
  * @}
  */
