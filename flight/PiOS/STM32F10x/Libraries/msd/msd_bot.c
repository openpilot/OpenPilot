/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : usb_bot.c
* Author             : MCD Application Team
* Version            : V3.0.1
* Date               : 04/27/2009
* Description        : BOT State Machine management
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
#include "msd_scsi.h"
#include "msd_bot.h"
#include "msd_memory.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/
uint8_t MSD_Bot_State;
uint8_t MSD_Bulk_Data_Buff[MSD_BULK_MAX_PACKET_SIZE];  /* data buffer*/
uint16_t MSD_Data_Len;
Bulk_Only_CBW MSD_CBW;
Bulk_Only_CSW MSD_CSW;

/* Private variables ---------------------------------------------------------*/
static uint32_t SCSI_LBA , SCSI_BlkLen;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : MSD_Mass_Storage_In
* Description    : Mass Storage IN transfer.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_Mass_Storage_In (void)
{
  switch (MSD_Bot_State)
  {
    case BOT_CSW_Send:
    case BOT_ERROR:
      MSD_Bot_State = BOT_IDLE;
      SetEPRxStatus(ENDP2, EP_RX_VALID);/* enable the Endpoint to recive the next cmd*/
      break;
    case BOT_DATA_IN:
      switch (MSD_CBW.CB[0])
      {
        case SCSI_READ10:
          MSD_SCSI_Read10_Cmd(MSD_CBW.bLUN , SCSI_LBA , SCSI_BlkLen);
          break;
      }
      break;
    case BOT_DATA_IN_LAST:
      MSD_Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
      SetEPRxStatus(ENDP2, EP_RX_VALID);
      break;

    default:
      break;
  }
}

/*******************************************************************************
* Function Name  : MSD_Mass_Storage_Out
* Description    : Mass Storage OUT transfer.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_Mass_Storage_Out (void)
{
  uint8_t CMD;
  CMD = MSD_CBW.CB[0];
  MSD_Data_Len = GetEPRxCount(ENDP2);

  PMAToUserBufferCopy(MSD_Bulk_Data_Buff, MSD_ENDP2_RXADDR, MSD_Data_Len);

  switch (MSD_Bot_State)
  {
    case BOT_IDLE:
      MSD_CBW_Decode();
      break;
    case BOT_DATA_OUT:
      if (CMD == SCSI_WRITE10)
      {
        MSD_SCSI_Write10_Cmd(MSD_CBW.bLUN , SCSI_LBA , SCSI_BlkLen);
        break;
      }
      MSD_Bot_Abort(DIR_OUT);
      MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      MSD_Set_CSW (CSW_PHASE_ERROR, SEND_CSW_DISABLE);
      break;
    default:
      MSD_Bot_Abort(BOTH_DIR);
      MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      MSD_Set_CSW (CSW_PHASE_ERROR, SEND_CSW_DISABLE);
      break;
  }
}

/*******************************************************************************
* Function Name  : MSD_CBW_Decode
* Description    : Decode the received CBW and call the related SCSI command
*                 routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_CBW_Decode(void)
{
  uint32_t Counter;

  for (Counter = 0; Counter < MSD_Data_Len; Counter++)
  {
    *((uint8_t *)&MSD_CBW + Counter) = MSD_Bulk_Data_Buff[Counter];
  }
  MSD_CSW.dTag = MSD_CBW.dTag;
  MSD_CSW.dDataResidue = MSD_CBW.dDataLength;
  if (MSD_Data_Len != BOT_CBW_PACKET_LENGTH)
  {
    MSD_Bot_Abort(BOTH_DIR);
    /* reset the MSD_CBW.dSignature to desible the clear feature until receiving a Mass storage reset*/
    MSD_CBW.dSignature = 0;
    MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, PARAMETER_LIST_LENGTH_ERROR);
    MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
    return;
  }

  if ((MSD_CBW.CB[0] == SCSI_READ10 ) || (MSD_CBW.CB[0] == SCSI_WRITE10 ))
  {
    /* Calculate Logical Block Address */
    SCSI_LBA = (MSD_CBW.CB[2] << 24) | (MSD_CBW.CB[3] << 16) | (MSD_CBW.CB[4] <<  8) | MSD_CBW.CB[5];
    /* Calculate the Number of Blocks to transfer */
    SCSI_BlkLen = (MSD_CBW.CB[7] <<  8) | MSD_CBW.CB[8];
  }

  if (MSD_CBW.dSignature == BOT_CBW_SIGNATURE)
  {
    /* Valid CBW */
    if ((MSD_CBW.bLUN >= MSD_NUM_LUN) || (MSD_CBW.bCBLength < 1) || (MSD_CBW.bCBLength > 16))
    {
      MSD_Bot_Abort(BOTH_DIR);
      MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
    }
    else
    {
      switch (MSD_CBW.CB[0])
      {
        case SCSI_REQUEST_SENSE:
          MSD_SCSI_RequestSense_Cmd (MSD_CBW.bLUN);
          break;
        case SCSI_INQUIRY:
          MSD_SCSI_Inquiry_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_START_STOP_UNIT:
          MSD_SCSI_Start_Stop_Unit_Cmd(MSD_CBW.bLUN);
          MSD_LUN_AvailableSet(MSD_CBW.bLUN, 0);
          break;
        case SCSI_ALLOW_MEDIUM_REMOVAL:
          MSD_SCSI_Start_Stop_Unit_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_MODE_SENSE6:
          MSD_SCSI_ModeSense6_Cmd (MSD_CBW.bLUN);
          break;
        case SCSI_MODE_SENSE10:
          MSD_SCSI_ModeSense10_Cmd (MSD_CBW.bLUN);
          break;
        case SCSI_READ_FORMAT_CAPACITIES:
          MSD_SCSI_ReadFormatCapacity_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_READ_CAPACITY10:
          MSD_SCSI_ReadCapacity10_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_TEST_UNIT_READY:
          MSD_SCSI_TestUnitReady_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_READ10:
          MSD_SCSI_Read10_Cmd(MSD_CBW.bLUN, SCSI_LBA , SCSI_BlkLen);
          break;
        case SCSI_WRITE10:
          MSD_SCSI_Write10_Cmd(MSD_CBW.bLUN, SCSI_LBA , SCSI_BlkLen);
          break;
        case SCSI_VERIFY10:
          MSD_SCSI_Verify10_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_FORMAT_UNIT:
          MSD_SCSI_Format_Cmd(MSD_CBW.bLUN);
          break;
          /*Unsupported command*/

        case SCSI_MODE_SELECT10:
          MSD_SCSI_Mode_Select10_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_MODE_SELECT6:
          MSD_SCSI_Mode_Select6_Cmd(MSD_CBW.bLUN);
          break;

        case SCSI_SEND_DIAGNOSTIC:
          MSD_SCSI_Send_Diagnostic_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_READ6:
          MSD_SCSI_Read6_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_READ12:
          MSD_SCSI_Read12_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_READ16:
          MSD_SCSI_Read16_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_READ_CAPACITY16:
          MSD_SCSI_READ_CAPACITY16_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_WRITE6:
          MSD_SCSI_Write6_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_WRITE12:
          MSD_SCSI_Write12_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_WRITE16:
          MSD_SCSI_Write16_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_VERIFY12:
          MSD_SCSI_Verify12_Cmd(MSD_CBW.bLUN);
          break;
        case SCSI_VERIFY16:
          MSD_SCSI_Verify16_Cmd(MSD_CBW.bLUN);
          break;

        default:
        {
          MSD_Bot_Abort(BOTH_DIR);
          MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
          MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
        }
      }
    }
  }
  else
  {
    /* Invalid CBW */
    MSD_Bot_Abort(BOTH_DIR);
    MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
    MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
  }
}

/*******************************************************************************
* Function Name  : MSD_Transfer_Data_Request
* Description    : Send the request response to the PC HOST.
* Input          : uint8_t* Data_Address : point to the data to transfer.
*                  uint16_t Data_Length : the nember of Bytes to transfer.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_Transfer_Data_Request(uint8_t* Data_Pointer, uint16_t Data_Len)
{
  UserToPMABufferCopy(Data_Pointer, MSD_ENDP1_TXADDR, Data_Len);

  SetEPTxCount(ENDP1, Data_Len);
  SetEPTxStatus(ENDP1, EP_TX_VALID);
  MSD_Bot_State = BOT_DATA_IN_LAST;
  MSD_CSW.dDataResidue -= Data_Len;
  MSD_CSW.bStatus = CSW_CMD_PASSED;
}

/*******************************************************************************
* Function Name  : MSD_Set_CSW
* Description    : Set the SCW with the needed fields.
* Input          : uint8_t CSW_Status this filed can be CSW_CMD_PASSED,CSW_CMD_FAILED,
*                  or CSW_PHASE_ERROR.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_Set_CSW (uint8_t CSW_Status, uint8_t Send_Permission)
{
  MSD_CSW.dSignature = BOT_CSW_SIGNATURE;
  MSD_CSW.bStatus = CSW_Status;

  UserToPMABufferCopy(((uint8_t *)& MSD_CSW), MSD_ENDP1_TXADDR, CSW_DATA_LENGTH);

  SetEPTxCount(ENDP1, CSW_DATA_LENGTH);
  MSD_Bot_State = BOT_ERROR;
  if (Send_Permission)
  {
    MSD_Bot_State = BOT_CSW_Send;
    SetEPTxStatus(ENDP1, EP_TX_VALID);
  }

}

/*******************************************************************************
* Function Name  : MSD_Bot_Abort
* Description    : Stall the needed Endpoint according to the selected direction.
* Input          : Endpoint direction IN, OUT or both directions
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_Bot_Abort(uint8_t Direction)
{
  switch (Direction)
  {
    case DIR_IN :
      SetEPTxStatus(ENDP1, EP_TX_STALL);
      break;
    case DIR_OUT :
      SetEPRxStatus(ENDP2, EP_RX_STALL);
      break;
    case BOTH_DIR :
      SetEPTxStatus(ENDP1, EP_TX_STALL);
      SetEPRxStatus(ENDP2, EP_RX_STALL);
      break;
    default:
      break;
  }
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

