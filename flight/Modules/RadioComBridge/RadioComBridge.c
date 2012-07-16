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
#include <pipxstatus.h>
#include <objectpersistence.h>
#include <pipxsettings.h>
#include <uavtalk_priv.h>
#include <pios_rfm22b.h>
#include <ecc.h>
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
#include <pios_eeprom.h>
#endif

#include <stdbool.h>

// ****************
// Private constants

#define TEMP_BUFFER_SIZE 25
#define STACK_SIZE_BYTES 150
#define TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define BRIDGE_BUF_LEN 512
#define MAX_RETRIES 2
#define RETRY_TIMEOUT_MS 20
#define STATS_UPDATE_PERIOD_MS 500
#define RADIOSTATS_UPDATE_PERIOD_MS 250
#define MAX_LOST_CONTACT_TIME 4
#define PACKET_QUEUE_SIZE 10
#define MAX_PORT_DELAY 200
#define EV_PACKET_RECEIVED 0x20
#define EV_TRANSMIT_PACKET 0x30
#define EV_SEND_ACK 0x40
#define EV_SEND_NACK 0x50

// ****************
// Private types

typedef struct {
	uint32_t pairID;
	uint16_t retries;
	uint16_t errors;
	uint16_t uavtalk_errors;
	uint16_t resets;
	uint16_t dropped;
	int8_t rssi;
	uint8_t lastContact;
} PairStats;

typedef struct {
	uint32_t comPort;
	UAVTalkConnection UAVTalkCon;
	xQueueHandle sendQueue;
	xQueueHandle recvQueue;
	xQueueHandle gcsQueue;
	uint16_t wdg;
	bool checkHID;
} UAVTalkComTaskParams;

typedef struct {

	// The task handles.
	xTaskHandle GCSUAVTalkRecvTaskHandle;
	xTaskHandle UAVTalkRecvTaskHandle;
	xTaskHandle radioReceiveTaskHandle;
	xTaskHandle sendPacketTaskHandle;
	xTaskHandle UAVTalkSendTaskHandle;
	xTaskHandle radioStatusTaskHandle;
	xTaskHandle transparentCommTaskHandle;
	xTaskHandle ppmInputTaskHandle;

	// The UAVTalk connection on the com side.
	UAVTalkConnection UAVTalkCon;
	UAVTalkConnection GCSUAVTalkCon;

	// Queue handles.
	xQueueHandle radioPacketQueue;
	xQueueHandle gcsEventQueue;
	xQueueHandle uavtalkEventQueue;
	xQueueHandle ppmOutQueue;

	// Error statistics.
	uint32_t comTxErrors;
	uint32_t comTxRetries;
	uint32_t comRxErrors;
	uint32_t radioTxErrors;
	uint32_t radioTxRetries;
	uint32_t radioRxErrors;
	uint32_t UAVTalkErrors;
	uint32_t packetErrors;
	uint32_t droppedPackets;
	uint16_t txBytes;
	uint16_t rxBytes;

	// The destination ID
	uint32_t destination_id;

	// The packet timeout.
	portTickType send_timeout;
	uint16_t min_packet_size;

	// Track other radios that are in range.
	PairStats pairStats[PIPXSTATUS_PAIRIDS_NUMELEM];

	// The RSSI of the last packet received.
	int8_t RSSI;

	// Thread parameters.
	UAVTalkComTaskParams uavtalk_params;
	UAVTalkComTaskParams gcs_uavtalk_params;

} RadioComBridgeData;

typedef struct {
	uint32_t com_port;
	uint8_t *buffer;
	uint16_t length;
	uint16_t index;
	uint16_t data_length;
} ReadBuffer, *BufferedReadHandle;

// ****************
// Private functions

static void UAVTalkRecvTask(void *parameters);
static void radioReceiveTask(void *parameters);
static void sendPacketTask(void *parameters);
static void UAVTalkSendTask(void *parameters);
static void transparentCommTask(void * parameters);
static void radioStatusTask(void *parameters);
static void ppmInputTask(void *parameters);
static int32_t UAVTalkSendHandler(uint8_t * data, int32_t length);
static int32_t GCSUAVTalkSendHandler(uint8_t * data, int32_t length);
static int32_t transmitPacket(PHPacketHandle packet);
static void receiveData(uint8_t *buf, uint8_t len, int8_t rssi, int8_t afc);
static void transmitData(uint32_t outputPort, uint8_t *buf, uint8_t len, bool checkHid);
static void StatusHandler(PHStatusPacketHandle p, int8_t rssi, int8_t afc);
static void PPMHandler(uint16_t *channels);
static BufferedReadHandle BufferedReadInit(uint32_t com_port, uint16_t buffer_length);
static bool BufferedRead(BufferedReadHandle h, uint8_t *value, uint32_t timeout_ms);
static void BufferedReadSetCom(BufferedReadHandle h, uint32_t com_port);
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
		// Start the primary tasks for receiving/sending UAVTalk packets from the GCS.
		xTaskCreate(UAVTalkRecvTask, (signed char *)"GCSUAVTalkRecvTask", STACK_SIZE_BYTES, (void*)&(data->gcs_uavtalk_params), TASK_PRIORITY + 2, &(data->GCSUAVTalkRecvTaskHandle));
		xTaskCreate(UAVTalkSendTask, (signed char *)"GCSUAVTalkSendTask", STACK_SIZE_BYTES, (void*)&(data->gcs_uavtalk_params), TASK_PRIORITY+ 2, &(data->UAVTalkSendTaskHandle));

		// If a UAVTalk (non-GCS) com port is set it implies that the com port is connected on the flight side.
		// In this case we want to start another com thread on the HID port to talk to the GCS when connected.
		if (PIOS_COM_UAVTALK)
		{
			xTaskCreate(UAVTalkRecvTask, (signed char *)"UAVTalkRecvTask", STACK_SIZE_BYTES, (void*)&(data->uavtalk_params), TASK_PRIORITY + 2, &(data->UAVTalkRecvTaskHandle));
			xTaskCreate(UAVTalkSendTask, (signed char *)"UAVTalkSendTask", STACK_SIZE_BYTES, (void*)&(data->uavtalk_params), TASK_PRIORITY+ 2, &(data->UAVTalkSendTaskHandle));
		}

		// Start the tasks
		if(PIOS_COM_TRANS_COM)
			xTaskCreate(transparentCommTask, (signed char *)"transparentComm", STACK_SIZE_BYTES, NULL, TASK_PRIORITY + 2, &(data->transparentCommTaskHandle));
		xTaskCreate(radioReceiveTask, (signed char *)"RadioReceive", STACK_SIZE_BYTES, NULL, TASK_PRIORITY+ 2, &(data->radioReceiveTaskHandle));
		xTaskCreate(sendPacketTask, (signed char *)"SendPacketTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY + 2, &(data->sendPacketTaskHandle));
		xTaskCreate(radioStatusTask, (signed char *)"RadioStatus", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->radioStatusTaskHandle));
		if(PIOS_PPM_RECEIVER)
			xTaskCreate(ppmInputTask, (signed char *)"PPMInputTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY + 2, &(data->ppmInputTaskHandle));
#ifdef PIOS_INCLUDE_WDG
		PIOS_WDG_RegisterFlag(PIOS_WDG_COMGCS);
		if(PIOS_COM_UAVTALK)
			PIOS_WDG_RegisterFlag(PIOS_WDG_COMUAVTALK);
		if(PIOS_COM_TRANS_COM)
			PIOS_WDG_RegisterFlag(PIOS_WDG_TRANSCOMM);
		PIOS_WDG_RegisterFlag(PIOS_WDG_RADIORECEIVE);
		//PIOS_WDG_RegisterFlag(PIOS_WDG_SENDPACKET);
		if(PIOS_PPM_RECEIVER)
			PIOS_WDG_RegisterFlag(PIOS_WDG_PPMINPUT);
#endif
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
	PipXStatusInitialize();
	ObjectPersistenceInitialize();
	updateSettings();

	// Initialise UAVTalk
	data->GCSUAVTalkCon = UAVTalkInitialize(&GCSUAVTalkSendHandler);
	if (PIOS_COM_UAVTALK)
		data->UAVTalkCon = UAVTalkInitialize(&UAVTalkSendHandler);

	// Initialize the queues.
	data->ppmOutQueue = 0;
	data->radioPacketQueue = xQueueCreate(PACKET_QUEUE_SIZE, sizeof(UAVObjEvent));
	data->gcsEventQueue = xQueueCreate(PACKET_QUEUE_SIZE, sizeof(UAVObjEvent));
	if (PIOS_COM_UAVTALK)
		data->uavtalkEventQueue = xQueueCreate(PACKET_QUEUE_SIZE, sizeof(UAVObjEvent));
	else
	{
		data->uavtalkEventQueue = 0;
		data->ppmOutQueue = data->radioPacketQueue;
	}

	// Initialize the statistics.
	data->radioTxErrors = 0;
	data->radioTxRetries = 0;
	data->radioRxErrors = 0;
	data->comTxErrors = 0;
	data->comTxRetries = 0;
	data->comRxErrors = 0;
	data->UAVTalkErrors = 0;
	data->packetErrors = 0;
	data->RSSI = -127;

	// Register the callbacks with the packet handler
	PHRegisterOutputStream(pios_packet_handler, transmitPacket);
	PHRegisterDataHandler(pios_packet_handler, receiveData);
	PHRegisterPPMHandler(pios_packet_handler, PPMHandler);
	PHRegisterStatusHandler(pios_packet_handler, StatusHandler);

	// Initialize the packet send timeout
	data->send_timeout = 25; // ms
	data->min_packet_size = 50;

	// Initialize the detected device statistics.
	for (uint8_t i = 0; i < PIPXSTATUS_PAIRIDS_NUMELEM; ++i)
	{
		data->pairStats[i].pairID = 0;
		data->pairStats[i].rssi = -127;
		data->pairStats[i].retries = 0;
		data->pairStats[i].errors = 0;
		data->pairStats[i].uavtalk_errors = 0;
		data->pairStats[i].resets = 0;
		data->pairStats[i].dropped = 0;
		data->pairStats[i].lastContact = 0;
	}
	// The first slot is reserved for our current pairID
	PipXSettingsPairIDGet(&(data->pairStats[0].pairID));

	// Configure our UAVObjects for updates.
	UAVObjConnectQueue(UAVObjGetByID(PIPXSTATUS_OBJID), data->gcsEventQueue, EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ);
	UAVObjConnectQueue(UAVObjGetByID(GCSRECEIVER_OBJID), data->uavtalkEventQueue ? data->uavtalkEventQueue : data->gcsEventQueue, EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ);
	UAVObjConnectQueue(UAVObjGetByID(OBJECTPERSISTENCE_OBJID), data->gcsEventQueue, EV_UPDATED | EV_UPDATED_MANUAL);

	// Initialize the UAVTalk comm parameters.
	data->gcs_uavtalk_params.UAVTalkCon = data->GCSUAVTalkCon;
	data->gcs_uavtalk_params.sendQueue = data->radioPacketQueue;
	data->gcs_uavtalk_params.recvQueue = data->gcsEventQueue;
	data->gcs_uavtalk_params.wdg = PIOS_WDG_COMGCS;
	data->gcs_uavtalk_params.checkHID = true;
	data->gcs_uavtalk_params.comPort = PIOS_COM_GCS;
	if (PIOS_COM_UAVTALK)
	{
		data->gcs_uavtalk_params.sendQueue = data->uavtalkEventQueue;
		data->uavtalk_params.UAVTalkCon = data->UAVTalkCon;
		data->uavtalk_params.sendQueue = data->radioPacketQueue;
		data->uavtalk_params.recvQueue = data->uavtalkEventQueue;
		data->uavtalk_params.gcsQueue = data->gcsEventQueue;
		data->uavtalk_params.wdg = PIOS_WDG_COMUAVTALK;
		data->uavtalk_params.checkHID = false;
		data->uavtalk_params.comPort = PIOS_COM_UAVTALK;
	}

	return 0;
}
MODULE_INITCALL(RadioComBridgeInitialize, RadioComBridgeStart)

/**
 * Reads UAVTalk messages froma com port and creates packets out of them.
 */
static void UAVTalkRecvTask(void *parameters)
{
	UAVTalkComTaskParams *params = (UAVTalkComTaskParams *)parameters;
	PHPacketHandle p = NULL;

	// Create the buffered reader.
	BufferedReadHandle f = BufferedReadInit(params->comPort, TEMP_BUFFER_SIZE);

	while (1) {

#ifdef PIOS_INCLUDE_WDG
		// Update the watchdog timer.
		if (params->wdg)
			PIOS_WDG_UpdateFlag(params->wdg);
#endif /* PIOS_INCLUDE_WDG */

		// Receive from USB HID if available, otherwise UAVTalk com if it's available.
#if defined(PIOS_INCLUDE_USB)
		// Determine input port (USB takes priority over telemetry port)
		if (params->checkHID && PIOS_USB_CheckAvailable(0))
			BufferedReadSetCom(f, PIOS_COM_USB_HID);
		else
#endif /* PIOS_INCLUDE_USB */
		{
			if (params->comPort)
				BufferedReadSetCom(f, params->comPort);
			else
			{
				vTaskDelay(5);
				continue;
			}
		}

		// Read the next byte
		uint8_t rx_byte;
		if(!BufferedRead(f, &rx_byte, MAX_PORT_DELAY))
			continue;

		// Get a TX packet from the packet handler if required.
		if (p == NULL)
		{

			// Wait until we receive a sync.
			UAVTalkRxState state = UAVTalkProcessInputStreamQuiet(params->UAVTalkCon, rx_byte);
			if (state != UAVTALK_STATE_TYPE)
				continue;

			// Get a packet when we see the sync
			p = PHGetTXPacket(pios_packet_handler);

			// No packets available?
			if (p == NULL)
			{
				data->droppedPackets++;
				continue;
			}

			// Initialize the packet.
			p->header.destination_id = data->destination_id;
			p->header.source_id = PIOS_RFM22B_DeviceID(pios_rfm22b_id);
			p->header.type = PACKET_TYPE_DATA;
			p->data[0] = rx_byte;
			p->header.data_size = 1;
			continue;
		}

		// Insert this byte.
		p->data[p->header.data_size++] = rx_byte;

		// Keep reading until we receive a completed packet.
		UAVTalkRxState state = UAVTalkProcessInputStreamQuiet(params->UAVTalkCon, rx_byte);
		UAVTalkConnectionData *connection = (UAVTalkConnectionData*)(params->UAVTalkCon);
		UAVTalkInputProcessor *iproc = &(connection->iproc);

		if (state == UAVTALK_STATE_COMPLETE)
		{
			xQueueHandle sendQueue = params->sendQueue;
#if defined(PIOS_INCLUDE_USB)
			if (params->gcsQueue)
				if (PIOS_USB_CheckAvailable(0) && PIOS_COM_USB_HID)
					sendQueue = params->gcsQueue;
#endif /* PIOS_INCLUDE_USB */

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
					if (obj_per.ObjectID == PIPXSETTINGS_OBJID)
					{
						// Queue up the ACK.
						queueEvent(params->recvQueue, (void*)iproc->obj, iproc->instId, EV_SEND_ACK);

						// Is this a save, load, or delete?
						bool success = true;
						switch (obj_per.Operation)
						{
						case OBJECTPERSISTENCE_OPERATION_LOAD:
						{
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
							// Load the settings.
							PipXSettingsData pipxSettings;
							if (PIOS_EEPROM_Load((uint8_t*)&pipxSettings, sizeof(PipXSettingsData)) == 0)
								PipXSettingsSet(&pipxSettings);
							else
								success = false;
#endif
							break;
						}
						case OBJECTPERSISTENCE_OPERATION_SAVE:
						{
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
							// Save the settings.
							PipXSettingsData pipxSettings;
							PipXSettingsGet(&pipxSettings);
							int32_t ret = PIOS_EEPROM_Save((uint8_t*)&pipxSettings, sizeof(PipXSettingsData));
							if (ret != 0)
								success = false;
#endif
							break;
						}
						case OBJECTPERSISTENCE_OPERATION_DELETE:
						{
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
							// Erase the settings.
							PipXSettingsData pipxSettings;
							uint8_t *ptr = (uint8_t*)&pipxSettings;
							memset(ptr, 0, sizeof(PipXSettingsData));
							int32_t ret = PIOS_EEPROM_Save(ptr, sizeof(PipXSettingsData));
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

						// Release the packet, since we don't need it.
						PHReleaseTXPacket(pios_packet_handler, p);
					}
					else
					{
						// Otherwise, queue the packet for transmission.
						queueEvent(sendQueue, (void*)p, 0, EV_TRANSMIT_PACKET);
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
						queueEvent(params->recvQueue, (void*)iproc->obj, iproc->instId, EV_UPDATE_REQ);
						break;
					case UAVTALK_TYPE_OBJ_ACK:
						if (UAVObjUnpack(iproc->obj, iproc->instId, connection->rxBuffer) == 0)
							// Queue up an ACK
							queueEvent(params->recvQueue, (void*)iproc->obj, iproc->instId, EV_SEND_ACK);
						break;
					}

					// Release the packet, since we don't need it.
					PHReleaseTXPacket(pios_packet_handler, p);
				}
			}
			else
			{
				// Queue the packet for transmission.
				queueEvent(sendQueue, (void*)p, 0, EV_TRANSMIT_PACKET);
			}
			p = NULL;

		} else if(state == UAVTALK_STATE_ERROR) {
			xQueueHandle sendQueue = params->sendQueue;
#if defined(PIOS_INCLUDE_USB)
			if (params->gcsQueue)
				if (PIOS_USB_CheckAvailable(0) && PIOS_COM_USB_HID)
					sendQueue = params->gcsQueue;
#endif /* PIOS_INCLUDE_USB */
			data->UAVTalkErrors++;

			// Send a NACK if required.
			if((iproc->obj) && (iproc->type == UAVTALK_TYPE_OBJ_ACK))
			{
				// Queue up a NACK
				queueEvent(params->recvQueue, iproc->obj, iproc->instId, EV_SEND_NACK);

				// Release the packet and start over again.
				PHReleaseTXPacket(pios_packet_handler, p);
			}
			else
			{
				// Transmit the packet anyway...
				queueEvent(sendQueue, (void*)p, 0, EV_TRANSMIT_PACKET);
			}
			p = NULL;
		}
	}
}

/**
 * The radio to com bridge task.
 */
static void radioReceiveTask(void *parameters)
{
	PHPacketHandle p = NULL;

	/* Handle radio -> usart/usb direction */
	while (1) {
		uint32_t rx_bytes;

#ifdef PIOS_INCLUDE_WDG
		// Update the watchdog timer.
		PIOS_WDG_UpdateFlag(PIOS_WDG_RADIORECEIVE);
#endif /* PIOS_INCLUDE_WDG */

		// Get a RX packet from the packet handler if required.
		if (p == NULL)
			p = PHGetRXPacket(pios_packet_handler);

		if(p == NULL) {
			DEBUG_PRINTF(2, "RX Packet Unavailable.!\n\r");
			// Wait a bit for a packet to come available.
			vTaskDelay(5);
			continue;
		}

		// Receive data from the radio port
		rx_bytes = PIOS_COM_ReceiveBuffer(PIOS_COM_RADIO, (uint8_t*)p, PIOS_PH_MAX_PACKET, MAX_PORT_DELAY);
		if(rx_bytes == 0)
			continue;
		data->rxBytes += rx_bytes;

		// Verify that the packet is valid and pass it on.
		if(PHVerifyPacket(pios_packet_handler, p, rx_bytes) > 0) {
			UAVObjEvent ev;
			ev.obj = (UAVObjHandle)p;
			ev.event = EV_PACKET_RECEIVED;
			xQueueSend(data->gcsEventQueue, &ev, portMAX_DELAY);
		} else
		{
			data->packetErrors++;
			PHReceivePacket(pios_packet_handler, p, true);
		}
		p = NULL;
	}
}

/**
 * Send packets to the radio.
 */
static void sendPacketTask(void *parameters)
{
	UAVObjEvent ev;

	// Loop forever
	while (1) {
#ifdef PIOS_INCLUDE_WDG
		// Update the watchdog timer.
		//PIOS_WDG_UpdateFlag(PIOS_WDG_SENDPACKET);
#endif /* PIOS_INCLUDE_WDG */
		// Wait for a packet on the queue.
		if (xQueueReceive(data->radioPacketQueue, &ev, MAX_PORT_DELAY) == pdTRUE) {
			PHPacketHandle p = (PHPacketHandle)ev.obj;
			// Send the packet.
			if(!PHTransmitPacket(pios_packet_handler, p))
				PHReleaseRXPacket(pios_packet_handler, p);
		}
	}
}

/**
 * Send packets to the com port.
 */
static void UAVTalkSendTask(void *parameters)
{
	UAVTalkComTaskParams *params = (UAVTalkComTaskParams *)parameters;
	UAVObjEvent ev;

	// Loop forever
	while (1) {
#ifdef PIOS_INCLUDE_WDG
		// Update the watchdog timer.
		// NOTE: this is temporarily turned off becase PIOS_Com_SendBuffer appears to block for an uncontrollable time,
		// and SendBufferNonBlocking doesn't seem to be working in this case.
		//PIOS_WDG_UpdateFlag(PIOS_WDG_SENDDATA);
#endif /* PIOS_INCLUDE_WDG */
		// Wait for a packet on the queue.
		if (xQueueReceive(params->recvQueue, &ev, MAX_PORT_DELAY) == pdTRUE) {
			if ((ev.event == EV_UPDATED) || (ev.event == EV_UPDATE_REQ))
			{
				// Send update (with retries)
				uint32_t retries = 0;
				int32_t success = -1;
				while (retries < MAX_RETRIES && success == -1) {
					success = UAVTalkSendObject(params->UAVTalkCon, ev.obj, 0, 0, RETRY_TIMEOUT_MS) == 0;
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
					success = UAVTalkSendAck(params->UAVTalkCon, ev.obj, ev.instId) == 0;
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
					success = UAVTalkSendNack(params->UAVTalkCon, UAVObjGetID(ev.obj)) == 0;
					if (!success)
						++retries;
				}
				data->comTxRetries += retries;
			}
			else if(ev.event == EV_PACKET_RECEIVED)
			{
				// Receive the packet.
				PHReceivePacket(pios_packet_handler, (PHPacketHandle)ev.obj, false);
			}
			else if(ev.event == EV_TRANSMIT_PACKET)
			{
				// Transmit the packet.
				PHPacketHandle p = (PHPacketHandle)ev.obj;
				transmitData(params->comPort, p->data, p->header.data_size, params->checkHID);
				PHReleaseTXPacket(pios_packet_handler, p);
			}
		}
	}
}

/**
 * The com to radio bridge task.
 */
static void transparentCommTask(void * parameters)
{
	portTickType packet_start_time = 0;
	uint32_t timeout = MAX_PORT_DELAY;
	PHPacketHandle p = NULL;

	/* Handle usart/usb -> radio direction */
	while (1) {

#ifdef PIOS_INCLUDE_WDG
		// Update the watchdog timer.
		PIOS_WDG_UpdateFlag(PIOS_WDG_TRANSCOMM);
#endif /* PIOS_INCLUDE_WDG */

		// Get a TX packet from the packet handler if required.
		if (p == NULL)
		{
			p = PHGetTXPacket(pios_packet_handler);

			// No packets available?
			if (p == NULL)
			{
				data->droppedPackets++;
				// Wait a bit for a packet to come available.
				vTaskDelay(5);
				continue;
			}

			// Initialize the packet.
			p->header.destination_id = data->destination_id;
			p->header.source_id = PIOS_RFM22B_DeviceID(pios_rfm22b_id);
			//p->header.type = PACKET_TYPE_ACKED_DATA;
			p->header.type = PACKET_TYPE_DATA;
			p->header.data_size = 0;
		}

		// Receive data from the com port
		uint32_t cur_rx_bytes =	PIOS_COM_ReceiveBuffer(PIOS_COM_TRANS_COM, p->data + p->header.data_size,
							       PH_MAX_DATA - p->header.data_size, timeout);

		// Do we have an data to send?
		p->header.data_size += cur_rx_bytes;
		if (p->header.data_size > 0) {

			// Check how long since last update
			portTickType cur_sys_time = xTaskGetTickCount();

			// Is this the start of a packet?
			if(packet_start_time == 0)
				packet_start_time = cur_sys_time;

			// Just send the packet on wraparound
			bool send_packet = (cur_sys_time < packet_start_time);
			if (!send_packet)
			{
				portTickType dT = (cur_sys_time - packet_start_time) / portTICK_RATE_MS;
				if (dT > data->send_timeout)
					send_packet = true;
				else
					timeout = data->send_timeout - dT;
			}

			// Also send the packet if the size is over the minimum.
			send_packet |= (p->header.data_size > data->min_packet_size);

			// Should we send this packet?
			if (send_packet)
			{
				// Queue the packet for transmission.
				queueEvent(data->radioPacketQueue, (void*)p, 0, EV_TRANSMIT_PACKET);

				// Reset the timeout
				timeout = MAX_PORT_DELAY;
				p = NULL;
				packet_start_time = 0;
			}
		}
	}
}

/**
 * The stats update task.
 */
static void radioStatusTask(void *parameters)
{
	PHStatusPacket status_packet;

	while (1) {
		PipXStatusData pipxStatus;
		uint32_t pairID;

		// Get object data
		PipXStatusGet(&pipxStatus);
		PipXSettingsPairIDGet(&pairID);

		// Update the status
		pipxStatus.DeviceID = PIOS_RFM22B_DeviceID(pios_rfm22b_id);
		pipxStatus.Retries = data->comTxRetries;
		pipxStatus.Errors = data->packetErrors;
		pipxStatus.UAVTalkErrors = data->UAVTalkErrors;
		pipxStatus.Dropped = data->droppedPackets;
		pipxStatus.Resets = PIOS_RFM22B_Resets(pios_rfm22b_id);
		pipxStatus.TXRate = (uint16_t)((float)(data->txBytes * 1000) / STATS_UPDATE_PERIOD_MS);
		data->txBytes = 0;
		pipxStatus.RXRate = (uint16_t)((float)(data->rxBytes * 1000) / STATS_UPDATE_PERIOD_MS);
		data->rxBytes = 0;
		pipxStatus.LinkState = PIPXSTATUS_LINKSTATE_DISCONNECTED;
		pipxStatus.RSSI = data->RSSI;
		LINK_LED_OFF;

		// Update the potential pairing contacts
		for (uint8_t i = 0; i < PIPXSTATUS_PAIRIDS_NUMELEM; ++i)
		{
			pipxStatus.PairIDs[i] = data->pairStats[i].pairID;
			pipxStatus.PairSignalStrengths[i] = data->pairStats[i].rssi;
			data->pairStats[i].lastContact++;
			// Remove this device if it's stale.
			if(data->pairStats[i].lastContact > MAX_LOST_CONTACT_TIME)
			{
				data->pairStats[i].pairID = 0;
				data->pairStats[i].rssi = -127;
				data->pairStats[i].retries = 0;
				data->pairStats[i].errors = 0;
				data->pairStats[i].uavtalk_errors = 0;
				data->pairStats[i].resets = 0;
				data->pairStats[i].dropped = 0;
				data->pairStats[i].lastContact = 0;
			}
			// Add the paired devices statistics to ours.
			if(pairID && (data->pairStats[i].pairID == pairID) && (data->pairStats[i].rssi > -127))
			{
				pipxStatus.Retries += data->pairStats[i].retries;
				pipxStatus.Errors += data->pairStats[i].errors;
				pipxStatus.UAVTalkErrors += data->pairStats[i].uavtalk_errors;
				pipxStatus.Dropped += data->pairStats[i].dropped;
				pipxStatus.Resets += data->pairStats[i].resets;
				pipxStatus.Dropped += data->pairStats[i].dropped;
				pipxStatus.LinkState = PIPXSTATUS_LINKSTATE_CONNECTED;
				LINK_LED_ON;
			}
		}

		// Update the object
		PipXStatusSet(&pipxStatus);

		// Broadcast the status.
		{
			static uint16_t cntr = 0;
			if(cntr++ == RADIOSTATS_UPDATE_PERIOD_MS / STATS_UPDATE_PERIOD_MS)
			{
				// Queue the status message
				status_packet.header.destination_id = 0xffffffff;
				status_packet.header.type = PACKET_TYPE_STATUS;
				status_packet.header.data_size = PH_STATUS_DATA_SIZE(&status_packet);
				status_packet.header.source_id = pipxStatus.DeviceID;
				status_packet.retries = data->comTxRetries;
				status_packet.errors = data->packetErrors;
				status_packet.uavtalk_errors = data->UAVTalkErrors;
				status_packet.dropped = data->droppedPackets;
				status_packet.resets = PIOS_RFM22B_Resets(pios_rfm22b_id);
				PHPacketHandle sph = (PHPacketHandle)&status_packet;
				queueEvent(data->radioPacketQueue, (void*)sph, 0, EV_TRANSMIT_PACKET);
				cntr = 0;
			}
		}

		// Delay until the next update period.
		vTaskDelay(STATS_UPDATE_PERIOD_MS / portTICK_RATE_MS);
	}
}

/**
 * The PPM input task.
 */
static void ppmInputTask(void *parameters)
{
	PHPpmPacket ppm_packet;
	PHPacketHandle pph = (PHPacketHandle)&ppm_packet;

	while (1) {

#ifdef PIOS_INCLUDE_WDG
		// Update the watchdog timer.
		PIOS_WDG_UpdateFlag(PIOS_WDG_PPMINPUT);
#endif /* PIOS_INCLUDE_WDG */

		// Read the receiver.
		for (uint8_t i = 1; i <= PIOS_PPM_NUM_INPUTS; ++i)
			ppm_packet.channels[i - 1] = PIOS_RCVR_Read(PIOS_PPM_RECEIVER, i);

		// Send the PPM packet
		if (data->ppmOutQueue)
		{
			ppm_packet.header.destination_id = data->destination_id;
			ppm_packet.header.source_id = PIOS_RFM22B_DeviceID(pios_rfm22b_id);
			ppm_packet.header.type = PACKET_TYPE_PPM;
			ppm_packet.header.data_size = PH_PPM_DATA_SIZE(&ppm_packet);
			queueEvent(data->ppmOutQueue, (void*)pph, 0, EV_TRANSMIT_PACKET);
		}
		else
			PPMHandler(ppm_packet.channels);

		// Delay until the next update period.
		vTaskDelay(PIOS_PPM_PACKET_UPDATE_PERIOD_MS / portTICK_RATE_MS);
	}
}

/**
 * Transmit data buffer to the com port.
 * \param[in] params The comm parameters.
 * \param[in] buf Data buffer to send
 * \param[in] length Length of buffer
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static int32_t UAVTalkSend(UAVTalkComTaskParams *params, uint8_t *buf, int32_t length)
{
	uint32_t outputPort = params->comPort;
#if defined(PIOS_INCLUDE_USB)
	if (params->checkHID)
	{
		// Determine output port (USB takes priority over telemetry port)
		if (PIOS_USB_CheckAvailable(0) && PIOS_COM_USB_HID)
			outputPort = PIOS_COM_USB_HID;
	}
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
static int32_t UAVTalkSendHandler(uint8_t *buf, int32_t length)
{
	return UAVTalkSend(&(data->uavtalk_params), buf, length);
}

/**
 * Transmit data buffer to the com port.
 * \param[in] buf Data buffer to send
 * \param[in] length Length of buffer
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static int32_t GCSUAVTalkSendHandler(uint8_t *buf, int32_t length)
{
	return UAVTalkSend(&(data->gcs_uavtalk_params), buf, length);
}

/**
 * Transmit a packet to the radio port.
 * \param[in] buf Data buffer to send
 * \param[in] length Length of buffer
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static int32_t transmitPacket(PHPacketHandle p)
{
	data->txBytes += PH_PACKET_SIZE(p);
	return PIOS_COM_SendBuffer(PIOS_COM_RADIO, (uint8_t*)p, PH_PACKET_SIZE(p));
}

/**
 * Receive a packet
 * \param[in] buf The received data buffer
 * \param[in] length Length of buffer
 */
static void transmitData(uint32_t outputPort, uint8_t *buf, uint8_t len, bool checkHid)
{
#if defined(PIOS_INCLUDE_USB)
	// See if USB is connected if requested.
	if(checkHid)
		// Determine output port (USB takes priority over telemetry port)
		if (PIOS_USB_CheckAvailable(0) && PIOS_COM_USB_HID)
			outputPort = PIOS_COM_USB_HID;
#endif /* PIOS_INCLUDE_USB */
	if (!outputPort)
		return;

	// Send the received data to the com port
	if (PIOS_COM_SendBuffer(outputPort, buf, len) != len)
		// Error on transmit
		data->comTxErrors++;
}

/**
 * Receive a packet
 * \param[in] buf The received data buffer
 * \param[in] length Length of buffer
 */
static void receiveData(uint8_t *buf, uint8_t len, int8_t rssi, int8_t afc)
{
	data->RSSI = rssi;

	// Packet data should go to transparent com if it's configured,
	// USB HID if it's connected, otherwise, UAVTalk com if it's configured.
	uint32_t outputPort = PIOS_COM_TRANS_COM ? PIOS_COM_TRANS_COM : PIOS_COM_UAVTALK;
	bool checkHid = (PIOS_COM_TRANS_COM == 0);
	transmitData(outputPort, buf, len, checkHid);
}

/**
 * Receive a status packet
 * \param[in] status The status structure
 */
static void StatusHandler(PHStatusPacketHandle status, int8_t rssi, int8_t afc)
{
	uint32_t id = status->header.source_id;
	bool found = false;
	// Have we seen this device recently?
	uint8_t id_idx = 0;
	for ( ; id_idx < PIPXSTATUS_PAIRIDS_NUMELEM; ++id_idx)
		if(data->pairStats[id_idx].pairID == id)
		{
			found = true;
			break;
		}

	// If we have seen it, update the RSSI and reset the last contact couter
	if(found)
	{
		data->pairStats[id_idx].rssi = rssi;
		data->pairStats[id_idx].retries = status->retries;
		data->pairStats[id_idx].errors = status->errors;
		data->pairStats[id_idx].uavtalk_errors = status->uavtalk_errors;
		data->pairStats[id_idx].resets = status->resets;
		data->pairStats[id_idx].dropped = status->dropped;
		data->pairStats[id_idx].lastContact = 0;
	}

	// If we haven't seen it, find a slot to put it in.
	if (!found)
	{
		uint32_t pairID;
		PipXSettingsPairIDGet(&pairID);

		uint8_t min_idx = 0;
		if(id != pairID)
		{
			int8_t min_rssi = data->pairStats[0].rssi;
			for (id_idx = 1; id_idx < PIPXSTATUS_PAIRIDS_NUMELEM; ++id_idx)
			{
				if(data->pairStats[id_idx].rssi < min_rssi)
				{
					min_rssi = data->pairStats[id_idx].rssi;
					min_idx = id_idx;
				}
			}
		}
		data->pairStats[min_idx].pairID = id;
		data->pairStats[min_idx].rssi = rssi;
		data->pairStats[min_idx].retries = status->retries;
		data->pairStats[min_idx].errors = status->errors;
		data->pairStats[min_idx].uavtalk_errors = status->uavtalk_errors;
		data->pairStats[min_idx].resets = status->resets;
		data->pairStats[min_idx].dropped = status->dropped;
		data->pairStats[min_idx].lastContact = 0;
	}
}

/**
 * Receive a ppm packet
 * \param[in] channels The ppm channels
 */
static void PPMHandler(uint16_t *channels)
{
	GCSReceiverData rcvr;

	// Copy the receiver channels into the GCSReceiver object.
	for (uint8_t i = 0; i < GCSRECEIVER_CHANNEL_NUMELEM; ++i)
		rcvr.Channel[i] = channels[i];

	// Set the GCSReceiverData object.
	GCSReceiverSet(&rcvr);
}

static BufferedReadHandle BufferedReadInit(uint32_t com_port, uint16_t buffer_length)
{
	BufferedReadHandle h = (BufferedReadHandle)pvPortMalloc(sizeof(ReadBuffer));
	if (!h)
		return NULL;

	h->com_port = com_port;
	h->buffer = (uint8_t*)pvPortMalloc(buffer_length);
	h->length = buffer_length;
	h->index = 0;
	h->data_length = 0;

	if (h->buffer == NULL)
		return NULL;

	return h;
}

static bool BufferedRead(BufferedReadHandle h, uint8_t *value, uint32_t timeout_ms)
{
	// Read some data if required.
	if(h->index == h->data_length)
	{
		uint32_t rx_bytes = PIOS_COM_ReceiveBuffer(h->com_port, h->buffer, h->length, timeout_ms);
		if (rx_bytes == 0)
			return false;
		h->index = 0;
		h->data_length = rx_bytes;
	}

	// Return the next byte.
	*value = h->buffer[h->index++];
	return true;
}

static void BufferedReadSetCom(BufferedReadHandle h, uint32_t com_port)
{
	h->com_port = com_port;
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
	PipXSettingsData pipxSettings;
	PipXSettingsGet(&pipxSettings);

	// Initialize the destination ID
	data->destination_id = pipxSettings.PairID ? pipxSettings.PairID : 0xffffffff;
	
	if (PIOS_COM_TELEMETRY) {
		switch (pipxSettings.TelemetrySpeed) {
		case PIPXSETTINGS_TELEMETRYSPEED_2400:
			PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 2400);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_4800:
			PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 4800);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_9600:
			PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 9600);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_19200:
			PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 19200);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_38400:
			PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 38400);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_57600:
			PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 57600);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_115200:
			PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 115200);
			break;
		}
	}
	if (PIOS_COM_FLEXI) {
		switch (pipxSettings.FlexiSpeed) {
		case PIPXSETTINGS_TELEMETRYSPEED_2400:
			PIOS_COM_ChangeBaud(PIOS_COM_FLEXI, 2400);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_4800:
			PIOS_COM_ChangeBaud(PIOS_COM_FLEXI, 4800);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_9600:
			PIOS_COM_ChangeBaud(PIOS_COM_FLEXI, 9600);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_19200:
			PIOS_COM_ChangeBaud(PIOS_COM_FLEXI, 19200);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_38400:
			PIOS_COM_ChangeBaud(PIOS_COM_FLEXI, 38400);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_57600:
			PIOS_COM_ChangeBaud(PIOS_COM_FLEXI, 57600);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_115200:
			PIOS_COM_ChangeBaud(PIOS_COM_FLEXI, 115200);
			break;
		}
	}
	if (PIOS_COM_VCP) {
		switch (pipxSettings.VCPSpeed) {
		case PIPXSETTINGS_TELEMETRYSPEED_2400:
			PIOS_COM_ChangeBaud(PIOS_COM_VCP, 2400);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_4800:
			PIOS_COM_ChangeBaud(PIOS_COM_VCP, 4800);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_9600:
			PIOS_COM_ChangeBaud(PIOS_COM_VCP, 9600);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_19200:
			PIOS_COM_ChangeBaud(PIOS_COM_VCP, 19200);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_38400:
			PIOS_COM_ChangeBaud(PIOS_COM_VCP, 38400);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_57600:
			PIOS_COM_ChangeBaud(PIOS_COM_VCP, 57600);
			break;
		case PIPXSETTINGS_TELEMETRYSPEED_115200:
			PIOS_COM_ChangeBaud(PIOS_COM_VCP, 115200);
			break;
		}
	}
}
