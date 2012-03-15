/**
 ******************************************************************************
 *
 * @file       pios_tcp.c   
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      TCP commands. Inits UDPs, controls UDPs & Interupt handlers.
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_UDP UDP Functions
 * @{
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


/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_TCP)

#include <signal.h>
#include <pios_tcp_priv.h>

/* We need a list of TCP devices */

#define PIOS_TCP_MAX_DEV 16
static int8_t pios_tcp_num_devices = 0;

static pios_tcp_dev pios_tcp_devices[PIOS_TCP_MAX_DEV];



/* Provide a COM driver */
static void PIOS_TCP_ChangeBaud(uint32_t udp_id, uint32_t baud);
static void PIOS_TCP_RegisterRxCallback(uint32_t udp_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_TCP_RegisterTxCallback(uint32_t udp_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_TCP_TxStart(uint32_t udp_id, uint16_t tx_bytes_avail);
static void PIOS_TCP_RxStart(uint32_t udp_id, uint16_t rx_bytes_avail);

const struct pios_com_driver pios_tcp_com_driver = {
	.set_baud   = PIOS_TCP_ChangeBaud,
	.tx_start   = PIOS_TCP_TxStart,
	.rx_start   = PIOS_TCP_RxStart,
	.bind_tx_cb = PIOS_TCP_RegisterTxCallback,
	.bind_rx_cb = PIOS_TCP_RegisterRxCallback,
};


static pios_tcp_dev * find_tcp_dev_by_id (uint8_t tcp)
{
	if (tcp >= pios_tcp_num_devices) {
		/* Undefined UDP port for this board (see pios_board.c) */
		PIOS_Assert(0);
		return NULL;
	}
	
	/* Get a handle for the device configuration */
	return &(pios_tcp_devices[tcp]);
}

/**
 * RxThread
 */
 
static void PIOS_TCP_RxTask(void *tcp_dev_n)
{
	bool rx_need_yield;

	pios_tcp_dev *tcp_dev = (pios_tcp_dev *) tcp_dev_n;
	while(1) {
		
		if (tcp_dev->rx_in_cb) {
			char buffer[PIOS_TCP_RX_BUFFER_SIZE];
			int received = fifoBuf_getData(&tcp_dev->rx_fifo, buffer, PIOS_TCP_RX_BUFFER_SIZE);
			(void) (tcp_dev->rx_in_cb)(tcp_dev->rx_in_context, (uint8_t *) buffer, received, NULL, &rx_need_yield);

			//fprintf(stderr, "Received %d\n", received);
#if defined(PIOS_INCLUDE_FREERTOS)
			// Not sure about this
			if (rx_need_yield) {
				vPortYieldFromISR();
			}
#endif	/* PIOS_INCLUDE_FREERTOS */
			
		} else
			fifoBuf_clearData(&tcp_dev->rx_fifo);	
		
		vTaskDelay(1);
	}
}

static void *PIOS_TCP_RxThread(void *tcp_dev_n)
{
	
	/* needed because of FreeRTOS.posix scheduling */
	sigset_t set;
	sigfillset(&set);
	sigprocmask(SIG_BLOCK, &set, NULL);
	
	pios_tcp_dev *tcp_dev = (pios_tcp_dev*) tcp_dev_n;
	
	const int INCOMING_BUFFER_SIZE = 16;
	char incoming_buffer[INCOMING_BUFFER_SIZE];
	/**
	 * com devices never get closed except by application "reboot"
	 * we also never give up our mutex except for waiting
	 */
	while(1) {
	
		tcp_dev->socket_connection = accept(tcp_dev->socket, NULL, NULL);
		if (0 > tcp_dev->socket_connection) {
			perror("Accept failed");
			close(tcp_dev->socket);
			exit(EXIT_FAILURE);
		}
		
		fprintf(stderr, "Connection accepted\n");
		
		int received;
		do {
			// Received is used to track the scoket whereas the dev variable is only updated when it can be
			received = read(tcp_dev->socket_connection, incoming_buffer, INCOMING_BUFFER_SIZE);
			
			//while(fifoBuf_getFree(&tcp_dev->rx_fifo) < received);
			
			fifoBuf_putData(&tcp_dev->rx_fifo, incoming_buffer, received);
		} while(received > 0);
		
		if (-1 == shutdown(tcp_dev->socket_connection, SHUT_RDWR))
		{
			//perror("can not shutdown socket");
			//close(tcp_dev->socket_connection);
			//exit(EXIT_FAILURE);
		}
		close(tcp_dev->socket_connection);
		tcp_dev->socket_connection = 0;
	}
}


/**
 * Open UDP socket
 */
xTaskHandle tcpRxTaskHandle;
int32_t PIOS_TCP_Init(uint32_t *tcp_id, const struct pios_tcp_cfg * cfg)
{
	
	pios_tcp_dev *tcp_dev = &pios_tcp_devices[pios_tcp_num_devices];
	
	pios_tcp_num_devices++;
	
	
	/* initialize */
	tcp_dev->rx_in_cb = NULL;
	tcp_dev->tx_out_cb = NULL;
	tcp_dev->cfg=cfg;
	
	/* assign socket */
	tcp_dev->socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&tcp_dev->server,0,sizeof(tcp_dev->server));
	memset(&tcp_dev->client,0,sizeof(tcp_dev->client));
	tcp_dev->server.sin_family = AF_INET;
	tcp_dev->server.sin_addr.s_addr = INADDR_ANY; //inet_addr(tcp_dev->cfg->ip);
	tcp_dev->server.sin_port = htons(tcp_dev->cfg->port);
	int res= bind(tcp_dev->socket, (struct sockaddr *)&tcp_dev->server,sizeof(tcp_dev->server));
	if (res == -1) {
		perror("Binding socket failed\n");
		exit(EXIT_FAILURE);
	}
	
	res = listen(tcp_dev->socket, 10);
	if (res == -1) {
		perror("Socket listen failed\n");
		exit(EXIT_FAILURE);
	}
	
	fifoBuf_init(&tcp_dev->rx_fifo, tcp_dev->rx_buffer, PIOS_TCP_RX_BUFFER_SIZE);
	
	pthread_create(&tcp_dev->rxThread, NULL, PIOS_TCP_RxThread, (void*)tcp_dev);

	xTaskCreate(PIOS_TCP_RxTask, (signed char *)"TcpRx", 1024, (void*)tcp_dev, 2, &tcpRxTaskHandle);
	
	printf("udp dev %i - socket %i opened - result %i\n",pios_tcp_num_devices-1,tcp_dev->socket,res);
	
	*tcp_id = pios_tcp_num_devices-1;
	
	return res;
}


void PIOS_TCP_ChangeBaud(uint32_t tcp_id, uint32_t baud)
{
	/**
	 * doesn't apply!
	 */
}


static void PIOS_TCP_RxStart(uint32_t tp_id, uint16_t rx_bytes_avail)
{
	/**
	 * lazy!
	 */
}


static void PIOS_TCP_TxStart(uint32_t tcp_id, uint16_t tx_bytes_avail)
{
	pios_tcp_dev *tcp_dev = find_tcp_dev_by_id(tcp_id);
	
	PIOS_Assert(tcp_dev);
	
	int32_t length,len,rem;
	
	/**
	 * we send everything directly whenever notified of data to send (lazy!)
	 */
	if (tcp_dev->tx_out_cb) {
		while (tx_bytes_avail>0) {
			bool tx_need_yield = false;
			length = (tcp_dev->tx_out_cb)(tcp_dev->tx_out_context, tcp_dev->tx_buffer, PIOS_TCP_RX_BUFFER_SIZE, NULL, &tx_need_yield);
			rem = length;
			while (rem>0) {
				if(tcp_dev->socket_connection != 0) {
					len = write(tcp_dev->socket_connection, tcp_dev->tx_buffer, length);
				}
				if (len<=0) {
					rem=0;
				} else {
					rem -= len;
				}
			}
			tx_bytes_avail -= length;
#if defined(PIOS_INCLUDE_FREERTOS)
			// Not sure about this
			if (tx_need_yield) {
				vPortYieldFromISR();
			}
#endif	/* PIOS_INCLUDE_FREERTOS */
		}
	}
	
}

static void PIOS_TCP_RegisterRxCallback(uint32_t tcp_id, pios_com_callback rx_in_cb, uint32_t context)
{
	pios_tcp_dev *tcp_dev = find_tcp_dev_by_id(tcp_id);
	
	PIOS_Assert(tcp_dev);
	
	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	tcp_dev->rx_in_context = context;
	tcp_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_TCP_RegisterTxCallback(uint32_t tcp_id, pios_com_callback tx_out_cb, uint32_t context)
{
	pios_tcp_dev *tcp_dev = find_tcp_dev_by_id(tcp_id);
	
	PIOS_Assert(tcp_dev);
	
	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	tcp_dev->tx_out_context = context;
	tcp_dev->tx_out_cb = tx_out_cb;
}

#endif
