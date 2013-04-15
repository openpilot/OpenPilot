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
#include "hwsettings.h"
#include "systemmod.h"

/* Task Priorities */
#define PRIORITY_TASK_HOOKS             (tskIDLE_PRIORITY + 3)

/* Global Variables */

/* Prototype of PIOS_Board_Init() function */
extern void PIOS_Board_Init(void);
extern void Stack_Change(void);

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
	PIOS_Flash_Jedec_EraseChip();
#if defined(PIOS_LED_HEARTBEAT)
	PIOS_LED_Off(PIOS_LED_HEARTBEAT);
#endif	/* PIOS_LED_HEARTBEAT */
	while (1) ;
#endif

	/* Initialize modules */
	MODULE_INITIALISE_ALL

	/* swap the stack to use the IRQ stack */
	Stack_Change();

	/* Start the FreeRTOS scheduler, which should never return.
	 *
	 * NOTE: OpenPilot runs an operating system (FreeRTOS), which constantly calls 
	 * (schedules) function files (modules). These functions never return from their
	 * while loops, which explains why each module has a while(1){} segment. Thus, 
	 * the OpenPilot software actually starts at the vTaskStartScheduler() function, 
	 * even though this is somewhat obscure.
	 *
	 * In addition, there are many main() functions in the OpenPilot firmware source tree
	 * This is because each main() refers to a separate hardware platform. Of course,
	 * C only allows one main(), so only the relevant main() function is compiled when 
	 * making a specific firmware.
	 *
	 */
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be running. */

	/* Do some indication to user that something bad just happened */
	while (1) {
#if defined(PIOS_LED_HEARTBEAT)
		PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
#endif	/* PIOS_LED_HEARTBEAT */
		PIOS_DELAY_WaitmS(100);
	}

	return 0;
}

/**
 * @}
 * @}
 */

