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

// Private constants
#define OVEROSYNC_PACKET_SIZE 256
#define MAX_QUEUE_SIZE   3
#define STACK_SIZE_BYTES 512
#define TASK_PRIORITY (tskIDLE_PRIORITY + 2)

// Private types

// Private variables
static xQueueHandle queue;
static xSemaphoreHandle lock;
static UAVTalkConnection uavTalkCon;
static xTaskHandle overoSyncTaskHandle;

// Private functions
static void overoSyncTask(void *parameters);
static int32_t transmitData(uint8_t * data, int32_t length);
static void transmitDataDone(bool crc_ok, uint8_t crc_val);
static void registerObject(UAVObjHandle obj);

// External variables
extern int32_t pios_spi_overo_id;

/**
 * Initialise the telemetry module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t OveroSyncInitialize(void)
{
	// Create object queues
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	// Create the semaphore for sending data
	lock = xSemaphoreCreateMutex();

	// Initialise UAVTalk
	uavTalkCon = UAVTalkInitialize(&transmitData);

	return 0;
}

/**
 * Initialise the telemetry module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t OveroSyncStart(void)
{
	// Process all registered objects and connect queue for updates
	UAVObjIterate(&registerObject);
	
	// Start telemetry tasks
	if(pios_spi_overo_id != 0)
		xTaskCreate(overoSyncTask, (signed char *)"OveroSync", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &overoSyncTaskHandle);
	
	// TaskMonitorAdd(TASKINFO_RUNNING_TELEMETRYRX, telemetryRxTaskHandle);
	
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


int32_t overosync_transfers = 0;
/**
 * Telemetry transmit task, regular priority
 */
static void overoSyncTask(void *parameters)
{
	UAVObjEvent ev;

	// Loop forever
	while (1) {
		// Wait for queue message
		if (xQueueReceive(queue, &ev, portMAX_DELAY) == pdTRUE) {
			// Lock
			xSemaphoreTake(lock, portMAX_DELAY);

			// Process event
			UAVTalkSendObject(uavTalkCon, ev.obj, ev.instId, false, 0);
			overosync_transfers++;
		}
	}
}

uint8_t tx_buffer[OVEROSYNC_PACKET_SIZE] __attribute__ ((aligned(4)));
uint8_t rx_buffer[OVEROSYNC_PACKET_SIZE] __attribute__ ((aligned(4)));

int32_t transactionsDone = 0;
static void transmitDataDone(bool crc_ok, uint8_t crc_val)
{
	transactionsDone ++;
	xSemaphoreGive(lock);
}

int32_t transactionsStarted = 0;
/**
 * Transmit data buffer to the modem or USB port.
 * \param[in] data Data buffer to send
 * \param[in] length Length of buffer
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static int32_t transmitData(uint8_t * data, int32_t length)
{
	memcpy(tx_buffer,data,length);
	memset(tx_buffer + length, 0xfe, sizeof(tx_buffer) - length);
//	memset(tx_buffer, 0x3d, sizeof(tx_buffer));
	int32_t retval = 0;
	
	transactionsStarted++;
	retval = PIOS_SPI_TransferBlock(pios_spi_overo_id, (uint8_t *) tx_buffer, (uint8_t *) rx_buffer, sizeof(tx_buffer), &transmitDataDone);
	
	for (uint32_t i = 0; rx_buffer[0] != 0 && i < sizeof(rx_buffer) ; i++)
		UAVTalkProcessInputStream(uavTalkCon, rx_buffer[i]);

	return retval;
}


/**
  * @}
  * @}
  */
