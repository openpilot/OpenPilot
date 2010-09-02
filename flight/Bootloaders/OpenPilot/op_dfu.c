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
#include <string.h>
#include "op_dfu.h"
#include "flash_dfu.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

//programmable devices
Device devicesTable[10];
uint8_t numberOfDevices = 0;

DFUProgType currentProgrammingDestination; //flash, flash_trough spi
uint8_t currentDeviceCanRead;
uint8_t currentDeviceCanWrite;
Device currentDevice;

uint8_t Buffer[64];
uint8_t echoBuffer[64];
uint8_t SendBuffer[64];
uint8_t Command = 0;
uint8_t EchoReqFlag = 0;
uint8_t EchoAnsFlag = 0;
uint8_t StartFlag = 0;
uint32_t Aditionals = 0;
uint32_t SizeOfTransfer = 0;
uint8_t SizeOfLastPacket = 0;
uint32_t Next_Packet = 0;
uint8_t TransferType;
uint32_t Count = 0;
uint32_t Data;
uint8_t Data0;
uint8_t Data1;
uint8_t Data2;
uint8_t Data3;
uint8_t offset = 0;
uint32_t aux;
//Download vars
uint32_t downSizeOfLastPacket = 0;
uint32_t downPacketTotal = 0;
uint32_t downPacketCurrent = 0;
DFUTransfer downType = 0;
/* Extern variables ----------------------------------------------------------*/
extern DFUStates DeviceState;
extern uint8_t JumpToApp;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void DataDownload() {
	if ((DeviceState == downloading) && (GetEPTxStatus(ENDP1) == EP_TX_NAK)) {
		uint8_t packetSize;
		SendBuffer[0] = 0x01;
		SendBuffer[1] = Download;
		SendBuffer[2] = downPacketCurrent >> 24;
		SendBuffer[3] = downPacketCurrent >> 16;
		SendBuffer[4] = downPacketCurrent >> 8;
		SendBuffer[5] = downPacketCurrent;
		if (downPacketCurrent == downPacketTotal) {
			packetSize = downSizeOfLastPacket;
		} else {
			packetSize = 14;
		}
		for (int x = 0; x < packetSize; ++x) {
			switch (currentProgrammingDestination) {
			case Self_flash:
				SendBuffer[6 + x * 4] = *FLASH_If_Read(baseOfAdressType(
						downType) + downPacketCurrent * 4 + x * 4, 0);
				SendBuffer[7 + x * 4] = *FLASH_If_Read(baseOfAdressType(
						downType) + 1 + downPacketCurrent * 4 + x * 4, 0);
				SendBuffer[8 + x * 4] = *FLASH_If_Read(baseOfAdressType(
						downType) + 2 + downPacketCurrent * 4 + x * 4, 0);
				SendBuffer[9 + x * 4] = *FLASH_If_Read(baseOfAdressType(
						downType) + 3 + downPacketCurrent * 4 + x * 4, 0);
				break;
			case Remote_flash_via_spi:
				//TODO result=SPI_FLASH();
				break;
			}

		}
	USB_SIL_Write(EP1_IN, (uint8_t*) SendBuffer, 64);
	downPacketCurrent = downPacketCurrent + 1;
	if (downPacketCurrent > downPacketTotal) {
		// STM_EVAL_LEDOn(LED2);
		DeviceState = Last_operation_Success;
		Aditionals = (uint32_t) Download;
	}
	SetEPTxValid(ENDP1);
}
}

void processComand(uint8_t *Receive_Buffer) {

Command = Receive_Buffer[COMMAND];
EchoReqFlag = (Command >> 7);
EchoAnsFlag = (Command >> 6) & 0x01;
StartFlag = (Command >> 5) & 0x01;
Count = Receive_Buffer[COUNT] << 24;
Count += Receive_Buffer[COUNT + 1] << 16;
Count += Receive_Buffer[COUNT + 2] << 8;
Count += Receive_Buffer[COUNT + 3];
Data = Receive_Buffer[DATA] << 24;
Data += Receive_Buffer[DATA + 1] << 16;
Data += Receive_Buffer[DATA + 2] << 8;
Data += Receive_Buffer[DATA + 3];
Data0 = Receive_Buffer[DATA];
Data1 = Receive_Buffer[DATA + 1];
Data2 = Receive_Buffer[DATA + 2];
Data3 = Receive_Buffer[DATA + 3];
Command = Command & 0b00011111;

if (EchoReqFlag == 1) {
	memcpy(echoBuffer, Buffer, 64);
}
switch (Command) {
case EnterDFU:
	if (((DeviceState == idle) && (Data0 < numberOfDevices)) || (DeviceState == DFUidle)) {
		DeviceState = DFUidle;
		currentProgrammingDestination = devicesTable[Data0].programmingType;
		currentDeviceCanRead = devicesTable[Data0].readWriteFlags & 0x01;
		currentDeviceCanWrite = devicesTable[Data0].readWriteFlags >> 1 & 0x01;
		currentDevice = devicesTable[Data0];
		uint8_t result = 0;
		switch (currentProgrammingDestination) {
		case Self_flash:
			result = FLASH_Ini();
			break;
		case Remote_flash_via_spi:
			//TODO result=SPI_FLASH();
			break;
		default:
			DeviceState = Last_operation_failed;
			Aditionals = (uint16_t) Command;
		}
		if (result != 1) {
			DeviceState = Last_operation_failed;
			Aditionals = (uint32_t) Command;
		}
	} else {
		DeviceState = outsideDevCapabilities;
		Aditionals = (uint32_t) Command;
	}
	break;
case Upload:
	if ((DeviceState == DFUidle) || (DeviceState == uploading)) {
		if ((StartFlag == 1) && (Next_Packet == 0)) {
			TransferType = Data0;
			SizeOfTransfer = Count;
			Next_Packet = 1;
			SizeOfLastPacket = Data1;
			if (isBiggerThanAvailable(TransferType, (SizeOfTransfer - 1) * 14
					* 4 + SizeOfLastPacket * 4) == TRUE) {
				DeviceState = outsideDevCapabilities;
				Aditionals = (uint32_t) Command;
			} else {
				uint8_t result = 1;
				if (TransferType == FW) {
					uint32_t size = (SizeOfTransfer - 1) * 14 * 4
							+ SizeOfLastPacket * 4;
					result = FLASH_Start(size);
				}
				if (result != 1) {
					DeviceState = Last_operation_failed;//ok
					Aditionals = (uint32_t) Command;
				} else {

					DeviceState = uploading;
				}
			}
		} else if ((StartFlag != 1) && (Next_Packet != 0)) {
			if (Count > SizeOfTransfer) {
				DeviceState = too_many_packets;
				Aditionals = Count;
			} else if (Count == Next_Packet - 1) {
				uint8_t numberOfWords = 14;
				if (Count == SizeOfTransfer-1)//is this the last packet?
				{
					numberOfWords = SizeOfLastPacket;
				}
				for (uint8_t x = 0; x < numberOfWords; ++x) {
					offset = 4 * x;
					Data = Receive_Buffer[DATA + offset] << 24;
					Data += Receive_Buffer[DATA + 1 + offset] << 16;
					Data += Receive_Buffer[DATA + 2 + offset] << 8;
					Data += Receive_Buffer[DATA + 3 + offset];
					aux = baseOfAdressType(TransferType) + (uint32_t)(Count
							* 14 * 4 + x * 4);
					uint8_t result = 0;
					//uint8_t lol=0;
					switch (currentProgrammingDestination) {
					case Self_flash:
							for (int retry = 0; retry < MAX_WRI_RETRYS; ++retry) {
								if (result == 0) {
									result = (FLASH_ProgramWord(aux, Data)
											== FLASH_COMPLETE) ? 1 : 0;
								}
							}
							break;
					case Remote_flash_via_spi:
						//TODO result=SPI_FLASH_ProgramWord(aux,Data);
						break;
					default:
						result = 0;
						break;
					}
					if (result != 1) {
						DeviceState = Last_operation_failed;
						Aditionals = (uint32_t) Command;
					}
				}
				++Next_Packet;
			} else {

				DeviceState = wrong_packet_received;
				Aditionals = Count;
			}
			// FLASH_ProgramWord(MemLocations[TransferType]+4,++Next_Packet);//+Count,Data);
		} else {
			DeviceState = Last_operation_failed;
			Aditionals = (uint32_t) Command;
		}
	}
	break;
case Req_Capabilities:
	Buffer[0] = 0x01;
	Buffer[1] = Rep_Capabilities;
	if (Data0 == 0) {
		Buffer[2] = 0;
		Buffer[3] = 0;
		Buffer[4] = 0;
		Buffer[5] = 0;
		Buffer[6] = 0;
		Buffer[7] = numberOfDevices;
		uint16_t WRFlags = 0;
		for (int x = 0; x < numberOfDevices; ++x) {
			WRFlags = ((devicesTable[x].readWriteFlags << (x * 2)) | WRFlags);
		}
		Buffer[8] = WRFlags >> 8;
		Buffer[9] = WRFlags;
	} else {
		Buffer[2] = devicesTable[Data0-1].sizeOfCode >> 24;
		Buffer[3] = devicesTable[Data0-1].sizeOfCode >> 16;
		Buffer[4] = devicesTable[Data0-1].sizeOfCode >> 8;
		Buffer[5] = devicesTable[Data0-1].sizeOfCode;
		Buffer[6] = Data0;
		Buffer[7] = devicesTable[Data0-1].sizeOfHash;
		Buffer[8] = devicesTable[Data0-1].sizeOfDescription;
		Buffer[9] = devicesTable[Data0-1].devID;
	}
	USB_SIL_Write(EP1_IN, (uint8_t*) Buffer, 64);
	SetEPTxValid(ENDP1);
	break;
case Rep_Capabilities:

	break;
case JumpFW:
	FLASH_Lock();
	JumpToApp = 1;
	break;
case Reset:
	Reset_Device();
	break;
case Abort_Operation:
	Next_Packet=0;
	DeviceState=DFUidle;
	break;

case Op_END:
	if (DeviceState == uploading) {
		if (Next_Packet - 1 == SizeOfTransfer) {
			Next_Packet=0;
			DeviceState = Last_operation_Success;
		}
		if (Next_Packet - 1 < SizeOfTransfer) {
			Next_Packet=0;
			DeviceState = too_few_packets;
		}
	}

	break;
case Download_Req:
	if (DeviceState == DFUidle) {
		downType = Data0;
		downPacketTotal = Count;
		downSizeOfLastPacket = Data1;
		if (isBiggerThanAvailable(downType, downPacketTotal - 1 * 14
				+ downSizeOfLastPacket) == 1) {
			DeviceState = outsideDevCapabilities;
			Aditionals = (uint32_t) Command;

		} else

			DeviceState = downloading;
	} else {
		DeviceState = Last_operation_failed;
		Aditionals = (uint32_t) Command;
	}
	break;

case Status_Request:
	Buffer[0] = 0x01;
	Buffer[1] = Status_Rep;
	if (DeviceState == wrong_packet_received) {
		Buffer[2] = Aditionals >> 24;
		Buffer[3] = Aditionals >> 16;
		Buffer[4] = Aditionals >> 8;
		Buffer[5] = Aditionals;
	} else {
		Buffer[2] = 0;
		Buffer[3] = ((uint16_t) Aditionals) >> 8;
		Buffer[4] = ((uint16_t) Aditionals);
		Buffer[5] = 0;
	}
	Buffer[6] = DeviceState;
	Buffer[7] = 0;
	Buffer[8] = 0;
	Buffer[9] = 0;

	USB_SIL_Write(EP1_IN, (uint8_t*) Buffer, 64);
	SetEPTxValid(ENDP1);
	if (DeviceState == Last_operation_Success) {
		DeviceState = DFUidle;
	}
	break;
case Status_Rep:

	break;

}
if (EchoReqFlag == 1) {
	echoBuffer[1] = echoBuffer[1] | EchoAnsFlag;
	USB_SIL_Write(EP1_IN, (uint8_t*) echoBuffer, 64);
	SetEPTxValid(ENDP1);
}
return;
}
void OPDfuIni(void) {
Device dev;
dev.programmingType = Self_flash;
dev.readWriteFlags = (board_can_read | (board_can_write << 1));
dev.startOfUserCode = StartOfUserCode;
dev.sizeOfCode = SizeOfCode;
dev.sizeOfDescription = SizeOfDescription;
dev.sizeOfHash = SizeOfHash;
dev.devID = deviceID;
numberOfDevices = 1;
devicesTable[0] = dev;
//TODO check other devices trough spi or whatever
}
uint32_t baseOfAdressType(DFUTransfer type) {
switch (type) {
case FW:
	return currentDevice.startOfUserCode;
	break;
case Hash:
	return currentDevice.startOfUserCode + currentDevice.sizeOfCode;
	break;
case Descript:
	return currentDevice.startOfUserCode + currentDevice.sizeOfCode
			+ currentDevice.sizeOfHash;
	;
	break;
default:
	return 0;
}
}
uint8_t isBiggerThanAvailable(DFUTransfer type, uint32_t size) {
switch (type) {
case FW:
	return (size > currentDevice.sizeOfCode) ? 1 : 0;
	break;
case Hash:
	return (size > currentDevice.sizeOfHash) ? 1 : 0;
	break;
case Descript:
	return (size > currentDevice.sizeOfDescription) ? 1 : 0;
	break;
default:
	return TRUE;
}
}

