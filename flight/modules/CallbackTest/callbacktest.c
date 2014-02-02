/**
 ******************************************************************************
 *
 * @file       callbacktest.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Example module to be used as a template for actual modules.
 *             Event callback version.
 *
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
 *
 * This is a test suite to test the callback scheduler,
 * including its forward scheduling ability
 *
 */

#include "openpilot.h"

// Private constants
#define STACK_SIZE        configMINIMAL_STACK_SIZE
#define CALLBACK_PRIORITY CALLBACK_PRIORITY_LOW
#define TASK_PRIORITY     CALLBACK_TASKPRIORITY_AUXILIARY
// Private types

// #define DEBUGPRINT(...) fprintf (stderr, __VA_ARGS__)
#define DEBUGPRINT(...) xSemaphoreTakeRecursive(mutex, portMAX_DELAY); fprintf(stderr, __VA_ARGS__); xSemaphoreGiveRecursive(mutex);

static xSemaphoreHandle mutex;

// Private variables
static DelayedCallbackInfo *cbinfo[10];

static volatile int32_t counter[10] = { 0 };

static void DelayedCb0();
static void DelayedCb1();
static void DelayedCb2();
static void DelayedCb3();
static void DelayedCb4();
static void DelayedCb5();
static void DelayedCb6();
/**
 * Initialise the module, called on startup.
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t CallbackTestInitialize()
{
    mutex     = xSemaphoreCreateRecursiveMutex();

    cbinfo[0] = PIOS_CALLBACKSCHEDULER_Create(&DelayedCb0, CALLBACK_PRIORITY_LOW, tskIDLE_PRIORITY + 2, -1, STACK_SIZE);
    cbinfo[1] = PIOS_CALLBACKSCHEDULER_Create(&DelayedCb1, CALLBACK_PRIORITY_LOW, tskIDLE_PRIORITY + 2, -1, STACK_SIZE);
    cbinfo[2] = PIOS_CALLBACKSCHEDULER_Create(&DelayedCb2, CALLBACK_PRIORITY_CRITICAL, tskIDLE_PRIORITY + 2, -1, STACK_SIZE);
    cbinfo[3] = PIOS_CALLBACKSCHEDULER_Create(&DelayedCb3, CALLBACK_PRIORITY_CRITICAL, tskIDLE_PRIORITY + 2, -1, STACK_SIZE);
    cbinfo[4] = PIOS_CALLBACKSCHEDULER_Create(&DelayedCb4, CALLBACK_PRIORITY_LOW, tskIDLE_PRIORITY + 2, -1, STACK_SIZE);
    cbinfo[5] = PIOS_CALLBACKSCHEDULER_Create(&DelayedCb5, CALLBACK_PRIORITY_LOW, tskIDLE_PRIORITY + 2, -1, STACK_SIZE);
    cbinfo[6] = PIOS_CALLBACKSCHEDULER_Create(&DelayedCb6, CALLBACK_PRIORITY_LOW, tskIDLE_PRIORITY + 20, -1, STACK_SIZE);


    return 0;
}
int32_t CallbackTestStart()
{
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    PIOS_CALLBACKSCHEDULER_Dispatch(cbinfo[3]);
    PIOS_CALLBACKSCHEDULER_Dispatch(cbinfo[2]);
    PIOS_CALLBACKSCHEDULER_Dispatch(cbinfo[1]);
    PIOS_CALLBACKSCHEDULER_Dispatch(cbinfo[0]);
    // different callback priorities within a taskpriority
    PIOS_CALLBACKSCHEDULER_Schedule(cbinfo[4], 30000, CALLBACK_UPDATEMODE_NONE);
    PIOS_CALLBACKSCHEDULER_Schedule(cbinfo[4], 5000, CALLBACK_UPDATEMODE_OVERRIDE);
    PIOS_CALLBACKSCHEDULER_Schedule(cbinfo[4], 4000, CALLBACK_UPDATEMODE_SOONER);
    PIOS_CALLBACKSCHEDULER_Schedule(cbinfo[4], 10000, CALLBACK_UPDATEMODE_SOONER);
    PIOS_CALLBACKSCHEDULER_Schedule(cbinfo[4], 1000, CALLBACK_UPDATEMODE_LATER);
    PIOS_CALLBACKSCHEDULER_Schedule(cbinfo[4], 4800, CALLBACK_UPDATEMODE_LATER);
    PIOS_CALLBACKSCHEDULER_Schedule(cbinfo[4], 48000, CALLBACK_UPDATEMODE_NONE);
    // should be at 4.8 seconds after this, allowing for exactly 9 prints of the following
    PIOS_CALLBACKSCHEDULER_Schedule(cbinfo[5], 500, CALLBACK_UPDATEMODE_NONE);
    // delayed counter with 500 ms
    PIOS_CALLBACKSCHEDULER_Dispatch(cbinfo[6]);
    // high task prio
    xSemaphoreGiveRecursive(mutex);
    return 0;
}

static void DelayedCb0()
{
    DEBUGPRINT("delayed counter low prio 0 updated: %i\n", counter[0]);
    if (++counter[0] < 10) {
        PIOS_CALLBACKSCHEDULER_Dispatch(cbinfo[0]);
    }
}
static void DelayedCb1()
{
    DEBUGPRINT("delayed counter low prio 1 updated: %i\n", counter[1]);
    if (++counter[1] < 10) {
        PIOS_CALLBACKSCHEDULER_Dispatch(cbinfo[1]);
    }
}
static void DelayedCb2()
{
    DEBUGPRINT("delayed counter high prio 2 updated: %i\n", counter[2]);
    if (++counter[2] < 10) {
        PIOS_CALLBACKSCHEDULER_Dispatch(cbinfo[2]);
    }
}
static void DelayedCb3()
{
    DEBUGPRINT("delayed counter high prio 3 updated: %i\n", counter[3]);
    if (++counter[3] < 10) {
        PIOS_CALLBACKSCHEDULER_Dispatch(cbinfo[3]);
    }
}
static void DelayedCb4()
{
    DEBUGPRINT("delayed scheduled callback 4 reached!\n");
    exit(0);
}
static void DelayedCb5()
{
    DEBUGPRINT("delayed scheduled counter 5 updated: %i\n", counter[5]);
    if (++counter[5] < 10) {
        PIOS_CALLBACKSCHEDULER_Schedule(cbinfo[5], 500, CALLBACK_UPDATEMODE_NONE);
    }
    // it will likely only reach 8 due to cb4 aborting the run
}
static void DelayedCb6()
{
    DEBUGPRINT("delayed counter 6 (high task prio) updated: %i\n", counter[6]);
    if (++counter[6] < 10) {
        PIOS_CALLBACKSCHEDULER_Dispatch(cbinfo[6]);
    }
}
