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

// *****************************************************************
// Timers and Channels Used

/*
Timer | Channel 1 | Channel 2 | Channel 3 | Channel 4
------+-----------+-----------+-----------+----------
TIM1  |           |           |           |
TIM2  | --------------- DELAY ----------------------
TIM3  | --------------- Timer Interrupt ------------
TIM4  | --------------- STOPWATCH ------------------
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
// BOOTLOADER_SETTINGS
//------------------------
#define FUNC_ID				3
#define HW_VERSION			21
#define BOOTLOADER_VERSION	0
#define MEM_SIZE			((uint32_t)(*((volatile uint16_t *)(0x1FFFF7E0))) * 1024 - 1024) //128K
#define SIZE_OF_DESCRIPTION	100
#define START_OF_USER_CODE	(uint32_t)0x08003000
#define SIZE_OF_CODE		(uint32_t)(MEM_SIZE-(START_OF_USER_CODE-0x08000000)-SIZE_OF_DESCRIPTION)
#ifdef STM32F10X_HD
		#define HW_TYPE			0 //0=high_density 1=medium_density;
#elif STM32F10X_MD
		#define HW_TYPE			1 //0=high_density 1=medium_density;
#endif
#define BOARD_READABLE	TRUE
#define BOARD_WRITABLA	TRUE
#define MAX_DEL_RETRYS	3


// *****************************************************************
// System Settings

#define PIOS_MASTER_CLOCK                       72000000ul
#define PIOS_PERIPHERAL_CLOCK                   (PIOS_MASTER_CLOCK / 2)

#if defined(USE_BOOTLOADER)
  #define PIOS_NVIC_VECTTAB_FLASH               (START_OF_USER_CODE)
#else
  #define PIOS_NVIC_VECTTAB_FLASH               ((uint32_t)0x08000000)
#endif

// *****************************************************************
// Interrupt Priorities

#define PIOS_IRQ_PRIO_LOW			12		// lower than RTOS
#define PIOS_IRQ_PRIO_MID			8		// higher than RTOS
#define PIOS_IRQ_PRIO_HIGH			5		// for SPI, ADC, I2C etc...
#define PIOS_IRQ_PRIO_HIGHEST		4 		// for USART etc...

// *****************************************************************
// PIOS_LED

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

// *****************************************************************
// Delay Timer

#define PIOS_DELAY_TIMER				TIM2
#define PIOS_DELAY_TIMER_RCC_FUNC		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE)

// *****************************************************************
// Timer interrupt

#define TIMER_INT_TIMER					TIM3
#define TIMER_INT_FUNC					TIM3_IRQHandler
#define TIMER_INT_PRIORITY				2

// *****************************************************************
// Stop watch timer

#define STOPWATCH_TIMER					TIM4

// *****************************************************************
// SPI
//
// See also pios_board.c

#define PIOS_SPI_PORT                   0

// *****************************************************************
// PIOS_USART

#define PIOS_USART_RX_BUFFER_SIZE       512
#define PIOS_USART_TX_BUFFER_SIZE       512

#define PIOS_COM_SERIAL                 0
//#define PIOS_COM_DEBUG                  PIOS_COM_SERIAL // comment this out if you don't want debug text sent out on the serial port

#define PIOS_USART_BAUDRATE             57600
//#define PIOS_USART_BAUDRATE           115200

#if defined(PIOS_INCLUDE_USB_HID)
  #define PIOS_COM_TELEM_USB            1
#endif

#if defined(PIOS_COM_DEBUG)
  #define DEBUG_PRINTF(...) PIOS_COM_SendFormattedString(PIOS_COM_DEBUG, __VA_ARGS__)
//      #define DEBUG_PRINTF(...) PIOS_COM_SendFormattedStringNonBlocking(PIOS_COM_DEBUG, __VA_ARGS__)
#else
  #define DEBUG_PRINTF(...)
#endif

// *****************************************************************
// ADC

// PIOS_ADC_PinGet(0) = Temperature Sensor (On-board)
// PIOS_ADC_PinGet(1) = PSU Voltage

#define PIOS_ADC_OVERSAMPLING_RATE			2

#define PIOS_ADC_USE_TEMP_SENSOR			1
#define PIOS_ADC_TEMP_SENSOR_ADC			ADC1
#define PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL	16		// Temperature sensor channel
//#define PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL	17		// VREF channel

#define PIOS_ADC_PIN1_GPIO_PORT				GPIOB			// Port B (PSU Voltage)
#define PIOS_ADC_PIN1_GPIO_PIN				GPIO_Pin_1		// PB1 .. ADC12_IN9
#define PIOS_ADC_PIN1_GPIO_CHANNEL			ADC_Channel_9
#define PIOS_ADC_PIN1_ADC					ADC2
#define PIOS_ADC_PIN1_ADC_NUMBER			1

#define PIOS_ADC_NUM_PINS					1

#define PIOS_ADC_PORTS						{ PIOS_ADC_PIN1_GPIO_PORT    }
#define PIOS_ADC_PINS						{ PIOS_ADC_PIN1_GPIO_PIN     }
#define PIOS_ADC_CHANNELS					{ PIOS_ADC_PIN1_GPIO_CHANNEL }
#define PIOS_ADC_MAPPING					{ PIOS_ADC_PIN1_ADC          }
#define PIOS_ADC_CHANNEL_MAPPING			{ PIOS_ADC_PIN1_ADC_NUMBER   }

#define PIOS_ADC_NUM_CHANNELS				(PIOS_ADC_NUM_PINS + PIOS_ADC_USE_TEMP_SENSOR)
#define PIOS_ADC_NUM_ADC_CHANNELS			2
#define PIOS_ADC_USE_ADC2					1
#define PIOS_ADC_CLOCK_FUNCTION				RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE)
//#define PIOS_ADC_ADCCLK					RCC_PCLK2_Div2	// ADC clock = PCLK2/2
//#define PIOS_ADC_ADCCLK					RCC_PCLK2_Div4	// ADC clock = PCLK2/4
//#define PIOS_ADC_ADCCLK					RCC_PCLK2_Div6	// ADC clock = PCLK2/6
#define PIOS_ADC_ADCCLK						RCC_PCLK2_Div8	// ADC clock = PCLK2/8
#define PIOS_ADC_PCLK2                          RCC_HCLK_Div16
//#define PIOS_ADC_SAMPLE_TIME				ADC_SampleTime_1Cycles5
//#define PIOS_ADC_SAMPLE_TIME				ADC_SampleTime_7Cycles5
//#define PIOS_ADC_SAMPLE_TIME				ADC_SampleTime_13Cycles5
//#define PIOS_ADC_SAMPLE_TIME				ADC_SampleTime_28Cycles5
//#define PIOS_ADC_SAMPLE_TIME				ADC_SampleTime_41Cycles5
//#define PIOS_ADC_SAMPLE_TIME				ADC_SampleTime_55Cycles5
//#define PIOS_ADC_SAMPLE_TIME				ADC_SampleTime_71Cycles5
#define PIOS_ADC_SAMPLE_TIME				ADC_SampleTime_239Cycles5
						/* Sample time: */
						/* With an ADCCLK = 14 MHz and a sampling time of 293.5 cycles: */
						/* Tconv = 239.5 + 12.5 = 252 cycles = 18�s */
						/* (1 / (ADCCLK / CYCLES)) = Sample Time (�S) */
#define PIOS_ADC_IRQ_PRIO					3
#define PIOS_ADC_MAX_OVERSAMPLING               1

// *****************************************************************
// GPIO output pins

// GPIO_Mode_Out_OD        Open Drain Output
// GPIO_Mode_Out_PP        Push-Pull Output
// GPIO_Mode_AF_OD         Open Drain Output Alternate-Function
// GPIO_Mode_AF_PP         Push-Pull Output Alternate-Function

// Serial port RTS line
#define PIOS_GPIO_OUT_0_PORT		GPIOB
#define PIOS_GPIO_OUT_0_PIN			GPIO_Pin_15
#define PIOS_GPIO_OUT_0_GPIO_CLK	RCC_APB2Periph_GPIOB

// RF module chip-select line
#define PIOS_GPIO_OUT_1_PORT		GPIOA
#define PIOS_GPIO_OUT_1_PIN			GPIO_Pin_4
#define PIOS_GPIO_OUT_1_GPIO_CLK	RCC_APB2Periph_GPIOA

// EEPROM chip-select line
#define PIOS_GPIO_OUT_2_PORT		GPIOA
#define PIOS_GPIO_OUT_2_PIN			GPIO_Pin_1
#define PIOS_GPIO_OUT_2_GPIO_CLK	RCC_APB2Periph_GPIOA
/*
// SPI-1 SCK line
#define PIOS_GPIO_OUT_3_PORT		GPIOA
#define PIOS_GPIO_OUT_3_PIN			GPIO_Pin_5
#define PIOS_GPIO_OUT_3_GPIO_CLK	RCC_APB2Periph_GPIOA

// SPI-1 MOSI line
#define PIOS_GPIO_OUT_4_PORT		GPIOA
#define PIOS_GPIO_OUT_4_PIN			GPIO_Pin_7
#define PIOS_GPIO_OUT_4_GPIO_CLK	RCC_APB2Periph_GPIOA
*/
#define PIOS_GPIO_NUM				3
#define PIOS_GPIO_PORTS				{ PIOS_GPIO_OUT_0_PORT,      PIOS_GPIO_OUT_1_PORT,      PIOS_GPIO_OUT_2_PORT,    }
#define PIOS_GPIO_PINS				{ PIOS_GPIO_OUT_0_PIN,       PIOS_GPIO_OUT_1_PIN,       PIOS_GPIO_OUT_2_PIN,     }
#define PIOS_GPIO_CLKS				{ PIOS_GPIO_OUT_0_GPIO_CLK,  PIOS_GPIO_OUT_1_GPIO_CLK,  PIOS_GPIO_OUT_2_GPIO_CLK }

#define SERIAL_RTS_ENABLE			PIOS_GPIO_Enable(0)
#define SERIAL_RTS_SET				PIOS_GPIO_Off(0)
#define SERIAL_RTS_CLEAR			PIOS_GPIO_On(0)

#define RF_CS_ENABLE				PIOS_GPIO_Enable(1)
#define RF_CS_HIGH					PIOS_GPIO_Off(1)
#define RF_CS_LOW					PIOS_GPIO_On(1)

#define EE_CS_ENABLE				PIOS_GPIO_Enable(2)
#define EE_CS_HIGH					PIOS_GPIO_Off(2)
#define EE_CS_LOW					PIOS_GPIO_On(2)
/*
#define SPI1_SCK_ENABLE				PIOS_GPIO_Enable(3)
#define SPI1_SCK_HIGH				PIOS_GPIO_Off(3)
#define SPI1_SCK_LOW				PIOS_GPIO_On(3)

#define SPI1_MOSI_ENABLE			PIOS_GPIO_Enable(4)
#define SPI1_MOSI_HIGH				PIOS_GPIO_Off(4)
#define SPI1_MOSI_LOW				PIOS_GPIO_On(4)
*/
// *****************************************************************
// GPIO input pins

// GPIO_Mode_AIN           Analog Input
// GPIO_Mode_IN_FLOATING   Input Floating
// GPIO_Mode_IPD           Input Pull-Down
// GPIO_Mode_IPU           Input Pull-up

// API mode line
#define GPIO_IN_0_PORT			GPIOB
#define GPIO_IN_0_PIN			GPIO_Pin_13
#define GPIO_IN_0_MODE			GPIO_Mode_IPU

// Serial port CTS line
#define GPIO_IN_1_PORT			GPIOB
#define GPIO_IN_1_PIN			GPIO_Pin_14
#define GPIO_IN_1_MODE			GPIO_Mode_IPU

// VBUS sense line
#define GPIO_IN_2_PORT			GPIOA
#define GPIO_IN_2_PIN			GPIO_Pin_8
#define GPIO_IN_2_MODE			GPIO_Mode_IN_FLOATING

// 868MHz jumper option
#define GPIO_IN_3_PORT			GPIOB
#define GPIO_IN_3_PIN			GPIO_Pin_8
#define GPIO_IN_3_MODE			GPIO_Mode_IPU

// 915MHz jumper option
#define GPIO_IN_4_PORT			GPIOB
#define GPIO_IN_4_PIN			GPIO_Pin_9
#define GPIO_IN_4_MODE			GPIO_Mode_IPU

// RF INT line
#define GPIO_IN_5_PORT			GPIOA
#define GPIO_IN_5_PIN			GPIO_Pin_2
#define GPIO_IN_5_MODE			GPIO_Mode_IN_FLOATING

// RF misc line
#define GPIO_IN_6_PORT			GPIOB
#define GPIO_IN_6_PIN			GPIO_Pin_0
#define GPIO_IN_6_MODE			GPIO_Mode_IN_FLOATING

#define GPIO_IN_NUM				7
#define GPIO_IN_PORTS			{ GPIO_IN_0_PORT, GPIO_IN_1_PORT, GPIO_IN_2_PORT, GPIO_IN_3_PORT, GPIO_IN_4_PORT, GPIO_IN_5_PORT, GPIO_IN_6_PORT }
#define GPIO_IN_PINS			{ GPIO_IN_0_PIN,  GPIO_IN_1_PIN,  GPIO_IN_2_PIN,  GPIO_IN_3_PIN,  GPIO_IN_4_PIN,  GPIO_IN_5_PIN,  GPIO_IN_6_PIN  }
#define GPIO_IN_MODES			{ GPIO_IN_0_MODE, GPIO_IN_1_MODE, GPIO_IN_2_MODE, GPIO_IN_3_MODE, GPIO_IN_4_MODE, GPIO_IN_5_MODE, GPIO_IN_6_MODE }

#define API_MODE_PIN			0
#define SERIAL_CTS_PIN			1
#define VBUS_SENSE_PIN			2
#define _868MHz_PIN				3
#define _915MHz_PIN				4
#define RF_INT_PIN				5
#define RF_MISC_PIN				6

// *****************************************************************
// USB

#if defined(PIOS_INCLUDE_USB_HID)
	#define PIOS_USB_ENABLED				1
	#define PIOS_USB_DETECT_GPIO_PORT		GPIO_IN_2_PORT
	#define PIOS_USB_DETECT_GPIO_PIN		GPIO_IN_2_PIN
	#define PIOS_USB_DETECT_EXTI_LINE		EXTI_Line4
	#define PIOS_IRQ_USB_PRIORITY			8
        #define PIOS_USB_RX_BUFFER_SIZE                 512
        #define PIOS_USB_TX_BUFFER_SIZE                 512
#endif

// *****************************************************************
// RFM22

//#define RFM22_EXT_INT_USE

#define RFM22_PIOS_SPI						PIOS_SPI_PORT	// SPIx

#if defined(RFM22_EXT_INT_USE)
	#define RFM22_EXT_INT_PORT_SOURCE		GPIO_PortSourceGPIOA
	#define RFM22_EXT_INT_PIN_SOURCE		GPIO_PinSource2

	#define RFM22_EXT_INT_LINE				EXTI_Line2
	#define RFM22_EXT_INT_IRQn				EXTI2_IRQn
	#define	RFM22_EXT_INT_FUNC				EXTI2_IRQHandler

	#define RFM22_EXT_INT_PRIORITY			1
#endif

// *****************************************************************

#endif /* PIOS_BOARD_H */
