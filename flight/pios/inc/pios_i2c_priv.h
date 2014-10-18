/**
 ******************************************************************************
 *
 * @file       pios_i2c_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      I2C private definitions.
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

#ifndef PIOS_I2C_PRIV_H
#define PIOS_I2C_PRIV_H

#include <pios.h>
#include <pios_stm32.h>
#include <stdbool.h>

struct pios_i2c_adapter_cfg {
    I2C_TypeDef       *regs;
    uint32_t          remap;
    I2C_InitTypeDef   init;

    uint32_t          transfer_timeout_ms;
    struct stm32_gpio scl;
    struct stm32_gpio sda;
    struct stm32_irq  event;
    struct stm32_irq  error;
};

enum pios_i2c_adapter_magic {
    PIOS_I2C_DEV_MAGIC = 0xa9a9b8b8,
};

struct pios_i2c_adapter {
    enum pios_i2c_adapter_magic magic;
    const struct pios_i2c_adapter_cfg *cfg;
#ifdef PIOS_INCLUDE_FREERTOS
    xSemaphoreHandle sem_busy;
    xSemaphoreHandle sem_ready;
#else
    uint8_t busy;
#endif

    /* variables for transfer timeouts */
    uint32_t transfer_delay_uS; // approx time to transfer one byte, calculated later basen on setting use here time based on 100 kbits/s
    uint32_t transfer_timeout_ticks; // take something tha makes sense for small transaction, calculated later based upon transmission desired

    bool     bus_error;
    bool     nack;

    volatile uint8_t curr_state;
    const struct pios_i2c_txn *first_txn;
    const struct pios_i2c_txn *active_txn;
    const struct pios_i2c_txn *last_txn;

    void    (*callback)();

    uint8_t *active_byte;
    uint8_t *last_byte;
};

int32_t PIOS_I2C_Init(uint32_t *i2c_id, const struct pios_i2c_adapter_cfg *cfg);

#endif /* PIOS_I2C_PRIV_H */
