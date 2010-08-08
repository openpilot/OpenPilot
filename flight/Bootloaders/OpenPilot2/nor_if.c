/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : nor_if.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : specific media access Layer for NOR flash
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
#include "platform_config.h"

#ifdef USE_STM3210E_EVAL

/* Includes ------------------------------------------------------------------*/
#include "fsmc_nor.h"
#include "nor_if.h"
#include "dfu_mal.h"
#include "stm32f10x_fsmc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern NOR_IDTypeDef NOR_ID;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : NOR_If_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t NOR_If_Init(void)
{
  /* Configure FSMC Bank1 NOR/SRAM2 */
  FSMC_NOR_Init();

  /* Enable FSMC Bank1 NOR/SRAM2 */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM2, ENABLE);

  return MAL_OK;
}

/*******************************************************************************
* Function Name  : NOR_If_Erase
* Description    : Erase sector
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t NOR_If_Erase(uint32_t Address)
{
  /* Erase the destination memory */
  FSMC_NOR_EraseBlock(Address & 0x00FFFFFF);    

  return MAL_OK;
}

/*******************************************************************************
* Function Name  : NOR_If_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t NOR_If_Write(uint32_t Address, uint32_t DataLength)
{
  if ((DataLength & 1) == 1) /* Not an aligned data */
  {
    DataLength += 1;
    MAL_Buffer[DataLength-1] = 0xFF;
  }
  
  FSMC_NOR_WriteBuffer((uint16_t *)MAL_Buffer, (Address&0x00FFFFFF), DataLength >> 1);  
  
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : NOR_If_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : buffer address pointer
*******************************************************************************/
uint8_t *NOR_If_Read(uint32_t Address, uint32_t DataLength)
{
  return  (uint8_t*)(Address);
}

#endif

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
