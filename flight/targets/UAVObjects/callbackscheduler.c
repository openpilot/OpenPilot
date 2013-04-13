/**
 ******************************************************************************
 *
 * @file       callbackscheduler.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Scheduler to run callback functions from a shared context with given priorities.
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

#include "openpilot.h"

// Private constants
#define MAX_QUEUE_SIZE 1

#define STACK_SIZE 128

#define TASK_PRIORITY (tskIDLE_PRIORITY + 1) // we run at regular update speed
#define MAX_UPDATE_PERIOD_MS 1000

// Private types
/**
 * callback information
 */
struct DelayedCallbackInfoStruct {
	DelayedCallback cb;
	bool volatile waiting;
	struct DelayedCallbackInfoStruct *next;
};

// Private variables
static DelayedCallbackInfo *callbackQueue[CALLBACK_PRIORITY_LOW+1];
static DelayedCallbackInfo *queueCursor[CALLBACK_PRIORITY_LOW+1];
static xQueueHandle queue;
static xTaskHandle callbackSchedulerTaskHandle;
static xSemaphoreHandle mutex;
static uint32_t stack_size;
static bool scheduler_started;

// Private functions
static void CallbackSchedulerTask();
static bool runNextCallback(DelayedCallbackPriority priority);


/**
 * Initialize the dispatcher
 * \return Success (0), failure (-1)
 */
int32_t CallbackSchedulerInitialize()
{
	// Initialize variables
	for (DelayedCallbackPriority p=0; p<=CALLBACK_PRIORITY_LOW; p++) callbackQueue[p] = NULL;

	// Create mutex
	mutex = xSemaphoreCreateRecursiveMutex();
	if (mutex == NULL)
		return -1;

	// Create event queue (dummy queue for self signals)
	queue = xQueueCreate(1,sizeof(DelayedCallbackInfo*));

	//minimal stack_size
	stack_size = STACK_SIZE; 

	scheduler_started = false;

	// Done
	return 0;
}

int32_t CallbackSchedulerStart()
{
	// Create task
	scheduler_started = true;
	xTaskCreate( CallbackSchedulerTask, (signed char*)"CallbackScheduler", stack_size/4, NULL, TASK_PRIORITY, &callbackSchedulerTaskHandle );
	TaskMonitorAdd(TASKINFO_RUNNING_CALLBACKSCHEDULER, callbackSchedulerTaskHandle);
	return 0;
}

/**
 * Dispatch an event by invoking the supplied callback. The function
 * returns imidiatelly, the callback is invoked from the event task.
 * \param[in] cbinfo the callback handle
 * \return Success (0), failure (-1)
 */
int32_t DelayedCallbackDispatch(DelayedCallbackInfo *cbinfo)
{
	// no semaphore needed
	cbinfo->waiting=true;
	// Push to queue
	return xQueueSend(queue, &cbinfo, 0); // will not block if queue is full
}

/**
 * Dispatch an event at periodic intervals.
 * \param[in] ev The event to be dispatched
 * \param[in] cb The callback to be invoked
 * \param[in] periodMs The period the event is generated
 * \return Success (0), failure (-1)
 */
DelayedCallbackInfo* DelayedCallbackCreate(DelayedCallback cb, DelayedCallbackPriority priority, uint32_t stacksize)
{
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
	if (!scheduler_started && stacksize>stack_size) stack_size=stacksize; // during system initialisation we can adapt to the maximum needed stack
	xSemaphoreGiveRecursive(mutex);

	if (stacksize>stack_size) return NULL; // error - not enough memory

	// create process info
	DelayedCallbackInfo *info = (DelayedCallbackInfo*)pvPortMalloc(sizeof(DelayedCallbackInfo));
	info->next     = NULL;
	info->waiting  = false;
	info->cb       = cb;

	// add to scheduling queue
	LL_APPEND(callbackQueue[priority],info);

	return info;
}

/**
 * Scheduler subtask
 * \param[in] priority The scheduling priority of the callback to search for
 */
static bool runNextCallback(DelayedCallbackPriority priority) {
	// no such queue
	if (priority>CALLBACK_PRIORITY_LOW) return false;

	// queue is empty, search a lower priority queue
	if (callbackQueue[priority]==NULL)
		return runNextCallback(priority+1);


	DelayedCallbackInfo *current=queueCursor[priority];
	DelayedCallbackInfo *next;
	do {
		if (current==NULL) {
			next=callbackQueue[priority]; // loop around the end of the list
			// also attempt to run a callback that has lower priority
			// every time the queue is completely traversed
			if (runNextCallback(priority+1)) {
				queueCursor[priority]=next;
				return true;
			}
		} else {
			next=current->next;
			if (current->waiting) {
				queueCursor[priority]=next;
				current->waiting=false; // flag is reset just before execution.
				current->cb(); // call the callback
				return true;
			}
		}
		current=next;
	} while (current!=queueCursor[priority]);
	// once the list has been traversed entirely once, abort (nothing to do)
	return false;
}


/**
 * Scheduler task, responsible of invoking callbacks.
 */
static void CallbackSchedulerTask()
{
	static DelayedCallbackInfo *dummy;
	while (1) {
		
		if (!runNextCallback(CALLBACK_PRIORITY_CRITICAL)) {
			// queue has no entries, sleep
			xQueueReceive(queue, &dummy, MAX_UPDATE_PERIOD_MS/portTICK_RATE_MS);
		}
	}
}



