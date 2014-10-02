/**
 ******************************************************************************
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Defines board specific static initializers for hardware for the GPS board.
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
#include <pios_board_info.h>

/*
 * Pull in the board-specific static HW definitions.
 * Including .c files is a bit ugly but this allows all of
 * the HW definitions to be const and static to limit their
 * scope.
 *
 * NOTE: THIS IS THE ONLY PLACE THAT SHOULD EVER INCLUDE THIS FILE
 */
#include "../board_hw_defs.c"
#include <pios_com_msg.h>
uint32_t PIOS_COM_TELEM_USB;
#define PIOS_COM_MAIN_RX_BUF_LEN 256
#define PIOS_COM_MAIN_TX_BUF_LEN 256
uint8_t rx_buffer[PIOS_COM_MAIN_RX_BUF_LEN];
uint8_t tx_buffer[PIOS_COM_MAIN_TX_BUF_LEN];


static void setupCom();
/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void)
{
    const struct pios_board_info *bdinfo = &pios_board_info_blob;

    FLASH_PrefetchBufferCmd(ENABLE);

#if defined(PIOS_INCLUDE_LED)
    const struct pios_gpio_cfg *led_cfg = PIOS_BOARD_HW_DEFS_GetLedCfg(bdinfo->board_rev);
    PIOS_Assert(led_cfg);
    PIOS_LED_Init(led_cfg);
#endif /* PIOS_INCLUDE_LED */
    setupCom();
}

void setupCom()
{
    uint32_t pios_usart_generic_id;

    if (PIOS_USART_Init(&pios_usart_generic_id, &pios_usart_generic_main_cfg)) {
        PIOS_Assert(0);
    }
    if (PIOS_COM_Init(&PIOS_COM_TELEM_USB, &pios_usart_com_driver, pios_usart_generic_id,
                      rx_buffer, PIOS_COM_MAIN_RX_BUF_LEN,
                      tx_buffer, PIOS_COM_MAIN_TX_BUF_LEN)) {
        PIOS_Assert(0);
    }
}
