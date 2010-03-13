/**
 ******************************************************************************
 *
 * @file       pios_i2c.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      I2C functions header.
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

#ifndef PIOS_I2C_H
#define PIOS_I2C_H

/* Local defines */
#define I2C_ERROR_INVALID_PORT				-1
#define I2C_ERROR_GENERAL				-2
#define I2C_ERROR_UNSUPPORTED_TRANSFER_TYPE		-3
#define I2C_ERROR_TIMEOUT				-4
#define I2C_ERROR_ARBITRATION_LOST			-5
#define I2C_ERROR_BUS					-6
#define I2C_ERROR_SLAVE_NOT_CONNECTED			-7
#define I2C_ERROR_UNEXPECTED_EVENT			-8
#define I2C_ERROR_RX_BUFFER_OVERRUN			-9
#define I2C_ERROR_BUSY						-10


/* Global Types */
typedef enum {
	I2C_Blocking,
	I2C_Non_Blocking
} I2CSemaphoreTypeDef;

typedef enum {
	I2C_Read,
	I2C_Write,
	I2C_Write_WithoutStop
} I2CTransferTypeDef;

/* Public Functions */
extern int32_t PIOS_I2C_Init(void);
extern int32_t PIOS_I2C_LockDevice(I2CSemaphoreTypeDef semaphore_type);
extern int32_t PIOS_I2C_UnlockDevice(void);
extern int32_t PIOS_I2C_TransferCheck(void);
extern int32_t PIOS_I2C_TransferWait(void);
extern int32_t PIOS_I2C_Transfer(I2CTransferTypeDef transfer, uint8_t address, uint8_t *buffer, uint16_t len);
extern void PIOS_I2C_StartTransfer(I2CTransferTypeDef transfer, uint8_t address, uint8_t *buffer, uint16_t len);

#endif /* PIOS_I2C_H */
