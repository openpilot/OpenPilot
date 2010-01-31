/**
 ******************************************************************************
 *
 * @file       pios_board.h   
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      Defines board hardware for the OpenPilot Version 0.1 hardware.
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
// DMA Channels Used
//------------------------
/* Channel 1  - 					*/
/* Channel 2  - SPI1 RX					*/
/* Channel 3  - SPI1 TX					*/
/* Channel 4  - SPI2 RX					*/
/* Channel 5  - SPI2 TX					*/
/* Channel 6  - 					*/
/* Channel 7  - 					*/
/* Channel 8  - 					*/
/* Channel 9  - 					*/
/* Channel 10 - 					*/
/* Channel 11 - 					*/
/* Channel 12 - 					*/

//------------------------
// Leds Definition   
//------------------------
#define LED1_GPIO_PORT					GPIOC
#define LED1_GPIO_PIN					GPIO_Pin_12
#define LED1_GPIO_CLK					RCC_APB2Periph_GPIOC
#define LED2_GPIO_PORT					GPIOC
#define LED2_GPIO_PIN					GPIO_Pin_13
#define LED2_GPIO_CLK					RCC_APB2Periph_GPIOC
#define NUM_LED						2

//------------------------
// I2C   
//------------------------
#define I2C_GPIO_PORT					GPIOB
#define I2C_SDA_PIN					GPIO_Pin_11
#define I2C_SCL_PIN					GPIO_Pin_10
#define I2C_DUTY_CYCLE					I2C_DutyCycle_2
#define I2C_BUS_FREQUENCY				400000
#define I2C_TIMEOUT_VALUE				5000
#define IRQ_I2C_EV_PRIORITY				2
#define IRQ_I2C_ER_PRIORITY				2

//------------------------
// Onboard Pressure sensor   
//------------------------
#define BMP085_EOC_GPIO_PORT				GPIOC
#define BMP085_EOC_GPIO_PIN				GPIO_Pin_15
#define BMP085_EOC_PORT_SOURCE				GPIO_PortSourceGPIOG
#define BMP085_EOC_PIN_SOURCE				GPIO_PinSource8
#define BMP085_EOC_CLK					RCC_APB2Periph_GPIOC
#define BMP085_EOC_EXTI_LINE				EXTI_Line15
#define BMP085_EOC_IRQn					EXTI15_10_IRQn

/* Unused, repurpose PA1 as another ADC input?
#define BMP085_XCLR_GPIO_PORT				GPIOA
#define BMP085_XCLR_GPIO_PIN				GPIO_Pin_1
*/

//-------------------------
// GPS USART 
//-------------------------
#define GPS_USART				USART2
#define GPS_GPIO_PORT				GPIOA
#define GPS_RX_PIN				GPIO_Pin_3
#define GPS_TX_PIN				GPIO_Pin_2
#define GPS_REMAP_FUNC				{ }
#define GPS_IRQ_CHANNEL				USART2_IRQn
#define GPS_IRQHANDLER_FUNC			void USART2_IRQHandler(void)
#define GPS_CLK_FUNC				RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE)
#define GPS_NVIC_PRIO				IRQ_PRIO_HIGHEST

//-------------------------
// Telemetry radio USART 
//-------------------------
#define TELEM_USART        			USART3
#define TELEM_GPIO_PORT				GPIOC
#define TELEM_RX_PIN      			GPIO_Pin_11
#define TELEM_TX_PIN      			GPIO_Pin_10
#define TELEM_REMAP_FUNC			{ GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE); }
#define TELEM_IRQ_CHANNEL			USART3_IRQn
#define TELEM_IRQHANDLER_FUNC			void USART3_IRQHandler(void)
#define TELEM_CLK_FUNC				RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE)
#define TELEM_NVIC_PRIO				IRQ_PRIO_HIGHEST

//-------------------------
// AUXSER USART (available instead of RX5/RX6)
//-------------------------
#define AUX_USART_USART				USART1
#define AUX_USART_GPIO_PORT			GPIOA
#define AUX_USART_RX_PIN			GPIO_Pin_10
#define AUX_USART_TX_PIN			GPIO_Pin_9
#define AUX_USART_REMAP_FUNC			{ }
#define AUX_USART_IRQ_CHANNEL			USART1_IRQn
#define AUX_USART_IRQHANDLER_FUNC		void USART1_IRQHandler(void)
#define AUX_USART_CLK_FUNC			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE)
#define AUX_USART_NVIC_PRIO			IRQ_PRIO_HIGH

//-------------------------
// USART Serial Ports
//-------------------------
#define USART_NUM				3
#define USART_RX_BUFFER_SIZE			1024
#define USART_TX_BUFFER_SIZE			256
#define COM_DEBUG_PORT				GPS

//-------------------------
// SPI
//-------------------------
#define PIOS_IRQ_SPI_DMA_PRIORITY		IRQ_PRIO_HIGH
#define PIOS_SPI0_PTR				SPI1
#define PIOS_SPI0_DMA_RX_PTR			DMA1_Channel2
#define PIOS_SPI0_DMA_TX_PTR			DMA1_Channel3
#define PIOS_SPI0_DMA_RX_IRQ_FLAGS		(DMA1_FLAG_TC2 | DMA1_FLAG_TE2 | DMA1_FLAG_HT2 | DMA1_FLAG_GL2)
#define PIOS_SPI0_DMA_IRQ_CHANNEL		DMA1_Channel2_IRQn
#define PIOS_SPI0_DMA_IRQHANDLER_FUNC		void DMA1_Channel2_IRQHandler(void)
#define PIOS_SPI0_RCLK1_PORT			GPIOA
#define PIOS_SPI0_RCLK1_PIN			GPIO_Pin_4
#define PIOS_SPI0_SCLK_PORT			GPIOA
#define PIOS_SPI0_SCLK_PIN			GPIO_Pin_5
#define PIOS_SPI0_MISO_PORT			GPIOA
#define PIOS_SPI0_MISO_PIN			GPIO_Pin_6
#define PIOS_SPI0_MOSI_PORT			GPIOA
#define PIOS_SPI0_MOSI_PIN			GPIO_Pin_7
#define PIOS_SPI1_PTR				SPI2
#define PIOS_SPI1_DMA_RX_PTR			DMA1_Channel4
#define PIOS_SPI1_DMA_TX_PTR			DMA1_Channel5
#define PIOS_SPI1_DMA_RX_IRQ_FLAGS		(DMA1_FLAG_TC4 | DMA1_FLAG_TE4 | DMA1_FLAG_HT4 | DMA1_FLAG_GL4)
#define PIOS_SPI1_DMA_IRQ_CHANNEL		DMA1_Channel4_IRQn
#define PIOS_SPI1_DMA_IRQHANDLER_FUNC		void DMA1_Channel4_IRQHandler(void)
#define PIOS_SPI1_RCLK1_PORT			GPIOB
#define PIOS_SPI1_RCLK1_PIN			GPIO_Pin_12
#define PIOS_SPI1_SCLK_PORT			GPIOB
#define PIOS_SPI1_SCLK_PIN			GPIO_Pin_13
#define PIOS_SPI1_MISO_PORT			GPIOB
#define PIOS_SPI1_MISO_PIN			GPIO_Pin_14
#define PIOS_SPI1_MOSI_PORT			GPIOB
#define PIOS_SPI1_MOSI_PIN			GPIO_Pin_15

//-------------------------
// SDCard
//-------------------------
#define PIOS_SDCARD_SPI				0

//-------------------------
// Receiver PWM inputs   
//-------------------------
#define RECEIVER1_GPIO_PORT			GPIOB
#define RECEIVER1_PIN				GPIO_Pin_0      // PB0
#define RECEIVER1_TIM_PORT			TIM3
#define RECEIVER1_CH				TIM_Channel_3   // TIM3_CH3
#define RECEIVER2_GPIO_PORT			GPIOB
#define RECEIVER2_PIN				GPIO_Pin_1      // PB1
#define RECEIVER2_TIM_PORT			TIM3
#define RECEIVER2_CH				TIM_Channel_4   // TIM3_CH4
#define RECEIVER3_GPIO_PORT			GPIOA
#define RECEIVER3_PIN				GPIO_Pin_8      // PA8
#define RECEIVER3_TIM_PORT			TIM1
#define RECEIVER3_CH				TIM_Channel_1   // TIM1_CH1
#define RECEIVER4_GPIO_PORT			GPIOA
#define RECEIVER4_PIN				GPIO_Pin_0      // PA0
#define RECEIVER4_TIM_PORT			TIM5
#define RECEIVER4_CH				TIM_Channel_1   // TIM5_CH1
#define RECEIVER5_GPIO_PORT			GPIOA
#define RECEIVER5_PIN				GPIO_Pin_10     // PA10
#define RECEIVER5_TIM_PORT			TIM1
#define RECEIVER5_CH				TIM_Channel_3   // TIM1_CH3
#define RECEIVER6_GPIO_PORT			GPIOA
#define RECEIVER6_PIN				GPIO_Pin_9      // PA9
#define RECEIVER6_TIM_PORT			TIM1
#define RECEIVER6_CH				TIM_Channel_2   // TIM1_CH2

/* Not used in v0.1 HW
#define RECEIVER7_GPIO_PORT			GPIOB
#define RECEIVER7_PIN				GPIO_Pin_4      // PB4
#define RECEIVER7_TIM_PORT			TIM3
#define RECEIVER7_CH				TIM_Channel_1   // TIM3_CH1
#define RECEIVER8_GPIO_PORT			GPIOB
#define RECEIVER8_PIN				GPIO_Pin_5      // PB5
#define RECEIVER8_TIM_PORT			TIM3
#define RECEIVER8_CH				TIM_Channel_2   // TIM3_CH2
*/

#define NUM_RECEIVER_INPUTS			6

//-------------------------
// Servo outputs   
//-------------------------
#define SERVO1TO4_PORT				GPIOB
#define SERVO1_PIN				GPIO_Pin_6
#define SERVO2_PIN				GPIO_Pin_7
#define SERVO3_PIN				GPIO_Pin_8
#define SERVO4_PIN				GPIO_Pin_9
#define SERVO5TO8_PORT				GPIOC
#define SERVO5_PIN				GPIO_Pin_6
#define SERVO6_PIN				GPIO_Pin_7
#define SERVO7_PIN				GPIO_Pin_8
#define SERVO8_PIN				GPIO_Pin_9
#define NUM_SERVO_OUTPUTS			8

//-------------------------
// ADC
//-------------------------
#define ADC_GPIO_PORT				GPIOC
#define ADC_Z_PIN				GPIO_Pin_0	// ADC123_IN10
#define ADC_A_PIN				GPIO_Pin_1	// ADC123_IN11
#define ADC_B_PIN				GPIO_Pin_2	// ADC123_IN12
#define ADC_Z_CHANNEL				ADC_Channel_10
#define ADC_A_CHANNEL				ADC_Channel_11
#define ADC_B_CHANNEL				ADC_Channel_12
#define NUM_ADC_PINS				4		// 3 but actually 4 because of temp sensor
#define ADC_IRQ_PRIO				IRQ_PRIO_HIGH

//-------------------------  
// USB
//-------------------------
#define USB_ACC_GPIO_PORT			GPIOC
#define USB_DETECT_PIN				GPIO_Pin_4
#define USB_PULLUP_PIN				GPIO_Pin_14
#define PIOS_IRQ_USB_PRIORITY			IRQ_PRIO_MID

//-------------------------
// Delay Timer    
//-------------------------
#define PIOS_DELAY_TIMER			TIM2
#define PIOS_DELAY_TIMER_RCC			RCC_APB1Periph_TIM2

//-------------------------
// Master Clock   
//-------------------------
#define MASTER_CLOCK				72000000
#define PERIPHERAL_CLOCK			(MASTER_CLOCK/2)

//-------------------------
// Interrupt Priorities
//-------------------------
#define IRQ_PRIO_LOW				12		// lower than RTOS
#define IRQ_PRIO_MID				8		// higher than RTOS
#define IRQ_PRIO_HIGH				5		// for SPI, ADC, I2C etc...
#define IRQ_PRIO_HIGHEST			4 		// for USART etc...

#endif /* PIOS_BOARD_H */
