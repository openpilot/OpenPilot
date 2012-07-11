/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SYS System Functions
 * @brief PIOS System Initialization code
 * @{
 *
 * @file       pios_sys.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
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

/* Private Function Prototypes */
void NVIC_Configuration(void);
void SysTick_Handler(void);

/* Local Macros */
#define MEM8(addr)  (*((volatile uint8_t  *)(addr)))
#define MEM16(addr)  (*((volatile uint16_t  *)(addr)))
#define MEM32(addr)  (*((volatile uint32_t  *)(addr)))

/**
* Initialises all system peripherals
*/
void PIOS_SYS_Init(void)
{
	/* Setup STM32 system (RCC, clock, PLL and Flash configuration) - CMSIS Function */
	SystemInit();

	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
			       RCC_APB2Periph_AFIO, ENABLE);

#if (PIOS_USB_ENABLED)
	/*  Ensure that pull-up is active on detect pin */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = PIOS_USB_DETECT_GPIO_PIN;
	GPIO_Init(PIOS_USB_DETECT_GPIO_PORT, &GPIO_InitStructure);
#endif

	/* Initialise Basic NVIC */
	NVIC_Configuration();

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

	RCC_APB2PeriphResetCmd(0xffffffff, DISABLE);
	RCC_APB1PeriphResetCmd(0xffffffff, DISABLE);
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
	return ((uint32_t) MEM16(0x1FFFF7E0) * 1000);
}

/**
* Returns the serial number as a string
* param[out] uint8_t pointer to a string which can store at least 12 bytes
* (12 bytes returned for STM32)
* return < 0 if feature not supported
*/
int32_t PIOS_SYS_SerialNumberGetBinary(uint8_t *array)
{
	int i;

	/* Stored in the so called "electronic signature" */
	for (i = 0; i < PIOS_SYS_SERIAL_NUM_BINARY_LEN; ++i) {
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
	for (i = 0; i < PIOS_SYS_SERIAL_NUM_ASCII_LEN; ++i) {
		uint8_t b = MEM8(0x1ffff7e8 + (i / 2));
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
void NVIC_Configuration(void)
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
