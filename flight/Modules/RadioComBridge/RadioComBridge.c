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

#include <stdbool.h>

#include "ecc.h"

#undef PIOS_INCLUDE_USB

extern char *debug_msg;

// ****************
// Private functions

static void radio2ComBridgeTask(void *parameters);
static void com2RadioBridgeTask(void *parameters);
static int32_t transmitData(uint8_t * data, int32_t length);
static int32_t transmitPacket(PHPacketHandle packet);
static void receiveData(uint8_t *buf, uint8_t len);
static void updateSettings();

// ****************
// Private constants

#define STACK_SIZE_BYTES 300
#define TASK_PRIORITY (tskIDLE_PRIORITY + 1)

#define BRIDGE_BUF_LEN 128

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

	PHPacket packet;

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

	// Initialize the packet send timeout
	data->send_timeout = 25; // ms
	data->min_packet_size = 50;

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
		for (uint8_t i = 0; i < cur_rx_bytes; i++) {
			UAVTalkProcessInputStream(data->uavTalkCon, *(data->com2radio_buf + i + rx_bytes));
			/*
			if(UAVTalkIdle(data->uavTalkCon))
				DEBUG_PRINTF(1, "Idle\n\r");
			*/
		}

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
	DEBUG_PRINTF(1, "Transmitting UAVTalk data\n\r");
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
	/* Send the received data to the com port */
	if (PIOS_COM_SendBuffer(outputPort, buf, len) != len)
		/* Error on transmit */
		data->com_tx_errors++;
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
