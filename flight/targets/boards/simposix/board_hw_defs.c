/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 *
 * @file       board_hw_defs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Defines board specific static initializers for hardware for the OpenPilot board.
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

#ifdef PIOS_INCLUDE_UDP

#include <pios_udp_priv.h>

#ifdef PIOS_INCLUDE_COM_TELEM
/*
 * Telemetry on main USART
 */
const struct pios_udp_cfg pios_udp_telem_cfg = {
    .ip   = "0.0.0.0",
    .port = 9000,
};
#endif /* PIOS_COM_TELEM */

#ifdef PIOS_INCLUDE_GPS
/*
 * GPS USART
 */
const struct pios_udp_cfg pios_udp_gps_cfg = {
    .ip   = "0.0.0.0",
    .port = 9001,
};

#endif /* PIOS_INCLUDE_GPS */

#ifdef PIOS_INCLUDE_COM_AUX
/*
 * AUX USART (UART label on rev2)
 */
const struct pios_udp_cfg pios_udp_aux_cfg = {
    .ip   = "0.0.0.0",
    .port = 9002,
};
#endif /* PIOS_COM_AUX */

#endif /* PIOS_UDP */

#if defined(PIOS_INCLUDE_COM)

#include <pios_com_priv.h>

#endif /* PIOS_INCLUDE_COM */


#if defined(PIOS_INCLUDE_FLASH)
#include "pios_flashfs_logfs_priv.h"
#include "pios_flash_internal_priv.h"

#if defined(PIOS_YAFFS)
static const struct flashfs_logfs_cfg flashfs_yaffs_norsim_cfg = {
    .fs_magic      = 0x99abceff,
    .total_fs_size = 0x00200000, /* 2M bytes (32 sectors = entire chip) */
    .arena_size    = 0x0000FE00, /* multiple of slot size but less than sector_size */
    .slot_size     = 0x00000200, /* 512 bytes chunk size for yaffs*/

    .start_offset  = 0x00000000, /* start offset */
    .sector_size   = 0x00010000, /* 64K bytes */
    .page_size     = 0x00000100, /* 256 bytes */
};
#endif

#endif

