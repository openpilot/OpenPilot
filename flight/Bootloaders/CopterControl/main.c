/**
 ******************************************************************************
 * @addtogroup CopterControlBL CopterControl BootLoader
 * @brief These files contain the code to the CopterControl Bootloader.
 *
 * @{
 * @file       main.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      This is the file with the main function of the OpenPilot BootLoader
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
/* Bootloader Includes */
#include <pios.h>
#include "stopwatch.h"
#include "op_dfu.h"
#include "usb_lib.h"
#include "pios_iap.h"
#include "fifo_buffer.h"
/* Prototype of PIOS_Board_Init() function */
extern void PIOS_Board_Init(void);
extern void FLASH_Download();
#define BSL_HOLD_STATE ((PIOS_USB_DETECT_GPIO_PORT->IDR & PIOS_USB_DETECT_GPIO_PIN) ? 0 : 1)

/* Private typedef -----------------------------------------------------------*/
typedef void (*pFunction)(void);
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
pFunction Jump_To_Application;
uint32_t JumpAddress;

/// LEDs PWM
uint32_t period1 = 50; // *100 uS -> 5 mS
uint32_t sweep_steps1 = 100; // * 5 mS -> 500 mS
uint32_t period2 = 50; // *100 uS -> 5 mS
uint32_t sweep_steps2 = 100; // * 5 mS -> 500 mS


////////////////////////////////////////
uint8_t tempcount = 0;

/* Extern variables ----------------------------------------------------------*/
DFUStates DeviceState;
int16_t status = 0;
uint8_t JumpToApp = FALSE;
uint8_t GO_dfu = FALSE;
uint8_t USB_connected = FALSE;
uint8_t User_DFU_request = FALSE;
static uint8_t mReceive_Buffer[64];
/* Private function prototypes -----------------------------------------------*/
uint32_t LedPWM(uint32_t pwm_period, uint32_t pwm_sweep_steps, uint32_t count);
uint8_t processRX();
void jump_to_app();

#define BLUE LED1
#define LED_PWM_TIMER	TIM1
int main() {
	PIOS_SYS_Init();
	if (BSL_HOLD_STATE == 0)
		USB_connected = TRUE;

	PIOS_IAP_Init();

	if (PIOS_IAP_CheckRequest() == TRUE) {
		PIOS_Board_Init();
		PIOS_DELAY_WaitmS(1000);
		User_DFU_request = TRUE;
		PIOS_IAP_ClearRequest();
	}

	GO_dfu = (USB_connected == TRUE) || (User_DFU_request == TRUE);

	if (GO_dfu == TRUE) {
		PIOS_Board_Init();
		if (User_DFU_request == TRUE)
			DeviceState = DFUidle;
		else
			DeviceState = BLidle;
		STOPWATCH_Init(100, LED_PWM_TIMER);
	} else
		JumpToApp = TRUE;

	STOPWATCH_Reset(LED_PWM_TIMER);

	while (TRUE) {
		if (JumpToApp == TRUE)
			jump_to_app();

		switch (DeviceState) {
		case Last_operation_Success:
		case uploadingStarting:
		case DFUidle:
			period1 = 50;
			sweep_steps1 = 100;
			PIOS_LED_Off(BLUE);
			period2 = 0;
			break;
		case uploading:
			period1 = 50;
			sweep_steps1 = 100;
			period2 = 25;
			sweep_steps2 = 50;
			break;
		case downloading:
			period1 = 25;
			sweep_steps1 = 50;
			PIOS_LED_Off(BLUE);
			period2 = 0;
			break;
		case BLidle:
			period1 = 0;
			PIOS_LED_On(BLUE);
			period2 = 0;
			break;
		default://error
			period1 = 50;
			sweep_steps1 = 100;
			period2 = 50;
			sweep_steps2 = 100;
		}

		if (period1 != 0) {
			if (LedPWM(period1, sweep_steps1, STOPWATCH_ValueGet(LED_PWM_TIMER)))
				PIOS_LED_On(BLUE);
			else
				PIOS_LED_Off(BLUE);
		} else
			PIOS_LED_On(BLUE);

		if (period2 != 0) {
			if (LedPWM(period2, sweep_steps2, STOPWATCH_ValueGet(LED_PWM_TIMER)))
				PIOS_LED_On(BLUE);
			else
				PIOS_LED_Off(BLUE);
		} else
			PIOS_LED_Off(BLUE);

		if (STOPWATCH_ValueGet(LED_PWM_TIMER) > 100 * 50 * 100)
			STOPWATCH_Reset(LED_PWM_TIMER);
		if ((STOPWATCH_ValueGet(LED_PWM_TIMER) > 60000) && (DeviceState
				== BLidle))
			JumpToApp = TRUE;

		processRX();
		DataDownload(start);
	}
}

void jump_to_app() {
	if (((*(__IO uint32_t*) START_OF_USER_CODE) & 0x2FFE0000) == 0x20000000) { /* Jump to user application */
		FLASH_Lock();
		RCC_APB2PeriphResetCmd(0xffffffff, ENABLE);
		RCC_APB1PeriphResetCmd(0xffffffff, ENABLE);
		RCC_APB2PeriphResetCmd(0xffffffff, DISABLE);
		RCC_APB1PeriphResetCmd(0xffffffff, DISABLE);
		_SetCNTR(0); // clear interrupt mask
		_SetISTR(0); // clear all requests
		JumpAddress = *(__IO uint32_t*) (START_OF_USER_CODE + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) START_OF_USER_CODE);
		Jump_To_Application();
	} else {
		DeviceState = failed_jump;
		return;
	}
}
uint32_t LedPWM(uint32_t pwm_period, uint32_t pwm_sweep_steps, uint32_t count) {
	uint32_t pwm_duty = ((count / pwm_period) % pwm_sweep_steps)
			/ (pwm_sweep_steps / pwm_period);
	if ((count % (2 * pwm_period * pwm_sweep_steps)) > pwm_period
			* pwm_sweep_steps)
		pwm_duty = pwm_period - pwm_duty; // negative direction each 50*100 ticks
	return ((count % pwm_period) > pwm_duty) ? 1 : 0;
}

uint8_t processRX() {
	while (PIOS_COM_ReceiveBufferUsed(PIOS_COM_TELEM_USB) >= 63) {
		for (int32_t x = 0; x < 63; ++x) {
			mReceive_Buffer[x] = PIOS_COM_ReceiveBuffer(PIOS_COM_TELEM_USB);
		}
		processComand(mReceive_Buffer);
	}
	return TRUE;
}

