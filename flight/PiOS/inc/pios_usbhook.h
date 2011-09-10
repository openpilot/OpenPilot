/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : pios_usbhook.h
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
#ifndef PIOS_USBHOOK_H
#define PIOS_USBHOOK_H

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
/* Exported define -----------------------------------------------------------*/
#define REPORT_DESCRIPTOR                  0x22

#endif /* PIOS_USBHOOK_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
