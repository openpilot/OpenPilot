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
#include "flighttelemetrystats.h"

// Private constants
#define MAX_QUEUE_SIZE 20
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define REQ_TIMEOUT_MS 250
#define MAX_RETRIES 3
#define STATS_UPDATE_PERIOD_MS 5000

// Private types

// Private variables
static COMPortTypeDef telemetryPort;
static xQueueHandle queue;
static xTaskHandle telemetryTxTaskHandle;
static xTaskHandle telemetryRxTaskHandle;
static uint32_t txErrors;
static uint32_t txRetries;

// Private functions
static void telemetryTxTask(void* parameters);
static void telemetryRxTask(void* parameters);
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
	UAVObjEvent ev;

	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	// TODO: Get telemetry settings object
	telemetryPort = COM_USART1;

	// Initialise UAVTalk
	UAVTalkInitialize(&transmitData);

	// Process all registered objects and connect queue for updates
	UAVObjIterate(&registerObject);

	// Create periodic event that will be used to update the telemetry stats
	txErrors = 0;
	txRetries = 0;
	memset(&ev, 0, sizeof(UAVObjEvent));
	EventPeriodicQueueCreate(&ev, queue, STATS_UPDATE_PERIOD_MS);

	// Start telemetry tasks
	xTaskCreate(telemetryTxTask, (signed char*)"TelemetryTx", STACK_SIZE, NULL, TASK_PRIORITY, &telemetryTxTaskHandle);
	xTaskCreate(telemetryRxTask, (signed char*)"TelemetryRx", STACK_SIZE, NULL, TASK_PRIORITY, &telemetryRxTaskHandle);

	return 0;
}

/**
 * Register a new object, adds object to local list and connects the queue depending on the object's
 * telemetry settings.
 * \param[in] obj Object to connect
 */
static void registerObject(UAVObjHandle obj)
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
static void updateObject(UAVObjHandle obj)
{
	UAVObjMetadata metadata;
	int32_t eventMask;

	// Get metadata
	UAVObjGetMetadata(obj, &metadata);

	// Setup object depending on update mode
	if(metadata.telemetryUpdateMode == UPDATEMODE_PERIODIC)
	{
		// Set update period
		setUpdatePeriod(obj, metadata.telemetryUpdatePeriod);
		// Connect queue
		eventMask = EV_UPDATED_MANUAL | EV_UPDATE_REQ;
		if(UAVObjIsMetaobject(obj))
		{
			eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnectQueue(obj, queue, eventMask);
	}
	else if(metadata.telemetryUpdateMode == UPDATEMODE_ONCHANGE)
	{
		// Set update period
		setUpdatePeriod(obj, 0);
		// Connect queue
		eventMask = EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ;
		if(UAVObjIsMetaobject(obj))
		{
			eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnectQueue(obj, queue, eventMask);
	}
	else if(metadata.telemetryUpdateMode == UPDATEMODE_MANUAL)
	{
		// Set update period
		setUpdatePeriod(obj, 0);
		// Connect queue
		eventMask = EV_UPDATED_MANUAL | EV_UPDATE_REQ;
		if(UAVObjIsMetaobject(obj))
		{
			eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnectQueue(obj, queue, eventMask);
	}
	else if(metadata.telemetryUpdateMode == UPDATEMODE_NEVER)
	{
		// Set update period
		setUpdatePeriod(obj, 0);
		// Disconnect queue
		UAVObjDisconnectQueue(obj, queue);
	}
}

/**
 * Telemetry transmit task. Processes queue events and periodic updates.
 */
static void telemetryTxTask(void* parameters)
{
	UAVObjEvent ev;
	UAVObjMetadata metadata;
	int32_t retries;
	int32_t success;
	UAVTalkStats utalkStats;
	FlightTelemetryStatsData stats;

	// Loop forever
	while(1)
	{
		// Wait for queue message
		if(xQueueReceive(queue, &ev, portMAX_DELAY) == pdTRUE)
		{
			// Check if this is a periodic timer event (used to update stats)
			if (ev.obj == 0)
			{
			    // Get stats
				UAVTalkGetStats(&utalkStats);
				UAVTalkResetStats();

			    // Update stats object
			    FlightTelemetryStatsGet(&stats);
			    if (utalkStats.rxBytes > 0)
			    {
			        stats.Connected = FLIGHTTELEMETRYSTATS_CONNECTED_TRUE;
			    }
			    else
			    {
			        stats.Connected = FLIGHTTELEMETRYSTATS_CONNECTED_FALSE;
			    }
			    stats.RxDataRate = (float)utalkStats.rxBytes / ((float)STATS_UPDATE_PERIOD_MS/1000.0);
			    stats.TxDataRate = (float)utalkStats.txBytes / ((float)STATS_UPDATE_PERIOD_MS/1000.0);
			    stats.RxFailures += utalkStats.rxErrors;
			    stats.TxFailures += txErrors;
			    stats.TxRetries += txRetries;
			    txErrors = 0;
			    txRetries = 0;
			    FlightTelemetryStatsSet(&stats);
			}
			// This is an object update, handle based on event type
			else
			{
				// Get object metadata
				UAVObjGetMetadata(ev.obj, &metadata);
				// Act on event
				retries = 0;
				success = -1;
				if(ev.event == EV_UPDATED || ev.event == EV_UPDATED_MANUAL)
				{
					// Send update to GCS (with retries)
					while(retries < MAX_RETRIES && success == -1)
					{
						success = UAVTalkSendObject(ev.obj, ev.instId, metadata.telemetryAcked, REQ_TIMEOUT_MS); // call blocks until ack is received or timeout
						++retries;
					}
					// Update stats
					txRetries += (retries-1);
					if ( success == -1 )
					{
						++txErrors;
					}
				}
				else if(ev.event == EV_UPDATE_REQ)
				{
					// Request object update from GCS (with retries)
					while(retries < MAX_RETRIES && success == -1)
					{
						success = UAVTalkSendObjectRequest(ev.obj, ev.instId, REQ_TIMEOUT_MS); // call blocks until update is received or timeout
						++retries;
					}
					// Update stats
					txRetries += (retries-1);
					if ( success == -1 )
					{
						++txErrors;
					}
				}
				// If this is a metaobject then make necessary telemetry updates
				if(UAVObjIsMetaobject(ev.obj))
				{
					updateObject(UAVObjGetLinkedObj(ev.obj)); // linked object will be the actual object the metadata are for
				}
			}
		}
	}
}

/**
 * Telemetry transmit task. Processes queue events and periodic updates.
 */
static void telemetryRxTask(void* parameters)
{
	COMPortTypeDef inputPort;
	int32_t len;

	// Task loop
	while (1)
	{
		// TODO: Disabled since the USB HID is not fully functional yet
		inputPort = telemetryPort; // force input port, remove once USB HID is tested
		// Determine input port (USB takes priority over telemetry port)
		//if(!PIOS_USB_HID_CheckAvailable())
		//{
		//	inputPort = telemetryPort;
		//}
		//else
		//{
		//	inputPort = COM_USB_HID;
		//}

		// Block until data are available
		// TODO: Update once the PIOS_COM is made blocking
		//xSemaphoreTake(PIOS_USART1_Buffer, portMAX_DELAY);
		len = PIOS_COM_ReceiveBufferUsed(inputPort);
		for (int32_t n = 0; n < len; ++n)
		{
			//PIOS_LED_Toggle(LED1);
			UAVTalkProcessInputStream(PIOS_COM_ReceiveBuffer(inputPort));
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
	COMPortTypeDef outputPort;

	// TODO: Disabled since the USB HID is not fully functional yet
	outputPort = telemetryPort; // force input port, remove once USB HID is tested
	// Determine input port (USB takes priority over telemetry port)
	//if(!PIOS_USB_HID_CheckAvailable())
	//{
	//	outputPort = telemetryPort;
	//}
	//else
	//{
	//	outputPort = COM_USB_HID;
	//}

	// TODO: Update once the PIOS_COM is made blocking (it is implemented as a busy loop for now!)
	//PIOS_LED_Toggle(LED2);
	return PIOS_COM_SendBuffer(outputPort, data, length);
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

