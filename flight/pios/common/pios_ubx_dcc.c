/**
 ******************************************************************************
 *
 * @file       pios_ubx_dcc.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      PIOS UBlox I2C(DDC) driver
 *             --
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

#include <stdint.h>
#include <pios_i2c.h>
#include <pios_helpers.h>
#define GPS_I2C_ADDRESS              (0x42 << 1)
#define GPS_I2C_STREAM_REG           0xFF
#define GPS_I2C_STREAM_SIZE_HIGH_REG 0xFD
#define GPS_I2C_STREAM_SIZE_LOW_REG  0xFE

int32_t PIOS_UBX_DDC_GetAvailableBytes(uint32_t i2c_id)
{
    uint8_t tmp[2];
    const uint8_t addr_buffer[1]   = { GPS_I2C_STREAM_SIZE_HIGH_REG };
    struct pios_i2c_txn txn_list[] = {
        {
            .info = __func__,
            .addr = GPS_I2C_ADDRESS,
            .rw   = PIOS_I2C_TXN_WRITE,
            .len  = 1,
            .buf  = (uint8_t *)addr_buffer,
        }
        ,
        {
            .info = __func__,
            .addr = GPS_I2C_ADDRESS,
            .rw   = PIOS_I2C_TXN_READ,
            .len  = 2,
            .buf  = tmp,
        }
    };

    if (PIOS_I2C_Transfer(i2c_id, txn_list, NELEMENTS(txn_list)) != 0) {
        return -1;
    }
    return (tmp[0] << 8) | tmp[1];
}

int32_t PIOS_UBX_DDC_ReadData(uint32_t i2c_id, uint8_t *buffer, uint8_t size)
{
    const uint8_t addr_buffer[1] = { GPS_I2C_STREAM_REG };
    const struct pios_i2c_txn txn_list[] = {
        {
            .info = __func__,
            .addr = GPS_I2C_ADDRESS,
            .rw   = PIOS_I2C_TXN_WRITE,
            .len  = 1,
            .buf  = (uint8_t *)addr_buffer,
        }
        ,
        {
            .info = __func__,
            .addr = GPS_I2C_ADDRESS,
            .rw   = PIOS_I2C_TXN_READ,
            .len  = size,
            .buf  = buffer,
        }
    };

    if (PIOS_I2C_Transfer(i2c_id, txn_list, NELEMENTS(txn_list)) != 0) {
        return -1;
    }
    return 0;
}

int32_t PIOS_UBX_DDC_WriteData(const uint32_t i2c_id, const uint8_t *buffer, const uint8_t size)
{
    const uint8_t addr_buffer[1] = { GPS_I2C_STREAM_REG };
    const struct pios_i2c_txn txn_list[] = {
        {
            .info = __func__,
            .addr = GPS_I2C_ADDRESS,
            .rw   = PIOS_I2C_TXN_WRITE,
            .len  = 1,
            .buf  = (uint8_t *)addr_buffer,
        }
        ,
        {
            .info = __func__,
            .addr = GPS_I2C_ADDRESS,
            .rw   = PIOS_I2C_TXN_WRITE,
            .len  = size,
            .buf  = (uint8_t *)buffer,
        }
    };

    if (PIOS_I2C_Transfer(i2c_id, txn_list, NELEMENTS(txn_list)) != 0) {
        return -1;
    }
    return 0;
}
