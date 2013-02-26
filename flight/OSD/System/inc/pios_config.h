/**
 ******************************************************************************
 *
 * @file       pios_config.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PiOS configuration header.
 *                 - Central compile time config for the project.
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

/* Major features */
#define PIOS_INCLUDE_FREERTOS
#define PIOS_INCLUDE_BL_HELPER

/* Enable/Disable PiOS Modules */
#define PIOS_INCLUDE_ADC
#define PIOS_INCLUDE_DELAY
#define PIOS_INCLUDE_I2C
#define PIOS_INCLUDE_IRQ
#define PIOS_INCLUDE_LED
#define PIOS_INCLUDE_IAP
//#define PIOS_INCLUDE_SERVO
#define PIOS_INCLUDE_SPI
#define PIOS_INCLUDE_SYS
#define PIOS_INCLUDE_USART
//#define PIOS_INCLUDE_GPIO
#define PIOS_INCLUDE_USB
#define PIOS_INCLUDE_USB_HID
#define PIOS_INCLUDE_EXTI
#define PIOS_INCLUDE_RTC
#define PIOS_INCLUDE_WDG
#define PIOS_INCLUDE_VIDEO

/* Com systems to include */
#define PIOS_INCLUDE_COM
#define PIOS_INCLUDE_COM_TELEM
#define PIOS_INCLUDE_COM_AUX
#define PIOS_INCLUDE_GPS
//#define PIOS_OVERO_SPI
#define PIOS_INCLUDE_BMP085
#define PIOS_INCLUDE_HMC5883
#define PIOS_INCLUDE_SDCARD
//#define PIOS_INCLUDE_WAVE
/* Supported receiver interfaces */
//#define PIOS_INCLUDE_RCVR
//#define PIOS_INCLUDE_DSM
//#define PIOS_INCLUDE_SBUS
//#define PIOS_INCLUDE_PPM
//#define PIOS_INCLUDE_PWM
//#define PIOS_INCLUDE_GCSRCVR

//#define PIOS_INCLUDE_SETTINGS
//#define PIOS_INCLUDE_FLASH
/* A really shitty setting saving implementation */
//#define PIOS_INCLUDE_FLASH_SECTOR_SETTINGS

#define PIOS_INCLUDE_INITCALL           /* Include init call structures */
#define PIOS_TELEM_PRIORITY_QUEUE       /* Enable a priority queue in telemetry */
#define PIOS_QUATERNION_STABILIZATION   /* Stabilization options */
#define PIOS_GPS_SETS_HOMELOCATION      /* GPS options */
#define PIOS_INCLUDE_GPS_NMEA_PARSER /* Include the NMEA protocol parser */
#define PIOS_INCLUDE_GPS_UBX_PARSER  /* Include the UBX protocol parser */

/* Alarm Thresholds */
#define HEAP_LIMIT_WARNING		4000
#define HEAP_LIMIT_CRITICAL		1000
#define IRQSTACK_LIMIT_WARNING		150
#define IRQSTACK_LIMIT_CRITICAL		80
#define CPULOAD_LIMIT_WARNING		80
#define CPULOAD_LIMIT_CRITICAL		95

// This actually needs calibrating
#define IDLE_COUNTS_PER_SEC_AT_NO_LOAD (8379692)
#define LOG_FILENAME 			"PIOS.LOG"

#endif /* PIOS_CONFIG_H */
