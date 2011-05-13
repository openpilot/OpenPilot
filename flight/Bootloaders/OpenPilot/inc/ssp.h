/*******************************************************************
 *
 *	NAME: ssp.h
 *
 *
 *******************************************************************/
#ifndef SSP_H
#define SSP_H
/** INCLUDE FILES **/
#include <stdint.h>

/** LOCAL DEFINITIONS **/
#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#define SSP_TX_IDLE       	0   // not expecting a ACK packet (no current transmissions in progress)
#define SSP_TX_WAITING    	1   // waiting for a valid ACK to arrive
#define SSP_TX_TIMEOUT    	2   // failed to receive a valid ACK in the timeout period, after retrying.
#define SSP_TX_ACKED      	3   // valid ACK received before timeout period.
#define SSP_TX_BUFOVERRUN 	4   // amount of data to send execeds the transmission buffer sizeof
#define SSP_TX_BUSY       	5   // Attempted to start a transmission while a transmission was already in  progress.
//#define SSP_TX_FAIL    - failure...

#define	SSP_RX_IDLE			0
#define SSP_RX_RECEIVING 	1
#define SSP_RX_COMPLETE		2

// types of packet that can be received
#define SSP_RX_DATA       	5
#define SSP_RX_ACK        	6
#define SSP_RX_SYNCH      	7

typedef enum decodeState_ {
	decode_len1_e = 0,
	decode_seqNo_e,
	decode_data_e,
	decode_crc1_e,
	decode_crc2_e,
	decode_idle_e
} DecodeState_t;

typedef enum ReceiveState {
	state_escaped_e = 0, state_unescaped_e
} ReceiveState_t;

typedef struct {
	uint8_t *pbuff;
	uint16_t length;
	uint16_t crc;
	uint8_t seqNo;
} Packet_t;

typedef struct {

	uint8_t *rxBuf; // Buffer used to store rcv data
	uint16_t rxBufSize; // rcv buffer size.
	uint8_t *txBuf; // Length of data in buffer
	uint16_t txBufSize; // CRC for data in Packet buff
	uint16_t max_retry; // Maximum number of retrys for a single transmit.
	int32_t timeoutLen; //  how long to wait for each retry to succeed
	void (*pfCallBack)(uint8_t *, uint16_t); // call back function that is called when a full packet has been received
	int16_t (*pfSerialRead)(void); // function to call to read a byte from serial hardware
	void (*pfSerialWrite)( uint8_t); // function used to write a byte to serial hardware for transmission
	uint32_t (*pfGetTime)(void); // function returns time in number of seconds that has elapsed from a given reference point
} PortConfig_t;

typedef struct Port_tag {
	void (*pfCallBack)(uint8_t *, uint16_t); // call back function that is called when a full packet has been received
	int16_t (*pfSerialRead)(void); // function to read a character from the serial input stream
	void (*pfSerialWrite)( uint8_t); // function to write a byte to be sent out the serial port
	uint32_t (*pfGetTime)(void); // function returns time in number of seconds that has elapsed from a given reference point
	uint8_t retryCount; // how many times have we tried to transmit the 'send' packet
	uint8_t maxRetryCount; // max. times to try to transmit the 'send' packet
	int32_t timeoutLen; // how long to wait for each retry to succeed
	int32_t timeout; // current timeout. when 'time' reaches this point we have timed out
	uint8_t txSeqNo; // current 'send' packet sequence number
	uint16_t rxBufPos; //  current buffer position in the receive packet
	uint16_t rxBufLen; // number of 'data' bytes in the buffer
	uint8_t rxSeqNo; // current 'receive' packet number
	uint16_t rxBufSize; // size of the receive buffer.
	uint16_t txBufSize; // size of the transmit buffer.
	uint8_t *txBuf; // transmit buffer. REquired to store a copy of packet data in case a retry is needed.
	uint8_t *rxBuf; // receive buffer. Used to store data as a packet is received.
	uint16_t sendSynch; // flag to indicate that we should send a synchronize packet to the host
	// this is required when switching from the application to the bootloader
	// and vice-versa. This fixes the firwmare download timeout.
	// when this flag is set to true, the next time we send a packet we will first
	// send a synchronize packet.
	ReceiveState_t InputState;
	DecodeState_t DecodeState;
	uint16_t SendState;
	uint16_t crc;
	uint32_t RxError;
	uint32_t TxError;
	uint16_t flags;
} Port_t;

/** Public Data **/

/** PUBLIC FUNCTIONS **/
int16_t ssp_ReceiveProcess(Port_t *thisport);
int16_t ssp_SendProcess(Port_t *thisport);
uint16_t ssp_SendString(Port_t *thisport, char *str);
int16_t ssp_SendData(Port_t *thisport, const uint8_t * data,
		const uint16_t length);
void ssp_Init(Port_t *thisport, const PortConfig_t* const info);
int16_t ssp_ReceiveByte(Port_t *thisport);
uint16_t ssp_Synchronise(Port_t *thisport);

/** EXTERNAL FUNCTIONS **/

#endif
