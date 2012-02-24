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


#ifndef STM32103C8_TRANSMITTER_H_
#define STM32103C8_TRANSMITTER_H_

#define ADD_ONE_ADC

//------------------------
// Timers and Channels Used
//------------------------
/*
Timer | Channel 1 | Channel 2 | Channel 3 | Channel 4
------+-----------+-----------+-----------+----------
TIM1  |  Servo 4  |           |           |
TIM2  |  RC In 5  |  RC In 6  |  Servo 6  |
TIM3  |  Servo 5  |  RC In 2  |  RC In 3  |  RC In 4
TIM4  |  RC In 1  |  Servo 3  |  Servo 2  |  Servo 1
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
#define BOARD_READABLE	TRUE
#define BOARD_WRITABLE	TRUE
#define MAX_DEL_RETRYS	3


//------------------------
// WATCHDOG_SETTINGS
//------------------------
#define PIOS_WATCHDOG_TIMEOUT    250
#define PIOS_WDG_REGISTER        BKP_DR4
#define PIOS_WDG_ACTUATOR        0x0001
#define PIOS_WDG_STABILIZATION   0x0002
#define PIOS_WDG_ATTITUDE        0x0004
#define PIOS_WDG_MANUAL          0x0008

//------------------------
// TELEMETRY
//------------------------
#define TELEM_QUEUE_SIZE         20

//------------------------
// PIOS_LED
//------------------------
#define PIOS_LED_HEARTBEAT	0
#ifdef MOVE_CONTROLLER
#define PIOS_LED_LED1_GPIO_PORT			GPIOC
//#define PIOS_LED_LED1_GPIO_PIN			GPIO_Pin_9
#define PIOS_LED_LED1_GPIO_PIN			GPIO_Pin_8
#define PIOS_LED_LED1_GPIO_CLK			RCC_APB2Periph_GPIOC
#define PIOS_LED_NUM				1
#define PIOS_LED_PORTS				{ PIOS_LED_LED1_GPIO_PORT }
#define PIOS_LED_PINS				{ PIOS_LED_LED1_GPIO_PIN }
#define PIOS_LED_CLKS				{ PIOS_LED_LED1_GPIO_CLK }

#define PIOS_FLASH_CS_PIN                       0
#define PIOS_FLASH_ENABLE                       PIOS_GPIO_On(0)
#define PIOS_FLASH_DISABLE                      PIOS_GPIO_Off(0)
#define PIOS_ADXL_ENABLE                        PIOS_SPI_RC_PinSet(PIOS_SPI_ACCEL,0)
#define PIOS_ADXL_DISABLE                       PIOS_SPI_RC_PinSet(PIOS_SPI_ACCEL,1)

// *****************************************************************
// PIOS_LED

#define PIOS_LED_USB	0
#define PIOS_LED_LINK	1
#define PIOS_LED_RX	2
#define PIOS_LED_TX	3

#define PIOS_LED_HEARTBEAT PIOS_LED_USB
#define PIOS_LED_ALARM PIOS_LED_TX

#define USB_LED_ON					PIOS_LED_On(PIOS_LED_USB)
#define USB_LED_OFF					PIOS_LED_Off(PIOS_LED_USB)
#define USB_LED_TOGGLE					PIOS_LED_Toggle(PIOS_LED_USB)

#define LINK_LED_ON					PIOS_LED_On(PIOS_LED_LINK)
#define LINK_LED_OFF					PIOS_LED_Off(PIOS_LED_LINK)
#define LINK_LED_TOGGLE					PIOS_LED_Toggle(PIOS_LED_LINK)

#define RX_LED_ON					PIOS_LED_On(PIOS_LED_RX)
#define RX_LED_OFF					PIOS_LED_Off(PIOS_LED_RX)
#define RX_LED_TOGGLE					PIOS_LED_Toggle(PIOS_LED_RX)

#define TX_LED_ON					PIOS_LED_On(PIOS_LED_TX)
#define TX_LED_OFF					PIOS_LED_Off(PIOS_LED_TX)
#define TX_LED_TOGGLE					PIOS_LED_Toggle(PIOS_LED_TX)

//------------------------
// PIOS_AK8794
//------------------------
#define PIOS_AK8974_DRDY_GPIO_PORT		GPIOE
#define PIOS_AK8974_DRDY_GPIO_PIN		GPIO_Pin_12
#define PIOS_AK8974_DRDY_PORT_SOURCE		GPIO_PortSourceGPIOE
#define PIOS_AK8974_DRDY_PIN_SOURCE		GPIO_PinSource12
#define PIOS_AK8974_DRDY_CLK			RCC_APB2Periph_GPIOE
#define PIOS_AK8974_DRDY_EXTI_LINE		EXTI_Line12
#define PIOS_AK8974_DRDY_IRQn			EXTI15_10_IRQn

#define PIOS_AK8974_DRDY_PRIO			PIOS_IRQ_PRIO_HIGH

#else

#define PIOS_LED_LED1_GPIO_PORT			GPIOB
#ifdef MAPLE_MINI
#define PIOS_LED_LED1_GPIO_PIN			GPIO_Pin_1
#else
#define PIOS_LED_LED1_GPIO_PIN			GPIO_Pin_8
#endif
#define PIOS_LED_LED1_GPIO_CLK			RCC_APB2Periph_GPIOB
#define PIOS_LED_LED2_GPIO_PORT                 GPIOB
#define PIOS_LED_LED2_GPIO_PIN                  GPIO_Pin_9
#define PIOS_LED_LED2_GPIO_CLK                  RCC_APB2Periph_GPIOB
#define PIOS_LED_NUM                            2
#define PIOS_LED_PORTS                          { PIOS_LED_LED1_GPIO_PORT, PIOS_LED_LED2_GPIO_PORT }
#define PIOS_LED_PINS                           { PIOS_LED_LED1_GPIO_PIN, PIOS_LED_LED2_GPIO_PIN }
#define PIOS_LED_CLKS                           { PIOS_LED_LED1_GPIO_CLK, PIOS_LED_LED2_GPIO_CLK }
#endif

//-------------------------
// System Settings
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

//------------------------
// PIOS_I2C
// See also pios_board.c
//------------------------
#define PIOS_I2C_MAX_DEVS			1
extern uint32_t pios_i2c_flexi_adapter_id;
#define PIOS_I2C_MAIN_ADAPTER			(pios_i2c_flexi_adapter_id)

//-------------------------
// SPI
//
// See also pios_board.c
//-------------------------
#define PIOS_SPI_MAX_DEVS			2

//-------------------------
// PIOS_USART
//-------------------------
#define PIOS_USART_MAX_DEVS			3

//-------------------------
// PIOS_COM
//
// See also pios_board.c
//-------------------------
#define PIOS_COM_MAX_DEVS			4

extern uint32_t pios_com_telem_usb_id;
#if defined(MOVE_CONTROLLER)
extern uint32_t pios_com_usart1_id;
#define PIOS_COM_TELEM_GCS              (pios_com_usart1_id)
#define PIOS_COM_TELEM_OUT              (pios_com_usart1_id)
#define PIOS_COM_DEBUG                  (pios_com_usart1_id)

#elif defined(ANALOG_TRANSMITTER)
extern uint32_t pios_com_usart1_id;
extern uint32_t pios_com_usart2_id;
extern uint32_t pios_com_usart3_id;
// 1 = Xbee, 2 = Extra, 3 = USB
// Debug mode
//#define PIOS_COM_TELEM_GCS              (pios_com_usart1_id)
//#define PIOS_COM_TELEM_OUT              (pios_com_usart2_id)
//#define PIOS_COM_DEBUG                  (pios_com_usart3_id)

// Normal mode
//#define PIOS_COM_TELEM_GCS              (pios_com_usart3_id)
//#define PIOS_COM_TELEM_OUT              (pios_com_usart1_id)
//#define PIOS_COM_DEBUG                  (pios_com_usart2_id)

#elif defined(MAPLE_MINI)
extern uint32_t pios_com_usart1_id;
extern uint32_t pios_com_usart2_id;
extern uint32_t pios_com_usart3_id;
// On Maple Mini:
// 1 = Debug, 2 = Xbee, 3 = GCS
#define PIOS_COM_TELEM_GCS              (pios_com_usart3_id)
#define PIOS_COM_TELEM_OUT              (pios_com_usart2_id)
#define PIOS_COM_DEBUG                  (pios_com_usart1_id)
#endif

#define PIOS_COM_TELEM_USB              (pios_com_telem_usb_id)

#if defined(ANALOG_TRANSMITTER)

//-------------------------
// ADC
// PIOS_ADC_PinGet(0) = Roll Prim
// PIOS_ADC_PinGet(1) = Roll Trim
// PIOS_ADC_PinGet(2) = Pitch Prim
// PIOS_ADC_PinGet(3) = Pitch Trim
// PIOS_ADC_PinGet(4) = Yaw Prim
// PIOS_ADC_PinGet(5) = Yaw Trim
// PIOS_ADC_PinGet(6) = Throt Prim
// PIOS_ADC_PinGet(7) = Throt Trim
// PIOS_ADC_PinGet(8) = Poten
//-------------------------
//#define PIOS_ADC_OVERSAMPLING_RATE		1
#define PIOS_ADC_USE_TEMP_SENSOR		1
#define PIOS_ADC_TEMP_SENSOR_ADC		ADC1
#define PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL	1

#define PIOS_ADC_PIN1_GPIO_PORT			GPIOA			// PA0
#define PIOS_ADC_PIN1_GPIO_PIN			GPIO_Pin_0		// ADC12_IN0
#define PIOS_ADC_PIN1_GPIO_CHANNEL		ADC_Channel_0
#define PIOS_ADC_PIN1_ADC			ADC2
#define PIOS_ADC_PIN1_ADC_NUMBER		1

#define PIOS_ADC_PIN2_GPIO_PORT			GPIOA			// PA1
#define PIOS_ADC_PIN2_GPIO_PIN			GPIO_Pin_1		// ADC123_IN1
#define PIOS_ADC_PIN2_GPIO_CHANNEL		ADC_Channel_1
#define PIOS_ADC_PIN2_ADC			ADC1
#define PIOS_ADC_PIN2_ADC_NUMBER		2

#define PIOS_ADC_PIN3_GPIO_PORT			GPIOA			// PA2
#define PIOS_ADC_PIN3_GPIO_PIN			GPIO_Pin_2		// ADC12_IN2
#define PIOS_ADC_PIN3_GPIO_CHANNEL		ADC_Channel_2
#define PIOS_ADC_PIN3_ADC			ADC2
#define PIOS_ADC_PIN3_ADC_NUMBER		2

#define PIOS_ADC_PIN4_GPIO_PORT			GPIOA			// PA3
#define PIOS_ADC_PIN4_GPIO_PIN			GPIO_Pin_3		// ADC12_IN3
#define PIOS_ADC_PIN4_GPIO_CHANNEL		ADC_Channel_3
#define PIOS_ADC_PIN4_ADC			ADC1
#define PIOS_ADC_PIN4_ADC_NUMBER		3

#define PIOS_ADC_PIN5_GPIO_PORT			GPIOA			// PA4
#define PIOS_ADC_PIN5_GPIO_PIN			GPIO_Pin_4		// ADC12_IN4
#define PIOS_ADC_PIN5_GPIO_CHANNEL		ADC_Channel_4
#define PIOS_ADC_PIN5_ADC			ADC2
#define PIOS_ADC_PIN5_ADC_NUMBER		3

#define PIOS_ADC_PIN6_GPIO_PORT			GPIOA			// PA5
#define PIOS_ADC_PIN6_GPIO_PIN			GPIO_Pin_5		// ADC12_IN5
#define PIOS_ADC_PIN6_GPIO_CHANNEL		ADC_Channel_5
#define PIOS_ADC_PIN6_ADC			ADC1
#define PIOS_ADC_PIN6_ADC_NUMBER		4

#define PIOS_ADC_PIN7_GPIO_PORT			GPIOA			// PA6
#define PIOS_ADC_PIN7_GPIO_PIN			GPIO_Pin_6		// ADC12_IN6
#define PIOS_ADC_PIN7_GPIO_CHANNEL		ADC_Channel_6
#define PIOS_ADC_PIN7_ADC			ADC2
#define PIOS_ADC_PIN7_ADC_NUMBER		4

#define PIOS_ADC_PIN8_GPIO_PORT			GPIOA			// PA7
#define PIOS_ADC_PIN8_GPIO_PIN			GPIO_Pin_7		// ADC12_IN7
#define PIOS_ADC_PIN8_GPIO_CHANNEL		ADC_Channel_7
#define PIOS_ADC_PIN8_ADC			ADC1
#define PIOS_ADC_PIN8_ADC_NUMBER		5

#define PIOS_ADC_PIN9_GPIO_PORT			GPIOB			// PB0
#define PIOS_ADC_PIN9_GPIO_PIN			GPIO_Pin_0		// ADC12_IN8
#define PIOS_ADC_PIN9_GPIO_CHANNEL		ADC_Channel_8
#define PIOS_ADC_PIN9_ADC			ADC2
#define PIOS_ADC_PIN9_ADC_NUMBER		5

#define PIOS_ADC_NUM_PINS			9

#define PIOS_ADC_PORTS				{ PIOS_ADC_PIN1_GPIO_PORT, PIOS_ADC_PIN2_GPIO_PORT, PIOS_ADC_PIN3_GPIO_PORT, PIOS_ADC_PIN4_GPIO_PORT, PIOS_ADC_PIN5_GPIO_PORT, PIOS_ADC_PIN6_GPIO_PORT, PIOS_ADC_PIN7_GPIO_PORT, PIOS_ADC_PIN8_GPIO_PORT, PIOS_ADC_PIN9_GPIO_PORT }
#define PIOS_ADC_PINS				{ PIOS_ADC_PIN1_GPIO_PIN, PIOS_ADC_PIN2_GPIO_PIN, PIOS_ADC_PIN3_GPIO_PIN, PIOS_ADC_PIN4_GPIO_PIN, PIOS_ADC_PIN5_GPIO_PIN, PIOS_ADC_PIN6_GPIO_PIN, PIOS_ADC_PIN7_GPIO_PIN, PIOS_ADC_PIN8_GPIO_PIN, PIOS_ADC_PIN9_GPIO_PIN }
#define PIOS_ADC_CHANNELS			{ PIOS_ADC_PIN1_GPIO_CHANNEL, PIOS_ADC_PIN2_GPIO_CHANNEL, PIOS_ADC_PIN3_GPIO_CHANNEL, PIOS_ADC_PIN4_GPIO_CHANNEL, PIOS_ADC_PIN5_GPIO_CHANNEL, PIOS_ADC_PIN6_GPIO_CHANNEL, PIOS_ADC_PIN7_GPIO_CHANNEL, PIOS_ADC_PIN8_GPIO_CHANNEL, PIOS_ADC_PIN9_GPIO_CHANNEL }
#define PIOS_ADC_MAPPING			{ PIOS_ADC_PIN1_ADC, PIOS_ADC_PIN2_ADC, PIOS_ADC_PIN3_ADC, PIOS_ADC_PIN4_ADC, PIOS_ADC_PIN5_ADC, PIOS_ADC_PIN6_ADC, PIOS_ADC_PIN7_ADC, PIOS_ADC_PIN8_ADC, PIOS_ADC_PIN9_ADC }
#define PIOS_ADC_CHANNEL_MAPPING		{ PIOS_ADC_PIN1_ADC_NUMBER, PIOS_ADC_PIN2_ADC_NUMBER, PIOS_ADC_PIN3_ADC_NUMBER, PIOS_ADC_PIN4_ADC_NUMBER, PIOS_ADC_PIN5_ADC_NUMBER, PIOS_ADC_PIN6_ADC_NUMBER, PIOS_ADC_PIN7_ADC_NUMBER, PIOS_ADC_PIN8_ADC_NUMBER, PIOS_ADC_PIN9_ADC_NUMBER }

#define PIOS_ADC_NUM_ADC_CHANNELS		2
#define PIOS_ADC_USE_ADC2			1

#elif defined(MAPLE_MINI)

//-------------------------
// ADC
// PIOS_ADC_PinGet(0) = Roll
// PIOS_ADC_PinGet(1) = Pitch
// PIOS_ADC_PinGet(2) = Yaw
// PIOS_ADC_PinGet(3) = Throt
//-------------------------
//#define PIOS_ADC_OVERSAMPLING_RATE		1
//#define PIOS_ADC_USE_TEMP_SENSOR		1
#define PIOS_ADC_USE_TEMP_SENSOR		0
#define PIOS_ADC_TEMP_SENSOR_ADC		ADC1
#define PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL	1

#define PIOS_ADC_PIN1_GPIO_PORT			GPIOA			// PA4
#define PIOS_ADC_PIN1_GPIO_PIN			GPIO_Pin_4		// ADC12_IN4
#define PIOS_ADC_PIN1_GPIO_CHANNEL		ADC_Channel_4
#define PIOS_ADC_PIN1_ADC			ADC1
#define PIOS_ADC_PIN1_ADC_NUMBER		1

#define PIOS_ADC_PIN2_GPIO_PORT			GPIOA			// PA5
#define PIOS_ADC_PIN2_GPIO_PIN			GPIO_Pin_5		// ADC12_IN5
#define PIOS_ADC_PIN2_GPIO_CHANNEL		ADC_Channel_5
#define PIOS_ADC_PIN2_ADC			ADC2
#define PIOS_ADC_PIN2_ADC_NUMBER		1

#define PIOS_ADC_PIN3_GPIO_PORT			GPIOA			// PA6
#define PIOS_ADC_PIN3_GPIO_PIN			GPIO_Pin_6		// ADC12_IN6
#define PIOS_ADC_PIN3_GPIO_CHANNEL		ADC_Channel_6
#define PIOS_ADC_PIN3_ADC			ADC1
#define PIOS_ADC_PIN3_ADC_NUMBER		2

#define PIOS_ADC_PIN4_GPIO_PORT			GPIOA			// PA7
#define PIOS_ADC_PIN4_GPIO_PIN			GPIO_Pin_7		// ADC12_IN7
#define PIOS_ADC_PIN4_GPIO_CHANNEL		ADC_Channel_7
#define PIOS_ADC_PIN4_ADC			ADC2
#define PIOS_ADC_PIN4_ADC_NUMBER		2

#ifdef NEVER
#define PIOS_ADC_PIN5_GPIO_PORT			GPIOA			// PA0
#define PIOS_ADC_PIN5_GPIO_PIN			GPIO_Pin_0		// ADC12_IN0
#define PIOS_ADC_PIN5_GPIO_CHANNEL		ADC_Channel_0
#define PIOS_ADC_PIN5_ADC			ADC2
#define PIOS_ADC_PIN5_ADC_NUMBER		3

#define PIOS_ADC_NUM_PINS			5

#define PIOS_ADC_PORTS				{ PIOS_ADC_PIN1_GPIO_PORT, PIOS_ADC_PIN2_GPIO_PORT, PIOS_ADC_PIN3_GPIO_PORT, PIOS_ADC_PIN4_GPIO_PORT, PIOS_ADC_PIN5_GPIO_PORT }
#define PIOS_ADC_PINS				{ PIOS_ADC_PIN1_GPIO_PIN, PIOS_ADC_PIN2_GPIO_PIN, PIOS_ADC_PIN3_GPIO_PIN, PIOS_ADC_PIN4_GPIO_PIN, PIOS_ADC_PIN5_GPIO_PIN }
#define PIOS_ADC_CHANNELS			{ PIOS_ADC_PIN1_GPIO_CHANNEL, PIOS_ADC_PIN2_GPIO_CHANNEL, PIOS_ADC_PIN3_GPIO_CHANNEL, PIOS_ADC_PIN4_GPIO_CHANNEL, PIOS_ADC_PIN5_GPIO_CHANNEL }
#define PIOS_ADC_MAPPING			{ PIOS_ADC_PIN1_ADC, PIOS_ADC_PIN2_ADC, PIOS_ADC_PIN3_ADC, PIOS_ADC_PIN4_ADC, PIOS_ADC_PIN5_ADC }
#define PIOS_ADC_CHANNEL_MAPPING		{ PIOS_ADC_PIN1_ADC_NUMBER, PIOS_ADC_PIN2_ADC_NUMBER, PIOS_ADC_PIN3_ADC_NUMBER, PIOS_ADC_PIN4_ADC_NUMBER, PIOS_ADC_PIN5_ADC_NUMBER }
#endif

#define PIOS_ADC_NUM_PINS			4

#define PIOS_ADC_PORTS				{ PIOS_ADC_PIN1_GPIO_PORT, PIOS_ADC_PIN2_GPIO_PORT, PIOS_ADC_PIN3_GPIO_PORT, PIOS_ADC_PIN4_GPIO_PORT }
#define PIOS_ADC_PINS				{ PIOS_ADC_PIN1_GPIO_PIN, PIOS_ADC_PIN2_GPIO_PIN, PIOS_ADC_PIN3_GPIO_PIN, PIOS_ADC_PIN4_GPIO_PIN }
#define PIOS_ADC_CHANNELS			{ PIOS_ADC_PIN1_GPIO_CHANNEL, PIOS_ADC_PIN2_GPIO_CHANNEL, PIOS_ADC_PIN3_GPIO_CHANNEL, PIOS_ADC_PIN4_GPIO_CHANNEL }
#define PIOS_ADC_MAPPING			{ PIOS_ADC_PIN1_ADC, PIOS_ADC_PIN2_ADC, PIOS_ADC_PIN3_ADC, PIOS_ADC_PIN4_ADC }
#define PIOS_ADC_CHANNEL_MAPPING		{ PIOS_ADC_PIN1_ADC_NUMBER, PIOS_ADC_PIN2_ADC_NUMBER, PIOS_ADC_PIN3_ADC_NUMBER, PIOS_ADC_PIN4_ADC_NUMBER }

#define PIOS_ADC_NUM_ADC_CHANNELS		2
#define PIOS_ADC_USE_ADC2			1

#elif defined(MOVE_CONTROLLER)

//-------------------------
// ADC
// PIOS_ADC_PinGet(0) = Accel X
// PIOS_ADC_PinGet(1) = Accel Y
// PIOS_ADC_PinGet(2) = Accel Z
// PIOS_ADC_PinGet(3) = (ND)
// PIOS_ADC_PinGet(4) = Gyro X
// PIOS_ADC_PinGet(5) = Gyro Y
// PIOS_ADC_PinGet(6) = Gyro Z
// PIOS_ADC_PinGet(7) = (ND)
//-------------------------
//#define PIOS_ADC_OVERSAMPLING_RATE		1
#define PIOS_ADC_USE_TEMP_SENSOR		0
#define PIOS_ADC_TEMP_SENSOR_ADC		ADC1
#define PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL	1

#define PIOS_ADC_PIN1_GPIO_PORT			GPIOA			// PA1 (Accel X)
#define PIOS_ADC_PIN1_GPIO_PIN			GPIO_Pin_1		// ADC12_IN1
#define PIOS_ADC_PIN1_GPIO_CHANNEL		ADC_Channel_1
#define PIOS_ADC_PIN1_ADC			ADC1
#define PIOS_ADC_PIN1_ADC_NUMBER		1

#define PIOS_ADC_PIN2_GPIO_PORT			GPIOC			// PC3 (Accel Y)
#define PIOS_ADC_PIN2_GPIO_PIN			GPIO_Pin_3		// ADC12_IN13
#define PIOS_ADC_PIN2_GPIO_CHANNEL		ADC_Channel_13
#define PIOS_ADC_PIN2_ADC			ADC2
#define PIOS_ADC_PIN2_ADC_NUMBER		1

#define PIOS_ADC_PIN3_GPIO_PORT			GPIOA			// PA3 (Accel Z)
#define PIOS_ADC_PIN3_GPIO_PIN			GPIO_Pin_3		// ADC12_IN3
#define PIOS_ADC_PIN3_GPIO_CHANNEL		ADC_Channel_3
#define PIOS_ADC_PIN3_ADC			ADC1
#define PIOS_ADC_PIN3_ADC_NUMBER		2

#define PIOS_ADC_PIN4_GPIO_PORT			GPIOA			// PA6 (not used)
#define PIOS_ADC_PIN4_GPIO_PIN			GPIO_Pin_6		// ADC12_IN6
#define PIOS_ADC_PIN4_GPIO_CHANNEL		ADC_Channel_6
#define PIOS_ADC_PIN4_ADC			ADC2
#define PIOS_ADC_PIN4_ADC_NUMBER		2

#define PIOS_ADC_PIN5_GPIO_PORT			GPIOC			// PC1 (Gyro X)
#define PIOS_ADC_PIN5_GPIO_PIN			GPIO_Pin_1		// ADC12_IN11
#define PIOS_ADC_PIN5_GPIO_CHANNEL		ADC_Channel_11
#define PIOS_ADC_PIN5_ADC			ADC1
#define PIOS_ADC_PIN5_ADC_NUMBER		3

#define PIOS_ADC_PIN6_GPIO_PORT			GPIOC			// PC0 (Gyro Y)
#define PIOS_ADC_PIN6_GPIO_PIN			GPIO_Pin_0		// ADC12_IN10
#define PIOS_ADC_PIN6_GPIO_CHANNEL		ADC_Channel_10
#define PIOS_ADC_PIN6_ADC			ADC2
#define PIOS_ADC_PIN6_ADC_NUMBER		3

#define PIOS_ADC_PIN7_GPIO_PORT			GPIOC			// PC2 (Gyro Z)
#define PIOS_ADC_PIN7_GPIO_PIN			GPIO_Pin_2		// ADC12_IN12
#define PIOS_ADC_PIN7_GPIO_CHANNEL		ADC_Channel_12
#define PIOS_ADC_PIN7_ADC			ADC1
#define PIOS_ADC_PIN7_ADC_NUMBER		4

#define PIOS_ADC_PIN8_GPIO_PORT			GPIOB			// PB1 (not used)
#define PIOS_ADC_PIN8_GPIO_PIN			GPIO_Pin_1		// ADC12_IN9
#define PIOS_ADC_PIN8_GPIO_CHANNEL		ADC_Channel_9
#define PIOS_ADC_PIN8_ADC			ADC2
#define PIOS_ADC_PIN8_ADC_NUMBER		4

#define PIOS_ADC_NUM_PINS			8

#define PIOS_ADC_PORTS				{ PIOS_ADC_PIN1_GPIO_PORT, PIOS_ADC_PIN2_GPIO_PORT, PIOS_ADC_PIN3_GPIO_PORT, PIOS_ADC_PIN4_GPIO_PORT, PIOS_ADC_PIN5_GPIO_PORT, PIOS_ADC_PIN6_GPIO_PORT, PIOS_ADC_PIN7_GPIO_PORT, PIOS_ADC_PIN8_GPIO_PORT }
#define PIOS_ADC_PINS				{ PIOS_ADC_PIN1_GPIO_PIN, PIOS_ADC_PIN2_GPIO_PIN, PIOS_ADC_PIN3_GPIO_PIN, PIOS_ADC_PIN4_GPIO_PIN, PIOS_ADC_PIN5_GPIO_PIN, PIOS_ADC_PIN6_GPIO_PIN, PIOS_ADC_PIN7_GPIO_PIN, PIOS_ADC_PIN8_GPIO_PIN }
#define PIOS_ADC_CHANNELS			{ PIOS_ADC_PIN1_GPIO_CHANNEL, PIOS_ADC_PIN2_GPIO_CHANNEL, PIOS_ADC_PIN3_GPIO_CHANNEL, PIOS_ADC_PIN4_GPIO_CHANNEL, PIOS_ADC_PIN5_GPIO_CHANNEL, PIOS_ADC_PIN6_GPIO_CHANNEL, PIOS_ADC_PIN7_GPIO_CHANNEL, PIOS_ADC_PIN8_GPIO_CHANNEL }
#define PIOS_ADC_MAPPING			{ PIOS_ADC_PIN1_ADC, PIOS_ADC_PIN2_ADC, PIOS_ADC_PIN3_ADC, PIOS_ADC_PIN4_ADC, PIOS_ADC_PIN5_ADC, PIOS_ADC_PIN6_ADC, PIOS_ADC_PIN7_ADC, PIOS_ADC_PIN8_ADC }
#define PIOS_ADC_CHANNEL_MAPPING		{ PIOS_ADC_PIN1_ADC_NUMBER, PIOS_ADC_PIN2_ADC_NUMBER, PIOS_ADC_PIN3_ADC_NUMBER, PIOS_ADC_PIN4_ADC_NUMBER, PIOS_ADC_PIN5_ADC_NUMBER, PIOS_ADC_PIN6_ADC_NUMBER, PIOS_ADC_PIN7_ADC_NUMBER,PIOS_ADC_PIN8_ADC_NUMBER }

#define PIOS_ADC_NUM_ADC_CHANNELS		2
#define PIOS_ADC_USE_ADC2			1

#else

#define PIOS_ADC_USE_TEMP_SENSOR		0
#define PIOS_ADC_TEMP_SENSOR_ADC		ADC1
#define PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL	1

#define PIOS_ADC_NUM_PINS			0

#define PIOS_ADC_PORTS				{ }
#define PIOS_ADC_PINS				{ }
#define PIOS_ADC_CHANNELS			{ }
#define PIOS_ADC_MAPPING			{ }
#define PIOS_ADC_CHANNEL_MAPPING		{ }

#define PIOS_ADC_NUM_ADC_CHANNELS		0
#define PIOS_ADC_USE_ADC2			0

#endif

#define PIOS_ADC_NUM_CHANNELS			(PIOS_ADC_NUM_PINS + PIOS_ADC_USE_TEMP_SENSOR)
#define PIOS_ADC_CLOCK_FUNCTION			RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE)
#define PIOS_ADC_ADCCLK				RCC_PCLK2_Div8
/* RCC_PCLK2_Div2: ADC clock = PCLK2/2 */
/* RCC_PCLK2_Div4: ADC clock = PCLK2/4 */
/* RCC_PCLK2_Div6: ADC clock = PCLK2/6 */
/* RCC_PCLK2_Div8: ADC clock = PCLK2/8 */
#define PIOS_ADC_SAMPLE_TIME			ADC_SampleTime_239Cycles5
/* Sample time: */
/* With an ADCCLK = 14 MHz and a sampling time of 239.5 cycles: */
/* Tconv = 239.5 + 12.5 = 252 cycles = 18�s */
/* (1 / (ADCCLK / CYCLES)) = Sample Time (�S) */
#define PIOS_ADC_IRQ_PRIO			PIOS_IRQ_PRIO_LOW

// Currently analog acquistion hard coded at 480 Hz
// PCKL2 = HCLK / 16
// ADCCLK = PCLK2 / 2
#define PIOS_ADC_RATE		(72.0e6 / 1.0 / 8.0 / 252.0 / (PIOS_ADC_NUM_CHANNELS >> PIOS_ADC_USE_ADC2))
#define PIOS_ADC_MAX_OVERSAMPLING               36

//------------------------
// PIOS_RCVR
// See also pios_board.c
//------------------------
#define PIOS_RCVR_MAX_DEVS                      3
#define PIOS_RCVR_MAX_CHANNELS			12

//-------------------------
// Receiver PPM input
//-------------------------
#define PIOS_PPM_MAX_DEVS			1
#define PIOS_PPM_NUM_INPUTS                     12

//-------------------------
// Receiver PWM input
//-------------------------
#define PIOS_PWM_MAX_DEVS			1
#define PIOS_PWM_NUM_INPUTS                     1

//-------------------------
// Servo outputs
//-------------------------
#define PIOS_SERVO_UPDATE_HZ                    50
#define PIOS_SERVOS_INITIAL_POSITION            0 /* dont want to start motors, have no pulse till settings loaded */

//--------------------------
// Timer controller settings
//--------------------------
#define PIOS_TIM_MAX_DEVS			3

//-------------------------
// GPIO
//-------------------------
#define PIOS_GPIO_PORTS				{ }
#define PIOS_GPIO_PINS				{ }
#define PIOS_GPIO_CLKS				{ }
#define PIOS_GPIO_NUM				0

//-------------------------
// USB
//-------------------------
#define PIOS_USB_HID_MAX_DEVS                   1

#define PIOS_USB_ENABLED                        1
#define PIOS_USB_HID_MAX_DEVS                   1
#define PIOS_USB_MAX_DEVS                       1
#ifdef MOVE_CONTROLLER
#define PIOS_USB_DETECT_GPIO_PORT               GPIOD
#define PIOS_USB_DETECT_GPIO_PIN                GPIO_Pin_7
#define PIOS_USB_DETECT_EXTI_LINE               EXTI_Line4
#else
#define PIOS_USB_DETECT_GPIO_PORT               GPIOC
#define PIOS_USB_DETECT_GPIO_PIN                GPIO_Pin_15
#define PIOS_USB_DETECT_EXTI_LINE               EXTI_Line15
#endif

#endif /* STM32103CB_AHRS_H_ */
