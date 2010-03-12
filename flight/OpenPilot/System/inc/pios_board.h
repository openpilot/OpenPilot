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
TIM1  | RC In 3   | RC In 6   | RC In 5   |
TIM2  | --------------- PIOS_DELAY -----------------
TIM3  | RC In 7   | RC In 8   | RC In 1   | RC In 2
TIM4  | Servo 1   | Servo 2   | Servo 3   | Servo 4
TIM5  | RC In 4   |           |           |
TIM6  | ----------- PIOS_PWM (Supervisor) ----------
TIM7  |           |           |           |
TIM8  | Servo 5   | Servo 6   | Servo 7   | Servo 8
------+-----------+-----------+-----------+----------
*/

//------------------------
// DMA Channels Used
//------------------------
/* Channel 1  - ADC				*/
/* Channel 2  - SPI1 RX				*/
/* Channel 3  - SPI1 TX				*/
/* Channel 4  - SPI2 RX				*/
/* Channel 5  - SPI2 TX				*/
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

//------------------------
// PIOS_I2C
//------------------------
#define PIOS_I2C_GPIO_PORT			GPIOB
#define PIOS_I2C_SDA_PIN			GPIO_Pin_11
#define PIOS_I2C_SCL_PIN			GPIO_Pin_10
#define PIOS_I2C_DUTY_CYCLE			I2C_DutyCycle_2
#define PIOS_I2C_BUS_FREQ			400000
#define PIOS_I2C_TIMEOUT_VALUE			5000
#define PIOS_I2C_IRQ_EV_PRIORITY		2
#define PIOS_I2C_IRQ_ER_PRIORITY		2

//------------------------
// PIOS_BMP085
//------------------------
#define PIOS_BMP085_EOC_GPIO_PORT		GPIOC
#define PIOS_BMP085_EOC_GPIO_PIN		GPIO_Pin_15
#define PIOS_BMP085_EOC_PORT_SOURCE		GPIO_PortSourceGPIOG
#define PIOS_BMP085_EOC_PIN_SOURCE		GPIO_PinSource8
#define PIOS_BMP085_EOC_CLK			RCC_APB2Periph_GPIOC
#define PIOS_BMP085_EOC_EXTI_LINE		EXTI_Line15
#define PIOS_BMP085_EOC_IRQn			EXTI15_10_IRQn
#define PIOS_BMP085_EOC_PRIO			PIOS_IRQ_PRIO_HIGH

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
#define PIOS_USART1_BAUDRATE			57600

//-------------------------
// PIOS_USART2 (GPS)
//-------------------------
#define PIOS_USART2_ENABLED			1
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
#define PIOS_USART_NUM				3
#define PIOS_USART_RX_BUFFER_SIZE		1024
#define PIOS_USART_TX_BUFFER_SIZE		256
#define PIOS_COM_DEBUG_PORT			USART_1

//-------------------------
// SPI
//-------------------------
#define PIOS_SPI_IRQ_DMA_PRIORITY		PIOS_IRQ_PRIO_HIGH
#define PIOS_SPI_NUM				2
#define PIOS_SPI0_ENABLED			1
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
#define PIOS_SPI1_ENABLED			1
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
// PIOS_SDCARD
//-------------------------
#define PIOS_SDCARD_SPI				0

//-------------------------
// Delay Timer
//-------------------------
#define PIOS_DELAY_TIMER			TIM2
#define PIOS_DELAY_TIMER_RCC_FUNC		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE)

//-------------------------
// Master Clock
//-------------------------
#define PIOS_MASTER_CLOCK			72000000
#define PIOS_PERIPHERAL_CLOCK			(PIOS_MASTER_CLOCK / 2)

//-------------------------
// Interrupt Priorities
//-------------------------
#define PIOS_IRQ_PRIO_LOW			12		// lower than RTOS
#define PIOS_IRQ_PRIO_MID			8		// higher than RTOS
#define PIOS_IRQ_PRIO_HIGH			5		// for SPI, ADC, I2C etc...
#define PIOS_IRQ_PRIO_HIGHEST			4 		// for USART etc...

//-------------------------
// Receiver PWM inputs
//-------------------------
#define RECEIVER1_GPIO_PORT			GPIOA
#define RECEIVER1_PIN				GPIO_Pin_9
#define RECEIVER1_TIM_PORT			TIM1
#define RECEIVER1_CH				TIM_Channel_2
#define RECEIVER1_CCR				TIM_IT_CC2

#define RECEIVER2_GPIO_PORT			GPIOA
#define RECEIVER2_PIN				GPIO_Pin_10
#define RECEIVER2_TIM_PORT			TIM1
#define RECEIVER2_CH				TIM_Channel_3
#define RECEIVER2_CCR				TIM_IT_CC3

#define RECEIVER3_GPIO_PORT			GPIOA
#define RECEIVER3_PIN				GPIO_Pin_0
#define RECEIVER3_TIM_PORT			TIM5
#define RECEIVER3_CH				TIM_Channel_1
#define RECEIVER3_CCR				TIM_IT_CC1

#define RECEIVER4_GPIO_PORT			GPIOA
#define RECEIVER4_PIN				GPIO_Pin_8
#define RECEIVER4_TIM_PORT			TIM1
#define RECEIVER4_CH				TIM_Channel_1
#define RECEIVER4_CCR				TIM_IT_CC1

#define RECEIVER5_GPIO_PORT			GPIOB
#define RECEIVER5_PIN				GPIO_Pin_1
#define RECEIVER5_TIM_PORT			TIM3
#define RECEIVER5_CH				TIM_Channel_4
#define RECEIVER5_CCR				TIM_IT_CC4

#define RECEIVER6_GPIO_PORT			GPIOB
#define RECEIVER6_PIN				GPIO_Pin_0
#define RECEIVER6_TIM_PORT			TIM3
#define RECEIVER6_CH				TIM_Channel_3
#define RECEIVER6_CCR				TIM_IT_CC3

#define RECEIVER7_GPIO_PORT			GPIOB
#define RECEIVER7_PIN				GPIO_Pin_4
#define RECEIVER7_TIM_PORT			TIM3
#define RECEIVER7_CH				TIM_Channel_1
#define RECEIVER7_CCR				TIM_IT_CC1

#define RECEIVER8_GPIO_PORT			GPIOB
#define RECEIVER8_PIN				GPIO_Pin_5
#define RECEIVER8_TIM_PORT			TIM3
#define RECEIVER8_CH				TIM_Channel_2
#define RECEIVER8_CCR				TIM_IT_CC2

#define PIOS_PWM_GPIO_PORTS			{ RECEIVER1_GPIO_PORT, RECEIVER2_GPIO_PORT, RECEIVER3_GPIO_PORT, RECEIVER4_GPIO_PORT, RECEIVER5_GPIO_PORT, RECEIVER6_GPIO_PORT, RECEIVER7_GPIO_PORT, RECEIVER8_GPIO_PORT }
#define PIOS_PWM_GPIO_PINS			{ RECEIVER1_PIN, RECEIVER2_PIN, RECEIVER3_PIN, RECEIVER4_PIN, RECEIVER5_PIN, RECEIVER6_PIN, RECEIVER7_PIN, RECEIVER8_PIN }
#define PIOS_PWM_TIM_PORTS			{ RECEIVER1_TIM_PORT, RECEIVER2_TIM_PORT, RECEIVER3_TIM_PORT, RECEIVER4_TIM_PORT, RECEIVER5_TIM_PORT, RECEIVER6_TIM_PORT, RECEIVER7_TIM_PORT, RECEIVER8_TIM_PORT }
#define PIOS_PWM_TIM_CHANNELS			{ RECEIVER1_CH, RECEIVER2_CH, RECEIVER3_CH, RECEIVER4_CH, RECEIVER5_CH, RECEIVER6_CH, RECEIVER7_CH, RECEIVER8_CH }
#define PIOS_PWM_TIM_CCRS			{ RECEIVER1_CCR, RECEIVER2_CCR, RECEIVER3_CCR, RECEIVER4_CCR, RECEIVER5_CCR, RECEIVER6_CCR, RECEIVER7_CCR, RECEIVER8_CCR }
#define PIOS_PWM_TIMS				{ TIM1, TIM3, TIM5 }
#define PIOS_PWM_TIM_IRQS			{ TIM1_CC_IRQn, TIM3_IRQn, TIM5_IRQn }
#define PIOS_PWM_NUM_INPUTS			8
#define PIOS_PWM_NUM_TIMS			3
#define PIOS_PWM_SUPV_ENABLED			1
#define PIOS_PWM_SUPV_TIMER			TIM6
#define PIOS_PWM_SUPV_TIMER_RCC_FUNC		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE)
#define PIOS_PWM_SUPV_HZ			25
#define PIOS_PWM_SUPV_IRQ_CHANNEL		TIM6_IRQn
#define PIOS_PWM_SUPV_IRQ_FUNC			void TIM6_IRQHandler(void)

//-------------------------
// Servo outputs   
//-------------------------
#define PIOS_SERVO_GPIO_PORT_1TO4		GPIOB
#define PIOS_SERVO_GPIO_PIN_1			GPIO_Pin_6
#define PIOS_SERVO_GPIO_PIN_2			GPIO_Pin_7
#define PIOS_SERVO_GPIO_PIN_3			GPIO_Pin_8
#define PIOS_SERVO_GPIO_PIN_4			GPIO_Pin_9
#define PIOS_SERVO_GPIO_PORT_5TO8		GPIOC
#define PIOS_SERVO_GPIO_PIN_5			GPIO_Pin_6
#define PIOS_SERVO_GPIO_PIN_6			GPIO_Pin_7
#define PIOS_SERVO_GPIO_PIN_7			GPIO_Pin_8
#define PIOS_SERVO_GPIO_PIN_8			GPIO_Pin_9
#define PIOS_SERVO_NUM_OUTPUTS			8
#define PIOS_SERVO_NUM_TIMERS			PIOS_SERVO_NUM_OUTPUTS
#define PIOS_SERVO_UPDATE_HZ			50
#define PIOS_SERVOS_INITIAL_POSITION		1500

//-------------------------
// ADC
// PIOS_ADC_PinGet(0) = Temperature Sensor (On-board)
// PIOS_ADC_PinGet(1) = Power Sensor (Current)
// PIOS_ADC_PinGet(2) = Power Sensor (Voltage)
// PIOS_ADC_PinGet(3) = On-board 5v Rail Sensor
//-------------------------
//#define PIOS_ADC_OVERSAMPLING_RATE		1 			// (1 to 16)
#define PIOS_ADC_USE_TEMP_SENSOR		1
#define PIOS_ADC_TEMP_SENSOR_ADC		ADC1
#define PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL	1

#define PIOS_ADC_PIN1_GPIO_PORT			GPIOA			// PA1 (Power Sense - Voltage)
#define PIOS_ADC_PIN1_GPIO_PIN			GPIO_Pin_1		// ADC123_IN1
#define PIOS_ADC_PIN1_GPIO_CHANNEL		ADC_Channel_1
#define PIOS_ADC_PIN1_ADC			ADC1
#define PIOS_ADC_PIN1_ADC_NUMBER		2

#define PIOS_ADC_PIN2_GPIO_PORT			GPIOC			// PC3 (Power Sense - Current)
#define PIOS_ADC_PIN2_GPIO_PIN			GPIO_Pin_3		// ADC123_IN13
#define PIOS_ADC_PIN2_GPIO_CHANNEL		ADC_Channel_13
#define PIOS_ADC_PIN2_ADC			ADC2
#define PIOS_ADC_PIN2_ADC_NUMBER		1

#define PIOS_ADC_PIN3_GPIO_PORT			GPIOC			// PC5 (Onboard 5v Sensor) PC5
#define PIOS_ADC_PIN3_GPIO_PIN			GPIO_Pin_5		// ADC12_IN15
#define PIOS_ADC_PIN3_GPIO_CHANNEL		ADC_Channel_15
#define PIOS_ADC_PIN3_ADC			ADC2
#define PIOS_ADC_PIN3_ADC_NUMBER		2

#if 1
	#define PIOS_ADC_NUM_PINS			3
	#define PIOS_ADC_PORTS				{ PIOS_ADC_PIN1_GPIO_PORT, PIOS_ADC_PIN2_GPIO_PORT, PIOS_ADC_PIN3_GPIO_PORT }
	#define PIOS_ADC_PINS				{ PIOS_ADC_PIN1_GPIO_PIN, PIOS_ADC_PIN2_GPIO_PIN, PIOS_ADC_PIN3_GPIO_PIN }
	#define PIOS_ADC_CHANNELS			{ PIOS_ADC_PIN1_GPIO_CHANNEL, PIOS_ADC_PIN2_GPIO_CHANNEL, PIOS_ADC_PIN3_GPIO_CHANNEL }
	#define PIOS_ADC_MAPPING			{ PIOS_ADC_PIN1_ADC, PIOS_ADC_PIN2_ADC, PIOS_ADC_PIN3_ADC }
	#define PIOS_ADC_CHANNEL_MAPPING		{ PIOS_ADC_PIN1_ADC_NUMBER, PIOS_ADC_PIN2_ADC_NUMBER, PIOS_ADC_PIN3_ADC_NUMBER }
#else
	#define PIOS_ADC_PIN4_GPIO_PORT			GPIOC			// PC0 (AUX 1)
	#define PIOS_ADC_PIN4_GPIO_PIN			GPIO_Pin_0		// ADC123_IN10
	#define PIOS_ADC_PIN4_GPIO_CHANNEL		ADC_Channel_10
	#define PIOS_ADC_PIN4_ADC			ADC1
	#define PIOS_ADC_PIN4_ADC_NUMBER		3

	#define PIOS_ADC_PIN5_GPIO_PORT			GPIOC			// PC1 (AUX 2)
	#define PIOS_ADC_PIN5_GPIO_PIN			GPIO_Pin_1		// ADC123_IN11
	#define PIOS_ADC_PIN5_GPIO_CHANNEL		ADC_Channel_11
	#define PIOS_ADC_PIN5_ADC			ADC1
	#define PIOS_ADC_PIN5_ADC_NUMBER		4

	#define PIOS_ADC_PIN6_GPIO_PORT			GPIOC			// PC2 (AUX 3)
	#define PIOS_ADC_PIN6_GPIO_PIN			GPIO_Pin_2		// ADC123_IN12
	#define PIOS_ADC_PIN6_GPIO_CHANNEL		ADC_Channel_12
	#define PIOS_ADC_PIN6_ADC			ADC2
	#define PIOS_ADC_PIN6_ADC_NUMBER		3

	#define PIOS_ADC_NUM_PINS			6
	#define PIOS_ADC_PORTS				{ PIOS_ADC_PIN1_GPIO_PORT, PIOS_ADC_PIN2_GPIO_PORT, PIOS_ADC_PIN3_GPIO_PORT, PIOS_ADC_PIN4_GPIO_PORT, PIOS_ADC_PIN5_GPIO_PORT, PIOS_ADC_PIN6_GPIO_PORT }
	#define PIOS_ADC_PINS				{ PIOS_ADC_PIN1_GPIO_PIN, PIOS_ADC_PIN2_GPIO_PIN, PIOS_ADC_PIN3_GPIO_PIN, PIOS_ADC_PIN4_GPIO_PIN, PIOS_ADC_PIN5_GPIO_PIN, PIOS_ADC_PIN6_GPIO_PIN }
	#define PIOS_ADC_CHANNELS			{ PIOS_ADC_PIN1_GPIO_CHANNEL, PIOS_ADC_PIN2_GPIO_CHANNEL, PIOS_ADC_PIN3_GPIO_CHANNEL, PIOS_ADC_PIN4_GPIO_CHANNEL, PIOS_ADC_PIN5_GPIO_CHANNEL, PIOS_ADC_PIN6_GPIO_CHANNEL }
	#define PIOS_ADC_MAPPING			{ PIOS_ADC_PIN1_ADC, PIOS_ADC_PIN2_ADC, PIOS_ADC_PIN3_ADC, PIOS_ADC_PIN4_ADC, PIOS_ADC_PIN5_ADC, PIOS_ADC_PIN6_ADC }
	#define PIOS_ADC_CHANNEL_MAPPING		{ PIOS_ADC_PIN1_ADC_NUMBER, PIOS_ADC_PIN2_ADC_NUMBER, PIOS_ADC_PIN3_ADC_NUMBER, PIOS_ADC_PIN4_ADC_NUMBER, PIOS_ADC_PIN5_ADC_NUMBER, PIOS_ADC_PIN6_ADC_NUMBER }
#endif

#define PIOS_ADC_NUM_CHANNELS			(PIOS_ADC_NUM_PINS + PIOS_ADC_USE_TEMP_SENSOR)

#define PIOS_ADC_NUM_ADC_CHANNELS		2
#define PIOS_ADC_USE_ADC2			1
#define PIOS_ADC_CLOCK_FUNCTION			RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE)
#define PIOS_ADC_SAMPLE_TIME			ADC_SampleTime_239Cycles5 /* Sample time: */
										/* With an ADCCLK = 14 MHz and a sampling time of 293.5 cycles: */
										/* Tconv = 239.5 + 12.5 = 252 cycles = 18µs */
#define PIOS_ADC_IRQ_PRIO			PIOS_IRQ_PRIO_HIGH

//-------------------------  
// USB
//-------------------------
#define PIOS_USB_ENABLED			1
#define PIOS_USB_DETECT_GPIO_PORT		GPIOC
#define PIOS_USB_DETECT_GPIO_PIN		GPIO_Pin_4
#define PIOS_IRQ_USB_PRIORITY			PIOS_IRQ_PRIO_MID

#endif /* PIOS_BOARD_H */
