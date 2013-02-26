
/**
 ******************************************************************************
 *
 * @file       main.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Main modem functions
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


// *****************************************************************************

//#define USE_WATCHDOG                    // comment this out if you don't want to use the watchdog

// *****************************************************************************
// OpenPilot Includes

#include <string.h>

#include "openpilot.h"
#include "systemmod.h"

/* Task Priorities */
#define PRIORITY_TASK_HOOKS             (tskIDLE_PRIORITY + 3)

/* Global Variables */

/* Prototype of PIOS_Board_Init() function */
extern void PIOS_Board_Init(void);
extern void Stack_Change(void);
static void Stack_Change_Weak () __attribute__ ((weakref ("Stack_Change")));


/* Function Prototypes */
static void initTask(void *parameters);
/* Local Variables */
#define INIT_TASK_PRIORITY	(tskIDLE_PRIORITY + configMAX_PRIORITIES - 1)	// max priority
#define INIT_TASK_STACK		(1024 / 4)										// XXX this seems excessive
static xTaskHandle initTaskHandle;

// *****************************************************************************
// Global Variables

// *****************************************************************************
// Local Variables

#if defined(USE_WATCHDOG)
  volatile uint16_t     watchdog_timer;
  uint16_t              watchdog_delay;
#endif

// *****************************************************************************

#if defined(USE_WATCHDOG)

  void processWatchdog(void)
  {
      // random32 = UpdateCRC32(random32, IWDG->SR);

      if (watchdog_timer < watchdog_delay)
          return;

      // the watchdog needs resetting

      watchdog_timer = 0;

      watchdog_Clear();
  }

  void enableWatchdog(void)
  {	// enable a watchdog
      watchdog_timer = 0;
      watchdog_delay = watchdog_Init(1000);	// 1 second watchdog timeout
  }

#endif

// *****************************************************************************

void sequenceLEDs(void)
{
    for (int i = 0; i < 2; i++)
    {
        //USB_LED_ON;
        PIOS_DELAY_WaitmS(100);
        //USB_LED_OFF;
        PIOS_DELAY_WaitmS(100);

         #if defined(USE_WATCHDOG)
            processWatchdog();		// process the watchdog
        #endif
    }
}

// *****************************************************************************
// find out what caused our reset and act on it

void processReset(void)
{
    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
    {	// Independant Watchdog Reset

        #if defined(PIOS_COM_DEBUG)
            DEBUG_PRINTF("\r\nINDEPENDANT WATCHDOG CAUSED A RESET\r\n");
        #endif

        // all led's ON
        //USB_LED_ON;


        PIOS_DELAY_WaitmS(500);	// delay a bit

        // all led's OFF
        //USB_LED_OFF;


    }
/*
	if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET)
	{	// Window Watchdog Reset

		DEBUG_PRINTF("\r\nWINDOW WATCHDOG CAUSED A REBOOT\r\n");

		// all led's ON
		USB_LED_ON;
		LINK_LED_ON;
		RX_LED_ON;
		TX_LED_ON;

		PIOS_DELAY_WaitmS(500);	// delay a bit

		// all led's OFF
		USB_LED_OFF;
		LINK_LED_OFF;
		RX_LED_OFF;
		TX_LED_OFF;
	}
*/
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
    {	// Power-On Reset
        #if defined(PIOS_COM_DEBUG)
            DEBUG_PRINTF("\r\nPOWER-ON-RESET\r\n");
        #endif
    }

    if (RCC_GetFlagStatus(RCC_FLAG_SFTRST) != RESET)
    {	// Software Reset
        #if defined(PIOS_COM_DEBUG)
            DEBUG_PRINTF("\r\nSOFTWARE RESET\r\n");
        #endif
    }

    if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST) != RESET)
    {	// Low-Power Reset
        #if defined(PIOS_COM_DEBUG)
            DEBUG_PRINTF("\r\nLOW POWER RESET\r\n");
        #endif
    }

    if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
    {	// Pin Reset
        #if defined(PIOS_COM_DEBUG)
            DEBUG_PRINTF("\r\nPIN RESET\r\n");
        #endif
    }

    // Clear reset flags
    //RCC_ClearFlag();
}

int main()
{
	int	result;
    // *************
    // init various variables
    // *************
	/* NOTE: Do NOT modify the following start-up sequence */
	/* Any new initialization functions should be added in OpenPilotInit() */
	vPortInitialiseBlocks();

	// Bring up System using CMSIS functions, enables the LEDs.
	PIOS_SYS_Init();

	/* For Revolution we use a FreeRTOS task to bring up the system so we can */
	/* always rely on FreeRTOS primitive */
	result = xTaskCreate(initTask, (const signed char *)"init",
						 INIT_TASK_STACK, NULL, INIT_TASK_PRIORITY,
						 &initTaskHandle);
	PIOS_Assert(result == pdPASS);

	/* Start the FreeRTOS scheduler which should never returns.*/
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be running. */
	/* Do some indication to user that something bad just happened */
	PIOS_LED_Off(PIOS_LED_HEARTBEAT); \
	for(;;) { \
		PIOS_LED_Toggle(PIOS_LED_HEARTBEAT); \
		PIOS_DELAY_WaitmS(100); \
	};

	return 0;
}

/**
 * Initialisation task.
 *
 * Runs board and module initialisation, then terminates.
 */
void
initTask(void *parameters)
{
	/* board driver init */
	PIOS_Board_Init();

	/* Initialize modules */
	MODULE_INITIALISE_ALL;

	/* terminate this task */
	vTaskDelete(NULL);
}

// *****************************************************************************
