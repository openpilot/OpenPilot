/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
 * File Name          : main.c
 * Author             : MCD Application Team
 * Version            : V3.2.1
 * Date               : 07/05/2010
 * Description        : Custom HID demo main file
 ********************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "stm32_eval.h"
#include "common.h"
#include "platform_config.h"
#include "stopwatch.h"
extern void FLASH_Download();
#define BSL_HOLD_STATE       ((USB_DISCONNECT->IDR & USB_DISCONNECT_PIN) ? 0 : 1)

/* Private typedef -----------------------------------------------------------*/
typedef void (*pFunction)(void);
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
pFunction Jump_To_Application;
uint32_t JumpAddress;

uint32_t cnt;
uint32_t pwm_period;
uint32_t pwm_sweep_steps;

/* Extern variables ----------------------------------------------------------*/
uint8_t DeviceState;
uint8_t JumpToApp = 0;
/* Private function prototypes -----------------------------------------------*/
void Delay(__IO uint32_t nCount);
void DelayWithDown(__IO uint32_t nCount);
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
 * Function Name  : main.
 * Description    : main routine.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
int main(void) {

	Set_System();
	if (BSL_HOLD_STATE==0) {

		USB_Interrupts_Config();
		Set_USBClock();
		USB_Init();
		DeviceState = idle;
		STOPWATCH_Init(100);
	}
	else
		JumpToApp = TRUE;
	STOPWATCH_Reset();

	while (JumpToApp == 0) {

		cnt = STOPWATCH_ValueGet(); // the reference counter (incremented each 100 uS)
		pwm_period = 50; // *100 uS -> 5 mS
		pwm_sweep_steps =100; // * 5 mS -> 500 mS
		uint32_t pwm_duty = ((cnt / pwm_period) % pwm_sweep_steps)
				/ (pwm_sweep_steps / pwm_period);
		if ((cnt % (2 * pwm_period * pwm_sweep_steps)) > pwm_period
				* pwm_sweep_steps)
			pwm_duty = pwm_period - pwm_duty; // negative direction each 50*100 ticks
		uint32_t led_on = ((cnt % pwm_period) > pwm_duty) ? 1 : 0;
		if(led_on==0)
			STM_EVAL_LEDOn(LED1);
		else
			STM_EVAL_LEDOff(LED1);
		if(STOPWATCH_ValueGet()>100*pwm_period*pwm_sweep_steps)
			STOPWATCH_Reset();
		if(STOPWATCH_ValueGet()>50000)
			JumpToApp=TRUE;
		//DelayWithDown(10);//1000000);
	}
	if (((*(__IO uint32_t*) StartOfUserCode) & 0x2FFE0000) == 0x20000000) { /* Jump to user application */
		FLASH_Lock();
		RCC_APB2PeriphResetCmd(0xffffffff, ENABLE);
		RCC_APB1PeriphResetCmd(0xffffffff, ENABLE);
		RCC_APB2PeriphResetCmd(0xffffffff, DISABLE);
		RCC_APB1PeriphResetCmd(0xffffffff, DISABLE);
		_SetCNTR(0); // clear interrupt mask
		_SetISTR(0); // clear all requests

		JumpAddress = *(__IO uint32_t*) (StartOfUserCode + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) StartOfUserCode);
		Jump_To_Application();
	}

	while (1) {
		STM_EVAL_LEDToggle(LED1);
		STM_EVAL_LEDToggle(LED2);
		Delay(1000000);
	}
}

/*******************************************************************************
 * Function Name  : Delay
 * Description    : Inserts a delay time.
 * Input          : nCount: specifies the delay time length.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void Delay(__IO uint32_t nCount) {
	for (; nCount != 0; nCount--) {

	}
}
/*******************************************************************************
 * Function Name  : Delay
 * Description    : Inserts a delay time.
 * Input          : nCount: specifies the delay time length.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void DelayWithDown(__IO uint32_t nCount) {
	for (; nCount != 0; nCount--) {
		for (__IO uint32_t delay = DownloadDelay; delay != 0; delay--) {
		}
		FLASH_Download();
	}
}
#ifdef  USE_FULL_ASSERT
/*******************************************************************************
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 *******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1)
	{
	}
}
#endif

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
