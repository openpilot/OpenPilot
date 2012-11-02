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


#include <stdbool.h>

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

//------------------------
// BOOTLOADER_SETTINGS
//------------------------
#define BOARD_READABLE	true
#define BOARD_WRITABLE	true
#define MAX_DEL_RETRYS	3

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

#define PIOS_SYSCLK										168000000
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
// TELEMETRY
//------------------------
#define TELEM_QUEUE_SIZE         20
#define PIOS_TELEM_STACK_SIZE    624

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
#define PIOS_COM_AUX                    (pios_com_aux_id)
#define PIOS_COM_GPS                    (pios_com_gps_id)
#define PIOS_COM_TELEM_USB              (pios_com_telem_usb_id)
#define PIOS_COM_TELEM_RF               (pios_com_telem_rf_id)
#define PIOS_COM_DEBUG                  PIOS_COM_AUX
#define PIOS_COM_OSD                 	(pios_com_aux_id)

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
// PIOS_ADC_PinGet(0) = Current
// PIOS_ADC_PinGet(1) = Voltage
// PIOS_ADC_PinGet(2) = Flight
// PIOS_ADC_PinGet(3) = Temperature sensor
// PIOS_ADC_PinGet(4) = Video
// PIOS_ADC_PinGet(5) = RSSI
// PIOS_ADC_PinGet(6) = VREF
//-------------------------

#define PIOS_DMA_PIN_CONFIG \
{ \
{GPIOC, GPIO_Pin_0, ADC_Channel_10}, \
{GPIOC, GPIO_Pin_1, ADC_Channel_11}, \
{GPIOC, GPIO_Pin_2, ADC_Channel_12}, \
{NULL, 0, ADC_Channel_TempSensor}, /* Temperature sensor */\
{GPIOC, GPIO_Pin_3, ADC_Channel_13}, \
{GPIOA, GPIO_Pin_7, ADC_Channel_7}, \
{NULL, 0, ADC_Channel_Vrefint} /* Voltage reference */\
}

/* we have to do all this to satisfy the PIOS_ADC_MAX_SAMPLES define in pios_adc.h */
/* which is annoying because this then determines the rate at which we generate buffer turnover events */
/* the objective here is to get enough buffer space to support 100Hz averaging rate */
#define PIOS_ADC_NUM_CHANNELS 7
#define PIOS_ADC_MAX_OVERSAMPLING 10
#define PIOS_ADC_USE_ADC2 0
#define PIOS_ADC_USE_TEMP_SENSOR 1

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

//------------------------
// PIOS_I2C
// See also pios_board.c
//------------------------
#define PIOS_I2C_MAX_DEVS			1
extern uint32_t pios_i2c_flexiport_adapter_id;
#define PIOS_I2C_MAIN_ADAPTER			(pios_i2c_flexiport_adapter_id)
#define PIOS_I2C_ESC_ADAPTER			(pios_i2c_flexiport_adapter_id)
#define PIOS_I2C_BMP085_ADAPTER			(pios_i2c_flexiport_adapter_id)

//------------------------
// PIOS_BMP085
//------------------------
#define PIOS_BMP085_OVERSAMPLING                3


/**
 * glue macros for file IO
 * STM32 uses DOSFS for file IO
 */
#define PIOS_FOPEN_READ(filename,file)  DFS_OpenFile(&PIOS_SDCARD_VolInfo, (uint8_t *)filename, DFS_READ, PIOS_SDCARD_Sector, &file) != DFS_OK

#define PIOS_FOPEN_WRITE(filename,file) DFS_OpenFile(&PIOS_SDCARD_VolInfo, (uint8_t *)filename, DFS_WRITE, PIOS_SDCARD_Sector, &file) != DFS_OK

#define PIOS_FREAD(file,bufferadr,length,resultadr)     DFS_ReadFile(file, PIOS_SDCARD_Sector, (uint8_t*)bufferadr, resultadr, length) != DFS_OK

#define PIOS_FWRITE(file,bufferadr,length,resultadr)    DFS_WriteFile(file, PIOS_SDCARD_Sector, (uint8_t*)bufferadr, resultadr, length)

#define PIOS_FCLOSE(file)               DFS_Close(&file)

#define PIOS_FUNLINK(filename)          DFS_UnlinkFile(&PIOS_SDCARD_VolInfo, (uint8_t *)filename, PIOS_SDCARD_Sector)


#endif /* PIOS_BOARD_H */
