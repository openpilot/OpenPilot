/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_I2C I2C Functions
 * @{
 *
 * @file       pios_i2c.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#define PIOS_I2C_DIAGNOSTICS

#include <stdbool.h>

/* Global Types */
enum pios_i2c_txn_direction {
	PIOS_I2C_TXN_READ,
	PIOS_I2C_TXN_WRITE
};

struct pios_i2c_txn {
	const char *info;
	uint16_t addr;
	enum pios_i2c_txn_direction rw;
	uint32_t len;
	uint8_t *buf;
};

#define I2C_LOG_DEPTH 20
enum pios_i2c_error_type {
	PIOS_I2C_ERROR_EVENT, 
	PIOS_I2C_ERROR_FSM,
	PIOS_I2C_ERROR_INTERRUPT
};

struct pios_i2c_fault_history {
	enum pios_i2c_error_type type;
	uint32_t evirq[I2C_LOG_DEPTH];
	uint32_t erirq[I2C_LOG_DEPTH];
	uint8_t event[I2C_LOG_DEPTH];
	uint8_t state[I2C_LOG_DEPTH];
};

/* Public Functions */
extern int32_t PIOS_I2C_Transfer(uint32_t i2c_id, const struct pios_i2c_txn txn_list[], uint32_t num_txns);
extern int32_t PIOS_I2C_Transfer_Callback(uint32_t i2c_id, const struct pios_i2c_txn txn_list[], uint32_t num_txns, void *callback);
extern void PIOS_I2C_EV_IRQ_Handler(uint32_t i2c_id);
extern void PIOS_I2C_ER_IRQ_Handler(uint32_t i2c_id);
extern void PIOS_I2C_GetDiagnostics(struct pios_i2c_fault_history * data, uint8_t * error_counts);

#endif /* PIOS_I2C_H */

/**
  * @}
  * @}
  */
