/**
 ******************************************************************************
 *
 * @file       pios_usb_hid.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 		Parts by Thorsten Klose (tk@midibox.org)
 * @brief      USB HID functions
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_USB_HID USB HID Functions
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

/* Local types */
typedef enum _HID_REQUESTS {
	GET_REPORT = 1,
	GET_IDLE,
	GET_PROTOCOL,
	SET_REPORT = 9,
	SET_IDLE,
	SET_PROTOCOL
} HID_REQUESTS;

/* Local Variables */
uint32_t ProtocolValue;

/* Local Functions */
static uint8_t *PIOS_USB_HID_GetHIDDescriptor(uint16_t Length);
static uint8_t *PIOS_USB_HID_GetReportDescriptor(uint16_t Length);
static uint8_t *PIOS_USB_HID_GetProtocolValue(uint16_t Length);

const uint8_t PIOS_USB_HID_ReportDescriptor[PIOS_USB_HID_SIZ_REPORT_DESC] = {
		0x05, 0x8c, /* USAGE_PAGE (ST Page)           */
		0x09, 0x01, /* USAGE (Demo Kit)               */
		0xa1, 0x01, /* COLLECTION (Application)       */
		/* 6 */

		/* Led 1 */
		0x85, 0x01, /*     REPORT_ID (1)		     */
		0x09, 0x01, /*     USAGE (LED 1)	             */
		0x15, 0x00, /*     LOGICAL_MINIMUM (0)        */
		0x25, 0x01, /*     LOGICAL_MAXIMUM (1)        */
		0x75, 0x08, /*     REPORT_SIZE (8)            */
		0x95, 0x01, /*     REPORT_COUNT (1)           */
		0xB1, 0x82, /*    FEATURE (Data,Var,Abs,Vol) */

		0x85, 0x01, /*     REPORT_ID (1)              */
		0x09, 0x01, /*     USAGE (LED 1)              */
		0x91, 0x82, /*     OUTPUT (Data,Var,Abs,Vol)  */
		/* 26 */

		/* Led 2 */
		0x85, 0x02, /*     REPORT_ID 2		     */
		0x09, 0x02, /*     USAGE (LED 2)	             */
		0x15, 0x00, /*     LOGICAL_MINIMUM (0)        */
		0x25, 0x01, /*     LOGICAL_MAXIMUM (1)        */
		0x75, 0x08, /*     REPORT_SIZE (8)            */
		0x95, 0x01, /*     REPORT_COUNT (1)           */
		0xB1, 0x82, /*    FEATURE (Data,Var,Abs,Vol) */

		0x85, 0x02, /*     REPORT_ID (2)              */
		0x09, 0x02, /*     USAGE (LED 2)              */
		0x91, 0x82, /*     OUTPUT (Data,Var,Abs,Vol)  */
		/* 46 */

		/* Led 3 */
		0x85, 0x03, /*     REPORT_ID (3)		     */
		0x09, 0x03, /*     USAGE (LED 3)	             */
		0x15, 0x00, /*     LOGICAL_MINIMUM (0)        */
		0x25, 0x01, /*     LOGICAL_MAXIMUM (1)        */
		0x75, 0x08, /*     REPORT_SIZE (8)            */
		0x95, 0x01, /*     REPORT_COUNT (1)           */
		0xB1, 0x82, /*    FEATURE (Data,Var,Abs,Vol) */

		0x85, 0x03, /*     REPORT_ID (3)              */
		0x09, 0x03, /*     USAGE (LED 3)              */
		0x91, 0x82, /*     OUTPUT (Data,Var,Abs,Vol)  */
		/* 66 */

		/* Led 4 */
		0x85, 0x04, /*     REPORT_ID 4)		     */
		0x09, 0x04, /*     USAGE (LED 4)	             */
		0x15, 0x00, /*     LOGICAL_MINIMUM (0)        */
		0x25, 0x01, /*     LOGICAL_MAXIMUM (1)        */
		0x75, 0x08, /*     REPORT_SIZE (8)            */
		0x95, 0x01, /*     REPORT_COUNT (1)           */
		0xB1, 0x82, /*     FEATURE (Data,Var,Abs,Vol) */

		0x85, 0x04, /*     REPORT_ID (4)              */
		0x09, 0x04, /*     USAGE (LED 4)              */
		0x91, 0x82, /*     OUTPUT (Data,Var,Abs,Vol)  */
		/* 86 */

		/* key Push Button */
		0x85, 0x05, /*     REPORT_ID (5)              */
		0x09, 0x05, /*     USAGE (Push Button)        */
		0x15, 0x00, /*     LOGICAL_MINIMUM (0)        */
		0x25, 0x01, /*     LOGICAL_MAXIMUM (1)        */
		0x75, 0x01, /*     REPORT_SIZE (1)            */
		0x81, 0x82, /*     INPUT (Data,Var,Abs,Vol)   */

		0x09, 0x05, /*     USAGE (Push Button)        */
		0x75, 0x01, /*     REPORT_SIZE (1)            */
		0xb1, 0x82, /*     FEATURE (Data,Var,Abs,Vol) */

		0x75, 0x07, /*     REPORT_SIZE (7)            */
		0x81, 0x83, /*     INPUT (Cnst,Var,Abs,Vol)   */
		0x85, 0x05, /*     REPORT_ID (2)              */

		0x75, 0x07, /*     REPORT_SIZE (7)            */
		0xb1, 0x83, /*     FEATURE (Cnst,Var,Abs,Vol) */
		/* 114 */

		/* Tamper Push Button */
		0x85, 0x06, /*     REPORT_ID (6)              */
		0x09, 0x06, /*     USAGE (Tamper Push Button) */
		0x15, 0x00, /*     LOGICAL_MINIMUM (0)        */
		0x25, 0x01, /*     LOGICAL_MAXIMUM (1)        */
		0x75, 0x01, /*     REPORT_SIZE (1)            */
		0x81, 0x82, /*     INPUT (Data,Var,Abs,Vol)   */

		0x09, 0x06, /*     USAGE (Tamper Push Button) */
		0x75, 0x01, /*     REPORT_SIZE (1)            */
		0xb1, 0x82, /*     FEATURE (Data,Var,Abs,Vol) */

		0x75, 0x07, /*     REPORT_SIZE (7)            */
		0x81, 0x83, /*     INPUT (Cnst,Var,Abs,Vol)   */
		0x85, 0x06, /*     REPORT_ID (6)              */

		0x75, 0x07, /*     REPORT_SIZE (7)            */
		0xb1, 0x83, /*     FEATURE (Cnst,Var,Abs,Vol) */
		/* 142 */

		/* ADC IN */
		0x85, 0x07, /*     REPORT_ID (7)              */
		0x09, 0x07, /*     USAGE (ADC IN)             */
		0x15, 0x00, /*     LOGICAL_MINIMUM (0)        */
		0x26, 0xff, 0x00, /*     LOGICAL_MAXIMUM (255)      */
		0x75, 0x08, /*     REPORT_SIZE (8)            */
		0x81, 0x82, /*     INPUT (Data,Var,Abs,Vol)   */
		0x85, 0x07, /*     REPORT_ID (7)              */
		0x09, 0x07, /*     USAGE (ADC in)             */
		0xb1, 0x82, /*     FEATURE (Data,Var,Abs,Vol) */
		/* 161 */

		0xc0 /*     END_COLLECTION	             */
		};
ONE_DESCRIPTOR PIOS_USB_HID_Report_Descriptor = {(uint8_t *) PIOS_USB_HID_ReportDescriptor, PIOS_USB_HID_SIZ_REPORT_DESC};
ONE_DESCRIPTOR PIOS_USB_HID_Hid_Descriptor = {(uint8_t*) PIOS_USB_HID_ReportDescriptor + PIOS_USB_HID_OFF_HID_DESC, PIOS_USB_HID_SIZ_HID_DESC};

/**
* This function is called by the USB driver on cable connection/disconnection
* \param[in] connected connection status (1 if connected)
* \return < 0 on errors
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
uint32_t PIOS_USB_HID_ChangeConnectionState(uint32_t Connected)
{
	return 0;
}

int32_t PIOS_USB_HID_CB_Data_Setup(uint8_t RequestNo)
{
	uint8_t *(*CopyRoutine)( uint16_t) = NULL;

	CopyRoutine = NULL;

	if((RequestNo == GET_DESCRIPTOR) && (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT)) && (pInformation->USBwIndex0 == 0)) {

		if(pInformation->USBwValue1 == PIOS_USB_HID_REPORT_DESCRIPTOR) {
			CopyRoutine = PIOS_USB_HID_GetReportDescriptor;
		} else if(pInformation->USBwValue1 == PIOS_USB_HID_HID_DESCRIPTOR_TYPE) {
			CopyRoutine = PIOS_USB_HID_GetHIDDescriptor;
		}

	}
	/* End of GET_DESCRIPTOR */

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

	// TODO:Unsure
	return USB_UNSUPPORT;
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
* \return address of the protcol value.
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
	uint8_t Receive_Buffer[2];
	BitAction Led_State;

	/* Read received data (2 bytes) */
	USB_SIL_Read(0x01, Receive_Buffer);

	if(Receive_Buffer[1] == 0) {
		Led_State = Bit_RESET;
	} else {
		Led_State = Bit_SET;
	}

	switch(Receive_Buffer[0]) {
		case 1: /* Led 1 */
			if(Led_State != Bit_RESET) {
				PIOS_LED_On(LED1);
			} else {
				PIOS_LED_Off(LED1);
			}
			break;
		case 2: /* Led 2 */
			if(Led_State != Bit_RESET) {
				PIOS_LED_On(LED2);
			} else {
				PIOS_LED_Off(LED2);
			}
			break;
		default:

			PIOS_LED_Off(LED1);
			PIOS_LED_Off(LED2);
			break;
	}

	SetEPRxStatus(ENDP1, EP_RX_VALID);
}

