/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 * @file       pios_board.h
 * @author     David "Buzz" Carlson (buzz@chebuzz.com)
 * 				The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
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


#ifndef STM3210E_INS_H_
#define STM3210E_INS_H_



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
/* Channel 1  -                                 */
/* Channel 2  - SPI1 RX                         */
/* Channel 3  - SPI1 TX                         */
/* Channel 4  - SPI2 RX                         */
/* Channel 5  - SPI2 TX                         */
/* Channel 6  -                                 */
/* Channel 7  -                                 */
/* Channel 8  -                                 */
/* Channel 9  -                                 */
/* Channel 10 -                                 */
/* Channel 11 -                                 */
/* Channel 12 -                                 */

//------------------------
// BOOTLOADER_SETTINGS
//------------------------

//#define FUNC_ID				1
//#define HW_VERSION			01

#define BOOTLOADER_VERSION	0
#define BOARD_TYPE			0x05  // INS board
#define BOARD_REVISION		0x01  // Beta version
//#define HW_VERSION	(BOARD_TYPE << 8) | BOARD_REVISION

#define MEM_SIZE			524288 //512K
#define SIZE_OF_DESCRIPTION	(uint8_t) 100
#define START_OF_USER_CODE	(uint32_t)0x08005000//REMEMBER SET ALSO IN link_stm32f10x_HD_BL.ld
#define SIZE_OF_CODE		(uint32_t) (MEM_SIZE-(START_OF_USER_CODE-0x08000000)-SIZE_OF_DESCRIPTION)

#ifdef STM32F10X_HD
		#define HW_TYPE			0 //0=high_density 1=medium_density;
#elif STM32F10X_MD
		#define HW_TYPE			1 //0=high_density 1=medium_density;
#endif
#define BOARD_READABLE	TRUE
#define BOARD_WRITABLA	TRUE
#define MAX_DEL_RETRYS	3

//------------------------
// PIOS_LED
//------------------------
#define PIOS_LED_LED1_GPIO_PORT                 GPIOA
#define PIOS_LED_LED1_GPIO_PIN                  GPIO_Pin_3
#define PIOS_LED_LED1_GPIO_CLK                  RCC_APB2Periph_GPIOA
#define PIOS_LED_LED2_GPIO_PORT                 GPIOA
#define PIOS_LED_LED2_GPIO_PIN                  GPIO_Pin_2
#define PIOS_LED_LED2_GPIO_CLK                  RCC_APB2Periph_GPIOA
#define PIOS_LED_NUM                            2
#define PIOS_LED_PORTS                          { PIOS_LED_LED1_GPIO_PORT, PIOS_LED_LED2_GPIO_PORT }
#define PIOS_LED_PINS                           { PIOS_LED_LED1_GPIO_PIN, PIOS_LED_LED2_GPIO_PIN }
#define PIOS_LED_CLKS                           { PIOS_LED_LED1_GPIO_CLK, PIOS_LED_LED2_GPIO_CLK }

//------------------------
// PIOS_SPI
// See also pios_board.c
//------------------------
#define PIOS_SPI_MAX_DEVS                       2

//------------------------
// PIOS_I2C
// See also pios_board.c
//------------------------
#define PIOS_I2C_MAX_DEVS			3
extern uint32_t pios_i2c_pres_mag_adapter_id;
#define PIOS_I2C_MAIN_ADAPTER			(pios_i2c_pres_mag_adapter_id)
extern uint32_t pios_i2c_gyro_adapter_id;
#define PIOS_I2C_GYRO_ADAPTER			(pios_i2c_gyro_adapter_id)

//------------------------
// PIOS_BMP085
//------------------------
#define PIOS_BMP085_EOC_GPIO_PORT               GPIOC
#define PIOS_BMP085_EOC_GPIO_PIN                GPIO_Pin_2
#define PIOS_BMP085_EOC_PORT_SOURCE             GPIO_PortSourceGPIOC
#define PIOS_BMP085_EOC_PIN_SOURCE              GPIO_PinSource2
#define PIOS_BMP085_EOC_CLK                     RCC_APB2Periph_GPIOC
#define PIOS_BMP085_EOC_EXTI_LINE               EXTI_Line2
#define PIOS_BMP085_EOC_IRQn                    EXTI15_10_IRQn
#define PIOS_BMP085_EOC_PRIO                    PIOS_IRQ_PRIO_LOW
#define PIOS_BMP085_XCLR_GPIO_PORT              GPIOC
#define PIOS_BMP085_XCLR_GPIO_PIN               GPIO_Pin_1
#define PIOS_BMP085_OVERSAMPLING                3

//-------------------------
// PIOS_USART
//
// See also pios_board.c
//-------------------------
#define PIOS_USART_MAX_DEVS             2

#define PIOS_USART_RX_BUFFER_SIZE       256
#define PIOS_USART_TX_BUFFER_SIZE       256

//-------------------------
// PIOS_COM
//
// See also pios_board.c
//-------------------------
#define PIOS_COM_MAX_DEVS               2

#define PIOS_COM_GPS_BAUDRATE           57600
extern uint32_t pios_com_gps_id;
#define PIOS_COM_GPS                    (pios_com_gps_id)

#ifdef PIOS_ENABLE_AUX_UART
#define PIOS_COM_AUX_BAUDRATE           57600
extern uint32_t pios_com_aux_id;
#define PIOS_COM_AUX                    (pios_com_aux_id)
#define PIOS_COM_DEBUG                  PIOS_COM_AUX
#endif

//-------------------------
// Delay Timer
//-------------------------
#define PIOS_DELAY_TIMER                        TIM2
#define PIOS_DELAY_TIMER_RCC_FUNC               RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE)

//-------------------------
// System Settings
//-------------------------
#define PIOS_MASTER_CLOCK                       72000000
#define PIOS_PERIPHERAL_CLOCK                   (PIOS_MASTER_CLOCK / 2)
#if defined(USE_BOOTLOADER)
#define PIOS_NVIC_VECTTAB_FLASH                 (START_OF_USER_CODE)
#else
#define PIOS_NVIC_VECTTAB_FLASH                 ((uint32_t)0x08000000)
#endif

//-------------------------
// Interrupt Priorities
//-------------------------
#define PIOS_IRQ_PRIO_LOW                       12              // lower than RTOS
#define PIOS_IRQ_PRIO_MID                       8               // higher than RTOS
#define PIOS_IRQ_PRIO_HIGH                      5               // for SPI, ADC, I2C etc...
#define PIOS_IRQ_PRIO_HIGHEST                   4               // for USART etc...

//-------------------------
// ADC
// None.
//-------------------------


//-------------------------
// GPIO
// Not used, but pios_gpio.c expects something
//-------------------------
#define PIOS_GPIO_1_PORT			GPIOA
#define PIOS_GPIO_1_PIN				GPIO_Pin_1
#define PIOS_GPIO_1_GPIO_CLK		RCC_APB2Periph_GPIOA

#define PIOS_GPIO_PORTS                         { PIOS_GPIO_1_PORT }
#define PIOS_GPIO_PINS                          { PIOS_GPIO_1_PIN }
#define PIOS_GPIO_CLKS                          { PIOS_GPIO_1_GPIO_CLK }
#define PIOS_GPIO_NUM                           1

#define PIOS_BMA_ENABLE                        PIOS_SPI_RC_PinSet(PIOS_SPI_ACCEL,0)
#define PIOS_BMA_DISABLE                       PIOS_SPI_RC_PinSet(PIOS_SPI_ACCEL,1)

//------------------------
// PIOS_HMC5883
//------------------------
#define PIOS_HMC5883_DRDY_GPIO_PORT		GPIOB
#define PIOS_HMC5883_DRDY_GPIO_PIN		GPIO_Pin_8
#define PIOS_HMC5883_DRDY_PORT_SOURCE		GPIO_PortSourceGPIOB
#define PIOS_HMC5883_DRDY_PIN_SOURCE		GPIO_PinSource8
#define PIOS_HMC5883_DRDY_CLK			RCC_APB2Periph_GPIOB
#define PIOS_HMC5883_DRDY_EXTI_LINE		EXTI_Line8
#define PIOS_HMC5883_DRDY_IRQn			EXTI9_5_IRQn
#define PIOS_HMC5883_DRDY_PRIO			PIOS_IRQ_PRIO_LOW

//------------------------
// PIOS_IMU3000
//------------------------
#define PIOS_IMU3000_INT_GPIO_PORT		GPIOB
#define PIOS_IMU3000_INT_GPIO_PIN		GPIO_Pin_1
#define PIOS_IMU3000_INT_PORT_SOURCE		GPIO_PortSourceGPIOB
#define PIOS_IMU3000_INT_PIN_SOURCE		GPIO_PinSource1
#define PIOS_IMU3000_INT_CLK			RCC_APB2Periph_GPIOB
#define PIOS_IMU3000_INT_EXTI_LINE		EXTI_Line1
#define PIOS_IMU3000_INT_IRQn			EXTI1_IRQn
#define PIOS_IMU3000_INT_PRIO			PIOS_IRQ_PRIO_HIGH




#endif /* STM3210E_INS_H_ */
/**
 * @}
 * @}
 */
