/**
 ******************************************************************************
 *
 * @file       pios_usb_hid.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 		Parts by Thorsten Klose (tk@midibox.org)
 * @brief      USB HID functions
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_USB_HID USB HID Functions
 * @notes      This implements a very simple HID device with a simple data in
 * and data out endpoints.
 * @{
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_USB_HID)


/* Local types */
typedef enum _HID_REQUESTS {
	GET_REPORT = 1,
	GET_IDLE,
	GET_PROTOCOL,
	SET_REPORT = 9,
	SET_IDLE,
	SET_PROTOCOL
} HID_REQUESTS;

/* Global Variables */
xSemaphoreHandle PIOS_HID_Buffer;
static portBASE_TYPE xHigherPriorityTaskWoken;

/* Local Variables */
static uint32_t ProtocolValue;


/* Local Functions */
static uint8_t *PIOS_USB_HID_GetHIDDescriptor(uint16_t Length);
static uint8_t *PIOS_USB_HID_GetReportDescriptor(uint16_t Length);
static uint8_t *PIOS_USB_HID_GetProtocolValue(uint16_t Length);

static const uint8_t PIOS_USB_HID_ReportDescriptor[PIOS_USB_HID_SIZ_REPORT_DESC] = {
		0x06, 0x9c, 0xff,			/* Usage Page (Vendor Defined)                     */
		0x09, 0x01,				/* Usage (Vendor Defined)                          */
		0xa1, 0x01,				/* Collection (Vendor Defined)                     */

		0x09, 0x02,				/*   Usage (Vendor Defined)                        */
		0x75, 0x08,				/*   Report Size (8)                               */
		0x95, (PIOS_USB_HID_DATA_LENGTH),	/*   Report Count (64)                             */
		0x15, 0x00,				/*   Logical Minimum (0)                           */
		0x25, 0xff,				/*   Logical Maximum (255)                         */
		0x81, 0x02,				/*   Input (Data, Variable, Absolute)              */

		0x09, 0x03,				/*   Usage (Vendor Defined)                        */
		0x75, 0x08,				/*   Report Size (8)                               */
		0x95, (PIOS_USB_HID_DATA_LENGTH),	/*   Report Count (64)                             */
		0x15, 0x00,				/*   Logical Minimum (0)                           */
		0x25, 0xff,				/*   Logical Maximum (255)                         */
		0x91, 0x02,				/*   Output (Data, Variable, Absolute)             */

		0xc0					/* End Collection                                  */
		};
static ONE_DESCRIPTOR PIOS_USB_HID_Report_Descriptor = {(uint8_t *) PIOS_USB_HID_ReportDescriptor, PIOS_USB_HID_SIZ_REPORT_DESC};
static ONE_DESCRIPTOR PIOS_USB_HID_Hid_Descriptor = {(uint8_t*) PIOS_USB_HID_ReportDescriptor + PIOS_USB_HID_OFF_HID_DESC, PIOS_USB_HID_SIZ_HID_DESC};

/* Rx/Tx status */
static volatile uint8_t rx_buffer_new_data_ctr = 0;
static volatile uint8_t rx_buffer_ix;
static uint8_t transfer_possible = 0;

static uint8_t rx_buffer[PIOS_USB_HID_DATA_LENGTH] = {0};

/**
* Initialises USB COM layer
* \param[in] mode currently only mode 0 supported
* \return < 0 if initialisation failed
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_Init(uint32_t mode)
{
	/* Currently only mode 0 supported */
	if(mode != 0) {
		/* Unsupported mode */
		return -1;
	}

	vSemaphoreCreateBinary(PIOS_HID_Buffer);

	return 0; /* No error */
}

/**
* This function is called by the USB driver on cable connection/disconnection
* \param[in] connected connection status (1 if connected)
* \return < 0 on errors
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_ChangeConnectionState(uint32_t Connected)
{
	/* In all cases: re-initialise USB HID driver */
	if(Connected) {
		transfer_possible = 1;
		//TODO: Check SetEPRxValid(ENDP1);
	} else {
		/* Cable disconnected: disable transfers */
		transfer_possible = 0;
	}
	return 0;
}

/**
* This function returns the connection status of the USB HID interface
* \return 1: interface available
* \return 0: interface not available
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_CheckAvailable(void)
{
  return transfer_possible ? 1 : 0;
}

/**
* Puts more than one byte onto the transmit buffer (used for atomic sends)
* \param[in] *buffer pointer to buffer which should be transmitted
* \param[in] len number of bytes which should be transmitted
* \return 0 if no error
* \return -1 if too many bytes to be send
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_TxBufferPutMoreNonBlocking(uint8_t *buffer, uint16_t len)
{
	if(len > PIOS_USB_HID_DATA_LENGTH) {
		/* Cannot get all requested bytes */
		return -1;
	}

	/* Copy bytes to be transmitted into transmit buffer */
	UserToPMABufferCopy((uint8_t*) buffer, GetEPTxAddr(EP1_IN & 0x7F), (PIOS_USB_HID_DATA_LENGTH + 1));
	SetEPTxCount(ENDP1, (PIOS_USB_HID_DATA_LENGTH + 1));

	/* Send Buffer */
	SetEPTxValid(ENDP1);

	/* No error */
	return 0;
}

/**
* Puts more than one byte onto the transmit buffer (used for atomic sends)<br>
* (Blocking Function)
* \param[in] *buffer pointer to buffer which should be transmitted
* \param[in] len number of bytes which should be transmitted
* \return 0 if no error
* \return -1 if too many bytes to be send
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_TxBufferPutMore(uint8_t *buffer, uint16_t len)
{
  int32_t error;

  while((error = PIOS_USB_HID_TxBufferPutMoreNonBlocking(buffer, len)) == -2);

  return error;
}

/**
* Gets a byte from the receive buffer
* \return -1 if no new byte available
* \return >= 0: received byte
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_RxBufferGet(void)
{
	if(!rx_buffer_new_data_ctr) {
		/* Nothing new in buffer */
		return -1;
	}

	/* This stops returning bytes after the first occurrence of '\0' */
	/* We don't need to do this but it does optimise things quite a bit */
	//if(rx_buffer[rx_buffer_ix] == 0) {
		/* TODO: Evaluate if this is really needed */
		/* Clean the buffer */
		/*for(uint8_t i = 0; i < PIOS_USB_HID_DATA_LENGTH; i++) {
			rx_buffer[i] = 0;
		}

		rx_buffer_new_data_ctr = 0;
		rx_buffer_ix = 0;
		SetEPRxStatus(ENDP1, EP_RX_VALID);
		return -1;
	}*/

	/* There is still data in the buffer */
	uint8_t b = rx_buffer[rx_buffer_ix++];
	if(!--rx_buffer_new_data_ctr) {
		rx_buffer_ix = 0;
		SetEPRxStatus(ENDP1, EP_RX_VALID);
	}

	/* Return received byte */
	return b;
}

int32_t PIOS_USB_HID_CB_Data_Setup(uint8_t RequestNo)
{
	uint8_t *(*CopyRoutine)( uint16_t) = NULL;

	CopyRoutine = NULL;

	/* GET_DESCRIPTOR */
	if((RequestNo == GET_DESCRIPTOR) && (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT)) && (pInformation->USBwIndex0 == 0)) {

		if(pInformation->USBwValue1 == PIOS_USB_HID_REPORT_DESCRIPTOR) {
			CopyRoutine = PIOS_USB_HID_GetReportDescriptor;
		} else if(pInformation->USBwValue1 == PIOS_USB_HID_HID_DESCRIPTOR_TYPE) {
			CopyRoutine = PIOS_USB_HID_GetHIDDescriptor;
		}

	}

	/* GET_PROTOCOL */
	else if((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) && RequestNo == GET_PROTOCOL) {
		CopyRoutine = PIOS_USB_HID_GetProtocolValue;
	}

	if(CopyRoutine == NULL) {
		return USB_UNSUPPORT;
	}

	pInformation->Ctrl_Info.CopyData = CopyRoutine;
	pInformation->Ctrl_Info.Usb_wOffset = 0;
	(*CopyRoutine)(0);

	return USB_SUCCESS;
}

int32_t PIOS_USB_HID_CB_NoData_Setup(uint8_t RequestNo)
{
	if((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) && (RequestNo == SET_PROTOCOL)) {
		uint8_t wValue0 = pInformation->USBwValue0;
		ProtocolValue = wValue0;
		return USB_SUCCESS;
	}

	else {
		return USB_UNSUPPORT;
	}
}

/**
* Gets the HID descriptor.
* \param[in] Length
* \return The address of the configuration descriptor.
*/
static uint8_t *PIOS_USB_HID_GetHIDDescriptor(uint16_t Length)
{
	return Standard_GetDescriptorData(Length, &PIOS_USB_HID_Hid_Descriptor);
}

/**
* Gets the HID report descriptor.
* \param[in] Length
* \return The address of the configuration descriptor.
*/
static uint8_t *PIOS_USB_HID_GetReportDescriptor(uint16_t Length)
{
	return Standard_GetDescriptorData(Length, &PIOS_USB_HID_Report_Descriptor);
}

/**
* Gets the protocol value
* \param[in] Length
* \return address of the protocol value.
*/
static uint8_t *PIOS_USB_HID_GetProtocolValue(uint16_t Length)
{
	if(Length == 0) {
		pInformation->Ctrl_Info.Usb_wLength = 1;
		return NULL;
	} else {
		return (uint8_t *) (&ProtocolValue);
	}
}

/**
* EP1 OUT Callback Routine
*/
void PIOS_USB_HID_EP1_OUT_Callback(void)
{
#if 1
	uint32_t DataLength = 0;

	/* Get the number of received data on the selected Endpoint */
	DataLength = GetEPRxCount(ENDP1 & 0x7F);

	/* Use the memory interface function to write to the selected endpoint */
	PMAToUserBufferCopy((uint8_t *) rx_buffer, GetEPRxAddr(ENDP1 & 0x7F), DataLength);

	/* We now have data waiting */
	rx_buffer_new_data_ctr = PIOS_USB_HID_DATA_LENGTH;
	xSemaphoreGiveFromISR(PIOS_HID_Buffer, &xHigherPriorityTaskWoken);

#else
	// FOR DEBUGGING USE ONLY

	uint8_t Receive_Buffer[PIOS_USB_HID_DATA_LENGTH];
	//uint32_t DataLength = 0;

	/* Read received data (63 bytes) */
	USB_SIL_Read(EP1_OUT, Receive_Buffer);

	/* Get the number of received data on the selected Endpoint */
	//DataLength = GetEPRxCount(ENDP1 & 0x7F);

	/* Use the memory interface function to write to the selected endpoint */
	//PMAToUserBufferCopy((uint8_t *) Receive_Buffer, GetEPRxAddr(ENDP1 & 0x7F), DataLength);

	/* Send it back */
	PIOS_COM_SendBuffer(GPS, Receive_Buffer, sizeof(Receive_Buffer));
	PIOS_COM_SendBuffer(GPS, "\r", 1);

	SetEPRxStatus(ENDP1, EP_RX_VALID);
#endif
}

#endif
