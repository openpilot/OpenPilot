/**
 ******************************************************************************
 * @file       pios.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2013
 * @author     PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
 * @brief      Main PiOS header.
 * @see        The GNU Public License (GPL) Version 3
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

/*
 * To add new PIOS options, drivers or functions please insert their
 * includes below into a corresponding group. If new driver has optional
 * features, add them as comments before including the driver header.
 * Finally update all pios_config.h files for every board in the same order
 * as in this file. Include new definition and all options, but comment
 * out unused ones.
 */

#ifndef PIOS_H
#define PIOS_H

#include <pios_helpers.h>
#include <pios_math.h>
#include <pios_constants.h>

#ifdef USE_SIM_POSIX
/* SimPosix version of this file. This will probably be removed later */
#include <pios_sim_posix.h>
#else

/* C Lib includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

/* STM32 Std Peripherals Lib */
#if defined(STM32F10X)
#include <stm32f10x.h>
#elif defined(STM32F4XX)
#include <stm32f4xx.h>
#include <stm32f4xx_rcc.h>
#endif

/* PIOS board specific feature selection */
#include "pios_config.h"

/* PIOS board specific device configuration */
#include "pios_board.h"

/* PIOS debug interface */
/* #define PIOS_INCLUDE_DEBUG_CONSOLE */
/* #define DEBUG_LEVEL 0 */
/* #define PIOS_ENABLE_DEBUG_PINS */
#include <pios_debug.h>
#include <pios_debuglog.h>

/* PIOS common functions */
#include <pios_crc.h>

/* PIOS FreeRTOS support */
#ifdef PIOS_INCLUDE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif

#include <stdbool.h>

#include <pios_architecture.h>

#ifdef PIOS_INCLUDE_TASK_MONITOR
#ifndef PIOS_INCLUDE_FREERTOS
#error PiOS Task Monitor requires PIOS_INCLUDE_FREERTOS to be defined
#endif
#include <pios_task_monitor.h>
#endif

/* PIOS CallbackScheduler */
#ifdef PIOS_INCLUDE_CALLBACKSCHEDULER
#ifndef PIOS_INCLUDE_FREERTOS
#error PiOS CallbackScheduler requires PIOS_INCLUDE_FREERTOS to be defined
#endif
#include <pios_callbackscheduler.h>
#endif

/* PIOS bootloader helper */
#ifdef PIOS_INCLUDE_BL_HELPER
/* #define PIOS_INCLUDE_BL_HELPER_WRITE_SUPPORT */
#include <pios_bl_helper.h>
#endif

/* PIOS system functions */
#ifdef PIOS_INCLUDE_DELAY
#include <pios_delay.h>
#include <pios_deltatime.h>
#endif

#ifdef PIOS_INCLUDE_INITCALL
#include "pios_initcall.h"
#endif

#ifdef PIOS_INCLUDE_SYS
#include <pios_sys.h>
#endif

/* PIOS hardware peripherals */
#ifdef PIOS_INCLUDE_IRQ
#include <pios_irq.h>
#endif

#ifdef PIOS_INCLUDE_RTC
#include <pios_rtc.h>
#endif

#ifdef PIOS_INCLUDE_TIM
#include <pios_tim.h>
#endif

#ifdef PIOS_INCLUDE_USART
#include <pios_usart.h>
#endif

#ifdef PIOS_INCLUDE_ADC
#include <pios_adc.h>
#endif

#ifdef PIOS_INCLUDE_I2C
#include <pios_i2c.h>
#endif

#ifdef PIOS_INCLUDE_SPI
#include <pios_spi.h>
#endif

#ifdef PIOS_INCLUDE_GPIO
#include <pios_gpio.h>
#endif

#ifdef PIOS_INCLUDE_EXTI
#include <pios_exti.h>
#endif

#ifdef PIOS_INCLUDE_WDG
#include <pios_wdg.h>
#endif

/* PIOS USB functions */
#ifdef PIOS_INCLUDE_USB
/* #define PIOS_INCLUDE_USB_HID */
/* #define PIOS_INCLUDE_USB_CDC */
/* #define PIOS_INCLUDE_USB_RCTX */
#include <pios_usb.h>
#ifdef PIOS_INCLUDE_USB_HID
#include <pios_usb_hid.h>
#endif
#ifdef PIOS_INCLUDE_USB_RCTX
#include <pios_usb_rctx.h>
#endif
#endif

/* PIOS sensor interfaces */
#ifdef PIOS_INCLUDE_ADXL345
/* ADXL345 3-Axis Accelerometer */
#include <pios_adxl345.h>
#endif

#ifdef PIOS_INCLUDE_BMA180
/* BMA180 3-Axis Accelerometer */
#include <pios_bma180.h>
#endif

#ifdef PIOS_INCLUDE_L3GD20
/* L3GD20 3-Axis Gyro */
#include <pios_l3gd20.h>
#endif

#ifdef PIOS_INCLUDE_MPU6000
/* MPU6000 3-Axis Gyro/Accelerometer */
/* #define PIOS_MPU6000_ACCEL */
#include <pios_mpu6000.h>
#endif

#ifdef PIOS_INCLUDE_HMC5843
/* HMC5843 3-Axis Digital Compass */
#include <pios_hmc5843.h>
#endif

#ifdef PIOS_INCLUDE_HMC5883
/* HMC5883 3-Axis Digital Compass */
/* #define PIOS_HMC5883_HAS_GPIOS */
#include <pios_hmc5883.h>
#endif

#ifdef PIOS_INCLUDE_BMP085
/* BMP085 Barometric Pressure Sensor */
#include <pios_bmp085.h>
#endif

#ifdef PIOS_INCLUDE_MS5611
/* MS5611 Barometric Pressure Sensor */
#include <pios_ms5611.h>
#endif

#ifdef PIOS_INCLUDE_MPXV
/* MPXV5004, MPXV7002 based Airspeed Sensor */
#include <pios_mpxv.h>
#endif

#ifdef PIOS_INCLUDE_ETASV3
/* Eagle Tree Systems Airspeed MicroSensor V3 */
#include <pios_etasv3.h>
#endif

#ifdef PIOS_INCLUDE_MS4525DO
/* PixHawk Airspeed Sensor based on MS4525DO */
#include <pios_ms4525do.h>
#endif


#ifdef PIOS_INCLUDE_HCSR04
/* HC-SR04 Ultrasonic Sensor */
#include <pios_hcsr04.h>
#endif

/* PIOS receiver drivers */
#ifdef PIOS_INCLUDE_PWM
#include <pios_pwm.h>
#endif

#ifdef PIOS_INCLUDE_PPM
#include <pios_ppm.h>
#endif

#ifdef PIOS_INCLUDE_PPM_FLEXI
/* PPM on CC flexi port */
#endif

#ifdef PIOS_INCLUDE_DSM
#include <pios_dsm.h>
#endif

#ifdef PIOS_INCLUDE_SBUS
#include <pios_sbus.h>
#endif

/* PIOS abstract receiver interface */
#ifdef PIOS_INCLUDE_RCVR
#include <pios_rcvr.h>
#endif

/* PIOS common peripherals */
#ifdef PIOS_INCLUDE_LED
#include <pios_led.h>
#endif

#ifdef PIOS_INCLUDE_IAP
#include <pios_iap.h>
#endif

#ifdef PIOS_INCLUDE_SERVO
#include <pios_servo.h>
#endif

#ifdef PIOS_INCLUDE_I2C_ESC
#include <pios_i2c_esc.h>
#endif

#ifdef PIOS_INCLUDE_OVERO
/* #define PIOS_OVERO_SPI */
#include <pios_overo.h>
#endif

#ifdef PIOS_INCLUDE_SDCARD
/* #define LOG_FILENAME "startup.log" */
#include <dosfs.h>
#include <pios_sdcard.h>
#endif

#ifdef PIOS_INCLUDE_FLASH
/* #define PIOS_INCLUDE_FLASH_LOGFS_SETTINGS */
/* #define FLASH_FREERTOS */
#include <pios_flash.h>
#include <pios_flashfs.h>
#endif

/* driver for storage on internal flash */
/* #define PIOS_INCLUDE_FLASH_INTERNAL */

#ifdef PIOS_INCLUDE_FLASH_EEPROM
#include <pios_eeprom.h>
#endif

/* PIOS radio modules */
#ifdef PIOS_INCLUDE_RFM22B
/* #define PIOS_INCLUDE_PPM_OUT */
/* #define PIOS_RFM22B_DEBUG_ON_TELEM */
#include <pios_rfm22b.h>
#ifdef PIOS_INCLUDE_RFM22B_COM
#include <pios_rfm22b_com.h>
#endif
#endif /* PIOS_INCLUDE_RFM22B */

/* PIOS misc peripherals */
#ifdef PIOS_INCLUDE_VIDEO
#include <pios_video.h>
#endif

#ifdef PIOS_INCLUDE_WAVE
#include <pios_wavplay.h>
#endif

#ifdef PIOS_INCLUDE_UDP
#include <pios_udp.h>
#endif

/* PIOS abstract comms interface with options */
#ifdef PIOS_INCLUDE_COM
/* #define PIOS_INCLUDE_COM_MSG */
/* #define PIOS_INCLUDE_TELEMETRY_RF */
/* #define PIOS_INCLUDE_COM_TELEM */
/* #define PIOS_INCLUDE_COM_FLEXI */
/* #define PIOS_INCLUDE_COM_AUX */
/* #define PIOS_TELEM_PRIORITY_QUEUE */
/* #define PIOS_INCLUDE_GPS */
/* #define PIOS_GPS_MINIMAL */
/* #define PIOS_INCLUDE_GPS_NMEA_PARSER */
/* #define PIOS_INCLUDE_GPS_UBX_PARSER */
/* #define PIOS_GPS_SETS_HOMELOCATION */
#include <pios_com.h>
#endif

/* Stabilization options */
/* #define PIOS_QUATERNION_STABILIZATION */

/* Performance counters */
/* #define IDLE_COUNTS_PER_SEC_AT_NO_LOAD 995998 */

#endif /* USE_SIM_POSIX */
#endif /* PIOS_H */
