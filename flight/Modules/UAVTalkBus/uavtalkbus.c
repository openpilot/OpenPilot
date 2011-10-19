/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup UAVTalkBusModule UAVTalkBus Module
 * @brief Main uavtalkbus module
 * Starts three tasks (RX, TX, and priority TX) that watch event queues
 * and handle all the uavtalkbus of the UAVobjects
 * @{ 
 *
 * @file       uavtalkbus.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      UAVTalkBus module, handles uavtalkbus and UAVObject updates
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
#include "uavtalkbus.h"
#include "gcstelemetrystats.h"
#include "flighttelemetrystats.h"
#include "systemstats.h"
#include "watchdogstatus.h"

// Private constants
#define MAX_QUEUE_SIZE   TELEM_QUEUE_SIZE
#define STACK_SIZE_BYTES PIOS_TELEM_STACK_SIZE
#define TASK_PRIORITY_RX (tskIDLE_PRIORITY + 2)
#define TASK_PRIORITY_TX (tskIDLE_PRIORITY + 2)
#define REQ_TIMEOUT_MS 250
#define MAX_RETRIES 2
#define STATS_UPDATE_PERIOD_MS 4000
#define CONNECTION_TIMEOUT_MS 8000
#define LINK_MIN_GRACE_TIME 10

// Private types

// Private variables
static xQueueHandle queue;

static xTaskHandle uavtalkbusTxTaskHandle;
static xTaskHandle uavtalkbusRxTaskHandle;
static uint32_t txErrors;
static uint32_t txRetries;
static UAVTalkConnection uavTalkCon;

// Private functions
static void uavtalkbusTxTask(void *parameters);
static void uavtalkbusRxTask(void *parameters);
static int32_t transmitData(uint8_t * data, int32_t length);
static void registerObject(UAVObjHandle obj);
static int32_t addObject(UAVObjHandle obj);
static void updateObject(UAVObjHandle obj,uint8_t onchange);
static int32_t setUpdatePeriod(UAVObjHandle obj, int32_t updatePeriodMs);
static void processObjEvent(UAVObjEvent * ev);

/**
 * Initialise the uavtalkbus module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t UAVTalkBusStart(void)
{
	// Process all registered objects and connect queue for updates
	UAVObjIterate(&registerObject);
    
	// Listen to objects of interest
    
	// Start uavtalkbus tasks
	xTaskCreate(uavtalkbusTxTask, (signed char *)"BusTx", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY_TX, &uavtalkbusTxTaskHandle);
	xTaskCreate(uavtalkbusRxTask, (signed char *)"BusRx", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY_RX, &uavtalkbusRxTaskHandle);
	// no monitoring...
	//TaskMonitorAdd(TASKINFO_RUNNING_UAVTALKBUSTX, uavtalkbusTxTaskHandle);
	//TaskMonitorAdd(TASKINFO_RUNNING_UAVTALKBUSRX, uavtalkbusRxTaskHandle);

	return 0;
}

/**
 * Initialise the uavtalkbus module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t UAVTalkBusInitialize(void)
{
	// Create object queues
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	// Initialise UAVTalk
	uavTalkCon = UAVTalkInitialize(&transmitData,256);
    
	// Create periodic event that will be used to update the uavtalkbus stats
	txErrors = 0;
	txRetries = 0;

	return 0;
}

MODULE_INITCALL(UAVTalkBusInitialize, UAVTalkBusStart)

/**
 * Register a new object, adds object to local list and connects the queue depending on the object's
 * uavtalkbus settings.
 * \param[in] obj Object to connect
 */
static void registerObject(UAVObjHandle obj)
{
	// Setup object for periodic updates
	addObject(obj);

	// Setup object for telemetry updates
	updateObject(obj,1);
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
	return EventPeriodicQueueCreate(&ev, queue, 0);
}

/**
 * Update object's queue connections and timer, depending on object's settings
 * \param[in] obj Object to updates
 */
static void updateObject(UAVObjHandle obj,uint8_t onchange)
{
	int32_t eventMask;

	if (onchange) {
		// Set update period
		setUpdatePeriod(obj, 0);
		// Connect queue
		eventMask = EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ;
	} else {
		// Set update period
		setUpdatePeriod(obj, LINK_MIN_GRACE_TIME);
		// Connect queue
		eventMask = EV_UPDATED_MANUAL | EV_UPDATE_REQ;
	}
	UAVObjConnectQueue(obj, queue, eventMask);
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
	return EventPeriodicQueueUpdate(&ev, queue, updatePeriodMs);
}

/**
 * Processes queue events
 */
static void processObjEvent(UAVObjEvent * ev)
{
	int32_t retries;
	int32_t success;

	// Some UAVObjects should not be synced since they would conflict.
	if (
		ev->obj &&
		ev->obj != GCSTelemetryStatsHandle() &&
		ev->obj != FlightTelemetryStatsHandle() &&
		ev->obj != SystemStatsHandle() &&
		ev->obj != SystemAlarmsHandle() &&
		ev->obj != TaskInfoHandle() &&
		ev->obj != WatchdogStatusHandle()
	   ) {

		// Act on event
		retries = 0;
		success = -1;
		if (ev->event == EV_UPDATED || ev->event == EV_UPDATED_MANUAL) {
			if (ev->event == EV_UPDATED) {
				updateObject(ev->obj,0);
			} else {
				updateObject(ev->obj,1);
			}
			// Send update to GCS (with retries)
			while (retries < MAX_RETRIES && success == -1) {
				success = UAVTalkSendObject(uavTalkCon, ev->obj, ev->instId, 0, REQ_TIMEOUT_MS);	// call blocks until ack is received or timeout
				++retries;
			}
			// Update stats
			txRetries += (retries - 1);
			if (success == -1) {
				++txErrors;
			}
		} else if (ev->event == EV_UPDATE_REQ) {
			// Request object update from GCS (with retries)
			while (retries < MAX_RETRIES && success == -1) {
				success = UAVTalkSendObjectRequest(uavTalkCon, ev->obj, ev->instId, REQ_TIMEOUT_MS);	// call blocks until update is received or timeout
				++retries;
			}
			// Update stats
			txRetries += (retries - 1);
			if (success == -1) {
				++txErrors;
			}
		}
	}
}

/**
 * UAVTalkBus transmit task, regular priority
 */
static void uavtalkbusTxTask(void *parameters)
{
	UAVObjEvent ev;

	// Loop forever
	while (1) {
		// Wait for queue message
		if (xQueueReceive(queue, &ev, portMAX_DELAY) == pdTRUE) {
			// Process event
			processObjEvent(&ev);
		}
	}
}

/**
 * UAVTalkBus transmit task. Processes queue events and periodic updates.
 */
static void uavtalkbusRxTask(void *parameters)
{
	uint32_t inputPort;

	// Task loop
	while (1) {
		// Determine input port (USB takes priority over uavtalkbus port)
#if defined(PIOS_INCLUDE_USB_HID)
		if (PIOS_USB_HID_CheckAvailable(0) && PIOS_COM_LINK_USB) {
			inputPort = PIOS_COM_LINK_USB;
		} else
#endif
		{
			inputPort = 0;
		}

		if (inputPort) {
			// Block until data are available
			uint8_t serial_data[1];
			uint16_t bytes_to_process;

			bytes_to_process = PIOS_COM_ReceiveBuffer(inputPort, serial_data, sizeof(serial_data), 500);
			if (bytes_to_process > 0) {
				for (uint8_t i = 0; i < bytes_to_process; i++) {
					UAVTalkProcessInputStream(uavTalkCon,serial_data[i]);
				}
			}
		} else {
			vTaskDelay(5);
		}
	}
}

/**
 * Transmit data buffer to the modem or USB port.
 * \param[in] data Data buffer to send
 * \param[in] length Length of buffer
 * \return 0 Success
 */
static int32_t transmitData(uint8_t * data, int32_t length)
{
	uint32_t outputPort;

	// Determine input port (USB takes priority over uavtalkbus port)
#if defined(PIOS_INCLUDE_USB_HID)
	if (PIOS_USB_HID_CheckAvailable(0) && PIOS_COM_LINK_USB) {
		outputPort = PIOS_COM_LINK_USB;
	} else
#endif
	{
		outputPort = 0;
	}

	if (outputPort) {
		return PIOS_COM_SendBufferNonBlocking(outputPort, data, length);
	} else {
		return -1;
	}
}

/**
  * @}
  * @}
  */
