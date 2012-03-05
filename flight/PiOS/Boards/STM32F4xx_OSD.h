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
Timer | Channel 1  | Channel 2  | Channel 3  | Channel 4
------+------------+------------+------------+------------
TIM1  |                       DELAY                      |
TIM2  |                         | PPM Output | PPM Input |
TIM3  |                  TIMER INTERRUPT                 |
TIM4  |                     STOPWATCH                    |
------+------------+------------+------------+------------
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

//-------------------------
// System Settings
//
// See also System_stm32f4xx.c
//-------------------------
//These macros are deprecated
//please use PIOS_PERIPHERAL_APBx_CLOCK According to the table below
//#define PIOS_MASTER_CLOCK
//#define PIOS_PERIPHERAL_CLOCK
//#define PIOS_PERIPHERAL_CLOCK

#define PIOS_SYSCLK										108000000
//	Peripherals that belongs to APB1 are:
//	DAC			|PWR				|CAN1,2
//	I2C1,2,3		|UART4,5			|USART3,2
//	I2S3Ext		|SPI3/I2S3		|SPI2/I2S2
//	I2S2Ext		|IWDG				|WWDG
//	RTC/BKP reg
// TIM2,3,4,5,6,7,12,13,14

// Calculated as SYSCLK / APBPresc * (APBPre == 1 ? 1 : 2)
// Default APB1 Prescaler = 4
#define PIOS_PERIPHERAL_APB1_CLOCK					(PIOS_SYSCLK / 2)

//	Peripherals belonging to APB2
//	SDIO			|EXTI				|SYSCFG			|SPI1
//	ADC1,2,3
//	USART1,6
//	TIM1,8,9,10,11
//
// Default APB2 Prescaler = 2
//
#define PIOS_PERIPHERAL_APB2_CLOCK					PIOS_SYSCLK


//------------------------
// BOOTLOADER_SETTINGS
//------------------------
#define BOARD_READABLE	TRUE
#define BOARD_WRITABLE	TRUE
#define MAX_DEL_RETRYS	3

//------------------------
// TELEMETRY
//------------------------
#define TELEM_QUEUE_SIZE         20
#define PIOS_TELEM_STACK_SIZE    624

// *****************************************************************
// System Settings

#define PIOS_MASTER_CLOCK                       108000000ul
#define PIOS_PERIPHERAL_CLOCK                   (PIOS_MASTER_CLOCK / 2)

// *****************************************************************
// Interrupt Priorities

#define PIOS_IRQ_PRIO_LOW			12		// lower than RTOS
#define PIOS_IRQ_PRIO_MID			8		// higher than RTOS
#define PIOS_IRQ_PRIO_HIGH			5		// for SPI, ADC, I2C etc...
#define PIOS_IRQ_PRIO_HIGHEST		4 		// for USART etc...


//------------------------
// WATCHDOG_SETTINGS
//------------------------
#define PIOS_WATCHDOG_TIMEOUT    250
#define PIOS_WDG_REGISTER        RTC_BKP_DR4
#define PIOS_WDG_ACTUATOR        0x0001
#define PIOS_WDG_STABILIZATION   0x0002
#define PIOS_WDG_ATTITUDE        0x0004
#define PIOS_WDG_MANUAL          0x0008


// *****************************************************************
// PIOS_LED
#define PIOS_LED_HEARTBEAT	0
#define PIOS_LED_ALARM		1

#if 0
#define PIOS_LED_LED1_GPIO_PORT					GPIOD
#define PIOS_LED_LED1_GPIO_PIN					GPIO_Pin_13 //LD3
#define PIOS_LED_LED1_GPIO_CLK					RCC_APB2Periph_GPIOD
#define PIOS_LED_LED2_GPIO_PORT                 GPIOD
#define PIOS_LED_LED2_GPIO_PIN                  GPIO_Pin_12 //LD4
#define PIOS_LED_LED2_GPIO_CLK                  RCC_APB2Periph_GPIOD
#define PIOS_LED_LED3_GPIO_PORT                 GPIOD
#define PIOS_LED_LED3_GPIO_PIN                  GPIO_Pin_14 //LD5
#define PIOS_LED_LED3_GPIO_CLK                  RCC_APB2Periph_GPIOD
#define PIOS_LED_LED4_GPIO_PORT                 GPIOD
#define PIOS_LED_LED4_GPIO_PIN                  GPIO_Pin_15 //LD6
#define PIOS_LED_LED4_GPIO_CLK                  RCC_APB2Periph_GPIOD
#define PIOS_LED_NUM                            4
#define PIOS_LED_PORTS                          { PIOS_LED_LED1_GPIO_PORT, PIOS_LED_LED2_GPIO_PORT, PIOS_LED_LED3_GPIO_PORT, PIOS_LED_LED4_GPIO_PORT }
#define PIOS_LED_PINS                           { PIOS_LED_LED1_GPIO_PIN, PIOS_LED_LED2_GPIO_PIN, PIOS_LED_LED3_GPIO_PIN, PIOS_LED_LED4_GPIO_PIN }
#define PIOS_LED_CLKS                           { PIOS_LED_LED1_GPIO_CLK, PIOS_LED_LED2_GPIO_CLK, PIOS_LED_LED3_GPIO_CLK, PIOS_LED_LED4_GPIO_CLK }
#endif

/*#define USB_LED_ON						PIOS_LED_On(LED1)
#define USB_LED_OFF						PIOS_LED_Off(LED1)
#define USB_LED_TOGGLE					PIOS_LED_Toggle(LED1)
*/

// *****************************************************************
// Delay Timer

//#define PIOS_DELAY_TIMER				TIM2
//#define PIOS_DELAY_TIMER_RCC_FUNC		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE)
#define PIOS_DELAY_TIMER				TIM1
#define PIOS_DELAY_TIMER_RCC_FUNC		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE)

// *****************************************************************
// Timer interrupt

/*#define TIMER_INT_TIMER					TIM3
#define TIMER_INT_FUNC					TIM3_IRQHandler
#define TIMER_INT_PRIORITY				2

// *****************************************************************
// Stop watch timer

#define STOPWATCH_TIMER					TIM4*/

//------------------------
// PIOS_SPI
// See also pios_board.c
//------------------------
#define PIOS_SPI_MAX_DEVS               1
extern uint32_t pios_spi_port_id;
#define PIOS_SPI_PORT                   (pios_spi_port_id)

//-------------------------
// PIOS_USART
//
// See also pios_board.c
//-------------------------
#define PIOS_USART_MAX_DEVS             5

//-------------------------
// PIOS_COM
//
// See also pios_board.c
//-------------------------
#define PIOS_COM_MAX_DEVS               5
extern uint32_t pios_com_telem_rf_id;
extern uint32_t pios_com_gps_id;
extern uint32_t pios_com_aux_id;
extern uint32_t pios_com_telem_usb_id;
extern uint32_t pios_com_cotelem_id;
#define PIOS_COM_AUX                    (pios_com_aux_id)
#define PIOS_COM_GPS                    (pios_com_gps_id)
#define PIOS_COM_TELEM_USB              (pios_com_telem_usb_id)
#define PIOS_COM_TELEM_RF               (pios_com_telem_rf_id)
#define PIOS_COM_COTELEM               (pios_com_cotelem_id)
#define PIOS_COM_DEBUG                  PIOS_COM_AUX


extern uint32_t pios_com_hkosd_id;
#define PIOS_COM_OSD                 (pios_com_hkosd_id)

//extern uint32_t pios_com_serial_id;
//#define PIOS_COM_SERIAL                 (pios_com_serial_id)
//#define PIOS_COM_DEBUG                  PIOS_COM_SERIAL           // uncomment this to send debug info out the serial port

//extern uint32_t pios_com_gps_id;
//#define PIOS_COM_GPS                    (pios_com_gps_id)


#if defined(PIOS_INCLUDE_USB_HID)
extern uint32_t pios_com_telem_usb_id;
#define PIOS_COM_TELEM_USB              (pios_com_telem_usb_id)
#endif

#if defined(PIOS_COM_DEBUG)
//  #define DEBUG_PRINTF(...) PIOS_COM_SendFormattedString(PIOS_COM_DEBUG, __VA_ARGS__)
  #define DEBUG_PRINTF(...) PIOS_COM_SendFormattedStringNonBlocking(PIOS_COM_DEBUG, __VA_ARGS__)
#else
  #define DEBUG_PRINTF(...)
#endif

// *****************************************************************
// ADC

//-------------------------
// ADC
// PIOS_ADC_PinGet(0) = External voltage
// PIOS_ADC_PinGet(1) = AUX1 (PX2IO external pressure port)
// PIOS_ADC_PinGet(2) = AUX2 (Current sensor, if available)
// PIOS_ADC_PinGet(3) = AUX3
// PIOS_ADC_PinGet(4) = VREF
// PIOS_ADC_PinGet(5) = Temperature sensor
//-------------------------

#define PIOS_DMA_PIN_CONFIG \
{ \
{GPIOC, GPIO_Pin_0, ADC_Channel_10}, \
{GPIOC, GPIO_Pin_1, ADC_Channel_11}, \
{GPIOC, GPIO_Pin_2, ADC_Channel_12}, \
{GPIOC, GPIO_Pin_3, ADC_Channel_13}, \
{GPIOA, GPIO_Pin_7, ADC_Channel_7}, \
{NULL, 0, ADC_Channel_Vrefint}, /* Voltage reference */\
{NULL, 0, ADC_Channel_TempSensor} /* Temperature sensor */\
}

/* we have to do all this to satisfy the PIOS_ADC_MAX_SAMPLES define in pios_adc.h */
/* which is annoying because this then determines the rate at which we generate buffer turnover events */
/* the objective here is to get enough buffer space to support 100Hz averaging rate */
#define PIOS_ADC_NUM_CHANNELS 7
#define PIOS_ADC_MAX_OVERSAMPLING 10
#define PIOS_ADC_USE_ADC2 0

#if 0

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

// PPM OUT line
#define PIOS_GPIO_OUT_2_PORT		GPIOB
#define PIOS_GPIO_OUT_2_PIN			GPIO_Pin_10
#define PIOS_GPIO_OUT_2_GPIO_CLK	RCC_APB2Periph_GPIOB

// spare pin
#define PIOS_GPIO_OUT_3_PORT		GPIOA
#define PIOS_GPIO_OUT_3_PIN			GPIO_Pin_0
#define PIOS_GPIO_OUT_3_GPIO_CLK	RCC_APB2Periph_GPIOA

// spare pin
#define PIOS_GPIO_OUT_4_PORT		GPIOA
#define PIOS_GPIO_OUT_4_PIN			GPIO_Pin_1
#define PIOS_GPIO_OUT_4_GPIO_CLK	RCC_APB2Periph_GPIOA

// spare pin
#define PIOS_GPIO_OUT_5_PORT		GPIOC
#define PIOS_GPIO_OUT_5_PIN			GPIO_Pin_13
#define PIOS_GPIO_OUT_5_GPIO_CLK	RCC_APB2Periph_GPIOC

// spare pin
#define PIOS_GPIO_OUT_6_PORT		GPIOC
#define PIOS_GPIO_OUT_6_PIN			GPIO_Pin_14
#define PIOS_GPIO_OUT_6_GPIO_CLK	RCC_APB2Periph_GPIOC

// spare pin
#define PIOS_GPIO_OUT_7_PORT		GPIOC
#define PIOS_GPIO_OUT_7_PIN			GPIO_Pin_15
#define PIOS_GPIO_OUT_7_GPIO_CLK	RCC_APB2Periph_GPIOC

#define PIOS_GPIO_NUM				8
#define PIOS_GPIO_PORTS				{PIOS_GPIO_OUT_0_PORT,     PIOS_GPIO_OUT_1_PORT,     PIOS_GPIO_OUT_2_PORT,     PIOS_GPIO_OUT_3_PORT,     PIOS_GPIO_OUT_4_PORT,     PIOS_GPIO_OUT_5_PORT,     PIOS_GPIO_OUT_6_PORT,     PIOS_GPIO_OUT_7_PORT}
#define PIOS_GPIO_PINS				{PIOS_GPIO_OUT_0_PIN,      PIOS_GPIO_OUT_1_PIN,      PIOS_GPIO_OUT_2_PIN,      PIOS_GPIO_OUT_3_PIN,      PIOS_GPIO_OUT_4_PIN,      PIOS_GPIO_OUT_5_PIN,      PIOS_GPIO_OUT_6_PIN,      PIOS_GPIO_OUT_7_PIN}
#define PIOS_GPIO_CLKS				{PIOS_GPIO_OUT_0_GPIO_CLK, PIOS_GPIO_OUT_1_GPIO_CLK, PIOS_GPIO_OUT_2_GPIO_CLK, PIOS_GPIO_OUT_3_GPIO_CLK, PIOS_GPIO_OUT_4_GPIO_CLK, PIOS_GPIO_OUT_5_GPIO_CLK, PIOS_GPIO_OUT_6_GPIO_CLK, PIOS_GPIO_OUT_7_GPIO_CLK}

#define SERIAL_RTS_ENABLE			PIOS_GPIO_Enable(0)
#define SERIAL_RTS_SET				PIOS_GPIO_Off(0)
#define SERIAL_RTS_CLEAR			PIOS_GPIO_On(0)

#define RF_CS_ENABLE				PIOS_GPIO_Enable(1)
#define RF_CS_HIGH					PIOS_GPIO_Off(1)
#define RF_CS_LOW					PIOS_GPIO_On(1)

#define PPM_OUT_PIN                 PIOS_GPIO_OUT_2_PIN
#define PPM_OUT_PORT                PIOS_GPIO_OUT_2_PORT
#define PPM_OUT_ENABLE				PIOS_GPIO_Enable(2)
#define PPM_OUT_HIGH				PIOS_GPIO_Off(2)
#define PPM_OUT_LOW					PIOS_GPIO_On(2)

#define SPARE1_ENABLE				PIOS_GPIO_Enable(3)
#define SPARE1_HIGH					PIOS_GPIO_Off(3)
#define SPARE1_LOW					PIOS_GPIO_On(3)

#define SPARE2_ENABLE				PIOS_GPIO_Enable(4)
#define SPARE2_HIGH					PIOS_GPIO_Off(4)
#define SPARE2_LOW					PIOS_GPIO_On(4)

#define SPARE3_ENABLE				PIOS_GPIO_Enable(5)
#define SPARE3_HIGH					PIOS_GPIO_Off(5)
#define SPARE3_LOW					PIOS_GPIO_On(5)

#define SPARE4_ENABLE				PIOS_GPIO_Enable(6)
#define SPARE4_HIGH					PIOS_GPIO_Off(6)
#define SPARE4_LOW					PIOS_GPIO_On(6)

#define SPARE5_ENABLE				PIOS_GPIO_Enable(7)
#define SPARE5_HIGH					PIOS_GPIO_Off(7)
#define SPARE5_LOW					PIOS_GPIO_On(7)

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

// PPM IN line
#define PPM_IN_PORT			    GPIOB
#define PPM_IN_PIN			    GPIO_Pin_11
#define PPM_IN_MODE	      		GPIO_Mode_IPD

#define GPIO_IN_NUM				8
#define GPIO_IN_PORTS			{ GPIO_IN_0_PORT, GPIO_IN_1_PORT, GPIO_IN_2_PORT, GPIO_IN_3_PORT, GPIO_IN_4_PORT, GPIO_IN_5_PORT, GPIO_IN_6_PORT, PPM_IN_PORT }
#define GPIO_IN_PINS			{ GPIO_IN_0_PIN,  GPIO_IN_1_PIN,  GPIO_IN_2_PIN,  GPIO_IN_3_PIN,  GPIO_IN_4_PIN,  GPIO_IN_5_PIN,  GPIO_IN_6_PIN,  PPM_IN_PIN  }
#define GPIO_IN_MODES			{ GPIO_IN_0_MODE, GPIO_IN_1_MODE, GPIO_IN_2_MODE, GPIO_IN_3_MODE, GPIO_IN_4_MODE, GPIO_IN_5_MODE, GPIO_IN_6_MODE, PPM_IN_MODE }

#define API_MODE_PIN			0
#define SERIAL_CTS_PIN			1
#define VBUS_SENSE_PIN			2
#define _868MHz_PIN				3
#define _915MHz_PIN				4
#define RF_INT_PIN				5
#define RF_MISC_PIN				6

#endif

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


#if 0
// *****************************************************************
// VIDEO
#define PIOS_VIDEO_HSYNC_GPIO_PORT		GPIOD
#define PIOS_VIDEO_HSYNC_GPIO_PIN		GPIO_Pin_0
#define PIOS_VIDEO_HSYNC_PORT_SOURCE		GPIO_PortSourceGPIOD
#define PIOS_VIDEO_HSYNC_PIN_SOURCE		GPIO_PinSource0
#define PIOS_VIDEO_HSYNC_CLK				RCC_AHB1Periph_GPIOD
#define PIOS_VIDEO_HSYNC_EXTI_LINE		EXTI_Line0
#define PIOS_VIDEO_HSYNC_EXTI_PORT_SOURCE		EXTI_PortSourceGPIOD
#define PIOS_VIDEO_HSYNC_EXTI_PIN_SOURCE		EXTI_PinSource0
#define PIOS_VIDEO_HSYNC_IRQn			EXTI0_IRQn
#define PIOS_VIDEO_HSYNC_PRIO			PIOS_IRQ_PRIO_HIGHEST
//#define PIOS_VIDEO_SYNC_PRIO			1

#define PIOS_VIDEO_VSYNC_GPIO_PORT		GPIOC
#define PIOS_VIDEO_VSYNC_GPIO_PIN		GPIO_Pin_11
#define PIOS_VIDEO_VSYNC_PORT_SOURCE		GPIO_PortSourceGPIOC
#define PIOS_VIDEO_VSYNC_PIN_SOURCE		GPIO_PinSource11
#define PIOS_VIDEO_VSYNC_CLK				RCC_AHB1Periph_GPIOC
#define PIOS_VIDEO_VSYNC_EXTI_LINE		EXTI_Line11
#define PIOS_VIDEO_VSYNC_EXTI_PORT_SOURCE		EXTI_PortSourceGPIOC
#define PIOS_VIDEO_VSYNC_EXTI_PIN_SOURCE		EXTI_PinSource11
#define PIOS_VIDEO_VSYNC_IRQn			EXTI15_10_IRQn
#define PIOS_VIDEO_VSYNC_PRIO			PIOS_IRQ_PRIO_HIGHEST
#endif

// *****************************************************************
//--------------------------
// Timer controller settings
//--------------------------
#define PIOS_TIM_MAX_DEVS			6

//-------------------------
// USB
//-------------------------
#define PIOS_USB_MAX_DEVS                       1
#define PIOS_USB_ENABLED                        1 /* Should remove all references to this */
#define PIOS_USB_HID_MAX_DEVS                   1


#endif /* PIOS_BOARD_H */
