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
#include "osd_bl.h"
#include <pios_board_info.h>
#include "bl_fsm.h"		/* lfsm_state */
//#include "stm32f2xx_flash.h"

extern void PIOS_Board_Init(void);

#define NSS_HOLD_STATE       ((GPIOB->IDR & GPIO_Pin_12) ? 0 : 1)
enum bootloader_status boot_status;
/* Private typedef -----------------------------------------------------------*/
typedef void
(*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress;
/* Function Prototypes */
//void
//process_spi_request(void);
void
jump_to_app();
uint32_t Fw_crc;
/**
 * @brief Bootloader Main function
 */
int main() {
	/* Brings up System using CMSIS functions, enables the LEDs. */
	PIOS_SYS_Init();
	PIOS_Board_Init();

	jump_to_app();
	return 0;
}


void jump_to_app() {
	const struct pios_board_info * bdinfo = &pios_board_info_blob;

	PIOS_LED_On(PIOS_LED_HEARTBEAT);
	if (((*(__IO uint32_t*) bdinfo->fw_base) & 0x2FFE0000) == 0x20000000) { /* Jump to user application */
		FLASH_Lock();
		RCC_APB2PeriphResetCmd(0xffffffff, ENABLE);
		RCC_APB1PeriphResetCmd(0xffffffff, ENABLE);
		RCC_APB2PeriphResetCmd(0xffffffff, DISABLE);
		RCC_APB1PeriphResetCmd(0xffffffff, DISABLE);
		//_SetCNTR(0); // clear interrupt mask
		//_SetISTR(0); // clear all requests

		JumpAddress = *(__IO uint32_t*) (bdinfo->fw_base + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) bdinfo->fw_base);
		Jump_To_Application();
	} else {
		boot_status = jump_failed;
		return;
	}
}
