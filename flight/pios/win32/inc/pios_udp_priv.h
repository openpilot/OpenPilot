/**
 ******************************************************************************
 *
 * @file       pios_udp_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      UDP private definitions.
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

#ifndef PIOS_UDP_PRIV_H
#define PIOS_UDP_PRIV_H

#include <pios.h>
#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>



struct pios_udp_cfg {
  const char * ip;
  uint16_t port;
};

struct pios_udp_buffer {
  uint8_t   buf[PIOS_UDP_RX_BUFFER_SIZE];
  uint16_t  head;
  uint16_t  tail;
  uint16_t  size;
};

struct pios_udp_dev {
  const struct pios_udp_cfg * const cfg;
  struct pios_udp_buffer      rx;
  SOCKET socket;
  struct sockaddr_in server;
  struct sockaddr_in client;
  uint32_t clientLength;
};

extern struct pios_udp_dev pios_udp_devs[];
extern uint8_t             pios_udp_num_devices;

#endif /* PIOS_UDP_PRIV_H */
