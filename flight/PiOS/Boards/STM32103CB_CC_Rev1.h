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


#ifndef STM32103CB_CC_H_
#define STM32103CB_CC_H_

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
//#define FUNC_ID				2
//#define HW_VERSION			69

#define BOOTLOADER_VERSION	0
#define BOARD_TYPE		0x04
#define BOARD_REVISION		0x01
//#define HW_VERSION	(BOARD_TYPE << 8) | BOARD_REVISION

#define MEM_SIZE			0x20000 //128K
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
#define PIOS_LED_LED1_GPIO_PORT			GPIOA
#define PIOS_LED_LED1_GPIO_PIN			GPIO_Pin_6
#define PIOS_LED_LED1_GPIO_CLK			RCC_APB2Periph_GPIOA
#define PIOS_LED_NUM				1
#define PIOS_LED_PORTS				{ PIOS_LED_LED1_GPIO_PORT }
#define PIOS_LED_PINS				{ PIOS_LED_LED1_GPIO_PIN }
#define PIOS_LED_CLKS				{ PIOS_LED_LED1_GPIO_CLK }

//-------------------------
// Delay Timer
//-------------------------
#define PIOS_DELAY_TIMER			TIM3
#define PIOS_DELAY_TIMER_RCC_FUNC		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE)

//-------------------------
// System Settings
//-------------------------
#define PIOS_MASTER_CLOCK			72000000
#define PIOS_PERIPHERAL_CLOCK			(PIOS_MASTER_CLOCK / 2)
#if defined(USE_BOOTLOADER)
#define PIOS_NVIC_VECTTAB_FLASH			(START_OF_USER_CODE)
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

//------------------------
// PIOS_I2C
// See also pios_board.c
//------------------------
#define PIOS_I2C_MAX_DEVS			1
extern uint32_t pios_i2c_main_adapter_id;
#define PIOS_I2C_MAIN_ADAPTER			(pios_i2c_main_adapter_id)

//-------------------------
// SPI
//
// See also pios_board.c
//-------------------------
#define PIOS_SPI_MAX_DEVS			2

//-------------------------
// PIOS_USART
//-------------------------
#define PIOS_USART_MAX_DEVS			2

#define PIOS_USART_RX_BUFFER_SIZE               256
#define PIOS_USART_TX_BUFFER_SIZE               256

//-------------------------
// PIOS_COM
//
// See also pios_board.c
//-------------------------
#define PIOS_COM_MAX_DEVS			4

#define PIOS_COM_TELEM_BAUDRATE         57600
extern uint32_t pios_com_telem_rf_id;
#define PIOS_COM_TELEM_RF               (pios_com_telem_rf_id)
#define PIOS_COM_DEBUG                  PIOS_COM_TELEM_RF

#if defined(PIOS_INCLUDE_GPS)
#define PIOS_COM_GPS_BAUDRATE           57600
extern uint32_t pios_com_gps_id;
#define PIOS_COM_GPS                    (pios_com_gps_id)
#endif	/* PIOS_INCLUDE_GPS */

extern uint32_t pios_com_telem_usb_id;
#define PIOS_COM_TELEM_USB              (pios_com_telem_usb_id)

#ifdef PIOS_INCLUDE_SPEKTRUM
#define PIOS_COM_SPEKTRUM_BAUDRATE      115200
extern uint32_t pios_com_spektrum_id;
#define PIOS_COM_SPEKTRUM               (pios_com_spektrum_id)
#endif

//-------------------------
// ADC
// PIOS_ADC_PinGet(0) = Gyro Z
// PIOS_ADC_PinGet(1) = Gyro Y
// PIOS_ADC_PinGet(2) = Gyro X
//-------------------------
//#define PIOS_ADC_OVERSAMPLING_RATE		1
#define PIOS_ADC_USE_TEMP_SENSOR		1
#define PIOS_ADC_TEMP_SENSOR_ADC		ADC1
#define PIOS_ADC_TEMP_SENSOR_ADC_CHANNEL	1

#define PIOS_ADC_PIN1_GPIO_PORT			GPIOA			// PA4 (Gyro X)
#define PIOS_ADC_PIN1_GPIO_PIN			GPIO_Pin_4		// ADC12_IN4
#define PIOS_ADC_PIN1_GPIO_CHANNEL		ADC_Channel_4
#define PIOS_ADC_PIN1_ADC			ADC2
#define PIOS_ADC_PIN1_ADC_NUMBER		1

#define PIOS_ADC_PIN2_GPIO_PORT			GPIOA			// PA5 (Gyro Y)
#define PIOS_ADC_PIN2_GPIO_PIN			GPIO_Pin_5		// ADC123_IN5
#define PIOS_ADC_PIN2_GPIO_CHANNEL		ADC_Channel_5
#define PIOS_ADC_PIN2_ADC			ADC1
#define PIOS_ADC_PIN2_ADC_NUMBER		2

#define PIOS_ADC_PIN3_GPIO_PORT			GPIOA			// PA3 (Gyro Z)
#define PIOS_ADC_PIN3_GPIO_PIN			GPIO_Pin_3		// ADC12_IN3
#define PIOS_ADC_PIN3_GPIO_CHANNEL		ADC_Channel_3
#define PIOS_ADC_PIN3_ADC			ADC2
#define PIOS_ADC_PIN3_ADC_NUMBER		2

#define PIOS_ADC_NUM_PINS			3

#define PIOS_ADC_PORTS				{ PIOS_ADC_PIN1_GPIO_PORT, PIOS_ADC_PIN2_GPIO_PORT, PIOS_ADC_PIN3_GPIO_PORT }
#define PIOS_ADC_PINS				{ PIOS_ADC_PIN1_GPIO_PIN, PIOS_ADC_PIN2_GPIO_PIN, PIOS_ADC_PIN3_GPIO_PIN }
#define PIOS_ADC_CHANNELS			{ PIOS_ADC_PIN1_GPIO_CHANNEL, PIOS_ADC_PIN2_GPIO_CHANNEL, PIOS_ADC_PIN3_GPIO_CHANNEL }
#define PIOS_ADC_MAPPING			{ PIOS_ADC_PIN1_ADC, PIOS_ADC_PIN2_ADC, PIOS_ADC_PIN3_ADC }
#define PIOS_ADC_CHANNEL_MAPPING		{ PIOS_ADC_PIN1_ADC_NUMBER, PIOS_ADC_PIN2_ADC_NUMBER, PIOS_ADC_PIN3_ADC_NUMBER }
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
/* With an ADCCLK = 14 MHz and a sampling time of 239.5 cycles: */
/* Tconv = 239.5 + 12.5 = 252 cycles = 18�s */
/* (1 / (ADCCLK / CYCLES)) = Sample Time (�S) */
#define PIOS_ADC_IRQ_PRIO			PIOS_IRQ_PRIO_LOW

// Currently analog acquistion hard coded at 480 Hz
// PCKL2 = HCLK / 16
// ADCCLK = PCLK2 / 2
#define PIOS_ADC_RATE		(72.0e6 / 1.0 / 8.0 / 252.0 / (PIOS_ADC_NUM_CHANNELS >> PIOS_ADC_USE_ADC2))
#define PIOS_ADC_MAX_OVERSAMPLING               36

//-------------------------
// Receiver PWM inputs
//-------------------------
#define PIOS_PWM_MAX_INPUTS                     6

//-------------------------
// Servo outputs
//-------------------------
#define PIOS_SERVO_UPDATE_HZ                    50
#define PIOS_SERVOS_INITIAL_POSITION            0 /* dont want to start motors, have no pulse till settings loaded */

//-------------------------
// GPIO
//-------------------------
#define PIOS_GPIO_1_PORT			GPIOA
#define PIOS_GPIO_1_PIN				GPIO_Pin_7
#define PIOS_GPIO_1_GPIO_CLK			RCC_APB2Periph_GPIOA

#define PIOS_GPIO_PORTS				{ PIOS_GPIO_1_PORT }
#define PIOS_GPIO_PINS				{ PIOS_GPIO_1_PIN }
#define PIOS_GPIO_CLKS				{ PIOS_GPIO_1_GPIO_CLK }
#define PIOS_GPIO_NUM				1

#define PIOS_FLASH_CS_PIN                       0
#define PIOS_FLASH_ENABLE                       PIOS_GPIO_On(0)
#define PIOS_FLASH_DISABLE                      PIOS_GPIO_Off(0)
#define PIOS_ADXL_ENABLE                        PIOS_SPI_RC_PinSet(PIOS_SPI_ACCEL,0)
#define PIOS_ADXL_DISABLE                       PIOS_SPI_RC_PinSet(PIOS_SPI_ACCEL,1)

//-------------------------
// USB
//-------------------------
#define PIOS_USB_ENABLED                        1
#define PIOS_USB_DETECT_GPIO_PORT               GPIOC
#define PIOS_USB_DETECT_GPIO_PIN                GPIO_Pin_15
#define PIOS_USB_DETECT_EXTI_LINE               EXTI_Line15
#define PIOS_IRQ_USB_PRIORITY                   PIOS_IRQ_PRIO_MID
#define PIOS_USB_RX_BUFFER_SIZE                 128
#define PIOS_USB_TX_BUFFER_SIZE                 128
#endif /* STM32103CB_AHRS_H_ */
