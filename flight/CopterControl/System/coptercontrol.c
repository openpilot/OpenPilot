/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @brief These files are the core system files of OpenPilot.
 * They are the ground layer just above PiOS. In practice, OpenPilot actually starts
 * in the main() function of openpilot.c
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @brief This is where the OP firmware starts. Those files also define the compile-time
 * options of the firmware.
 * @{
 * @file       openpilot.c 
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Sets up and runs main OpenPilot tasks.
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


/* OpenPilot Includes */
#include "openpilot.h"
#include "uavobjectsinit.h"
#include "systemmod.h"

/* Task Priorities */
#define PRIORITY_TASK_HOOKS             (tskIDLE_PRIORITY + 3)

/* Global Variables */

/* Prototype of generated InitModules() function */
extern void InitModules(void);

/* Prototype of PIOS_Board_Init() function */
extern void PIOS_Board_Init(void);

/* Init module section */
extern initcall_t __module_initcall_start[], __module_initcall_end[];

/**
* OpenPilot Main function:
*
* Initialize PiOS<BR>
* Create the "System" task (SystemModInitializein Modules/System/systemmod.c) <BR>
* Start FreeRTOS Scheduler (vTaskStartScheduler) (Now handled by caller)
* If something goes wrong, blink LED1 and LED2 every 100ms
*
*/
int main()
{
	/* NOTE: Do NOT modify the following start-up sequence */
	/* Any new initialization functions should be added in OpenPilotInit() */

	/* Brings up System using CMSIS functions, enables the LEDs. */
	PIOS_SYS_Init();

	/* Architecture dependant Hardware and
	 * core subsystem initialisation
	 * (see pios_board.c for your arch)
	 * */
	PIOS_Board_Init();

#ifdef ERASE_FLASH
	PIOS_Flash_W25X_EraseChip();
	while(TRUE){};
#endif

	/* Initialize modules */
	/* TODO: add id so we can parse this list later and replace module on the fly */
	/* property flag will be add to give information like:
	 *  - importance of the module (can be dropped or required for flight
	 *  - parameter to enable feature at run-time (based on user setup on GCS)
	 *  All this will be handled by bootloader. this section will add a function pointer
	 *  and a pointer to a 32 bit in RAM with all the flags.
	 *  Limited on CC this could be mapped on RAM for OP so we can grow the list at run-time.
	 */
	initcall_t *fn;
	int32_t ret;

	/* this one needs to be done first */
	SystemModInitialize();

	for (fn = __module_initcall_start; fn < __module_initcall_end; fn++)
			ret = (*fn)();

#if defined(ARCH_POSIX) || defined(ARCH_WIN32)
	/* Start the FreeRTOS scheduler which never returns.*/
	/* only do this for posix and win32 since the caller will take care
	 * of starting the scheduler and increase the heap and swith back to
	 * MSP stack. (all arch specific is hidden from here and take care by reset handler)
	 */
    vTaskStartScheduler();
#endif

    return 0;
}

/**
 * @}
 * @}
 */

