/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_I2C_ESC Code for controlling I2C based ESCs
 * @brief Deals with the hardware interface to the magnetometers
 * @{
 *
 * @file       pios_i2c_esc.c
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_I2C_ESC)

/* HMC5843 Addresses */
#define MK_I2C_ADDR			        0x29
#define ASTEC4_I2C_ADDR                         0x02

bool PIOS_SetMKSpeed(uint8_t motornum, uint8_t speed);

uint8_t base_address = MK_I2C_ADDR;
uint8_t valid_motors = 0;
bool PIOS_I2C_ESC_Config()
{
	base_address = MK_I2C_ADDR;
	valid_motors = 0;
	for(uint8_t i = 0; i < 4; i ++)
		if(PIOS_SetMKSpeed(i, 0)) 
			valid_motors |= (1 << i);
		
	return true;
}

bool PIOS_I2C_ESC_SetSpeed(uint8_t speed[4])
{
	/*bool success = true;
	for(uint8_t i = 0; i < 4; i++) {
		//if(valid_motors & (1 << i))
			success &= PIOS_SetMKSpeed(i, speed[i]);
	}
	return success; */
	
	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = MK_I2C_ADDR + 0,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(speed[0]),
			.buf = &speed[0],
		} ,
		{
			.info = __func__,
			.addr = MK_I2C_ADDR + 1,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(speed[1]),
			.buf = &speed[1],
		}, 
		{
			.info = __func__,
			.addr = MK_I2C_ADDR + 2,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(speed[2]),
			.buf = &speed[2],
		},
		{
			.info = __func__,
			.addr = MK_I2C_ADDR + 3,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(speed[3]),
			.buf = &speed[3],
		} 
	};
	
	return PIOS_I2C_Transfer(PIOS_I2C_ESC_ADAPTER, txn_list, NELEMENTS(txn_list));	
}

bool PIOS_SetMKSpeed(uint8_t motornum, uint8_t speed) {
	static uint8_t speeds[4] = {0,0,0,0};
	
	if(motornum >= 4) 
		return false;
	
	if(speeds[motornum] == speed)
		return true;
	
	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = MK_I2C_ADDR + motornum,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(speed),
			.buf = &speed,
		} 
	};
	
	return PIOS_I2C_Transfer(PIOS_I2C_ESC_ADAPTER, txn_list, NELEMENTS(txn_list));
}

bool PIOS_SetAstec4Address(uint8_t new_address) {
	if((new_address < 0) || (new_address > 4)) 
		return false;
	
	uint8_t data[4] = {250, 0, new_address, 230+new_address};
	
	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = ASTEC4_I2C_ADDR,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(data),
			.buf = &data[0],
		} 
	};
	
	return PIOS_I2C_Transfer(PIOS_I2C_ESC_ADAPTER, txn_list, NELEMENTS(txn_list));		
}

bool PIOS_SetAstec4Speed(uint8_t motornum, uint8_t speed) {
	static uint8_t speeds[5] = {0,0,0,0};
	
	if((motornum < 0) || (motornum >= 4)) 
		return false;
	
	speeds[motornum] = speed;
	
	if(motornum != 3) 
		return true;
	
	/* Write in chunks of four */
	speeds[4] = 0xAA + speeds[0] + speeds[1] + speeds[2] + speeds[3];
	
	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = ASTEC4_I2C_ADDR,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(speeds),
			.buf = &speeds[0],
		} 
	};
	
	return PIOS_I2C_Transfer(PIOS_I2C_ESC_ADAPTER, txn_list, NELEMENTS(txn_list));	
}

#endif

/**
 * @}
 * @}
 */
