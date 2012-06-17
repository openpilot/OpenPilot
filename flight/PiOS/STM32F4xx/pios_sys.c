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
 * 			The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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
	RCC_AHB1PeriphClockCmd(
			       RCC_AHB1Periph_GPIOA |
			       RCC_AHB1Periph_GPIOB |
			       RCC_AHB1Periph_GPIOC |
			       RCC_AHB1Periph_GPIOD |
			       RCC_AHB1Periph_GPIOE |
			       RCC_AHB1Periph_GPIOF |
			       RCC_AHB1Periph_GPIOG |
			       RCC_AHB1Periph_GPIOH |
			       RCC_AHB1Periph_GPIOI |
			       RCC_AHB1Periph_CRC |
			       RCC_AHB1Periph_FLITF |
			       RCC_AHB1Periph_SRAM1 |
			       RCC_AHB1Periph_SRAM2 |
			       RCC_AHB1Periph_BKPSRAM |
			       RCC_AHB1Periph_DMA1 |
			       RCC_AHB1Periph_DMA2 |
			       //RCC_AHB1Periph_ETH_MAC |			No ethernet
			       //RCC_AHB1Periph_ETH_MAC_Tx |
			       //RCC_AHB1Periph_ETH_MAC_Rx |
			       //RCC_AHB1Periph_ETH_MAC_PTP |
			       //RCC_AHB1Periph_OTG_HS |			No high-speed USB (requires external PHY)
			       //RCC_AHB1Periph_OTG_HS_ULPI |		No ULPI PHY (see above)
			0, ENABLE);
	RCC_AHB2PeriphClockCmd(
			       //RCC_AHB2Periph_DCMI |				No camera   @todo might make sense later for basic vision support?
			       //RCC_AHB2Periph_CRYP |				No crypto
			       //RCC_AHB2Periph_HASH |				No hash generator
			       //RCC_AHB2Periph_RNG |				No random numbers @todo might be good to have later if entropy is desired
			       //RCC_AHB2Periph_OTG_FS |
			0, ENABLE);
	RCC_AHB3PeriphClockCmd(
			       //RCC_AHB3Periph_FSMC |				No external static memory
			0, ENABLE);
	RCC_APB1PeriphClockCmd(
			       RCC_APB1Periph_TIM2 |
			       RCC_APB1Periph_TIM3 |
			       RCC_APB1Periph_TIM4 |
			       RCC_APB1Periph_TIM5 |
			       RCC_APB1Periph_TIM6 |
			       RCC_APB1Periph_TIM7 |
			       RCC_APB1Periph_TIM12 |
			       RCC_APB1Periph_TIM13 |
			       RCC_APB1Periph_TIM14 |
			       RCC_APB1Periph_WWDG |
			       RCC_APB1Periph_SPI2 |
			       RCC_APB1Periph_SPI3 |
			       RCC_APB1Periph_USART2 |
			       RCC_APB1Periph_USART3 |
			       RCC_APB1Periph_UART4 |
			       RCC_APB1Periph_UART5 |
			       RCC_APB1Periph_I2C1 |
			       RCC_APB1Periph_I2C2 |
			       RCC_APB1Periph_I2C3 |
			       RCC_APB1Periph_CAN1 |
			       RCC_APB1Periph_CAN2 |
			       RCC_APB1Periph_PWR |
			       RCC_APB1Periph_DAC |
			0, ENABLE);

	RCC_APB2PeriphClockCmd(
			       RCC_APB2Periph_TIM1 |
			       RCC_APB2Periph_TIM8 |
			       RCC_APB2Periph_USART1 |
			       RCC_APB2Periph_USART6 |
			       RCC_APB2Periph_ADC |
			       RCC_APB2Periph_ADC1 |
			       RCC_APB2Periph_ADC2 |
			       RCC_APB2Periph_ADC3 |
			       RCC_APB2Periph_SDIO |
			       RCC_APB2Periph_SPI1 |
			       RCC_APB2Periph_SYSCFG |
			       RCC_APB2Periph_TIM9 |
			       RCC_APB2Periph_TIM10 |
			       RCC_APB2Periph_TIM11 |
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
	GPIO_InitStructure.GPIO_Pin &= ~(GPIO_Pin_3 | GPIO_Pin_4);					// leave JTAG pins alone
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_Init(GPIOH, &GPIO_InitStructure);
	GPIO_Init(GPIOI, &GPIO_InitStructure);
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
#if (PIOS_LED_NUM == 1)
	PIOS_LED_Off(LED1);
#elif (PIOS_LED_NUM == 2)
	PIOS_LED_Off(LED1);
	PIOS_LED_Off(LED2);
#endif

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
	return ((uint32_t) MEM_SIZE);	// it might be possible to locate in the OTP area, but haven't looked and not documented
}

/**
 * Returns the serial number as a string
 * param[out] str pointer to a string which can store at least 32 digits + zero terminator!
 * (24 digits returned for STM32)
 * return < 0 if feature not supported
 */
int32_t PIOS_SYS_SerialNumberGetBinary(uint8_t *array)
{
	int i;
	
	/* Stored in the so called "electronic signature" */
	for (i = 0; i < 12; ++i) {
		uint8_t b = MEM8(0x1ffff7e8 + i);
		
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
	for (i = 0; i < 24; ++i) {
		uint8_t b = *(uint8_t *)(0x1fff7a10 + i/2);
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
#if defined(PIOS_LED_ALARM)
	PIOS_LED_On(PIOS_LED_ALARM);
#endif
#if defiend(PIOS_LED_HEARTBEAT)
	PIOS_LED_Off(PIOS_LED_HEARTBEAT);
#endif

	/* Infinite loop */
	while (1) {
		PIOS_LED_Toggle(LED1);
		PIOS_LED_Toggle(LED2);
		for (int i = 0; i < 1000000; i++) ;
	}
}
#endif

#endif

/**
  * @}
  * @}
  */
