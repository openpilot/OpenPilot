/**
 ******************************************************************************
 * @addtogroup OpenPilotBL OpenPilot BootLoader
 * @{
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
#define PIOS_INCLUDE_BL_HELPER
#define PIOS_INCLUDE_BL_HELPER_WRITE_SUPPORT
#define USB_HID
/* Enable/Disable PiOS Modules */
#define PIOS_INCLUDE_DELAY
#define PIOS_INCLUDE_IRQ
#define PIOS_INCLUDE_LED
#define PIOS_INCLUDE_SPI
#define PIOS_INCLUDE_SYS
#define PIOS_INCLUDE_USART
#define PIOS_INCLUDE_USB_HID
#define PIOS_INCLUDE_OPAHRS
#define PIOS_INCLUDE_COM
#define PIOS_INCLUDE_GPIO
#define PIOS_NO_GPS
//#define DEBUG_SSP

/* Defaults for Logging */
#define LOG_FILENAME 			"PIOS.LOG"
#define STARTUP_LOG_ENABLED		1

/* COM Module */
#define GPS_BAUDRATE			19200
#define TELEM_BAUDRATE			19200
#define AUXUART_ENABLED			0
#define AUXUART_BAUDRATE		19200

/* Servos */
#define SERVOS_POSITION_MIN		800
#define SERVOS_POSITION_MAX		2200

#endif /* PIOS_CONFIG_H */
/**
 * @}
 * @}
 */
