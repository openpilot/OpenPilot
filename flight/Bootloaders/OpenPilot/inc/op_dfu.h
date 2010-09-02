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
#ifndef __OP_DFU_H
#define __OP_DFU_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct
{
	uint8_t programmingType;
	uint8_t readWriteFlags;
	uint32_t startOfUserCode;
	uint32_t sizeOfCode;
	uint8_t sizeOfDescription;
	uint8_t sizeOfHash;
	uint8_t devID;
}Device;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define COMMAND	0
#define COUNT	1
#define DATA	5

/* Exported functions ------------------------------------------------------- */
void processComand(uint8_t *Receive_Buffer);
uint32_t baseOfAdressType(uint8_t type);
uint8_t isBiggerThanAvailable(uint8_t type, uint32_t size);
void OPDfuIni(void);
void DataDownload();
#endif /* __OP_DFU_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
