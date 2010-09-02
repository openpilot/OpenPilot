/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
 * File Name          : usb_endp.c
 * Author             : MCD Application Team
 * Version            : V3.2.1
 * Date               : 07/05/2010
 * Description        : Endpoint routines
 ********************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"
#include "stm32f10x.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include "stm32_eval.h"
#include "stm32f10x_flash.h"
#include "common.h"
#include "hw_config.h"
#include <string.h>
#include "op_dfu.h"
//extern uint32_t baseOfAdressType(DFUTransfer type);
//extern uint8_t* SendBuffer;
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//uint32_t downPacketCurrent = 0;
//uint8_t downType = 0;
//uint32_t downSizeOfLastPacket = 0;
//uint32_t downPacketTotal = 0;
//extern DFUStates DeviceState;
//extern uint32_t Aditionals;
uint8_t FLASH_Ini() {
	FLASH_Unlock();
	return 1;
}

uint8_t *FLASH_If_Read(uint32_t SectorAddress, uint32_t DataLength) {
	return (uint8_t*) (SectorAddress);
}
uint8_t FLASH_Start(uint32_t size) {
	FLASH_ErasePage(0x08008800);
	uint32_t pageAdress;
	pageAdress = StartOfUserCode;
	uint8_t fail = FALSE;
	while ((pageAdress < StartOfUserCode + size) || (fail == TRUE)) {
		for (int retry = 0; retry < MAX_DEL_RETRYS; ++retry) {
			if (FLASH_ErasePage(pageAdress) == FLASH_COMPLETE) {
				fail = FALSE;
				break;
			} else {
				fail = TRUE;
			}

		}

#ifdef STM32F10X_HD
		pageAdress += 2048;
#endif
#ifdef STM32F10X_MD
		pageAdress += 1024;
#endif
	}
	if (fail == FALSE) {
#ifdef STM32F10X_HD
		pageAdress = StartOfUserCode + SizeOfHash + SizeOfDescription
				+ SizeOfCode - 2048;
#endif
#ifdef STM32F10X_MD
		pageAdress = StartOfUserCode+SizeOfHash+SizeOfDescription+SizeOfCode-1024;
#endif
		for (int retry = 0; retry < MAX_DEL_RETRYS; ++retry) {
			if (FLASH_ErasePage(pageAdress) == FLASH_COMPLETE) {
				fail = FALSE;
				break;
			} else
				fail = TRUE;
		}
	}
	return (fail == TRUE) ? 0 : 1;
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

