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

#include "telemetry.h"
#include "uavtalk.h"
#include "uavobject.h"
#include "uavobjectlist.h"
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

// Private variables
xQueueHandle queue;
xTaskHandle telemetryTaskHandle;

// Private functions
void telemetryTask();
void receiveTask();
int32_t transmitData(uint8_t* data, int32_t length);
void registerObject(UAVObject* obj);
void updateObject(UAVObject* obj);

/**
* Initialize the telemetry module
* \return -1 if initialization failed
* \return 0 on success
*/
int32_t TelemetryInitialize()
{
	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjectQMsg));

	// TODO: Get telemetry settings object

	// TODO: Initialize communication ports

	// Initialize UAVTalk
	UAVTalkInitialize(&transmitData);

	// Process all registered objects, register in UAVTalk and connect queue for updates
	UAVObjectListIterate(&registerObject);

	// Start tasks
	xTaskCreate( telemetryTask, (signed char*)"Telemetry", STACK_SIZE, NULL, TASK_PRIORITY, &telemetryTaskHandle );
	// TODO: Start receive task

	return 0;
}

/**
* Register a new object, connects object to UAVTalk and connects the queue depending on the object's
* telemetry settings.
* \param[in] obj Object to connect
*/
void registerObject(UAVObject* obj)
{
	// Register object in UAVTalk
	UAVTalkConnectObject(obj->objectID, obj->pack, obj->unpack, 0);

	// Setup object for telemetry updates
	updateObject(obj);
}

/**
 * Update object's queue connections and timer, depending on object's settings
 * \param[in] obj Object to updates
 */
void updateObject(UAVObject* obj)
{
	UAVObjectMetadata metadata;
	int32_t eventMask;

	// Get metadata
	obj->getMetadata(&metadata);

	// Setup object depending on update mode
	if (metadata.telemetryUpdateMode == UPDATEMODE_PERIODIC)
	{
		// Set update period
		UAVTalkSetUpdatePeriod(obj->objectID, metadata.telemetryUpdatePeriod);
		// Connect queue
		eventMask = QMSG_UPDATED_MANUAL|QMSG_UPDATE_REQ;
		if (obj->isMetadata)
		{
			eventMask |= QMSG_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		obj->connect(queue, eventMask);
	}
	else if (metadata.telemetryUpdateMode == UPDATEMODE_ONCHANGE)
	{
		// Set update period
		UAVTalkSetUpdatePeriod(obj->objectID, 0);
		// Connect queue
		eventMask = QMSG_UPDATED|QMSG_UPDATED_MANUAL|QMSG_UPDATE_REQ;
		if (obj->isMetadata)
		{
			eventMask |= QMSG_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		obj->connect(queue, eventMask);
	}
	else if (metadata.telemetryUpdateMode == UPDATEMODE_MANUAL)
	{
		// Set update period
		UAVTalkSetUpdatePeriod(obj->objectID, 0);
		// Connect queue
		eventMask = QMSG_UPDATED_MANUAL|QMSG_UPDATE_REQ;
		if (obj->isMetadata)
		{
			eventMask |= QMSG_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		obj->connect(queue, eventMask);
	}
	else if (metadata.telemetryUpdateMode == UPDATEMODE_NEVER)
	{
		// Set update period
		UAVTalkSetUpdatePeriod(obj->objectID, 0);
		// Disconnect queue
		obj->disconnect(queue);
	}
}

/**
 * Telemetry task. Processes queue events and periodic updates. It does not return.
 */
void telemetryTask()
{
	static int32_t timeToNextUpdateMs;
	static int32_t delayMs;
	static UAVObjectQMsg msg;
	static UAVObjectMetadata metadata;
	static int32_t retries;
	static int32_t success;

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
			msg.obj->getMetadata(&metadata);
			// Act on event
			if (msg.event == QMSG_UPDATED || msg.event == QMSG_UPDATED_MANUAL)
			{
				// Send update to GCS (with retries)
				retries = 0;
				while (retries < MAX_RETRIES && success == -1)
				{
					success = UAVTalkSendObject(msg.obj->objectID, metadata.ackRequired, REQ_TIMEOUT_MS); // call blocks until ack is received or timeout
					++retries;
				}
			}
			else if (msg.event == QMSG_UPDATE_REQ)
			{
				// Request object update from GCS (with retries)
				retries = 0;
				while (retries < MAX_RETRIES && success == -1)
				{
					success = UAVTalkSendObjectRequest(msg.obj->objectID, REQ_TIMEOUT_MS); // call blocks until update is received or timeout
					++retries;
				}
			}
			// If this is a metadata object then make necessary telemetry updates
			if (msg.obj->isMetadata)
			{
				updateObject(msg.obj->linkedObj); // linked object will be the actual object the metadata are for
			}
		}

		// Process periodic updates
		if ((xTaskGetTickCount()*portTICK_RATE_MS) >= timeToNextUpdateMs )
		{
			delayMs = UAVTalkProcessPeriodicUpdates();
			timeToNextUpdateMs = xTaskGetTickCount()*portTICK_RATE_MS + delayMs;
		}
	}
}

/**
 * Receive task. Processes received bytes (from the modem or USB) and passes them to
 * UAVTalk for decodings. Does not return.
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



