/**
 ******************************************************************************
 *
 * @file       eventdispatcher.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Event dispatcher, distributes object events as callbacks. Alternative
 * 	       to using tasks and queues. All callbacks are invoked from the event task.
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
#if defined(PIOS_EVENTDISAPTCHER_QUEUE)
#define MAX_QUEUE_SIZE PIOS_EVENTDISAPTCHER_QUEUE
#else
#define MAX_QUEUE_SIZE 20
#endif

#if defined(PIOS_EVENTDISPATCHER_STACK_SIZE)
#define STACK_SIZE PIOS_EVENTDISPATCHER_STACK_SIZE
#else
#define STACK_SIZE configMINIMAL_STACK_SIZE
#endif /* PIOS_EVENTDISPATCHER_STACK_SIZE */

#define TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define MAX_UPDATE_PERIOD_MS 1000

// Private types


/**
 * Event callback information
 */
typedef struct {
	UAVObjEvent ev; /** The actual event */
	UAVObjEventCallback cb; /** The callback function, or zero if none */
	xQueueHandle queue; /** The queue or zero if none */
} EventCallbackInfo;

/**
 * List of object properties that are needed for the periodic updates.
 */
struct PeriodicObjectListStruct {
	EventCallbackInfo evInfo; /** Event callback information */
    uint16_t updatePeriodMs; /** Update period in ms or 0 if no periodic updates are needed */
    int32_t timeToNextUpdateMs; /** Time delay to the next update */
    struct PeriodicObjectListStruct* next; /** Needed by linked list library (utlist.h) */
};
typedef struct PeriodicObjectListStruct PeriodicObjectList;

// Private variables
static PeriodicObjectList* objList;
static xQueueHandle queue;
static xTaskHandle eventTaskHandle;
static xSemaphoreHandle mutex;
static EventStats stats;

// Private functions
static int32_t processPeriodicUpdates();
static void eventTask();
static int32_t eventPeriodicCreate(UAVObjEvent* ev, UAVObjEventCallback cb, xQueueHandle queue, uint16_t periodMs);
static int32_t eventPeriodicUpdate(UAVObjEvent* ev, UAVObjEventCallback cb, xQueueHandle queue, uint16_t periodMs);
static uint16_t randomizePeriod(uint16_t periodMs);


/**
 * Initialize the dispatcher
 * \return Success (0), failure (-1)
 */
int32_t EventDispatcherInitialize()
{
	// Initialize variables
	objList = NULL;
	memset(&stats, 0, sizeof(EventStats));

	// Create mutex
	mutex = xSemaphoreCreateRecursiveMutex();
	if (mutex == NULL)
		return -1;

	// Create event queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(EventCallbackInfo));

	// Create task
	xTaskCreate( eventTask, (signed char*)"Event", STACK_SIZE, NULL, TASK_PRIORITY, &eventTaskHandle );

	// Done
	return 0;
}

/**
 * Get the statistics counters
 * @param[out] statsOut The statistics counters will be copied there
 */
void EventGetStats(EventStats* statsOut)
{
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
	memcpy(statsOut, &stats, sizeof(EventStats));
	xSemaphoreGiveRecursive(mutex);
}

/**
 * Clear the statistics counters
 */
void EventClearStats()
{
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
	memset(&stats, 0, sizeof(EventStats));
	xSemaphoreGiveRecursive(mutex);
}

/**
 * Dispatch an event by invoking the supplied callback. The function
 * returns imidiatelly, the callback is invoked from the event task.
 * \param[in] ev The event to be dispatched
 * \param[in] cb The callback function
 * \return Success (0), failure (-1)
 */
int32_t EventCallbackDispatch(UAVObjEvent* ev, UAVObjEventCallback cb)
{
	EventCallbackInfo evInfo;
	// Initialize event callback information
	memcpy(&evInfo.ev, ev, sizeof(UAVObjEvent));
	evInfo.cb = cb;
	evInfo.queue = 0;
	// Push to queue
	return xQueueSend(queue, &evInfo, 0); // will not block if queue is full
}

/**
 * Dispatch an event at periodic intervals.
 * \param[in] ev The event to be dispatched
 * \param[in] cb The callback to be invoked
 * \param[in] periodMs The period the event is generated
 * \return Success (0), failure (-1)
 */
int32_t EventPeriodicCallbackCreate(UAVObjEvent* ev, UAVObjEventCallback cb, uint16_t periodMs)
{
	return eventPeriodicCreate(ev, cb, 0, periodMs);
}

/**
 * Update the period of a periodic event.
 * \param[in] ev The event to be dispatched
 * \param[in] cb The callback to be invoked
 * \param[in] periodMs The period the event is generated
 * \return Success (0), failure (-1)
 */
int32_t EventPeriodicCallbackUpdate(UAVObjEvent* ev, UAVObjEventCallback cb, uint16_t periodMs)
{
	return eventPeriodicUpdate(ev, cb, 0, periodMs);
}

/**
 * Dispatch an event at periodic intervals.
 * \param[in] ev The event to be dispatched
 * \param[in] queue The queue that the event will be pushed in
 * \param[in] periodMs The period the event is generated
 * \return Success (0), failure (-1)
 */
int32_t EventPeriodicQueueCreate(UAVObjEvent* ev, xQueueHandle queue, uint16_t periodMs)
{
	return eventPeriodicCreate(ev, 0, queue, periodMs);
}

/**
 * Update the period of a periodic event.
 * \param[in] ev The event to be dispatched
 * \param[in] queue The queue
 * \param[in] periodMs The period the event is generated
 * \return Success (0), failure (-1)
 */
int32_t EventPeriodicQueueUpdate(UAVObjEvent* ev, xQueueHandle queue, uint16_t periodMs)
{
	return eventPeriodicUpdate(ev, 0, queue, periodMs);
}

/**
 * Dispatch an event through a callback at periodic intervals.
 * \param[in] ev The event to be dispatched
 * \param[in] cb The callback to be invoked or zero if none
 * \param[in] queue The queue or zero if none
 * \param[in] periodMs The period the event is generated
 * \return Success (0), failure (-1)
 */
static int32_t eventPeriodicCreate(UAVObjEvent* ev, UAVObjEventCallback cb, xQueueHandle queue, uint16_t periodMs)
{
	PeriodicObjectList* objEntry;
	// Get lock
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
	// Check that the object is not already connected
	LL_FOREACH(objList, objEntry)
	{
		if (objEntry->evInfo.cb == cb &&
			objEntry->evInfo.queue == queue &&
			objEntry->evInfo.ev.obj == ev->obj &&
			objEntry->evInfo.ev.instId == ev->instId &&
			objEntry->evInfo.ev.event == ev->event)
		{
			// Already registered, do nothing
			xSemaphoreGiveRecursive(mutex);
			return -1;
		}
	}
    // Create handle
	objEntry = (PeriodicObjectList*)pvPortMalloc(sizeof(PeriodicObjectList));
	if (objEntry == NULL) return -1;
	objEntry->evInfo.ev.obj = ev->obj;
	objEntry->evInfo.ev.instId = ev->instId;
	objEntry->evInfo.ev.event = ev->event;
	objEntry->evInfo.cb = cb;
	objEntry->evInfo.queue = queue;
    objEntry->updatePeriodMs = periodMs;
    objEntry->timeToNextUpdateMs = randomizePeriod(periodMs); // avoid bunching of updates
    // Add to list
    LL_APPEND(objList, objEntry);
	// Release lock
	xSemaphoreGiveRecursive(mutex);
    return 0;
}

/**
 * Update the period of a periodic event.
 * \param[in] ev The event to be dispatched
 * \param[in] cb The callback to be invoked or zero if none
 * \param[in] queue The queue or zero if none
 * \param[in] periodMs The period the event is generated
 * \return Success (0), failure (-1)
 */
static int32_t eventPeriodicUpdate(UAVObjEvent* ev, UAVObjEventCallback cb, xQueueHandle queue, uint16_t periodMs)
{
	PeriodicObjectList* objEntry;
	// Get lock
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
	// Find object
	LL_FOREACH(objList, objEntry)
	{
		if (objEntry->evInfo.cb == cb &&
			objEntry->evInfo.queue == queue &&
			objEntry->evInfo.ev.obj == ev->obj &&
			objEntry->evInfo.ev.instId == ev->instId &&
			objEntry->evInfo.ev.event == ev->event)
		{
			// Object found, update period
			objEntry->updatePeriodMs = periodMs;
			objEntry->timeToNextUpdateMs = randomizePeriod(periodMs); // avoid bunching of updates
			// Release lock
			xSemaphoreGiveRecursive(mutex);
			return 0;
		}
	}
    // If this point is reached the object was not found
	xSemaphoreGiveRecursive(mutex);
    return -1;
}

/**
 * Event task, responsible of invoking callbacks.
 */
static void eventTask()
{
	int32_t timeToNextUpdateMs;
	int32_t delayMs;
	EventCallbackInfo evInfo;

	/* Must do this in task context to ensure that TaskMonitor has already finished its init */
	TaskMonitorAdd(TASKINFO_RUNNING_EVENTDISPATCHER, eventTaskHandle);

	// Initialize time
	timeToNextUpdateMs = xTaskGetTickCount()*portTICK_RATE_MS;

	// Loop forever
	while (1)
	{
		// Calculate delay time
		delayMs = timeToNextUpdateMs-(xTaskGetTickCount()*portTICK_RATE_MS);
		if (delayMs < 0)
		{
			delayMs = 0;
		}

		// Wait for queue message
		if ( xQueueReceive(queue, &evInfo, delayMs/portTICK_RATE_MS) == pdTRUE )
		{
			// Invoke callback, if one
			if ( evInfo.cb != 0)
			{
				evInfo.cb(&evInfo.ev); // the function is expected to copy the event information
			}
		}

		// Process periodic updates
		if ((xTaskGetTickCount()*portTICK_RATE_MS) >= timeToNextUpdateMs )
		{
			timeToNextUpdateMs = processPeriodicUpdates();
		}
	}
}

/**
 * Handle periodic updates for all objects.
 * \return The system time until the next update (in ms) or -1 if failed
 */
static int32_t processPeriodicUpdates()
{
	PeriodicObjectList* objEntry;
	int32_t timeNow;
    int32_t timeToNextUpdate;
    int32_t offset;

	// Get lock
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);

    // Iterate through each object and update its timer, if zero then transmit object.
    // Also calculate smallest delay to next update.
    timeToNextUpdate = xTaskGetTickCount()*portTICK_RATE_MS + MAX_UPDATE_PERIOD_MS;
    LL_FOREACH(objList, objEntry)
    {
        // If object is configured for periodic updates
        if (objEntry->updatePeriodMs > 0)
        {
            // Check if time for the next update
        	timeNow = xTaskGetTickCount()*portTICK_RATE_MS;
            if (objEntry->timeToNextUpdateMs <= timeNow)
            {
                // Reset timer
            	offset = ( timeNow - objEntry->timeToNextUpdateMs ) % objEntry->updatePeriodMs;
            	objEntry->timeToNextUpdateMs = timeNow + objEntry->updatePeriodMs - offset;
    			// Invoke callback, if one
    			if ( objEntry->evInfo.cb != 0)
    			{
    				objEntry->evInfo.cb(&objEntry->evInfo.ev); // the function is expected to copy the event information
    			}
    			// Push event to queue, if one
    			if ( objEntry->evInfo.queue != 0)
    			{
    				if ( xQueueSend(objEntry->evInfo.queue, &objEntry->evInfo.ev, 0) != pdTRUE ) // do not block if queue is full
    				{
						if (objEntry->evInfo.ev.obj != NULL)
							stats.lastErrorID = UAVObjGetID(objEntry->evInfo.ev.obj);
    					++stats.eventErrors;
    				}
    			}
            }
            // Update minimum delay
            if (objEntry->timeToNextUpdateMs < timeToNextUpdate)
            {
            	timeToNextUpdate = objEntry->timeToNextUpdateMs;
            }
        }
    }

    // Done
    xSemaphoreGiveRecursive(mutex);
    return timeToNextUpdate;
}

/**
 * Return a psedorandom integer from 0 to periodMs
 * Based on the Park-Miller-Carta Pseudo-Random Number Generator
 * http://www.firstpr.com.au/dsp/rand31/
 */
static uint16_t randomizePeriod(uint16_t periodMs)
{
	static uint32_t seed = 1;
	uint32_t hi, lo;
	lo = 16807 * (seed & 0xFFFF);
	hi = 16807 * (seed >> 16);
	lo += (hi & 0x7FFF) << 16;
	lo += hi >> 15;
	if (lo > 0x7FFFFFFF) lo -= 0x7FFFFFFF;
	seed = lo;
	return (uint16_t)( ((float)periodMs * (float)lo) / (float)0x7FFFFFFF );
}

