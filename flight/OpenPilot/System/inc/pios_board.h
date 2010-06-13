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
#define PIOS_I2C_PORT				I2C2
#define PIOS_I2C_CLK				RCC_APB1Periph_I2C2
#define PIOS_I2C_GPIO_PORT			GPIOB
#define PIOS_I2C_SDA_PIN			GPIO_Pin_11
#define PIOS_I2C_SCL_PIN			GPIO_Pin_10
#define PIOS_I2C_DUTY_CYCLE			I2C_DutyCycle_2
#define PIOS_I2C_BUS_FREQ			400000
#define PIOS_I2C_TIMEOUT_VALUE			5000
#define PIOS_I2C_IRQ_EV_HANDLER			void I2C2_EV_IRQHandler(void)
#define PIOS_I2C_IRQ_ER_HANDLER			void I2C2_ER_IRQHandler(void)
#define PIOS_I2C_IRQ_EV_CHANNEL			I2C2_EV_IRQn
#define PIOS_I2C_IRQ_ER_CHANNEL			I2C2_ER_IRQn
#define PIOS_I2C_IRQ_EV_PRIORITY		PIOS_IRQ_PRIO_HIGH
#define PIOS_I2C_IRQ_ER_PRIORITY		PIOS_IRQ_PRIO_HIGH

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
#define PIOS_BMP085_OVERSAMPLING		2

//-------------------------
// USART
//
// See also pios_board.c
//-------------------------
#define PIOS_USART_RX_BUFFER_SIZE		1024
#define PIOS_USART_TX_BUFFER_SIZE		256

#define PIOS_COM_TELEM_RF                       0
#define PIOS_COM_GPS                            1
#define PIOS_COM_TELEM_USB                      2

#if 0
#define PIOS_COM_AUX                            3
#define PIOS_COM_DEBUG                          PIOS_COM_AUX
#endif

//-------------------------
// SPI
//
// See also pios_board.c
//-------------------------
#define PIOS_SDCARD_SPI				0
#define PIOS_OPAHRS_SPI				1

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
#define PIOS_IRQ_PRIO_HIGHEST			4 		// for USART etc...

//-------------------------
// Receiver PWM inputs
//-------------------------
#define PIOS_PWM_CH1_GPIO_PORT			GPIOA
#define PIOS_PWM_CH1_PIN			GPIO_Pin_9
#define PIOS_PWM_CH1_TIM_PORT			TIM1
#define PIOS_PWM_CH1_CH				TIM_Channel_2
#define PIOS_PWM_CH1_CCR			TIM_IT_CC2
#define PIOS_PWM_CH2_GPIO_PORT			GPIOA
#define PIOS_PWM_CH2_PIN			GPIO_Pin_10
#define PIOS_PWM_CH2_TIM_PORT			TIM1
#define PIOS_PWM_CH2_CH				TIM_Channel_3
#define PIOS_PWM_CH2_CCR			TIM_IT_CC3
#define PIOS_PWM_CH3_GPIO_PORT			GPIOA
#define PIOS_PWM_CH3_PIN			GPIO_Pin_0
#define PIOS_PWM_CH3_TIM_PORT			TIM5
#define PIOS_PWM_CH3_CH				TIM_Channel_1
#define PIOS_PWM_CH3_CCR			TIM_IT_CC1
#define PIOS_PWM_CH4_GPIO_PORT			GPIOA
#define PIOS_PWM_CH4_PIN			GPIO_Pin_8
#define PIOS_PWM_CH4_TIM_PORT			TIM1
#define PIOS_PWM_CH4_CH				TIM_Channel_1
#define PIOS_PWM_CH4_CCR			TIM_IT_CC1
#define PIOS_PWM_CH5_GPIO_PORT			GPIOB
#define PIOS_PWM_CH5_PIN			GPIO_Pin_1
#define PIOS_PWM_CH5_TIM_PORT			TIM3
#define PIOS_PWM_CH5_CH				TIM_Channel_4
#define PIOS_PWM_CH5_CCR			TIM_IT_CC4
#define PIOS_PWM_CH6_GPIO_PORT			GPIOB
#define PIOS_PWM_CH6_PIN			GPIO_Pin_0
#define PIOS_PWM_CH6_TIM_PORT			TIM3
#define PIOS_PWM_CH6_CH				TIM_Channel_3
#define PIOS_PWM_CH6_CCR			TIM_IT_CC3
#define PIOS_PWM_CH7_GPIO_PORT			GPIOB
#define PIOS_PWM_CH7_PIN			GPIO_Pin_4
#define PIOS_PWM_CH7_TIM_PORT			TIM3
#define PIOS_PWM_CH7_CH				TIM_Channel_1
#define PIOS_PWM_CH7_CCR			TIM_IT_CC1
#define PIOS_PWM_CH8_GPIO_PORT			GPIOB
#define PIOS_PWM_CH8_PIN			GPIO_Pin_5
#define PIOS_PWM_CH8_TIM_PORT			TIM3
#define PIOS_PWM_CH8_CH				TIM_Channel_2
#define PIOS_PWM_CH8_CCR			TIM_IT_CC2
#define PIOS_PWM_GPIO_PORTS			{ PIOS_PWM_CH1_GPIO_PORT, PIOS_PWM_CH2_GPIO_PORT, PIOS_PWM_CH3_GPIO_PORT, PIOS_PWM_CH4_GPIO_PORT, PIOS_PWM_CH5_GPIO_PORT, PIOS_PWM_CH6_GPIO_PORT, PIOS_PWM_CH7_GPIO_PORT, PIOS_PWM_CH8_GPIO_PORT }
#define PIOS_PWM_GPIO_PINS			{ PIOS_PWM_CH1_PIN, PIOS_PWM_CH2_PIN, PIOS_PWM_CH3_PIN, PIOS_PWM_CH4_PIN, PIOS_PWM_CH5_PIN, PIOS_PWM_CH6_PIN, PIOS_PWM_CH7_PIN, PIOS_PWM_CH8_PIN }
#define PIOS_PWM_TIM_PORTS			{ PIOS_PWM_CH1_TIM_PORT, PIOS_PWM_CH2_TIM_PORT, PIOS_PWM_CH3_TIM_PORT, PIOS_PWM_CH4_TIM_PORT, PIOS_PWM_CH5_TIM_PORT, PIOS_PWM_CH6_TIM_PORT, PIOS_PWM_CH7_TIM_PORT, PIOS_PWM_CH8_TIM_PORT }
#define PIOS_PWM_TIM_CHANNELS			{ PIOS_PWM_CH1_CH, PIOS_PWM_CH2_CH, PIOS_PWM_CH3_CH, PIOS_PWM_CH4_CH, PIOS_PWM_CH5_CH, PIOS_PWM_CH6_CH, PIOS_PWM_CH7_CH, PIOS_PWM_CH8_CH }
#define PIOS_PWM_TIM_CCRS			{ PIOS_PWM_CH1_CCR, PIOS_PWM_CH2_CCR, PIOS_PWM_CH3_CCR, PIOS_PWM_CH4_CCR, PIOS_PWM_CH5_CCR, PIOS_PWM_CH6_CCR, PIOS_PWM_CH7_CCR, PIOS_PWM_CH8_CCR }
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
// Receiver PPM input
//-------------------------
#define PIOS_PPM_GPIO_PORT			PIOS_PWM_CH1_GPIO_PORT
#define PIOS_PPM_GPIO_PIN			PIOS_PWM_CH1_PIN
#define PIOS_PPM_TIM_PORT			PIOS_PWM_CH1_TIM_PORT
#define PIOS_PPM_TIM_CHANNEL			PIOS_PWM_CH1_CH
#define PIOS_PPM_TIM_CCR			PIOS_PWM_CH1_CCR
#define PIOS_PPM_TIM				TIM1
#define PIOS_PPM_TIM_IRQ			TIM1_CC_IRQn
#define PIOS_PPM_NUM_INPUTS			8  //Could be more if needed
#define PIOS_PPM_SUPV_ENABLED			1
#define PIOS_PPM_SUPV_TIMER			TIM6
#define PIOS_PPM_SUPV_TIMER_RCC_FUNC		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE)
#define PIOS_PPM_SUPV_HZ			25
#define PIOS_PPM_SUPV_IRQ_CHANNEL		TIM6_IRQn
#define PIOS_PPM_SUPV_IRQ_FUNC			void TIM6_IRQHandler(void)


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
// PIOS_ADC_PinGet(4) = Auxiliary Input 1
// PIOS_ADC_PinGet(5) = Auxiliary Input 2
// PIOS_ADC_PinGet(6) = Auxiliary Input 3
//-------------------------
//#define PIOS_ADC_OVERSAMPLING_RATE		1
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
#define PIOS_ADC_PIN4_GPIO_PORT			GPIOC			// PC0 (AUX 1)
#define PIOS_ADC_PIN4_GPIO_PIN			GPIO_Pin_0		// ADC123_IN10
#define PIOS_ADC_PIN4_GPIO_CHANNEL		ADC_Channel_10
#define PIOS_ADC_PIN4_ADC			ADC1
#define PIOS_ADC_PIN4_ADC_NUMBER		3
#define PIOS_ADC_PIN5_GPIO_PORT			GPIOC			// PC1 (AUX 2)
#define PIOS_ADC_PIN5_GPIO_PIN			GPIO_Pin_1		// ADC123_IN11
#define PIOS_ADC_PIN5_GPIO_CHANNEL		ADC_Channel_11
#define PIOS_ADC_PIN5_ADC			ADC2
#define PIOS_ADC_PIN5_ADC_NUMBER		3
#define PIOS_ADC_PIN6_GPIO_PORT			GPIOC			// PC2 (AUX 3)
#define PIOS_ADC_PIN6_GPIO_PIN			GPIO_Pin_2		// ADC123_IN12
#define PIOS_ADC_PIN6_GPIO_CHANNEL		ADC_Channel_12
#define PIOS_ADC_PIN6_ADC			ADC1
#define PIOS_ADC_PIN6_ADC_NUMBER		4
#define PIOS_ADC_NUM_PINS			6
#define PIOS_ADC_PORTS				{ PIOS_ADC_PIN1_GPIO_PORT, PIOS_ADC_PIN2_GPIO_PORT, PIOS_ADC_PIN3_GPIO_PORT, PIOS_ADC_PIN4_GPIO_PORT, PIOS_ADC_PIN5_GPIO_PORT, PIOS_ADC_PIN6_GPIO_PORT }
#define PIOS_ADC_PINS				{ PIOS_ADC_PIN1_GPIO_PIN, PIOS_ADC_PIN2_GPIO_PIN, PIOS_ADC_PIN3_GPIO_PIN, PIOS_ADC_PIN4_GPIO_PIN, PIOS_ADC_PIN5_GPIO_PIN, PIOS_ADC_PIN6_GPIO_PIN }
#define PIOS_ADC_CHANNELS			{ PIOS_ADC_PIN1_GPIO_CHANNEL, PIOS_ADC_PIN2_GPIO_CHANNEL, PIOS_ADC_PIN3_GPIO_CHANNEL, PIOS_ADC_PIN4_GPIO_CHANNEL, PIOS_ADC_PIN5_GPIO_CHANNEL, PIOS_ADC_PIN6_GPIO_CHANNEL }
#define PIOS_ADC_MAPPING			{ PIOS_ADC_PIN1_ADC, PIOS_ADC_PIN2_ADC, PIOS_ADC_PIN3_ADC, PIOS_ADC_PIN4_ADC, PIOS_ADC_PIN5_ADC, PIOS_ADC_PIN6_ADC }
#define PIOS_ADC_CHANNEL_MAPPING		{ PIOS_ADC_PIN1_ADC_NUMBER, PIOS_ADC_PIN2_ADC_NUMBER, PIOS_ADC_PIN3_ADC_NUMBER, PIOS_ADC_PIN4_ADC_NUMBER, PIOS_ADC_PIN5_ADC_NUMBER, PIOS_ADC_PIN6_ADC_NUMBER }
#define PIOS_ADC_NUM_CHANNELS			(PIOS_ADC_NUM_PINS + PIOS_ADC_USE_TEMP_SENSOR)
#define PIOS_ADC_NUM_ADC_CHANNELS		2
#define PIOS_ADC_USE_ADC2			1
#define PIOS_ADC_CLOCK_FUNCTION			RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE)
#define PIOS_ADC_ADCCLK				RCC_PCLK2_Div8
						/* RCC_PCLK2_Div2: ADC clock = PCLK2/2 */
						/* RCC_PCLK2_Div4: ADC clock = PCLK2/4 */
						/* RCC_PCLK2_Div6: ADC clock = PCLK2/6 */
						/* RCC_PCLK2_Div8: ADC clock = PCLK2/8 */
#define PIOS_ADC_SAMPLE_TIME			ADC_SampleTime_239Cycles5
						/* Sample time: */
						/* With an ADCCLK = 14 MHz and a sampling time of 293.5 cycles: */
						/* Tconv = 239.5 + 12.5 = 252 cycles = 18�s */
						/* (1 / (ADCCLK / CYCLES)) = Sample Time (�S) */
#define PIOS_ADC_IRQ_PRIO			PIOS_IRQ_PRIO_HIGH

//-------------------------  
// GPIO
//-------------------------
#define PIOS_GPIO_1_PORT			GPIOC
#define PIOS_GPIO_1_PIN				GPIO_Pin_0
#define PIOS_GPIO_1_GPIO_CLK			RCC_APB2Periph_GPIOC
#define PIOS_GPIO_2_PORT			GPIOC
#define PIOS_GPIO_2_PIN				GPIO_Pin_1
#define PIOS_GPIO_2_GPIO_CLK			RCC_APB2Periph_GPIOC
#define PIOS_GPIO_3_PORT			GPIOC
#define PIOS_GPIO_3_PIN				GPIO_Pin_2
#define PIOS_GPIO_3_GPIO_CLK			RCC_APB2Periph_GPIOC
#define PIOS_GPIO_4_PORT			GPIOD
#define PIOS_GPIO_4_PIN				GPIO_Pin_2
#define PIOS_GPIO_4_GPIO_CLK			RCC_APB2Periph_GPIOD
#define PIOS_GPIO_PORTS				{ PIOS_GPIO_1_PORT, PIOS_GPIO_2_PORT, PIOS_GPIO_3_PORT, PIOS_GPIO_4_PORT }
#define PIOS_GPIO_PINS				{ PIOS_GPIO_1_PIN, PIOS_GPIO_2_PIN, PIOS_GPIO_3_PIN, PIOS_GPIO_4_PIN }
#define PIOS_GPIO_CLKS				{ PIOS_GPIO_1_GPIO_CLK, PIOS_GPIO_2_GPIO_CLK, PIOS_GPIO_3_GPIO_CLK, PIOS_GPIO_4_GPIO_CLK }
#define PIOS_GPIO_NUM				4

//-------------------------
// USB
//-------------------------
#define PIOS_USB_ENABLED			1
#define PIOS_USB_DETECT_GPIO_PORT		GPIOC
#define PIOS_USB_DETECT_GPIO_PIN		GPIO_Pin_4
#define PIOS_USB_DETECT_EXTI_LINE		EXTI_Line4
#define PIOS_IRQ_USB_PRIORITY			PIOS_IRQ_PRIO_MID

#endif /* PIOS_BOARD_H */
