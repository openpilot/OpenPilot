/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MS4525DO MS4525DO Functions
 * @brief Hardware functions to deal with the PixHawk Airspeed Sensor based on MS4525DO
 * @{
 *
 * @file       pios_ms4525do.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      PixHawk MS4525DO Airspeed Sensor Driver
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************/
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


#include "pios.h"

#ifdef PIOS_INCLUDE_MS4525DO

/* Local Defs and Variables */


static int8_t PIOS_MS4525DO_ReadI2C(uint8_t *buffer, uint8_t len)
{
    const struct pios_i2c_txn txn_list[] = {
        {
            .info = __func__,
            .addr = MS4525DO_I2C_ADDR,
            .rw   = PIOS_I2C_TXN_READ,
            .len  = len,
            .buf  = buffer,
        }
    };

    return PIOS_I2C_Transfer(PIOS_I2C_MS4525DO_ADAPTER, txn_list, NELEMENTS(txn_list));
}


// values has to ba an arrray with two elements
// values stay untouched on error
int8_t PIOS_MS4525DO_Read(uint16_t *values)
{
    uint8_t data[4];
    int8_t retVal  = PIOS_MS4525DO_ReadI2C(data, sizeof(data));

    uint8_t status = data[0] & 0xC0;
    if (status == 0x80) {
        /* stale data */
        return -5;
    } else if (status == 0xC0) {
        /* device probably broken */
        return -6;
    }

    /* differential pressure */
    values[0]  = (data[0] << 8);
    values[0] += data[1];

    /* temperature */
    values[1]  = (data[2] << 8);
    values[1] += data[3];
    values[1]  = (values[1] >> 5);

    return retVal;
}

#endif /* PIOS_INCLUDE_MS4525DO */
