/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : usb_desc.h
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : Descriptor Header for Device Firmware Upgrade (DFU)
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
#include "platform_config.h"
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DFU_SIZ_DEVICE_DESC            18

#ifdef USE_STM3210B_EVAL
  #define DFU_SIZ_CONFIG_DESC          36
#elif defined(USE_STM3210C_EVAL)
  #define DFU_SIZ_CONFIG_DESC          27
#elif defined(USE_STM3210E_EVAL)
  #define DFU_SIZ_CONFIG_DESC          45
#endif /* USE_STM3210B_EVAL */

#define DFU_SIZ_STRING_LANGID           4
#define DFU_SIZ_STRING_VENDOR           38
#define DFU_SIZ_STRING_PRODUCT          20
#define DFU_SIZ_STRING_SERIAL           26
#define DFU_SIZ_STRING_INTERFACE0       96    /* Flash Bank 0 */

#define DFU_SIZ_STRING_INTERFACE1       98     /* SPI Flash : M25P64*/
#define DFU_SIZ_STRING_INTERFACE2       106    /* NOR Flash : M26M128*/

extern  uint8_t DFU_DeviceDescriptor[DFU_SIZ_DEVICE_DESC];
extern  uint8_t DFU_ConfigDescriptor[DFU_SIZ_CONFIG_DESC];
extern  uint8_t DFU_StringLangId     [DFU_SIZ_STRING_LANGID];
extern  uint8_t DFU_StringVendor     [DFU_SIZ_STRING_VENDOR];
extern  uint8_t DFU_StringProduct    [DFU_SIZ_STRING_PRODUCT];
extern  uint8_t DFU_StringSerial     [DFU_SIZ_STRING_SERIAL];
extern  uint8_t DFU_StringInterface0 [DFU_SIZ_STRING_INTERFACE0];
extern  uint8_t DFU_StringInterface1 [DFU_SIZ_STRING_INTERFACE1];
extern  uint8_t DFU_StringInterface2_1 [DFU_SIZ_STRING_INTERFACE2];
extern  uint8_t DFU_StringInterface2_2 [DFU_SIZ_STRING_INTERFACE2];
extern  uint8_t DFU_StringInterface2_3 [DFU_SIZ_STRING_INTERFACE2];

#define bMaxPacketSize0             0x40     /* bMaxPacketSize0 = 64 bytes   */
#define wTransferSize               0x0400   /* wTransferSize   = 1024 bytes */
/* bMaxPacketSize0 <= wTransferSize <= 32kbytes */
#define wTransferSizeB0             0x00
#define wTransferSizeB1             0x04
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* External variables --------------------------------------------------------*/

#endif /* __USB_DESC_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/




