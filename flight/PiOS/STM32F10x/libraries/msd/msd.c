/**
 ******************************************************************************
 *
 * @file       msd.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      USB Mass Storage Device Driver
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   MSD MSD Functions
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

/* Include files */
#include <pios.h>
#include <usb_lib.h>

#include <string.h>

#include "msd.h"
#include "msd_desc.h"
#include "msd_bot.h"
#include "msd_memory.h"

/* Local definitions */

/* MASS Storage Requests */
#define GET_MAX_LUN                0xFE
#define MASS_STORAGE_RESET         0xFF
#define LUN_DATA_LENGTH            1

/* ISTR events */
/* IMR_MSK */
/* mask defining which events has to be handled */
/* by the device application software */
#define MSD_IMR_MSK (CNTR_RESETM)

/* Local prototypes */
static void MSD_MASS_Reset(void);
static void MSD_Mass_Storage_SetConfiguration(void);
static void MSD_Mass_Storage_ClearFeature(void);
static void MSD_Mass_Storage_SetDeviceAddress(void);
static void MSD_MASS_Status_In(void);
static void MSD_MASS_Status_Out(void);
static RESULT MSD_MASS_Data_Setup(uint8_t);
static RESULT MSD_MASS_NoData_Setup(uint8_t);
static RESULT MSD_MASS_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
static uint8_t *MSD_MASS_GetDeviceDescriptor(uint16_t);
static uint8_t *MSD_MASS_GetConfigDescriptor(uint16_t);
static uint8_t *MSD_MASS_GetStringDescriptor(uint16_t);
static uint8_t *Get_Max_Lun(uint16_t Length);

/* Local variables */
static const DEVICE My_Device_Table = {MSD_EP_NUM, 1};
static const DEVICE_PROP My_Device_Property = {
		0, // Init hook not used, done by PIOS_USB module!
		MSD_MASS_Reset, MSD_MASS_Status_In, MSD_MASS_Status_Out, MSD_MASS_Data_Setup, MSD_MASS_NoData_Setup, MSD_MASS_Get_Interface_Setting,
		MSD_MASS_GetDeviceDescriptor, MSD_MASS_GetConfigDescriptor, MSD_MASS_GetStringDescriptor, 0, 0x40 /*MAX PACKET SIZE*/
};
static const USER_STANDARD_REQUESTS My_User_Standard_Requests = {
		NOP_Process,
		MSD_Mass_Storage_SetConfiguration,
		NOP_Process,
		NOP_Process,
		NOP_Process,
		MSD_Mass_Storage_ClearFeature,
		NOP_Process, NOP_Process,
		MSD_Mass_Storage_SetDeviceAddress
};
static ONE_DESCRIPTOR Device_Descriptor = {(uint8_t *) MSD_MASS_DeviceDescriptor, MSD_MASS_SIZ_DEVICE_DESC};
static ONE_DESCRIPTOR Config_Descriptor = {(uint8_t *) MSD_MASS_ConfigDescriptor, MSD_MASS_SIZ_CONFIG_DESC};
static ONE_DESCRIPTOR String_Descriptor[5] = {
		{(uint8_t *) MSD_MASS_StringLangID, MSD_MASS_SIZ_STRING_LANGID},
		{(uint8_t *) MSD_MASS_StringVendor, MSD_MASS_SIZ_STRING_VENDOR},
		{(uint8_t *) MSD_MASS_StringProduct, MSD_MASS_SIZ_STRING_PRODUCT},
		{(uint8_t *) MSD_MASS_StringSerial, MSD_MASS_SIZ_STRING_SERIAL},
		{(uint8_t *) MSD_MASS_StringInterface, MSD_MASS_SIZ_STRING_INTERFACE},
};
static uint8_t lun_available;

/**
* Initialises the USB Device Driver for a Mass Storage Device
*
* Should be called during runtime once a SD Card has been connected.<BR>
* It is possible to switch back to the original device driver provided
* by calling PIOS_USB_Init(1)
*
* \param[in] mode currently only mode 0 supported
* \return < 0 if initialisation failed
*/
int32_t MSD_Init(uint32_t mode)
{
	/* Update the serial number string descriptor with the data from the unique ID*/
	uint8_t serial_number_str[40];
	int i, len;
	PIOS_SYS_SerialNumberGet((char *) serial_number_str);
	for(i = 0, len = 0; serial_number_str[i] != '\0' && len < 25; ++i) {
		MSD_MASS_StringSerial[len++] = serial_number_str[i];
		MSD_MASS_StringSerial[len++] = 0;
	}

	lun_available = 0;

	/* All LUNs available after USB init */
	for(i = 0; i < MSD_NUM_LUN; ++i) {
		MSD_LUN_AvailableSet(i, 1);
	}

	PIOS_IRQ_Disable();

	/* Clear all USB interrupt requests */
	_SetCNTR(0); /* Interrupt Mask */
	_SetISTR(0); /* clear pending requests */

	/* Switch to MSD driver hooks */
	memcpy(&Device_Table, (DEVICE *) &My_Device_Table, sizeof(Device_Table));
	pProperty = (DEVICE_PROP *) &My_Device_Property;
	pUser_Standard_Requests = (USER_STANDARD_REQUESTS *) &My_User_Standard_Requests;

	/* Change endpoints */
	pEpInt_IN[0] = MSD_Mass_Storage_In;
	pEpInt_OUT[1] = MSD_Mass_Storage_Out;

	/* Force re-enumeration w/o overwriting PIOS hooks */
	PIOS_USB_Init(2);

	/* Clear pending interrupts (again) */
	_SetISTR(0);

	/* Set interrupts mask */
	_SetCNTR(MSD_IMR_MSK);

	PIOS_IRQ_Enable();

	/* No error */
	return 0;
}

/**
* Should be called periodically each millisecond so long this driver is
* active (and only then!) to handle USBtransfers.
*
* Take care that no other task accesses SD Card while this function is
* processed!
*
* Ensure that this function isn't called when a PIOS USB driver is running!
*
* \return < 0 on errors
*/
int32_t MSD_Periodic_mS(void)
{
	/* Call endpoint handler of STM32 USB driver */
	CTR_LP();

	/* No error */
	return 0;
}

/**
* This function returns the connection status of the USB HID interface
* \return 1: interface available
* \return 0: interface not available
*/
int32_t MSD_CheckAvailable(void)
{
	return pInformation->Current_Configuration ? 1 : 0;
}

/**
* The logical unit is available whenever MSD_Init() is called, or the USB
* cable has been reconnected.
*
* It will be disabled when the host unmounts the file system (like if the
* SD Card would be removed.
*
* When this happens, the application can either call PIOS_USB_Init(1)
* again, e.g. to switch to USB HID, or it can make the LUN available
* again by calling MSD_LUN_AvailableSet(0, 1)
* \param[in] lun Logical Unit number (0)
* \param[in] available 0 or 1
* \return < 0 on errors
*/
int32_t MSD_LUN_AvailableSet(uint8_t lun, uint8_t available)
{
	if(lun >= MSD_NUM_LUN) {
		return -1;
	}

	if(available) {
		lun_available |= (1 << lun);
	} else {
		lun_available &= ~(1 << lun);
	}

	/* No error */
	return 0;
}

/**
* \return 1 if device is mounted by host
* \return 0 if device is not mounted by host
*/
int32_t MSD_LUN_AvailableGet(uint8_t lun)
{
	if(lun >= MSD_NUM_LUN)
		return 0;

	return (lun_available & (1 << lun)) ? 1 : 0;
}

/**
* Mass Storage reset routine.
*/
static void MSD_MASS_Reset()
{
	/* Set the device as not configured */
	pInformation->Current_Configuration = 0;

	/* Current Feature initialization */
	pInformation->Current_Feature = MSD_MASS_ConfigDescriptor[7];

	SetBTABLE(MSD_BTABLE_ADDRESS);

	/* Initialize Endpoint 0 */
	SetEPType(ENDP0, EP_CONTROL);
	SetEPTxStatus(ENDP0, EP_TX_NAK);
	SetEPRxAddr(ENDP0, MSD_ENDP0_RXADDR);
	SetEPRxCount(ENDP0, pProperty->MaxPacketSize);
	SetEPTxAddr(ENDP0, MSD_ENDP0_TXADDR);
	Clear_Status_Out(ENDP0);
	SetEPRxValid(ENDP0);

	/* Initialize Endpoint 1 */
	SetEPType(ENDP1, EP_BULK);
	SetEPTxAddr(ENDP1, MSD_ENDP1_TXADDR);
	SetEPTxStatus(ENDP1, EP_TX_NAK);
	SetEPRxStatus(ENDP1, EP_RX_DIS);

	/* Initialize Endpoint 2 */
	SetEPType(ENDP2, EP_BULK);
	SetEPRxAddr(ENDP2, MSD_ENDP2_RXADDR);
	SetEPRxCount(ENDP2, pProperty->MaxPacketSize);
	SetEPRxStatus(ENDP2, EP_RX_VALID);
	SetEPTxStatus(ENDP2, EP_TX_DIS);

	SetEPRxCount(ENDP0, pProperty->MaxPacketSize);
	SetEPRxValid(ENDP0);

	/* Set the device to response on default address */
	SetDeviceAddress(0);

	MSD_CBW.dSignature = BOT_CBW_SIGNATURE;
	MSD_Bot_State = BOT_IDLE;
}

/**
* Handle the SetConfiguration request.
*/
static void MSD_Mass_Storage_SetConfiguration(void)
{
	if(pInformation->Current_Configuration != 0) {
		ClearDTOG_TX(ENDP1);
		ClearDTOG_RX(ENDP2);
		MSD_Bot_State = BOT_IDLE; /* Set the Bot state machine to the IDLE state */

		/* All LUNs available after USB (re-)connection */
		for(int i = 0; i < MSD_NUM_LUN; ++i) {
			MSD_LUN_AvailableSet(i, 1);
		}
	}
}

/**
* Handle the ClearFeature request.
*/
static void MSD_Mass_Storage_ClearFeature(void)
{
	/* When the host send a CBW with invalid signature or invalid length the two */
	/* Endpoints (IN & OUT) shall stall until receiving a Mass Storage Reset */
	if(MSD_CBW.dSignature != BOT_CBW_SIGNATURE) {
		MSD_Bot_Abort(BOTH_DIR);
	}
}

/**
* Udpade the device state to addressed.
*/
static void MSD_Mass_Storage_SetDeviceAddress(void)
{

}

/**
* Mass Storage Status IN routine.
*/
static void MSD_MASS_Status_In(void)
{
	return;
}

/*******************************************************************************
* Mass Storage Status OUT routine.
*/
static void MSD_MASS_Status_Out(void)
{
	return;
}

/*******************************************************************************
* Handle the data class specific requests..
* \param[in] RequestNo
* \return RESULT
*/
static RESULT MSD_MASS_Data_Setup(uint8_t RequestNo)
{
	uint8_t *(*CopyRoutine)( uint16_t);

	CopyRoutine = NULL;
	if((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) && (RequestNo == GET_MAX_LUN) && (pInformation->USBwValue == 0)
			&& (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x01)) {
		CopyRoutine = Get_Max_Lun;
	} else {
		return USB_UNSUPPORT;
	}

	if(CopyRoutine == NULL) {
		return USB_UNSUPPORT;
	}

	pInformation->Ctrl_Info.CopyData = CopyRoutine;
	pInformation->Ctrl_Info.Usb_wOffset = 0;
	(*CopyRoutine)(0);

	return USB_SUCCESS;
}

/**
* Handle the no data class specific requests.
* \param[in] RequestNo
* \return RESULT
*/
static RESULT MSD_MASS_NoData_Setup(uint8_t RequestNo)
{
	if((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) && (RequestNo == MASS_STORAGE_RESET) && (pInformation->USBwValue == 0)
			&& (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x00)) {
		/* Initialise Endpoint 1 */
		ClearDTOG_TX(ENDP1);

		/* Initialise Endpoint 2 */
		ClearDTOG_RX(ENDP2);

		/* Initialise the CBW signature to enable the clear feature*/
		MSD_CBW.dSignature = BOT_CBW_SIGNATURE;
		MSD_Bot_State = BOT_IDLE;

		return USB_SUCCESS;
	}
	return USB_UNSUPPORT;
}

/**
* Test the interface and the alternate setting according to the supported one.
* \param[in] Interface
* \param[in] AlternateSetting
* \return RESULT
*/
static RESULT MSD_MASS_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
	if(AlternateSetting > 0) {
		return USB_UNSUPPORT;/* In this application we don't have AlternateSetting*/
	} else if(Interface > 0) {
		return USB_UNSUPPORT;/* In this application we have only 1 interfaces*/
	}
	return USB_SUCCESS;
}

/**
* Get the device descriptor.
* \param[in] Length
*/
static uint8_t *MSD_MASS_GetDeviceDescriptor(uint16_t Length)
{
	return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/**
* Get the configuration descriptor.
* \param[in] Length
*/
static uint8_t *MSD_MASS_GetConfigDescriptor(uint16_t Length)
{
	return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/**
* Get the string descriptors according to the needed index.
* \param[in] Length
*/
static uint8_t *MSD_MASS_GetStringDescriptor(uint16_t Length)
{
	uint8_t wValue0 = pInformation->USBwValue0;

	if(wValue0 > 5) {
		return NULL;
	} else {
		return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
	}
}

/**
* Handle the Get Max Lun request.
* \param[in] Length
*/
static uint8_t *Get_Max_Lun(uint16_t Length)
{
	static uint32_t Max_Lun = MSD_NUM_LUN - 1;

	if(Length == 0) {
		pInformation->Ctrl_Info.Usb_wLength = LUN_DATA_LENGTH;
		return 0;
	} else {
		/* This copy concept requires statically allocated data... grr! */
		return ((uint8_t*) (&Max_Lun));
	}
}

