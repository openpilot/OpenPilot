/**
 ******************************************************************************
 *
 * @file       pios_udp.c   
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      UDP commands. Inits UDPs, controls UDPs & Interupt handlers.
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

#if defined(PIOS_INCLUDE_UDP)

#include <signal.h>
#include <pios_udp_priv.h>

/* We need a list of UDP devices */

#define PIOS_UDP_MAX_DEV 256
static int8_t pios_udp_num_devices = 0;

static pios_udp_dev pios_udp_devices[PIOS_UDP_MAX_DEV];



/* Provide a COM driver */
static void PIOS_UDP_ChangeBaud(uint32_t udp_id, uint32_t baud);
static void PIOS_UDP_RegisterRxCallback(uint32_t udp_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_UDP_RegisterTxCallback(uint32_t udp_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_UDP_TxStart(uint32_t udp_id, uint16_t tx_bytes_avail);
static void PIOS_UDP_RxStart(uint32_t udp_id, uint16_t rx_bytes_avail);

const struct pios_com_driver pios_udp_com_driver = {
	.set_baud   = PIOS_UDP_ChangeBaud,
	.tx_start   = PIOS_UDP_TxStart,
	.rx_start   = PIOS_UDP_RxStart,
	.bind_tx_cb = PIOS_UDP_RegisterTxCallback,
	.bind_rx_cb = PIOS_UDP_RegisterRxCallback,
};


static pios_udp_dev * find_udp_dev_by_id (uint8_t udp)
{
  if (udp >= pios_udp_num_devices) {
    /* Undefined UDP port for this board (see pios_board.c) */
	PIOS_Assert(0);
    return NULL;
  }

  /* Get a handle for the device configuration */
  return &(pios_udp_devices[udp]);
}

/**
 * RxThread
 */
void PIOS_UDP_RxThread(void * udp_dev_n)
{

	/* needed because of FreeRTOS.posix scheduling */
	sigset_t set;
	sigfillset(&set);
	sigprocmask(SIG_BLOCK, &set, NULL);

	pios_udp_dev * udp_dev = (pios_udp_dev*) udp_dev_n;

   /**
	* com devices never get closed except by application "reboot"
	* we also never give up our mutex except for waiting
	*/
   while(1) {

		/**
		 * receive 
		 */
		int received;
		udp_dev->clientLength=sizeof(udp_dev->client);
		if ((received = recvfrom(udp_dev->socket,
				&udp_dev->rx_buffer,
				PIOS_UDP_RX_BUFFER_SIZE,
				0,
				(struct sockaddr *) &udp_dev->client,
				(socklen_t*)&udp_dev->clientLength)) >= 0)
		{

			/* copy received data to buffer if possible */
			/* we do NOT buffer data locally. If the com buffer can't receive, data is discarded! */
			/* (thats what the USART driver does too!) */
			bool rx_need_yield = false;
			if (udp_dev->rx_in_cb) {
			  (void) (udp_dev->rx_in_cb)(udp_dev->rx_in_context, udp_dev->rx_buffer, received, NULL, &rx_need_yield);
			}

#if defined(PIOS_INCLUDE_FREERTOS)
			if (rx_need_yield) {
				vPortYieldFromISR();
			}
#endif	/* PIOS_INCLUDE_FREERTOS */

		}


	}
}


/**
* Open UDP socket
*/
int32_t PIOS_UDP_Init(uint32_t * udp_id, const struct pios_udp_cfg * cfg)
{

  pios_udp_dev * udp_dev = &pios_udp_devices[pios_udp_num_devices];

  pios_udp_num_devices++;


  /* initialize */
  udp_dev->rx_in_cb = NULL;
  udp_dev->tx_out_cb = NULL;
  udp_dev->cfg=cfg;

  /* assign socket */
  udp_dev->socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  memset(&udp_dev->server,0,sizeof(udp_dev->server));
  memset(&udp_dev->client,0,sizeof(udp_dev->client));
  udp_dev->server.sin_family = AF_INET;
  udp_dev->server.sin_addr.s_addr = inet_addr(udp_dev->cfg->ip);
  udp_dev->server.sin_port = htons(udp_dev->cfg->port);
  int res= bind(udp_dev->socket, (struct sockaddr *)&udp_dev->server,sizeof(udp_dev->server));

  /* Create transmit thread for this connection */
  xTaskCreate(PIOS_UDP_RxThread, (signed char *)"UdpRx", 1024, (void*)udp_dev, 2, &udp_dev->rxThread);

  printf("udp dev %i - socket %i opened - result %i\n",pios_udp_num_devices-1,udp_dev->socket,res);

  *udp_id = pios_udp_num_devices-1;

  return res;
}


void PIOS_UDP_ChangeBaud(uint32_t udp_id, uint32_t baud)
{
	/**
	 * doesn't apply!
	 */
}


static void PIOS_UDP_RxStart(uint32_t udp_id, uint16_t rx_bytes_avail)
{
	/**
	 * lazy!
	 */
}


static void PIOS_UDP_TxStart(uint32_t udp_id, uint16_t tx_bytes_avail)
{
	pios_udp_dev * udp_dev = find_udp_dev_by_id(udp_id);

	PIOS_Assert(udp_dev);

	int32_t length,len,rem;

	/**
	 * we send everything directly whenever notified of data to send (lazy!)
	 */
	if (udp_dev->tx_out_cb) {
		while (tx_bytes_avail>0) {
			bool tx_need_yield = false;
			length = (udp_dev->tx_out_cb)(udp_dev->tx_out_context, udp_dev->tx_buffer, PIOS_UDP_RX_BUFFER_SIZE, NULL, &tx_need_yield);
			rem = length;
			while (rem>0) {
				len = sendto(udp_dev->socket, udp_dev->tx_buffer, length, 0,
						 (struct sockaddr *) &udp_dev->client,
						 sizeof(udp_dev->client));
				if (len<=0) {
					rem=0;
				} else {
					rem -= len;
				}
			}
			tx_bytes_avail -= length;
		}
	}

}

static void PIOS_UDP_RegisterRxCallback(uint32_t udp_id, pios_com_callback rx_in_cb, uint32_t context)
{
	pios_udp_dev * udp_dev = find_udp_dev_by_id(udp_id);

	PIOS_Assert(udp_dev);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	udp_dev->rx_in_context = context;
	udp_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_UDP_RegisterTxCallback(uint32_t udp_id, pios_com_callback tx_out_cb, uint32_t context)
{
	pios_udp_dev * udp_dev = find_udp_dev_by_id(udp_id);

	PIOS_Assert(udp_dev);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	udp_dev->tx_out_context = context;
	udp_dev->tx_out_cb = tx_out_cb;
}





#endif
