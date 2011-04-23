/**
 ******************************************************************************
 * @addtogroup AHRS BOOTLOADER
 * @brief The AHRS Modules perform
 *
 * @{ 
 * @addtogroup AHRS_BOOTLOADER_Main
 * @brief Main function which does the hardware dependent stuff
 * @{ 
 *
 *
 * @file       main.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
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

/* OpenPilot Includes */
#include "ahrs_bl.h"
#include "pios_opahrs_proto.h"
#include "bl_fsm.h"		/* lfsm_state */
#include "stm32f10x_flash.h"

extern void PIOS_Board_Init(void);

#define NSS_HOLD_STATE       ((GPIOB->IDR & GPIO_Pin_12) ? 0 : 1)
enum bootloader_status boot_status;
/* Private typedef -----------------------------------------------------------*/
typedef void
(*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress;
/* Function Prototypes */
void
process_spi_request(void);
void
jump_to_app();
uint8_t jumpFW = FALSE;
uint32_t Fw_crc;
/**
 * @brief Bootloader Main function
 */
int main() {

	uint8_t GO_dfu = false;
	/* Brings up System using CMSIS functions, enables the LEDs. */
	PIOS_SYS_Init();
	/* Enable Prefetch Buffer */
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

	/* Flash 2 wait state */
	FLASH_SetLatency(FLASH_Latency_2);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
	/* Delay system */
	PIOS_DELAY_Init();

	for (uint32_t t = 0; t < 10000000; ++t) {
		if (NSS_HOLD_STATE == 1)
			GO_dfu = TRUE;
		else {
			GO_dfu = FALSE;
			break;
		}
	}
	//while(TRUE)
	//	{
	//		PIOS_LED_Toggle(LED1);
	//		PIOS_DELAY_WaitmS(1000);
	//	}
	//GO_dfu = TRUE;
	PIOS_IAP_Init();
	GO_dfu = GO_dfu | PIOS_IAP_CheckRequest();// OR with app boot request
	if (GO_dfu == FALSE) {
		jump_to_app();
	}
	if(PIOS_IAP_CheckRequest())
	{
		PIOS_DELAY_WaitmS(1000);
		PIOS_IAP_ClearRequest();
	}
	PIOS_Board_Init();
	boot_status = idle;
	Fw_crc = FLASH_crc_memory_calc();
	PIOS_LED_On(LED1);
	while (1) {
		process_spi_request();
	}
	return 0;
}

static struct opahrs_msg_v0 link_tx_v0;
static struct opahrs_msg_v0 link_rx_v0;
static struct opahrs_msg_v0 user_tx_v0;
static struct opahrs_msg_v0 user_rx_v0;
void process_spi_request(void) {
	bool msg_to_process = FALSE;

	PIOS_IRQ_Disable();
	/* Figure out if we're in an interesting stable state */
	switch (lfsm_get_state()) {
	case LFSM_STATE_USER_BUSY:
		msg_to_process = TRUE;
		break;
	case LFSM_STATE_INACTIVE:
		/* Queue up a receive buffer */
		lfsm_user_set_rx_v0(&user_rx_v0);
		lfsm_user_done();
		break;
	case LFSM_STATE_STOPPED:
		/* Get things going */
		lfsm_set_link_proto_v0(&link_tx_v0, &link_rx_v0);
		break;
	default:
		/* Not a stable state */
		break;
	}
	PIOS_IRQ_Enable();

	if (!msg_to_process) {
		/* Nothing to do */
		//PIOS_COM_SendFormattedString(PIOS_COM_AUX, ".");
		return;
	}

	if (user_rx_v0.tail.magic != OPAHRS_MSG_MAGIC_TAIL) {
		return;
	}

	switch (user_rx_v0.payload.user.t) {

	case OPAHRS_MSG_V0_REQ_FWUP_VERIFY:
		opahrs_msg_v0_init_user_tx(&user_tx_v0, OPAHRS_MSG_V0_RSP_FWUP_STATUS);
		Fw_crc = FLASH_crc_memory_calc();
		lfsm_user_set_tx_v0(&user_tx_v0);
		boot_status = idle;
		PIOS_LED_Off(LED1);
		user_tx_v0.payload.user.v.rsp.fwup_status.status = boot_status;
		break;
	case OPAHRS_MSG_V0_REQ_RESET:
		PIOS_DELAY_WaitmS(user_rx_v0.payload.user.v.req.reset.reset_delay_in_ms);
		PIOS_SYS_Reset();
		break;
	case OPAHRS_MSG_V0_REQ_VERSIONS:
		//PIOS_LED_On(LED1);
		opahrs_msg_v0_init_user_tx(&user_tx_v0, OPAHRS_MSG_V0_RSP_VERSIONS);
		user_tx_v0.payload.user.v.rsp.versions.bl_version = BOOTLOADER_VERSION;
		user_tx_v0.payload.user.v.rsp.versions.hw_version = (BOARD_TYPE << 8) | BOARD_REVISION;
		user_tx_v0.payload.user.v.rsp.versions.fw_crc = Fw_crc;
		lfsm_user_set_tx_v0(&user_tx_v0);
		break;
	case OPAHRS_MSG_V0_REQ_MEM_MAP:
		opahrs_msg_v0_init_user_tx(&user_tx_v0, OPAHRS_MSG_V0_RSP_MEM_MAP);
		user_tx_v0.payload.user.v.rsp.mem_map.density = HW_TYPE;
		user_tx_v0.payload.user.v.rsp.mem_map.rw_flags = (BOARD_READABLE
				| (BOARD_WRITABLA << 1));
		user_tx_v0.payload.user.v.rsp.mem_map.size_of_code_memory
				= SIZE_OF_CODE;
		user_tx_v0.payload.user.v.rsp.mem_map.size_of_description
				= SIZE_OF_DESCRIPTION;
		user_tx_v0.payload.user.v.rsp.mem_map.start_of_user_code
				= START_OF_USER_CODE;
		lfsm_user_set_tx_v0(&user_tx_v0);
		break;
	case OPAHRS_MSG_V0_REQ_SERIAL:
		opahrs_msg_v0_init_user_tx(&user_tx_v0, OPAHRS_MSG_V0_RSP_SERIAL);
		PIOS_SYS_SerialNumberGet(
				(char *) &(user_tx_v0.payload.user.v.rsp.serial.serial_bcd));
		lfsm_user_set_tx_v0(&user_tx_v0);
		break;
	case OPAHRS_MSG_V0_REQ_FWUP_STATUS:
		opahrs_msg_v0_init_user_tx(&user_tx_v0, OPAHRS_MSG_V0_RSP_FWUP_STATUS);
		user_tx_v0.payload.user.v.rsp.fwup_status.status = boot_status;
		lfsm_user_set_tx_v0(&user_tx_v0);
		break;
	case OPAHRS_MSG_V0_REQ_FWUP_DATA:
		PIOS_LED_On(LED1);
		opahrs_msg_v0_init_user_tx(&user_tx_v0, OPAHRS_MSG_V0_RSP_FWUP_STATUS);
		if (!(user_rx_v0.payload.user.v.req.fwup_data.adress
				< START_OF_USER_CODE)) {
			for (uint8_t x = 0; x
					< user_rx_v0.payload.user.v.req.fwup_data.size; ++x) {
				if (FLASH_ProgramWord(
						(user_rx_v0.payload.user.v.req.fwup_data.adress
								+ ((uint32_t)(x * 4))),
						user_rx_v0.payload.user.v.req.fwup_data.data[x])
						!= FLASH_COMPLETE) {
					boot_status = write_error;
					break;
				}
			}
		} else {
			boot_status = outside_dev_capabilities;
		}
		PIOS_LED_Off(LED1);
		user_tx_v0.payload.user.v.rsp.fwup_status.status = boot_status;
		lfsm_user_set_tx_v0(&user_tx_v0);
		break;
	case OPAHRS_MSG_V0_REQ_FWDN_DATA:
			opahrs_msg_v0_init_user_tx(&user_tx_v0, OPAHRS_MSG_V0_RSP_FWDN_DATA);
			uint32_t adr=user_rx_v0.payload.user.v.req.fwdn_data.adress;
			for(uint8_t x=0;x<4;++x)
			{
				user_tx_v0.payload.user.v.rsp.fw_dn.data[x]=*FLASH_If_Read(adr+x);
			}
			lfsm_user_set_tx_v0(&user_tx_v0);
		break;
	case OPAHRS_MSG_V0_REQ_FWUP_START:
		FLASH_Unlock();
		opahrs_msg_v0_init_user_tx(&user_tx_v0, OPAHRS_MSG_V0_RSP_FWUP_STATUS);
		user_tx_v0.payload.user.v.rsp.fwup_status.status = boot_status;
		lfsm_user_set_tx_v0(&user_tx_v0);
		PIOS_LED_On(LED1);
		if (FLASH_Start() == TRUE) {
			boot_status = started;
			PIOS_LED_Off(LED1);
		} else {
			boot_status = start_failed;
			break;
		}

		break;
	case OPAHRS_MSG_V0_REQ_BOOT:
		PIOS_DELAY_WaitmS(user_rx_v0.payload.user.v.req.boot.boot_delay_in_ms);
		FLASH_Lock();
		jump_to_app();
		break;
	default:
		break;
	}

	/* Finished processing the received message, requeue it */
	lfsm_user_set_rx_v0(&user_rx_v0);
	lfsm_user_done();
	return;
}
void jump_to_app() {
	//while(TRUE)
	//{
	//	PIOS_LED_Toggle(LED1);
	//	PIOS_DELAY_WaitmS(1000);
	//}
	PIOS_LED_On(LED1);
	if (((*(__IO uint32_t*) START_OF_USER_CODE) & 0x2FFE0000) == 0x20000000) { /* Jump to user application */
		FLASH_Lock();
		RCC_APB2PeriphResetCmd(0xffffffff, ENABLE);
		RCC_APB1PeriphResetCmd(0xffffffff, ENABLE);
		RCC_APB2PeriphResetCmd(0xffffffff, DISABLE);
		RCC_APB1PeriphResetCmd(0xffffffff, DISABLE);
		//_SetCNTR(0); // clear interrupt mask
		//_SetISTR(0); // clear all requests

		JumpAddress = *(__IO uint32_t*) (START_OF_USER_CODE + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) START_OF_USER_CODE);
		Jump_To_Application();
	} else {
		boot_status = jump_failed;
		return;
	}
}
