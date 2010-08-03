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

#include <pios_udp_priv.h>

/* Provide a COM driver */
const struct pios_com_driver pios_udp_com_driver = {
  .set_baud = PIOS_UDP_ChangeBaud,
  .tx_nb    = PIOS_UDP_TxBufferPutMoreNonBlocking,
  .tx       = PIOS_UDP_TxBufferPutMore,
  .rx       = PIOS_UDP_RxBufferGet,
  .rx_avail = PIOS_UDP_RxBufferUsed,
};

WSADATA wsaData;

static struct pios_udp_dev * find_udp_dev_by_id (uint8_t udp)
{
  if (udp >= pios_udp_num_devices) {
    /* Undefined UDP port for this board (see pios_board.c) */
    return NULL;
  }

  /* Get a handle for the device configuration */
  return &(pios_udp_devs[udp]);
}

/**
* Open some UDP sockets
*/
void PIOS_UDP_Init(void)
{
  struct pios_udp_dev * udp_dev;
  uint8_t                 i;

  WSAStartup(MAKEWORD(2, 0), &wsaData);

  for (i = 0; i < pios_udp_num_devices; i++) {
    /* Get a handle for the device configuration */
    udp_dev = find_udp_dev_by_id(i);
    //PIOS_DEBUG_Assert(udp_dev);

    /* Clear buffer counters */
    udp_dev->rx.head = udp_dev->rx.tail = udp_dev->rx.size = 0;

    /* assign socket */
    udp_dev->socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    memset(&udp_dev->server,0,sizeof(udp_dev->server));
    memset(&udp_dev->client,0,sizeof(udp_dev->client));
    udp_dev->server.sin_family = AF_INET;
    udp_dev->server.sin_addr.s_addr = inet_addr(udp_dev->cfg->ip);
    udp_dev->server.sin_port = htons(udp_dev->cfg->port);
    BOOL tmp = TRUE;
    setsockopt(udp_dev->socket, SOL_SOCKET, SO_BROADCAST, (char *)&tmp, sizeof(tmp));
    int res= bind(udp_dev->socket, (struct sockaddr *)&udp_dev->server,sizeof(udp_dev->server));
    /* use nonblocking IO */
    u_long argp = 1;
    ioctlsocket(udp_dev->socket, FIONBIO, &argp);
    printf("udp dev %i - socket %i opened - result %i\n",i,udp_dev->socket,res);

    /* TODO do some error handling - wait no, we can't - we are void anyway ;) */
  }
}

/**
 * Clean up Winsock
 */
void PIOS_UDP_Close(void)
{
	WSACleanup();
}

/**
* Changes the baud rate of the UDP peripheral without re-initialising.
* \param[in] udp UDP name (GPS, TELEM, AUX)
* \param[in] baud Requested baud rate
*/
void PIOS_UDP_ChangeBaud(uint8_t udp, uint32_t baud)
{
}


/**
* puts a byte onto the receive buffer
* \param[in] UDP UDP name
* \param[in] b byte which should be put into Rx buffer
* \return 0 if no error
* \return -1 if UDP not available
* \return -2 if buffer full (retry)
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_UDP_RxBufferPut(uint8_t udp, uint8_t b)
{
  struct pios_udp_dev * udp_dev;

  /* Get a handle for the device configuration */
  udp_dev = find_udp_dev_by_id(udp);

  if (!udp_dev) {
    /* Undefined UDP port for this board (see pios_board.c) */
    return -1;
  }

  if (udp_dev->rx.size >= sizeof(udp_dev->rx.buf)) {
    /* Buffer full (retry) */
    return -2;
  }

  /* Copy received byte into receive buffer */
  udp_dev->rx.buf[udp_dev->rx.head++] = b;
  if (udp_dev->rx.head >= sizeof(udp_dev->rx.buf)) {
    udp_dev->rx.head = 0;
  }
  udp_dev->rx.size++;

  /* No error */
  return 0;
}

/**
 * attempt to receive
 */
void PIOS_UDP_RECV(uint8_t udp) {

  struct pios_udp_dev * udp_dev;
  char localbuffer[PIOS_UDP_RX_BUFFER_SIZE];

  /* Get a handle for the device configuration */
  udp_dev = find_udp_dev_by_id(udp);

  if (!udp_dev) {
    /* Undefined UDP port for this board (see pios_board.c) */
    return;
  }

  /* use nonblocking IO */
  u_long argp = 1;
  ioctlsocket(udp_dev->socket, FIONBIO, &argp);
  
  /* receive data */
  int received;
  udp_dev->clientLength=sizeof(udp_dev->client);
  if ((received = recvfrom(udp_dev->socket,
  			localbuffer,
			(PIOS_UDP_RX_BUFFER_SIZE - udp_dev->rx.size),
			0,
			(struct sockaddr *) &udp_dev->client,
			(int*)&udp_dev->clientLength)) == SOCKET_ERROR) {

    return;
  }
  /* copy received data to buffer */
  int t;
  for (t=0;t<received;t++) {
    PIOS_UDP_RxBufferPut(udp,localbuffer[t]);
  }


}

/**
* Returns number of free bytes in receive buffer
* \param[in] UDP UDP name
* \return UDP number of free bytes
* \return 1: UDP available
* \return 0: UDP not available
*/
int32_t PIOS_UDP_RxBufferFree(uint8_t udp)
{
  struct pios_udp_dev * udp_dev;

  /* Get a handle for the device configuration */
  udp_dev = find_udp_dev_by_id(udp);

  if (!udp_dev) {
    /* Undefined UDP port for this board (see pios_board.c) */
    return -2;
  }

  /* fill buffer */
  PIOS_UDP_RECV(udp);

  return (sizeof(udp_dev->rx.buf) - udp_dev->rx.size);
}

/**
* Returns number of used bytes in receive buffer
* \param[in] UDP UDP name
* \return > 0: number of used bytes
* \return 0 if UDP not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_UDP_RxBufferUsed(uint8_t udp)
{
  struct pios_udp_dev * udp_dev;

  /* Get a handle for the device configuration */
  udp_dev = find_udp_dev_by_id(udp);

  if (!udp_dev) {
    /* Undefined UDP port for this board (see pios_board.c) */
    return -2;
  }

  /* fill buffer */
  PIOS_UDP_RECV(udp);

  return (udp_dev->rx.size);
}

/**
* Gets a byte from the receive buffer
* \param[in] UDP UDP name
* \return -1 if UDP not available
* \return -2 if no new byte available
* \return >= 0: number of received bytes
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_UDP_RxBufferGet(uint8_t udp)
{
  struct pios_udp_dev * udp_dev;

  /* Get a handle for the device configuration */
  udp_dev = find_udp_dev_by_id(udp);

  if (!udp_dev) {
    /* Undefined UDP port for this board (see pios_board.c) */
    return -2;
  }

  /* fill buffer */
  PIOS_UDP_RECV(udp);

  if (!udp_dev->rx.size) {
    /* Nothing new in the buffer */
	//printf("PIOS_UDP: nothing new in the buffer\n");
    return -1;
  }

  /* get byte */
  uint8_t b = udp_dev->rx.buf[udp_dev->rx.tail++];
  if (udp_dev->rx.tail >= sizeof(udp_dev->rx.buf)) {
    udp_dev->rx.tail = 0;
  }
  udp_dev->rx.size--;
  
  //printf("PIOS_UDP: received byte: %c\n", (char) b);

  /* Return received byte */
  return b;
}

/**
* Returns the next byte of the receive buffer without taking it
* \param[in] UDP UDP name
* \return -1 if UDP not available
* \return -2 if no new byte available
* \return >= 0: number of received bytes
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_UDP_RxBufferPeek(uint8_t udp)
{
  struct pios_udp_dev * udp_dev;

  /* Get a handle for the device configuration */
  udp_dev = find_udp_dev_by_id(udp);

  if (!udp_dev) {
    /* Undefined UDP port for this board (see pios_board.c) */
    return -2;
  }

  /* fill buffer */
  PIOS_UDP_RECV(udp);

  if (!udp_dev->rx.size) {
    /* Nothing new in the buffer */
    return -1;
  }

  /* get byte */
  uint8_t b = udp_dev->rx.buf[udp_dev->rx.tail];
  
  /* Return received byte */
  return b;
}

/**
* returns number of free bytes in transmit buffer
* \param[in] UDP UDP name
* \return number of free bytes
* \return 0 if UDP not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_UDP_TxBufferFree(uint8_t udp)
{
  struct pios_udp_dev * udp_dev;

  /* Get a handle for the device configuration */
  udp_dev = find_udp_dev_by_id(udp);

  if (!udp_dev) {
    /* Undefined UDP port for this board (see pios_board.c) */
    return 0;
  }

  return PIOS_UDP_RX_BUFFER_SIZE;
}

/**
* returns number of used bytes in transmit buffer
* \param[in] UDP UDP name
* \return number of used bytes
* \return 0 if UDP not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_UDP_TxBufferUsed(uint8_t udp)
{
  struct pios_udp_dev * udp_dev;

  /* Get a handle for the device configuration */
  udp_dev = find_udp_dev_by_id(udp);

  if (!udp_dev) {
    /* Undefined UDP port for this board (see pios_board.c) */
    return 0;
  }

  return 0;
}


/**
* puts more than one byte onto the transmit buffer (used for atomic sends)
* \param[in] UDP UDP name
* \param[in] *buffer pointer to buffer to be sent
* \param[in] len number of bytes to be sent
* \return 0 if no error
* \return -1 if UDP not available
* \return -2 if buffer full or cannot get all requested bytes (retry)
* \return -3 if UDP not supported by UDPTxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_UDP_TxBufferPutMoreNonBlocking(uint8_t udp, char *buffer, uint16_t len)
{
  struct pios_udp_dev * udp_dev;

  /* Get a handle for the device configuration */
  udp_dev = find_udp_dev_by_id(udp);

  if (!udp_dev) {
    /* Undefined UDP port for this board (see pios_board.c) */
    return -1;
  }

  if (len >= PIOS_UDP_RX_BUFFER_SIZE) {
    /* Buffer cannot accept all requested bytes (retry) */
    return -2;
  }
  /* send data to client - non blocking*/

  /* use nonblocking IO */
  u_long argp = 1;
  ioctlsocket(udp_dev->socket, FIONBIO, &argp);
  sendto(udp_dev->socket, buffer, len, 0,
                         (struct sockaddr *) &udp_dev->client,
                         (int)sizeof(udp_dev->client));


  /* No error */
  return 0;
}

/**
* puts more than one byte onto the transmit buffer (used for atomic sends)<BR>
* (blocking function)
* \param[in] UDP UDP name
* \param[in] *buffer pointer to buffer to be sent
* \param[in] len number of bytes to be sent
* \return 0 if no error
* \return -1 if UDP not available
* \return -3 if UDP not supported by UDPTxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_UDP_TxBufferPutMore(uint8_t udp, char *buffer, uint16_t len)
{
  struct pios_udp_dev * udp_dev;

  /* Get a handle for the device configuration */
  udp_dev = find_udp_dev_by_id(udp);

  if (!udp_dev) {
    /* Undefined UDP port for this board (see pios_board.c) */
    return -1;
  }

  if (len >= PIOS_UDP_RX_BUFFER_SIZE) {
    /* Buffer cannot accept all requested bytes (retry) */
    return -2;
  }

  /* send data to client - blocking*/
  /* use blocking IO */
  u_long argp = 1;
  ioctlsocket(udp_dev->socket, FIONBIO, &argp);
  sendto(udp_dev->socket, buffer, len, 0,
                         (struct sockaddr *) &udp_dev->client,
                         sizeof(udp_dev->client));

  //printf("PIOS_UDP: sent data\n");

  /* No error */
  return 0;
}

/**
* puts a byte onto the transmit buffer
* \param[in] UDP UDP name
* \param[in] b byte which should be put into Tx buffer
* \return 0 if no error
* \return -1 if UDP not available
* \return -2 if buffer full (retry)
* \return -3 if UDP not supported by UDPTxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_UDP_TxBufferPut_NonBlocking(uint8_t udp, char b)
{
  return PIOS_UDP_TxBufferPutMoreNonBlocking(udp, &b, 1);
}

/**
* puts a byte onto the transmit buffer<BR>
* (blocking function)
* \param[in] UDP UDP name
* \param[in] b byte which should be put into Tx buffer
* \return 0 if no error
* \return -1 if UDP not available
* \return -3 if UDP not supported by UDPTxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_UDP_TxBufferPut(uint8_t udp, char b)
{
  return PIOS_UDP_TxBufferPutMore(udp, &b, 1);
}


#endif
