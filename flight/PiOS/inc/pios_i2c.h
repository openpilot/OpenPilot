/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_I2C I2C Functions
 * @{
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

#include <stdbool.h>

/* Global Types */
enum pios_i2c_txn_direction {
  PIOS_I2C_TXN_READ,
  PIOS_I2C_TXN_WRITE
};

struct pios_i2c_txn {
  uint16_t                      addr;
  enum pios_i2c_txn_direction   rw;
  uint32_t                      len;
  uint8_t                     * buf;
};

/* Public Functions */
extern int32_t PIOS_I2C_Init(void);
extern bool PIOS_I2C_Transfer(uint8_t i2c, const struct pios_i2c_txn txn_list[], uint32_t num_txns);
extern void PIOS_I2C_EV_IRQ_Handler(uint8_t i2c);
extern void PIOS_I2C_ER_IRQ_Handler(uint8_t i2c);

#endif /* PIOS_I2C_H */

/**
  * @}
  * @}
  */
