/**
 ******************************************************************************
 *
 * @file       callbackscheduler.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include files of the uavobjectlist library
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

#ifndef CALLBACKSCHEDULER_H
#define CALLBACKSCHEDULER_H

// Public types
typedef enum{
	CALLBACK_PRIORITY_CRITICAL = 0,
	CALLBACK_PRIORITY_REGULAR  = 1,
	CALLBACK_PRIORITY_LOW      = 2
	} DelayedCallbackPriority;

typedef enum{
	CALLBACK_UPDATEMODE_NONE     = 0,
	CALLBACK_UPDATEMODE_SOONER   = 1,
	CALLBACK_UPDATEMODE_LATER    = 2,
	CALLBACK_UPDATEMODE_OVERRIDE = 3
	} DelayedCallbackUpdateMode;

typedef void (*DelayedCallback)(void);

struct DelayedCallbackInfoStruct;
typedef struct DelayedCallbackInfoStruct DelayedCallbackInfo;

// Public functions
//

/**
 * Initialize the scheduler
 * must be called before any other functions are called
 * \return Success (0), failure (-1)
 */
int32_t CallbackSchedulerInitialize();

/**
 * Start all scheduler tasks
 * Will instantiate all scheduler tasks registered so far.  Although new
 * callbacks CAN be registered beyond that point, any further scheduling tasks
 * will be started the moment of instantiation.  It is not possible to increase
 * the STACK requirements of a scheduler task after this function has been
 * run.  No callbacks will be run before this function is called, although
 * they can be marked for later execution by executing the dispatch function.
 * \return Success (0), failure (-1)
 */
int32_t CallbackSchedulerStart();

/**
 * Register a new callback to be called by a delayed callback scheduler task.
 * If a scheduler task with the specified taskPriority does not exist yet, it
 * will be created.
 * \param[in] cb The callback to be invoked
 * \param[in] priority Priority of the callback compared to other callbacks called by the same delayed callback scheduler task.
 * \param[in] taskPriority Priority of the entire scheduler task. One scheduler task will be spawned for each distinct task priority in use, further callbacks with the same task priority will be handled by the same delayed callback scheduler task, according to their callback priority.
 * \param[in] stacksize The stack requirements of the callback when called by the scheduler.
 * \return CallbackInfo Pointer on success, NULL if failed.
 */
DelayedCallbackInfo *DelayedCallbackCreate(
	DelayedCallback cb,
	DelayedCallbackPriority priority,
	long taskPriority,
	uint32_t stacksize
	);

/**
 * Schedule dispatching a callback at some point in the future. The function returns immediately.
 * \param[in] *cbinfo the callback handle
 * \param[in] milliseconds How far in the future to dispatch the callback
 * \param[in] updatemode What to do if the callback is already scheduled but not dispatched yet.
 * The options are:
 * UPDATEMODE_NONE: An existing schedule will not be touched, the call will have no effect at all if there's an existing schedule.
 * UPDATEMODE_SOONER: The callback will be rescheduled only if the new schedule triggers before the original one would have triggered.
 * UPDATEMODE_LATER: The callback will be rescheduled only if the new schedule triggers after the original one would have triggered.
 * UPDATEMODE_OVERRIDE: The callback will be rescheduled in any case, effectively overriding any previous schedule. (sooner+later=override)
 * \return 0: not scheduled, previous schedule takes precedence, 1: new schedule, 2: previous schedule overridden
 */
int32_t DelayedCallbackSchedule(
	DelayedCallbackInfo *cbinfo,
	int32_t milliseconds,
	DelayedCallbackUpdateMode updatemode
	);

/**
 * Dispatch an event by invoking the supplied callback. The function
 * returns immediately, the callback is invoked from the event task.
 * \param[in] *cbinfo the callback handle
 * \return Success (-1), failure (0)
 */
int32_t DelayedCallbackDispatch( DelayedCallbackInfo *cbinfo );

/**
 * Dispatch an event by invoking the supplied callback. The function
 * returns immediately, the callback is invoked from the event task.
 * \param[in] *cbinfo the callback handle
 * \param[in] pxHigherPriorityTaskWoken
 * xSemaphoreGiveFromISR() will set *pxHigherPriorityTaskWoken to pdTRUE if
 * giving the semaphore caused a task to unblock, and the unblocked task has a
 * priority higher than the currently running task.  If xSemaphoreGiveFromISR()
 * sets this value to pdTRUE then a context switch should be requested before
 * the interrupt is exited.
 * From FreeRTOS Docu: Context switching from an ISR uses port specific syntax.
 * Check the demo task for your port to find the syntax required.
 * \return Success (-1), failure (0)
 */
int32_t DelayedCallbackDispatchFromISR( DelayedCallbackInfo *cbinfo, long *pxHigherPriorityTaskWoken );

#endif // CALLBACKSCHEDULER_H
