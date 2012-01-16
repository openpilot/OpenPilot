/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup ComUsbBridgeModule Com Port to USB VCP Bridge Module
 * @brief Bridge Com and USB VCP ports
 * @{ 
 *
 * @file       ComUsbBridge.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Bridges selected Com Port to the USB VCP emulated serial port
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

#include <stdbool.h>

// ****************
// Private functions

static void com2UsbBridgeTask(void *parameters);
static void usb2ComBridgeTask(void *parameters);
static void updateSettings();

// ****************
// Private constants

#define STACK_SIZE_BYTES            280
#define TASK_PRIORITY                   (tskIDLE_PRIORITY + 1)

#define BRIDGE_BUF_LEN 10

// ****************
// Private variables

static xTaskHandle com2UsbBridgeTaskHandle;
static xTaskHandle usb2ComBridgeTaskHandle;

static uint8_t * com2usb_buf;
static uint8_t * usb2com_buf;

static uint32_t usart_port;
static uint32_t vcp_port;

static bool bridge_enabled = false;

/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */

static int32_t comUsbBridgeStart(void)
{
	if (bridge_enabled) {
		// Start tasks
		xTaskCreate(com2UsbBridgeTask, (signed char *)"Com2UsbBridge", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &com2UsbBridgeTaskHandle);
		TaskMonitorAdd(TASKINFO_RUNNING_COM2USBBRIDGE, com2UsbBridgeTaskHandle);
		xTaskCreate(usb2ComBridgeTask, (signed char *)"Usb2ComBridge", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &usb2ComBridgeTaskHandle);
		TaskMonitorAdd(TASKINFO_RUNNING_USB2COMBRIDGE, usb2ComBridgeTaskHandle);
		return 0;
	}

	return -1;
}
/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
static int32_t comUsbBridgeInitialize(void)
{
	// TODO: Get from settings object
	usart_port = PIOS_COM_BRIDGE;
	vcp_port = PIOS_COM_VCP;

#ifdef MODULE_ComUsbBridge_BUILTIN
	bridge_enabled = true;
#else
	HwSettingsInitialize();
	uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];

	HwSettingsOptionalModulesGet(optionalModules);

	if (usart_port && vcp_port &&
		(optionalModules[HWSETTINGS_OPTIONALMODULES_COMUSBBRIDGE] == HWSETTINGS_OPTIONALMODULES_ENABLED))
		bridge_enabled = true;
	else
		bridge_enabled = false;
#endif

	if (bridge_enabled) {
		com2usb_buf = pvPortMalloc(BRIDGE_BUF_LEN);
		PIOS_Assert(com2usb_buf);
		usb2com_buf = pvPortMalloc(BRIDGE_BUF_LEN);
		PIOS_Assert(usb2com_buf);

		updateSettings();
	}

	return 0;
}
MODULE_INITCALL(comUsbBridgeInitialize, comUsbBridgeStart)

/**
 * Main task. It does not return.
 */

static void com2UsbBridgeTask(void *parameters)
{
	/* Handle usart -> vcp direction */
	volatile uint32_t tx_errors = 0;
	while (1) {
		uint32_t rx_bytes;

		rx_bytes = PIOS_COM_ReceiveBuffer(usart_port, com2usb_buf, BRIDGE_BUF_LEN, 500);
		if (rx_bytes > 0) {
			/* Bytes available to transfer */
			if (PIOS_COM_SendBuffer(vcp_port, com2usb_buf, rx_bytes) != rx_bytes) {
				/* Error on transmit */
				tx_errors++;
			}
		}
	}
}

static void usb2ComBridgeTask(void * parameters)
{
	/* Handle vcp -> usart direction */
	volatile uint32_t tx_errors = 0;
	while (1) {
		uint32_t rx_bytes;

		rx_bytes = PIOS_COM_ReceiveBuffer(vcp_port, usb2com_buf, BRIDGE_BUF_LEN, 500);
		if (rx_bytes > 0) {
			/* Bytes available to transfer */
			if (PIOS_COM_SendBuffer(usart_port, usb2com_buf, rx_bytes) != rx_bytes) {
				/* Error on transmit */
				tx_errors++;
			}
		}
	}
}


static void updateSettings()
{
	if (usart_port) {

		// Retrieve settings
		uint8_t speed;
		HwSettingsComUsbBridgeSpeedGet(&speed);

		// Set port speed
		switch (speed) {
		case HWSETTINGS_COMUSBBRIDGESPEED_2400:
			PIOS_COM_ChangeBaud(usart_port, 2400);
			break;
		case HWSETTINGS_COMUSBBRIDGESPEED_4800:
			PIOS_COM_ChangeBaud(usart_port, 4800);
			break;
		case HWSETTINGS_COMUSBBRIDGESPEED_9600:
			PIOS_COM_ChangeBaud(usart_port, 9600);
			break;
		case HWSETTINGS_COMUSBBRIDGESPEED_19200:
			PIOS_COM_ChangeBaud(usart_port, 19200);
			break;
		case HWSETTINGS_COMUSBBRIDGESPEED_38400:
			PIOS_COM_ChangeBaud(usart_port, 38400);
			break;
		case HWSETTINGS_COMUSBBRIDGESPEED_57600:
			PIOS_COM_ChangeBaud(usart_port, 57600);
			break;
		case HWSETTINGS_COMUSBBRIDGESPEED_115200:
			PIOS_COM_ChangeBaud(usart_port, 115200);
			break;
		}
	}
}
