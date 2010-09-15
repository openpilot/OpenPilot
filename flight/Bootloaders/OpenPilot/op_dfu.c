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
//#include "platform_config.h"
#include "pios.h"
//#include "stm32f10x.h"
//#include "usb_lib.h"
//#include "usb_istr.h"
//#include "stm32_eval.h"
//#include "stm32f10x_flash.h"
//#include "stm32f10x_crc.h"
//#include "hw_config.h"
//#include <string.h>
#include "op_dfu.h"
#include "pios_bl_helper.h"
#include "pios_opahrs.h"
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
uint32_t Expected_CRC = 0;
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
uint8_t spi_dev_desc[200];
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

uint32_t CalcFirmCRC(void);

void DataDownload(DownloadAction action) {
	if ((DeviceState == downloading)) {

		uint8_t packetSize;
		uint32_t offset;
		SendBuffer[0] = 0x01;
		SendBuffer[1] = Download;
		SendBuffer[2] = downPacketCurrent >> 24;
		SendBuffer[3] = downPacketCurrent >> 16;
		SendBuffer[4] = downPacketCurrent >> 8;
		SendBuffer[5] = downPacketCurrent;
		if (downPacketCurrent == downPacketTotal - 1) {
			packetSize = downSizeOfLastPacket;
		} else {
			packetSize = 14;
		}
		for (uint8_t x = 0; x < packetSize; ++x) {
			offset = baseOfAdressType(downType) + (downPacketCurrent * 14 * 4)
					+ (x * 4);
			switch (currentProgrammingDestination) {
			case Self_flash:
				SendBuffer[6 + (x * 4)] = spi_dev_desc[offset];
				SendBuffer[7 + (x * 4)] = spi_dev_desc[offset + 1];
				SendBuffer[8 + (x * 4)] = spi_dev_desc[offset + 2];
				SendBuffer[9 + (x * 4)] = spi_dev_desc[offset + 3];
				break;
			case Remote_flash_via_spi:
				if (downType == Descript) {
					SendBuffer[6 + (x * 4)] = *FLASH_If_Read(offset);
					SendBuffer[7 + (x * 4)] = *FLASH_If_Read(offset + 1);
					SendBuffer[8 + (x * 4)] = *FLASH_If_Read(offset + 2);
					SendBuffer[9 + (x * 4)] = *FLASH_If_Read(offset + 3);
					break;
				}
			}

		}
		//PIOS USB_SIL_Write(EP1_IN, (uint8_t*) SendBuffer, 64);
		downPacketCurrent = downPacketCurrent + 1;
		if (downPacketCurrent > downPacketTotal - 1) {
			// STM_EVAL_LEDOn(LED2);
			DeviceState = Last_operation_Success;
			Aditionals = (uint32_t) Download;
		}
		PIOS_COM_SendBuffer(PIOS_COM_TELEM_USB, SendBuffer + 1, 63);//FIX+1
		//PIOS SetEPTxValid(ENDP1);
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
		if (((DeviceState == BLidle) && (Data0 < numberOfDevices))
				|| (DeviceState == DFUidle)) {
			if(Data0>0)
				OPDfuIni(TRUE);
			DeviceState = DFUidle;
			currentProgrammingDestination = devicesTable[Data0].programmingType;
			currentDeviceCanRead = devicesTable[Data0].readWriteFlags & 0x01;
			currentDeviceCanWrite = devicesTable[Data0].readWriteFlags >> 1
					& 0x01;
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
				Expected_CRC = Data2 << 24;
				Expected_CRC += Data3 << 16;
				Expected_CRC += Receive_Buffer[DATA + 4] << 8;
				Expected_CRC += Receive_Buffer[DATA + 5];
				SizeOfLastPacket = Data1;

				if (isBiggerThanAvailable(TransferType, (SizeOfTransfer - 1)
						* 14 * 4 + SizeOfLastPacket * 4) == TRUE) {
					DeviceState = outsideDevCapabilities;
					Aditionals = (uint32_t) Command;
				} else {
					uint8_t result = 1;
					struct opahrs_msg_v0 rsp;
					if (TransferType == FW) {
						switch (currentProgrammingDestination) {
						case Self_flash:
							result = FLASH_Start();
							break;
						case Remote_flash_via_spi:
							if (PIOS_OPAHRS_bl_FwupVerify(&rsp)
									== OPAHRS_RESULT_OK) {
								if (rsp.payload.user.v.rsp.fwup_status.status
										== started) {
									result = TRUE;
								} else
									result = FALSE;
							} else
								result = FALSE;
							break;
						default:

							break;
						}
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
					if (Count == SizeOfTransfer - 1)//is this the last packet?
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
						struct opahrs_msg_v0 rsp;
													struct opahrs_msg_v0 req;
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
							req.payload.user.v.req.fwup_data.adress = aux;
							req.payload.user.v.req.fwup_data.data = Data;
							if (PIOS_OPAHRS_bl_FwupData(&req, &rsp)
									== OPAHRS_RESULT_OK) {
								if (rsp.payload.user.v.rsp.fwup_status.status
										== started) {
									result = TRUE;
								} else if (rsp.payload.user.v.rsp.fwup_status.status
										== outside_dev_capabilities) {
									result = TRUE;
									DeviceState = outsideDevCapabilities;
								} else
									result = FALSE;
							}
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
		OPDfuIni(TRUE);
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
				WRFlags = ((devicesTable[x].readWriteFlags << (x * 2))
						| WRFlags);
			}
			Buffer[8] = WRFlags >> 8;
			Buffer[9] = WRFlags;
		} else {
			Buffer[2] = devicesTable[Data0 - 1].sizeOfCode >> 24;
			Buffer[3] = devicesTable[Data0 - 1].sizeOfCode >> 16;
			Buffer[4] = devicesTable[Data0 - 1].sizeOfCode >> 8;
			Buffer[5] = devicesTable[Data0 - 1].sizeOfCode;
			Buffer[6] = Data0;
			Buffer[7] = devicesTable[Data0 - 1].BL_Version;
			Buffer[8] = devicesTable[Data0 - 1].sizeOfDescription;
			Buffer[9] = devicesTable[Data0 - 1].devID;
			Buffer[10] = devicesTable[Data0 - 1].FW_Crc >> 24;
			Buffer[11] = devicesTable[Data0 - 1].FW_Crc >> 16;
			Buffer[12] = devicesTable[Data0 - 1].FW_Crc >> 8;
			Buffer[13] = devicesTable[Data0 - 1].FW_Crc;
		}
		//PIOS USB_SIL_Write(EP1_IN, (uint8_t*) Buffer, 64);
		PIOS_COM_SendBuffer(PIOS_COM_TELEM_USB, Buffer + 1, 63);//FIX+1
		break;
	case JumpFW:
		FLASH_Lock();
		PIOS_OPAHRS_bl_boot();
		JumpToApp = 1;
		break;
	case Reset:
		//PIOS Reset_Device();
		break;
	case Abort_Operation:
		Next_Packet = 0;
		DeviceState = DFUidle;
		break;

	case Op_END:
		if (DeviceState == uploading) {
			if (Next_Packet - 1 == SizeOfTransfer) {
				Next_Packet = 0;
				if ((TransferType != FW) || (Expected_CRC == CalcFirmCRC())) {
					DeviceState = Last_operation_Success;
				} else {
					DeviceState = CRC_Fail;
				}
			}
			if (Next_Packet - 1 < SizeOfTransfer) {
				Next_Packet = 0;
				DeviceState = too_few_packets;
			}
		}

		break;
	case Download_Req:
		if (DeviceState == DFUidle) {
			downType = Data0;
			downPacketTotal = Count;
			downSizeOfLastPacket = Data1;
			if (isBiggerThanAvailable(downType, (downPacketTotal - 1) * 14
					+ downSizeOfLastPacket) == 1) {
				DeviceState = outsideDevCapabilities;
				Aditionals = (uint32_t) Command;

			} else
				downPacketCurrent = 0;
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
		PIOS_COM_SendBuffer(PIOS_COM_TELEM_USB, Buffer + 1, 63);//FIX+1
		//PIOS USB_SIL_Write(EP1_IN, (uint8_t*) Buffer, 64);
		//PIOS SetEPTxValid(ENDP1);
		if (DeviceState == Last_operation_Success) {
			DeviceState = DFUidle;
		}
		break;
	case Status_Rep:

		break;

	}
	if (EchoReqFlag == 1) {
		echoBuffer[1] = echoBuffer[1] | EchoAnsFlag;
		PIOS_COM_SendBuffer(PIOS_COM_TELEM_USB, echoBuffer, 63);
		//PIOS USB_SIL_Write(EP1_IN, (uint8_t*) echoBuffer, 64);
		//PIOS SetEPTxValid(ENDP1);
	}
	return;
}
void OPDfuIni(uint8_t discover) {
	Device dev;
	dev.programmingType = Self_flash;
	dev.readWriteFlags = (BOARD_READABLE | (BOARD_WRITABLA << 1));
	dev.startOfUserCode = START_OF_USER_CODE;
	dev.sizeOfCode = SIZE_OF_CODE;
	dev.sizeOfDescription = SIZE_OF_DESCRIPTION;
	dev.BL_Version = BOOTLOADER_VERSION;
	dev.FW_Crc = CalcFirmCRC();
	dev.devID = HW_VERSION;
	dev.devType = HW_TYPE;
	numberOfDevices = 1;
	devicesTable[0] = dev;
	if (discover) {
		uint8_t found_spi_device = FALSE;

		for (int t = 0; t < 10; ++t) {
			if (PIOS_OPAHRS_bl_resync() == OPAHRS_RESULT_OK) {
				found_spi_device = TRUE;
				dev.FW_Crc = 0;
				break;
			}
			PIOS_DELAY_WaitmS(500);
		}
		if (found_spi_device == TRUE) {
			struct opahrs_msg_v0 rsp;
			if (PIOS_OPAHRS_bl_GetVersions(&rsp) == OPAHRS_RESULT_OK) {
				dev.programmingType = Remote_flash_via_spi;
				dev.BL_Version = rsp.payload.user.v.rsp.versions.bl_version;
				dev.FW_Crc = rsp.payload.user.v.rsp.versions.fw_version;
				dev.devID = rsp.payload.user.v.rsp.versions.hw_version;
				memcpy(spi_dev_desc,
						rsp.payload.user.v.rsp.versions.description,
						sizeof(rsp.payload.user.v.rsp.versions.description));
				if (PIOS_OPAHRS_bl_GetMemMap(&rsp) == OPAHRS_RESULT_OK) {
					dev.readWriteFlags
							= rsp.payload.user.v.rsp.mem_map.rw_flags;
					dev.startOfUserCode
							= rsp.payload.user.v.rsp.mem_map.start_of_user_code;
					dev.sizeOfCode
							= rsp.payload.user.v.rsp.mem_map.size_of_code_memory;
					dev.sizeOfDescription
							= rsp.payload.user.v.rsp.mem_map.size_of_description;
					dev.devType = rsp.payload.user.v.rsp.mem_map.density;
					numberOfDevices = 2;
					devicesTable[1] = dev;
				}
			}
		}
	}
	//TODO check other devices trough spi or whatever
}
uint32_t baseOfAdressType(DFUTransfer type) {
	switch (type) {
	case FW:
		return currentDevice.startOfUserCode;
		break;
	case Descript:
		return currentDevice.startOfUserCode + currentDevice.sizeOfCode;
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
	case Descript:
		return (size > currentDevice.sizeOfDescription) ? 1 : 0;
		break;
	default:
		return TRUE;
	}
}

uint32_t CalcFirmCRC() {
	struct opahrs_msg_v0 rsp;
	switch (currentProgrammingDestination) {
	case Self_flash:
		return crc_memory_calc();
		break;
	case Remote_flash_via_spi:
		if (PIOS_OPAHRS_bl_FwupVerify(&rsp) == OPAHRS_RESULT_OK) {
			if (PIOS_OPAHRS_bl_GetVersions(&rsp) == OPAHRS_RESULT_OK) {
				return rsp.payload.user.v.rsp.versions.fw_version;
			}
		}
		return 0;
		break;
	default:
		return 0;
		break;
	}

}
