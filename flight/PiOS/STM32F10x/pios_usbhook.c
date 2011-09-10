/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : pios_usbhook.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : All processings related to Custom HID Demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "usb_lib.h"
#include "usb_conf.h"
#include "pios.h"
#include "pios_usbhook.h"
#include "pios_usb_hid_desc.h"
#include "pios_usb_hid_pwr.h"
#include "pios_usb_hid.h"
#include "pios_usb_com_priv.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t ProtocolValue;
__IO uint8_t EXTI_Enable;

/* -------------------------------------------------------------------------- */
/*  Structures initializations */
/* -------------------------------------------------------------------------- */

DEVICE Device_Table = {
	EP_NUM,
	1
};

static void PIOS_USBHOOK_Init(void);
static void PIOS_USBHOOK_Reset(void);
static void PIOS_USBHOOK_Status_In(void);
static void PIOS_USBHOOK_Status_Out(void);
static RESULT PIOS_USBHOOK_Data_Setup(uint8_t RequestNo);
static RESULT PIOS_USBHOOK_NoData_Setup(uint8_t RequestNo);
static RESULT PIOS_USBHOOK_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
static uint8_t *PIOS_USBHOOK_GetDeviceDescriptor(uint16_t Length);
static uint8_t *PIOS_USBHOOK_GetConfigDescriptor(uint16_t Length);
static uint8_t *PIOS_USBHOOK_GetStringDescriptor(uint16_t Length);

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

static ONE_DESCRIPTOR Device_Descriptor = {
	(uint8_t *) PIOS_HID_DeviceDescriptor,
	PIOS_HID_SIZ_DEVICE_DESC
};

static ONE_DESCRIPTOR Config_Descriptor = {
	(uint8_t *) PIOS_HID_ConfigDescriptor,
	PIOS_HID_SIZ_CONFIG_DESC
};

static ONE_DESCRIPTOR PIOS_HID_Report_Descriptor = {
	(uint8_t *) PIOS_HID_ReportDescriptor,
	PIOS_HID_SIZ_REPORT_DESC
};

static ONE_DESCRIPTOR PIOS_HID_Hid_Descriptor = {
	(uint8_t *) PIOS_HID_ConfigDescriptor + PIOS_HID_OFF_HID_DESC,
	PIOS_HID_SIZ_HID_DESC
};

static ONE_DESCRIPTOR String_Descriptor[4] = {
	{(uint8_t *) PIOS_HID_StringLangID, PIOS_HID_SIZ_STRING_LANGID}
	,
	{(uint8_t *) PIOS_HID_StringVendor, PIOS_HID_SIZ_STRING_VENDOR}
	,
	{(uint8_t *) PIOS_HID_StringProduct, PIOS_HID_SIZ_STRING_PRODUCT}
	,
	{(uint8_t *) PIOS_HID_StringSerial, PIOS_HID_SIZ_STRING_SERIAL}
};

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static RESULT PIOS_USBHOOK_SetProtocol(void);
static uint8_t *PIOS_USBHOOK_GetProtocolValue(uint16_t Length);
static uint8_t *PIOS_USBHOOK_GetReportDescriptor(uint16_t Length);
static uint8_t *PIOS_USBHOOK_GetHIDDescriptor(uint16_t Length);
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_Init.
* Description    : Custom HID init routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
static void PIOS_USBHOOK_Init(void)
{
	/* Update the serial number string descriptor with the data from the unique 
	   ID */
	//Get_SerialNum();

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
	pInformation->Current_Feature = PIOS_HID_ConfigDescriptor[7];

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

	/* Initialize Endpoint 1 (HID) */
	SetEPType(ENDP1, EP_INTERRUPT);
	SetEPTxAddr(ENDP1, ENDP1_TXADDR);
	SetEPTxCount(ENDP1, PIOS_USB_COM_DATA_LENGTH + 2);
	SetEPTxStatus(ENDP1, EP_TX_NAK);

	SetEPRxAddr(ENDP1, ENDP1_RXADDR);
	SetEPRxCount(ENDP1, PIOS_USB_COM_DATA_LENGTH + 2);
	SetEPRxStatus(ENDP1, EP_RX_VALID);

#if defined(PIOS_INCLUDE_USB_COM_CDC)
	/* Initialize Endpoint 2 (CDC Call Control) */
	SetEPType(ENDP2, EP_INTERRUPT);
	SetEPTxAddr(ENDP2, ENDP2_TXADDR);
	SetEPTxStatus(ENDP2, EP_TX_NAK);

	SetEPRxAddr(ENDP2, ENDP2_RXADDR);
	SetEPRxCount(ENDP2, PIOS_USB_COM_DATA_LENGTH + 2);
	SetEPRxStatus(ENDP2, EP_RX_DIS);

	/* Initialize Endpoint 3 (CDC Data) */
	SetEPType(ENDP3, EP_BULK);
	SetEPTxAddr(ENDP3, ENDP3_TXADDR);
	SetEPTxStatus(ENDP3, EP_TX_NAK);

	SetEPRxAddr(ENDP3, ENDP3_RXADDR);
	SetEPRxCount(ENDP3, PIOS_USB_COM_DATA_LENGTH + 2);
	SetEPRxStatus(ENDP3, EP_RX_VALID);

#endif	/* PIOS_INCLUDE_USB_COM_CDC */

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
	PIOS_USB_HID_ChangeConnectionState(pInformation->Current_Configuration != 0);
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
static RESULT PIOS_USBHOOK_Data_Setup(uint8_t RequestNo)
{
	uint8_t *(*CopyRoutine) (uint16_t);

	CopyRoutine = NULL;

	switch (Type_Recipient) {
	case (STANDARD_REQUEST | INTERFACE_RECIPIENT):
		switch (pInformation->USBwIndex0) {
		case 0:		/* HID Interface */
			switch (RequestNo) {
			case GET_DESCRIPTOR:
				switch (pInformation->USBwValue1) {
				case REPORT_DESCRIPTOR:
					CopyRoutine = PIOS_USBHOOK_GetReportDescriptor;
					break;
				case HID_DESCRIPTOR_TYPE:
					CopyRoutine = PIOS_USBHOOK_GetHIDDescriptor;
					break;
				}
			}
		}
		break;

	case (CLASS_REQUEST | INTERFACE_RECIPIENT):
		switch (pInformation->USBwIndex0) {
		case 0:		/* HID Interface */
			switch (RequestNo) {
			case GET_PROTOCOL:
				CopyRoutine = PIOS_USBHOOK_GetProtocolValue;
				break;
			}

			break;
#if defined(PIOS_INCLUDE_USB_COM_CDC)
		case 1:		/* CDC Call Control Interface */
			switch (RequestNo) {
			case GET_LINE_CODING:
				CopyRoutine = PIOS_CDC_GetLineCoding;
				break;
			}

			break;

		case 2:		/* CDC Data Interface */
			switch (RequestNo) {
			case 0:
				break;
			}

			break;
#endif	/* PIOS_INCLUDE_USB_COM_CDC */
		}
		break;
	}

	if (CopyRoutine == NULL) {
		return USB_UNSUPPORT;
	}

	pInformation->Ctrl_Info.CopyData = CopyRoutine;
	pInformation->Ctrl_Info.Usb_wOffset = 0;
	(*CopyRoutine) (0);
	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_NoData_Setup
* Description    : handle the no data class specific requests
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
static RESULT PIOS_USBHOOK_NoData_Setup(uint8_t RequestNo)
{
	switch (Type_Recipient) {
	case (CLASS_REQUEST | INTERFACE_RECIPIENT):
		switch (pInformation->USBwIndex0) {
		case 0:		/* HID */
			switch (RequestNo) {
			case SET_PROTOCOL:
				return PIOS_USBHOOK_SetProtocol();
				break;
			}

			break;

#if defined(PIOS_INCLUDE_USB_COM_CDC)
		case 1:		/* CDC Call Control Interface */
			switch (RequestNo) {
			case SET_LINE_CODING:
				return PIOS_CDC_SetLineCoding();
				break;
			case SET_CONTROL_LINE_STATE:
				return PIOS_CDC_SetControlLineState();
				break;
			}

			break;
#endif	/* PIOS_INCLUDE_USB_COM_CDC */
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
static uint8_t *PIOS_USBHOOK_GetDeviceDescriptor(uint16_t Length)
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
static uint8_t *PIOS_USBHOOK_GetConfigDescriptor(uint16_t Length)
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
static uint8_t *PIOS_USBHOOK_GetStringDescriptor(uint16_t Length)
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
static uint8_t *PIOS_USBHOOK_GetReportDescriptor(uint16_t Length)
{
	return Standard_GetDescriptorData(Length, &PIOS_HID_Report_Descriptor);
}

/*******************************************************************************
* Function Name  : PIOS_USBHOOK_GetHIDDescriptor.
* Description    : Gets the HID descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
static uint8_t *PIOS_USBHOOK_GetHIDDescriptor(uint16_t Length)
{
	return Standard_GetDescriptorData(Length, &PIOS_HID_Hid_Descriptor);
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
static uint8_t *PIOS_USBHOOK_GetProtocolValue(uint16_t Length)
{
	if (Length == 0) {
		pInformation->Ctrl_Info.Usb_wLength = 1;
		return NULL;
	} else {
		return (uint8_t *) (&ProtocolValue);
	}
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
