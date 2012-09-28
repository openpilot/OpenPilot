/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_USBHOOK USB glue code
 * @brief Glue between PiOS and STM32 libs
 * @{
 *
 * @file       pios_usbhook.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Glue between PiOS and STM32 libs
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

#include "pios.h"
#include "pios_usb.h"		/* PIOS_USB_* */
#include "pios_usbhook.h"
#include "pios_usb_defs.h"	/* struct usb_* */
#include "pios_usb_hid_pwr.h"
#include "pios_usb_cdc_priv.h"	/* PIOS_USB_CDC_* */
#include "pios_usb_board_data.h" /* PIOS_USB_BOARD_* */

/* STM32 USB Library Definitions */
#include "usb_lib.h"

static ONE_DESCRIPTOR Device_Descriptor;

void PIOS_USBHOOK_RegisterDevice(const uint8_t * desc, uint16_t desc_size)
{
	Device_Descriptor.Descriptor      = desc;
	Device_Descriptor.Descriptor_Size = desc_size;
}

static ONE_DESCRIPTOR Config_Descriptor;

void PIOS_USBHOOK_RegisterConfig(uint8_t config_id, const uint8_t * desc, uint16_t desc_size)
{
	Config_Descriptor.Descriptor      = desc;
	Config_Descriptor.Descriptor_Size = desc_size;
}

static ONE_DESCRIPTOR String_Descriptor[4];

void PIOS_USBHOOK_RegisterString(enum usb_string_desc string_id, const uint8_t * desc, uint16_t desc_size)
{
	if (string_id < NELEMENTS(String_Descriptor)) {
		String_Descriptor[string_id].Descriptor      = desc;
		String_Descriptor[string_id].Descriptor_Size = desc_size;
	}
}

static ONE_DESCRIPTOR Hid_Descriptor;

void PIOS_USB_HID_RegisterHidDescriptor(const uint8_t * desc, uint16_t desc_size)
{
	Hid_Descriptor.Descriptor      = desc;
	Hid_Descriptor.Descriptor_Size = desc_size;
}

static ONE_DESCRIPTOR Hid_Report_Descriptor;

void PIOS_USB_HID_RegisterHidReport(const uint8_t * desc, uint16_t desc_size)
{
	Hid_Report_Descriptor.Descriptor      = desc;
	Hid_Report_Descriptor.Descriptor_Size = desc_size;
}

#include "stm32f10x.h"		/* __IO */
__IO uint8_t EXTI_Enable;

uint32_t ProtocolValue;

DEVICE Device_Table = {
	PIOS_USB_BOARD_EP_NUM,
	1
};

static void PIOS_USBHOOK_Init(void);
static void PIOS_USBHOOK_Reset(void);
static void PIOS_USBHOOK_Status_In(void);
static void PIOS_USBHOOK_Status_Out(void);
static RESULT PIOS_USBHOOK_Data_Setup(uint8_t RequestNo);
static RESULT PIOS_USBHOOK_NoData_Setup(uint8_t RequestNo);
static RESULT PIOS_USBHOOK_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
static const uint8_t *PIOS_USBHOOK_GetDeviceDescriptor(uint16_t Length);
static const uint8_t *PIOS_USBHOOK_GetConfigDescriptor(uint16_t Length);
static const uint8_t *PIOS_USBHOOK_GetStringDescriptor(uint16_t Length);

DEVICE_PROP Device_Property = {
	.Init                        = PIOS_USBHOOK_Init,
	.Reset                       = PIOS_USBHOOK_Reset,
	.Process_Status_IN           = PIOS_USBHOOK_Status_In,
	.Process_Status_OUT          = PIOS_USBHOOK_Status_Out,
	.Class_Data_Setup            = PIOS_USBHOOK_Data_Setup,
	.Class_NoData_Setup          = PIOS_USBHOOK_NoData_Setup,
	.Class_Get_Interface_Setting = PIOS_USBHOOK_Get_Interface_Setting,
	.GetDeviceDescriptor         = PIOS_USBHOOK_GetDeviceDescriptor,
	.GetConfigDescriptor         = PIOS_USBHOOK_GetConfigDescriptor,
	.GetStringDescriptor         = PIOS_USBHOOK_GetStringDescriptor,
	.RxEP_buffer                 = 0,
	.MaxPacketSize               = 0x40,
};

static void PIOS_USBHOOK_SetConfiguration(void);
static void PIOS_USBHOOK_SetDeviceAddress(void);

USER_STANDARD_REQUESTS User_Standard_Requests = {
	.User_GetConfiguration   = NOP_Process,
	.User_SetConfiguration   = PIOS_USBHOOK_SetConfiguration,
	.User_GetInterface       = NOP_Process,
	.User_SetInterface       = NOP_Process,
	.User_GetStatus          = NOP_Process,
	.User_ClearFeature       = NOP_Process,
	.User_SetEndPointFeature = NOP_Process,
	.User_SetDeviceFeature   = NOP_Process,
	.User_SetDeviceAddress   = PIOS_USBHOOK_SetDeviceAddress
};

static RESULT PIOS_USBHOOK_SetProtocol(void);
static const uint8_t *PIOS_USBHOOK_GetProtocolValue(uint16_t Length);
static const uint8_t *PIOS_USBHOOK_GetReportDescriptor(uint16_t Length);
static const uint8_t *PIOS_USBHOOK_GetHIDDescriptor(uint16_t Length);

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_Init.
* Description    : Custom HID init routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
static void PIOS_USBHOOK_Init(void)
{
	pInformation->Current_Configuration = 0;

	/* Connect the device */
	PowerOn();

	/* Perform basic device initialization operations */
	USB_SIL_Init();

	bDeviceState = UNCONNECTED;
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_Reset.
* Description    : Custom HID reset routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
static void PIOS_USBHOOK_Reset(void)
{
	/* Set DEVICE as not configured */
	pInformation->Current_Configuration = 0;
	pInformation->Current_Interface = 0;	/*the default Interface */

	/* Current Feature initialization */
	pInformation->Current_Feature = 0;

#ifdef STM32F10X_CL
	/* EP0 is already configured in DFU_Init() by USB_SIL_Init() function */

	/* Init EP1 IN as Interrupt endpoint */
	OTG_DEV_EP_Init(EP1_IN, OTG_DEV_EP_TYPE_INT, 2);

	/* Init EP1 OUT as Interrupt endpoint */
	OTG_DEV_EP_Init(EP1_OUT, OTG_DEV_EP_TYPE_INT, 2);
#else
	SetBTABLE(BTABLE_ADDRESS);

	/* Initialize Endpoint 0 (Control) */
	SetEPType(ENDP0, EP_CONTROL);
	SetEPTxAddr(ENDP0, ENDP0_TXADDR);
	SetEPTxStatus(ENDP0, EP_TX_STALL);
	Clear_Status_Out(ENDP0);

	SetEPRxAddr(ENDP0, ENDP0_RXADDR);
	SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
	SetEPRxValid(ENDP0);

#if defined(PIOS_INCLUDE_USB_HID)
	/* Initialize Endpoint 1 (HID) */
	SetEPType(ENDP1, EP_INTERRUPT);
	SetEPTxAddr(ENDP1, ENDP1_TXADDR);
	SetEPTxCount(ENDP1, PIOS_USB_BOARD_HID_DATA_LENGTH);
	SetEPTxStatus(ENDP1, EP_TX_NAK);

	SetEPRxAddr(ENDP1, ENDP1_RXADDR);
	SetEPRxCount(ENDP1, PIOS_USB_BOARD_HID_DATA_LENGTH);
	SetEPRxStatus(ENDP1, EP_RX_VALID);
#endif	/* PIOS_INCLUDE_USB_HID */

#if defined(PIOS_INCLUDE_USB_CDC)
	/* Initialize Endpoint 2 (CDC Call Control) */
	SetEPType(ENDP2, EP_INTERRUPT);
	SetEPTxAddr(ENDP2, ENDP2_TXADDR);
	SetEPTxStatus(ENDP2, EP_TX_NAK);

	SetEPRxAddr(ENDP2, ENDP2_RXADDR);
	SetEPRxCount(ENDP2, PIOS_USB_BOARD_CDC_MGMT_LENGTH);
	SetEPRxStatus(ENDP2, EP_RX_DIS);

	/* Initialize Endpoint 3 (CDC Data) */
	SetEPType(ENDP3, EP_BULK);
	SetEPTxAddr(ENDP3, ENDP3_TXADDR);
	SetEPTxStatus(ENDP3, EP_TX_NAK);

	SetEPRxAddr(ENDP3, ENDP3_RXADDR);
	SetEPRxCount(ENDP3, PIOS_USB_BOARD_CDC_DATA_LENGTH);
	SetEPRxStatus(ENDP3, EP_RX_VALID);

#endif	/* PIOS_INCLUDE_USB_CDC */

	/* Set this device to response on default address */
	SetDeviceAddress(0);
#endif /* STM32F10X_CL */

	bDeviceState = ATTACHED;
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_SetConfiguration.
* Description    : Update the device state to configured
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
static void PIOS_USBHOOK_SetConfiguration(void)
{
	if (pInformation->Current_Configuration != 0) {
		/* Device configured */
		bDeviceState = CONFIGURED;
	}

	/* Enable transfers */
	PIOS_USB_ChangeConnectionState(pInformation->Current_Configuration != 0);
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_SetConfiguration.
* Description    : Update the device state to addressed.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
static void PIOS_USBHOOK_SetDeviceAddress(void)
{
	bDeviceState = ADDRESSED;
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_Status_In.
* Description    : status IN routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
static void PIOS_USBHOOK_Status_In(void)
{
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_Status_Out
* Description    : status OUT routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
static void PIOS_USBHOOK_Status_Out(void)
{
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_Data_Setup
* Description    : Handle the data class specific requests.
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
extern uint8_t *PIOS_USB_CDC_SetLineCoding(uint16_t Length);
extern const uint8_t *PIOS_USB_CDC_GetLineCoding(uint16_t Length);

static RESULT PIOS_USBHOOK_Data_Setup(uint8_t RequestNo)
{
	uint8_t *(*CopyOutRoutine) (uint16_t);
	const uint8_t *(*CopyInRoutine) (uint16_t);

	CopyInRoutine = NULL;
	CopyOutRoutine = NULL;

	switch (Type_Recipient) {
	case (STANDARD_REQUEST | INTERFACE_RECIPIENT):
		switch (pInformation->USBwIndex0) {
#if defined(PIOS_INCLUDE_USB_CDC)
		case 2:		/* HID Interface */
#else
		case 0:		/* HID Interface */
#endif
			switch (RequestNo) {
			case GET_DESCRIPTOR:
				switch (pInformation->USBwValue1) {
				case USB_DESC_TYPE_REPORT:
					CopyInRoutine = PIOS_USBHOOK_GetReportDescriptor;
					break;
				case USB_DESC_TYPE_HID:
					CopyInRoutine = PIOS_USBHOOK_GetHIDDescriptor;
					break;
				}
			}
		}
		break;

	case (CLASS_REQUEST | INTERFACE_RECIPIENT):
		switch (pInformation->USBwIndex0) {
#if defined(PIOS_INCLUDE_USB_CDC)
		case 2:		/* HID Interface */
#else
		case 0:		/* HID Interface */
#endif
			switch (RequestNo) {
			case USB_HID_REQ_GET_PROTOCOL:
				CopyInRoutine = PIOS_USBHOOK_GetProtocolValue;
				break;
			}

			break;
#if defined(PIOS_INCLUDE_USB_CDC)
		case 0:		/* CDC Call Control Interface */
			switch (RequestNo) {
			case USB_CDC_REQ_SET_LINE_CODING:
				CopyOutRoutine = PIOS_USB_CDC_SetLineCoding;
				break;
			case USB_CDC_REQ_GET_LINE_CODING:
				CopyInRoutine = PIOS_USB_CDC_GetLineCoding;
				break;
			}

			break;

		case 1:		/* CDC Data Interface */
			switch (RequestNo) {
			case 0:
				break;
			}

			break;
#endif	/* PIOS_INCLUDE_USB_CDC */
		}
		break;
	}

	/* No registered copy routine */
	if ((CopyInRoutine == NULL) && (CopyOutRoutine == NULL)) {
		return USB_UNSUPPORT;
	}

	/* Registered copy in AND copy out routine */
	if ((CopyInRoutine != NULL) && (CopyOutRoutine != NULL)) {
		/* This should never happen */
		return USB_UNSUPPORT;
	}

	if (CopyInRoutine != NULL) {
		pInformation->Ctrl_Info.CopyDataIn = CopyInRoutine;
		pInformation->Ctrl_Info.Usb_wOffset = 0;
		(*CopyInRoutine) (0);
	} else if (CopyOutRoutine != NULL) {
		pInformation->Ctrl_Info.CopyDataOut = CopyOutRoutine;
		pInformation->Ctrl_Info.Usb_rOffset = 0;
		(*CopyOutRoutine) (0);
	}

	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_NoData_Setup
* Description    : handle the no data class specific requests
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
extern RESULT PIOS_USB_CDC_SetControlLineState(void);

static RESULT PIOS_USBHOOK_NoData_Setup(uint8_t RequestNo)
{
	switch (Type_Recipient) {
	case (CLASS_REQUEST | INTERFACE_RECIPIENT):
		switch (pInformation->USBwIndex0) {
#if defined(PIOS_INCLUDE_USB_CDC)
		case 2:		/* HID */
#else
		case 0:		/* HID */
#endif
			switch (RequestNo) {
			case USB_HID_REQ_SET_PROTOCOL:
				return PIOS_USBHOOK_SetProtocol();
				break;
			}

			break;

#if defined(PIOS_INCLUDE_USB_CDC)
		case 0:		/* CDC Call Control Interface */
			switch (RequestNo) {
			case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
				return PIOS_USB_CDC_SetControlLineState();
				break;
			}

			break;
#endif	/* PIOS_INCLUDE_USB_CDC */
		}

		break;
	}

	return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_GetDeviceDescriptor.
* Description    : Gets the device descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the device descriptor.
*******************************************************************************/
static const uint8_t *PIOS_USBHOOK_GetDeviceDescriptor(uint16_t Length)
{
	return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_GetConfigDescriptor.
* Description    : Gets the configuration descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
static const uint8_t *PIOS_USBHOOK_GetConfigDescriptor(uint16_t Length)
{
	return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_GetStringDescriptor
* Description    : Gets the string descriptors according to the needed index
* Input          : Length
* Output         : None.
* Return         : The address of the string descriptors.
*******************************************************************************/
static const uint8_t *PIOS_USBHOOK_GetStringDescriptor(uint16_t Length)
{
	uint8_t wValue0 = pInformation->USBwValue0;
	if (wValue0 > 4) {
		return NULL;
	} else {
		return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
	}
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_GetReportDescriptor.
* Description    : Gets the HID report descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
static const uint8_t *PIOS_USBHOOK_GetReportDescriptor(uint16_t Length)
{
	return Standard_GetDescriptorData(Length, &Hid_Report_Descriptor);
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_GetHIDDescriptor.
* Description    : Gets the HID descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
static const uint8_t *PIOS_USBHOOK_GetHIDDescriptor(uint16_t Length)
{
	return Standard_GetDescriptorData(Length, &Hid_Descriptor);
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_Get_Interface_Setting.
* Description    : tests the interface and the alternate setting according to the
*                  supported one.
* Input          : - Interface : interface number.
*                  - AlternateSetting : Alternate Setting number.
* Output         : None.
* Return         : USB_SUCCESS or USB_UNSUPPORT.
*******************************************************************************/
static RESULT PIOS_USBHOOK_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
	if (AlternateSetting > 0) {
		return USB_UNSUPPORT;
	} else if (Interface > 0) {
		return USB_UNSUPPORT;
	}
	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_SetProtocol
* Description    : Set Protocol request routine.
* Input          : None.
* Output         : None.
* Return         : USB SUCCESS.
*******************************************************************************/
static RESULT PIOS_USBHOOK_SetProtocol(void)
{
	uint8_t wValue0 = pInformation->USBwValue0;
	ProtocolValue = wValue0;
	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_GetProtocolValue
* Description    : get the protocol value
* Input          : Length.
* Output         : None.
* Return         : address of the protcol value.
*******************************************************************************/
static const uint8_t *PIOS_USBHOOK_GetProtocolValue(uint16_t Length)
{
	if (Length == 0) {
		pInformation->Ctrl_Info.Usb_wLength = 1;
		return NULL;
	} else {
		return (uint8_t *) (&ProtocolValue);
	}
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
