/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_TSLRSdebug TSLRS debug functions
 * @brief Code to read TSLRS debug serial stream
 * @{
 *
 * @file       pios_tslrsdebug.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Code to read TSLRS debug serial stream
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

#include "pios.h"

#ifdef PIOS_INCLUDE_TSLRSDEBUG

#include "pios_tslrsdebug.h"
#include "pios_tslrsdebug_priv.h"

struct tslrs tslrs_debug = {
        .data = 0,
};

/* Forward Declarations */
static uint16_t PIOS_TSLRSdebug_RxInCallback(uint32_t context, uint8_t *buf, uint16_t buf_len, uint16_t *headroom, bool *need_yield);

enum pios_tslrsdebug_dev_magic {
    PIOS_TSLRSDEBUG_DEV_MAGIC = 0x42537357,
};

struct pios_tslrsdebug_state {
    uint8_t  received_data[TSLRSDEBUG_BUFFER];
    uint8_t  byte_count;
};

struct pios_tslrsdebug_dev {
    enum pios_tslrsdebug_dev_magic   magic;
    const struct pios_tslrsdebug_cfg *cfg;
    struct pios_tslrsdebug_state     state;
};

/* Allocate device descriptor */
#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_tslrsdebug_dev *PIOS_TSLRSdebug_Alloc(void)
{
    struct pios_tslrsdebug_dev *tslrsdebug_dev;

    tslrsdebug_dev = (struct pios_tslrsdebug_dev *)pvPortMalloc(sizeof(*tslrsdebug_dev));
    if (!tslrsdebug_dev) {
        return NULL;
    }

    tslrsdebug_dev->magic = PIOS_TSLRSDEBUG_DEV_MAGIC;
    return tslrsdebug_dev;
}
#else
static struct pios_tslrsdebug_dev pios_tslrsdebug_devs[1];
static uint8_t pios_tslrsdebug_num_devs;
static struct pios_tslrsdebug_dev *PIOS_TSLRSdebug_Alloc(void)
{
    struct pios_tslrsdebug_dev *tslrsdebug_dev;

    if (pios_tslrsdebug_num_devs >= 1) {
        return NULL;
    }

    tslrsdebug_dev = &pios_tslrsdebug_devs[pios_tslrsdebug_num_devs++];
    tslrsdebug_dev->magic = PIOS_TSLRSDEBUG_DEV_MAGIC;

    return tslrsdebug_dev;
}
#endif /* if defined(PIOS_INCLUDE_FREERTOS) */

/* Validate device descriptor */
static bool PIOS_TSLRSdebug_Validate(struct pios_tslrsdebug_dev *tslrsdebug_dev)
{
    return tslrsdebug_dev->magic == PIOS_TSLRSDEBUG_DEV_MAGIC;
}

/* Reset receiver state */
static void PIOS_TSLRSdebug_ResetState(struct pios_tslrsdebug_state *state)
{
    state->byte_count = 1;
}

/* Initialise TSLRSdebug interface */
int32_t PIOS_TSLRSdebug_Init(uint32_t *tslrsdebug_id, const struct pios_tslrsdebug_cfg *cfg, const struct pios_com_driver *driver, uint32_t lower_id)
{
    PIOS_DEBUG_Assert(tslrsdebug_id);
    PIOS_DEBUG_Assert(cfg);
    PIOS_DEBUG_Assert(driver);

    struct pios_tslrsdebug_dev *tslrsdebug_dev;

    tslrsdebug_dev = (struct pios_tslrsdebug_dev *)PIOS_TSLRSdebug_Alloc();
    if (!tslrsdebug_dev) {
        goto out_fail;
    }

    /* Bind the configuration to the device instance */
    tslrsdebug_dev->cfg = cfg;

    PIOS_TSLRSdebug_ResetState(&(tslrsdebug_dev->state));

    *tslrsdebug_id = (uint32_t)tslrsdebug_dev;

    /* Set comm driver callback */
    (driver->bind_rx_cb)(lower_id, PIOS_TSLRSdebug_RxInCallback, *tslrsdebug_id);

    tslrs_debug.data = tslrsdebug_dev->state.received_data;

    return 0;
out_fail:
    return -1;
}

/* Processing byte from the TSLRS debug stream */
static void PIOS_TSLRSdebug_UpdateState(struct pios_tslrsdebug_state *state, __attribute__((unused)) uint8_t b)
{
    state->received_data[state->byte_count - 1] = b;
    if (state->byte_count++ >= TSLRSDEBUG_BUFFER)
        state->byte_count     = 1;
}

/* Comm byte received callback */
static uint16_t PIOS_TSLRSdebug_RxInCallback(uint32_t context, uint8_t *buf, uint16_t buf_len, uint16_t *headroom, bool *need_yield)
{
    struct pios_tslrsdebug_dev *tslrsdebug_dev = (struct pios_tslrsdebug_dev *)context;

    bool valid = PIOS_TSLRSdebug_Validate(tslrsdebug_dev);

    PIOS_Assert(valid);

    struct pios_tslrsdebug_state *state = &(tslrsdebug_dev->state);

    /* process byte(s) and clear receive timer */
    for (uint8_t i = 0; i < buf_len; i++) {
        PIOS_TSLRSdebug_UpdateState(state, buf[i]);
    }

    /* Always signal that we can accept another byte */
    if (headroom) {
        *headroom = 1;
    }

    /* We never need a yield */
    *need_yield = false;

    /* Always indicate that all bytes were consumed */
    return buf_len;
}

#endif /* PIOS_INCLUDE_TSLRSDEBUG */

/**
 * @}
 * @}
 */
