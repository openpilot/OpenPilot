/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MPU6050 MPU6050 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       PIOS_MPU6050.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      MPU6050 3-axis gyor function headers
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

#ifndef PIOS_MPU6050_H
#define PIOS_MPU6050_H

#include "pios.h"

/* MPU6050 Addresses */
#define PIOS_MPU6050_I2C_ADDR             0x68
#define PIOS_MPU6050_SMPLRT_DIV_REG       0X19
#define PIOS_MPU6050_DLPF_CFG_REG         0X1A
#define PIOS_MPU6050_GYRO_CFG_REG         0X1B
#define PIOS_MPU6050_ACCEL_CFG_REG        0X1C
#define PIOS_MPU6050_FIFO_EN_REG          0x23
#define PIOS_MPU6050_INT_CFG_REG          0x37
#define PIOS_MPU6050_INT_EN_REG           0x38
#define PIOS_MPU6050_INT_STATUS_REG       0x3A
#define PIOS_MPU6050_ACCEL_X_OUT_MSB      0x3B
#define PIOS_MPU6050_ACCEL_X_OUT_LSB      0x3C
#define PIOS_MPU6050_ACCEL_Y_OUT_MSB      0x3D
#define PIOS_MPU6050_ACCEL_Y_OUT_LSB      0x3E
#define PIOS_MPU6050_ACCEL_Z_OUT_MSB      0x3F
#define PIOS_MPU6050_ACCEL_Z_OUT_LSB      0x40
#define PIOS_MPU6050_TEMP_OUT_MSB         0x41
#define PIOS_MPU6050_TEMP_OUT_LSB         0x42
#define PIOS_MPU6050_GYRO_X_OUT_MSB       0x43
#define PIOS_MPU6050_GYRO_X_OUT_LSB       0x44
#define PIOS_MPU6050_GYRO_Y_OUT_MSB       0x45
#define PIOS_MPU6050_GYRO_Y_OUT_LSB       0x46
#define PIOS_MPU6050_GYRO_Z_OUT_MSB       0x47
#define PIOS_MPU6050_GYRO_Z_OUT_LSB       0x48
#define PIOS_MPU6050_USER_CTRL_REG        0x6A
#define PIOS_MPU6050_PWR_MGMT_REG         0x6B
#define PIOS_MPU6050_FIFO_CNT_MSB         0x72
#define PIOS_MPU6050_FIFO_CNT_LSB         0x73
#define PIOS_MPU6050_FIFO_REG             0x74
#define PIOS_MPU6050_WHOAMI               0x75

/* FIFO enable for storing different values */
#define PIOS_MPU6050_FIFO_TEMP_OUT        0x80
#define PIOS_MPU6050_FIFO_GYRO_X_OUT      0x40
#define PIOS_MPU6050_FIFO_GYRO_Y_OUT      0x20
#define PIOS_MPU6050_FIFO_GYRO_Z_OUT      0x10
#define PIOS_MPU6050_ACCEL_OUT            0x08

/* Interrupt Configuration */
#define PIOS_MPU6050_INT_ACTL             0x80
#define PIOS_MPU6050_INT_OPEN             0x40
#define PIOS_MPU6050_INT_LATCH_EN         0x20
#define PIOS_MPU6050_INT_CLR_ANYRD        0x10

#define PIOS_MPU6050_INTEN_OVERFLOW       0x10
#define PIOS_MPU6050_INTEN_DATA_RDY       0x01

/* Interrupt status */
#define PIOS_MPU6050_INT_STATUS_FIFO_FULL 0x80
#define PIOS_MPU6050_INT_STATUS_IMU_RDY   0X04
#define PIOS_MPU6050_INT_STATUS_DATA_RDY  0X01

/* User control functionality */
#define PIOS_MPU6050_USERCTL_FIFO_EN      0X40
#define PIOS_MPU6050_USERCTL_FIFO_RST     0X02
#define PIOS_MPU6050_USERCTL_GYRO_RST     0X01

/* Power management and clock selection */
#define PIOS_MPU6050_PWRMGMT_IMU_RST      0X80
#define PIOS_MPU6050_PWRMGMT_INTERN_CLK   0X00
#define PIOS_MPU6050_PWRMGMT_PLL_X_CLK    0X01
#define PIOS_MPU6050_PWRMGMT_PLL_Y_CLK    0X02
#define PIOS_MPU6050_PWRMGMT_PLL_Z_CLK    0X03
#define PIOS_MPU6050_PWRMGMT_STOP_CLK     0X07

enum pios_mpu6050_range {
	PIOS_MPU6050_SCALE_250_DEG  = 0x00,
	PIOS_MPU6050_SCALE_500_DEG  = 0x08,
	PIOS_MPU6050_SCALE_1000_DEG = 0x10,
	PIOS_MPU6050_SCALE_2000_DEG = 0x18
};

enum pios_mpu6050_filter {
	PIOS_MPU6050_LOWPASS_256_HZ = 0x00,
	PIOS_MPU6050_LOWPASS_188_HZ = 0x01,
	PIOS_MPU6050_LOWPASS_98_HZ  = 0x02,
	PIOS_MPU6050_LOWPASS_42_HZ  = 0x03,
	PIOS_MPU6050_LOWPASS_20_HZ  = 0x04,
	PIOS_MPU6050_LOWPASS_10_HZ  = 0x05,
	PIOS_MPU6050_LOWPASS_5_HZ   = 0x06
};

enum pios_mpu6050_accel_range {
	PIOS_MPU6050_ACCEL_2G = 0x00,
	PIOS_MPU6050_ACCEL_4G = 0x08,
	PIOS_MPU6050_ACCEL_8G = 0x10,
	PIOS_MPU6050_ACCEL_16G = 0x18
};

enum pios_mpu6050_orientation { // clockwise rotation from board forward
	PIOS_MPU6050_TOP_0DEG    = 0x00,
	PIOS_MPU6050_TOP_90DEG   = 0x01,
	PIOS_MPU6050_TOP_180DEG  = 0x02,
	PIOS_MPU6050_TOP_270DEG  = 0x03
};

struct pios_mpu6050_data {
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
#if defined(PIOS_MPU6050_ACCEL)
	int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
#endif /* PIOS_MPU6050_ACCEL */
	int16_t temperature;
};

struct pios_mpu6050_cfg {
	const struct pios_exti_cfg * exti_cfg; /* Pointer to the EXTI configuration */
	
	uint8_t Fifo_store;		/* FIFO storage of different readings (See datasheet page 31 for more details) */
	uint8_t Smpl_rate_div;	/* Sample rate divider to use (See datasheet page 32 for more details) */
	uint8_t interrupt_cfg;	/* Interrupt configuration (See datasheet page 35 for more details) */
	uint8_t interrupt_en;	/* Interrupt configuration (See datasheet page 35 for more details) */
	uint8_t User_ctl;		/* User control settings (See datasheet page 41 for more details)  */
	uint8_t Pwr_mgmt_clk;	/* Power management and clock selection (See datasheet page 32 for more details) */
	enum pios_mpu6050_accel_range accel_range;
	enum pios_mpu6050_range gyro_range;
	enum pios_mpu6050_filter filter;
	enum pios_mpu6050_orientation orientation;
};

/* Public Functions */
extern int32_t PIOS_MPU6050_Init(uint32_t i2c_id,const struct pios_mpu6050_cfg * new_cfg);
extern xQueueHandle PIOS_MPU6050_GetQueue();
extern int32_t PIOS_MPU6050_ReadGyros(struct pios_mpu6050_data * buffer);
extern int32_t PIOS_MPU6050_ReadAcc(struct pios_mpu6050_data * data);
extern int32_t PIOS_MPU6050_ReadID();
extern uint8_t PIOS_MPU6050_Test();
extern float PIOS_MPU6050_GetScale();
extern float PIOS_MPU6050_GetAccelScale();
extern bool PIOS_MPU6050_IRQHandler(void);

#endif /* PIOS_MPU6050_H */

/** 
  * @}
  * @}
  */
