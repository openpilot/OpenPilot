/**
 ******************************************************************************
 *
 * @file       callbackscheduler.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
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
#define STACK_SIZE 128

// Private types
/**
 * task information
 */
struct DelayedCallbackTaskStruct {
        DelayedCallbackInfo *callbackQueue[CALLBACK_PRIORITY_LOW+1];
        DelayedCallbackInfo *queueCursor[CALLBACK_PRIORITY_LOW+1];
	xTaskHandle callbackSchedulerTaskHandle;
	signed char name[3];
	uint32_t stackSize;
	long taskPriority;
	xSemaphoreHandle signal;
	struct DelayedCallbackTaskStruct *next;
};

/**
 * callback information
 */
struct DelayedCallbackInfoStruct {
	DelayedCallback cb;
	bool volatile waiting;
	struct DelayedCallbackTaskStruct *task;
	struct DelayedCallbackInfoStruct *next;
};



// Private variables
static struct DelayedCallbackTaskStruct *schedulerTasks;
static xSemaphoreHandle mutex;
static bool schedulerStarted;

// Private functions
static void CallbackSchedulerTask(void *task);
static bool runNextCallback(struct DelayedCallbackTaskStruct *task, DelayedCallbackPriority priority);


/**
 * Initialize the scheduler
 * must be called before any othyer functions are called
 * \return Success (0), failure (-1)
 */
int32_t CallbackSchedulerInitialize()
{
	// Initialize variables
	schedulerTasks = NULL;
	schedulerStarted = false;

	// Create mutex
	mutex = xSemaphoreCreateRecursiveMutex();
	if (mutex == NULL) {
		return -1;
	}

	// Done
	return 0;
}

/**
 * Start the scheduler
 * Will 
 * \return Success (0), failure (-1)
 */
int32_t CallbackSchedulerStart()
{
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);

	// only call once
	PIOS_Assert(schedulerStarted==false);

	// start tasks
	struct DelayedCallbackTaskStruct *cursor;
	int t=0;
	LL_FOREACH(schedulerTasks,cursor) {
		xTaskCreate( CallbackSchedulerTask, cursor->name, cursor->stackSize/4, cursor, cursor->taskPriority, &cursor->callbackSchedulerTaskHandle );
		if (TASKINFO_RUNNING_CALLBACKSCHEDULER0+t <= TASKINFO_RUNNING_CALLBACKSCHEDULER3) {
			TaskMonitorAdd(TASKINFO_RUNNING_CALLBACKSCHEDULER0 + t, cursor->callbackSchedulerTaskHandle);
		}
		t++;
	}
	
	schedulerStarted = true;

	xSemaphoreGiveRecursive(mutex);
	
	return 0;
}

/**
 * Dispatch an event by invoking the supplied callback. The function
 * returns imidiatelly, the callback is invoked from the event task.
 * \param[in] cbinfo the callback handle
 * \return Success (-1), failure (0)
 */
int32_t DelayedCallbackDispatch(DelayedCallbackInfo *cbinfo)
{
	PIOS_Assert(cbinfo);

	// no semaphore needed for the callback
	cbinfo->waiting=true;
	// but the scheduler as a whole needs to be notified
	return xSemaphoreGive(cbinfo->task->signal);
}

/**
 * Dispatch an event by invoking the supplied callback. The function
 * returns imidiatelly, the callback is invoked from the event task.
 * \param[in] cbinfo the callback handle
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
int32_t DelayedCallbackDispatchFromISR(DelayedCallbackInfo *cbinfo, long *pxHigherPriorityTaskWoken)
{
	PIOS_Assert(cbinfo);

	// no semaphore needed for the callback
	cbinfo->waiting=true;
	// but the scheduler as a whole needs to be notified
	return xSemaphoreGiveFromISR(cbinfo->task->signal, pxHigherPriorityTaskWoken);
}

/**
 * Register a new callback to be called by a delayed callback scheduler task
 * \param[in] cb The callback to be invoked
 * \param[in] priority Priority of the callback compared to other callbacks called by the same delayed callback scheduler task.
 * \param[in] taskPriority Priority of the scheduler task. One scheduler task will be spawned for each distinct task priority in use, further callbacks with the same task priority will be handled by the same delayed callback scheduler task alternatingly according to their callback priority.
 * \param[in] stacksize The stack requirements of the callback when called by the scheduler.
 * \return Success (0), failure (-1)
 */
DelayedCallbackInfo* DelayedCallbackCreate(DelayedCallback cb, DelayedCallbackPriority priority, long taskPriority, uint32_t stacksize)
{
	
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
	
	// find appropriate scheduler task matching taskPriority
	struct DelayedCallbackTaskStruct *task=NULL;
	int t=0;
	LL_FOREACH(schedulerTasks, task) {
		if (task->taskPriority == taskPriority) {
			break; // found
		}
		t++;
	}
	// if scheduler task for given taskPriority does not exist, create it
	if (!task) {
		// allocate memory if possible
		task = (struct DelayedCallbackTaskStruct*)pvPortMalloc(sizeof(struct DelayedCallbackTaskStruct));
		if (!task) {
	                xSemaphoreGiveRecursive(mutex);
			return NULL;
		}

		// initialize structure
		for (DelayedCallbackPriority p=0; p<=CALLBACK_PRIORITY_LOW; p++) {
			task->callbackQueue[p] = NULL;
			task->queueCursor[p] = NULL;
		}
		task->name[0]      = 'C';
		task->name[1]      = 'a'+t;
		task->name[2]      = 0;
		task->stackSize   = ((STACK_SIZE>stacksize)?STACK_SIZE:stacksize);
		task->taskPriority = taskPriority;
		task->next         = NULL;

		// create the signaling semaphore
		vSemaphoreCreateBinary( task->signal );
		if (!task->signal) {
	                xSemaphoreGiveRecursive(mutex);
			return NULL;
		}

		// add to list of scheduler tasks
		LL_APPEND(schedulerTasks, task);

		// Previously registered tasks are spawned when CallbackSchedulerStart() is called.
		// Tasks registered afterwards need to spawn upon creation.
		if (schedulerStarted) {
			xTaskCreate( CallbackSchedulerTask, task->name, task->stackSize/4, task, task->taskPriority, &task->callbackSchedulerTaskHandle );
			if (TASKINFO_RUNNING_CALLBACKSCHEDULER0 + t <= TASKINFO_RUNNING_CALLBACKSCHEDULER3) {
				TaskMonitorAdd(TASKINFO_RUNNING_CALLBACKSCHEDULER0 + t, task->callbackSchedulerTaskHandle);
			}
		}
	}

	if (!schedulerStarted && stacksize > task->stackSize) {
		task->stackSize = stacksize; // previous to task initialisation we can still adapt to the maximum needed stack
	}


	if (stacksize > task->stackSize) {
		xSemaphoreGiveRecursive(mutex);
		return NULL; // error - not enough memory
	}

	// initialize callback scheduling info
	DelayedCallbackInfo *info = (DelayedCallbackInfo*)pvPortMalloc(sizeof(DelayedCallbackInfo));
	if (!info) {
		xSemaphoreGiveRecursive(mutex);
		return NULL; // error - not enough memory
	}
	info->next     = NULL;
	info->waiting  = false;
	info->task     = task;
	info->cb       = cb;

	// add to scheduling queue
	LL_APPEND(task->callbackQueue[priority],info);
	
	xSemaphoreGiveRecursive(mutex);

	return info;
}

/**
 * Scheduler subtask
 * \param[in] priority The scheduling priority of the callback to search for
 * \return Callback has been executed (boolean)
 */
static bool runNextCallback(struct DelayedCallbackTaskStruct *task, DelayedCallbackPriority priority) {
	// no such queue
	if (priority > CALLBACK_PRIORITY_LOW) {
		return false;
	}

	// queue is empty, search a lower priority queue
	if (task->callbackQueue[priority] == NULL) {
		return runNextCallback(task, priority + 1);
	}

	DelayedCallbackInfo *current = task->queueCursor[priority];
	DelayedCallbackInfo *next;
	do {
		if (current==NULL) {
			next=task->callbackQueue[priority]; // loop around the end of the list
			// also attempt to run a callback that has lower priority
			// every time the queue is completely traversed
			if (runNextCallback(task, priority + 1)) {
				task->queueCursor[priority] = next;
				return true;
			}
		} else {
			next = current->next;
			if (current->waiting) {
				task->queueCursor[priority] = next;
				current->waiting = false; // flag is reset just before execution.
				current->cb(); // call the callback
				return true;
			}
		}
		current = next;
	} while (current != task->queueCursor[priority]);
	// once the list has been traversed entirely once, abort (nothing to do)
	return false;
}


/**
 * Scheduler task, responsible of invoking callbacks.
 * \param[in] task The scheduling task being run
 */
static void CallbackSchedulerTask(void *task)
{
	
	while (1) {
		
		if (!runNextCallback((struct DelayedCallbackTaskStruct*) task,CALLBACK_PRIORITY_CRITICAL)) {
			// queue has no entries, sleep
			xSemaphoreTake( ((struct DelayedCallbackTaskStruct*)task)->signal, portMAX_DELAY);
		}
	}
}



