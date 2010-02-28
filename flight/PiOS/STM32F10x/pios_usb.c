/**
 ******************************************************************************
 *
 * @file       pios_usb.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 		Parts by Thorsten Klose (tk@midibox.org)
 * @brief      USB functions
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_USB USB Functions
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

#if !defined(PIOS_DONT_USE_USB)


/* Local definitions */
#define DSCR_DEVICE     1       /* Descriptor type: Device */
#define DSCR_CONFIG     2       /* Descriptor type: Configuration */
#define DSCR_STRING     3       /* Descriptor type: String */
#define DSCR_INTRFC     4       /* Descriptor type: Interface */
#define DSCR_ENDPNT     5       /* Descriptor type: Endpoint */
#define CS_INTERFACE    0x24    /* Class-specific type: Interface */
#define CS_ENDPOINT     0x25    /* Class-specific type: Endpoint */

/* ISTR events */
/* mask defining which events has to be handled by the device application software */
#define IMR_MSK (CNTR_RESETM | CNTR_SOFM | CNTR_CTRM)

/* Local types */
typedef enum _DEVICE_STATE {
	UNCONNECTED,
	ATTACHED,
	POWERED,
	SUSPENDED,
	ADDRESSED,
	CONFIGURED
} DEVICE_STATE;

/* Global Variables used by STM32 USB Driver */
/* (unfortunately no unique names are used...) */
/* Points to the DEVICE_INFO/DEVICE_PROP_USER_STANDARD_REQUESTS structure of current device */
/* The purpose of this register is to speed up the execution */
DEVICE_INFO *pInformation;
DEVICE Device_Table;
DEVICE_PROP *pProperty;
USER_STANDARD_REQUESTS *pUser_Standard_Requests;

/* Stored in RAM, vectors can be changed on-the-fly */
void (*pEpInt_IN[7])(void) = {
		NOP_Process,
		NOP_Process,
		NOP_Process,
		NOP_Process,
		NOP_Process,
		NOP_Process,
		NOP_Process
		};
void (*pEpInt_OUT[7])(void) = {
		NOP_Process,
		NOP_Process,
		NOP_Process,
		NOP_Process,
		NOP_Process,
		NOP_Process,
		NOP_Process
		};

#define USB_ENDPOINT_DESCRIPTOR_TYPE		0x05

#define PIOS_USB_HID_NUM_INTERFACES		1
#define PIOS_USB_HID_SIZ_CLASS_DESC		18
#define PIOS_USB_HID_SIZ_CONFIG_DESC		32

#define PIOS_USB_NUM_INTERFACES			(PIOS_USB_HID_NUM_INTERFACES)
#define PIOS_USB_SIZ_CONFIG_DESC		(9 + PIOS_USB_HID_SIZ_CONFIG_DESC)

/* USB Standard Device Descriptor */
#define PIOS_USB_SIZ_DEVICE_DESC 18
static const uint8_t PIOS_USB_DeviceDescriptor[PIOS_USB_SIZ_DEVICE_DESC] = {
		(uint8_t)(PIOS_USB_SIZ_DEVICE_DESC & 0xff), /* Device Descriptor length */
		DSCR_DEVICE, /* Descriptor type */
		(uint8_t)(0x0200 & 0xff), /* Specification Version (BCD, LSB) */
		(uint8_t)(0x0200 >> 8), /* Specification Version (BCD, MSB) */
		0x00, /* Device class "Communication" */
		0x00, /* Device sub-class */
		0x00, /* Device sub-sub-class */
		0x40, /* Maximum packet size */
		(uint8_t)((PIOS_USB_VENDOR_ID) & 0xff), /* Vendor ID (LSB) */
		(uint8_t)((PIOS_USB_VENDOR_ID) >> 8), /* Vendor ID (MSB) */
		(uint8_t)((PIOS_USB_PRODUCT_ID) & 0xff), /* Product ID (LSB) */
		(uint8_t)((PIOS_USB_PRODUCT_ID) >> 8), /* Product ID (MSB) */
		(uint8_t)((PIOS_USB_VERSION_ID) & 0xff), /* Product version ID (LSB) */
		(uint8_t)((PIOS_USB_VERSION_ID) >> 8), /* Product version ID (MSB) */
		0x01, /* Manufacturer string index */
		0x02, /* Product string index */
		0x03, /* Serial number string index */
		0x01 /* Number of configurations */
		};

/* USB Configuration Descriptor */
static const uint8_t PIOS_USB_ConfigDescriptor[PIOS_USB_SIZ_CONFIG_DESC] = {
		/* Configuration Descriptor */
		9, /* Descriptor length */
		DSCR_CONFIG, // Descriptor type */
		(PIOS_USB_SIZ_CONFIG_DESC) & 0xff, /* Config + End Points length (LSB) */
		(PIOS_USB_SIZ_CONFIG_DESC) >> 8, /* Config + End Points length (LSB) */
		PIOS_USB_NUM_INTERFACES, /* Number of interfaces */
		0x01, /* Configuration Value */
		0x00, /* Configuration string */
		0x80, /* Attributes (b7 - buspwr, b6 - selfpwr, b5 - rwu) */
		0x32, /* Power requirement (div 2 ma) */

		/* HID */
		/************** Descriptor of Custom HID interface ****************/
		/* 09 */
		0x09, /* bLength: Interface Descriptor size */
		0x04,/* bDescriptorType: Interface descriptor type */
		0x00, /* bInterfaceNumber: Number of Interface */
		0x00, /* bAlternateSetting: Alternate setting */
		0x02, /* bNumEndpoints */
		0x03, /* bInterfaceClass: HID */
		0x00, /* bInterfaceSubClass : 1=BOOT, 0=no boot */
		0x00, /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
		0, /* iInterface: Index of string descriptor */

		/******************** Descriptor of Custom HID HID ********************/
		/* 18 */
		0x09, /* bLength: HID Descriptor size */
		0x21, /* bDescriptorType: HID */
		0x10, /* bcdHID: HID Class Spec release number */
		0x01, 0x00, /* bCountryCode: Hardware target country */
		0x01, /* bNumDescriptors: Number of HID class descriptors to follow */
		0x22, /* bDescriptorType */
		PIOS_USB_HID_SIZ_REPORT_DESC,/* wItemLength: Total length of Report descriptor */
		0x00,

		/******************** Descriptor of Custom HID endpoints ******************/
		/* 27 */
		0x07, /* bLength: Endpoint Descriptor size */
		USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: */

		0x81, /* bEndpointAddress: Endpoint Address (IN) */
		0x03, /* bmAttributes: Interrupt endpoint */
		(PIOS_USB_HID_DATA_LENGTH + 1), /* wMaxPacketSize: 2 Bytes max */
		0x00, 2,//0x20, /* bInterval: Polling Interval (2 ms) */
		/* 34 */

		0x07, /* bLength: Endpoint Descriptor size */
		USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: */
		/*	Endpoint descriptor type */
		0x01, /* bEndpointAddress: */
		/*	Endpoint Address (OUT) */
		0x03, /* bmAttributes: Interrupt endpoint */
		(PIOS_USB_HID_DATA_LENGTH + 1), /* wMaxPacketSize: 2 Bytes max  */
		0x00, 8,//0x20, /* bInterval: Polling Interval (8 ms) */
		/* 41 */
	};

/* Local prototypes */
static void PIOS_USB_CB_Reset(void);
static void PIOS_USB_CB_SetConfiguration(void);
static void PIOS_USB_CB_SetDeviceAddress(void);
static void PIOS_USB_CB_Status_In(void);
static void PIOS_USB_CB_Status_Out(void);
static RESULT PIOS_USB_CB_Data_Setup(uint8_t RequestNo);
static RESULT PIOS_USB_CB_NoData_Setup(uint8_t RequestNo);
static uint8_t *PIOS_USB_CB_GetDeviceDescriptor(uint16_t Length);
static uint8_t *PIOS_USB_CB_GetConfigDescriptor(uint16_t Length);
static uint8_t *PIOS_USB_CB_GetStringDescriptor(uint16_t Length);
static RESULT PIOS_USB_CB_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);


/* USB callback vectors */
static const DEVICE My_Device_Table = {PIOS_USB_EP_NUM, 1};
static const DEVICE_PROP My_Device_Property = {
		0, /* PIOS_USB_CB_Init, */
		PIOS_USB_CB_Reset,
		PIOS_USB_CB_Status_In,
		PIOS_USB_CB_Status_Out,
		PIOS_USB_CB_Data_Setup,
		PIOS_USB_CB_NoData_Setup,
		PIOS_USB_CB_Get_Interface_Setting,
		PIOS_USB_CB_GetDeviceDescriptor,
		PIOS_USB_CB_GetConfigDescriptor,
		PIOS_USB_CB_GetStringDescriptor,
		0,
		0x40 /*MAX PACKET SIZE*/
};
static const USER_STANDARD_REQUESTS My_User_Standard_Requests = {
		NOP_Process, /* PIOS_USB_CB_GetConfiguration, */
		PIOS_USB_CB_SetConfiguration,
		NOP_Process, /* PIOS_USB_CB_GetInterface, */
		NOP_Process, /* PIOS_USB_CB_SetInterface, */
		NOP_Process, /* PIOS_USB_CB_GetStatus, */
		NOP_Process, /* PIOS_USB_CB_ClearFeature, */
		NOP_Process, /* PIOS_USB_CB_SetEndPointFeature, */
		NOP_Process, /* PIOS_USB_CB_SetDeviceFeature, */
		PIOS_USB_CB_SetDeviceAddress};

/* Local Variables */
/* USB Device informations */
static DEVICE_INFO My_Device_Info;
/* USB device status */
static volatile uint32_t bDeviceState = UNCONNECTED;
__IO uint8_t bIntPackSOF = 0;

/**
* Initialises USB interface
* \param[in] mode
*   <UL>
*     <LI>if 0, USB peripheral won't be initialised if this has already been done before
*     <LI>if 1, USB peripheral re-initialisation will be forced
*     <LI>if 2, USB peripheral re-initialisation will be forced, STM32 driver hooks won't be overwritten.<BR>
*         This mode can be used for a local USB driver which installs it's own hooks during runtime.<BR>
*         The application can switch back to PIOS drivers by calling PIOS_USB_Init(1)
*   </UL>
* \return < 0 if initialisation failed
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_Init(uint32_t mode)
{
	/* Currently only mode 0..2 supported */
	if(mode >= 3) {
		/* Unsupported mode */
		return -1;
	}

	/* Clear all USB interrupt requests */
	PIOS_IRQ_Disable();
	/* Interrupt Mask */
	_SetCNTR(0);
	PIOS_IRQ_Enable();

	/* If mode != 2: install PIOS hooks */
	/* A local driver can install it's own hooks and call PIOS_USB_Init(2) to force re-enumeration */
	if(mode != 2) {
		/* Note: usually no need to duplicate this for external drivers */
		pInformation = &My_Device_Info;
		pInformation->Ctrl_Info.Usb_wLength = (PIOS_USB_HID_DATA_LENGTH + 1); /* TODO: Is this required? */

		/* Following hooks/pointers should be replaced by external drivers */
		memcpy(&Device_Table, (DEVICE *) &My_Device_Table, sizeof(Device_Table));
		pProperty = (DEVICE_PROP *) &My_Device_Property;
		pUser_Standard_Requests = (USER_STANDARD_REQUESTS *) &My_User_Standard_Requests;

		pEpInt_OUT[0] = PIOS_USB_HID_EP1_OUT_Callback;
	}

	PIOS_USB_HID_ChangeConnectionState(0);

	pInformation->ControlState = 2;
	pInformation->Current_Configuration = 0;

	/* If mode == 0: don't initialise USB if not required (important for BSL) */
	if(mode == 0 && PIOS_USB_IsInitialized()) {
		pInformation->Current_Feature = PIOS_USB_ConfigDescriptor[7];
		pInformation->Current_Configuration = 1;
		pUser_Standard_Requests->User_SetConfiguration();
	} else {
		/* Force USB reset and power-down (this will also release the USB pins for direct GPIO control) */
		_SetCNTR(CNTR_FRES | CNTR_PDWN);

		/* Using a "dirty" method to force a re-enumeration: */
		/* Force DPM (Pin PA12) low for ca. 10 mS before USB Tranceiver will be enabled */
		/* This overrules the external Pull-Up at PA12, and at least Windows & MacOS will enumerate again */
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		PIOS_DELAY_WaitmS(50);

		/* Release power-down, still hold reset */
		_SetCNTR(CNTR_PDWN);
		PIOS_DELAY_WaituS(5);

		/* CNTR_FRES = 0 */
		_SetCNTR(0);

		/* Clear pending interrupts */
		_SetISTR(0);

		/* Configure USB clock */
		/* USBCLK = PLLCLK / 1.5 */
		RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
		/* Enable USB clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
	}

	/* Don't set interrupt mask on custom driver installation */
	if(mode != 2) {
		/* Clear pending interrupts (again) */
		_SetISTR(0);

		/* Set interrupts mask */
		_SetCNTR(IMR_MSK);
	}

	bDeviceState = UNCONNECTED;

	/* Enable USB interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_USB_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* No error */
	return 0;
}

/**
* Interrupt handler for USB
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	uint16_t wIstr = _GetISTR();

	if(wIstr & ISTR_RESET) {
		_SetISTR((uint16_t)CLR_RESET);
		pProperty->Reset();
	}

	if(wIstr & ISTR_SOF) {
		_SetISTR((uint16_t)CLR_SOF);
	}

	if(wIstr & ISTR_CTR) {
		/* Servicing of the endpoint correct transfer interrupt */
		/* Clear of the CTR flag into the sub */
		CTR_LP();
	}
}

/**
* Allows to query, if the USB interface has already been initialised.<BR>
* This function is used by the bootloader to avoid a reconnection, it isn't
* relevant for typical applications!
* \return 1 if USB already initialised, 0 if not initialised
*/
int32_t PIOS_USB_IsInitialized(void)
{
	/* We assume that initialisation has been done when endpoint 0 contains a value */
	return GetEPType(ENDP0) ? 1 : 0;
}

/**
* Reads the USB detect pin to determine if a USB cable is connected
* \return 0 if cable not connected
* \return 1 if cable is connected
*/
int32_t PIOS_USB_CableConnected(void)
{
	return GPIO_ReadInputDataBit(USB_ACC_GPIO_PORT, USB_DETECT_PIN);
}


/*
* Hooks of STM32 USB library
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/

/**
* Reset Routine
*/
static void PIOS_USB_CB_Reset(void)
{
	/* Set PIOS Device as not configured */
	pInformation->Current_Configuration = 0;

	/* Current Feature initialisation */
	pInformation->Current_Feature = PIOS_USB_ConfigDescriptor[7];

	/* Set PIOS Device with the default Interface */
	pInformation->Current_Interface = 0;
	SetBTABLE(PIOS_USB_BTABLE_ADDRESS);

	/* Initialise Endpoint 0 */
	SetEPType(ENDP0, EP_CONTROL);
	SetEPTxStatus(ENDP0, EP_TX_STALL);
	SetEPRxAddr(ENDP0, PIOS_USB_ENDP0_RXADDR);
	SetEPTxAddr(ENDP0, PIOS_USB_ENDP0_TXADDR);
	Clear_Status_Out(ENDP0);
	SetEPRxCount(ENDP0, pProperty->MaxPacketSize);
	SetEPRxValid(ENDP0);

	/* Initialise Endpoint 1 */
	SetEPType(ENDP1, EP_INTERRUPT);
	SetEPTxAddr(ENDP1, PIOS_USB_ENDP1_TXADDR);
	SetEPRxAddr(ENDP1, PIOS_USB_ENDP1_RXADDR);
	SetEPTxCount(ENDP1, (PIOS_USB_HID_DATA_LENGTH + 1));
	SetEPRxCount(ENDP1, (PIOS_USB_HID_DATA_LENGTH + 1));
	SetEPTxStatus(ENDP1, EP_TX_NAK);
	SetEPRxStatus(ENDP1, EP_RX_VALID);

	/* Propagate connection state to USB HID driver */
	PIOS_USB_HID_ChangeConnectionState(0);

	/* Set this device to response on default address */
	SetDeviceAddress(0);

	bDeviceState = ATTACHED;
}

/**
* Update the device state to configured
*/
static void PIOS_USB_CB_SetConfiguration(void)
{
	if(pInformation->Current_Configuration != 0) {
		/* Propagate connection state to USB HID driver */
		PIOS_USB_HID_ChangeConnectionState(1); /* Connected */
		bDeviceState = CONFIGURED;

	}
}

/**
* Update the device state to addressed
*/
static void PIOS_USB_CB_SetDeviceAddress(void)
{
	bDeviceState = ADDRESSED;
}

/**
* Status IN routine
*/
static void PIOS_USB_CB_Status_In(void)
{

}

/**
* Status OUT routine
*/
static void PIOS_USB_CB_Status_Out(void)
{

}

/**
* Data setup routine
*/
static RESULT PIOS_USB_CB_Data_Setup(uint8_t RequestNo)
{
	RESULT Result;
	if((Result = PIOS_USB_HID_CB_Data_Setup(RequestNo)) != USB_UNSUPPORT) {
		return Result;
	}

	return USB_UNSUPPORT;
}

/**
* Handles the non data class specific requests
*/
static RESULT PIOS_USB_CB_NoData_Setup(uint8_t RequestNo)
{
	RESULT res;
	if((res = PIOS_USB_HID_CB_NoData_Setup(RequestNo)) != USB_UNSUPPORT) {
		return res;
	}

	return USB_UNSUPPORT;
}

/**
* Gets the device descriptor
*/
static uint8_t *PIOS_USB_CB_GetDeviceDescriptor(uint16_t Length)
{
	ONE_DESCRIPTOR desc = {(uint8_t *) PIOS_USB_DeviceDescriptor, PIOS_USB_SIZ_DEVICE_DESC};
	return Standard_GetDescriptorData(Length, &desc);
}

/**
* Gets the configuration descriptor
*/
static uint8_t *PIOS_USB_CB_GetConfigDescriptor(uint16_t Length)
{
	ONE_DESCRIPTOR desc = {(uint8_t *) PIOS_USB_ConfigDescriptor, PIOS_USB_SIZ_CONFIG_DESC};
	return Standard_GetDescriptorData(Length, &desc);
}

/**
* Gets the string descriptors according to the needed index
*/
static uint8_t *PIOS_USB_CB_GetStringDescriptor(uint16_t Length)
{
	const uint8_t vendor_str[] = PIOS_USB_VENDOR_STR;
	const uint8_t product_str[] = PIOS_USB_PRODUCT_STR;

	uint8_t buffer[200];
	uint16_t len;
	int i;

	switch(pInformation->USBwValue0) {
		case 0: /* Language */
			/* buffer[0] and [1] initialised below */
			buffer[2] = 0x09; // CharSet
			buffer[3] = 0x04; // U.S.
			len = 4;
			break;

		case 1: /* Vendor */
			/* buffer[0] and [1] initialised below */
			for(i = 0, len = 2; vendor_str[i] != '\0' && len < 200; ++i) {
				buffer[len++] = vendor_str[i];
				buffer[len++] = 0;
			}
			break;

		case 2: /* Product */
			/* buffer[0] and [1] initialised below */
			for(i = 0, len = 2; product_str[i] != '\0' && len < 200; ++i) {
				buffer[len++] = product_str[i];
				buffer[len++] = 0;
			}
			break;

		case 3: { /* Serial Number */
			uint8_t serial_number_str[40];
			if(PIOS_SYS_SerialNumberGet((char *) serial_number_str) >= 0) {
				for(i = 0, len = 2; serial_number_str[i] != '\0' && len < 200; ++i) {
					buffer[len++] = serial_number_str[i];
					buffer[len++] = 0;
				}
			} else
				return NULL;
		}
			break;
		default: /* string ID not supported */
			return NULL;
	}

	buffer[0] = len; /* Descriptor Length */
	buffer[1] = DSCR_STRING; /* Descriptor Type */
	ONE_DESCRIPTOR desc = {(uint8_t *) buffer, len};
	return Standard_GetDescriptorData(Length, &desc);
}

/**
* Test the interface and the alternate setting according to the supported one.
*/
static RESULT PIOS_USB_CB_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
	if(AlternateSetting > 0) {
		return USB_UNSUPPORT;
	} else if(Interface >= PIOS_USB_NUM_INTERFACES) {
		return USB_UNSUPPORT;
	}

	return USB_SUCCESS;
}

#endif
