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

/* L3GD20 Addresses */
#define PIOS_L3GD20_WHOAMI               0x0F
#define PIOS_L3GD20_CTRL_REG1            0X20
#define PIOS_L3GD20_CTRL_REG2            0X21
#define PIOS_L3GD20_CTRL_REG3            0X22
#define PIOS_L3GD20_CTRL_REG4            0X23
#define PIOS_L3GD20_CTRL_REG5            0X24
#define PIOS_L3GD20_REFERENCE            0X25
#define PIOS_L3GD20_OUT_TEMP             0x26
#define PIOS_L3GD20_STATUS_REG           0x27
#define PIOS_L3GD20_GYRO_X_OUT_LSB       0x28
#define PIOS_L3GD20_GYRO_X_OUT_MSB       0x29
#define PIOS_L3GD20_GYRO_Y_OUT_LSB       0x2A
#define PIOS_L3GD20_GYRO_Y_OUT_MSB       0x2B
#define PIOS_L3GD20_GYRO_Z_OUT_LSB       0x2C
#define PIOS_L3GD20_GYRO_Z_OUT_MSB       0x2D
#define PIOS_L3GD20_FIFO_CTRL_REG        0x2E
#define PIOS_L3GD20_FIFO_SRC_REG         0x2F
#define PIOS_L3GD20_INT1_CFG             0x30
#define PIOS_L3GD20_INT1_SRC             0x31
#define PIOS_L3GD20_INT1_TSH_XH          0x32
#define PIOS_L3GD20_INT1_TSH_XL          0x33
#define PIOS_L3GD20_INT1_TSH_YH          0x34
#define PIOS_L3GD20_INT1_TSH_YL          0x35
#define PIOS_L3GD20_INT1_TSH_ZH          0x36
#define PIOS_L3GD20_INT1_TSH_ZL          0x37
#define PIOS_L3GD20_INT1_DURATION        0x38

/* Ctrl1 flags */
#define PIOS_L3GD20_CTRL1_FASTEST        0xF0
#define PIOS_L3GD20_CTRL1_380HZ_100HZ    0xB0
#define PIOS_L3GD20_CTRL1_PD             0x08
#define PIOS_L3GD20_CTRL1_ZEN            0x04
#define PIOS_L3GD20_CTRL1_YEN            0x02
#define PIOS_L3GD20_CTRL1_XEN            0x01

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
	PIOS_L3GD20_SCALE_500_DEG  = 0x10,
	PIOS_L3GD20_SCALE_2000_DEG = 0x3
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
	const struct pios_exti_cfg * exti_cfg; /* Pointer to the EXTI configuration */

	enum pios_l3gd20_range range;
};

/* Public Functions */
extern int32_t PIOS_L3GD20_Init(uint32_t spi_id, uint32_t slave_num, const struct pios_l3gd20_cfg * cfg);
extern xQueueHandle PIOS_L3GD20_GetQueue();
extern int32_t PIOS_L3GD20_ReadGyros(struct pios_l3gd20_data * buffer);
extern int32_t PIOS_L3GD20_SetRange(enum pios_l3gd20_range range);
extern float PIOS_L3GD20_GetScale();
extern int32_t PIOS_L3GD20_ReadID();
extern uint8_t PIOS_L3GD20_Test();
bool void PIOS_L3GD20_IRQHandler();

#endif /* PIOS_L3GD20_H */

/** 
  * @}
  * @}
  */
