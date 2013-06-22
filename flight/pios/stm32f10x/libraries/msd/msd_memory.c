/**
 ******************************************************************************
 *
 * @file       msd_memory.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      Memory Management Layer
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   MSD MSD Functions
 * @{
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : memory.c
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

/* Includes ------------------------------------------------------------------*/

#include <pios.h>
#include <usb_lib.h>

#include "msd.h"
#include "msd_memory.h"
#include "msd_scsi.h"
#include "msd_bot.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/
uint32_t MSD_Mass_Memory_Size[MSD_NUM_LUN];
uint32_t MSD_Mass_Block_Size[MSD_NUM_LUN];
uint32_t MSD_Mass_Block_Count[MSD_NUM_LUN];


/* Private variables ---------------------------------------------------------*/
static __IO uint32_t Block_Read_count = 0;
static __IO uint32_t Block_offset;
static __IO uint32_t Counter = 0;
static uint32_t  Idx;
static uint32_t Data_Buffer[MSD_BULK_MAX_PACKET_SIZE *2]; /* 512 bytes*/
static uint8_t TransferState = TXFR_IDLE;


/* Extern variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
* Function Name  : Read_Memory
* Description    : Handle the Read operation from the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_Read_Memory(uint8_t lun, uint32_t Memory_Offset, uint32_t Transfer_Length)
{
  static uint32_t Offset, Length;

  if (TransferState == TXFR_IDLE )
  {
    Offset = Memory_Offset * MSD_Mass_Block_Size[lun];
    Length = Transfer_Length * MSD_Mass_Block_Size[lun];
    TransferState = TXFR_ONGOING;
  }

  if (TransferState == TXFR_ONGOING )
  {
    if (!Block_Read_count)
    {
      s32 status;

      switch( lun ) {
        case 0:
          if( MSD_Mass_Block_Size[lun] != 512 )
            break;
          status = PIOS_SDCARD_SectorRead(Offset/512, (u8 *)Data_Buffer);
          // TK: how to handle an error here?
          break;

        default:
          status = -1;
          // TK: how to handle an error here?
      }

      UserToPMABufferCopy((uint8_t *)Data_Buffer, MSD_ENDP1_TXADDR, MSD_BULK_MAX_PACKET_SIZE);
      Block_Read_count = MSD_Mass_Block_Size[lun] - MSD_BULK_MAX_PACKET_SIZE;
      Block_offset = MSD_BULK_MAX_PACKET_SIZE;
    }
    else
    {
      UserToPMABufferCopy((uint8_t *)Data_Buffer + Block_offset, MSD_ENDP1_TXADDR, MSD_BULK_MAX_PACKET_SIZE);
      Block_Read_count -= MSD_BULK_MAX_PACKET_SIZE;
      Block_offset += MSD_BULK_MAX_PACKET_SIZE;
    }

    SetEPTxCount(ENDP1, MSD_BULK_MAX_PACKET_SIZE);
    SetEPTxStatus(ENDP1, EP_TX_VALID);
    Offset += MSD_BULK_MAX_PACKET_SIZE;
    Length -= MSD_BULK_MAX_PACKET_SIZE;

    MSD_CSW.dDataResidue -= MSD_BULK_MAX_PACKET_SIZE;
  }
  if (Length == 0)
  {
    Block_Read_count = 0;
    Block_offset = 0;
    Offset = 0;
    MSD_Bot_State = BOT_DATA_IN_LAST;
    TransferState = TXFR_IDLE;
  }
}

/*******************************************************************************
* Function Name  : Write_Memory
* Description    : Handle the Write operation to the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_Write_Memory (uint8_t lun, uint32_t Memory_Offset, uint32_t Transfer_Length)
{

  static uint32_t W_Offset, W_Length;

  uint32_t temp =  Counter + 64;

  if (TransferState == TXFR_IDLE )
  {
    W_Offset = Memory_Offset * MSD_Mass_Block_Size[lun];
    W_Length = Transfer_Length * MSD_Mass_Block_Size[lun];
    TransferState = TXFR_ONGOING;
  }

  if (TransferState == TXFR_ONGOING )
  {

    for (Idx = 0 ; Counter < temp; Counter++)
    {
      *((uint8_t *)Data_Buffer + Counter) = MSD_Bulk_Data_Buff[Idx++];
    }

    W_Offset += MSD_Data_Len;
    W_Length -= MSD_Data_Len;

    if (!(W_Length % MSD_Mass_Block_Size[lun]))
    {
      Counter = 0;

      s32 status;
      u32 Offset = W_Offset - MSD_Mass_Block_Size[lun];

      switch( lun ) {
        case 0:
          if( MSD_Mass_Block_Size[lun] != 512 )
            break;
          status = PIOS_SDCARD_SectorWrite(Offset/512, (u8 *)Data_Buffer);
          // TK: how to handle an error here?
          break;

        default:
          status = -1;
          // TK: how to handle an error here?
      }
    }

    MSD_CSW.dDataResidue -= MSD_Data_Len;
    SetEPRxStatus(ENDP2, EP_RX_VALID); /* enable the next transaction*/
  }

  if ((W_Length == 0) || (MSD_Bot_State == BOT_CSW_Send))
  {
    Counter = 0;
    MSD_Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
    TransferState = TXFR_IDLE;
  }
}


/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MSD_MAL_GetStatus (uint8_t lun)
{
  // LUN removed or disabled?
  if( !MSD_LUN_AvailableGet(lun) )
    return 1;

  if( lun == 0 ) {
	  SDCARDCsdTypeDef csd;
    if( PIOS_SDCARD_CSDRead(&csd) < 0 )
      return 1;

    u32 DeviceSizeMul = csd.DeviceSizeMul + 2;
    u32 temp_block_mul = (1 << csd.RdBlockLen)/ 512;
    MSD_Mass_Block_Count[lun] = ((csd.DeviceSize + 1) * (1 << (DeviceSizeMul))) * temp_block_mul;
    MSD_Mass_Block_Size[lun] = 512;
    MSD_Mass_Memory_Size[lun] = (MSD_Mass_Block_Count[lun] * MSD_Mass_Block_Size[0]);

    return 0;
  }

  return 1;
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

