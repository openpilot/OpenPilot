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
#include <uavtalk_priv.h>
#include <ecc.h>

#include <stdbool.h>

#undef PIOS_INCLUDE_USB

// ****************
// Private functions

static void radio2ComBridgeTask(void *parameters);
static void com2RadioBridgeTask(void *parameters);
static int32_t transmitData(uint8_t * data, int32_t length);
static int32_t transmitPacket(PHPacketHandle packet);
static void receiveData(uint8_t *buf, uint8_t len);
static void SendGCSReceiver(void);
static void PPMHandler(uint16_t *channels);
static void updateSettings();

// ****************
// Private constants

#define STACK_SIZE_BYTES 300
#define TASK_PRIORITY (tskIDLE_PRIORITY + 1)

#define BRIDGE_BUF_LEN 128

#define MAX_RETRIES 2
#define REQ_TIMEOUT_MS 10

// ****************
// Private types

typedef struct {
	// The task handles.
	xTaskHandle radio2ComBridgeTaskHandle;
	xTaskHandle com2RadioBridgeTaskHandle;

	// The com buffers.
	uint8_t *radio2com_buf;
	uint8_t *com2radio_buf;

	// The com ports
	uint32_t com_port;
	uint32_t radio_port;

	// The UAVTalk connection on the com side.
	UAVTalkConnection uavTalkCon;

	// Error statistics.
	uint32_t com_tx_errors;
	uint32_t radio_tx_errors;

	// The packet timeout.
	portTickType send_timeout;
	uint16_t min_packet_size;

	// Flag used to indicate an update of the GCSReceiver object.
	bool send_gcsreceiver;

	// Tracks the UAVTalk messages transmitted from radio to com.
	bool uavtalk_idle;
	int16_t uavtalk_packet_len;
	int16_t uavtalk_packet_index;

} RadioComBridgeData;

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
		// Start the tasks
		xTaskCreate(radio2ComBridgeTask, (signed char *)"Radio2ComBridge", STACK_SIZE_BYTES/2, NULL, TASK_PRIORITY, &(data->radio2ComBridgeTaskHandle));
		xTaskCreate(com2RadioBridgeTask, (signed char *)"Com2RadioBridge", STACK_SIZE_BYTES/2, NULL, TASK_PRIORITY, &(data->com2RadioBridgeTaskHandle));
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

	// Initialize the GCSReceiver object.
	GCSReceiverInitialize();
	data->send_gcsreceiver = false;

	// TODO: Get from settings object
	data->com_port = PIOS_COM_BRIDGE_COM;
	data->radio_port = PIOS_COM_BRIDGE_RADIO;

	// Allocate the com buffers.
	data->radio2com_buf = pvPortMalloc(BRIDGE_BUF_LEN);
	PIOS_Assert(data->radio2com_buf);
	data->com2radio_buf = pvPortMalloc(BRIDGE_BUF_LEN);
	PIOS_Assert(data->com2radio_buf);

	// Initialise UAVTalk
	data->uavTalkCon = UAVTalkInitialize(&transmitData);

	// Initialize the statistics.
	data->com_tx_errors = 0;
	data->radio_tx_errors = 0;

	// Register the callbacks with the packet handler
	PHRegisterOutputStream(pios_packet_handler, transmitPacket);
	PHRegisterDataHandler(pios_packet_handler, receiveData);
	PHRegisterPPMHandler(pios_packet_handler, PPMHandler);

	// Initialize the packet send timeout
	data->send_timeout = 25; // ms
	data->min_packet_size = 50;

	// Initialize the rado->com UAVTalk message tracker.
	data->uavtalk_idle = true;
	data->uavtalk_packet_len = -1;
	data->uavtalk_packet_index = -1;

	updateSettings();

	return 0;
}
MODULE_INITCALL(RadioComBridgeInitialize, RadioComBridgeStart)

/**
 * The radio to com bridge task.
 */
static void radio2ComBridgeTask(void *parameters)
{
	/* Handle radio -> usart/usb direction */
	while (1) {
		uint32_t rx_bytes;

		// Receive data from the radio port
		rx_bytes = PIOS_COM_ReceiveBuffer(data->radio_port, data->radio2com_buf, BRIDGE_BUF_LEN, 500);
		if (rx_bytes > 0)
			PHReceivePacket(pios_packet_handler, (PHPacketHandle)data->radio2com_buf);
	}
}

/**
 * The com to radio bridge task.
 */
static void com2RadioBridgeTask(void * parameters)
{
	uint32_t rx_bytes = 0;
	portTickType packet_start_time = 0;
	uint32_t timeout = 500;
	uint32_t inputPort;

	/* Handle usart/usb -> radio direction */
	while (1) {
#if defined(PIOS_INCLUDE_USB)
		// Determine input port (USB takes priority over telemetry port)
		if (PIOS_USB_CheckAvailable(0) && PIOS_COM_TELEM_USB)
			inputPort = PIOS_COM_TELEM_USB;
		else
#endif /* PIOS_INCLUDE_USB */
			inputPort = data->com_port;

		// Receive data from the com port
		uint32_t cur_rx_bytes = PIOS_COM_ReceiveBuffer(inputPort, data->com2radio_buf +
							       rx_bytes, BRIDGE_BUF_LEN - rx_bytes, timeout);

		// Pass the new data through UAVTalk
		for (uint8_t i = 0; i < cur_rx_bytes; i++)
			UAVTalkProcessInputStream(data->uavTalkCon, *(data->com2radio_buf + i + rx_bytes));

		// Do we have an data to send?
		rx_bytes += cur_rx_bytes;
		if (rx_bytes > 0) {

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
			send_packet |= (rx_bytes > data->min_packet_size);

			// Should we send this packet?
			if (send_packet)
			{
				// Get a TX packet from the packet handler
				PHPacketHandle p = PHGetTXPacket(pios_packet_handler);

				// Initialize the packet.
				//p->header.type = PACKET_TYPE_ACKED_DATA;
				p->header.type = PACKET_TYPE_DATA;
				p->header.data_size = rx_bytes;

				// Copy the data into the packet.
				memcpy(p->data, data->com2radio_buf, rx_bytes);

				// Transmit the packet
				PHTransmitPacket(pios_packet_handler, p);

				// Reset the timeout
				timeout = 500;
				rx_bytes = 0;
				packet_start_time = 0;
			}
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
static int32_t transmitData(uint8_t *buf, int32_t length)
{
	uint32_t outputPort = data->com_port;
#if defined(PIOS_INCLUDE_USB)
	// Determine output port (USB takes priority over telemetry port)
	if (PIOS_USB_CheckAvailable(0) && PIOS_COM_TELEM_USB)
		outputPort = PIOS_COM_TELEM_USB;
#endif /* PIOS_INCLUDE_USB */
	return PIOS_COM_SendBuffer(outputPort, buf, length);
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
	return PIOS_COM_SendBuffer(data->radio_port, (uint8_t*)p, PH_PACKET_SIZE(p));
}

/**
 * Receive a packet
 * \param[in] buf The received data buffer
 * \param[in] length Length of buffer
 */
static void receiveData(uint8_t *buf, uint8_t len)
{
	uint32_t outputPort = data->com_port;
#if defined(PIOS_INCLUDE_USB)
	// Determine output port (USB takes priority over telemetry port)
	if (PIOS_USB_CheckAvailable(0) && PIOS_COM_TELEM_USB)
		outputPort = PIOS_COM_TELEM_USB;
#endif /* PIOS_INCLUDE_USB */

	// Send a local UAVTalk message if one is waiting to be sent.
	if (data->send_gcsreceiver && data->uavtalk_idle)
		SendGCSReceiver();

	// Parse the data for UAVTalk packets.
	uint8_t sent_bytes = 0;
	for (uint8_t i = 0; i < len; ++i)
	{
		uint8_t val = buf[i];

		// Looking for sync value
		if (data->uavtalk_packet_index == -1)
		{
			if (val == UAVTALK_SYNC_VAL)
				data->uavtalk_packet_index = 1;
		}
		else
		{
			if (data->uavtalk_packet_index == 2)
				// Length LSB
				data->uavtalk_packet_len = val;
			else if (data->uavtalk_packet_index == 3)
			{
				// Length MSB
				data->uavtalk_packet_len |= val << 8;
				// Is the length valid.
				if ((data->uavtalk_packet_len < UAVTALK_MIN_HEADER_LENGTH) ||
						(data->uavtalk_packet_len > UAVTALK_MAX_HEADER_LENGTH + UAVTALK_MAX_PAYLOAD_LENGTH))
					data->uavtalk_packet_index = -2;
			}
			else if (data->uavtalk_packet_index == data->uavtalk_packet_len)
			{
				// Packet received.
				data->uavtalk_packet_index = -2;

				// Send the buffer up to this point
				uint8_t send_bytes = len - sent_bytes;
				if (PIOS_COM_SendBuffer(outputPort, buf + sent_bytes, send_bytes) != send_bytes)
					// Error on transmit
					data->com_tx_errors++;
				sent_bytes = i;

				// Send any UAVTalk messages that need to be sent.
				if (data->send_gcsreceiver)
					SendGCSReceiver();
			}
			++(data->uavtalk_packet_index);
		}
	}

	// Send the received data to the com port
	uint8_t send_bytes = len - sent_bytes;
	if (PIOS_COM_SendBuffer(outputPort, buf + sent_bytes, send_bytes) != send_bytes)
		// Error on transmit
		data->com_tx_errors++;
}

/**
 * Transmit the GCSReceiver object using UAVTalk
 * \param[in] channels The ppm channels
 */
static void SendGCSReceiver(void)
{
	// Send update (with retries)
	uint32_t retries = 0;
	int32_t success = -1;
	while (retries < MAX_RETRIES && success == -1) {
		success = UAVTalkSendObject(data->uavTalkCon, GCSReceiverHandle(), 0, 0, REQ_TIMEOUT_MS);
		++retries;
	}
	if(success >= 0)
		data->send_gcsreceiver = false;
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
	{
		UAVObjMetadata metadata;
		UAVObjGetMetadata(GCSReceiverHandle(), &metadata);
		metadata.access = ACCESS_READWRITE;
		UAVObjSetMetadata(GCSReceiverHandle(), &metadata);
	}
	GCSReceiverSet(&rcvr);
	data->send_gcsreceiver = true;
}

static void updateSettings()
{
	if (data->com_port) {

#ifdef NEVER
		// Retrieve settings
		uint8_t speed;
		HwSettingsRadioComBridgeSpeedGet(&speed);

		// Set port speed
		switch (speed) {
		case HWSETTINGS_RADIOCOMBRIDGESPEED_2400:
			PIOS_COM_ChangeBaud(data->com_port, 2400);
			break;
		case HWSETTINGS_RADIOCOMBRIDGESPEED_4800:
			PIOS_COM_ChangeBaud(data->com_port, 4800);
			break;
		case HWSETTINGS_RADIOCOMBRIDGESPEED_9600:
			PIOS_COM_ChangeBaud(data->com_port, 9600);
			break;
		case HWSETTINGS_RADIOCOMBRIDGESPEED_19200:
			PIOS_COM_ChangeBaud(data->com_port, 19200);
			break;
		case HWSETTINGS_RADIOCOMBRIDGESPEED_38400:
			PIOS_COM_ChangeBaud(data->com_port, 38400);
			break;
		case HWSETTINGS_RADIOCOMBRIDGESPEED_57600:
			PIOS_COM_ChangeBaud(data->com_port, 57600);
			break;
		case HWSETTINGS_RADIOCOMBRIDGESPEED_115200:
			PIOS_COM_ChangeBaud(data->com_port, 115200);
			break;
		}
#endif
	}
}
