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

#include "openpilot.h"
#include "hwsettings.h"
#include "radiocombridge.h"

#include <stdbool.h>

// ****************
// Private functions

static void radio2ComBridgeTask(void *parameters);
static void com2RadioBridgeTask(void *parameters);
static void updateSettings();

// ****************
// Private constants

#define STACK_SIZE_BYTES 280
#define TASK_PRIORITY (tskIDLE_PRIORITY + 1)

#define BRIDGE_BUF_LEN 10

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
} RadioComBridgeData;

// ****************
// Private variables

static RadioComBridgeData *data;

/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */

static int32_t RadioComBridgeStart(void)
{
	if(data) {
		// Start the tasks
		xTaskCreate(radio2ComBridgeTask, (signed char *)"Radio2ComBridge", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &(data->radio2ComBridgeTaskHandle));
		xTaskCreate(com2RadioBridgeTask, (signed char *)"Com2RadioBridge", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &(data->com2RadioBridgeTaskHandle));
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
	//data->com_port = PIOS_COM_VCP;
	data->com_port = PIOS_COM_TELEM_USB;
	data->radio_port = PIOS_COM_TELEM_SERIAL;

	data->radio2com_buf = pvPortMalloc(BRIDGE_BUF_LEN);
	PIOS_Assert(data->radio2com_buf);
	data->com2radio_buf = pvPortMalloc(BRIDGE_BUF_LEN);
	PIOS_Assert(data->com2radio_buf);

	updateSettings();

	return 0;
}
MODULE_INITCALL(RadioComBridgeInitialize, RadioComBridgeStart)

/**
 * Main task. It does not return.
 */

static void radio2ComBridgeTask(void *parameters)
{
	/* Handle radio -> usart/usb direction */
	volatile uint32_t tx_errors = 0;
	while (1) {
		uint32_t rx_bytes;

		rx_bytes = PIOS_COM_ReceiveBuffer(data->radio_port, data->radio2com_buf, BRIDGE_BUF_LEN, 500);
		if (rx_bytes > 0) {
			/* Bytes available to transfer */
			if (PIOS_COM_SendBuffer(data->com_port, data->radio2com_buf, rx_bytes) != rx_bytes) {
				/* Error on transmit */
				tx_errors++;
			}
		}
	}
}

static void com2RadioBridgeTask(void * parameters)
{
	/* Handle usart/usb -> radio direction */
	volatile uint32_t tx_errors = 0;
	while (1) {
		uint32_t rx_bytes;

		rx_bytes = PIOS_COM_ReceiveBuffer(data->com_port, data->com2radio_buf, BRIDGE_BUF_LEN, 500);
		if (rx_bytes > 0) {
			PIOS_COM_SendString(PIOS_COM_TELEM_SERIAL, "Rec com\n\r");
			/* Bytes available to transfer */
			if (PIOS_COM_SendBuffer(data->radio_port, data->com2radio_buf, rx_bytes) != rx_bytes) {
				/* Error on transmit */
				tx_errors++;
			}
		}
	}
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
