/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_I2C_ESC Code for controlling I2C based ESCs
 * @brief Deals with the hardware interface to the magnetometers
 * @{
 *
 * @file       pios_i2c_esc.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      HMC5843 Magnetic Sensor Functions from AHRS
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

#ifndef PIOS_I2C_ESC_H
#define PIOS_I2C_ESC_H

/* Public Functions */
#include <stdbool.h>
#include <stdint.h>

bool PIOS_I2C_ESC_Config();
bool PIOS_I2C_ESC_SetSpeed(uint8_t speed[4]);
bool PIOS_SetMKSpeed(uint8_t motornum, uint8_t speed);
bool PIOS_SetAstec4Speed(uint8_t motornum, uint8_t speed);

#endif /* PIOS_I2C_ESC_H */

/** 
 * @}
 * @}
 */
