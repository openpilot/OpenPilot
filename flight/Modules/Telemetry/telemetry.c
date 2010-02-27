/**
 ******************************************************************************
 *
 * @file       telemetry.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Telemetry module, handles telemetry and UAVObject updates
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

#include <stdlib.h> // for malloc
#include "telemetry.h"
#include "uavtalk.h"
#include "uavobjectmanager.h"
#include "utlist.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Private constants
#define MAX_QUEUE_SIZE 20
#define STACK_SIZE 100
#define TASK_PRIORITY 100
#define REQ_TIMEOUT_MS 500
#define MAX_RETRIES 2
#define MAX_UPDATE_PERIOD_MS 1000
#define MIN_UPDATE_PERIOD_MS 1

// Private types

/**
 * List of object properties that are needed for the periodic updates.
 */
struct ObjectListStruct {
	UAVObjHandle obj; /** Object handle */
    int32_t updatePeriodMs; /** Update period in ms or 0 if no periodic updates are needed */
    int32_t timeToNextUpdateMs; /** Time delay to the next update */
    struct ObjectListStruct* next; /** Needed by linked list library (utlist.h) */
};
typedef struct ObjectListStruct ObjectList;

// Private variables
xQueueHandle queue;
xTaskHandle telemetryTaskHandle;
ObjectList* objList;

// Private functions
void telemetryTask();
void receiveTask();
int32_t transmitData(uint8_t* data, int32_t length);
void registerObject(UAVObjHandle obj);
void updateObject(UAVObjHandle obj);
int32_t addObject(UAVObjHandle obj);
int32_t setUpdatePeriod(UAVObjHandle obj, int32_t updatePeriodMs);
int32_t processPeriodicUpdates();

/**
* Initialize the telemetry module
* \return -1 if initialization failed
* \return 0 on success
*/
int32_t TelemetryInitialize()
{
	// Initialize object list
	objList = NULL;

	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjQMsg));

	// TODO: Get telemetry settings object

	// TODO: Initialize communication ports

	// Initialize UAVTalk
	UAVTalkInitialize(&transmitData);

	// Process all registered objects and connect queue for updates
	UAVObjIterate(&registerObject);

	// Start tasks
	xTaskCreate( telemetryTask, (signed char*)"Telemetry", STACK_SIZE, NULL, TASK_PRIORITY, &telemetryTaskHandle );
	// TODO: Start receive task

	return 0;
}

/**
* Register a new object, adds object to local list and connects the queue depending on the object's
* telemetry settings.
* \param[in] obj Object to connect
*/
void registerObject(UAVObjHandle obj)
{
	// Add object to list
	addObject(obj);

	// Setup object for telemetry updates
	updateObject(obj);
}

/**
 * Update object's queue connections and timer, depending on object's settings
 * \param[in] obj Object to updates
 */
void updateObject(UAVObjHandle obj)
{
	UAVObjMetadata metadata;
	int32_t eventMask;

	// Get metadata
	UAVObjGetMetadata(obj, &metadata);

	// Setup object depending on update mode
	if (metadata.telemetryUpdateMode == UPDATEMODE_PERIODIC)
	{
		// Set update period
		setUpdatePeriod(obj, metadata.telemetryUpdatePeriod);
		// Connect queue
		eventMask = QMSG_UPDATED_MANUAL|QMSG_UPDATE_REQ;
		if (UAVObjIsMetaobject(obj))
		{
			eventMask |= QMSG_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnect(obj, queue, eventMask);
	}
	else if (metadata.telemetryUpdateMode == UPDATEMODE_ONCHANGE)
	{
		// Set update period
		setUpdatePeriod(obj, 0);
		// Connect queue
		eventMask = QMSG_UPDATED|QMSG_UPDATED_MANUAL|QMSG_UPDATE_REQ;
		if (UAVObjIsMetaobject(obj))
		{
			eventMask |= QMSG_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnect(obj, queue, eventMask);
	}
	else if (metadata.telemetryUpdateMode == UPDATEMODE_MANUAL)
	{
		// Set update period
		setUpdatePeriod(obj, 0);
		// Connect queue
		eventMask = QMSG_UPDATED_MANUAL|QMSG_UPDATE_REQ;
		if (UAVObjIsMetaobject(obj))
		{
			eventMask |= QMSG_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnect(obj, queue, eventMask);
	}
	else if (metadata.telemetryUpdateMode == UPDATEMODE_NEVER)
	{
		// Set update period
		setUpdatePeriod(obj, 0);
		// Disconnect queue
		UAVObjDisconnect(obj, queue);
	}
}

/**
 * Telemetry task. Processes queue events and periodic updates. It does not return.
 */
void telemetryTask()
{
	int32_t timeToNextUpdateMs;
	int32_t delayMs;
	UAVObjQMsg msg;
	UAVObjMetadata metadata;
	int32_t retries;
	int32_t success;

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
		if ( xQueueReceive(queue, &msg, delayMs/portTICK_RATE_MS) == pdTRUE )
		{
			// Get object metadata
			UAVObjGetMetadata(msg.obj, &metadata);
			// Act on event
			if (msg.event == QMSG_UPDATED || msg.event == QMSG_UPDATED_MANUAL)
			{
				// Send update to GCS (with retries)
				retries = 0;
				while (retries < MAX_RETRIES && success == -1)
				{
					success = UAVTalkSendObject(msg.obj, msg.instId, metadata.ackRequired, REQ_TIMEOUT_MS); // call blocks until ack is received or timeout
					++retries;
				}
			}
			else if (msg.event == QMSG_UPDATE_REQ)
			{
				// Request object update from GCS (with retries)
				retries = 0;
				while (retries < MAX_RETRIES && success == -1)
				{
					success = UAVTalkSendObjectRequest(msg.obj, msg.instId, REQ_TIMEOUT_MS); // call blocks until update is received or timeout
					++retries;
				}
			}
			// If this is a metadata object then make necessary telemetry updates
			if (UAVObjIsMetaobject(msg.obj))
			{
				updateObject(UAVObjGetLinkedObj(msg.obj)); // linked object will be the actual object the metadata are for
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
 * Receive task. Processes received bytes (from the modem or USB) and passes them to
 * UAVTalk for decoding. Does not return.
 */
void receiveTask()
{
	// Main thread
	while (1)
	{
		// TODO: Wait for bytes and pass to UAVTalk for processing
	}
}

/**
 * Transmit data buffer to the modem or USB port.
 * \param[in] data Data buffer to send
 * \param[in] length Length of buffer
 */
int32_t transmitData(uint8_t* data, int32_t length)
{
	// TODO: Send data to communication port
	return 0;
}

/**
 * Setup object for periodic updates.
 * \param[in] obj The object to update
 * \param[in] updatePeriodMs The update period in ms, if zero then periodic updates are disabled
 * \return 0 Success
 * \return -1 Failure
 */
int32_t setUpdatePeriod(UAVObjHandle obj, int32_t updatePeriodMs)
{
	ObjectList* objEntry;
	// Check that the object is not already connected
	LL_FOREACH(objList, objEntry)
	{
		if (objEntry->obj == obj)
		{
			objEntry->updatePeriodMs = updatePeriodMs;
			objEntry->timeToNextUpdateMs = 0;
			return 0;
		}
	}
	// If this point is reached then the object was not found
	return -1;
}

/**
 * Handle periodic updates for all objects.
 * \return The system time until the next update (in ms) or -1 if failed
 */
int32_t processPeriodicUpdates()
{
	static int32_t timeOfLastUpdate = 0;
	ObjectList* objEntry;
	int32_t delaySinceLastUpdateMs;
    int32_t minDelay = MAX_UPDATE_PERIOD_MS;

    // Iterate through each object and update its timer, if zero then transmit object.
    // Also calculate smallest delay to next update.
    delaySinceLastUpdateMs = xTaskGetTickCount()*portTICK_RATE_MS - timeOfLastUpdate;
    LL_FOREACH(objList, objEntry)
    {
        // If object is configured for periodic updates
        if (objEntry->updatePeriodMs > 0)
        {
        	objEntry->timeToNextUpdateMs -= delaySinceLastUpdateMs;
            // Check if time for the next update
            if (objEntry->timeToNextUpdateMs <= 0)
            {
                // Reset timer
            	objEntry->timeToNextUpdateMs = objEntry->updatePeriodMs;
                // Send object (trigger update)
            	UAVObjUpdated(objEntry->obj);
            }
            // Update minimum delay
            if (objEntry->timeToNextUpdateMs < minDelay)
            {
                minDelay = objEntry->timeToNextUpdateMs;
            }
        }
    }

    // Check if delay for the next update is too short
    if (minDelay < MIN_UPDATE_PERIOD_MS)
    {
        minDelay = MIN_UPDATE_PERIOD_MS;
    }

    // Done
    timeOfLastUpdate = xTaskGetTickCount()*portTICK_RATE_MS;
    return timeOfLastUpdate + minDelay;
}

/**
 * Add a new object to the object list
 */
int32_t addObject(UAVObjHandle obj)
{
	ObjectList* objEntry;
	// Check that the object is not already connected
	LL_FOREACH(objList, objEntry)
	{
		if (objEntry->obj == obj)
		{
			// Already registered, ignore
			return -1;
		}
	}
    // Create handle
	objEntry = (ObjectList*)malloc(sizeof(ObjectList));
	if (objEntry == NULL) return -1;
	objEntry->obj = obj;
    objEntry->updatePeriodMs = 0;
    objEntry->timeToNextUpdateMs = 0;
    // Add to list
    LL_APPEND(objList, objEntry);
    return 0;
}







