/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup RadioComBridgeModule Com Port to Radio Bridge Module
 * @brief Bridge Com and Radio ports
 * @{ 
 *
 * @file       RadioComBridge.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Bridges selected Com Port to the COM VCP emulated serial port
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

// ****************

#include <openpilot.h>
#include <radiocombridge.h>
#include <packet_handler.h>
#include <gcsreceiver.h>
#include <oplinkstatus.h>
#include <objectpersistence.h>
#include <oplinksettings.h>
#include <uavtalk_priv.h>
#include <pios_rfm22b.h>
#include <ecc.h>
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
#include <pios_eeprom.h>
#endif

#include <stdbool.h>

// ****************
// Private constants

#define STACK_SIZE_BYTES 150
#define TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define MAX_RETRIES 2
#define RETRY_TIMEOUT_MS 20
#define EVENT_QUEUE_SIZE 10
#define MAX_PORT_DELAY 200
#define EV_SEND_ACK 0x20
#define EV_SEND_NACK 0x30

// ****************
// Private types

typedef struct {

	// The task handles.
	xTaskHandle telemetryTxTaskHandle;
	xTaskHandle radioRxTaskHandle;
	xTaskHandle radioTxTaskHandle;

	// The UAVTalk connection on the com side.
	UAVTalkConnection outUAVTalkCon;
	UAVTalkConnection inUAVTalkCon;

	// Queue handles.
	xQueueHandle gcsEventQueue;
	xQueueHandle uavtalkEventQueue;

	// Error statistics.
	uint32_t comTxErrors;
	uint32_t comTxRetries;
	uint32_t UAVTalkErrors;
	uint32_t droppedPackets;

} RadioComBridgeData;

// ****************
// Private functions

static void telemetryTxTask(void *parameters);
static void radioRxTask(void *parameters);
static void radioTxTask(void *parameters);
static void testRxTask(void *parameters);
static void testTxTask(void *parameters);
static int32_t UAVTalkSendHandler(uint8_t *buf, int32_t length);
static int32_t RadioSendHandler(uint8_t *buf, int32_t length);
static void ProcessInputStream(UAVTalkConnection connectionHandle, uint8_t rxbyte);
static void queueEvent(xQueueHandle queue, void *obj, uint16_t instId, UAVObjEventType type);
static void updateSettings();

// ****************
// Private variables

static RadioComBridgeData *data;

/**
 * Start the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
static int32_t RadioComBridgeStart(void)
{
	if(data) {

		// Set the baudrates, etc.
		updateSettings();

		// Start the primary tasks for receiving/sending UAVTalk packets from the GCS.
		xTaskCreate(telemetryTxTask, (signed char *)"telemTxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->telemetryTxTaskHandle));
		xTaskCreate(radioRxTask, (signed char *)"radioRxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->radioRxTaskHandle));
		xTaskCreate(radioTxTask, (signed char *)"radioTxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->radioTxTaskHandle));
		if (0)
		{
			xTaskCreate(testRxTask, (signed char *)"testRxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->radioRxTaskHandle));
			xTaskCreate(testTxTask, (signed char *)"testTxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->radioTxTaskHandle));
		}

		return 0;
	}

	return -1;
}

/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
static int32_t RadioComBridgeInitialize(void)
{

	// allocate and initialize the static data storage only if module is enabled
	data = (RadioComBridgeData *)pvPortMalloc(sizeof(RadioComBridgeData));
	if (!data)
		return -1;

	// Initialize the UAVObjects that we use
	GCSReceiverInitialize();
	OPLinkStatusInitialize();
	ObjectPersistenceInitialize();

	// Initialise UAVTalk
	data->outUAVTalkCon = UAVTalkInitialize(&UAVTalkSendHandler);
	data->inUAVTalkCon = UAVTalkInitialize(&RadioSendHandler);

	// Initialize the queues.
	data->uavtalkEventQueue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(UAVObjEvent));

	// Configure our UAVObjects for updates.
	UAVObjConnectQueue(UAVObjGetByID(OPLINKSTATUS_OBJID), data->uavtalkEventQueue, EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ);
	UAVObjConnectQueue(UAVObjGetByID(OBJECTPERSISTENCE_OBJID), data->uavtalkEventQueue, EV_UPDATED | EV_UPDATED_MANUAL);
	//UAVObjConnectQueue(UAVObjGetByID(GCSRECEIVER_OBJID), data->uavtalkEventQueue, EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ);

	// Initialize the statistics.
	data->comTxErrors = 0;
	data->comTxRetries = 0;
	data->UAVTalkErrors = 0;

	return 0;
}
MODULE_INITCALL(RadioComBridgeInitialize, RadioComBridgeStart)

/**
 * Telemetry transmit task, regular priority
 */
static void telemetryTxTask(void *parameters)
{
	UAVObjEvent ev;

	// Loop forever
	while (1) {
		// Wait for queue message
		if (xQueueReceive(data->uavtalkEventQueue, &ev, portMAX_DELAY) == pdTRUE) {
			if ((ev.event == EV_UPDATED) || (ev.event == EV_UPDATE_REQ))
			{
				// Send update (with retries)
				uint32_t retries = 0;
				int32_t success = -1;
				while (retries < MAX_RETRIES && success == -1) {
					success = UAVTalkSendObject(data->outUAVTalkCon, ev.obj, 0, 0, RETRY_TIMEOUT_MS) == 0;
					if (!success)
						++retries;
				}
				data->comTxRetries += retries;
			}
			else if(ev.event == EV_SEND_ACK)
			{
				// Send the ACK
				uint32_t retries = 0;
				int32_t success = -1;
				while (retries < MAX_RETRIES && success == -1) {
					success = UAVTalkSendAck(data->outUAVTalkCon, ev.obj, ev.instId) == 0;
					if (!success)
						++retries;
				}
				data->comTxRetries += retries;
			}
			else if(ev.event == EV_SEND_NACK)
			{
				// Send the NACK
				uint32_t retries = 0;
				int32_t success = -1;
				while (retries < MAX_RETRIES && success == -1) {
					success = UAVTalkSendNack(data->outUAVTalkCon, UAVObjGetID(ev.obj)) == 0;
					if (!success)
						++retries;
				}
				data->comTxRetries += retries;
			}
		}
	}
}

/**
 * Radio rx task.  Receive data packets from the radio and pass them on.
 */
static void radioRxTask(void *parameters)
{
	// Task loop
	while (1) {
		uint8_t serial_data[1];
		uint16_t bytes_to_process = PIOS_COM_ReceiveBuffer(PIOS_COM_RADIO, serial_data, sizeof(serial_data), MAX_PORT_DELAY);
		if (bytes_to_process > 0)
			for (uint8_t i = 0; i < bytes_to_process; i++)
				if (UAVTalkRelayInputStream(data->outUAVTalkCon, serial_data[i]) == UAVTALK_STATE_ERROR)
					data->UAVTalkErrors++;
	}
}

/**
 * Radio rx task.  Receive data from a com port and pass it on to the radio.
 */
static void radioTxTask(void *parameters)
{
	// Task loop
	while (1) {
		uint32_t inputPort = PIOS_COM_TELEMETRY;
#if defined(PIOS_INCLUDE_USB)
		// Determine output port (USB takes priority over telemetry port)
		if (PIOS_USB_CheckAvailable(0) && PIOS_COM_TELEM_USB)
			inputPort = PIOS_COM_TELEM_USB;
#endif /* PIOS_INCLUDE_USB */
		if(inputPort)
		{
			uint8_t serial_data[1];
			uint16_t bytes_to_process = PIOS_COM_ReceiveBuffer(inputPort, serial_data, sizeof(serial_data), MAX_PORT_DELAY);
			if (bytes_to_process > 0)
				for (uint8_t i = 0; i < bytes_to_process; i++)
					ProcessInputStream(data->inUAVTalkCon, serial_data[i]);
		}
	}
}

static void testTxTask(void *parameters)
{
	uint8_t buf[100];
	for (uint8_t i = 0; i < 100; ++i)
		buf[i] = i;

	// Task loop
	uint16_t packets_sent = 0;
	while (1) {
		PIOS_COM_SendBuffer(PIOS_COM_RADIO, buf, 100);
		packets_sent++;
		OPLinkStatusTXRateSet(&packets_sent);
		vTaskDelay(1000);
	}
}

/**
 * Radio rx task.  Receive data from a com port and pass it on to the radio.
 */
static void testRxTask(void *parameters)
{
	// Task loop
	uint16_t packets_recv = 0;
	while (1) {
		uint8_t buf[100];
		int16_t bytes_received = PIOS_COM_ReceiveBuffer(PIOS_COM_RADIO, buf, 100, MAX_PORT_DELAY);
		if (bytes_received > 0) {
			packets_recv++;
			OPLinkStatusRXRateSet(&packets_recv);
		}
	}
}

/**
 * Transmit data buffer to the com port.
 * \param[in] buf Data buffer to send
 * \param[in] length Length of buffer
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static int32_t UAVTalkSendHandler(uint8_t *buf, int32_t length)
{
	uint32_t outputPort = PIOS_COM_TELEMETRY;
#if defined(PIOS_INCLUDE_USB)
	// Determine output port (USB takes priority over telemetry port)
	if (PIOS_USB_CheckAvailable(0) && PIOS_COM_TELEM_USB)
		outputPort = PIOS_COM_TELEM_USB;
#endif /* PIOS_INCLUDE_USB */
	if(outputPort)
		return PIOS_COM_SendBuffer(outputPort, buf, length);
	else
		return -1;
}

/**
 * Transmit data buffer to the com port.
 * \param[in] buf Data buffer to send
 * \param[in] length Length of buffer
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static int32_t RadioSendHandler(uint8_t *buf, int32_t length)
{
	uint32_t outputPort = PIOS_COM_RADIO;
	if(outputPort)
		return PIOS_COM_SendBuffer(outputPort, buf, length);
	else
		return -1;
}

static void ProcessInputStream(UAVTalkConnection connectionHandle, uint8_t rxbyte)
{
	// Keep reading until we receive a completed packet.
	UAVTalkRxState state = UAVTalkRelayInputStream(connectionHandle, rxbyte);
	UAVTalkConnectionData *connection = (UAVTalkConnectionData*)(connectionHandle);
	UAVTalkInputProcessor *iproc = &(connection->iproc);

	if (state == UAVTALK_STATE_COMPLETE)
 	{
		// Is this a local UAVObject?
		// We only generate GcsReceiver ojects, we don't consume them.
		if ((iproc->obj != NULL) && (iproc->objId != GCSRECEIVER_OBJID))
		{
			// We treat the ObjectPersistence object differently
			if(iproc->objId == OBJECTPERSISTENCE_OBJID)
			{
				// Unpack object, if the instance does not exist it will be created!
				UAVObjUnpack(iproc->obj, iproc->instId, connection->rxBuffer);

				// Get the ObjectPersistence object.
				ObjectPersistenceData obj_per;
				ObjectPersistenceGet(&obj_per);

				// Is this concerning or setting object?
				if (obj_per.ObjectID == OPLINKSETTINGS_OBJID)
				{
					// Queue up the ACK.
					queueEvent(data->uavtalkEventQueue, (void*)iproc->obj, iproc->instId, EV_SEND_ACK);

					// Is this a save, load, or delete?
					bool success = true;
					switch (obj_per.Operation)
					{
					case OBJECTPERSISTENCE_OPERATION_LOAD:
					{
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
						// Load the settings.
						OPLinkSettingsData oplinkSettings;
						if (PIOS_EEPROM_Load((uint8_t*)&oplinkSettings, sizeof(OPLinkSettingsData)) == 0)
							OPLinkSettingsSet(&oplinkSettings);
						else
							success = false;
#endif
						break;
					}
					case OBJECTPERSISTENCE_OPERATION_SAVE:
					{
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
						// Save the settings.
						OPLinkSettingsData oplinkSettings;
						OPLinkSettingsGet(&oplinkSettings);
						int32_t ret = PIOS_EEPROM_Save((uint8_t*)&oplinkSettings, sizeof(OPLinkSettingsData));
						if (ret != 0)
							success = false;
#endif
						break;
					}
					case OBJECTPERSISTENCE_OPERATION_DELETE:
					{
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
						// Erase the settings.
						OPLinkSettingsData oplinkSettings;
						uint8_t *ptr = (uint8_t*)&oplinkSettings;
						memset(ptr, 0, sizeof(OPLinkSettingsData));
						int32_t ret = PIOS_EEPROM_Save(ptr, sizeof(OPLinkSettingsData));
						if (ret != 0)
							success = false;
#endif
						break;
					}
					default:
						break;
					}
					if (success == true)
					{
						obj_per.Operation = OBJECTPERSISTENCE_OPERATION_COMPLETED;
						ObjectPersistenceSet(&obj_per);
					}
				}
			}
			else
			{
				switch (iproc->type)
				{
				case UAVTALK_TYPE_OBJ:
					// Unpack object, if the instance does not exist it will be created!
					UAVObjUnpack(iproc->obj, iproc->instId, connection->rxBuffer);
					break;
				case UAVTALK_TYPE_OBJ_REQ:
					// Queue up an object send request.
					queueEvent(data->uavtalkEventQueue, (void*)iproc->obj, iproc->instId, EV_UPDATE_REQ);
					break;
				case UAVTALK_TYPE_OBJ_ACK:
					if (UAVObjUnpack(iproc->obj, iproc->instId, connection->rxBuffer) == 0)
						// Queue up an ACK
						queueEvent(data->uavtalkEventQueue, (void*)iproc->obj, iproc->instId, EV_SEND_ACK);
					break;
				}
			}
		}

	} else if(state == UAVTALK_STATE_ERROR) {
		data->UAVTalkErrors++;

		// Send a NACK if required.
		if((iproc->obj) && (iproc->type == UAVTALK_TYPE_OBJ_ACK))
			// Queue up a NACK
			queueEvent(data->uavtalkEventQueue, iproc->obj, iproc->instId, EV_SEND_NACK);
 	}
}

/**
 * Queue and event into an event queue.
 * \param[in] queue  The event queue
 * \param[in] obj  The data pointer
 * \param[in] type The event type
 */
static void queueEvent(xQueueHandle queue, void *obj, uint16_t instId, UAVObjEventType type)
{
	UAVObjEvent ev;
	ev.obj = (UAVObjHandle)obj;
	ev.instId = instId;
	ev.event = type;
	xQueueSend(queue, &ev, portMAX_DELAY);
}

 /**
 * Update the telemetry settings, called on startup.
 * FIXME: This should be in the TelemetrySettings object. But objects
 * have too much overhead yet. Also the telemetry has no any specific
 * settings, etc. Thus the HwSettings object which contains the
 * telemetry port speed is used for now.
 */
static void updateSettings()
{

	// Get the settings.
	OPLinkSettingsData oplinkSettings;
	OPLinkSettingsGet(&oplinkSettings);
	
	switch (oplinkSettings.ComSpeed) {
	case OPLINKSETTINGS_COMSPEED_2400:
		if (PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 2400);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 2400);
		break;
	case OPLINKSETTINGS_COMSPEED_4800:
		if (PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 4800);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 4800);
		break;
	case OPLINKSETTINGS_COMSPEED_9600:
		if (PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 9600);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 9600);
		break;
	case OPLINKSETTINGS_COMSPEED_19200:
		if (PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 19200);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 19200);
		break;
	case OPLINKSETTINGS_COMSPEED_38400:
		if (PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 38400);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 38400);
		break;
	case OPLINKSETTINGS_COMSPEED_57600:
		if (PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 57600);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 57600);
		break;
	case OPLINKSETTINGS_COMSPEED_115200:
		if (PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 115200);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 115200);
		break;
	}

	// Set the frequencies.
	PIOS_RFM22B_SetFrequencyRange(pios_rfm22b_id, oplinkSettings.MinFrequency, oplinkSettings.MaxFrequency);

	// Set the maximum radio RF power.
	switch (oplinkSettings.MaxRFPower)
	{
	case OPLINKSETTINGS_MAXRFPOWER_125:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_0);
		break;
	case OPLINKSETTINGS_MAXRFPOWER_16:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_1);
		break;
	case OPLINKSETTINGS_MAXRFPOWER_316:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_2);
		break;
	case OPLINKSETTINGS_MAXRFPOWER_63:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_3);
		break;
	case OPLINKSETTINGS_MAXRFPOWER_126:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_4);
		break;
	case OPLINKSETTINGS_MAXRFPOWER_25:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_5);
		break;
	case OPLINKSETTINGS_MAXRFPOWER_50:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_6);
		break;
	case OPLINKSETTINGS_MAXRFPOWER_100:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_7);
		break;
	}

	// Set the radio destination ID.
	PIOS_RFM22B_SetDestinationId(pios_rfm22b_id, oplinkSettings.PairID);
}
