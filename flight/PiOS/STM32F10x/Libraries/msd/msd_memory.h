/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : memory.h
* Author             : MCD Application Team
* Version            : V3.0.1
* Date               : 04/27/2009
* Description        : Memory management layer
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __memory_H
#define __memory_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "msd.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define TXFR_IDLE     0
#define TXFR_ONGOING  1

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern void MSD_Write_Memory (uint8_t lun, uint32_t Memory_Offset, uint32_t Transfer_Length);
extern void MSD_Read_Memory (uint8_t lun, uint32_t Memory_Offset, uint32_t Transfer_Length);
extern uint16_t MSD_MAL_GetStatus (uint8_t lun);

/* Exported variables ------------------------------------------------------- */
extern uint32_t MSD_Mass_Memory_Size[MSD_NUM_LUN];
extern uint32_t MSD_Mass_Block_Size[MSD_NUM_LUN];
extern uint32_t MSD_Mass_Block_Count[MSD_NUM_LUN];

#endif /* __memory_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

