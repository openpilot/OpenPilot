/**
 ******************************************************************************
 *
 * @file       pios_config.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PiOS configuration header. 
 *             Central compile time config for the project.
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


#ifndef PIOS_CONFIG_POSIX_H
#define PIOS_CONFIG_POSIX_H


/* Enable/Disable PiOS Modules */
#define PIOS_INCLUDE_SYS
#define PIOS_INCLUDE_DELAY
#define PIOS_INCLUDE_LED
#define PIOS_INCLUDE_SDCARD
#define PIOS_INCLUDE_FREERTOS
#define PIOS_INCLUDE_COM
#define PIOS_INCLUDE_UDP
#define PIOS_INCLUDE_SERVO


/* Defaults for Logging */
#define LOG_FILENAME 			"PIOS.LOG"
#define STARTUP_LOG_ENABLED		1

/* COM Module */
#define GPS_BAUDRATE			19200
#define TELEM_BAUDRATE			19200
#define AUXUART_ENABLED			0
#define AUXUART_BAUDRATE		19200

#define TELEM_QUEUE_SIZE                20
#define PIOS_TELEM_STACK_SIZE           2048

/* Stabilization options */
#define PIOS_QUATERNION_STABILIZATION

/* GPS options */
#define PIOS_GPS_SETS_HOMELOCATION

#endif /* PIOS_CONFIG_POSIX_H */
