/**
 ******************************************************************************
 *
 * @file       pios_apa102.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      A driver for apa102 rgb led controller.
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
#include <pios.h>

#ifdef PIOS_INCLUDE_APA102

#include "pios_apa102.h"

#include <pios_ext_leds.h>

enum pios_apa102_dev_magic {
    PIOS_APA102_DEV_MAGIC = 0xcfb93755,
};

static Color_t led_array[PIOS_APA102_NUMLEDS] = {};


struct apa102_dev {
    uint32_t spi_id;
    uint32_t slave_num;
    enum pios_apa102_dev_magic magic;
};

// ! Global structure for this device device
static struct apa102_dev *dev;

// ! Private functions
static struct apa102_dev *PIOS_APA102_alloc(void);
static int32_t PIOS_APA102_SetColorRGB(Color_t c, uint8_t led, bool update);
static int32_t PIOS_APA102_Update();
static int32_t PIOS_APA102_Validate(struct apa102_dev *dev);
static int32_t PIOS_APA102_ClaimBus();
// static int32_t PIOS_APA102_ClaimBusISR(bool *woken);
static int32_t PIOS_APA102_ReleaseBus();
// static int32_t PIOS_APA102_ReleaseBusISR(bool *woken);

/**
 * @brief Allocate a new device
 */
static struct apa102_dev *PIOS_APA102_alloc(void)
{
    struct apa102_dev *apa102_dev;

    apa102_dev = (struct apa102_dev *)pios_malloc(sizeof(*apa102_dev));
    if (!apa102_dev) {
        return NULL;
    }

    apa102_dev->magic = PIOS_APA102_DEV_MAGIC;
    return apa102_dev;
}

/**
 * @brief Validate the handle to the spi device
 * @returns 0 for valid device or -1 otherwise
 */
static int32_t PIOS_APA102_Validate(struct apa102_dev *vdev)
{
    if (vdev == NULL) {
        return -1;
    }
    if (vdev->magic != PIOS_APA102_DEV_MAGIC) {
        return -2;
    }
    if (vdev->spi_id == 0) {
        return -3;
    }
    return 0;
}


int32_t PIOS_APA102_Init(uint32_t spi_id, uint32_t slave_num)
{
    dev = PIOS_APA102_alloc();
    if (dev == NULL) {
        return -1;
    }

    dev->spi_id    = spi_id;
    dev->slave_num = slave_num;

    const Color_t ledoff = Color_Blue;
    for (uint8_t i = 0; i < PIOS_APA102_NUMLEDS; i++) {
        PIOS_APA102_SetColorRGB(ledoff, i, false);
    }

    return 0;
}

/**
 * Set a led color
 * @param c color
 * @param led led number
 * @param update Perform an update after changing led color
 */
static int32_t PIOS_APA102_SetColorRGB(Color_t c, uint8_t led, bool update)
{
    if (led >= PIOS_APA102_NUMLEDS) {
        return -1;
    }

    led_array[led].R = c.R;
    led_array[led].G = c.G;
    led_array[led].B = c.B;

    if (update) {
        return PIOS_APA102_Update();
    }

    return 0;
}


static void request_callback(bool arg0, uint8_t arg1)
{
    (void)arg0;
    (void)arg1;
}

/**
 * trigger an update cycle if not already running
 */
static int32_t PIOS_APA102_Update()
{
    if (PIOS_APA102_ClaimBus() != 0) {
        return -1;
    }

    // NOTE: The datasheet says that the four first bytes should be all 0 and
    // that the led sequence should be terminated with four 0xFF bytes.
    // However, this doesn't work. The last four bytes will turn on the following led, and the sequence will continue until the next four 0 bytes are seen.
    // It seems to be much more reliable to send four 0 bytes both at the
    // beginning and at the end of the led sequence.

#define LED    4
#define PRE    4
#define POST   4
#define LENGTH (PRE + PIOS_APA102_NUMLEDS * LED + POST)

    // Pre and Post is pre-set to 0.
    static uint8_t buffer[LENGTH] = { 0, };

    for (int i = 0; i < PIOS_APA102_NUMLEDS; i++) {
#define START      0b11100000
#define BRIGHTNESS 0b00011111
        buffer[PRE + i * LED + 0] = START | BRIGHTNESS;
        buffer[PRE + i * LED + 1] = led_array[i].B;
        buffer[PRE + i * LED + 2] = led_array[i].G;
        buffer[PRE + i * LED + 3] = led_array[i].R;
    }

    uint32_t ret = PIOS_SPI_TransferBlock(dev->spi_id, buffer, NULL, LENGTH, request_callback);
    PIOS_Assert(ret == 0);

    PIOS_APA102_ReleaseBus();

    return ret;
}

static uint8_t PIOS_APA102_NumLeds()
{
    return PIOS_APA102_NUMLEDS;
}

const struct pios_ext_leds_driver *PIOS_APA102_Driver()
{
    static const struct pios_ext_leds_driver pios_apa102_driver = {
        .NumLeds     = PIOS_APA102_NumLeds,
        .SetColorRGB = PIOS_APA102_SetColorRGB,
        .Update      = PIOS_APA102_Update
    };

    return &pios_apa102_driver;
}


/**
 * @brief Claim the SPI bus for the accel communications and select this chip
 * @return 0 if successful, -1 if unable to claim bus
 */
static int32_t PIOS_APA102_ClaimBus()
{
    if (PIOS_APA102_Validate(dev) != 0) {
        return -1;
    }

    if (PIOS_SPI_ClaimBus(dev->spi_id) != 0) {
        return -1;
    }

    PIOS_SPI_RC_PinSet(dev->spi_id, dev->slave_num, 0);

    return 0;
}

#if 0
/**
 * @brief Claim the SPI bus for the accel communications and select this chip. Call from an ISR.
 * @param woken[in,out] If non-NULL, will be set to true if woken was false and a higher priority
 *                      task has is now eligible to run, else unchanged
 * @return 0 if successful, -1 if unable to claim bus
 */
static int32_t PIOS_APA102_ClaimBusISR(bool *woken)
{
    if (PIOS_APA102_Validate(dev) != 0) {
        return -1;
    }

    if (PIOS_SPI_ClaimBusISR(dev->spi_id, woken) != 0) {
        return -1;
    }

    PIOS_SPI_RC_PinSet(dev->spi_id, dev->slave_num, 0);
    return 0;
}
#endif

/**
 * @brief Release the SPI bus for the accel communications and end the transaction
 * @return 0 if successful
 */
static int32_t PIOS_APA102_ReleaseBus()
{
    if (PIOS_APA102_Validate(dev) != 0) {
        return -1;
    }
    PIOS_SPI_RC_PinSet(dev->spi_id, dev->slave_num, 1);

    return PIOS_SPI_ReleaseBus(dev->spi_id);
}

#if 0
/**
 * @brief Release the SPI bus for the accel communications and end the transaction. Call from an ISR
 * @param woken[in,out] If non-NULL, will be set to true if woken was false and a higher priority
 *                      task has is now eligible to run, else unchanged
 * @return 0 if successful
 */
static int32_t PIOS_APA102_ReleaseBusISR(bool *woken)
{
    if (PIOS_APA102_Validate(dev) != 0) {
        return -1;
    }

    PIOS_SPI_RC_PinSet(dev->spi_id, dev->slave_num, 1);

    return PIOS_SPI_ReleaseBusISR(dev->spi_id, woken);
}
#endif

#endif // PIOS_INCLUDE_APA102
