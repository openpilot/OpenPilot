/**
  ******************************************************************************
  * @file    stm32f2xx_syscfg.c
  * @author  MCD Application Team
  * @version V1.0.0RC1
  * @date    18-March-2011
  * @brief   This file provides firmware functions to manage the SYSCFG peripheral.
  *
  *  @verbatim
  *  
  *          ===================================================================
  *                                 How to use this driver
  *          ===================================================================
  *                  
  *          This driver provides functions for:
  *          
  *          1. Remapping the memory accessible in the code area using SYSCFG_MemoryRemapConfig()
  *              
  *          2. Manage the EXTI lines connection to the GPIOs using SYSCFG_EXTILineConfig()
  *            
  *          3. Select the ETHERNET media interface (RMII/RII) using SYSCFG_ETH_MediaInterfaceConfig()
  *
  *  @note  SYSCFG APB clock must be enabled to get write access to SYSCFG registers,
  *         using RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  *                 
  *  @endverbatim
  *      
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_syscfg.h"
#include "stm32f2xx_rcc.h"

/** @addtogroup STM32F2xx_StdPeriph_Driver
  * @{
  */

/** @defgroup SYSCFG 
  * @brief SYSCFG driver modules
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* ------------ RCC registers bit address in the alias region ----------- */
#define SYSCFG_OFFSET                 (SYSCFG_BASE - PERIPH_BASE)
/* ---  PMC Register ---*/ 
/* Alias word address of MII_RMII_SEL bit */ 
#define PMC_OFFSET                  (SYSCFG_OFFSET + 0x04) 
#define MII_RMII_SEL_BitNumber      ((uint8_t)0x17) 
#define PMC_MII_RMII_SEL_BB         (PERIPH_BB_BASE + (PMC_OFFSET * 32) + (MII_RMII_SEL_BitNumber * 4)) 

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup SYSCFG_Private_Functions
  * @{
  */ 

/**
  * @brief  Deinitializes the Alternate Functions (remap and EXTI configuration)
  *   registers to their default reset values.
  * @param  None
  * @retval None
  */
void SYSCFG_DeInit(void)
{
   RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE);
   RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, DISABLE);
}

/**
  * @brief  Changes the mapping of the specified pin.
  * @param  SYSCFG_Memory: selects the memory remapping.
  *         This parameter can be one of the following values:
  *            @arg SYSCFG_MemoryRemap_Flash:       Main Flash memory mapped at 0x00000000  
  *            @arg SYSCFG_MemoryRemap_SystemFlash: System Flash memory mapped at 0x00000000
  *            @arg SYSCFG_MemoryRemap_FSMC:        FSMC (Bank1 (NOR/PSRAM 1 and 2) mapped at 0x00000000
  *            @arg SYSCFG_MemoryRemap_SRAM:        Embedded SRAM (112kB) mapped at 0x00000000
  *  
  * @note   In remap mode, the FSMC addressing is fixed to the remap address area only
  *        (Bank1 NOR/PSRAM 1 and NOR/PSRAM 2) and FSMC control registers are not
  *         accessible. The FSMC remap function must be disabled to allows addressing
  *         other memory devices through the FSMC and/or to access FSMC control
  *         registers. 
  *        
  * @retval None
  */
void SYSCFG_MemoryRemapConfig(uint8_t SYSCFG_MemoryRemap)
{
  /* Check the parameters */
  assert_param(IS_SYSCFG_MEMORY_REMAP_CONFING(SYSCFG_MemoryRemap));

  SYSCFG->MEMRMP = SYSCFG_MemoryRemap;
}

/**
  * @brief  Selects the GPIO pin used as EXTI Line.
  * @param  EXTI_PortSourceGPIOx : selects the GPIO port to be used as source for
  *          EXTI lines where x can be (A..I).
  * @param  EXTI_PinSourcex: specifies the EXTI line to be configured.
  *           This parameter can be EXTI_PinSourcex where x can be (0..15, except
  *           for EXTI_PortSourceGPIOI x can be (0..11).
  * @retval None
  */
void SYSCFG_EXTILineConfig(uint8_t EXTI_PortSourceGPIOx, uint8_t EXTI_PinSourcex)
{
  uint32_t tmp = 0x00;

  /* Check the parameters */
  assert_param(IS_EXTI_PORT_SOURCE(EXTI_PortSourceGPIOx));
  assert_param(IS_EXTI_PIN_SOURCE(EXTI_PinSourcex));

  tmp = ((uint32_t)0x0F) << (0x04 * (EXTI_PinSourcex & (uint8_t)0x03));
  SYSCFG->EXTICR[EXTI_PinSourcex >> 0x02] &= ~tmp;
  SYSCFG->EXTICR[EXTI_PinSourcex >> 0x02] |= (((uint32_t)EXTI_PortSourceGPIOx) << (0x04 * (EXTI_PinSourcex & (uint8_t)0x03)));
}

/**
  * @brief  Selects the ETHERNET media interface 
  * @param  SYSCFG_ETH_MediaInterface: specifies the Media Interface mode. 
  *          This parameter can be one of the following values: 
  *            @arg SYSCFG_ETH_MediaInterface_MII: MII mode 
  *            @arg SYSCFG_ETH_MediaInterface_RMII: RMII mode 
  * @retval None 
  */
void SYSCFG_ETH_MediaInterfaceConfig(uint32_t SYSCFG_ETH_MediaInterface) 
{ 
  assert_param(IS_SYSCFG_ETH_MEDIA_INTERFACE(SYSCFG_ETH_MediaInterface)); 
  /* Configure MII_RMII selection bit */ 
  *(__IO uint32_t *) PMC_MII_RMII_SEL_BB = SYSCFG_ETH_MediaInterface; 
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/   
