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
#include "pios_config.h"

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

#include "pios_config.h"
/* STM32 Std Perf Lib */
#if defined(STM32F4XX)
# include <stm32f4xx.h>
# include <stm32f4xx_rcc.h>
#elif defined(STM32F2XX)
#include <stm32f2xx.h>
#include <stm32f2xx_syscfg.h>
#else
#include <stm32f10x.h>
#endif

#if defined(PIOS_INCLUDE_SDCARD)
/* Dosfs Includes */
#include <dosfs.h>

/* Mass Storage Device Includes */
#include <msd.h>
#endif

/* Generic initcall infrastructure */
#if defined(PIOS_INCLUDE_INITCALL)
#include "pios_initcall.h"
#endif

/* PIOS Board Specific Device Configuration */
#include "pios_board.h"

/* PIOS Hardware Includes (STM32F10x) */
#include <pios_sys.h>
#include <pios_delay.h>
#include <pios_led.h>
#include <pios_sdcard.h>
#include <pios_usart.h>
#include <pios_irq.h>
#include <pios_adc.h>
#include <pios_servo.h>
#include <pios_rtc.h>
#include <pios_i2c.h>
#include <pios_spi.h>
#include <pios_ppm.h>
#include <pios_pwm.h>
#include <pios_rcvr.h>
#include <pios_dsm.h>
#include <pios_sbus.h>
#include <pios_usb_hid.h>
#include <pios_debug.h>
#include <pios_gpio.h>
#include <pios_exti.h>
#include <pios_wdg.h>

/* PIOS Hardware Includes (Common) */
#include <pios_sdcard.h>
#include <pios_com.h>
#if defined(PIOS_INCLUDE_BMP085)
#include <pios_bmp085.h>
#endif
#if defined(PIOS_INCLUDE_HCSR04)
#include <pios_hcsr04.h>
#endif
#if defined(PIOS_INCLUDE_HMC5843)
#include <pios_hmc5843.h>
#endif
#if defined(PIOS_INCLUDE_HMC5883)
#include <pios_hmc5883.h>
#endif
#if defined(PIOS_INCLUDE_I2C_ESC)
#include <pios_i2c_esc.h>
#endif
#if defined(PIOS_INCLUDE_IMU3000)
#include <pios_imu3000.h>
#endif
#if defined(PIOS_INCLUDE_MPU6050)
#include <pios_mpu6050.h>
#endif
#if defined(PIOS_INCLUDE_MPU6000)
#include <pios_mpu6000.h>
#endif
#if defined(PIOS_INCLUDE_L3GD20)
#include <pios_l3gd20.h>
#endif
#if defined(PIOS_INCLUDE_MS5611)
#include <pios_ms5611.h>
#endif
#if defined(PIOS_INCLUDE_IAP)
#include <pios_iap.h>
#endif
#if defined(PIOS_INCLUDE_ADXL345)
#include <pios_adxl345.h>
#endif
#if defined(PIOS_INCLUDE_BMA180)
#include <pios_bma180.h>
#endif

#if defined(PIOS_INCLUDE_FLASH)
#include <pios_flash_jedec.h>
#include <pios_flashfs_objlist.h>
#endif

#if defined(PIOS_INCLUDE_BL_HELPER)
#include <pios_bl_helper.h>
#endif

#if defined(PIOS_INCLUDE_USB)
#include <pios_usb.h>
#endif

#include <pios_crc.h>

#define NELEMENTS(x) (sizeof(x) / sizeof(*(x)))

#endif /* PIOS_H */
