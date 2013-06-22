/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : usb_desc.h
* Author             : MCD Application Team
* Version            : V3.0.1
* Date               : 04/27/2009
* Description        : Descriptor Header for Mass Storage Device
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
#include "stm32f10x.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define MSD_MASS_SIZ_DEVICE_DESC              18
#define MSD_MASS_SIZ_CONFIG_DESC              32

#define MSD_MASS_SIZ_STRING_LANGID            4
#define MSD_MASS_SIZ_STRING_VENDOR            20
#define MSD_MASS_SIZ_STRING_PRODUCT           46
#define MSD_MASS_SIZ_STRING_SERIAL            26
#define MSD_MASS_SIZ_STRING_INTERFACE         16

/* Exported functions ------------------------------------------------------- */
extern const uint8_t MSD_MASS_DeviceDescriptor[MSD_MASS_SIZ_DEVICE_DESC];
extern const uint8_t MSD_MASS_ConfigDescriptor[MSD_MASS_SIZ_CONFIG_DESC];

extern const uint8_t MSD_MASS_StringLangID[MSD_MASS_SIZ_STRING_LANGID];
extern const uint8_t MSD_MASS_StringVendor[MSD_MASS_SIZ_STRING_VENDOR];
extern const uint8_t MSD_MASS_StringProduct[MSD_MASS_SIZ_STRING_PRODUCT];
extern uint8_t MSD_MASS_StringSerial[MSD_MASS_SIZ_STRING_SERIAL];
extern const uint8_t MSD_MASS_StringInterface[MSD_MASS_SIZ_STRING_INTERFACE];

#endif /* __USB_DESC_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

