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


/* PIOS Compile Time Configuration */
#include "pios_config.h"
#include "pios_board.h"

/* C Lib Includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

/* STM32 Std Perf Lib */
#include <stm32f10x.h>
#include <stm32f10x_conf.h>

#if !defined(PIOS_DONT_USE_SDCARD)
/* Dosfs Includes */
#include <dosfs.h>

/* minIni Includes */
#include <minIni.h>

/* Mass Storage Device Includes */
#include <msd.h>
#endif

/* PIOS Hardware Includes (STM32F10x) */
#include <pios_sys.h>
#include <pios_delay.h>
#include <pios_led.h>
#include <pios_sdcard.h>
#include <pios_usart.h>
#include <pios_irq.h>
#include <pios_adc.h>
#include <pios_servo.h>
#include <pios_i2c.h>
#include <pios_spi.h>
#include <pios_pwm.h>
#include <pios_usb.h>
#include <pios_usb_hid.h>

/* PIOS Hardware Includes (Common) */
#include <pios_settings.h>
#include <pios_sdcard.h>
#include <pios_com.h>
#include <pios_bmp085.h>

/* More added here as they get written */

#if !defined(PIOS_DONT_USE_USB)
/* USB Libs */
#include <usb_lib.h>
#endif

#endif /* PIOS_H */
