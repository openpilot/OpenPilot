/**
 ******************************************************************************
 *
 * @file       pios_board.h   
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board hardware for the OpenPilot Version 1.1 hardware.
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


#ifndef PIOS_BOARD_H
#define PIOS_BOARD_H


//------------------------
// PIOS_LED
//------------------------
#define PIOS_LED_LED1_GPIO_PORT			GPIOC
#define PIOS_LED_LED1_GPIO_PIN			GPIO_Pin_12
#define PIOS_LED_LED1_GPIO_CLK			RCC_APB2Periph_GPIOC
#define PIOS_LED_LED2_GPIO_PORT			GPIOC
#define PIOS_LED_LED2_GPIO_PIN			GPIO_Pin_13
#define PIOS_LED_LED2_GPIO_CLK			RCC_APB2Periph_GPIOC
#define PIOS_LED_NUM				2
#define PIOS_LED_PORTS				{ PIOS_LED_LED1_GPIO_PORT, PIOS_LED_LED2_GPIO_PORT }
#define PIOS_LED_PINS				{ PIOS_LED_LED1_GPIO_PIN, PIOS_LED_LED2_GPIO_PIN }
#define PIOS_LED_CLKS				{ PIOS_LED_LED1_GPIO_CLK, PIOS_LED_LED2_GPIO_CLK }

//-------------------------
// PIOS_USART1 (TELEM)
//-------------------------
#define PIOS_USART1_ENABLED			1
#define PIOS_USART1_USART			USART2
#define PIOS_USART1_GPIO_PORT			GPIOA
#define PIOS_USART1_RX_PIN			GPIO_Pin_3
#define PIOS_USART1_TX_PIN			GPIO_Pin_2
#define PIOS_USART1_REMAP_FUNC			{ }
#define PIOS_USART1_IRQ_CHANNEL			USART2_IRQn
#define PIOS_USART1_IRQHANDLER_FUNC		void USART2_IRQHandler(void)
#define PIOS_USART1_CLK_FUNC			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE)
#define PIOS_USART1_NVIC_PRIO			PIOS_IRQ_PRIO_HIGHEST
#define PIOS_USART1_BAUDRATE			115200

//-------------------------
// PIOS_USART2 (GPS)
//-------------------------
#define PIOS_USART2_ENABLED			0
#define PIOS_USART2_USART       		USART3
#define PIOS_USART2_GPIO_PORT			GPIOC
#define PIOS_USART2_RX_PIN      		GPIO_Pin_11
#define PIOS_USART2_TX_PIN      		GPIO_Pin_10
#define PIOS_USART2_REMAP_FUNC			{ GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE); }
#define PIOS_USART2_IRQ_CHANNEL			USART3_IRQn
#define PIOS_USART2_IRQHANDLER_FUNC		void USART3_IRQHandler(void)
#define PIOS_USART2_CLK_FUNC			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE)
#define PIOS_USART2_NVIC_PRIO			PIOS_IRQ_PRIO_HIGHEST
#define PIOS_USART2_BAUDRATE			57600

//-------------------------
//  PIOS_USART3 (AUX) (RX5/RX6)
//-------------------------
#define PIOS_USART3_ENABLED			0
#define PIOS_USART3_USART			USART1
#define PIOS_USART3_GPIO_PORT			GPIOA
#define PIOS_USART3_RX_PIN			GPIO_Pin_10
#define PIOS_USART3_TX_PIN			GPIO_Pin_9
#define PIOS_USART3_REMAP_FUNC			{ }
#define PIOS_USART3_IRQ_CHANNEL			USART1_IRQn
#define PIOS_USART3_IRQHANDLER_FUNC		void USART1_IRQHandler(void)
#define PIOS_USART3_CLK_FUNC			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE)
#define PIOS_USART3_NVIC_PRIO			PIOS_IRQ_PRIO_HIGH
#define PIOS_USART3_BAUDRATE			57600

//-------------------------
// PIOS_USART
//-------------------------
#define PIOS_USART_NUM				1
#define PIOS_USART_RX_BUFFER_SIZE		64
#define PIOS_USART_TX_BUFFER_SIZE		64
#define PIOS_COM_DEBUG_PORT			USART_1

//-------------------------
// Delay Timer
//-------------------------
#define PIOS_DELAY_TIMER			TIM2
#define PIOS_DELAY_TIMER_RCC_FUNC		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE)

//-------------------------
// System Settings
//-------------------------
#define PIOS_MASTER_CLOCK			72000000
#define PIOS_PERIPHERAL_CLOCK			(PIOS_MASTER_CLOCK / 2)
#define PIOS_NVIC_VECTTAB_FLASH			((uint32_t)0x08000000)

//-------------------------
// Interrupt Priorities
//-------------------------
#define PIOS_IRQ_PRIO_LOW			12		// lower than RTOS
#define PIOS_IRQ_PRIO_MID			8		// higher than RTOS
#define PIOS_IRQ_PRIO_HIGH			5		// for SPI, ADC, I2C etc...
#define PIOS_IRQ_PRIO_HIGHEST			4 		// for USART etc...

//-------------------------  
// USB
//-------------------------
#define PIOS_USB_ENABLED			1
#define PIOS_USB_DETECT_GPIO_PORT		GPIOC
#define PIOS_USB_DETECT_GPIO_PIN		GPIO_Pin_4
#define PIOS_IRQ_USB_PRIORITY			PIOS_IRQ_PRIO_MID

#endif /* PIOS_BOARD_H */
