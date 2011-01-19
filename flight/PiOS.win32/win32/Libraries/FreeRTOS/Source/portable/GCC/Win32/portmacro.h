/*
	FreeRTOS.org V5.2.0 - Copyright (C) 2003-2009 Richard Barry.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify it
	under the terms of the GNU General Public License (version 2) as published
	by the Free Software Foundation and modified by the FreeRTOS exception.

	FreeRTOS.org is distributed in the hope that it will be useful,	but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
	FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
	more details.

	You should have received a copy of the GNU General Public License along
	with FreeRTOS.org; if not, write to the Free Software Foundation, Inc., 59
	Temple Place, Suite 330, Boston, MA  02111-1307  USA.

	A special exception to the GPL is included to allow you to distribute a
	combined work that includes FreeRTOS.org without being obliged to provide
	the source code for any proprietary components.  See the licensing section
	of http://www.FreeRTOS.org for full details.


	***************************************************************************
	*                                                                         *
	* Get the FreeRTOS eBook!  See http://www.FreeRTOS.org/Documentation      *
	*                                                                         *
	* This is a concise, step by step, 'hands on' guide that describes both   *
	* general multitasking concepts and FreeRTOS specifics. It presents and   *
	* explains numerous examples that are written using the FreeRTOS API.     *
	* Full source code for all the examples is provided in an accompanying    *
	* .zip file.                                                              *
	*                                                                         *
	***************************************************************************

	1 tab == 4 spaces!

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting,
	licensing and training services.
*/


#ifndef PORTMACRO_H
#define PORTMACRO_H

#define _WIN32_WINNT 0x600

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>
#include "cpuemu.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR		char
#define portFLOAT		float
#define portDOUBLE		double
#define portLONG		int
#define portSHORT		short
#define portSTACK_TYPE  unsigned long
#define portBASE_TYPE   long

#if( configUSE_16_BIT_TICKS == 1 )
	typedef unsigned portSHORT portTickType;
	#define portMAX_DELAY ( portTickType ) 0xffff
#else
	typedef unsigned portLONG portTickType;
	#define portMAX_DELAY ( portTickType ) 0xffffffff
#endif
/*-----------------------------------------------------------*/

/* Hardware specifics. */
#define portSTACK_GROWTH			( -1 )
#define portTICK_RATE_MS				( ( portTickType ) 1000 / configTICK_RATE_HZ )
#define portTICK_RATE_MICROSECONDS		( ( portTickType ) 1000000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT			4

#define portYIELD()			__swi();
#define portDISABLE_INTERRUPTS()	__disable_interrupt()
#define portENABLE_INTERRUPTS()		__enable_interrupt()

/* Critical section handling. */
#define portENTER_CRITICAL()		vPortEnterCritical()
#define portEXIT_CRITICAL()			vPortExitCritical()

/* Task utilities. */
#define portEND_SWITCHING_ISR( xSwitchRequired ) 	\
{													\
extern void vTaskSwitchContext( void );				\
														\
	if( xSwitchRequired ) 							\
	{												\
		vTaskSwitchContext();						\
	}												\
}

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

#define portNOP()

#define portOUTPUT_BYTE( a, b )

#define traceTASK_DELETE( pxTaskToDelete )		__delete_task( pxTaskToDelete )

#define traceTASK_CREATE( pxNewTCB )

/* Make use of times(man 2) to gather run-time statistics on the tasks. */
extern void vPortFindTicksPerSecond( void );
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()	vPortFindTicksPerSecond()		/* Nothing to do because the timer is already present. */
extern unsigned long ulPortGetTimerValue( void );
#define portGET_RUN_TIME_COUNTER_VALUE()			ulPortGetTimerValue() /* Query the System time stats for this process. */

/* Assign a handler to an interrupt no */
int iPortSetIsrHandler(int iNo, void (*handler)(void));

/* enable specified interrupt no */
void vPortEnableInt(int iNo);

/* disable specified interrupt no */
void vPortDisableInt(int iNo);

/* query interrupt enabled status */
BOOL bPortIsEnabledInt(int iNo);

/* global interrupt disable */
void __disable_interrupt(void);

/* global interrupt enable */
void __enable_interrupt(void);

/* generate software interrupt - non - maskable */
void __swi(void);

/* generate an interrupt which causes the scheduler to delete the task */
void __delete_task(void *tcb);

/* To be used by interrupt generation threads. e.g. Create a thread that
monitors console events and generate an interrupt whenever a key is pressed.
See also implimentation of tick_generator.
*/
void __generate_interrupt(int iNo);

void vPortEnterCritical( void );
void vPortExitCritical( void );

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

