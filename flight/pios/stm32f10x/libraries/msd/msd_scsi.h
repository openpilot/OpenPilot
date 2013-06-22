/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : usb_scsi.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_SCSI_H
#define __USB_SCSI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* SCSI Commands */
#define SCSI_FORMAT_UNIT                            0x04
#define SCSI_INQUIRY                                0x12
#define SCSI_MODE_SELECT6                           0x15
#define SCSI_MODE_SELECT10                          0x55
#define SCSI_MODE_SENSE6                            0x1A
#define SCSI_MODE_SENSE10                           0x5A
#define SCSI_ALLOW_MEDIUM_REMOVAL                   0x1E
#define SCSI_READ6                                  0x08
#define SCSI_READ10                                 0x28
#define SCSI_READ12                                 0xA8
#define SCSI_READ16                                 0x88

#define SCSI_READ_CAPACITY10                        0x25
#define SCSI_READ_CAPACITY16                        0x9E

#define SCSI_REQUEST_SENSE                          0x03
#define SCSI_START_STOP_UNIT                        0x1B
#define SCSI_TEST_UNIT_READY                        0x00
#define SCSI_WRITE6                                 0x0A
#define SCSI_WRITE10                                0x2A
#define SCSI_WRITE12                                0xAA
#define SCSI_WRITE16                                0x8A

#define SCSI_VERIFY10                               0x2F
#define SCSI_VERIFY12                               0xAF
#define SCSI_VERIFY16                               0x8F

#define SCSI_SEND_DIAGNOSTIC                        0x1D
#define SCSI_READ_FORMAT_CAPACITIES                 0x23

#define NO_SENSE                                    0
#define RECOVERED_ERROR                             1
#define NOT_READY                                   2
#define MEDIUM_ERROR                                3
#define HARDWARE_ERROR                              4
#define ILLEGAL_REQUEST                             5
#define UNIT_ATTENTION                              6
#define DATA_PROTECT                                7
#define BLANK_CHECK                                 8
#define VENDOR_SPECIFIC                             9
#define COPY_ABORTED                                10
#define ABORTED_COMMAND                             11
#define VOLUME_OVERFLOW                             13
#define MISCOMPARE                                  14


#define INVALID_COMMAND                             0x20
#define INVALID_FIELED_IN_COMMAND                   0x24
#define PARAMETER_LIST_LENGTH_ERROR                 0x1A
#define INVALID_FIELD_IN_PARAMETER_LIST             0x26
#define ADDRESS_OUT_OF_RANGE                        0x21
#define MEDIUM_NOT_PRESENT                          0x3A
#define MEDIUM_HAVE_CHANGED                         0x28

#define READ_FORMAT_CAPACITY_DATA_LEN               0x0C
#define READ_CAPACITY10_DATA_LEN                    0x08
#define MODE_SENSE10_DATA_LEN                       0x08
#define MODE_SENSE6_DATA_LEN                        0x04
#define REQUEST_SENSE_DATA_LEN                      0x12
#define STANDARD_INQUIRY_DATA_LEN                   0x24
#define BLKVFY                                      0x04

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern void MSD_SCSI_Inquiry_Cmd(uint8_t lun);
extern void MSD_SCSI_ReadFormatCapacity_Cmd(uint8_t lun);
extern void MSD_SCSI_ReadCapacity10_Cmd(uint8_t lun);
extern void MSD_SCSI_RequestSense_Cmd (uint8_t lun);
extern void MSD_SCSI_Start_Stop_Unit_Cmd(uint8_t lun);
extern void MSD_SCSI_ModeSense6_Cmd (uint8_t lun);
extern void MSD_SCSI_ModeSense10_Cmd (uint8_t lun);
extern void MSD_SCSI_Write10_Cmd(uint8_t lun , uint32_t LBA , uint32_t BlockNbr);
extern void MSD_SCSI_Read10_Cmd(uint8_t lun , uint32_t LBA , uint32_t BlockNbr);
extern void MSD_SCSI_Verify10_Cmd(uint8_t lun);

extern void MSD_SCSI_Invalid_Cmd(uint8_t lun);
extern void MSD_SCSI_Valid_Cmd(uint8_t lun);
extern bool MSD_SCSI_Address_Management(uint8_t lun , uint8_t Cmd , uint32_t LBA , uint32_t BlockNbr);

extern void MSD_Set_Scsi_Sense_Data(uint8_t lun , uint8_t Sens_Key, uint8_t Asc);
extern void MSD_SCSI_TestUnitReady_Cmd (uint8_t lun);
extern void MSD_SCSI_Format_Cmd (uint8_t lun);

//#define MSD_SCSI_TestUnitReady_Cmd           MSD_SCSI_Valid_Cmd
#define MSD_SCSI_Prevent_Removal_Cmd         MSD_SCSI_Valid_Cmd

/* Invalid (Unsupported) commands */
#define MSD_SCSI_READ_CAPACITY16_Cmd         MSD_SCSI_Invalid_Cmd
//#define MSD_SCSI_FormatUnit_Cmd              MSD_SCSI_Invalid_Cmd
#define MSD_SCSI_Write6_Cmd                  MSD_SCSI_Invalid_Cmd
#define MSD_SCSI_Write16_Cmd                 MSD_SCSI_Invalid_Cmd
#define MSD_SCSI_Write12_Cmd                 MSD_SCSI_Invalid_Cmd
#define MSD_SCSI_Read6_Cmd                   MSD_SCSI_Invalid_Cmd
#define MSD_SCSI_Read12_Cmd                  MSD_SCSI_Invalid_Cmd
#define MSD_SCSI_Read16_Cmd                  MSD_SCSI_Invalid_Cmd
#define MSD_SCSI_Send_Diagnostic_Cmd         MSD_SCSI_Invalid_Cmd
#define MSD_SCSI_Mode_Select6_Cmd            MSD_SCSI_Invalid_Cmd
#define MSD_SCSI_Mode_Select10_Cmd           MSD_SCSI_Invalid_Cmd
#define MSD_SCSI_Verify12_Cmd                MSD_SCSI_Invalid_Cmd
#define MSD_SCSI_Verify16_Cmd                MSD_SCSI_Invalid_Cmd


/* Exported variables ------------------------------------------------------- */
extern  uint8_t MSD_Page00_Inquiry_Data[];
extern  uint8_t MSD_Standard_Inquiry_Data[];
extern  uint8_t MSD_Standard_Inquiry_Data2[];
extern  uint8_t MSD_Mode_Sense6_data[];
extern  uint8_t MSD_Mode_Sense10_data[];
extern  uint8_t MSD_Scsi_Sense_Data[];
extern  uint8_t MSD_ReadCapacity10_Data[];
extern  uint8_t MSD_ReadFormatCapacity_Data [];


#endif /* __USB_SCSI_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

