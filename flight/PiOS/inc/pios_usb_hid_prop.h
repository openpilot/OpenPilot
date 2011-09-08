/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : usb_prop.h
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : All processings related to Custom HID demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_PROP_H
#define __USB_PROP_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum _HID_REQUESTS {
	GET_REPORT = 1,
	GET_IDLE,
	GET_PROTOCOL,

	SET_REPORT = 9,
	SET_IDLE,
	SET_PROTOCOL
} HID_REQUESTS;

typedef enum CDC_REQUESTS {
	SET_LINE_CODING = 0x20,
	GET_LINE_CODING = 0x21,
	SET_CONTROL_LINE_STATE = 0x23,
} CDC_REQUESTS;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void PIOS_HID_init(void);
void PIOS_HID_Reset(void);
void PIOS_HID_SetConfiguration(void);
void PIOS_HID_SetDeviceAddress(void);
void PIOS_HID_Status_In(void);
void PIOS_HID_Status_Out(void);
RESULT PIOS_HID_Data_Setup(uint8_t);
RESULT PIOS_HID_NoData_Setup(uint8_t);
RESULT PIOS_HID_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *PIOS_HID_GetDeviceDescriptor(uint16_t);
uint8_t *PIOS_HID_GetConfigDescriptor(uint16_t);
uint8_t *PIOS_HID_GetStringDescriptor(uint16_t);
RESULT PIOS_HID_SetProtocol(void);
uint8_t *PIOS_HID_GetProtocolValue(uint16_t Length);
RESULT PIOS_HID_SetProtocol(void);
uint8_t *PIOS_HID_GetReportDescriptor(uint16_t Length);
uint8_t *PIOS_HID_GetHIDDescriptor(uint16_t Length);

/* Exported define -----------------------------------------------------------*/
#define PIOS_HID_GetConfiguration          NOP_Process
//#define PIOS_HID_SetConfiguration          NOP_Process
#define PIOS_HID_GetInterface              NOP_Process
#define PIOS_HID_SetInterface              NOP_Process
#define PIOS_HID_GetStatus                 NOP_Process
#define PIOS_HID_ClearFeature              NOP_Process
#define PIOS_HID_SetEndPointFeature        NOP_Process
#define PIOS_HID_SetDeviceFeature          NOP_Process
//#define PIOS_HID_SetDeviceAddress          NOP_Process

#define REPORT_DESCRIPTOR                  0x22

#endif /* __USB_PROP_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
