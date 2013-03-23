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
#include <pios_config.h>

#ifdef PIOS_INCLUDE_UDP

#include <pios_udp_priv.h>

#ifdef PIOS_INCLUDE_COM_TELEM
/*
 * Telemetry on main USART
 */
const struct pios_udp_cfg pios_udp_telem_cfg = {
  .ip = "0.0.0.0",
  .port = 9000,
};
#endif /* PIOS_COM_TELEM */

#ifdef PIOS_INCLUDE_GPS
/*
 * GPS USART
 */
const struct pios_udp_cfg pios_udp_gps_cfg = {
  .ip = "0.0.0.0",
  .port = 9001,
};

#endif /* PIOS_INCLUDE_GPS */

#ifdef PIOS_INCLUDE_COM_AUX
/*
 * AUX USART (UART label on rev2)
 */
const struct pios_udp_cfg pios_udp_aux_cfg = {
  .ip = "0.0.0.0",
  .port = 9002,
};
#endif /* PIOS_COM_AUX */

#endif /* PIOS_UDP */

#if defined(PIOS_INCLUDE_COM)

#include <pios_com_priv.h>

#endif /* PIOS_INCLUDE_COM */

