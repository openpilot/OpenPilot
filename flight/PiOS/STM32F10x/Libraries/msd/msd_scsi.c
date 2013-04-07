/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : usb_scsi.c
* Author             : MCD Application Team
* Version            : V3.0.1
* Date               : 04/27/2009
* Description        : All processing related to the SCSI commands
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

#include <string.h>

#include "msd.h"
#include "msd_scsi.h"
#include "msd_bot.h"
#include "msd_memory.h"
//#include "nand_if.h"
//#include "platform_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : SCSI_Inquiry_Cmd
* Description    : SCSI Inquiry Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_Inquiry_Cmd(uint8_t lun)
{
  uint8_t* Inquiry_Data;
  uint16_t Inquiry_Data_Length;

  if (MSD_CBW.CB[1] & 0x01)/*Evpd is set*/
  {
    Inquiry_Data = MSD_Page00_Inquiry_Data;
    Inquiry_Data_Length = 5;
  }
  else
  {

    if ( lun == 0)
    {
      Inquiry_Data = MSD_Standard_Inquiry_Data;
    }
    else
    {
      Inquiry_Data = MSD_Standard_Inquiry_Data2;
    }

    if (MSD_CBW.CB[4] <= STANDARD_INQUIRY_DATA_LEN)
      Inquiry_Data_Length = MSD_CBW.CB[4];
    else
      Inquiry_Data_Length = STANDARD_INQUIRY_DATA_LEN;

  }
  MSD_Transfer_Data_Request(Inquiry_Data, Inquiry_Data_Length);
}

/*******************************************************************************
* Function Name  : MSD_SCSI_ReadFormatCapacity_Cmd
* Description    : SCSI ReadFormatCapacity Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_ReadFormatCapacity_Cmd(uint8_t lun)
{

  if (MSD_MAL_GetStatus(lun) != 0 )
  {
    MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    MSD_Bot_Abort(DIR_IN);
    return;
  }
  MSD_ReadFormatCapacity_Data[4] = (uint8_t)(MSD_Mass_Block_Count[lun] >> 24);
  MSD_ReadFormatCapacity_Data[5] = (uint8_t)(MSD_Mass_Block_Count[lun] >> 16);
  MSD_ReadFormatCapacity_Data[6] = (uint8_t)(MSD_Mass_Block_Count[lun] >>  8);
  MSD_ReadFormatCapacity_Data[7] = (uint8_t)(MSD_Mass_Block_Count[lun]);

  MSD_ReadFormatCapacity_Data[9] = (uint8_t)(MSD_Mass_Block_Size[lun] >>  16);
  MSD_ReadFormatCapacity_Data[10] = (uint8_t)(MSD_Mass_Block_Size[lun] >>  8);
  MSD_ReadFormatCapacity_Data[11] = (uint8_t)(MSD_Mass_Block_Size[lun]);
  MSD_Transfer_Data_Request(MSD_ReadFormatCapacity_Data, READ_FORMAT_CAPACITY_DATA_LEN);
}

/*******************************************************************************
* Function Name  : MSD_SCSI_ReadCapacity10_Cmd
* Description    : SCSI ReadCapacity10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_ReadCapacity10_Cmd(uint8_t lun)
{

  if (MSD_MAL_GetStatus(lun))
  {
    MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    MSD_Bot_Abort(DIR_IN);
    return;
  }

  MSD_ReadCapacity10_Data[0] = (uint8_t)((MSD_Mass_Block_Count[lun] - 1) >> 24);
  MSD_ReadCapacity10_Data[1] = (uint8_t)((MSD_Mass_Block_Count[lun] - 1) >> 16);
  MSD_ReadCapacity10_Data[2] = (uint8_t)((MSD_Mass_Block_Count[lun] - 1) >>  8);
  MSD_ReadCapacity10_Data[3] = (uint8_t)((MSD_Mass_Block_Count[lun] - 1));

  MSD_ReadCapacity10_Data[4] = (uint8_t)(MSD_Mass_Block_Size[lun] >>  24);
  MSD_ReadCapacity10_Data[5] = (uint8_t)(MSD_Mass_Block_Size[lun] >>  16);
  MSD_ReadCapacity10_Data[6] = (uint8_t)(MSD_Mass_Block_Size[lun] >>  8);
  MSD_ReadCapacity10_Data[7] = (uint8_t)(MSD_Mass_Block_Size[lun]);
  MSD_Transfer_Data_Request(MSD_ReadCapacity10_Data, READ_CAPACITY10_DATA_LEN);
}

/*******************************************************************************
* Function Name  : MSD_SCSI_ModeSense6_Cmd
* Description    : SCSI ModeSense6 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_ModeSense6_Cmd (uint8_t lun)
{
  MSD_Transfer_Data_Request(MSD_Mode_Sense6_data, MODE_SENSE6_DATA_LEN);
}

/*******************************************************************************
* Function Name  : MSD_SCSI_ModeSense10_Cmd
* Description    : SCSI ModeSense10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_ModeSense10_Cmd (uint8_t lun)
{
  MSD_Transfer_Data_Request(MSD_Mode_Sense10_data, MODE_SENSE10_DATA_LEN);
}

/*******************************************************************************
* Function Name  : MSD_SCSI_RequestSense_Cmd
* Description    : SCSI RequestSense Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_RequestSense_Cmd (uint8_t lun)
{
  uint8_t Request_Sense_data_Length;

  if (MSD_CBW.CB[4] <= REQUEST_SENSE_DATA_LEN)
  {
    Request_Sense_data_Length = MSD_CBW.CB[4];
  }
  else
  {
    Request_Sense_data_Length = REQUEST_SENSE_DATA_LEN;
  }
  MSD_Transfer_Data_Request(MSD_Scsi_Sense_Data, Request_Sense_data_Length);
}

/*******************************************************************************
* Function Name  : MSD_Set_Scsi_Sense_Data
* Description    : Set Scsi Sense Data routine.
* Input          : uint8_t Sens_Key
                   uint8_t Asc.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_Set_Scsi_Sense_Data(uint8_t lun, uint8_t Sens_Key, uint8_t Asc)
{
  MSD_Scsi_Sense_Data[2] = Sens_Key;
  MSD_Scsi_Sense_Data[12] = Asc;
}

/*******************************************************************************
* Function Name  : MSD_SCSI_Start_Stop_Unit_Cmd
* Description    : SCSI Start_Stop_Unit Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_Start_Stop_Unit_Cmd(uint8_t lun)
{
  MSD_Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
}

/*******************************************************************************
* Function Name  : MSD_SCSI_Read10_Cmd
* Description    : SCSI Read10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_Read10_Cmd(uint8_t lun , uint32_t LBA , uint32_t BlockNbr)
{

  if (MSD_Bot_State == BOT_IDLE)
  {
    if (!(MSD_SCSI_Address_Management(MSD_CBW.bLUN, SCSI_READ10, LBA, BlockNbr)))/*address out of range*/
    {
      return;
    }

    if ((MSD_CBW.bmFlags & 0x80) != 0)
    {
      MSD_Bot_State = BOT_DATA_IN;
      MSD_Read_Memory(lun, LBA , BlockNbr);
    }
    else
    {
      MSD_Bot_Abort(BOTH_DIR);
      MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    }
    return;
  }
  else if (MSD_Bot_State == BOT_DATA_IN)
  {
    MSD_Read_Memory(lun , LBA , BlockNbr);
  }
}

/*******************************************************************************
* Function Name  : MSD_SCSI_Write10_Cmd
* Description    : SCSI Write10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_Write10_Cmd(uint8_t lun , uint32_t LBA , uint32_t BlockNbr)
{

  if (MSD_Bot_State == BOT_IDLE)
  {
    if (!(MSD_SCSI_Address_Management(MSD_CBW.bLUN, SCSI_WRITE10 , LBA, BlockNbr)))/*address out of range*/
    {
      return;
    }

    if ((MSD_CBW.bmFlags & 0x80) == 0)
    {
      MSD_Bot_State = BOT_DATA_OUT;
      SetEPRxStatus(ENDP2, EP_RX_VALID);
    }
    else
    {
      MSD_Bot_Abort(DIR_IN);
      MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
    }
    return;
  }
  else if (MSD_Bot_State == BOT_DATA_OUT)
  {
    MSD_Write_Memory(lun , LBA , BlockNbr);
  }
}

/*******************************************************************************
* Function Name  : MSD_SCSI_Verify10_Cmd
* Description    : SCSI Verify10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_Verify10_Cmd(uint8_t lun)
{
  if ((MSD_CBW.dDataLength == 0) && !(MSD_CBW.CB[1] & BLKVFY))/* BLKVFY not set*/
  {
    MSD_Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
  }
  else
  {
    MSD_Bot_Abort(BOTH_DIR);
    MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
    MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
  }
}
/*******************************************************************************
* Function Name  : MSD_SCSI_Valid_Cmd
* Description    : Valid Commands routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_Valid_Cmd(uint8_t lun)
{
  if (MSD_CBW.dDataLength != 0)
  {
    MSD_Bot_Abort(BOTH_DIR);
    MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
    MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
  }
  else
    MSD_Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
}
/*******************************************************************************
* Function Name  : MSD_SCSI_Valid_Cmd
* Description    : Valid Commands routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_TestUnitReady_Cmd(uint8_t lun)
{
  if (MSD_MAL_GetStatus(lun))
  {
    MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    MSD_Bot_Abort(DIR_IN);
    return;
  }
  else
  {
    MSD_Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
  }
}
/*******************************************************************************
* Function Name  : MSD_SCSI_Format_Cmd
* Description    : Format Commands routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_Format_Cmd(uint8_t lun)
{
  if (MSD_MAL_GetStatus(lun))
  {
    MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    MSD_Bot_Abort(DIR_IN);
    return;
  }
#ifdef USE_STM3210E_EVAL
  else
  {
    NAND_Format();
    MSD_Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
  }
#endif

}
/*******************************************************************************
* Function Name  : MSD_SCSI_Invalid_Cmd
* Description    : Invalid Commands routine
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void MSD_SCSI_Invalid_Cmd(uint8_t lun)
{
  if (MSD_CBW.dDataLength == 0)
  {
    MSD_Bot_Abort(DIR_IN);
  }
  else
  {
    if ((MSD_CBW.bmFlags & 0x80) != 0)
    {
      MSD_Bot_Abort(DIR_IN);
    }
    else
    {
      MSD_Bot_Abort(BOTH_DIR);
    }
  }
  MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
  MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
}

/*******************************************************************************
* Function Name  : MSD_SCSI_Address_Management
* Description    : Test the received address.
* Input          : uint8_t Cmd : the command can be SCSI_READ10 or SCSI_WRITE10.
* Output         : None.
* Return         : Read\Write status (bool).
*******************************************************************************/
bool MSD_SCSI_Address_Management(uint8_t lun , uint8_t Cmd , uint32_t LBA , uint32_t BlockNbr)
{

  if ((LBA + BlockNbr) > MSD_Mass_Block_Count[lun] )
  {
    if (Cmd == SCSI_WRITE10)
    {
      MSD_Bot_Abort(BOTH_DIR);
    }
    MSD_Bot_Abort(DIR_IN);
    MSD_Set_Scsi_Sense_Data(lun, ILLEGAL_REQUEST, ADDRESS_OUT_OF_RANGE);
    MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
    return (FALSE);
  }


  if (MSD_CBW.dDataLength != BlockNbr * MSD_Mass_Block_Size[lun])
  {
    if (Cmd == SCSI_WRITE10)
    {
      MSD_Bot_Abort(BOTH_DIR);
    }
    else
    {
      MSD_Bot_Abort(DIR_IN);
    }
    MSD_Set_Scsi_Sense_Data(MSD_CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
    MSD_Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
    return (FALSE);
  }
  return (TRUE);
}
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

