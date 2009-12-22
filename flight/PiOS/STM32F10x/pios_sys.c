/**
 ******************************************************************************
 *
 * @file       pios_sys.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      Sets up basic system hardware, functions are called from Main.
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_SYS System Functions
 * @{
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

/* Private Function Prototypes */
void NVIC_Configuration(void);

/* File system object for each logical drive */
static FATFS Fatfs[_DRIVES];

/**
* Initializes all system peripherals
*/
void PIOS_SYS_Init(void)
{
	/* Setup STM32 system (RCC, clock, PLL and Flash configuration) - CMSIS Function */
	SystemInit();
	
	/* Initialize Basic NVIC */
	NVIC_Configuration();
	
	/* Initialize LEDs */
	PIOS_LED_Init();
	
	/* Initialize FatFS disk */
	if(f_mount(0, &Fatfs[0]) != FR_OK) {
		/* Failed to mount MicroSD filesystem, flash LED1 forever */
		while(1) {
			for(int i = 0; i < 100000; i++);
			PIOS_LED_Toggle(LED1);
		}
	}		
}


/**
* Configures Vector Table base location and SysTick
*/
void NVIC_Configuration(void)
{
	/* Set the Vector Table base address as specified in .ld file */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);

	/* 4 bits for Interupt priorities so no sub priorities */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
}

#ifdef  USE_FULL_ASSERT
/**
* Reports the name of the source file and the source line number
*   where the assert_param error has occurred.
* \param[in]  file pointer to the source file name
* \param[in]  line assert_param error line source number
* \retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
	/* When serial debugging is implemented, use something like this. */
	/* printf("Wrong parameters value: file %s on line %d\r\n", file, line); */

	/* Setup the LEDs to Alternate */
	PIOS_LED_On(LED1);
	PIOS_LED_Off(LED2);

	/* Infinite loop */
	while (1)
	{
		for(int i = 0; i < 1000; i++);
		PIOS_LED_Toggle(LED1);
		for(int i = 0; i < 1000; i++);
		PIOS_LED_Toggle(LED2);
	}
}
#endif
