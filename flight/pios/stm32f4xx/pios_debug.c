/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @defgroup   PIOS_DEBUG Debugging Functions
 * @brief Debugging functionality
 * @{
 *
 * @file       pios_debug.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Debugging Functions
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

// Global variables
const char *PIOS_DEBUG_AssertMsg = "ASSERT FAILED";

#ifdef PIOS_ENABLE_DEBUG_PINS
static const struct pios_tim_channel *debug_channels;
static uint8_t debug_num_channels;
#endif /* PIOS_ENABLE_DEBUG_PINS */

/**
 * Initialise Debug-features
 */
void PIOS_DEBUG_Init(__attribute__((unused)) const struct pios_tim_channel *channels, __attribute__((unused)) uint8_t num_channels)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
    PIOS_Assert(channels);
    PIOS_Assert(num_channels);

    /* Store away the GPIOs we've been given */
    debug_channels     = channels;
    debug_num_channels = num_channels;

    /* Configure the GPIOs we've been given */
    for (uint8_t i = 0; i < num_channels; i++) {
        const struct pios_tim_channel *chan = &channels[i];

        // Initialise pins as standard output pins
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_StructInit(&GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin   = chan->pin.init.GPIO_Pin;

        /* Initialize the GPIO */
        GPIO_Init(chan->pin.gpio, &GPIO_InitStructure);

        /* Set the pin low */
        GPIO_WriteBit(chan->pin.gpio, chan->pin.init.GPIO_Pin, Bit_RESET);
    }
#endif // PIOS_ENABLE_DEBUG_PINS
}

/**
 * Set debug-pin high
 * \param pin 0 for S1 output
 */
void PIOS_DEBUG_PinHigh(__attribute__((unused)) uint8_t pin)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
    if (!debug_channels || pin >= debug_num_channels) {
        return;
    }

    const struct pios_tim_channel *chan = &debug_channels[pin];

    GPIO_WriteBit(chan->pin.gpio, chan->pin.init.GPIO_Pin, Bit_SET);

#endif // PIOS_ENABLE_DEBUG_PINS
}

/**
 * Set debug-pin low
 * \param pin 0 for S1 output
 */
void PIOS_DEBUG_PinLow(__attribute__((unused)) uint8_t pin)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
    if (!debug_channels || pin >= debug_num_channels) {
        return;
    }

    const struct pios_tim_channel *chan = &debug_channels[pin];

    GPIO_WriteBit(chan->pin.gpio, chan->pin.init.GPIO_Pin, Bit_RESET);

#endif // PIOS_ENABLE_DEBUG_PINS
}

/**
 * Report a serious error and halt
 */
void PIOS_DEBUG_Panic(__attribute__((unused)) const char *msg)
{
#ifdef PIOS_INCLUDE_DEBUG_CONSOLE
    register int *lr asm ("lr"); // Link-register holds the PC of the caller
    DEBUG_PRINTF(0, "\r%s @0x%x\r", msg, lr);
#endif

    // Stay put
    while (1) {
        ;
    }
}

/**
 * @}
 * @}
 */
