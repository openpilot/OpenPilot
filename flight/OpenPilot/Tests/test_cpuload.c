/**
 ******************************************************************************
 *
 * @file       test_cpuload.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Calibration for the CPU load calculation
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

/**
 * This file is used to calculate the number of counts we get when
 * the CPU is fully idle (no load). This is used by systemmod.c to
 * calculate the CPU load. This calibration should be run whenever
 * changes are made in the FreeRTOS configuration or on the compiler
 * optimisation parameters.
 */

#include "openpilot.h"

// Local constants
#define BENCHMARK_DURATION_MS 10000

// Local functions
static void testTask(void *pvParameters);

// Variables
static uint32_t idleCounter = 0;
static uint32_t idleCounterClear = 0;

int main()
{
	PIOS_SYS_Init();

	// Create test task
	xTaskCreate(testTask, (signed portCHAR *)"Test", 1000 , NULL, 1, NULL);

	// Start the FreeRTOS scheduler
	vTaskStartScheduler();
	return 0;
}

static void testTask(void *pvParameters)
{
	uint32_t countsPerSecond = 0;

	while (1)
	{
		// Delay until enough required test duration
		vTaskDelay( BENCHMARK_DURATION_MS / portTICK_RATE_MS );

		// Calculate counts per second, set breakpoint here
		countsPerSecond = idleCounter / (BENCHMARK_DURATION_MS/1000);

		// Reset and start again - do not clear idleCounter directly!
		// SET BREAKPOINT HERE and read the countsPerSecond variable
		// this should be used to update IDLE_COUNTS_PER_SEC_AT_NO_LOAD in systemmod.c
		idleCounterClear = 1;
	}
}

void vApplicationIdleHook(void)
{
	/* Called when the scheduler has no tasks to run */
	if (idleCounterClear == 0)
	{
		++idleCounter;
	}
	else
	{
		idleCounter = 0;
		idleCounterClear = 0;
	}
}
