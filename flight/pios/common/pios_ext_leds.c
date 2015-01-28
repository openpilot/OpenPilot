/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS External LEDs Functions
 * @{
 *
 * @file       pios_ext_leds.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
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

#include "pios_ext_leds.h"

enum pios_ext_leds_dev_magic {
    PIOS_ext_leds_DEV_MAGIC = 0x1ED51ED5,
};

struct pios_ext_leds_dev {
    enum pios_ext_leds_dev_magic magic;
    const struct pios_ext_leds_driver *driver;
};

static bool PIOS_ExtLeds_validate(struct pios_ext_leds_dev *ext_leds_dev)
{
    return ext_leds_dev->magic == PIOS_ext_leds_DEV_MAGIC;
}

static struct pios_ext_leds_dev *PIOS_ExtLeds_alloc(void)
{
    struct pios_ext_leds_dev *ext_leds_dev;

    ext_leds_dev = (struct pios_ext_leds_dev *)pios_malloc(sizeof(*ext_leds_dev));
    if (!ext_leds_dev) {
        return NULL;
    }

    ext_leds_dev->magic = PIOS_ext_leds_DEV_MAGIC;
    return ext_leds_dev;
}

/**
 * Initialize the External LEDs device.
 */
int32_t PIOS_ExtLeds_Init(uint32_t *ext_leds_id, const struct pios_ext_leds_driver *driver)
{
    PIOS_DEBUG_Assert(ext_leds_id);
    PIOS_DEBUG_Assert(driver);

    struct pios_ext_leds_dev *ext_leds_dev;

    ext_leds_dev = (struct pios_ext_leds_dev *)PIOS_ExtLeds_alloc();
    if (!ext_leds_dev) {
        goto out_fail;
    }

    ext_leds_dev->driver = driver;

    *ext_leds_id = (uint32_t)ext_leds_dev;
    return 0;

out_fail:
    return -1;
}

/**
 * Get the number of External LEDs connected to this device.
 */
uint8_t PIOS_ExtLeds_NumLeds(uint32_t ext_leds_id)
{
    PIOS_DEBUG_Assert(ext_leds_id);

    struct pios_ext_leds_dev *ext_leds_dev = (struct pios_ext_leds_dev *)ext_leds_id;

    if (!PIOS_ExtLeds_validate(ext_leds_dev)) {
        PIOS_Assert(0);
    }

    PIOS_DEBUG_Assert(ext_leds_dev->driver->NumLeds);

    return ext_leds_dev->driver->NumLeds();
}

/**
 * Set the color of the LED with the given index.
 */
int32_t PIOS_ExtLeds_SetColorRGB(uint32_t ext_leds_id, const Color_t color, uint8_t index, bool update)
{
    PIOS_DEBUG_Assert(ext_leds_id);

    struct pios_ext_leds_dev *ext_leds_dev = (struct pios_ext_leds_dev *)ext_leds_id;

    if (!PIOS_ExtLeds_validate(ext_leds_dev)) {
        PIOS_Assert(0);
    }

    PIOS_DEBUG_Assert(ext_leds_dev->driver->SetColorRGB);

    return ext_leds_dev->driver->SetColorRGB(color, index, update);
}

/**
 * Update the External LEDs.
 */
int32_t PIOS_ExtLeds_Update(uint32_t ext_leds_id)
{
    PIOS_DEBUG_Assert(ext_leds_id);

    struct pios_ext_leds_dev *ext_leds_dev = (struct pios_ext_leds_dev *)ext_leds_id;

    if (!PIOS_ExtLeds_validate(ext_leds_dev)) {
        PIOS_Assert(0);
    }

    PIOS_DEBUG_Assert(ext_leds_dev->driver->Update);

    return ext_leds_dev->driver->Update();
}
