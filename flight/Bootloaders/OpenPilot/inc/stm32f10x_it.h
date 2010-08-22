/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : stm32f10x_it.h
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : This file contains the headers of the interrupt handlers.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_IT_H
#define __STM32F10x_IT_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#ifndef STM32F10X_CL
void USB_LP_CAN1_RX0_IRQHandler(void);
#endif /* STM32F10X_CL */

void DMA1_Channel1_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);

#ifdef STM32F10X_CL
void OTG_FS_IRQHandler(void);
#endif /* STM32F10X_CL */

#endif /* __STM32F10x_IT_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
