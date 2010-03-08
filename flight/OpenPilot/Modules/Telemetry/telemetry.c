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

#include "openpilot.h"

// Private constants
#define MAX_QUEUE_SIZE 20
#define STACK_SIZE 100
#define TASK_PRIORITY 100
#define REQ_TIMEOUT_MS 500
#define MAX_RETRIES 2

// Private types

// Private variables
static COMPortTypeDef TelemetryPort;
static xQueueHandle queue;
static xTaskHandle telemetryTaskHandle;

// Private functions
static void telemetryTask(void);
static void periodicEventHandler(UAVObjEvent* ev);
static int32_t transmitData(uint8_t* data, int32_t length);
static void registerObject(UAVObjHandle obj);
static void updateObject(UAVObjHandle obj);
static int32_t addObject(UAVObjHandle obj);
static int32_t setUpdatePeriod(UAVObjHandle obj, int32_t updatePeriodMs);

/**
 * Initialise the telemetry module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t TelemetryInitialize(void)
{
	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	// TODO: Get telemetry settings object
	TelemetryPort = COM_USART1;

	// Initialise UAVTalk
	UAVTalkInitialize(&transmitData);

	// Process all registered objects and connect queue for updates
	UAVObjIterate(&registerObject);

	// Start telemetry task
	xTaskCreate(telemetryTask, (signed char*)"Telemetry", STACK_SIZE, NULL, TASK_PRIORITY, &telemetryTaskHandle);

	return 0;
}

/**
 * Register a new object, adds object to local list and connects the queue depending on the object's
 * telemetry settings.
 * \param[in] obj Object to connect
 */
void registerObject(UAVObjHandle obj)
{
	// Setup object for periodic updates
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
	if(metadata.telemetryUpdateMode == UPDATEMODE_PERIODIC) {
		// Set update period
		setUpdatePeriod(obj, metadata.telemetryUpdatePeriod);
		// Connect queue
		eventMask = EV_UPDATED_MANUAL | EV_UPDATE_REQ;
		if(UAVObjIsMetaobject(obj)) {
			eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnectQueue(obj, queue, eventMask);
	} else if(metadata.telemetryUpdateMode == UPDATEMODE_ONCHANGE) {
		// Set update period
		setUpdatePeriod(obj, 0);
		// Connect queue
		eventMask = EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ;
		if(UAVObjIsMetaobject(obj)) {
			eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnectQueue(obj, queue, eventMask);
	} else if(metadata.telemetryUpdateMode == UPDATEMODE_MANUAL) {
		// Set update period
		setUpdatePeriod(obj, 0);
		// Connect queue
		eventMask = EV_UPDATED_MANUAL | EV_UPDATE_REQ;
		if(UAVObjIsMetaobject(obj)) {
			eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnectQueue(obj, queue, eventMask);
	} else if(metadata.telemetryUpdateMode == UPDATEMODE_NEVER) {
		// Set update period
		setUpdatePeriod(obj, 0);
		// Disconnect queue
		UAVObjDisconnectQueue(obj, queue);
	}
}

/**
 * Telemetry task. Processes queue events and periodic updates. It does not return.
 */
static void telemetryTask(void)
{
	UAVObjEvent ev;
	UAVObjMetadata metadata;
	int32_t retries;
	int32_t success;

	// Loop forever
	while(1) {
		// Wait for queue message
		if(xQueueReceive(queue, &ev, portMAX_DELAY) == pdTRUE) {
			// Get object metadata
			UAVObjGetMetadata(ev.obj, &metadata);
			// Act on event
			if(ev.event == EV_UPDATED || ev.event == EV_UPDATED_MANUAL) {
				// Send update to GCS (with retries)
				retries = 0;
				while(retries < MAX_RETRIES && success == -1) {
					success = UAVTalkSendObject(ev.obj, ev.instId, metadata.ackRequired, REQ_TIMEOUT_MS); // call blocks until ack is received or timeout
					++retries;
				}
			} else if(ev.event == EV_UPDATE_REQ) {
				// Request object update from GCS (with retries)
				retries = 0;
				while(retries < MAX_RETRIES && success == -1) {
					success = UAVTalkSendObjectRequest(ev.obj, ev.instId, REQ_TIMEOUT_MS); // call blocks until update is received or timeout
					++retries;
				}
			}
			// If this is a metadata object then make necessary telemetry updates
			if(UAVObjIsMetaobject(ev.obj)) {
				updateObject(UAVObjGetLinkedObj(ev.obj)); // linked object will be the actual object the metadata are for
			}
		}

		/* This blocks the task until there is something on the buffer */
		if(PIOS_COM_ReceiveBufferUsed(TelemetryPort) > 0)
		{
			UAVTalkProcessInputStream(PIOS_COM_ReceiveBuffer(TelemetryPort));
		} else if(PIOS_COM_ReceiveBufferUsed(COM_USB_HID) > 0)
		{
			UAVTalkProcessInputStream(PIOS_COM_ReceiveBuffer(COM_USB_HID));
		}
	}
}

/**
 * Transmit data buffer to the modem or USB port.
 * \param[in] data Data buffer to send
 * \param[in] length Length of buffer
 * \return 0 Success
 */
static int32_t transmitData(uint8_t* data, int32_t length)
{
	COMPortTypeDef OutputPort;

	/* If USB HID transfer is possible */
	if(!PIOS_USB_HID_CheckAvailable()) {
		OutputPort = COM_USART1;
	} else {
		OutputPort = COM_USB_HID;
	}

	return PIOS_COM_SendBuffer(OutputPort, data, length);
}

/**
 * Event handler for periodic object updates (called by the event dispatcher)
 */
static void periodicEventHandler(UAVObjEvent* ev)
{
	// Push event to the telemetry queue
	xQueueSend(queue, ev, 0); // do not wait if queue is full
}

/**
 * Setup object for periodic updates.
 * \param[in] obj The object to update
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t addObject(UAVObjHandle obj)
{
	UAVObjEvent ev;

	// Add object for periodic updates
	ev.obj = obj;
	ev.instId = UAVOBJ_ALL_INSTANCES;
	ev.event = EV_UPDATED_MANUAL;
	return EventPeriodicCreate(&ev, &periodicEventHandler, 0);
}

/**
 * Set update period of object (it must be already setup for periodic updates)
 * \param[in] obj The object to update
 * \param[in] updatePeriodMs The update period in ms, if zero then periodic updates are disabled
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t setUpdatePeriod(UAVObjHandle obj, int32_t updatePeriodMs)
{
	UAVObjEvent ev;

	// Add object for periodic updates
	ev.obj = obj;
	ev.instId = UAVOBJ_ALL_INSTANCES;
	ev.event = EV_UPDATED_MANUAL;
	return EventPeriodicUpdate(&ev, &periodicEventHandler, updatePeriodMs);
}

