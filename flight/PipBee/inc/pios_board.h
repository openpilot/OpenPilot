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
// Timers and Channels Used
//------------------------
/*
Timer | Channel 1 | Channel 2 | Channel 3 | Channel 4
------+-----------+-----------+-----------+----------
TIM1  |           |           |           |
TIM2  | --------------- PIOS_DELAY -----------------
TIM3  |           |           |           |
TIM4  |           |           |           |
TIM5  |           |           |           |
TIM6  |           |           |           |
TIM7  |           |           |           |
TIM8  |           |           |           |
------+-----------+-----------+-----------+----------
*/

//------------------------
// DMA Channels Used
//------------------------
/* Channel 1  - 				*/
/* Channel 2  - 				*/
/* Channel 3  - 				*/
/* Channel 4  - 				*/
/* Channel 5  - 				*/
/* Channel 6  - 				*/
/* Channel 7  - 				*/
/* Channel 8  - 				*/
/* Channel 9  - 				*/
/* Channel 10 - 				*/
/* Channel 11 - 				*/
/* Channel 12 - 				*/

//------------------------
// PIOS_LED
//------------------------
#define PIOS_LED_LED1_GPIO_PORT			GPIOA					// USB Activity LED
#define PIOS_LED_LED1_GPIO_PIN			GPIO_Pin_3
#define PIOS_LED_LED1_GPIO_CLK			RCC_APB2Periph_GPIOA

#define PIOS_LED_LED2_GPIO_PORT			GPIOB					// LINK LED
#define PIOS_LED_LED2_GPIO_PIN			GPIO_Pin_5
#define PIOS_LED_LED2_GPIO_CLK			RCC_APB2Periph_GPIOB

#define PIOS_LED_LED3_GPIO_PORT			GPIOB					// RX LED
#define PIOS_LED_LED3_GPIO_PIN			GPIO_Pin_6
#define PIOS_LED_LED3_GPIO_CLK			RCC_APB2Periph_GPIOB

#define PIOS_LED_LED4_GPIO_PORT			GPIOB					// TX LED
#define PIOS_LED_LED4_GPIO_PIN			GPIO_Pin_7
#define PIOS_LED_LED4_GPIO_CLK			RCC_APB2Periph_GPIOB

#define PIOS_LED_NUM					4
#define PIOS_LED_PORTS					{ PIOS_LED_LED1_GPIO_PORT, PIOS_LED_LED2_GPIO_PORT, PIOS_LED_LED3_GPIO_PORT, PIOS_LED_LED4_GPIO_PORT }
#define PIOS_LED_PINS					{ PIOS_LED_LED1_GPIO_PIN,  PIOS_LED_LED2_GPIO_PIN,  PIOS_LED_LED3_GPIO_PIN,  PIOS_LED_LED4_GPIO_PIN }
#define PIOS_LED_CLKS					{ PIOS_LED_LED1_GPIO_CLK,  PIOS_LED_LED2_GPIO_CLK,  PIOS_LED_LED3_GPIO_CLK,  PIOS_LED_LED4_GPIO_CLK }

#define USB_LED_ON						PIOS_LED_On(LED1)
#define USB_LED_OFF						PIOS_LED_Off(LED1)
#define USB_LED_TOGGLE					PIOS_LED_Toggle(LED1)
#define LINK_LED_ON						PIOS_LED_On(LED2)
#define LINK_LED_OFF					PIOS_LED_Off(LED2)
#define LINK_LED_TOGGLE					PIOS_LED_Toggle(LED2)
#define RX_LED_ON						PIOS_LED_On(LED3)
#define RX_LED_OFF						PIOS_LED_Off(LED3)
#define RX_LED_TOGGLE					PIOS_LED_Toggle(LED3)
#define TX_LED_ON						PIOS_LED_On(LED4)
#define TX_LED_OFF						PIOS_LED_Off(LED4)
#define TX_LED_TOGGLE					PIOS_LED_Toggle(LED4)

//-------------------------
// Delay Timer
//-------------------------
#define PIOS_DELAY_TIMER				TIM2
#define PIOS_DELAY_TIMER_RCC_FUNC		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE)

//-------------------------
// System Settings
//-------------------------
#define PIOS_MASTER_CLOCK				72000000
#define PIOS_PERIPHERAL_CLOCK			(PIOS_MASTER_CLOCK / 2)
#if defined(USE_BOOTLOADER)
#define PIOS_NVIC_VECTTAB_FLASH			((uint32_t)0x08006000)
#else
#define PIOS_NVIC_VECTTAB_FLASH			((uint32_t)0x08000000)
#endif

//-------------------------
// Interrupt Priorities
//-------------------------
#define PIOS_IRQ_PRIO_LOW			12		// lower than RTOS
#define PIOS_IRQ_PRIO_MID			8		// higher than RTOS
#define PIOS_IRQ_PRIO_HIGH			5		// for SPI, ADC, I2C etc...
#define PIOS_IRQ_PRIO_HIGHEST		4 		// for USART etc...

//-------------------------
// SPI
//
// See also pios_board.c
//-------------------------
#define PIOS_SPI_PERIF				0

//-------------------------
// PIOS_USART
//-------------------------
#define PIOS_USART_RX_BUFFER_SIZE	256
#define PIOS_USART_TX_BUFFER_SIZE	256
#define PIOS_COM_COM				0
#define PIOS_COM_DEBUG				PIOS_COM_COM

#if 0
//-------------------------
// ADC
// PIOS_ADC_PinGet(0) = Accel Z
// PIOS_ADC_PinGet(1) =
// PIOS_ADC_PinGet(2) =
// PIOS_ADC_PinGet(3) =
// PIOS_ADC_PinGet(4) = Accel ?
// PIOS_ADC_PinGet(5) =
// PIOS_ADC_PinGet(6) =
// PIOS_ADC_PinGet(7) =
//-------------------------
//#define PIOS_ADC_OVERSAMPLING_RATE		1
#define PIOS_ADC_USE_TEMP_SENSOR			0
#define PIOS_ADC_TEMP_SENSOR_ADC			ADC1
#define PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL	1

#define PIOS_ADC_PIN1_GPIO_PORT				GPIOB			// PB1 (PSU Voltage)
#define PIOS_ADC_PIN1_GPIO_PIN				GPIO_Pin_1		// ADC12_IN9
#define PIOS_ADC_PIN1_GPIO_CHANNEL			ADC_Channel_9
#define PIOS_ADC_PIN1_ADC					ADC2
#define PIOS_ADC_PIN1_ADC_NUMBER			4

#define PIOS_ADC_NUM_PINS			1

#define PIOS_ADC_PORTS				{ PIOS_ADC_PIN1_GPIO_PORT }
#define PIOS_ADC_PINS				{ PIOS_ADC_PIN1_GPIO_PIN }
#define PIOS_ADC_CHANNELS			{ PIOS_ADC_PIN1_GPIO_CHANNEL }
#define PIOS_ADC_MAPPING			{ PIOS_ADC_PIN1_ADC }
#define PIOS_ADC_CHANNEL_MAPPING	{ PIOS_ADC_PIN1_ADC_NUMBER }
#define PIOS_ADC_NUM_CHANNELS		(PIOS_ADC_NUM_PINS + PIOS_ADC_USE_TEMP_SENSOR)
#define PIOS_ADC_NUM_ADC_CHANNELS	2
#define PIOS_ADC_USE_ADC2			1
#define PIOS_ADC_CLOCK_FUNCTION		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE)
#define PIOS_ADC_ADCCLK				RCC_PCLK2_Div8
						/* RCC_PCLK2_Div2: ADC clock = PCLK2/2 */
						/* RCC_PCLK2_Div4: ADC clock = PCLK2/4 */
						/* RCC_PCLK2_Div6: ADC clock = PCLK2/6 */
						/* RCC_PCLK2_Div8: ADC clock = PCLK2/8 */
#define PIOS_ADC_SAMPLE_TIME		ADC_SampleTime_239Cycles5
						/* Sample time: */
						/* With an ADCCLK = 14 MHz and a sampling time of 293.5 cycles: */
						/* Tconv = 239.5 + 12.5 = 252 cycles = 18�s */
						/* (1 / (ADCCLK / CYCLES)) = Sample Time (�S) */
#define PIOS_ADC_IRQ_PRIO			PIOS_IRQ_PRIO_HIGH
#endif

//-------------------------
// ADC
// PIOS_ADC_PinGet(0) = Accel Z
// PIOS_ADC_PinGet(1) =
// PIOS_ADC_PinGet(2) =
// PIOS_ADC_PinGet(3) =
// PIOS_ADC_PinGet(4) = Accel ?
// PIOS_ADC_PinGet(5) =
// PIOS_ADC_PinGet(6) =
// PIOS_ADC_PinGet(7) =
//-------------------------
//#define PIOS_ADC_OVERSAMPLING_RATE		1
#define PIOS_ADC_USE_TEMP_SENSOR			0
#define PIOS_ADC_TEMP_SENSOR_ADC			ADC1
#define PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL	1

#define PIOS_ADC_PIN1_GPIO_PORT			GPIOB			// PB1 (PSU Voltage)
#define PIOS_ADC_PIN1_GPIO_PIN			GPIO_Pin_1		// ADC12_IN9
#define PIOS_ADC_PIN1_GPIO_CHANNEL		ADC_Channel_9
#define PIOS_ADC_PIN1_ADC				ADC2
#define PIOS_ADC_PIN1_ADC_NUMBER		4

#define PIOS_ADC_NUM_PINS				1

#define PIOS_ADC_PORTS				{ PIOS_ADC_PIN1_GPIO_PORT }
#define PIOS_ADC_PINS				{ PIOS_ADC_PIN1_GPIO_PIN }
#define PIOS_ADC_CHANNELS			{ PIOS_ADC_PIN1_GPIO_CHANNEL }
#define PIOS_ADC_MAPPING			{ PIOS_ADC_PIN1_ADC }
#define PIOS_ADC_CHANNEL_MAPPING	{ PIOS_ADC_PIN1_ADC_NUMBER }
#define PIOS_ADC_NUM_CHANNELS		(PIOS_ADC_NUM_PINS + PIOS_ADC_USE_TEMP_SENSOR)
#define PIOS_ADC_NUM_ADC_CHANNELS	2
#define PIOS_ADC_USE_ADC2			1
#define PIOS_ADC_CLOCK_FUNCTION		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE)
#define PIOS_ADC_ADCCLK				RCC_PCLK2_Div8
						/* RCC_PCLK2_Div2: ADC clock = PCLK2/2 */
						/* RCC_PCLK2_Div4: ADC clock = PCLK2/4 */
						/* RCC_PCLK2_Div6: ADC clock = PCLK2/6 */
						/* RCC_PCLK2_Div8: ADC clock = PCLK2/8 */
#define PIOS_ADC_SAMPLE_TIME		ADC_SampleTime_239Cycles5
						/* Sample time: */
						/* With an ADCCLK = 14 MHz and a sampling time of 293.5 cycles: */
						/* Tconv = 239.5 + 12.5 = 252 cycles = 18�s */
						/* (1 / (ADCCLK / CYCLES)) = Sample Time (�S) */
#define PIOS_ADC_IRQ_PRIO			PIOS_IRQ_PRIO_HIGH

//-------------------------
// GPIO
//-------------------------

#define PIOS_GPIO_1_PORT			GPIOB
#define PIOS_GPIO_1_PIN				GPIO_Pin_15
#define PIOS_GPIO_1_GPIO_CLK		RCC_APB2Periph_GPIOB
#define PIOS_GPIO_PORTS				{ PIOS_GPIO_1_PORT }
#define PIOS_GPIO_PINS				{ PIOS_GPIO_1_PIN }
#define PIOS_GPIO_CLKS				{ PIOS_GPIO_1_GPIO_CLK }
#define PIOS_GPIO_NUM				1

#define SET_RTS						PIOS_GPIO_On(0);
#define CLEAR_RTS					PIOS_GPIO_Off(0);

//-------------------------

#endif /* PIOS_BOARD_H */
