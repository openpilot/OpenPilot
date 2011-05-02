/***********************************************************************************************************
 *
 *  NAME:          ssp.c
 *  DESCRIPTION:   simple serial protocol - packet based serial transport layer.
 *  AUTHOR:        Joe Hlebasko
 *  HISTORY:       Created 1/1/2010
 *
 * Packet Formats
 *   Format:
 *    +------+----+------+---------------------------+--------+
 *    | 225  | L1 | S#   | App Data (0-254 bytes)    | CRC 16 |
 *    +------+----+------+---------------------------+--------+
 *
 * 225 = sync byte, indicates start of a packet
 * L1 = 1 byte for size of data payload. (sequence number is part of data payload.)
 * S# = 1 byte for sequence number.
 * 			Seq of 0 = seq # synchronise request, forces other end to reset receive sequence number to 1.
 * 				sender of synchronise request will reset the tx seq number to 1
 * 			Seq # of 1..127 = normal data packets. Sequence number is incremented by for each transmitted
 * 				packet.  Rolls over from 127 to 1.
 * 			if most sig. bit is set then the packet is an ACK packet of data packet sequence number of the
 * 				lower 7 bits (1..127)
 * App Data may contain 0..254 bytes. The sequence number is consider part of the payload.
 * CRC 16 - 16 bits of CRC values of Sequence # and data bytes.
 *
 * Protocol has two types of packets: data and ack packets. ACK packets have the most sig. bit set in the
 * sequence number, this implies that valid sequence numbers are 1..127
 *
 * This protocol uses the concept of sequences numbers to determine if a given packet has been received. This
 * requires both devices to be able to synchronize sequence numbers. This is accomplished by sending a packet
 * length 1 and sequence number = 0. The receive then resets it's transmit sequence number to 1.
 *
 * ACTIVE_SYNCH is a version that will automatically send a synch request if it receives a synch packet. Only
 * one device in the communication should do otherwise you end up with an endless loops of synchronization.
 * Right now each side needs to manually issues a synch request.
 *
 * This protocol is best used in cases where one device is the master and the other is the slave, or a don't
 * speak unless spoken to type of approach.
 *
 * The following are items are required to initialize a port for communications:
 * 1. The number attempts for each packet
 * 2. time to wait for an ack.
 * 3. pointer to buffer to be used for receiving.
 * 4. pointer to a buffer to be used for transmission
 * 5. length of each buffer (rx and tx)
 * 6. Four functions:
 *  	1. write byte = writes a byte out the serial port (or other comm device)
 * 	2. read byte = retrieves a byte from the serial port. Returns -1 if a byte is not available
 * 	3. callback = function to call when a valid data packet has been received. This function is responsible
 * 		to do what needs to be done with the data when it is received. The primary mission of this function
 * 		should be to copy the data to a private buffer out of the working receive buffer to prevent overrun.
 * 		processing should be kept to a minimum.
 * 	4. get time = function should return the current time. Note that time units are not specified it just
 * 		needs to be some measure of time that increments as time passes by.  The timeout values for a given
 * 		port should the units used/returned by the get time function.
 *
 * All of the state information of a communication port is contained in a Port_t structure. This allows this
 * module to operature on multiple communication ports with a single code base.
 *
 * The ssp_ReceiveProcess and ssp_SendProcess functions need to be called to process data through the
 * respective state machines. Typical implementation would have a serial ISR to pull bytes out of the UART
 * and place into a circular buffer.  The serial read function would then pull bytes out this buffer
 * processing. The TX side has the write function placing bytes into a circular buffer with the TX ISR
 * pulling bytes out of the buffer and putting into the UART.  It is possible to run the receive process from
 * the receive ISR but care must be taken on processing data when it is received to avoid holding up the ISR
 * and sending ACK packets from the receive ISR.
 *
 ***********************************************************************************************************/

/** INCLUDE FILES **/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <pios.h>
#include "ssp.h"
/** PRIVATE DEFINITIONS **/
#define SYNC                        225                     // Sync character used in Serial Protocol
#define ESC                         224                     // ESC character used in Serial Protocol
#define ESC_SYNC                    1                       // ESC_SYNC character used in Serial Protocol
#define ACK_BIT                     0x80                    // Ack bit, bit 7 of sequence number, 1 = Acknowledge, 0 =
//    new packet
// packet location definitions.
#define LENGTH  0
#define SEQNUM  1
#define DATA	2

// Make larger sized integers from smaller sized integers
#define MAKEWORD16( ub, lb )  ((uint16_t)0x0000 | ((uint16_t)(ub) << 8) | (uint16_t)(lb) )
#define MAKEWORD32( uw, lw ) ((uint32_t)(0x0UL | ((uint32_t)(uw) << 16) | (uint32_t)(lw)) )
#define MAKEWORD32B( b3, b2, b1, b0 ) ((uint32_t)((uint32_t)(b3)<< 24) | ((uint32_t)(b2)<<16) | ((uint32_t)(b1)<<8) | ((uint32_t)(b0) )

// Used to extract smaller integers from larger sized intergers
#define LOWERBYTE( w )        (uint8_t)((w) & 0x00ff )
#define UPPERBYTE( w )        (uint8_t)(((w) & 0xff00) >> 8 )
#define UPPERWORD(lw)         (uint16_t)(((lw) & 0xffff0000) >> 16 )
#define LOWERWORD(lw)         (uint16_t)((lw) & 0x0000ffff)

// Macros to operate on a target and bitmask.
#define CLEARBIT( a, b )  ((a) = (a) & ~(b))
#define SETBIT( a, b )    ((a) = (a) | (b) )
#define	TOGGLEBIT(a,b)	  ((a) = (a) ^ (b) )

// test bit macros operate using a bit mask.
#define	ISBITSET( a, b )  ( ((a) & (b)) == (b) ? TRUE : FALSE )
#define	ISBITCLEAR( a, b) (	(~(a) & (b)) == (b) ? TRUE : FALSE )

/** PRIVATE FUNCTIONS **/
//static void   	sf_SendSynchPacket( Port_t *thisport );
static uint16_t sf_crc16(uint16_t crc, uint8_t data);
static void sf_write_byte(Port_t *thisport, uint8_t c);
static void sf_SetSendTimeout(Port_t *thisport);
static uint16_t sf_CheckTimeout(Port_t *thisport);
static int16_t sf_DecodeState(Port_t *thisport, uint8_t c);
static int16_t sf_ReceiveState(Port_t *thisport, uint8_t c);

static void sf_SendPacket(Port_t *thisport);
static void sf_SendAckPacket(Port_t *thisport, uint8_t seqNumber);
static void sf_MakePacket(uint8_t *buf, const uint8_t * pdata, uint16_t length,
		uint8_t seqNo);
static int16_t sf_ReceivePacket(Port_t *thisport);

/* Flag bit masks...*/
#define SENT_SYNCH		(0x01)
#define	ACK_RECEIVED	(0x02)
#define ACK_EXPECTED	(0x04)

#define SSP_AWAITING_ACK	0
#define SSP_ACKED			1
#define SSP_IDLE			2

/** PRIVATE DATA **/
static const uint16_t CRC_TABLE[] = { 0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301,
		0x03C0, 0x0280, 0xC241, 0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1,
		0xC481, 0x0440, 0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81,
		0x0E40, 0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
		0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 0x1E00,
		0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41, 0x1400, 0xD4C1,
		0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641, 0xD201, 0x12C0, 0x1380,
		0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040, 0xF001, 0x30C0, 0x3180, 0xF141,
		0x3300, 0xF3C1, 0xF281, 0x3240, 0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501,
		0x35C0, 0x3480, 0xF441, 0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0,
		0x3E80, 0xFE41, 0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881,
		0x3840, 0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
		0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40, 0xE401,
		0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 0x2200, 0xE2C1,
		0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041, 0xA001, 0x60C0, 0x6180,
		0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 0x6600, 0xA6C1, 0xA781, 0x6740,
		0xA501, 0x65C0, 0x6480, 0xA441, 0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01,
		0x6FC0, 0x6E80, 0xAE41, 0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1,
		0xA881, 0x6840, 0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80,
		0xBA41, 0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
		0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640, 0x7200,
		0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041, 0x5000, 0x90C1,
		0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241, 0x9601, 0x56C0, 0x5780,
		0x9741, 0x5500, 0x95C1, 0x9481, 0x5440, 0x9C01, 0x5CC0, 0x5D80, 0x9D41,
		0x5F00, 0x9FC1, 0x9E81, 0x5E40, 0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901,
		0x59C0, 0x5880, 0x9841, 0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1,
		0x8A81, 0x4A40, 0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80,
		0x8C41, 0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
		0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 };

/** EXTERNAL DATA **/

/** EXTERNAL FUNCTIONS **/

/** VERIFICATION FUNCTIONS **/

/***********************************************************************************************************/

/*!
 * \brief   Initializes the communication port for use
 * \param   thisport = pointer to port structure to initialize
 * \param	info = config struct with default values.
 * \return  None.
 *
 * \note
 * Must be called before calling the Send or REceive process functions.
 */

void ssp_Init(Port_t *thisport, const PortConfig_t* const info) {
	thisport->pfCallBack = info->pfCallBack;
	thisport->pfSerialRead = info->pfSerialRead;
	thisport->pfSerialWrite = info->pfSerialWrite;
	thisport->pfGetTime = info->pfGetTime;

	thisport->maxRetryCount = info->max_retry;
	thisport->timeoutLen = info->timeoutLen;
	thisport->txBufSize = info->txBufSize;
	thisport->rxBufSize = info->rxBufSize;
	thisport->txBuf = info->txBuf;
	thisport->rxBuf = info->rxBuf;
	thisport->retryCount = 0;
	thisport->sendSynch = FALSE; //TRUE;
	thisport->rxSeqNo = 255;
	thisport->txSeqNo = 255;
	thisport->SendState = SSP_IDLE;
}

/*!
 * \brief   Runs the send process, checks for receipt of ack, timeouts and resends if needed.
 * \param   thisport = which port to use
 * \return  SSP_TX_WAITING - waiting for a valid ACK to arrive
 * \return  SSP_TX_TIMEOUT - failed to receive a valid ACK in the timeout period, after retrying.
 * \return  SSP_TX_IDLE    - not expecting a ACK packet (no current transmissions in progress)
 * \return  SSP_TX_ACKED   - valid ACK received before timeout period.
 *
 * \note
 *
 */
int16_t ssp_SendProcess(Port_t *thisport) {
	int16_t value = SSP_TX_WAITING;

	if (thisport->SendState == SSP_AWAITING_ACK) {
		if (sf_CheckTimeout(thisport) == TRUE) {
			if (thisport->retryCount < thisport->maxRetryCount) {
				// Try again
				sf_SendPacket(thisport);
				sf_SetSendTimeout(thisport);
				value = SSP_TX_WAITING;
			} else {
				// Give up, # of trys has exceded the limit
#ifdef DEBUG_SSP
				char str[63]= {0};
				sprintf(str,"Send Timeout|");
				PIOS_COM_SendString(PIOS_COM_TELEM_USB,str);
#endif
				value = SSP_TX_TIMEOUT;
				CLEARBIT( thisport->flags, ACK_RECEIVED);
				thisport->SendState = SSP_IDLE;
			}
		} else {
			value = SSP_TX_WAITING;
		}
	} else if (thisport->SendState == SSP_ACKED) {
		SETBIT( thisport->flags, ACK_RECEIVED);
		value = SSP_TX_ACKED;
		thisport->SendState = SSP_IDLE;
	} else {
		thisport->SendState = SSP_IDLE;
		value = SSP_TX_IDLE;
	}
	return value;
}

/*!
 * \brief   Runs the receive process. fetches a byte at a time and runs the byte through the protocol receive state machine.
 * \param   thisport - which port to use.
 * \return  receive status.
 *
 * \note
 *
 */
int16_t ssp_ReceiveProcess(Port_t *thisport) {
	int16_t b;
	int16_t packet_status = SSP_RX_IDLE;

	do {
		b = thisport->pfSerialRead(); // attempt to read a char from the serial buffer
		if (b != -1) {
			packet_status = sf_ReceiveState(thisport, b); // process the newly received byte in the receive state machine
		}
		// keep going until either we received a full packet or there are no more bytes to process
	} while (packet_status != SSP_RX_COMPLETE && b != -1);
	return packet_status;
}

/*!
 * \brief   processes a single byte through the receive state machine.
 * \param   thisport = which port to use
 * \return  current receive status
 *
 * \note
 *
 */

int16_t ssp_ReceiveByte(Port_t *thisport) {
	int16_t b;
	int16_t packet_status = SSP_RX_IDLE;

	b = thisport->pfSerialRead();
	if (b != -1) {
		packet_status = sf_ReceiveState(thisport, b);
	}
	return packet_status;
}

/*!
 * \brief   Sends a data packet and blocks until timeout or ack is received.
 * \param   thisport = which port to use
 * \param	data = pointer to data to send
 * \param	length = number of data bytes to send. Must be less than 254
 * \return  true = ack was received within number of retries
 * \return	false = ack was not received.
 *
 * \note
 *
 */
uint16_t ssp_SendDataBlock(Port_t *thisport, uint8_t *data, uint16_t length) {
	int16_t packet_status = SSP_TX_WAITING;
	uint16_t retval = FALSE;

	packet_status = ssp_SendData(thisport, data, length); // send the data
	while (packet_status == SSP_TX_WAITING) { // check the status
		(void) ssp_ReceiveProcess(thisport); // process any bytes received.
		packet_status = ssp_SendProcess(thisport); // check the send status
	}
	if (packet_status == SSP_TX_ACKED) { // figure out what happened to the packet
		retval = TRUE;
	} else {
		retval = FALSE;
	}
	return retval;
}

/*!
 * \brief   sends a chunk of data and does not block
 * \param   thisport = which port to use
 * \param	data = pointer to data to send
 * \param	length = number of bytes to send
 * \return	SSP_TX_BUFOVERRUN = tried to send too much data
 * \return	SSP_TX_WAITING = data sent and waiting for an ack to arrive
 * \return	SSP_TX_BUSY = a packet has already been sent, but not yet acked
 *
 * \note
 *
 */
int16_t ssp_SendData(Port_t *thisport, const uint8_t *data,
		const uint16_t length) {

	int16_t value = SSP_TX_WAITING;

	if ((length + 2) > thisport->txBufSize) {
		// TRYING to send too much data.
		value = SSP_TX_BUFOVERRUN;
	} else if (thisport->SendState == SSP_IDLE) {
#ifdef ACTIVE_SYNCH
		if( thisport->sendSynch == TRUE ) {
			sf_SendSynchPacket(thisport);
		}
#endif

#ifdef SYNCH_SEND
		if( length == 0 ) {
			// TODO this method could allow a task/user to start a synchronisation step if a zero is mistakenly passed to this function.
			//      could add a check for a NULL data pointer, or use some sort of static flag that can only be accessed by a static function
			//      that must be called before calling this function.
			// we are attempting to send a synch packet
			thisport->txSeqNo = 0; // make this zero to cause the other end to re-synch with us
			SETBIT(thisport->flags, SENT_SYNCH);
		} else {
			// we are sending a data packet
			CLEARBIT( thisport->txSeqNo, ACK_BIT ); // make sure we are not sending a ACK packet
			thisport->txSeqNo++; // update the sequence number.
			if( thisport->txSeqNo > 0x7F) { // check for sequence number rollover
				thisport->txSeqNo = 1; // if we do have rollover then reset to 1 not zero,
				// zero is reserviced for synchronization requests
			}
		}

#else
		CLEARBIT( thisport->txSeqNo, ACK_BIT ); // make sure we are not sending a ACK packet
		thisport->txSeqNo++; // update the sequence number.
		if (thisport->txSeqNo > 0x7F) { // check for sequence number rollover
			thisport->txSeqNo = 1; // if we do have rollover then reset to 1 not zero,
			// zero is reserved for synchronization requests
		}
#endif
		CLEARBIT( thisport->flags, ACK_RECEIVED);
		thisport->SendState = SSP_AWAITING_ACK;
		value = SSP_TX_WAITING;
		thisport->retryCount = 0; // zero out the retry counter for this transmission
		sf_MakePacket(thisport->txBuf, data, length, thisport->txSeqNo);
		sf_SendPacket(thisport); // punch out the packet to the serial port
		sf_SetSendTimeout(thisport); // do the timeout values
#ifdef DEBUG_SSP
		char str[63]= {0};
		sprintf(str,"Sent DATA PACKET:%d|",thisport->txSeqNo);
		PIOS_COM_SendString(PIOS_COM_TELEM_USB,str);
#endif
	} else {
		// error we are already sending a packet. Need to wait for the current packet to be acked or timeout.
#ifdef DEBUG_SSP
		char str[63]= {0};
		sprintf(str,"Error sending TX was busy|");
		PIOS_COM_SendString(PIOS_COM_TELEM_USB,str);
#endif
		value = SSP_TX_BUSY;
	}
	return value;
}

/*!
 * \brief   Attempts to synchronize the sequence numbers with the other end of the connectin.
 * \param   thisport = which port to use
 * \return  true = success
 * \return	false = failed to receive an ACK to our synch request
 *
 * \note
 * A. send a packet with a sequence number equal to zero
 * B. if timed out then:
 * 		send synch packet again
 * 		increment try counter
 * 		if number of tries exceed maximum try limit then exit
 * C. goto A
 */
uint16_t ssp_Synchronise(Port_t *thisport) {
	int16_t packet_status;
	uint16_t retval = FALSE;

#ifndef USE_SENDPACKET_DATA
	thisport->txSeqNo = 0; // make this zero to cause the other end to re-synch with us
	SETBIT(thisport->flags, SENT_SYNCH);
	// TODO - should this be using ssp_SendPacketData()??
	sf_MakePacket(thisport->txBuf, NULL, 0, thisport->txSeqNo); // construct the packet
	sf_SendPacket(thisport);
	sf_SetSendTimeout(thisport);
	thisport->SendState = SSP_AWAITING_ACK;
	packet_status = SSP_TX_WAITING;
#else
	packet_status = ssp_SendData( thisport, NULL, 0 );
#endif
	while (packet_status == SSP_TX_WAITING) { // we loop until we time out.
		(void) ssp_ReceiveProcess(thisport); // do the receive process
		packet_status = ssp_SendProcess(thisport); // do the send process
	}
	thisport->sendSynch = FALSE;
	switch (packet_status) {
	case SSP_TX_ACKED:
		retval = TRUE;
		break;
	case SSP_TX_BUSY: // intentional fall through.
	case SSP_TX_TIMEOUT: // intentional fall through.
	case SSP_TX_BUFOVERRUN:
		retval = FALSE;
		break;
	default:
		retval = FALSE;
		break;
	};
	return retval;
}

/*!
 * \brief   sends out a preformatted packet for a give port
 * \param   thisport = which port to use.
 * \return  none.
 *
 * \note
 * Packet should be formed through the use of sf_MakePacket before calling this function.
 */
static void sf_SendPacket(Port_t *thisport) {
	// add 3 to packet data length for: 1 length + 2 CRC (packet overhead)
	uint8_t packetLen = thisport->txBuf[LENGTH] + 3;

	// use the raw serial write function so the SYNC byte does not get 'escaped'
	thisport->pfSerialWrite(SYNC);
	for (uint8_t x = 0; x < packetLen; x++) {
		sf_write_byte(thisport, thisport->txBuf[x]);
	}
	thisport->retryCount++;
}

/*!
 * \brief   converts data to transport layer protocol packet format.
 * \param   txbuf = buffer to use when forming the packet
 * \param	pdata = pointer to data to use
 * \param	length = number of bytes to use
 * \param	seqNo = sequence number of this packet
 * \return  none.
 *
 * \note
 *  1. This function does not try to interpret ACK or SYNCH packets.  This should
 *      be done by the caller of this function.
 *  2. This function will attempt to format all data upto the size of the tx buffer.
 *      Any extra data beyond that will be ignored.
 *  3. TODO: Should this function return an error if data length to be sent is greater th tx buffer size?
 *
 */
void sf_MakePacket(uint8_t *txBuf, const uint8_t * pdata, uint16_t length,
		uint8_t seqNo) {
	uint16_t crc = 0xffff;
	uint16_t bufPos = 0;
	uint8_t b;

	// add 1 for the seq. number
	txBuf[LENGTH] = length + 1;
	txBuf[SEQNUM] = seqNo;
	crc = sf_crc16(crc, seqNo);

	length = length + 2; // add two for the length and seqno bytes which are added before the loop.
	for (bufPos = 2; bufPos < length; bufPos++) {
		b = *pdata++;
		txBuf[bufPos] = b;
		crc = sf_crc16(crc, b); // update CRC value
	}
	txBuf[bufPos++] = LOWERBYTE(crc);
	txBuf[bufPos] = UPPERBYTE(crc);

}

/*!
 * \brief   sends out an ack packet to given sequence number
 * \param   thisport = which port to use
 * \param	seqNumber = sequence number of the packet we would like to ack
 * \return  none.
 *
 * \note
 *
 */

static void sf_SendAckPacket(Port_t *thisport, uint8_t seqNumber) {
#ifdef DEBUG_SSP
	char str[63]= {0};
	sprintf(str,"Sent ACK PACKET:%d|",seqNumber);
	PIOS_COM_SendString(PIOS_COM_TELEM_USB,str);
#endif
	uint8_t AckSeqNumber = SETBIT( seqNumber, ACK_BIT );

	// create the packet, note we pass AckSequenceNumber directly
	sf_MakePacket(thisport->txBuf, NULL, 0, AckSeqNumber);
	sf_SendPacket(thisport);
	// we don't set the timeout for an ACK because we don't ACK our ACKs in this protocol
}

/*!
 * \brief   writes a byte out the output channel. Adds escape byte where needed
 * \param   thisport = which port to use
 * \param	c = byte to send
 * \return  none.
 *
 * \note
 *
 */
static void sf_write_byte(Port_t *thisport, uint8_t c) {
	if (c == SYNC) { // check for SYNC byte
		thisport->pfSerialWrite(ESC); // since we are not starting a packet we must ESCAPE the SYNCH byte
		thisport->pfSerialWrite(ESC_SYNC); // now send the escaped synch char
	} else if (c == ESC) { // Check for ESC character
		thisport->pfSerialWrite(ESC); // if it is, we need to send it twice
		thisport->pfSerialWrite(ESC);
	} else {
		thisport->pfSerialWrite(c); // otherwise write the byte to serial port
	}
}

/************************************************************************************************************
 *
 *  NAME:          uint16_t ssp_crc16( uint16_t crc, uint16_t data )
 *  DESCRIPTION:   Uses crc_table to calculate new crc
 *  ARGUMENTS:
 *        arg1:    crc
 *        arg2:    data - byte to calculate into CRC
 *  RETURN:        New crc
 *  CREATED:    5/8/02
 *
 *************************************************************************************************************/
/*!
 * \brief   calculates the new CRC value for 'data'
 * \param   crc = current CRC value
 * \param	data = new byte
 * \return  updated CRC value
 *
 * \note
 *
 */

static uint16_t sf_crc16(uint16_t crc, uint8_t data) {
	return (crc >> 8) ^ CRC_TABLE[(crc ^ data) & 0x00FF];
}

/*!
 * \brief   sets the timeout for the given packet
 * \param   thisport = which port to use
 * \return  none.
 *
 * \note
 *
 */

static void sf_SetSendTimeout(Port_t *thisport) {
	uint32_t timeout;
	timeout = thisport->pfGetTime() + thisport->timeoutLen;
	thisport->timeout = timeout;
}

/*!
 * \brief   checks to see if a timeout occured
 * \param   thisport = which port to use
 * \return  true = a timeout has occurred
 * \return	false = has not timed out
 *
 * \note
 *
 */
static uint16_t sf_CheckTimeout(Port_t *thisport) {
	uint16_t retval = FALSE;
	uint32_t current_time;

	current_time = thisport->pfGetTime();
	if (current_time > thisport->timeout) {
		retval = TRUE;
	}
	return retval;
}

/****************************************************************************
 *  NAME:   sf_ReceiveState
 *  DESC:   Implements the receive state handling code for escaped and unescaped data
 *  ARGS:	thisport - which port to operate on
 *  		c - incoming byte
 *  RETURN:
 *  CREATED:
 *  NOTES:
 *  1. change from using pointer to functions.
 ****************************************************************************/
/*!
 * \brief   implements the receive state handling code for escaped and unescaped data
 * \param   thisport = which port to use
 * \param	c = byte to process through the receive state machine
 * \return  receive status
 *
 * \note
 *
 */
static int16_t sf_ReceiveState(Port_t *thisport, uint8_t c) {
	int16_t retval = SSP_RX_RECEIVING;

	switch (thisport->InputState) {
	case state_unescaped_e:
		if (c == SYNC) {
			thisport->DecodeState = decode_len1_e;
		} else if (c == ESC) {
			thisport->InputState = state_escaped_e;
		} else {
			retval = sf_DecodeState(thisport, c);
		}
		break; // end of unescaped state.
	case state_escaped_e:
		thisport->InputState = state_unescaped_e;
		if (c == SYNC) {
			thisport->DecodeState = decode_len1_e;
		} else if (c == ESC_SYNC) {
			retval = sf_DecodeState(thisport, SYNC);
		} else {
			retval = sf_DecodeState(thisport, c);
		}
		break; // end of the escaped state.
	default:
		break;
	}
	return retval;
}

/****************************************************************************
 *  NAME:   sf_DecodeState
 *  DESC:   Implements the receive state finite state machine
 *  ARGS:	thisport - which port to operate on
 *  		c - incoming byte
 *  RETURN:
 *  CREATED:
 *  NOTES:
 *  1. change from using pointer to functions.
 ****************************************************************************/

/*!
 * \brief   implements the receiving decoding state machine
 * \param   thisport = which port to use
 * \param	c = byte to process
 * \return  receive status
 *
 * \note
 *
 */
static int16_t sf_DecodeState(Port_t *thisport, uint8_t c) {
	int16_t retval;
	switch (thisport->DecodeState) {
	case decode_idle_e:
		// 'c' is ignored in this state as the only way to leave the idle state is
		// recognition of the SYNC byte in the sf_ReceiveState function.
		retval = SSP_RX_IDLE;
		break;
	case decode_len1_e:
		thisport->rxBuf[LENGTH] = c;
		thisport->rxBufLen = c;
		if (thisport->rxBufLen <= thisport->rxBufSize) {
			thisport->DecodeState = decode_seqNo_e;
			retval = SSP_RX_RECEIVING;
		} else {
			thisport->DecodeState = decode_idle_e;
			retval = SSP_RX_IDLE;
		}
		break;
	case decode_seqNo_e:
		thisport->rxBuf[SEQNUM] = c;
		thisport->crc = 0xffff;
		thisport->rxBufLen--; // subtract 1 for the seq. no.
		thisport->rxBufPos = 2;

		thisport->crc = sf_crc16(thisport->crc, c);
		if (thisport->rxBufLen > 0) {
			thisport->DecodeState = decode_data_e;
		} else {
			thisport->DecodeState = decode_crc1_e;
		}
		retval = SSP_RX_RECEIVING;
		break;
	case decode_data_e:
		thisport->rxBuf[(thisport->rxBufPos)++] = c;
		thisport->crc = sf_crc16(thisport->crc, c);
		if (thisport->rxBufPos == (thisport->rxBufLen + 2)) {
			thisport->DecodeState = decode_crc1_e;
		}
		retval = SSP_RX_RECEIVING;
		break;
	case decode_crc1_e:
		thisport->crc = sf_crc16(thisport->crc, c);
		thisport->DecodeState = decode_crc2_e;
		retval = SSP_RX_RECEIVING;
		break;
	case decode_crc2_e:
		thisport->DecodeState = decode_idle_e;
		// verify the CRC value for the packet
		if (sf_crc16(thisport->crc, c) == 0) {
			// TODO shouldn't the return value of sf_ReceivePacket() be checked?
			sf_ReceivePacket(thisport);
			retval = SSP_RX_COMPLETE;
		} else {
			thisport->RxError++;
			retval = SSP_RX_IDLE;
		}
		break;
	default:
		thisport->DecodeState = decode_idle_e; // unknown state so reset to idle state and wait for the next start of a packet.
		retval = SSP_RX_IDLE;
		break;
	}
	return retval;
}

/************************************************************************************************************
 *
 *  NAME:             int16_t sf_ReceivePacket( )
 *  DESCRIPTION:      Receive one packet, assumed that data is in rec.buff[]
 *   ARGUMENTS:
 *   RETURN:          0 . no new packet was received, could be ack or same packet
 *                    1 . new packet received
 *                    SSP_PACKET_?
 *                    SSP_PACKET_COMPLETE
 *                    SSP_PACKET_ACK
 *   CREATED:         5/8/02
 *
 *************************************************************************************************************/
/*!
 * \brief   receive one packet. calls the callback function if needed.
 * \param   thisport = which port to use
 * \return  true = valid data packet received.
 * \return	false = otherwise
 *
 * \note
 *
 *	Created: Oct 7, 2010 12:07:22 AM by joe
 */

static int16_t sf_ReceivePacket(Port_t *thisport) {
	int16_t value = FALSE;

	if (ISBITSET(thisport->rxBuf[SEQNUM], ACK_BIT )) {
		//  Received an ACK packet, need to check if it matches the previous sent packet
		if ((thisport->rxBuf[SEQNUM] & 0x7F) == (thisport->txSeqNo & 0x7f)) {
			//  It matches the last packet sent by us
			SETBIT( thisport->txSeqNo, ACK_BIT );
			thisport->SendState = SSP_ACKED;
#ifdef DEBUG_SSP
			char str[63]= {0};
			sprintf(str,"Received ACK:%d|",(thisport->txSeqNo & 0x7F));
			PIOS_COM_SendString(PIOS_COM_TELEM_USB,str);
#endif
			value = FALSE;
		}
		// else ignore the ACK packet
	} else {
		//  Received a 'data' packet, figure out what type of packet we received...
		if (thisport->rxBuf[SEQNUM] == 0) {
#ifdef DEBUG_SSP
			PIOS_COM_SendString(PIOS_COM_TELEM_USB,"Received SYNC Request|");
#endif
			// Synchronize sequence number with host
#ifdef ACTIVE_SYNCH
			thisport->sendSynch = TRUE;
#endif
			sf_SendAckPacket(thisport, thisport->rxBuf[SEQNUM]);
			thisport->rxSeqNo = 0;
			value = FALSE;
		} else if (thisport->rxBuf[SEQNUM] == thisport->rxSeqNo) {
			// Already seen this packet, just ack it, don't act on the packet.
			sf_SendAckPacket(thisport, thisport->rxBuf[SEQNUM]);
			value = FALSE;
		} else {
			//New Packet
			thisport->rxSeqNo = thisport->rxBuf[SEQNUM];
			// Let the application do something with the data/packet.
			if (thisport->pfCallBack != NULL) {
#ifdef DEBUG_SSP
				char str[63]= {0};
				sprintf(str,"Received DATA PACKET:%d [0]=%d %d %d|",thisport->rxSeqNo,(uint8_t)thisport->rxBuf[2],(uint8_t)thisport->rxBuf[3],(uint8_t)thisport->rxBuf[4]);
				PIOS_COM_SendString(PIOS_COM_TELEM_USB,str);
#endif
				// skip the first two bytes (length and seq. no.) in the buffer.
				thisport->pfCallBack(&(thisport->rxBuf[2]), thisport->rxBufLen);
			}
			// after we send the ACK, it is possible for the host to send a new packet.
			// Thus the application needs to copy the data and reset the receive buffer
			// inside of thisport->pfCallBack()
			sf_SendAckPacket(thisport, thisport->rxBuf[SEQNUM]);
			value = TRUE;
		}
	}
	return value;
}

