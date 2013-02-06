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
#define PIOS_INCLUDE_DELAY
#define PIOS_INCLUDE_IRQ
#define PIOS_INCLUDE_LED
#define PIOS_INCLUDE_IAP
#define PIOS_INCLUDE_RFM22B
#define PIOS_INCLUDE_RFM22B_COM
#define PIOS_INCLUDE_RCVR
#define PIOS_INCLUDE_TIM

/* Supported receiver interfaces */
#define PIOS_INCLUDE_PPM

/* Supported USART-based PIOS modules */
#define PIOS_INCLUDE_SPI
#define PIOS_INCLUDE_SYS
#define PIOS_INCLUDE_USART
#define PIOS_INCLUDE_USB
#define PIOS_INCLUDE_USB_HID
#define PIOS_INCLUDE_USB_CDC
#define PIOS_INCLUDE_COM
#define PIOS_INCLUDE_SETTINGS
#define PIOS_INCLUDE_FREERTOS
#define PIOS_INCLUDE_GPIO
#define PIOS_INCLUDE_EXTI
#define PIOS_INCLUDE_RTC
#define PIOS_INCLUDE_WDG
#define PIOS_INCLUDE_BL_HELPER
#define PIOS_INCLUDE_FLASH_EEPROM
#define PIOS_INCLUDE_RFM22B
#define PIOS_INCLUDE_PACKET_HANDLER

/* Defaults for Logging */
#define LOG_FILENAME 			"PIOS.LOG"
#define STARTUP_LOG_ENABLED		1

/* COM Module */
#define GPS_BAUDRATE			19200
#define TELEM_BAUDRATE			19200
#define AUXUART_ENABLED			0
#define AUXUART_BAUDRATE		19200

/* Alarm Thresholds */
#define HEAP_LIMIT_WARNING             220
#define HEAP_LIMIT_CRITICAL             40
#define IRQSTACK_LIMIT_WARNING		100
#define IRQSTACK_LIMIT_CRITICAL		60
#define CPULOAD_LIMIT_WARNING		85
#define CPULOAD_LIMIT_CRITICAL		95

/* Task stack sizes */
#define PIOS_ACTUATOR_STACK_SIZE       1020
#define PIOS_MANUAL_STACK_SIZE          724
#define PIOS_SYSTEM_STACK_SIZE          460
#define PIOS_STABILIZATION_STACK_SIZE   524
#define PIOS_TELEM_STACK_SIZE           500
#define PIOS_EVENTDISPATCHER_STACK_SIZE 130
#define IDLE_COUNTS_PER_SEC_AT_NO_LOAD 1995998
//#define PIOS_QUATERNION_STABILIZATION

// This can't be too high to stop eventdispatcher thread overflowing
#define PIOS_EVENTDISAPTCHER_QUEUE      10

/* PIOS Initcall infrastructure */
#define PIOS_INCLUDE_INITCALL

/* Turn on debugging signals on the telemetry port */
//#define PIOS_RFM22B_DEBUG_ON_TELEM

#endif /* PIOS_CONFIG_H */
/**
 * @}
 * @}
 */
