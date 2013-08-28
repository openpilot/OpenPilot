/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SYS System Functions
 * @brief PIOS System Initialization code
 * @{
 *
 * @file       pios_sys.c  
 * @author     Michael Smith Copyright (C) 2011
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2012-2013
 * @brief      Sets up basic STM32 system hardware, functions are called from Main.
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_SYS)

#define MEM8(addr)  (*((volatile uint8_t  *)(addr)))

/* Private Function Prototypes */
static void NVIC_Configuration(void);

/**
* Initialises all system peripherals
*/
void PIOS_SYS_Init(void)
{
	/* Setup STM32 system (RCC, clock, PLL and Flash configuration) - CMSIS Function */
	SystemInit();
	SystemCoreClockUpdate();	/* update SystemCoreClock for use elsewhere */

	/*
	 * @todo might make sense to fetch the bus clocks and save them somewhere to avoid
	 * having to use the clunky get-all-clocks API everytime we need one.
	 */

	/* Initialise Basic NVIC */
	/* do this early to ensure that we take exceptions in the right place */
	NVIC_Configuration();

	/* Init the delay system */
	PIOS_DELAY_Init();

	/*
	 * Turn on all the peripheral clocks.
	 * Micromanaging clocks makes no sense given the power situation in the system, so
	 * light up everything we might reasonably use here and just leave it on.
	 */
	RCC_AHBPeriphClockCmd(
			       RCC_AHBPeriph_GPIOA |
			       RCC_AHBPeriph_GPIOB |
			       RCC_AHBPeriph_GPIOC |
			       RCC_AHBPeriph_GPIOD |
			       RCC_AHBPeriph_GPIOE |
			       RCC_AHBPeriph_GPIOF |
			       RCC_AHBPeriph_TS |
			       RCC_AHBPeriph_CRC |
			       RCC_AHBPeriph_FLITF |
			       RCC_AHBPeriph_SRAM |
			       RCC_AHBPeriph_DMA2 |
			       RCC_AHBPeriph_DMA1 |
			       RCC_AHBPeriph_ADC34 |
			       RCC_AHBPeriph_ADC12 |
			0, ENABLE);

	RCC_APB1PeriphClockCmd(
			       RCC_APB1Periph_TIM2 |
			       RCC_APB1Periph_TIM3 |
			       RCC_APB1Periph_TIM4 |
			       RCC_APB1Periph_TIM6 |
			       RCC_APB1Periph_TIM7 |
			       RCC_APB1Periph_WWDG |
			       RCC_APB1Periph_SPI2 |
			       RCC_APB1Periph_SPI3 |
			       RCC_APB1Periph_USART2 |
			       RCC_APB1Periph_USART3 |
			       RCC_APB1Periph_UART4 |
			       RCC_APB1Periph_UART5 |
			       RCC_APB1Periph_I2C1 |
			       RCC_APB1Periph_I2C2 |
			       RCC_APB1Periph_USB |
			       RCC_APB1Periph_CAN1 |
			       RCC_APB1Periph_PWR |
			       RCC_APB1Periph_DAC |
			0, ENABLE);

	RCC_APB2PeriphClockCmd(
			       RCC_APB2Periph_TIM1 |
			       RCC_APB2Periph_TIM8 |
			       RCC_APB2Periph_TIM15 |
			       RCC_APB2Periph_TIM16 |
			       RCC_APB2Periph_TIM17 |
			       RCC_APB2Periph_USART1 |
			       RCC_APB2Periph_SPI1 |
			       RCC_APB2Periph_SYSCFG |
			0, ENABLE);

	/*
	 * Configure all pins as input / pullup to avoid issues with
	 * uncommitted pins, excepting special-function pins that we need to
	 * remain as-is.
	 */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	// default is un-pulled input

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
#if (PIOS_USB_ENABLED)
	GPIO_InitStructure.GPIO_Pin &= ~(GPIO_Pin_11 | GPIO_Pin_12);				// leave USB D+/D- alone
#endif
	GPIO_InitStructure.GPIO_Pin &= ~(GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);	// leave JTAG pins alone
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_Init(GPIOF, &GPIO_InitStructure);
}

/**
* Shutdown PIOS and reset the microcontroller:<BR>
* <UL>
*   <LI>Disable all RTOS tasks
*   <LI>Disable all interrupts
*   <LI>Turn off all board LEDs
*   <LI>Reset STM32
* </UL>
* \return < 0 if reset failed
*/
int32_t PIOS_SYS_Reset(void)
{
	/* Disable all RTOS tasks */
#if defined(PIOS_INCLUDE_FREERTOS)
	/* port specific FreeRTOS function to disable tasks (nested) */
	portENTER_CRITICAL();
#endif

	// disable all interrupts
	PIOS_IRQ_Disable();

	// turn off all board LEDs
#if defined(PIOS_LED_HEARTBEAT)
	PIOS_LED_Off(PIOS_LED_HEARTBEAT);
#endif	/* PIOS_LED_HEARTBEAT */
#if defined(PIOS_LED_ALARM)
	PIOS_LED_Off(PIOS_LED_ALARM);
#endif	/* PIOS_LED_ALARM */

	/* XXX F10x port resets most (but not all) peripherals ... do we care? */

	/* Reset STM32 */
	NVIC_SystemReset();

	while (1) ;

	/* We will never reach this point */
	return -1;
}

/**
* Returns the CPU's flash size (in bytes)
*/
uint32_t PIOS_SYS_getCPUFlashSize(void)
{
	return ((uint32_t) MEM_SIZE);	// FIXME: it might be possible to locate in the OTP area, but haven't looked and not documented
}

/**
 * Returns the serial number as byte array
 * param[out] pointer to a byte array which can store at least 12 bytes
 * (12 bytes returned for STM32)
 * return < 0 if feature not supported
 */
int32_t PIOS_SYS_SerialNumberGetBinary(uint8_t *array)
{
	int i;
	
	/* Stored in the so called "electronic signature" */
	for (i = 0; i < PIOS_SYS_SERIAL_NUM_BINARY_LEN; ++i) {
		uint8_t b = MEM8(0x1ffff7ac + i);
		
		array[i] = b;
	}
	
	/* No error */
	return 0;
}

/**
* Returns the serial number as a string
* param[out] str pointer to a string which can store at least 32 digits + zero terminator!
* (24 digits returned for STM32)
* return < 0 if feature not supported
*/
int32_t PIOS_SYS_SerialNumberGet(char *str)
{
	int i;

	/* Stored in the so called "electronic signature" */
	for (i = 0; i < PIOS_SYS_SERIAL_NUM_ASCII_LEN; ++i) {
		uint8_t b = MEM8(0x1ffff7ac + (i / 2));
		if (!(i & 1))
			b >>= 4;
		b &= 0x0f;

		str[i] = ((b > 9) ? ('A' - 10) : '0') + b;
	}
	str[i] = '\0';

	/* No error */
	return 0;
}

/**
* Configures Vector Table base location and SysTick
*/
static void NVIC_Configuration(void)
{
	/* Set the Vector Table base address as specified in .ld file */
	extern void *pios_isr_vector_table_base;
	NVIC_SetVectorTable((uint32_t)&pios_isr_vector_table_base, 0x0);

	/* 4 bits for Interrupt priorities so no sub priorities */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}

#ifdef USE_FULL_ASSERT
/**
* Reports the name of the source file and the source line number
*   where the assert_param error has occurred.
* \param[in]  file pointer to the source file name
* \param[in]  line assert_param error line source number
* \retval None
*/
void assert_failed(uint8_t * file, uint32_t line)
{
	/* When serial debugging is implemented, use something like this. */
	/* printf("Wrong parameters value: file %s on line %d\r\n", file, line); */

	/* Setup the LEDs to Alternate */
#if defined(PIOS_LED_HEARTBEAT)
	PIOS_LED_On(PIOS_LED_HEARTBEAT);
#endif	/* PIOS_LED_HEARTBEAT */
#if defined(PIOS_LED_ALARM)
	PIOS_LED_Off(PIOS_LED_ALARM);
#endif	/* PIOS_LED_ALARM */

	/* Infinite loop */
	while (1) {
#if defined(PIOS_LED_HEARTBEAT)
		PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
#endif	/* PIOS_LED_HEARTBEAT */
#if defined(PIOS_LED_ALARM)
		PIOS_LED_Toggle(PIOS_LED_ALARM);
#endif	/* PIOS_LED_ALARM */
		for (int i = 0; i < 1000000; i++) ;
	}
}
#endif

#endif

/**
  * @}
  * @}
  */
