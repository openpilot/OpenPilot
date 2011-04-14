/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_IMU3000 IMU3000 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       pios_imu3000.h
 * @author     David "Buzz" Carlson (buzz@chebuzz.com)
 * 				The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      IMU3000 3-axis gyor function headers
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
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

#ifndef PIOS_IMU3000_H
#define PIOS_IMU3000_H

/* IMU3000 Addresses */
#define PIOS_IMU3000_I2C_ADDR             0x69
#define PIOS_IMU3000_I2C_READ_ADDR        0xD2
#define PIOS_IMU3000_I2C_WRITE_ADDR       0xD3
#define PIOS_IMU3000_X_MSB_OFFSET_REG     0x0C
#define PIOS_IMU3000_X_LSB_OFFSET_REG     0x0D
#define PIOS_IMU3000_Y_MSB_OFFSET_REG     0x0E
#define PIOS_IMU3000_Y_LSB_OFFSET_REG     0x0F
#define PIOS_IMU3000_Z_MSB_OFFSET_REG     0x10
#define PIOS_IMU3000_Z_LSB_OFFSET_REG     0x11
#define PIOS_IMU3000_FIFO_EN_REG          0x12
#define PIOS_IMU3000_SMPLRT_DIV_REG       0X15
#define PIOS_IMU3000_DLPF_CFG_REG         0X16
#define PIOS_IMU3000_INT_CFG_REG          0x17
#define PIOS_IMU3000_INT_STATUS_REG       0x1A
#define PIOS_IMU3000_TEMP_OUT_MSB         0x1B
#define PIOS_IMU3000_TEMP_OUT_LSB         0x1C
#define PIOS_IMU3000_GYRO_X_OUT_MSB       0x1D
#define PIOS_IMU3000_GYRO_X_OUT_LSB       0x1E
#define PIOS_IMU3000_GYRO_Y_OUT_MSB       0x1F
#define PIOS_IMU3000_GYRO_Y_OUT_LSB       0x20
#define PIOS_IMU3000_GYRO_Z_OUT_MSB       0x21
#define PIOS_IMU3000_GYRO_Z_OUT_LSB       0x22
#define PIOS_IMU3000_FIFO_CNT_MSB         0x3A
#define PIOS_IMU3000_FIFO_CNT_LSB         0x3B
#define PIOS_IMU3000_FIFO_REG             0x3C
#define PIOS_IMU3000_USER_CTRL_REG        0x3D
#define PIOS_IMU3000_PWR_MGMT_REG         0x3E

/* FIFO enable for storing different values */
#define PIOS_IMU3000_FIFO_TEMP_OUT        0x80
#define PIOS_IMU3000_FIFO_GYRO_X_OUT      0x40
#define PIOS_IMU3000_FIFO_GYRO_Y_OUT      0x20
#define PIOS_IMU3000_FIFO_GYRO_Z_OUT      0x10
#define PIOS_IMU3000_FIFO_FOOTER          0x01

/* Gyro full-scale range */
#define PIOS_IMU3000_SCALE_250_DEG        0x00
#define PIOS_IMU3000_SCALE_500_DEG        0x01
#define PIOS_IMU3000_SCALE_1000_DEG       0x02
#define PIOS_IMU3000_SCALE_2000_DEG       0x03

/* Digital low-pass filter configuration */
#define PIOS_IMU3000_LOWPASS_256_HZ       0x00
#define PIOS_IMU3000_LOWPASS_188_HZ       0x01
#define PIOS_IMU3000_LOWPASS_98_HZ        0x02
#define PIOS_IMU3000_LOWPASS_42_HZ        0x03
#define PIOS_IMU3000_LOWPASS_20_HZ        0x04
#define PIOS_IMU3000_LOWPASS_10_HZ        0x05
#define PIOS_IMU3000_LOWPASS_5_HZ         0x06

/* Interrupt Configuration */
#define PIOS_IMU3000_INT_ACTL             0x80
#define PIOS_IMU3000_INT_OPEN             0x40
#define PIOS_IMU3000_INT_LATCH_EN         0x20
#define PIOS_IMU3000_INT_CLR_ANYRD        0x10
#define PIOS_IMU3000_INT_IMU_RDY          0x04
#define PIOS_IMU3000_INT_DATA_RDY         0x01

/* Interrupt status */
#define PIOS_IMU3000_INT_STATUS_FIFO_FULL 0x80
#define PIOS_IMU3000_INT_STATUS_IMU_RDY   0X04
#define PIOS_IMU3000_INT_STATUS_DATA_RDY  0X01

/* User control functionality */
#define PIOS_IMU3000_USERCTL_FIFO_EN      0X40
#define PIOS_IMU3000_USERCTL_FIFO_RST     0X02
#define PIOS_IMU3000_USERCTL_GYRO_RST     0X01

/* Power management and clock selection */
#define PIOS_IMU3000_PWRMGMT_IMU_RST      0X80
#define PIOS_IMU3000_PWRMGMT_INTERN_CLK   0X00
#define PIOS_IMU3000_PWRMGMT_PLL_X_CLK    0X01
#define PIOS_IMU3000_PWRMGMT_PLL_Y_CLK    0X02
#define PIOS_IMU3000_PWRMGMT_PLL_Z_CLK    0X03
#define PIOS_IMU3000_PWRMGMT_STOP_CLK     0X07


/* Public Functions */
extern void PIOS_IMU3000_Init(void);
extern bool PIOS_IMU3000_NewDataAvailable(void);
extern void PIOS_IMU3000_ReadGyros(int16_t out[3]);
extern uint8_t PIOS_IMU3000_ReadID();
extern uint8_t PIOS_IMU3000_Test();

#endif /* PIOS_IMU3000_H */

/** 
  * @}
  * @}
  */
