/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_GPIO GPIO Functions
 * @brief PIOS interface for GPIOss
 * @{
 *
 * @file       pios_gpio_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      GPIO private definitions.
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

#ifndef PIOS_GPIO_PRIV_H
#define PIOS_GPIO_PRIV_H

#include <pios.h>
#include <pios_stm32.h>

struct pios_gpio {
    struct stm32_gpio pin;
    uint32_t remap;
    bool active_low;
};

struct pios_gpio_cfg {
    const struct pios_gpio *gpios;
    uint8_t num_gpios;
};

extern int32_t PIOS_GPIO_Init(uint32_t *gpios_dev_id, const struct pios_gpio_cfg *cfg);

#endif /* PIOS_GPIO_PRIV_H */

/**
 * @}
 * @}
 */
