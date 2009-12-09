/**
 ******************************************************************************
 *
 * @file       pios.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
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

/* C Lib Includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* STM32 Std Perf Lib */
#include <stm32f10x.h>
#include <stm32f10x_conf.h>

/* FatFS Includes */
#include <ff.h>
#include <diskio.h>

/* minIni Includes */
#include <minIni.h>

/* PIOS Hardware Includes (STM32F10x) */
#include "pios_board.h"
#include "pios_sys.h"
#include "pios_led.h"
#include "pios_usart.h"
#include "pios_irq.h"
#include "pios_adc.h"
#include "pios_servo.h"
#include "pios_i2c.h"

/* PIOS Hardware Includes (Common) */
#include "pios_settings.h"
#include "pios_com.h"

/* More added here as they get written */


#endif /* PIOS_H */
