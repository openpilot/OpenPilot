/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 *
 * @file       pios_config.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PiOS configuration header. 
 *             Central compile time config for the project.
 *             In particular, pios_config.h is where you define which PiOS libraries
 *             and features are included in the firmware.
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

#ifndef PIOS_CONFIG_H
#define PIOS_CONFIG_H

/* Enable/Disable PiOS Modules */
#define PIOS_INCLUDE_ADC
#define PIOS_INCLUDE_DELAY
#if defined(USE_I2C)
#define PIOS_INCLUDE_I2C
#define PIOS_INCLUDE_I2C_ESC
#endif
#define PIOS_INCLUDE_IRQ
#define PIOS_INCLUDE_LED

/*
 * Serial port configuration.
 * TODO: This should be dynamic in the future.
 * But for now define any compatile combination of:
 *   USE_I2C (shared with USART3)
 *   USE_TELEMETRY
 *   USE_GPS
 *   USE_SPEKTRUM
 *   USE_SBUS (USART1 only, it needs an invertor)
 * and optionally define PIOS_PORT_* to USART port numbers
 */

/* Current defaults - mimic original behavior */
#define USE_TELEMETRY
#if !defined(USE_SPEKTRUM)
#define USE_GPS
#endif

/* Serial telemetry: USART1 or USART3 */
#if !defined(PIOS_PORT_TELEMETRY)
#define PIOS_PORT_TELEMETRY	1
#endif

/* GPS receiver: USART1 or USART3 */
#if !defined(PIOS_PORT_GPS)
#define PIOS_PORT_GPS		3
#endif

/* Spektrum satellite receiver: USART1 or USART3 */
#if !defined(PIOS_PORT_SPEKTRUM)
#define PIOS_PORT_SPEKTRUM	3
#endif

/* Futaba S.Bus receiver: USART1 only (needs invertor) */
#if !defined(PIOS_PORT_SBUS)
#define PIOS_PORT_SBUS		1
#endif

/*
 * Define USART ports and check for conflicts.
 * Make sure it does not conflict with each other and with I2C.
 */
#define USART_GPIO(port)	(((port) == 1) ? GPIOA : GPIOB)
#define USART_RXIO(port)	(((port) == 1) ? GPIO_Pin_10 : GPIO_Pin_11)
#define USART_TXIO(port)	(((port) == 1) ? GPIO_Pin_9  : GPIO_Pin_10)
 
#if defined(USE_TELEMETRY)
#if defined(USE_I2C) && (PIOS_PORT_TELEMETRY == 3)
#error defined(USE_I2C) && (PIOS_PORT_TELEMETRY == 3)
#endif
#if (PIOS_PORT_TELEMETRY == 1)
#define PIOS_USART_TELEMETRY	USART1
#define PIOS_IRQH_TELEMETRY	USART1_IRQHandler
#define PIOS_IRQC_TELEMETRY	USART1_IRQn
#else
#define PIOS_USART_TELEMETRY	USART3
#define PIOS_IRQH_TELEMETRY	USART3_IRQHandler
#define PIOS_IRQC_TELEMETRY	USART3_IRQn
#endif
#define PIOS_GPIO_TELEMETRY	USART_GPIO(PIOS_PORT_TELEMETRY)
#define PIOS_RXIO_TELEMETRY	USART_RXIO(PIOS_PORT_TELEMETRY)
#define PIOS_TXIO_TELEMETRY	USART_TXIO(PIOS_PORT_TELEMETRY)
#define PIOS_INCLUDE_TELEMETRY_RF
#endif

#if defined(USE_GPS)
#if defined(USE_I2C) && (PIOS_PORT_GPS == 3)
#error defined(USE_I2C) && (PIOS_PORT_GPS == 3)
#endif
#if defined(USE_TELEMETRY) && (PIOS_PORT_TELEMETRY == PIOS_PORT_GPS)
#error defined(USE_TELEMETRY) && (PIOS_PORT_TELEMETRY == PIOS_PORT_GPS)
#endif
#if (PIOS_PORT_GPS == 1)
#define PIOS_USART_GPS		USART1
#define PIOS_IRQH_GPS		USART1_IRQHandler
#define PIOS_IRQC_GPS		USART1_IRQn
#else
#define PIOS_USART_GPS		USART3
#define PIOS_IRQH_GPS		USART3_IRQHandler
#define PIOS_IRQC_GPS		USART3_IRQn
#endif
#define PIOS_GPIO_GPS		USART_GPIO(PIOS_PORT_GPS)
#define PIOS_RXIO_GPS		USART_RXIO(PIOS_PORT_GPS)
#define PIOS_TXIO_GPS		USART_TXIO(PIOS_PORT_GPS)
#define PIOS_INCLUDE_GPS
#endif

#if defined(USE_SPEKTRUM)
#if defined(USE_I2C) && (PIOS_PORT_SPEKTRUM == 3)
#error defined(USE_I2C) && (PIOS_PORT_SPEKTRUM == 3)
#endif
#if defined(USE_TELEMETRY) && (PIOS_PORT_SPEKTRUM == PIOS_PORT_TELEMETRY)
#error defined(USE_TELEMETRY) && (PIOS_PORT_SPEKTRUM == PIOS_PORT_TELEMETRY)
#endif
#if defined(USE_GPS) && (PIOS_PORT_SPEKTRUM == PIOS_PORT_GPS)
#error defined(USE_GPS) && (PIOS_PORT_SPEKTRUM == PIOS_PORT_GPS)
#endif
#if defined(USE_SBUS)
#error defined(USE_SPEKTRUM) && defined(USE_SBUS)
#endif
#if (PIOS_PORT_SPEKTRUM == 1)
#define PIOS_USART_SPEKTRUM	USART1
#define PIOS_IRQH_SPEKTRUM	USART1_IRQHandler
#define PIOS_IRQC_SPEKTRUM	USART1_IRQn
#else
#define PIOS_USART_SPEKTRUM	USART3
#define PIOS_IRQH_SPEKTRUM	USART3_IRQHandler
#define PIOS_IRQC_SPEKTRUM	USART3_IRQn
#endif
#define PIOS_GPIO_SPEKTRUM	USART_GPIO(PIOS_PORT_SPEKTRUM)
#define PIOS_RXIO_SPEKTRUM	USART_RXIO(PIOS_PORT_SPEKTRUM)
#define PIOS_TXIO_SPEKTRUM	USART_TXIO(PIOS_PORT_SPEKTRUM)
#define PIOS_INCLUDE_SPEKTRUM
#endif

#if defined(USE_SBUS)
#if (PIOS_PORT_SBUS != 1)
#error (PIOS_PORT_SBUS != 1)
#endif
#if defined(USE_TELEMETRY) && (PIOS_PORT_SBUS == PIOS_PORT_TELEMETRY)
#error defined(USE_TELEMETRY) && (PIOS_PORT_SBUS == PIOS_PORT_TELEMETRY)
#endif
#if defined(USE_GPS) && (PIOS_PORT_SBUS == PIOS_PORT_GPS)
#error defined(USE_GPS) && (PIOS_PORT_SBUS == PIOS_PORT_GPS)
#endif
#if defined(USE_SPEKTRUM)
#error defined(USE_SPEKTRUM) && defined(USE_SBUS)
#endif
#define PIOS_USART_SBUS		USART1
#define PIOS_IRQH_SBUS		USART1_IRQHandler
#define PIOS_IRQC_SBUS		USART1_IRQn
#define PIOS_GPIO_SBUS		USART_GPIO(PIOS_PORT_SBUS)
#define PIOS_RXIO_SBUS		USART_RXIO(PIOS_PORT_SBUS)
#define PIOS_TXIO_SBUS		USART_TXIO(PIOS_PORT_SBUS)
#define PIOS_INCLUDE_SBUS
#endif

/* Receiver interfaces - only one allowed */
#if !defined(USE_SPEKTRUM) && !defined(USE_SBUS)
//#define PIOS_INCLUDE_PPM
#define PIOS_INCLUDE_PWM
#endif

#define PIOS_INCLUDE_SERVO
#define PIOS_INCLUDE_SPI
#define PIOS_INCLUDE_SYS
#define PIOS_INCLUDE_USART
#define PIOS_INCLUDE_USB_HID
#define PIOS_INCLUDE_COM
#define PIOS_INCLUDE_SETTINGS
#define PIOS_INCLUDE_FREERTOS
#define PIOS_INCLUDE_GPIO
#define PIOS_INCLUDE_EXTI
#define PIOS_INCLUDE_RTC
#define PIOS_INCLUDE_WDG
#define PIOS_INCLUDE_BL_HELPER

#define PIOS_INCLUDE_ADXL345
#define PIOS_INCLUDE_FLASH

/* A really shitty setting saving implementation */
#define PIOS_INCLUDE_FLASH_SECTOR_SETTINGS

/* Defaults for Logging */
#define LOG_FILENAME 			"PIOS.LOG"
#define STARTUP_LOG_ENABLED		1

/* COM Module */
#define GPS_BAUDRATE			19200
#define TELEM_BAUDRATE			19200
#define AUXUART_ENABLED			0
#define AUXUART_BAUDRATE		19200

/* Alarm Thresholds */
#define HEAP_LIMIT_WARNING		350
#define HEAP_LIMIT_CRITICAL		250
#define CPULOAD_LIMIT_WARNING		80
#define CPULOAD_LIMIT_CRITICAL		95

/* Task stack sizes */
#define PIOS_ACTUATOR_STACK_SIZE       1020
#define PIOS_MANUAL_STACK_SIZE          724
#define PIOS_SYSTEM_STACK_SIZE          560
#define PIOS_STABILIZATION_STACK_SIZE   524
#define PIOS_TELEM_STACK_SIZE           500

#define IDLE_COUNTS_PER_SEC_AT_NO_LOAD 1995998
//#define PIOS_QUATERNION_STABILIZATION

#endif /* PIOS_CONFIG_H */
/**
 * @}
 * @}
 */
