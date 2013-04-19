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
// Use the CallbackPriority to define how frequent a callback needs to be
// called in relation to others in the same callback scheduling task.
// The scheduler will call callbacks waiting for execution with the same
// priority in a round robin way. However one slot in this queue is reserved
// for a chosen member of the next lower priority. This member will also be
// chosen in a round robin way.
// Example:
// Assume you have 6 callbacks in the same PriorityTask, all constantly wanting
// to be executed.
// A and B are priority CRITICAL,
// c and d are priority REGULAR,
// x and y are priority LOW.
// Then the execution schedule will look as follows:
// ...ABcABdABxABcABdAByABcABdABxABcABdABy...
// However if only the 3 callbacks, A, c and x want to execute, you will get:
// ...AcAxAcAxAcAxAcAxAcAxAcAxAcAxAcAxAcAx...
// And if onlz A and y need execution it will be:
// ...AyAyAyAyAyAyAyAyAyAyAyAyAyAyAyAyAyAy...
// despite their different priority they would get treated equally in this case.
//
// WARNING: Callbacks ALWAYS should return as quickly as possible.  Otherwise
// a low priority callback can block a critical one from being executed.
// Callbacks MUST NOT block execution!

typedef enum{
	CALLBACK_TASK_AUXILIARY     = (tskIDLE_PRIORITY + 1),
	CALLBACK_TASK_NAVIGATION    = (tskIDLE_PRIORITY + 2),
	CALLBACK_TASK_FLIGHTCONTROL = (tskIDLE_PRIORITY + 3),
	CALLBACK_TASK_DEVICEDRIVER  = (tskIDLE_PRIORITY + 4),
	} DelayedCallbackPriorityTask;
// Use the PriorityTask to define the global importance of callback execution
// compared to other processes in the system.
// Callbacks dispatched in a higher PriorityTasks will halt the execution of
// any lower priority processes, including callbacks and even callback
// scheduling tasks until they are done!
// Assume you have two callbacks:
// A in priorityTask DEVICEDRIVER,
// b and c in priorityTask AUXILIARY,
// Then the execution schedule can look as follows: (| marks a task switch)
// <b ... /b><c ... |<A ... /A>| ... /c><b ...
// be aware that if A gets constantly dispatched this would look like this:
// <b ... |<A><A><A><A><A><A><A><A><A><A><A><A><A><A><A><A><A><A><A>...
//
// WARNING: Any higher priority task can prevent lower priority code from being
// executed! (This does not only apply to callbacks but to all FreeRTOS tasks!)

typedef enum{
	CALLBACK_UPDATEMODE_NONE     = 0,
	CALLBACK_UPDATEMODE_SOONER   = 1,
	CALLBACK_UPDATEMODE_LATER    = 2,
	CALLBACK_UPDATEMODE_OVERRIDE = 3
	} DelayedCallbackUpdateMode;
// When scheduling a callback for execution at a time in the future, use the
// update mode to define what should happen if the callback is already
// scheduled.
// With NONE, the schedule will not be updated and the callback will be
// executed at the original time.
// With SOONER, the closer of the two schedules will take precedence
// With LATER, the schedule more distant in the future will be used.
// With OVERRIDE, the original schedule will be discarded.

typedef void (*DelayedCallback)(void);
// Use this type for the callback function.

struct DelayedCallbackInfoStruct;
typedef struct DelayedCallbackInfoStruct DelayedCallbackInfo;
// Use a pointer to DelayedCallbackInfo as a handle to identify registered callbacks.
// be aware that the same callback function can be registered as a callback
// several times, even with different callback priorities and even
// priorityTasks, using different handles and as such different dispatch calls.
// Be aware that using different priorityTasks for the same callback function
// might cause your callback to be executed recursively in different task contexts!

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
 * If a scheduler task with the specified task priority does not exist yet, it
 * will be created.
 * \param[in] cb The callback to be invoked
 * \param[in] priority Priority of the callback compared to other callbacks scheduled by the same delayed callback scheduler task.
 * \param[in] priorityTask Task priority of the scheduler task. One scheduler task will be spawned for each distinct value specified, further callbacks created  with the same priorityTask will all be handled by the same delayed callback scheduler task and scheduled according to their individual callback priorities
 * \param[in] stacksize The stack requirements of the callback when called by the scheduler.
 * \return CallbackInfo Pointer on success, NULL if failed.
 */
DelayedCallbackInfo *DelayedCallbackCreate(
	DelayedCallback cb,
	DelayedCallbackPriority priority,
	DelayedCallbackPriorityTask priorityTask,
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
