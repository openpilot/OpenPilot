/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : usb_desc.h
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : Descriptor Header for Custom HID Demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DESC_H
#define __USB_DESC_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define HID_DESCRIPTOR_TYPE                     0x21
#define PIOS_HID_SIZ_HID_DESC                  0x09
#define PIOS_HID_OFF_HID_DESC                  0x12

#define PIOS_HID_SIZ_DEVICE_DESC               18
#define PIOS_HID_SIZ_CONFIG_DESC               41
#define PIOS_HID_SIZ_REPORT_DESC               36
#define PIOS_HID_SIZ_STRING_LANGID             4
#define PIOS_HID_SIZ_STRING_VENDOR             28
#define PIOS_HID_SIZ_STRING_PRODUCT            28
#define PIOS_HID_SIZ_STRING_SERIAL             52 /* 96 bits, 12 bytes, 24 characters, 48 in unicode */

#define STANDARD_ENDPOINT_DESC_SIZE             0x09

/* Exported functions ------------------------------------------------------- */
extern const uint8_t PIOS_HID_DeviceDescriptor[PIOS_HID_SIZ_DEVICE_DESC];
extern const uint8_t PIOS_HID_ConfigDescriptor[PIOS_HID_SIZ_CONFIG_DESC];
extern const uint8_t PIOS_HID_ReportDescriptor[PIOS_HID_SIZ_REPORT_DESC];
extern const uint8_t PIOS_HID_StringLangID[PIOS_HID_SIZ_STRING_LANGID];
extern const uint8_t PIOS_HID_StringVendor[PIOS_HID_SIZ_STRING_VENDOR];
extern const uint8_t PIOS_HID_StringProduct[PIOS_HID_SIZ_STRING_PRODUCT];
extern uint8_t PIOS_HID_StringSerial[PIOS_HID_SIZ_STRING_SERIAL];

#endif /* __USB_DESC_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
