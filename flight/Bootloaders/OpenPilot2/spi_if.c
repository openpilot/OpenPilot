/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : spi_if.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : specific media access Layer for SPI flash
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "spi_flash.h"
#include "spi_if.h"
#include "dfu_mal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : SPI_If_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t SPI_If_Init(void)
{
  SPI_FLASH_Init();
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : SPI_If_Erase
* Description    : Erase sector
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t SPI_If_Erase(uint32_t SectorAddress)
{
  SPI_FLASH_SectorErase(SectorAddress);
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : SPI_If_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t SPI_If_Write(uint32_t SectorAddress, uint32_t DataLength)
{
  uint32_t idx, pages;

  pages = (((DataLength & 0xFF00)) >> 8);

  if  (DataLength & 0xFF) /* Not a 256 aligned data */
  {
    for ( idx = DataLength; idx < ((DataLength & 0xFF00) + 0x100) ; idx++)
    {
      MAL_Buffer[idx] = 0xFF;
    }
    pages = (((DataLength & 0xFF00)) >> 8 ) + 1;
  }

  for (idx = 0; idx < pages; idx++)
  {
    SPI_FLASH_PageWrite(&MAL_Buffer[idx*256], SectorAddress, 256);
    SectorAddress += 0x100;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : SPI_If_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : buffer address pointer
*******************************************************************************/
uint8_t *SPI_If_Read(uint32_t SectorAddress, uint32_t DataLength)
{
  SPI_FLASH_BufferRead(MAL_Buffer, SectorAddress, (uint16_t)DataLength);
  return MAL_Buffer;
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
