/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup TelemetryModule Telemetry Module
 * @brief Main telemetry module
 * Starts three tasks (RX, TX, and priority TX) that watch event queues
 * and handle all the telemetry of the UAVobjects
 * @{ 
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
#include "overosync.h"
#include "overosyncstats.h"
#include "systemstats.h"

// Private constants
#define OVEROSYNC_PACKET_SIZE 1024
#define MAX_QUEUE_SIZE   40
#define STACK_SIZE_BYTES 512
#define TASK_PRIORITY (tskIDLE_PRIORITY + 0)

// Private types

// Private variables
static xQueueHandle queue;
static UAVTalkConnection uavTalkCon;
static xTaskHandle overoSyncTaskHandle;
volatile bool buffer_swap_failed;
volatile uint32_t buffer_swap_timeval;
FILE * fid;

// Private functions
static void overoSyncTask(void *parameters);
static int32_t packData(uint8_t * data, int32_t length);
static void registerObject(UAVObjHandle obj);

struct dma_transaction {
	uint8_t tx_buffer[OVEROSYNC_PACKET_SIZE] __attribute__ ((aligned(4)));
	uint8_t rx_buffer[OVEROSYNC_PACKET_SIZE] __attribute__ ((aligned(4)));
};

struct overosync {
	struct dma_transaction transactions[2];
	uint32_t active_transaction_id;
	uint32_t loading_transaction_id;
	xSemaphoreHandle transaction_lock;
	xSemaphoreHandle buffer_lock;
	volatile bool transaction_done;
	uint32_t sent_bytes;
	uint32_t write_pointer;
	uint32_t sent_objects;
	uint32_t failed_objects;
	uint32_t received_objects;
	uint32_t framesync_error;
};

struct overosync *overosync;

/**
 * Initialise the telemetry module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t OveroSyncInitialize(void)
{
	OveroSyncStatsInitialize();

	// Create object queues
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	// Initialise UAVTalk
	uavTalkCon = UAVTalkInitialize(&packData);

	return 0;
}

/**
 * Initialise the telemetry module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t OveroSyncStart(void)
{
	overosync = (struct overosync *) pvPortMalloc(sizeof(*overosync));
	if(overosync == NULL)
		return -1;

	overosync->transaction_lock = xSemaphoreCreateMutex();
	if(overosync->transaction_lock == NULL)
		return -1;

	overosync->buffer_lock = xSemaphoreCreateMutex();
	if(overosync->buffer_lock == NULL)
		return -1;

	overosync->active_transaction_id = 0;
	overosync->loading_transaction_id = 0;
	overosync->write_pointer = 0;
	overosync->sent_bytes = 0;
	overosync->framesync_error = 0;

	// Process all registered objects and connect queue for updates
	UAVObjIterate(&registerObject);
	
	// Start telemetry tasks
	xTaskCreate(overoSyncTask, (signed char *)"OveroSync", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &overoSyncTaskHandle);
	
	TaskMonitorAdd(TASKINFO_RUNNING_OVEROSYNC, overoSyncTaskHandle);
	
	return 0;
}

MODULE_INITCALL(OveroSyncInitialize, OveroSyncStart)

/**
 * Register a new object, adds object to local list and connects the queue depending on the object's
 * telemetry settings.
 * \param[in] obj Object to connect
 */
static void registerObject(UAVObjHandle obj)
{
	int32_t eventMask;
	eventMask = EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ;
	if (UAVObjIsMetaobject(obj)) {
		eventMask |= EV_UNPACKED;	// we also need to act on remote updates (unpack events)
	}
	UAVObjConnectQueue(obj, queue, eventMask);
}

/**
 * Telemetry transmit task, regular priority
 *
 * Logic: We need to double buffer the DMA transfers.  Pack the buffer until either
 * 1) it is full (and then we should record the number of missed events then)
 * 2) the current transaction is done (we should immediately schedule since we are slave)
 * when done packing the buffer we should call PIOS_SPI_TransferBlock, change the active buffer
 * and then take the semaphrore
 */
static void overoSyncTask(void *parameters)
{
	UAVObjEvent ev;

	// Kick off SPI transfers (once one is completed another will automatically transmit)
	overosync->transaction_done = true;
	overosync->sent_objects = 0;
	overosync->failed_objects = 0;
	overosync->received_objects = 0;
	
	portTickType lastUpdateTime = xTaskGetTickCount();
	portTickType updateTime;
	
	fid = fopen("sim_log.opl", "w");

	// Loop forever
	while (1) {
		// Wait for queue message
		if (xQueueReceive(queue, &ev, portMAX_DELAY) == pdTRUE) {
		
			// Check it will fit before packetizing
			if ((overosync->write_pointer + UAVObjGetNumBytes(ev.obj) + 12) >=
				sizeof(overosync->transactions[overosync->loading_transaction_id].tx_buffer)) {
				overosync->failed_objects ++;
			} else {
				// Process event.  This calls transmitData
				UAVTalkSendObject(uavTalkCon, ev.obj, ev.instId, false, 0);
			}

			updateTime = xTaskGetTickCount();
			if(((portTickType) (updateTime - lastUpdateTime)) > 1000) {
				// Update stats.  This will trigger a local send event too
				OveroSyncStatsData syncStats;
				syncStats.Send = overosync->sent_bytes;
				syncStats.Received = 0;
				syncStats.Connected = syncStats.Send > 500 ? OVEROSYNCSTATS_CONNECTED_TRUE : OVEROSYNCSTATS_CONNECTED_FALSE;
				syncStats.DroppedUpdates = overosync->failed_objects;
				OveroSyncStatsSet(&syncStats);
				overosync->failed_objects = 0;
				overosync->sent_bytes = 0;
				lastUpdateTime = updateTime;
			}
		}
	}
}

/**
 * Transmit data buffer to the modem or USB port.
 * \param[in] data Data buffer to send
 * \param[in] length Length of buffer
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static int32_t packData(uint8_t * data, int32_t length)
{
	// Get the lock for manipulating the buffer
	xSemaphoreTake(overosync->buffer_lock, portMAX_DELAY);

	portTickType tickTime = xTaskGetTickCount();
	uint64_t packetSize = data[2] + (data[3] << 8);
	fwrite((void *) &tickTime, 1, sizeof(tickTime), fid);
	fwrite((void *) &packetSize, sizeof(packetSize), 1, fid);
	fwrite((void *) data, 1, length, fid);
	overosync->sent_bytes += length;
	overosync->sent_objects++;

	xSemaphoreGive(overosync->buffer_lock);
	
	return length;
}

/**
  * @}
  * @}
  */
