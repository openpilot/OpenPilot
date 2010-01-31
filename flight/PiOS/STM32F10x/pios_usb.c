/**
 ******************************************************************************
 *
 * @file       pios_usb.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.
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
#define PIOS_DONT_USE_USB_MIDI
#define PIOS_USE_USB_COM

/* Private Function Prototypes */

/* Local Variables */

#include <usb_lib.h>
#include <string.h>

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
#define IMR_MSK (CNTR_CTRM | CNTR_RESETM)

/* Local types */
typedef enum _DEVICE_STATE {
	UNCONNECTED, ATTACHED, POWERED, SUSPENDED, ADDRESSED, CONFIGURED
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
void (*pEpInt_IN[7])(void) = {NOP_Process, NOP_Process, NOP_Process, NOP_Process, NOP_Process, NOP_Process, NOP_Process};
void (*pEpInt_OUT[7])(void) = {NOP_Process, NOP_Process, NOP_Process, NOP_Process, NOP_Process, NOP_Process, NOP_Process};

#define PIOS_USB_NUM_INTERFACES              (0)
#define PIOS_USB_SIZ_CONFIG_DESC             (9 + 0)

/* USB Standard Device Descriptor */
#define PIOS_USB_SIZ_DEVICE_DESC 18
static const uint8_t PIOS_USB_DeviceDescriptor[PIOS_USB_SIZ_DEVICE_DESC] = {(uint8_t)(PIOS_USB_SIZ_DEVICE_DESC & 0xff), /* Device Descriptor length */
		DSCR_DEVICE, /* Descriptor type */
		(uint8_t)(0x0200 & 0xff), /* Specification Version (BCD, LSB) */
		(uint8_t)(0x0200 >> 8), /* Specification Version (BCD, MSB) */
#if 1
		0x02, /* Device class "Communication"   -- required for MacOS to find the COM device. Audio Device works fine in parallel to this */
#else
		0x00, /* Device class "Composite" */
#endif
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

/* USB Config Descriptor */
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

		/* TODO:HID */

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
		PIOS_USB_CB_Reset, PIOS_USB_CB_Status_In, PIOS_USB_CB_Status_Out, PIOS_USB_CB_Data_Setup, PIOS_USB_CB_NoData_Setup,
		PIOS_USB_CB_Get_Interface_Setting, PIOS_USB_CB_GetDeviceDescriptor, PIOS_USB_CB_GetConfigDescriptor, PIOS_USB_CB_GetStringDescriptor, 0, 0x40 /*MAX PACKET SIZE*/
};
static const USER_STANDARD_REQUESTS My_User_Standard_Requests = {NOP_Process, /* PIOS_USB_CB_GetConfiguration, */
		PIOS_USB_CB_SetConfiguration, NOP_Process, /* PIOS_USB_CB_GetInterface, */
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
static __IO uint32_t bDeviceState = UNCONNECTED;

/**
* Initialises USB interface
* \param[in] mode
*   <UL>
*     <LI>if 0, USB peripheral won't be initialised if this has already been done before
*     <LI>if 1, USB peripheral re-initialisation will be forced
*     <LI>if 2, USB peripheral re-initialisation will be forced, STM32 driver hooks won't be overwritten.<BR>
*         This mode can be used for a local USB driver which installs it's own hooks during runtime.<BR>
*         The application can switch back to MIOS32 drivers (e.g. PIOS_USB_MIDI) by calling PIOS_USB_Init(1)
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
	_SetCNTR(0); /* Interrupt Mask */
	PIOS_IRQ_Enable();

	/* if mode != 2: install PIOS hooks */
	/* a local driver can install it's own hooks and call PIOS_USB_Init(2) to force re-enumeration */
	if(mode != 2) {
		pInformation = &My_Device_Info; /* Note: usually no need to duplicate this for external drivers */

		/* Following hooks/pointers should be replaced by external drivers */
		memcpy(&Device_Table, (DEVICE *) &My_Device_Table, sizeof(Device_Table));
		pProperty = (DEVICE_PROP *) &My_Device_Property;
		pUser_Standard_Requests = (USER_STANDARD_REQUESTS *) &My_User_Standard_Requests;
	}

	pInformation->ControlState = 2;
	pInformation->Current_Configuration = 0;

	/* if mode == 0: don't initialise USB if not required (important for BSL) */
	if(mode == 0 && PIOS_USB_IsInitialized()) {
		pInformation->Current_Feature = PIOS_USB_ConfigDescriptor[7];
		pInformation->Current_Configuration = 1;
		pUser_Standard_Requests->User_SetConfiguration();

	} else {
		/* Force USB reset and power-down (this will also release the USB pins for direct GPIO control) */
		_SetCNTR(CNTR_FRES | CNTR_PDWN);

#if 0
		/* Disabled because it doesn't work, hardware needs to be looked into */
		/* Configure USB disconnect pin */
		/* first we hold it low for ca. 50 mS to force a re-enumeration */
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = USB_PULLUP_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(USB_ACC_GPIO_PORT, &GPIO_InitStructure);

		GPIO_SetBits(USB_ACC_GPIO_PORT, USB_PULLUP_PIN);
		PIOS_DELAY_WaitmS(50);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
		GPIO_Init(USB_ACC_GPIO_PORT, &GPIO_InitStructure);
#endif

		/* Using a "dirty" method to force a re-enumeration: */
		/* Force DPM (Pin PA12) low for ca. 10 mS before USB Tranceiver will be enabled */
		/* This overrules the external Pull-Up at PA12, and at least Windows & MacOS will enumerate again */
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_ResetBits(USB_ACC_GPIO_PORT, USB_PULLUP_PIN);

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
		_SetCNTR(IMR_MSK); /* Interrupt mask */
	}

	bDeviceState = UNCONNECTED;

	/* Enable USB interrupts (unfortunately shared with CAN Rx0, as either CAN or USB can be used, but not at the same time) */
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


/*
* Hooks of STM32 USB library
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
/**
* Reset Routine
*/
static void PIOS_USB_CB_Reset(void)
{
	/* Set MIOS32 Device as not configured */
	pInformation->Current_Configuration = 0;

	/* Current Feature initialization */
	pInformation->Current_Feature = PIOS_USB_ConfigDescriptor[7];

	/* Set PIOS Device with the default Interface */
	pInformation->Current_Interface = 0;
	SetBTABLE(PIOS_USB_BTABLE_ADDRESS);

	/* Initialize Endpoint 0 */
	SetEPType(ENDP0, EP_CONTROL);
	SetEPTxStatus(ENDP0, EP_TX_STALL);
	SetEPRxAddr(ENDP0, PIOS_USB_ENDP0_RXADDR);
	SetEPTxAddr(ENDP0, PIOS_USB_ENDP0_TXADDR);
	Clear_Status_Out(ENDP0);
	SetEPRxCount(ENDP0, pProperty->MaxPacketSize);
	SetEPRxValid(ENDP0);

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
	return USB_UNSUPPORT;
}

/**
* Handles the non data class specific requests
*/
static RESULT PIOS_USB_CB_NoData_Setup(uint8_t RequestNo)
{
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
			/* buffer[0] and [1] initialized below */
			buffer[2] = 0x09; // CharSet
			buffer[3] = 0x04; // U.S.
			len = 4;
			break;

		case 1: /* Vendor */
			/* buffer[0] and [1] initialized below */
			for(i = 0, len = 2; vendor_str[i] != '\0' && len < 200; ++i) {
				buffer[len++] = vendor_str[i];
				buffer[len++] = 0;
			}
			break;

		case 2: /* Product */
			/* buffer[0] and [1] initialized below */
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

