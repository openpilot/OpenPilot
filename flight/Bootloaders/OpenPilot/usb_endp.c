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



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t Receive_Buffer[64];
uint8_t Buffer[64];
uint8_t Command=0;
uint8_t EchoReqFlag=0;
uint8_t EchoAnsFlag=0;
uint8_t StartFlag=0;
uint32_t Aditionals=0;
uint32_t SizeOfTransfer=0;
uint8_t SizeOfLastPacket=0;
uint32_t Next_Packet=0;
uint8_t TransferType;
uint32_t Count=0;
uint32_t Data;
uint8_t Data0;
uint8_t Data1;
uint8_t Data2;
uint8_t Data3;
uint8_t offset=0;
uint32_t aux;
//Download vars
volatile uint32_t downPacketCurrent=0;
uint32_t downPacketTotal=0;
uint8_t downType=0;
uint32_t downSizeOfLastPacket=0;
uint32_t MemLocations[3]=
{
		StartOfUserCode, StartOfUserCode-SizeOfHash, StartOfUserCode-SizeOfHash-SizeOfDescription
};
/* Extern variables ----------------------------------------------------------*/
extern  uint8_t DeviceState ;
extern 	uint8_t JumpToApp;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
uint8_t *FLASH_If_Read (uint32_t SectorAddress, uint32_t DataLength)
{
  return  (uint8_t*)(SectorAddress);
}
void FLASH_Download() {
	if ((DeviceState == downloading) && (GetEPTxStatus(ENDP1)==EP_TX_NAK)) {
		uint8_t packetSize;
		Buffer[0] = 0x01;
		Buffer[1] = Download;
		Buffer[2] = downPacketCurrent >> 24;
		Buffer[3] = downPacketCurrent >> 16;
		Buffer[4] = downPacketCurrent >> 8;
		Buffer[5] = downPacketCurrent;
		if(downPacketCurrent==downPacketTotal)
		{
			packetSize=downSizeOfLastPacket;
		}
		else
		{
			packetSize=14;
		}
		for(int x=0;x<packetSize;++x)
		{
		Buffer[6+x*4] = *FLASH_If_Read(MemLocations[downType]+  downPacketCurrent*4+x*4, 0);
		Buffer[7+x*4] = *FLASH_If_Read(MemLocations[downType] + 1+downPacketCurrent*4+x*4, 0);
		Buffer[8+x*4] = *FLASH_If_Read(MemLocations[downType] + 2+downPacketCurrent*4+x*4, 0);
		Buffer[9+x*4] = *FLASH_If_Read(MemLocations[downType] + 3+downPacketCurrent*4+x*4, 0);
		}
		USB_SIL_Write(EP1_IN, (uint8_t*) Buffer, 64);
		downPacketCurrent=downPacketCurrent+1;
		if(downPacketCurrent>downPacketTotal)
		{
			// STM_EVAL_LEDOn(LED2);
			DeviceState=Last_operation_Success;
			Aditionals=(uint32_t)Download;
		}
		SetEPTxValid(ENDP1);
	}
}

/*******************************************************************************
* Function Name  : EP1_OUT_Callback.
* Description    : EP1 OUT Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_OUT_Callback(void)
{
  BitAction Led_State;

  /* Read received data (2 bytes) */
  USB_SIL_Read(EP1_OUT, Receive_Buffer);
  Command=Receive_Buffer[1];
  EchoReqFlag=(Command>>7);
  EchoAnsFlag=(Command>>6) & 0x01;
  StartFlag=(Command>>5) & 0x01;
  Count=Receive_Buffer[2]<<24;
  Count+=Receive_Buffer[3]<<16;
  Count+=Receive_Buffer[4]<<8;
  Count+=Receive_Buffer[5];
  Data=Receive_Buffer[6]<<24;
  Data+=Receive_Buffer[7]<<16;
  Data+=Receive_Buffer[8]<<8;
  Data+=Receive_Buffer[9];
  Data0=Receive_Buffer[6];
  Data1=Receive_Buffer[7];
  Data2=Receive_Buffer[8];
  Data3=Receive_Buffer[9];
  Command=Command & 0b00011111;
  switch(Command)
  {
  case EnterDFU:
	  if (DeviceState==idle)
	  {
		  DeviceState=DFUidle;
		  FLASH_Unlock();
	  }
	  else
	  {
		  DeviceState=Last_operation_failed;
		  Aditionals=(uint16_t) Command;
	  }
	  break;
  case Upload:
	  if((DeviceState==DFUidle) || (DeviceState==uploading))
	  {
		  if((StartFlag==1) && (Next_Packet==0))
		  {

			  TransferType=Data0;
			  SizeOfTransfer=Count;
			  Next_Packet=1;
			  DeviceState=uploading;
			  SizeOfLastPacket=Data1;


		  }
		  else if((StartFlag!=1) && (Next_Packet!=0))
		  {
			 // STM_EVAL_LEDOn(LED2);
			  if(Count==Next_Packet-1)
			  {
				  uint8_t numberOfWords=14;
				  if(Count==SizeOfTransfer)//is this the last packet?
				  {
					  numberOfWords=SizeOfLastPacket;
				  }
				  for(uint8_t x=0;x<numberOfWords;++x)
				  {
					  offset=4*x;
					  Data=Receive_Buffer[6+offset]<<24;
					  Data+=Receive_Buffer[7+offset]<<16;
					  Data+=Receive_Buffer[8+offset]<<8;
					  Data+=Receive_Buffer[9+offset];
					  aux=MemLocations[TransferType]+(uint32_t)(Count*14*4+x*4);
					  FLASH_ProgramWord(aux,Data);
				  }
				  ++Next_Packet;
			  }
			  else
			  {

				  DeviceState=wrong_packet_received;
				  Aditionals=Count;
			  }
			 // FLASH_ProgramWord(MemLocations[TransferType]+4,++Next_Packet);//+Count,Data);
		  }
	  else
	 	  {
		  STM_EVAL_LEDOn(LED2);
	 		  DeviceState=Last_operation_failed;
	 		  Aditionals=(uint32_t) Command;
	 	  }
	  }
	  break;
  case Req_Capabilities:

	  break;
  case Rep_Capabilities:

	  break;

  case JumpFW:
	  JumpToApp=1;
	  break;
  case Reset:
	  Reset_Device();
	  break;
  case Abort_Operation:

	  break;

  case Op_END:
	  if(DeviceState==uploading)
	  {
		  if(Next_Packet-1==SizeOfTransfer)
		  {
			  DeviceState=Last_operation_Success;
		  }
		  if(Next_Packet-1<SizeOfTransfer)
		  {
		  	  DeviceState=too_few_packets;
		  }
	  }

	  break;
  case Download_Req:
		 if(DeviceState==DFUidle)
		 {
			 downType=Data0;
			 downPacketCurrent=0;
			 downPacketTotal=Count;
			 DeviceState=downloading;
			 downSizeOfLastPacket=Data1;
		 }
		 else
		 {
			 DeviceState=Last_operation_failed;
			 Aditionals=(uint32_t) Command;
		 }
	  break;

  case Status_Request:
	  Buffer[0]=0x01;
	  Buffer[1]=Status_Rep;
	  if(DeviceState==wrong_packet_received)
	  {
		  Buffer[2]=Aditionals>>24;
		  Buffer[3]=Aditionals>>16;
		  Buffer[4]=Aditionals>>8;
		  Buffer[4]=Aditionals;
	  }
	  else
	  {
		  Buffer[2]=0;
		  Buffer[3]=((uint16_t)Aditionals)>>8;
		  Buffer[4]=((uint16_t)Aditionals);
		  Buffer[5]=0;
	  }
	  Buffer[6]=DeviceState;
	  Buffer[7]=0;
	  Buffer[8]=0;
	  Buffer[9]=0;

	  USB_SIL_Write(EP1_IN, (uint8_t*)Buffer,64);
	  SetEPTxValid(ENDP1);
	  if(DeviceState==Last_operation_Success)
	  {
		  DeviceState=DFUidle;
	  }
	  break;
  case Status_Rep:

	  break;

  }
  SetEPRxStatus(ENDP1, EP_RX_VALID);
  //uint8_t Buffer[2];
  Buffer[0]=0x07;
  Buffer[1]=2;
  //USB_SIL_Write(EP1_IN, (uint8_t*)Buffer,2);
  //SetEPTxValid(ENDP1);
  return;

  if (Receive_Buffer[1] == 0)
  {
    Led_State = Bit_RESET;
  }
  else 
  {
    Led_State = Bit_SET;
  }
 
 
  switch (Receive_Buffer[0])
  {
    case 1: /* Led 1 */
     if (Led_State != Bit_RESET)
     {
       STM_EVAL_LEDOn(LED1);
     }
     else
     {
       STM_EVAL_LEDOff(LED1);
     }
     break;
    case 2: /* Led 2 */
     if (Led_State != Bit_RESET)
     {
       STM_EVAL_LEDOn(LED2);
     }
     else
     {
       STM_EVAL_LEDOff(LED2);
     }
      break;
    case 3: /* Led 3 */
     if (Led_State != Bit_RESET)
     {
       STM_EVAL_LEDOn(LED3);
     }
     else
     {
       STM_EVAL_LEDOff(LED3);
     }
      break;
    case 4: /* Led 4 */
     if (Led_State != Bit_RESET)
     {
       STM_EVAL_LEDOn(LED4);
     }
     else
     {
       STM_EVAL_LEDOff(LED4);
     }
      break;
  default:
    STM_EVAL_LEDOff(LED1);
    STM_EVAL_LEDOff(LED2);
    STM_EVAL_LEDOff(LED3);
    STM_EVAL_LEDOff(LED4); 
    break;
  }
 
#ifndef STM32F10X_CL   
  SetEPRxStatus(ENDP1, EP_RX_VALID);
#endif /* STM32F10X_CL */
 
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

