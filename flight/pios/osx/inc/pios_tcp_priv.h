/**
 ******************************************************************************
 *
 * @file       pios_tcp_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      TCP private definitions.
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

#ifndef PIOS_TCP_PRIV_H
#define PIOS_TCP_PRIV_H

#include <pios.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include "fifo_buffer.h"

struct pios_tcp_cfg {
	const char *ip;
	uint16_t port;
};

typedef struct {
	const struct pios_tcp_cfg * cfg;
	pthread_t rxThread;
	
	int socket;
	struct sockaddr_in server;
	struct sockaddr_in client;
	uint32_t clientLength;
	int socket_connection;
	
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	
	pios_com_callback tx_out_cb;
	uint32_t tx_out_context;
	pios_com_callback rx_in_cb;
	uint32_t rx_in_context;
	
	t_fifo_buffer rx_fifo;
	uint8_t rx_buffer[PIOS_TCP_RX_BUFFER_SIZE];
	uint8_t tx_buffer[PIOS_TCP_RX_BUFFER_SIZE];
} pios_tcp_dev;

extern int32_t PIOS_TCP_Init(uint32_t *tcp_id, const struct pios_tcp_cfg *cfg);

#endif /* PIOS_TCP_PRIV_H */
