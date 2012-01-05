/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_L3GD20 L3GD20 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       PIOS_L3GD20.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      L3GD20 3-axis gyor function headers
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

#ifndef PIOS_L3GD20_H
#define PIOS_L3GD20_H

#include "pios.h"

/* MPU6050 Addresses */
#define PIOS_L3GD20_SMPLRT_DIV_REG       0X19
#define PIOS_L3GD20_DLPF_CFG_REG         0X1A
#define PIOS_L3GD20_GYRO_CFG_REG         0X1B
#define PIOS_L3GD20_ACCEL_CFG_REG        0X1C
#define PIOS_L3GD20_FIFO_EN_REG          0x23
#define PIOS_L3GD20_INT_CFG_REG          0x37
#define PIOS_L3GD20_INT_EN_REG           0x38
#define PIOS_L3GD20_INT_STATUS_REG       0x3A
#define PIOS_L3GD20_ACCEL_X_OUT_MSB      0x3B
#define PIOS_L3GD20_ACCEL_X_OUT_LSB      0x3C
#define PIOS_L3GD20_ACCEL_Y_OUT_MSB      0x3D
#define PIOS_L3GD20_ACCEL_Y_OUT_LSB      0x3E
#define PIOS_L3GD20_ACCEL_Z_OUT_MSB      0x3F
#define PIOS_L3GD20_ACCEL_Z_OUT_LSB      0x40
#define PIOS_L3GD20_TEMP_OUT_MSB         0x41
#define PIOS_L3GD20_TEMP_OUT_LSB         0x42
#define PIOS_L3GD20_GYRO_X_OUT_MSB       0x43
#define PIOS_L3GD20_GYRO_X_OUT_LSB       0x44
#define PIOS_L3GD20_GYRO_Y_OUT_MSB       0x45
#define PIOS_L3GD20_GYRO_Y_OUT_LSB       0x46
#define PIOS_L3GD20_GYRO_Z_OUT_MSB       0x47
#define PIOS_L3GD20_GYRO_Z_OUT_LSB       0x48
#define PIOS_L3GD20_USER_CTRL_REG        0x6A
#define PIOS_L3GD20_PWR_MGMT_REG         0x6B
#define PIOS_L3GD20_FIFO_CNT_MSB         0x72
#define PIOS_L3GD20_FIFO_CNT_LSB         0x73
#define PIOS_L3GD20_FIFO_REG             0x74
#define PIOS_L3GD20_WHOAMI               0x75

/* FIFO enable for storing different values */
#define PIOS_L3GD20_FIFO_TEMP_OUT        0x80
#define PIOS_L3GD20_FIFO_GYRO_X_OUT      0x40
#define PIOS_L3GD20_FIFO_GYRO_Y_OUT      0x20
#define PIOS_L3GD20_FIFO_GYRO_Z_OUT      0x10
#define PIOS_L3GD20_ACCEL_OUT            0x08

/* Interrupt Configuration */
#define PIOS_L3GD20_INT_ACTL             0x80
#define PIOS_L3GD20_INT_OPEN             0x40
#define PIOS_L3GD20_INT_LATCH_EN         0x20
#define PIOS_L3GD20_INT_CLR_ANYRD        0x10

#define PIOS_L3GD20_INTEN_OVERFLOW       0x10
#define PIOS_L3GD20_INTEN_DATA_RDY       0x01

/* Interrupt status */
#define PIOS_L3GD20_INT_STATUS_FIFO_FULL 0x80
#define PIOS_L3GD20_INT_STATUS_IMU_RDY   0X04
#define PIOS_L3GD20_INT_STATUS_DATA_RDY  0X01

/* User control functionality */
#define PIOS_L3GD20_USERCTL_FIFO_EN      0X40
#define PIOS_L3GD20_USERCTL_FIFO_RST     0X02
#define PIOS_L3GD20_USERCTL_GYRO_RST     0X01

/* Power management and clock selection */
#define PIOS_L3GD20_PWRMGMT_IMU_RST      0X80
#define PIOS_L3GD20_PWRMGMT_INTERN_CLK   0X00
#define PIOS_L3GD20_PWRMGMT_PLL_X_CLK    0X01
#define PIOS_L3GD20_PWRMGMT_PLL_Y_CLK    0X02
#define PIOS_L3GD20_PWRMGMT_PLL_Z_CLK    0X03
#define PIOS_L3GD20_PWRMGMT_STOP_CLK     0X07

enum pios_l3gd20_range {
	PIOS_L3GD20_SCALE_250_DEG  = 0x00,
	PIOS_L3GD20_SCALE_500_DEG  = 0x08,
	PIOS_L3GD20_SCALE_1000_DEG = 0x10,
	PIOS_L3GD20_SCALE_2000_DEG = 0x18
};

enum pios_l3gd20_filter {
	PIOS_L3GD20_LOWPASS_256_HZ = 0x00,
	PIOS_L3GD20_LOWPASS_188_HZ = 0x01,
	PIOS_L3GD20_LOWPASS_98_HZ  = 0x02,
	PIOS_L3GD20_LOWPASS_42_HZ  = 0x03,
	PIOS_L3GD20_LOWPASS_20_HZ  = 0x04,
	PIOS_L3GD20_LOWPASS_10_HZ  = 0x05,
	PIOS_L3GD20_LOWPASS_5_HZ   = 0x06
};

struct pios_l3gd20_data {
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
	int16_t temperature;
};

struct pios_l3gd20_cfg {
	struct stm32_gpio drdy;
	struct stm32_exti eoc_exti;
	struct stm32_irq eoc_irq;
	
	uint8_t Fifo_store;		/* FIFO storage of different readings (See datasheet page 31 for more details) */
	uint8_t Smpl_rate_div;	/* Sample rate divider to use (See datasheet page 32 for more details) */
	uint8_t interrupt_cfg;	/* Interrupt configuration (See datasheet page 35 for more details) */
	uint8_t interrupt_en;	/* Interrupt configuration (See datasheet page 35 for more details) */
	uint8_t User_ctl;		/* User control settings (See datasheet page 41 for more details)  */
	uint8_t Pwr_mgmt_clk;	/* Power management and clock selection (See datasheet page 32 for more details) */
	enum pios_l3gd20_range gyro_range;
	enum pios_l3gd20_filter filter;
};

/* Public Functions */
extern void PIOS_L3GD20_Init(const struct pios_l3gd20_cfg * cfg);
extern void PIOS_L3GD20_Attach(uint32_t spi_id);
extern int32_t PIOS_L3GD20_ReadFifo(struct pios_l3gd20_data * buffer);
extern int32_t PIOS_L3GD20_ReadGyros(struct pios_l3gd20_data * buffer);
extern int32_t PIOS_L3GD20_ReadID();
extern uint8_t PIOS_L3GD20_Test();
extern float PIOS_L3GD20_GetScale();

#endif /* PIOS_L3GD20_H */

/** 
  * @}
  * @}
  */
