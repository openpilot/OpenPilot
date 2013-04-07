#ifndef QSSP_H
#define QSSP_H
#include <stdint.h>
#include "port.h"
#include "common.h"
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


typedef struct
{
    uint8_t 	*pbuff;
    uint16_t 	length;
    uint16_t 	crc;
    uint8_t 	seqNo;
} Packet_t;

typedef struct {

    uint8_t 	*rxBuf;                             	// Buffer used to store rcv data
    uint16_t 	rxBufSize;                         	// rcv buffer size.
    uint8_t 	*txBuf;                            	// Length of data in buffer
    uint16_t 	txBufSize;                        	// CRC for data in Packet buff
    uint16_t 	max_retry;                             	// Maximum number of retrys for a single transmit.
    int32_t 	timeoutLen;                          	//  how long to wait for each retry to succeed
    // function returns time in number of seconds that has elapsed from a given reference point
}	PortConfig_t;





/** Public Data **/




/** EXTERNAL FUNCTIONS **/

class qssp
{
private:
    port * thisport;
    decodeState_ DecodeState_t;
    /** PRIVATE FUNCTIONS **/
    //static void   	sf_SendSynchPacket( Port_t *thisport );
    uint16_t sf_crc16( uint16_t crc, uint8_t data );
    void   	sf_write_byte(uint8_t c );
    void   	sf_SetSendTimeout();
    uint16_t sf_CheckTimeout();
    int16_t 	sf_DecodeState(uint8_t c );
    int16_t 	sf_ReceiveState(uint8_t c );

    void   	sf_SendPacket();
    void   	sf_SendAckPacket(uint8_t seqNumber);
    void     sf_MakePacket( uint8_t *buf, const uint8_t * pdata, uint16_t length, uint8_t seqNo );
    int16_t 	sf_ReceivePacket();
    uint16_t ssp_SendDataBlock(uint8_t *data, uint16_t length );
    bool debug;
public:
    /** PUBLIC FUNCTIONS **/
     virtual void pfCallBack( uint8_t *, uint16_t);	// call back function that is called when a full packet has been received
    int16_t     ssp_ReceiveProcess();
    int16_t 	ssp_SendProcess();
    uint16_t    ssp_SendString(char *str );
    int16_t     ssp_SendData( const uint8_t * data,const uint16_t length );
    void        ssp_Init( const PortConfig_t* const info);
    int16_t		ssp_ReceiveByte( );
    uint16_t 	ssp_Synchronise(  );
    qssp(port * info,bool debug);
};

#endif // QSSP_H
