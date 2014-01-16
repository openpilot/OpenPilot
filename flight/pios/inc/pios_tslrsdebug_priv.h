/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_TSLRSdebug TSLRS debug functions
 * @brief PIOS interface to read TSLRS debug serial stream
 * @{
 *
 * @file       pios_tslrsdebug_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      TSLRS debug Private structures.
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

#ifndef PIOS_TSLRSDEBUG_PRIV_H
#define PIOS_TSLRSDEBUG_PRIV_H

#ifdef PIOS_INCLUDE_TSLRSDEBUG

#include <pios.h>
#include <pios_stm32.h>
#include <pios_usart_priv.h>

#define TSLRSDEBUG_BUFFER       10


/* TSLRSdebug instance configuration */
struct pios_tslrsdebug_cfg {
    struct stm32_gpio bind;
};

extern int32_t PIOS_TSLRSdebug_Init(uint32_t *tslrsdebug_id, const struct pios_tslrsdebug_cfg *cfg, const struct pios_com_driver *driver, uint32_t lower_id);

#endif /* PIOS_INCLUDE_TSLRSDEBUG */

#endif /* PIOS_TSLRSDEBUG_PRIV_H */

/**
 * @}
 * @}
 */
