/******************************************************************************
 *
 * @file       packet_handler.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Modem packet handling routines
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


// ********

// We use 128-bit AES CBC encryption if encryption is enabled


// encrypted packet format
// 16-byte  CBC .. 1st byte must not be zero
//  4-byte  source id
//  4-byte  destination id
//  1-byte  packet type
//  1-byte  tx sequence value
//  1-byte  rx sequence value
//  1-byte  data size
//  4-byte  crc of entire packet not including CBC bytes


// unencrypted packet format
//  1-byte  null byte .. set to zero to indicate packet is not encrypted
//  4-byte  source id
//  4-byte  destination id
//  1-byte  packet type
//  1-byte  tx sequence value
//  1-byte  rx sequence value
//  1-byte  data size
//  4-byte  crc of entire packet not including the null byte

// ********

#include <string.h>	// memmove

#include "main.h"
#include "rfm22b.h"
#include "fifo_buffer.h"
#include "aes.h"
#include "crc.h"
#include "saved_settings.h"
#include "packet_handler.h"

#if defined(PIOS_COM_DEBUG)
//  #define PACKET_DEBUG
#endif

// *****************************************************************************

#define PH_FIFO_BUFFER_SIZE             2048    // FIFO buffer size

// *****************************************************************************

#define AES_BLOCK_SIZE                  16      // AES encryption does it in 16-byte blocks ONLY

// default aes 128-bit encryption key
const uint8_t default_aes_key[AES_BLOCK_SIZE] = {0x65, 0x3b, 0x71, 0x89, 0x4a, 0xf4, 0xc8, 0xcb, 0x18, 0xd4, 0x9b, 0x4d, 0x4a, 0xbe, 0xc8, 0x37};

// *****************************************************************************

#define RETRY_RECONNECT_COUNT           60      // if transmission retries this many times then reset the link to the remote modem

#define PACKET_TYPE_DATA_COMP_BIT       0x80    // data compressed bit. if set then the data in the packet is compressed
#define PACKET_TYPE_MASK                0x7f    // packet type mask

enum {
	PACKET_TYPE_NONE = 0,

	PACKET_TYPE_CONNECT,						// for requesting a connection
	PACKET_TYPE_CONNECT_ACK,					// ack

	PACKET_TYPE_DISCONNECT,						// to tell the other modem they cannot connect to us

	PACKET_TYPE_DATA,							// data packet (packet contains user data)
	PACKET_TYPE_DATA_ACK,						// ack

	PACKET_TYPE_READY,							// tells the other modem we are ready to accept more data
	PACKET_TYPE_READY_ACK,						// ack

	PACKET_TYPE_NOTREADY,						// tells the other modem we're not ready to accept more data - we can also send user data in this packet type
	PACKET_TYPE_NOTREADY_ACK,					// ack

	PACKET_TYPE_DATARATE,						// for changing the RF data rate
	PACKET_TYPE_DATARATE_ACK,					// ack

	PACKET_TYPE_PING,							// used to check link is still up
	PACKET_TYPE_PONG,							// ack

	PACKET_TYPE_ADJUST_TX_PWR,					// used to ask the other modem to adjust it's tx power
	PACKET_TYPE_ADJUST_TX_PWR_ACK				// ack
};

#define BROADCAST_ADDR                          0xffffffff

//#pragma pack(push)
//#pragma pack(1)

typedef struct
{
    uint32_t            source_id;
    uint32_t            destination_id;
    uint8_t             type;
    uint8_t             tx_seq;
    uint8_t             rx_seq;
    uint8_t             data_size;
    uint32_t            crc;
} __attribute__((__packed__)) t_packet_header;

// this structure must be a multiple of 'AES_BLOCK_SIZE' bytes in size and no more than 255 bytes in size
typedef struct
{
    uint8_t             cbc[AES_BLOCK_SIZE];            // AES encryption Cipher-Block-Chaining key .. 1st byte must not be zero - to indicate the packet is encrypted
    t_packet_header     header;
    uint8_t             data[240 - sizeof(t_packet_header) - AES_BLOCK_SIZE];
} __attribute__((__packed__)) t_encrypted_packet;

// this structure must be no more than 255 bytes in size (255 = the maximum packet size)
typedef struct
{
    uint8_t             null_byte;                      // this must be set to zero - to indicate the packet is unencrypted
    t_packet_header     header;
    uint8_t             data[255 - sizeof(t_packet_header) - 1];
} __attribute__((__packed__)) t_unencrypted_packet;

//#pragma pack(pop)

// *****************************************************************************
// link state for each remote connection

enum {
	LINK_DISCONNECTED = 0,
	LINK_CONNECTING,
	LINK_CONNECTED
};

typedef struct
{
    uint32_t            serial_number;                  // their serial number

    uint8_t             tx_buffer[PH_FIFO_BUFFER_SIZE] __attribute__ ((aligned(4)));
    t_fifo_buffer       tx_fifo_buffer;                 // holds the data to be transmitted to the other modem

    uint8_t             rx_buffer[PH_FIFO_BUFFER_SIZE] __attribute__ ((aligned(4)));
    t_fifo_buffer       rx_fifo_buffer;                 // holds the data received from the other modem

    uint8_t             link_state;                     // holds our current RF link state

    uint8_t             tx_sequence;                    // incremented with each data packet transmitted, sent in every packet transmitted
    uint8_t             tx_sequence_data_size;          // the size of data we sent in our last packet

    uint8_t             rx_sequence;                    // incremented with each data packet received contain data, sent in every packet transmitted

    volatile uint16_t   tx_packet_timer;                // ms .. used for packet timing

    uint16_t            tx_retry_time_slots;            //       add's some random packet transmission timing - to try to prevent transmission collisions
    uint16_t            tx_retry_time_slot_len;         // ms ..   "                "                    "
    uint16_t            tx_retry_time;                  // ms ..   "                "                    "
    uint16_t            tx_retry_counter;               // incremented on each transmission, reset back to '0' when we receive an ack to our transmission

    volatile uint16_t   data_speed_timer;               // used for calculating the transmit/receive data rate
    volatile uint32_t   tx_data_speed_count;            // incremented with the number of data bits we send in our transmit packets
    volatile uint32_t   tx_data_speed;                  // holds the number of data bits we have sent each second
    volatile uint32_t   rx_data_speed_count;            // incremented with the number of data bits we send in our transmit packets
    volatile uint32_t   rx_data_speed;                  // holds the number of data bits we have received each second

    uint16_t            ping_time;                      // ping timer
    uint16_t            fast_ping_time;                 // ping timer
    bool                pinging;                        // TRUE if we are doing a ping test with the other modem - to check if it is still present

    bool                rx_not_ready_mode;              // TRUE if we have told the other modem we cannot receive data (due to buffer filling up).
    													// we set it back to FALSE when our received buffer starts to empty

    volatile int16_t    ready_to_send_timer;            // ms .. used to hold off packet transmission to wait a bit for data to mount up for transmission (improves data thru-put speed)

    volatile int32_t    not_ready_timer;                // ms .. >= 0 while we have been asked not to send anymore data to the other modem, -1 when we are allowed to send data

    bool                send_encrypted;                 // TRUE if we are to AES encrypt in every packet we transmit

    int16_t             rx_rssi_dBm;                    // the strength of the received packet
    int32_t             rx_afc_Hz;                      // the frequency offset of the received packet

} t_connection;

// *****************************************************************************

uint32_t        our_serial_number = 0;										// our serial number

t_connection    connection[PH_MAX_CONNECTIONS];								// holds each connection state

uint8_t         aes_key[AES_BLOCK_SIZE] __attribute__ ((aligned(4)));		// holds the aes encryption key - the same for ALL connections
uint8_t         dec_aes_key[AES_BLOCK_SIZE] __attribute__ ((aligned(4)));	// holds the pre-calculated decryption key
uint8_t         enc_cbc[AES_BLOCK_SIZE] __attribute__ ((aligned(4)));		// holds the tx aes cbc bytes

uint8_t         ph_tx_buffer[256] __attribute__ ((aligned(4)));				// holds the transmit packet

uint8_t         ph_rx_buffer[256] __attribute__ ((aligned(4)));				// holds the received packet

int16_t         rx_rssi_dBm;
int32_t         rx_afc_Hz;

bool			fast_ping;

// *****************************************************************************
// return TRUE if we are connected to the remote modem

bool ph_connected(const int connection_index)
{
    if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
        return FALSE;

    t_connection *conn = &connection[connection_index];

    return (conn->link_state == LINK_CONNECTED);
}

// *****************************************************************************
// public tx buffer functions

uint16_t ph_putData_free(const int connection_index)
{	// return the free space size
    if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
      return 0;

    return fifoBuf_getFree(&connection[connection_index].tx_fifo_buffer);
}

uint16_t ph_putData(const int connection_index, const void *data, uint16_t len)
{	// add data to our tx buffer to be sent
    if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
        return 0;

    return fifoBuf_putData(&connection[connection_index].tx_fifo_buffer, data, len);
}

// *****************************************************************************
// public rx buffer functions

uint16_t ph_getData_used(const int connection_index)
{	// return the number of bytes available in the rx buffer
    if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
      return 0;

    return fifoBuf_getUsed(&connection[connection_index].rx_fifo_buffer);
}

uint16_t ph_getData(const int connection_index, void *data, uint16_t len)
{	// get data from our rx buffer
    if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
        return 0;

    return fifoBuf_getData(&connection[connection_index].rx_fifo_buffer, data, len);
}

// *****************************************************************************
// start a connection to another modem

int ph_startConnect(int connection_index, uint32_t sn)
{
    random32 = updateCRC32(random32, 0xff);

    if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
      return -1;

    t_connection *conn = &connection[connection_index];

	conn->link_state = LINK_DISCONNECTED;

    LINK_LED_OFF;

	conn->serial_number = sn;

    conn->tx_sequence = 0;
    conn->tx_sequence_data_size = 0;
    conn->rx_sequence = 0;

//    fifoBuf_init(&conn->tx_fifo_buffer, conn->tx_buffer, PH_FIFO_BUFFER_SIZE);
//    fifoBuf_init(&conn->rx_fifo_buffer, conn->rx_buffer, PH_FIFO_BUFFER_SIZE);

    conn->tx_packet_timer = 0;

    conn->tx_retry_time_slots = 5;

    uint32_t ms = 1280000ul / rfm22_getDatarate();
    if (ms < 10) ms = 10;
    else
    if (ms > 32000) ms = 32000;
    conn->tx_retry_time_slot_len = ms;

    conn->tx_retry_time = conn->tx_retry_time_slot_len * 4 + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len * 4;

    conn->tx_retry_counter = 0;

    conn->data_speed_timer = 0;
    conn->tx_data_speed_count = 0;
    conn->tx_data_speed = 0;
    conn->rx_data_speed_count = 0;
    conn->rx_data_speed = 0;

    conn->ping_time = 8000 + (random32 % 100) * 10;
    conn->fast_ping_time = 600 + (random32 % 50) * 10;
    conn->pinging = false;

    conn->rx_not_ready_mode = false;

    conn->ready_to_send_timer = -1;

    conn->not_ready_timer = -1;

//	conn->send_encrypted = true;
//	conn->send_encrypted = false;

    conn->rx_rssi_dBm = -200;
    conn->rx_afc_Hz = 0;

    if (sn != 0 && sn == our_serial_number)
        return -2;							// same as our own

    if (sn == BROADCAST_ADDR)
    {

        return -3;
    }

    if (conn->serial_number != 0)
    	conn->link_state = LINK_CONNECTING;

    return connection_index;
}

// *****************************************************************************
// return a byte for the tx packet transmission.
//
// return value < 0 if no more bytes available, otherwise return byte to be sent

int16_t ph_TxDataByteCallback(void)
{
	return -1;
}

// *************************************************************
// we are being given a block of received bytes
//
// return TRUE to continue current packet receive, otherwise return FALSE to halt current packet reception

bool ph_RxDataCallback(void *data, uint8_t len)
{
	return true;
}

// *****************************************************************************
// transmit a packet

bool ph_sendPacket(int connection_index, bool encrypt, uint8_t packet_type, bool send_immediately)
{
  uint8_t key[AES_BLOCK_SIZE];

  t_connection *conn = NULL;

  // ***********

  t_encrypted_packet *encrypted_packet = (t_encrypted_packet*)&ph_tx_buffer;			// point to the tx buffer
  t_unencrypted_packet *unencrypted_packet = (t_unencrypted_packet*)&ph_tx_buffer;	// point to the tx buffer

  t_packet_header *header;
  uint8_t *data;
  uint16_t max_data_size;

  if (encrypt)
  {
      header = (t_packet_header *)&encrypted_packet->header;
      data = (uint8_t *)&encrypted_packet->data;
      max_data_size = sizeof(encrypted_packet->data);
  }
  else
  {
      header = (t_packet_header *)&unencrypted_packet->header;
      data = (uint8_t *)&unencrypted_packet->data;
      max_data_size = sizeof(unencrypted_packet->data);
  }

  // ***********

  if (!rfm22_txReady())
    return false;

  if ((packet_type & PACKET_TYPE_MASK) == PACKET_TYPE_NONE)
    return false;

  if (connection_index >= PH_MAX_CONNECTIONS)
    return false;

  if (connection_index >= 0)
    conn = (t_connection *)&connection[connection_index];
  else
    return false;

  // ******************
  // stuff

  uint8_t pack_type = packet_type & PACKET_TYPE_MASK;

  bool data_packet = (pack_type == PACKET_TYPE_DATA || pack_type == PACKET_TYPE_NOTREADY);

  // ******************
  // calculate how many user data bytes we are going to add to the packet

  uint16_t data_size = 0;

  if (data_packet && conn)
  {	// we're adding user data to the packet
      data_size = fifoBuf_getUsed(&connection[connection_index].tx_fifo_buffer);	// the number of data bytes waiting to be sent

      if (data_size > max_data_size)
        data_size = max_data_size;

      if (conn->tx_sequence_data_size > 0)
      {	// we are re-sending data the same data
          if (data_size > conn->tx_sequence_data_size)
            data_size = conn->tx_sequence_data_size;
      }
  }

  // ******************
  // calculate the total packet size (including null data bytes if we have to add null data byte in AES encrypted packets)

  uint32_t packet_size;

  if (encrypt)
  {
      packet_size = AES_BLOCK_SIZE + sizeof(t_packet_header) + data_size;

      // total packet size must be a multiple of 'AES_BLOCK_SIZE' bytes - aes encryption works on 16-byte blocks
      packet_size = (packet_size + (AES_BLOCK_SIZE - 1)) & ~(AES_BLOCK_SIZE - 1);
  }
  else
  {
      packet_size = 1 + sizeof(t_packet_header) + data_size;
  }

  // ******************
  // construct the packets entire header

  if (encrypt)
  {
      memmove(key, aes_key, sizeof(key));						// fetch the encryption key
      aes_encrypt_cbc_128(enc_cbc, key, NULL);				// help randomize the CBC bytes

      // ensure the 1st byte is not zero - to indicate this packet is encrypted
      while (enc_cbc[0] == 0)
      {
          random32 = updateCRC32(random32, 0xff);
          enc_cbc[0] ^= random32;
      }

      memmove(encrypted_packet->cbc, enc_cbc, AES_BLOCK_SIZE);	// copy the AES CBC bytes into the packet
  }
  else
      unencrypted_packet->null_byte = 0;						// packet is not encrypted

  header->source_id = our_serial_number;						// our serial number
//      header->destination_id = BROADCAST_ADDR;					// broadcast packet
  header->destination_id = conn->serial_number;				// the other modems serial number
  header->type = packet_type;									// packet type
  header->tx_seq = conn->tx_sequence;							// our TX sequence number
  header->rx_seq = conn->rx_sequence;							// our RX sequence number
  header->data_size = data_size;								// the number of user data bytes in the packet
  header->crc = 0;											// the CRC of the header and user data bytes

  // ******************
  // add the user data to the packet

  if (data_packet)
  {	// we're adding user data to the packet
      fifoBuf_getDataPeek(&connection[connection_index].tx_fifo_buffer, data, data_size);

      if (encrypt)
      {	// zero unused bytes
          if (data_size < max_data_size)
            memset(data + data_size, 0, max_data_size - data_size);
      }

      conn->tx_sequence_data_size = data_size;	// remember how much data we are sending in this packet
  }

  // ******************
  // complete the packet header by adding the CRC

  if (encrypt)
    header->crc = updateCRC32Data(0xffffffff, header, packet_size - AES_BLOCK_SIZE);
  else
    header->crc = updateCRC32Data(0xffffffff, header, packet_size - 1);

  // ******************
  // encrypt the packet

  if (encrypt)
  {	// encrypt the packet .. 'AES_BLOCK_SIZE' bytes at a time
      uint8_t *p = (uint8_t *)encrypted_packet;

      // encrypt the cbc
      memmove(key, aes_key, sizeof(key));				// fetch the encryption key
      aes_encrypt_cbc_128(p, key, NULL);				// encrypt block of data (the CBC bytes)
      p += AES_BLOCK_SIZE;

      // encrypt the rest of the packet
      for (uint16_t i = AES_BLOCK_SIZE; i < packet_size; i += AES_BLOCK_SIZE)
      {
          memmove(key, aes_key, sizeof(key));			// fetch the encryption key
          aes_encrypt_cbc_128(p, key, enc_cbc);		// encrypt block of data
          p += AES_BLOCK_SIZE;
      }
  }

  // ******************
  // send the packet

  int32_t res = rfm22_sendData(&ph_tx_buffer, packet_size, send_immediately);

  // ******************

  if (data_size > 0 && conn->tx_retry_counter == 0)
    conn->tx_data_speed_count += data_size * 8;	// + the number of data bits we just sent .. used for calculating the transmit data rate

  // ******************
  // debug stuff

#if defined(PACKET_DEBUG)

  DEBUG_PRINTF("T-PACK ");
  switch (pack_type)
  {
    case PACKET_TYPE_NONE:              DEBUG_PRINTF("none"); break;
    case PACKET_TYPE_CONNECT:           DEBUG_PRINTF("connect"); break;
    case PACKET_TYPE_CONNECT_ACK:       DEBUG_PRINTF("connect_ack"); break;
    case PACKET_TYPE_DISCONNECT:        DEBUG_PRINTF("disconnect"); break;
    case PACKET_TYPE_DATA:              DEBUG_PRINTF("data"); break;
    case PACKET_TYPE_DATA_ACK:          DEBUG_PRINTF("data_ack"); break;
    case PACKET_TYPE_READY:             DEBUG_PRINTF("ready"); break;
    case PACKET_TYPE_READY_ACK:         DEBUG_PRINTF("ready_ack"); break;
    case PACKET_TYPE_NOTREADY:          DEBUG_PRINTF("notready"); break;
    case PACKET_TYPE_NOTREADY_ACK:      DEBUG_PRINTF("notready_ack"); break;
    case PACKET_TYPE_DATARATE:          DEBUG_PRINTF("datarate"); break;
    case PACKET_TYPE_DATARATE_ACK:      DEBUG_PRINTF("datarate_ack"); break;
    case PACKET_TYPE_PING:              DEBUG_PRINTF("ping"); break;
    case PACKET_TYPE_PONG:              DEBUG_PRINTF("pong"); break;
    case PACKET_TYPE_ADJUST_TX_PWR:     DEBUG_PRINTF("PACKET_TYPE_ADJUST_TX_PWR"); break;
    case PACKET_TYPE_ADJUST_TX_PWR_ACK: DEBUG_PRINTF("PACKET_TYPE_ADJUST_TX_PWR_ACK"); break;
    default:                            DEBUG_PRINTF("UNKNOWN [%d]", pack_type); break;
  }
  DEBUG_PRINTF(" tseq:%d rseq:%d", conn->tx_sequence, conn->rx_sequence);
  DEBUG_PRINTF(" drate:%dbps", conn->tx_data_speed);
  if (data_size > 0) DEBUG_PRINTF(" data_size:%d", data_size);
  if (conn->tx_retry_counter > 0) DEBUG_PRINTF(" retry:%d", conn->tx_retry_counter);
  DEBUG_PRINTF("\r\n");
#endif

  // ******************

  switch (pack_type)
  {
    case PACKET_TYPE_CONNECT:
    case PACKET_TYPE_DISCONNECT:
    case PACKET_TYPE_DATA:
    case PACKET_TYPE_READY:
    case PACKET_TYPE_NOTREADY:
    case PACKET_TYPE_DATARATE:
    case PACKET_TYPE_PING:
    case PACKET_TYPE_ADJUST_TX_PWR:
      if (conn->tx_retry_counter < 0xffff)
        conn->tx_retry_counter++;
      break;

    case PACKET_TYPE_CONNECT_ACK:
    case PACKET_TYPE_DATA_ACK:
    case PACKET_TYPE_READY_ACK:
    case PACKET_TYPE_NOTREADY_ACK:
    case PACKET_TYPE_DATARATE_ACK:
    case PACKET_TYPE_PONG:
    case PACKET_TYPE_ADJUST_TX_PWR_ACK:
      break;

    case PACKET_TYPE_NONE:
      break;
  }

  return (res >= packet_size);
}

// *****************************************************************************

void ph_processPacket2(bool was_encrypted, t_packet_header *header, uint8_t *data)
{	// process the received decrypted error-free packet

  USB_LED_TOGGLE;			// TEST ONLY

  // ***********

  // fetch the data compressed bit
  bool compressed_data = (header->type & PACKET_TYPE_DATA_COMP_BIT) != 0;

  // fetch the packet type
  uint8_t packet_type = header->type & PACKET_TYPE_MASK;

  // fetch the number of data bytes in the packet
  uint16_t data_size = header->data_size;

  // update the ramdon number
  random32 = updateCRC32(random32, 0xff);

  // *********************
  // debug stuff
/*
#if defined(PACKET_DEBUG)
	if (data_size > 0)
	{
		DEBUG_PRINTF("rx packet:");
		for (uint16_t i = 0; i < data_size; i++)
			DEBUG_PRINTF(" %u", data[i]);
		DEBUG_PRINTF("\r\n");
	}
#endif
*/
  // ***********
  // debug stuff

#if defined(PACKET_DEBUG)
  DEBUG_PRINTF("R-PACK ");
  switch (packet_type)
  {
    case PACKET_TYPE_NONE:              DEBUG_PRINTF("none"); break;
    case PACKET_TYPE_CONNECT:           DEBUG_PRINTF("connect"); break;
    case PACKET_TYPE_CONNECT_ACK:       DEBUG_PRINTF("connect_ack"); break;
    case PACKET_TYPE_DISCONNECT:        DEBUG_PRINTF("disconnect"); break;
    case PACKET_TYPE_DATA:              DEBUG_PRINTF("data"); break;
    case PACKET_TYPE_DATA_ACK:          DEBUG_PRINTF("data_ack"); break;
    case PACKET_TYPE_READY:             DEBUG_PRINTF("ready"); break;
    case PACKET_TYPE_READY_ACK:         DEBUG_PRINTF("ready_ack"); break;
    case PACKET_TYPE_NOTREADY:          DEBUG_PRINTF("notready"); break;
    case PACKET_TYPE_NOTREADY_ACK:      DEBUG_PRINTF("notready_ack"); break;
    case PACKET_TYPE_DATARATE:          DEBUG_PRINTF("datarate"); break;
    case PACKET_TYPE_DATARATE_ACK:      DEBUG_PRINTF("datarate_ack"); break;
    case PACKET_TYPE_PING:              DEBUG_PRINTF("ping"); break;
    case PACKET_TYPE_PONG:              DEBUG_PRINTF("pong"); break;
    case PACKET_TYPE_ADJUST_TX_PWR:     DEBUG_PRINTF("PACKET_TYPE_ADJUST_TX_PWR"); break;
    case PACKET_TYPE_ADJUST_TX_PWR_ACK: DEBUG_PRINTF("PACKET_TYPE_ADJUST_TX_PWR_ACK"); break;
    default:                            DEBUG_PRINTF("UNKNOWN [%d]", packet_type); break;
  }
  DEBUG_PRINTF(" tseq-%d rseq-%d", header->tx_seq, header->rx_seq);
//      DEBUG_PRINTF(" drate:%dbps", conn->rx_data_speed);
  if (data_size > 0) DEBUG_PRINTF(" data_size:%d", data_size);
  DEBUG_PRINTF(" %ddBm", rx_rssi_dBm);
  DEBUG_PRINTF(" %dHz", rx_afc_Hz);
  DEBUG_PRINTF("\r\n");
#endif

  // *********************

  if (header->source_id == our_serial_number)
    return;	// it's our own packet .. ignore it

  if (header->destination_id == BROADCAST_ADDR)
  {	// it's a broadcast packet



		// todo:




      return;
  }

  if (header->destination_id != our_serial_number)
    return;		// the packet is not meant for us

  // *********************
  // find out which remote connection this packet is from

  int connection_index = 0;
  while (connection_index < PH_MAX_CONNECTIONS)
  {
      uint32_t sn = connection[connection_index].serial_number;
      if (sn != 0)
      {	// connection used
          if (header->source_id == sn)
            break;	// found it
      }
      connection_index++;
  }

  if (connection_index >= PH_MAX_CONNECTIONS)
  {	// the packet is from an unknown source ID (unknown modem)

      if (packet_type != PACKET_TYPE_NONE)
      {	// send a disconnect packet back to them
//              ph_sendPacket(-1, was_encrypted, PACKET_TYPE_DISCONNECT, true);
      }

      return;
  }

  t_connection *conn = &connection[connection_index];

  // ***********

  conn->rx_rssi_dBm = rx_rssi_dBm;				// remember the packets signal strength
  conn->rx_afc_Hz = rx_afc_Hz;					// remember the packets frequency offset

  // ***********
  // decompress the data

  if (compressed_data && data_size > 0)
  {


      // todo:


  }

  // ***********

  if (packet_type == PACKET_TYPE_NONE)
    return;

  if (packet_type == PACKET_TYPE_DISCONNECT)
  {
      conn->link_state = LINK_DISCONNECTED;
      LINK_LED_OFF;
      return;
  }

  if (packet_type == PACKET_TYPE_CONNECT)
  {
      LINK_LED_ON;

//      fifoBuf_init(&conn->tx_fifo_buffer, conn->tx_buffer, PH_FIFO_BUFFER_SIZE);
//      fifoBuf_init(&conn->rx_fifo_buffer, conn->rx_buffer, PH_FIFO_BUFFER_SIZE);

      conn->tx_packet_timer = 0;

      conn->tx_retry_counter = 0;
      conn->tx_retry_time = conn->tx_retry_time_slot_len + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len;

      conn->rx_sequence = header->tx_seq;
      conn->tx_sequence = 0;
      conn->tx_sequence_data_size = 0;

      conn->data_speed_timer = 0;
      conn->tx_data_speed_count = 0;
      conn->tx_data_speed = 0;
      conn->rx_data_speed_count = 0;
      conn->rx_data_speed = 0;

      conn->ping_time = 8000 + (random32 % 100) * 10;
      conn->fast_ping_time = 600 + (random32 % 50) * 10;
      conn->pinging = false;

      conn->rx_not_ready_mode = false;

      conn->ready_to_send_timer = -1;

      conn->not_ready_timer = -1;

      conn->link_state = LINK_CONNECTED;

      // send an ack back
      if (ph_sendPacket(connection_index, conn->send_encrypted, PACKET_TYPE_CONNECT_ACK, true))
      {
          conn->tx_packet_timer = 0;
      }

      return;
  }

  if (packet_type == PACKET_TYPE_CONNECT_ACK)
  {
      LINK_LED_ON;

      if (conn->link_state != LINK_CONNECTING)
      {	// reset the link
          ph_set_remote_serial_number(connection_index, conn->serial_number);
          return;
      }

      conn->tx_packet_timer = 0;

      conn->tx_retry_counter = 0;
      conn->tx_retry_time = conn->tx_retry_time_slot_len + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len;

      conn->rx_sequence = header->tx_seq;
      conn->tx_sequence = 0;
      conn->tx_sequence_data_size = 0;

      conn->data_speed_timer = 0;
      conn->tx_data_speed_count = 0;
      conn->tx_data_speed = 0;
      conn->rx_data_speed_count = 0;
      conn->rx_data_speed = 0;

      conn->ping_time = 8000 + (random32 % 100) * 10;
      conn->fast_ping_time = 600 + (random32 % 50) * 10;
      conn->pinging = false;

      conn->rx_not_ready_mode = false;

      conn->ready_to_send_timer = -1;

      conn->not_ready_timer = -1;

      conn->link_state = LINK_CONNECTED;

      return;
  }




  if (conn->link_state == LINK_CONNECTING)
  {	// we are trying to connect to them .. reply with a connect request packet
      if (ph_sendPacket(connection_index, conn->send_encrypted, PACKET_TYPE_CONNECT, true))
      {
          conn->tx_packet_timer = 0;
          conn->tx_retry_time = conn->tx_retry_time_slot_len * 4 + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len * 4;
      }
      return;
  }

  if (conn->link_state != LINK_CONNECTED)
  {	// they have sent us a packet when we are not in a connected state - start a connection
      ph_startConnect(connection_index, conn->serial_number);
      return;
  }




  // check to make sure it's a wanted packet type
  switch (packet_type)
  {
    case PACKET_TYPE_DATA:
    case PACKET_TYPE_DATA_ACK:
    case PACKET_TYPE_READY:
    case PACKET_TYPE_READY_ACK:
    case PACKET_TYPE_NOTREADY:
    case PACKET_TYPE_NOTREADY_ACK:
    case PACKET_TYPE_DATARATE:
    case PACKET_TYPE_DATARATE_ACK:
    case PACKET_TYPE_PING:
    case PACKET_TYPE_PONG:
    case PACKET_TYPE_ADJUST_TX_PWR:
    case PACKET_TYPE_ADJUST_TX_PWR_ACK:
      break;
    default:
      return;
  }





  if ((conn->tx_sequence_data_size > 0) && (header->rx_seq == (uint8_t)(conn->tx_sequence + 1)))
  {	// they received our last data packet

      // remove the data we have sent and they have acked
      fifoBuf_removeData(&conn->tx_fifo_buffer, conn->tx_sequence_data_size);

      conn->tx_sequence++;
      conn->tx_retry_counter = 0;
      conn->tx_sequence_data_size = 0;
      conn->not_ready_timer = -1;	// stop timer
  }

  uint16_t size = fifoBuf_getUsed(&conn->tx_fifo_buffer);	// the size of data waiting to be sent




  if (packet_type == PACKET_TYPE_DATA || packet_type == PACKET_TYPE_DATA_ACK)
  {
      if (packet_type == PACKET_TYPE_DATA && header->tx_seq == conn->rx_sequence)
      {	// the packet number is what we expected

          if (data_size > 0)
          {	// save the data

              conn->rx_data_speed_count += data_size * 8;	// + the number of data bits we just received

              uint16_t num = fifoBuf_getFree(&conn->rx_fifo_buffer);
              if (num < data_size)
              {	// error .. we don't have enough space left in our fifo buffer to save the data .. discard it and tell them to hold off a sec
//                      conn->rx_not_ready_mode = true;
              }
              else
              {	// save the received data into our fifo buffer
                  fifoBuf_putData(&conn->rx_fifo_buffer, data, data_size);
                  conn->rx_sequence++;
                  conn->rx_not_ready_mode = false;
              }
          }
      }

      if (size >= 200 || (conn->ready_to_send_timer >= 10 && size > 0) || (conn->tx_sequence_data_size > 0 && size > 0))
      {	// send data
          uint8_t pack_type = PACKET_TYPE_DATA;
          if (conn->rx_not_ready_mode)
            pack_type = PACKET_TYPE_NOTREADY;

          if (ph_sendPacket(connection_index, conn->send_encrypted, pack_type, true))
          {
              conn->tx_packet_timer = 0;
              conn->tx_retry_time = conn->tx_retry_time_slot_len + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len;
              return;
          }
      }

      if (packet_type == PACKET_TYPE_DATA)
      {	// send an ack back
          uint8_t pack_type = PACKET_TYPE_DATA_ACK;
          if (conn->rx_not_ready_mode)
            pack_type = PACKET_TYPE_NOTREADY_ACK;

          if (ph_sendPacket(connection_index, conn->send_encrypted, pack_type, true))
          {
              conn->tx_packet_timer = 0;
              conn->tx_retry_time = conn->tx_retry_time_slot_len + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len;
              return;
          }
      }

      return;
  }

  if (packet_type == PACKET_TYPE_READY)
  {
      conn->not_ready_timer = -1;	// stop timer

      // send an ack back
      if (ph_sendPacket(connection_index, conn->send_encrypted, PACKET_TYPE_READY_ACK, true))
      {
          conn->tx_packet_timer = 0;
          conn->tx_retry_time = conn->tx_retry_time_slot_len * 4 + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len * 4;
          return;
      }

      return;
  }

  if (packet_type == PACKET_TYPE_READY_ACK)
  {
      conn->rx_not_ready_mode = false;
      return;
  }

  if (packet_type == PACKET_TYPE_NOTREADY)
  {
//      conn->not_ready_timer = 0;	// start timer

      if (header->tx_seq == conn->rx_sequence)
      {	// the packet number is what we expected

          if (data_size > 0)
          {	// save the data

              conn->rx_data_speed_count += data_size * 8;	// + the number of data bits we just received

              uint16_t num = fifoBuf_getFree(&conn->rx_fifo_buffer);
              if (num < data_size)
              {	// error .. we don't have enough space left in our fifo buffer to save the data .. discard it and tell them to hold off a sec
//              conn->rx_not_ready_mode = true;
              }
              else
              {	// save the received data into our fifo buffer
                  fifoBuf_putData(&conn->rx_fifo_buffer, data, data_size);
                  conn->rx_sequence++;
                  conn->rx_not_ready_mode = false;
              }
          }
      }

      // send an ack back
      if (ph_sendPacket(connection_index, conn->send_encrypted, PACKET_TYPE_NOTREADY_ACK, true))
      {
          conn->tx_packet_timer = 0;
          conn->tx_retry_time = conn->tx_retry_time_slot_len * 4 + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len * 4;
          return;
      }

      return;
  }

  if (packet_type == PACKET_TYPE_NOTREADY_ACK)
  {
      return;
  }

  if (packet_type == PACKET_TYPE_PING)
  {	// send a pong back
      if (ph_sendPacket(connection_index, conn->send_encrypted, PACKET_TYPE_PONG, true))
      {
          conn->tx_packet_timer = 0;
          conn->tx_retry_time = conn->tx_retry_time_slot_len + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len;
      }
      return;
  }

  if (packet_type == PACKET_TYPE_PONG)
  {
      if (conn->pinging)
      {
          conn->pinging = false;
          conn->tx_retry_counter = 0;
      }
      return;
  }

  if (packet_type == PACKET_TYPE_DATARATE)
  {
      // send an ack back
      if (ph_sendPacket(connection_index, conn->send_encrypted, PACKET_TYPE_DATARATE_ACK, true))
      {
          conn->tx_packet_timer = 0;
          conn->tx_retry_time = conn->tx_retry_time_slot_len + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len;
      }
      return;
  }

  if (packet_type == PACKET_TYPE_DATARATE_ACK)
  {
      return;
  }

  if (packet_type == PACKET_TYPE_ADJUST_TX_PWR)
  {
      // send an ack back
      if (ph_sendPacket(connection_index, conn->send_encrypted, PACKET_TYPE_ADJUST_TX_PWR_ACK, true))
      {
          conn->tx_packet_timer = 0;
          conn->tx_retry_time = conn->tx_retry_time_slot_len + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len;
      }
      return;
  }

  if (packet_type == PACKET_TYPE_ADJUST_TX_PWR_ACK)
  {
      return;
  }

  // *********************
}

void ph_processRxPacket(void)
{
  uint32_t crc1, crc2;
  uint8_t key[AES_BLOCK_SIZE];
  register uint8_t *p;

  // ***********
  // fetch the received packet

  uint16_t packet_size = rfm22_receivedLength();
  if (packet_size == 0)
    return;					// nothing received

  if (packet_size > sizeof(ph_rx_buffer))
  {	// packet too big .. discard it
      rfm22_receivedDone();
      return;
  }

  rx_rssi_dBm = rfm22_receivedRSSI();	// fetch the packets signal stength
  rx_afc_Hz = rfm22_receivedAFCHz();	// fetch the packets frequency offset

  // copy the received packet into our own buffer
  memmove(ph_rx_buffer, rfm22_receivedPointer(), packet_size);

  rfm22_receivedDone();				// the received packet has been saved

  // *********************
  // if the 1st byte in the packet is not zero, then the packet is encrypted

  bool encrypted = (ph_rx_buffer[0] != 0);

  // ***********

  t_encrypted_packet *encrypted_packet = (t_encrypted_packet *)&ph_rx_buffer;			// point to the rx buffer
  t_unencrypted_packet *unencrypted_packet = (t_unencrypted_packet *)&ph_rx_buffer;	// point to the rx buffer

  t_packet_header *header;
  uint8_t *data;
  uint16_t min_packet_size;
  uint16_t max_data_size;

  if (encrypted)
  {
      header = (t_packet_header *)&encrypted_packet->header;
      data = (uint8_t *)&encrypted_packet->data;
      min_packet_size = AES_BLOCK_SIZE + sizeof(t_packet_header);
      max_data_size = sizeof(encrypted_packet->data);
  }
  else
  {
      header = (t_packet_header *)&unencrypted_packet->header;
      data = (uint8_t *)&unencrypted_packet->data;
      min_packet_size = 1 + sizeof(t_packet_header);
      max_data_size = sizeof(unencrypted_packet->data);
  }

  if (packet_size < min_packet_size)
  {	// packet too small .. discard it
      return;
  }

  random32 = updateCRC32(random32 ^ header->crc, 0xff);	// help randomize the random number

  // *********************
  // help to randomize the tx aes cbc bytes by using the received packet

  p = (uint8_t *)&ph_rx_buffer;
  for (uint16_t i = 0; i < packet_size; i += AES_BLOCK_SIZE)
  {
      for (int j = AES_BLOCK_SIZE - 1; j >= 0; j--)
        enc_cbc[j] ^= *p++;
  }

  // *********************
  // check the packet size

  if (encrypted)
  {
      if ((packet_size & (AES_BLOCK_SIZE - 1)) != 0)
        return;	// packet must be a multiple of 'AES_BLOCK_SIZE' bytes in length - for the aes decryption
  }

  // *********************
  // decrypt the packet

  if (encrypted)
  {
      p = (uint8_t *)encrypted_packet;						// point to the received packet

      // decrypt the cbc
      memmove(key, (void *)dec_aes_key, sizeof(key));			// fetch the decryption key
      aes_decrypt_cbc_128(p, key, NULL);						// decrypt the cbc bytes
      p += AES_BLOCK_SIZE;

      // decrypt the rest of the packet
      for (uint16_t i = AES_BLOCK_SIZE; i < packet_size; i += AES_BLOCK_SIZE)
      {
          memmove(key, (void *)dec_aes_key, sizeof(key));		// fetch the decryption key
          aes_decrypt_cbc_128(p, key, (void *)encrypted_packet->cbc);
          p += AES_BLOCK_SIZE;
      }
  }

  // *********************

#if defined(PACKET_DEBUG)
  DEBUG_PRINTF("rx packet: ");
  DEBUG_PRINTF("%s", encrypted ? "encrypted " : "unencrypted");
  if (encrypted)
  {
      for (int i = 0; i < AES_BLOCK_SIZE; i++)
        DEBUG_PRINTF("%02X", encrypted_packet->cbc[i]);
  }
  DEBUG_PRINTF(" %08X %08X %u %u %u %u %08X\r\n",
      header->source_id,
      header->destination_id,
      header->type,
      header->tx_seq,
      header->rx_seq,
      header->data_size,
      header->crc);

  if (header->data_size > 0)
  {
      DEBUG_PRINTF("rx packet [%u]: ", header->data_size);
      for (int i = 0; i < header->data_size; i++)
        DEBUG_PRINTF("%02X", data[i]);
      DEBUG_PRINTF("\r\n");
  }
#endif

  // *********************

  uint32_t data_size = header->data_size;

  if (packet_size < (min_packet_size + data_size))
    return;				// packet too small

#if defined(PACKET_DEBUG)
//      DEBUG_PRINTF("rx packet size: %d\r\n", packet_size);
#endif

  // *********************
  // check the packet is error free

  crc1 = header->crc;
  header->crc = 0;
  if (encrypted)
    crc2 = updateCRC32Data(0xffffffff, header, packet_size - AES_BLOCK_SIZE);
  else
    crc2 = updateCRC32Data(0xffffffff, header, packet_size - 1);
  if (crc1 != crc2)
  {	// corrupt packet
      #if defined(PACKET_DEBUG)
        if (encrypted)
          DEBUG_PRINTF("ENC-R-PACK corrupt %08X %08X\r\n", crc1, crc2);
        else
          DEBUG_PRINTF("R-PACK corrupt %08X %08X\r\n", crc1, crc2);
      #endif
      return;
  }

  // *********************
  // process the data

  ph_processPacket2(encrypted, header, data);

  // *********************
}

// *****************************************************************************
// do all the link/packet handling stuff - request connection/disconnection, send data, acks etc

void ph_processLinks(int connection_index)
{
  if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
    return;

  random32 = updateCRC32(random32, 0xff);

  t_connection *conn = &connection[connection_index];

  bool canTx = (!rfm22_transmitting() && rfm22_channelIsClear());// TRUE is we can transmit

  bool timeToRetry = (rfm22_txReady() && conn->tx_packet_timer >= conn->tx_retry_time);

  bool tomanyRetries = (conn->tx_retry_counter >= RETRY_RECONNECT_COUNT);

  if (conn->tx_retry_counter > 3)
	  conn->rx_rssi_dBm = -200;

  switch (conn->link_state)
  {
    case LINK_DISCONNECTED:
      if (!canTx)
      {
          conn->tx_packet_timer = 0;
          break;
      }

      if (!rfm22_txReady() || conn->tx_packet_timer < 60000)
        break;

      if (our_serial_number != 0 && conn->serial_number != 0)
      {	// try to reconnect with the remote modem
          ph_startConnect(connection_index, conn->serial_number);
          break;
      }

      break;

    case LINK_CONNECTING:
      if (!canTx)
      {
          conn->tx_packet_timer = 0;
          break;
      }

      if (!timeToRetry)
        break;

      if (ph_sendPacket(connection_index, conn->send_encrypted, PACKET_TYPE_CONNECT, false))
      {
          conn->tx_packet_timer = 0;
          conn->tx_retry_time = conn->tx_retry_time_slot_len * 4 + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len * 4;
          break;
      }

      break;

    case LINK_CONNECTED:
      if (!canTx)
      {
          conn->tx_packet_timer = 0;
          break;
      }

      if (!timeToRetry)
        break;

      if (tomanyRetries)
      {	// reset the link if we have sent tomany retries
          ph_startConnect(connection_index, conn->serial_number);
          break;
      }

      if (conn->pinging)
      {	// we are trying to ping them
          if (ph_sendPacket(connection_index, conn->send_encrypted, PACKET_TYPE_PING, false))
          {
              conn->tx_packet_timer = 0;
              conn->tx_retry_time = conn->tx_retry_time_slot_len * 4 + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len * 4;
          }
          break;
      }

      uint16_t ping_time = conn->ping_time;
      if (fast_ping) ping_time = conn->fast_ping_time;
      if (conn->tx_packet_timer >= ping_time)
      {	// start pinging
          if (ph_sendPacket(connection_index, conn->send_encrypted, PACKET_TYPE_PING, false))
          {
              conn->ping_time = 8000 + (random32 % 100) * 10;
              conn->fast_ping_time = 600 + (random32 % 50) * 10;
              conn->tx_packet_timer = 0;
              conn->tx_retry_time = conn->tx_retry_time_slot_len * 4 + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len * 4;
              conn->pinging = true;
          }
          break;
      }

      // ***********
      // exit rx not ready mode if we have space in our rx buffer for more data
/*
			if (conn->rx_not_ready_mode)
			{
				uint16_t size = fifoBuf_getFree(&conn->rx_fifo_buffer);
				if (size >= conn->rx_fifo_buffer.buf_size / 6)
				{	// leave 'rx not ready' mode
					if (ph_sendPacket(connection_index, conn->send_encrypted, PACKET_TYPE_READY, false))
					{
						conn->tx_packet_timer = 0;
						conn->tx_retry_time = conn->tx_retry_time_slot_len + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len;
						break;
					}
				}
			}
*/
			// ***********
			// send data packets

//			if (conn->not_ready_timer < 0)
      {
        uint16_t size = fifoBuf_getUsed(&conn->tx_fifo_buffer);

        if (size == 0)
          conn->ready_to_send_timer = -1;	// no data to send
        else
        if (conn->ready_to_send_timer < 0)
          conn->ready_to_send_timer = 0;	// start timer

        if (size >= 200 || (conn->ready_to_send_timer >= saved_settings.rts_time && size > 0) || (conn->tx_sequence_data_size > 0 && size > 0))
        {       // send data

            uint8_t pack_type = PACKET_TYPE_DATA;
            if (conn->rx_not_ready_mode)
              pack_type = PACKET_TYPE_NOTREADY;

            if (ph_sendPacket(connection_index, conn->send_encrypted, pack_type, false))
            {
                conn->tx_packet_timer = 0;
                conn->tx_retry_time = conn->tx_retry_time_slot_len + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len;
            }
            break;
        }
      }

      // ***********

      break;

    default:	// we should never end up here - maybe we should do a reboot?
      conn->link_state = LINK_DISCONNECTED;
/*
			// disable all interrupts
			PIOS_IRQ_Disable();

			// turn off all leds
			USB_LED_OFF;
			LINK_LED_OFF;
			RX_LED_OFF;
			TX_LED_OFF;

			PIOS_SYS_Reset();

			while (1);
*/
      break;
  }
}

// *****************************************************************************

void ph_setFastPing(bool fast)
{
	fast_ping = fast;
}

// *****************************************************************************

uint8_t ph_getCurrentLinkState(const int connection_index)
{
    if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
        return 0;

    return connection[connection_index].link_state;
}

// *****************************************************************************

uint16_t ph_getRetries(const int connection_index)
{
    if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
        return 0;

    return connection[connection_index].tx_retry_counter;
}

// *****************************************************************************

int16_t ph_getLastRSSI(const int connection_index)
{
    if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
        return 0;

    return connection[connection_index].rx_rssi_dBm;
}

int32_t ph_getLastAFC(const int connection_index)
{
    if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
        return 0;

    return connection[connection_index].rx_afc_Hz;
}

// *****************************************************************************
// set/get the carrier frequency

void ph_setNominalCarrierFrequency(uint32_t frequency_hz)
{
	rfm22_setNominalCarrierFrequency(frequency_hz);
}

uint32_t ph_getNominalCarrierFrequency(void)
{
	return rfm22_getNominalCarrierFrequency();
}

// *****************************************************************************
// set/get the RF datarate

void ph_setDatarate(uint32_t datarate_bps)
{
  rfm22_setDatarate(datarate_bps, TRUE);

  uint32_t ms = 1280000ul / rfm22_getDatarate();
  if (ms < 10) ms = 10;
  else
  if (ms > 32000) ms = 32000;

  for (int i = 0; i < PH_MAX_CONNECTIONS; i++)
    connection[i].tx_retry_time_slot_len = ms;
}

uint32_t ph_getDatarate(void)
{
  return rfm22_getDatarate();
}

// *****************************************************************************

void ph_setTxPower(uint8_t tx_power)
{
  rfm22_setTxPower(tx_power);
}

uint8_t ph_getTxPower(void)
{
  return rfm22_getTxPower();
}

// *****************************************************************************
// set the AES encryption key

void ph_set_AES128_key(const void *key)
{
	if (!key)
		return;

	memmove(aes_key, key, sizeof(aes_key));

	// create the AES decryption key
	aes_decrypt_key_128_create(aes_key, (void *)&dec_aes_key);
}

// *****************************************************************************

int ph_set_remote_serial_number(int connection_index, uint32_t sn)
{
  random32 = updateCRC32(random32, 0xff);

  if (ph_startConnect(connection_index, sn) >= 0)
  {
      t_connection *conn = &connection[connection_index];

      // wipe any user data present in the buffers
      fifoBuf_init(&conn->tx_fifo_buffer, conn->tx_buffer, PH_FIFO_BUFFER_SIZE);
      fifoBuf_init(&conn->rx_fifo_buffer, conn->rx_buffer, PH_FIFO_BUFFER_SIZE);

      return connection_index;
  }

  return -4;
}

void ph_set_remote_encryption(int connection_index, bool enabled, const void *key)
{
    if (connection_index < 0 || connection_index >= PH_MAX_CONNECTIONS)
        return;

    ph_set_AES128_key(key);

    connection[connection_index].send_encrypted = enabled;
}

// *****************************************************************************
// can be called from an interrupt if you wish.
// call this once every ms

void ph_1ms_tick(void)
{
	if (booting) return;

	if (saved_settings.mode == MODE_NORMAL)
	{
		// help randomize the encryptor cbc bytes
		register uint32_t *cbc = (uint32_t *)&enc_cbc;
		for (int i = 0; i < sizeof(enc_cbc) / 4; i++)
		{
			random32 = updateCRC32(random32, 0xff);
			*cbc++ ^= random32;
		}

		for (int i = 0; i < PH_MAX_CONNECTIONS; i++)
		{
			t_connection *conn = &connection[i];

			if (conn->tx_packet_timer < 0xffff)
				conn->tx_packet_timer++;

			if (conn->link_state == LINK_CONNECTED)
			{	// we are connected

				if (conn->ready_to_send_timer >= 0 && conn->ready_to_send_timer < 0x7fff)
					conn->ready_to_send_timer++;

				if (conn->not_ready_timer >= 0 && conn->not_ready_timer < 0x7fffffff)
					conn->not_ready_timer++;

				if (conn->data_speed_timer < 0xffff)
				{
					if (++conn->data_speed_timer >= 1000)
					{	// 1 second gone by
						conn->data_speed_timer = 0;
						conn->tx_data_speed = (conn->tx_data_speed + conn->tx_data_speed_count) >> 1;
						conn->tx_data_speed_count = 0;
						conn->rx_data_speed = (conn->rx_data_speed + conn->rx_data_speed_count) >> 1;
						conn->rx_data_speed_count = 0;
					}
				}
			}
			else
			{	// we are not connected
				if (conn->data_speed_timer) conn->data_speed_timer = 0;
				if (conn->tx_data_speed_count) conn->tx_data_speed_count = 0;
				if (conn->tx_data_speed) conn->tx_data_speed = 0;
				if (conn->rx_data_speed_count) conn->rx_data_speed_count = 0;
				if (conn->rx_data_speed) conn->rx_data_speed = 0;
			}
		}
	}
}

// *****************************************************************************
// call this as often as possible - not from an interrupt

void ph_process(void)
{
	if (booting) return;

	if (saved_settings.mode == MODE_NORMAL)
	{
		ph_processRxPacket();

		for (int i = 0; i < PH_MAX_CONNECTIONS; i++)
			ph_processLinks(i);
	}
	else
	{

	}
}

// *****************************************************************************

void ph_disconnectAll(void)
{
	for (int i = 0; i < PH_MAX_CONNECTIONS; i++)
	{
		random32 = updateCRC32(random32, 0xff);

		t_connection *conn = &connection[i];

		conn->serial_number = 0;

		conn->tx_sequence = 0;
		conn->tx_sequence_data_size = 0;

		conn->rx_sequence = 0;

		fifoBuf_init(&conn->tx_fifo_buffer, conn->tx_buffer, PH_FIFO_BUFFER_SIZE);
		fifoBuf_init(&conn->rx_fifo_buffer, conn->rx_buffer, PH_FIFO_BUFFER_SIZE);

		conn->link_state = LINK_DISCONNECTED;

		conn->tx_packet_timer = 0;

		conn->tx_retry_time_slots = 5;
		conn->tx_retry_time_slot_len = 40;
		conn->tx_retry_time = conn->tx_retry_time_slot_len * 4 + (random32 % conn->tx_retry_time_slots) * conn->tx_retry_time_slot_len * 4;
		conn->tx_retry_counter = 0;

		conn->data_speed_timer = 0;
		conn->tx_data_speed_count = 0;
		conn->tx_data_speed = 0;
		conn->rx_data_speed_count = 0;
		conn->rx_data_speed = 0;

		conn->ping_time = 8000 + (random32 % 100) * 10;
		conn->fast_ping_time = 600 + (random32 % 50) * 10;
		conn->pinging = false;

		conn->rx_not_ready_mode = false;

		conn->ready_to_send_timer = -1;

		conn->not_ready_timer = -1;

		conn->send_encrypted = false;

		conn->rx_rssi_dBm = -200;
		conn->rx_afc_Hz = 0;
	}
}

// *****************************************************************************

void ph_deinit(void)
{
	ph_disconnectAll();
}

void ph_init(uint32_t our_sn)
{
	our_serial_number = our_sn;	// remember our own serial number

	fast_ping = false;

	ph_disconnectAll();

	// set the AES encryption key using the default AES key
	ph_set_AES128_key(default_aes_key);

	// try too randomise the tx AES CBC bytes
	for (uint32_t j = 0, k = 0; j < 123 + (random32 & 1023); j++)
	{
		random32 = updateCRC32(random32, 0xff);
		enc_cbc[k] ^= random32 >> 3;
		if (++k >= sizeof(enc_cbc)) k = 0;
	}

	// ******

	rfm22_init_normal(saved_settings.min_frequency_Hz, saved_settings.max_frequency_Hz, rfm22_freqHopSize());

	rfm22_TxDataByte_SetCallback(ph_TxDataByteCallback);
	rfm22_RxData_SetCallback(ph_RxDataCallback);

	rfm22_setFreqCalibration(saved_settings.rf_xtal_cap);
	ph_setNominalCarrierFrequency(saved_settings.frequency_Hz);
	ph_setDatarate(saved_settings.max_rf_bandwidth);
	ph_setTxPower(saved_settings.max_tx_power);

	ph_set_remote_encryption(0, saved_settings.aes_enable, (const void *)saved_settings.aes_key);
	ph_set_remote_serial_number(0, saved_settings.destination_id);

	// ******
}

// *****************************************************************************
