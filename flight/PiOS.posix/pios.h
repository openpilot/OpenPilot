/**
 ******************************************************************************
 *
 * @file       pios.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Main PiOS header. 
 *                 - Central header for the project.
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


#ifndef PIOS_H
#define PIOS_H

/* PIOS Feature Selection */
#include "pios_config_posix.h"
#include <pios_posix.h>

#if defined(PIOS_INCLUDE_FREERTOS)
/* FreeRTOS Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif

/* C Lib Includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

/* Generic initcall infrastructure */
#include "pios_initcall.h"

/* PIOS Board Specific Device Configuration */
#include "pios_board_posix.h"

/* PIOS Hardware Includes (posix) */
#include <pios_sys.h>
#include <pios_delay.h>
#include <pios_led.h>
#include <pios_sdcard.h>
#include <pios_udp.h>
#include <pios_com.h>
#include <pios_servo.h>
#include <pios_wdg.h>
#include <pios_debug.h>

#define NELEMENTS(x) (sizeof(x) / sizeof(*(x)))

#endif /* PIOS_H */
