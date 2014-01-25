/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_PacketRxOk PacketRxOk  functions
 * @brief Code to read PacketRxOk
 * @{
 *
 * @file       pios_packetrxok.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Code to read PacketRxOk
 *             see http://code.google.com/p/minoposd/wiki/PacketRxOk
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

#ifdef PIOS_INCLUDE_PACKETRXOK

#include "pios_packetrxok.h"
#include "pios_packetrxok_priv.h"

#define MIN_READ_DELAY      100     // [ms]


GPIO_TypeDef* PacketRxOk_GPIO;
uint16_t PacketRxOk_GPIO_Pin;

volatile int packet_ok  = 0;
volatile int packet_nok = 0;

/* Forward Declarations */
static void PIOS_PacketRxOk_Supervisor(uint32_t pios_gpio_packetrxok_id);

// read the PacketRxOk percentage
uint8_t PacketRxOk_read(void)
{
    static portTickType last_read = 0;
    static float last_percent = 100.0f;

    portTickType current_time = xTaskGetTickCount();

    if (current_time >= last_read + MIN_READ_DELAY) {
        last_read = current_time;
        last_percent = (float)(10 * packet_ok / (packet_ok + packet_nok)) + 0.05f + last_percent * .9f;
        packet_ok = 0;
        packet_nok = 0;
    }
    return (uint8_t) last_percent;
}

/* Initialise PacketRxOk interface */
int32_t PIOS_PacketRxOk_Init(uint32_t *pios_packetrxok_id, uint32_t pios_gpio_packetrxok_id, GPIO_TypeDef* GPIO, uint16_t GPIO_Pin)
{
    *pios_packetrxok_id = (uint32_t)pios_gpio_packetrxok_id;
    PacketRxOk_GPIO = GPIO;
    PacketRxOk_GPIO_Pin = GPIO_Pin;

    if (!PIOS_RTC_RegisterTickCallback(PIOS_PacketRxOk_Supervisor, *pios_packetrxok_id)) {
        PIOS_DEBUG_Assert(0);
    }

    return 0;
}

/* Supervisor callback */
static void PIOS_PacketRxOk_Supervisor(__attribute__((unused)) uint32_t pios_gpio_packetrxok_id)
{
    if (GPIO_ReadInputDataBit(PacketRxOk_GPIO, PacketRxOk_GPIO_Pin) == Bit_SET) {
        packet_ok++;
    } else {
        packet_nok++;
    }
}

#endif /* PIOS_INCLUDE_PACKETRXOK */

/**
 * @}
 * @}
 */
