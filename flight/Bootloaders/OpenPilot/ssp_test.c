// test functions for the SSP module.
// this module performs unit test on the SSP functions.

#include "ssp.h"
#include "buffer.h"

#ifndef true
#define true    1
#endif
#ifndef false
#define false   0
#endif

#define MAX_PACKET_DATA_LEN	255
#define MAX_PACKET_BUF_SIZE	(1+1+255+2)
// master buffers...
uint8_t	masterTxBuf[MAX_PACKET_BUF_SIZE];
uint8_t	masterRxBuf[MAX_PACKET_BUF_SIZE];

// slave buffers...
uint8_t	slaveTxBuf[MAX_PACKET_BUF_SIZE];
uint8_t	slaveRxBuf[MAX_PACKET_BUF_SIZE];

void 		masterCallBack(uint8_t *buf, uint16_t len);
int16_t		masterSerialRead(void);
void		masterSerialWrite(uint8_t);
uint32_t 	masterGetTime(void);

void 		slaveCallBack(uint8_t *buf, uint16_t len);
int16_t		slaveSerialRead(void);
void		slaveSerialWrite(uint8_t);
uint32_t 	slaveGetTime(void);

PortConfig_t	masterPortConfig = {
	.rxBuf 		= masterRxBuf,
	.rxBufSize 	= MAX_PACKET_DATA_LEN,
	.txBuf 		= masterTxBuf,
	.txBufSize 	= 255,
	.max_retry	= 3,
	.timeoutLen	= 100,
	.pfCallBack	= masterCallBack,
	.pfSerialRead = masterSerialRead,
	.pfSerialWrite = masterSerialWrite,
	.pfGetTime	= masterGetTime,
};
PortConfig_t	slavePortConfig = {
	.rxBuf 		= slaveRxBuf,
	.rxBufSize 	= MAX_PACKET_DATA_LEN,
	.txBuf 		= slaveTxBuf,
	.txBufSize 	= 255,
	.max_retry	= 3,
	.timeoutLen	= 100,
	.pfCallBack	= slaveCallBack,
	.pfSerialRead = slaveSerialRead,
	.pfSerialWrite = slaveSerialWrite,
	.pfGetTime	= slaveGetTime,
};

Port_t 	master_port;
Port_t	slave_port;

cBuffer	m2sBuffer;
cBuffer	s2mBuffer;

#define BUFFER	1024

// buffer space for the simulated serial buffers.
uint8_t	m2sDataBuffer[BUFFER];
uint8_t	s2mDataBuffer[BUFFER];

void ssp_test(void)
{
	uint8_t	masterSendBuf[255];
//	uint8_t	slaveSendBuf[255];
	Port_t *master = &master_port;
	Port_t *slave = &slave_port;
	int16_t	packet_status;
    int16_t retval;
    
    uint8_t master_respond = TRUE;
    uint8_t slave_respond = TRUE;
    uint8_t master_send_respond = TRUE;
    
	bufferInit(&m2sBuffer, m2sDataBuffer, BUFFER);
	bufferInit(&s2mBuffer, s2mDataBuffer, BUFFER);

	ssp_Init( master, &masterPortConfig);
	ssp_Init( slave, &slavePortConfig);

	masterSendBuf[0] = 0;
	masterSendBuf[1] = 1;
	masterSendBuf[2] = 2;
	masterSendBuf[3] = 3;
	masterSendBuf[4] = 4;

	ssp_Synchronise(master);
	while (1) {
        packet_status = ssp_SendData( master, masterSendBuf, 5 );		// send the data
    	while( packet_status == SSP_TX_WAITING )  {	// check the status
            if( slave_respond == TRUE ) {
                (void)ssp_ReceiveProcess(slave);          // process simulated input to the slave
            }
            if( master_respond == TRUE ) {
                (void)ssp_ReceiveProcess( master );		// process any bytes received.
            }
            if( master_send_respond == TRUE ) {
                packet_status = ssp_SendProcess( master );// check the packet
            }
        }
        if (packet_status == SSP_TX_ACKED ) {
            retval = TRUE;
        } else {
          	// figure out what happened to the packet
            // possible errors are: timeout, busy, bufoverrun (tried to send too much data.
            retval = FALSE;
        }
        // just a more explicit way to see what happened...
        switch( packet_status ) {
        case SSP_TX_ACKED:
            // quick data manipulation to see something different...
            for (int32_t x = 0; x < 5; ++x) {
    			masterSendBuf[x] += 5;
    		}
            retval = TRUE;
            break;
        case SSP_TX_BUSY:
            retval = FALSE;
            break;
        case SSP_TX_TIMEOUT:
            retval = FALSE;
            break;
        case SSP_TX_BUFOVERRUN:
            retval = false;
            break;
        default:
            retval = -3;
            break;
        }
#ifdef OLD_CODE    
        do {
            packetStatus = ssp_SendPacketData( master, masterSendBuf, 5);
            if( packetStatus == SSP_TX_FAIL) {
                ssp_ReceiveProcess(slave);
                ssp_ReceiveProcess(master);
                ssp_SendProcess(master);
            }
        } while( packetStatus != SSP_TX_WAITING );
        
		do {
			// let the slave process simulated input.
			ssp_ReceiveProcess(slave);
			// process simulated input from the slave to master. Slave 'may' have sent an ACK
			if( ssp_ReceiveProcess(master) == SSP_RX_COMPLETE) {
				// at this point an ACK or 'data' packet was received.
                
			}
			packetStatus = ssp_SendProcess(master);
		} while ( packetStatus == SSP_TX_WAITING);
#endif
		
	}
}


// these functions implement a simulated serial in/out for both a master
// and a slave device. In reality these functions do not send anything out
// but just puts them into a circular buffer.
// In a real system these would use the PIOS_COM_xxxx functions.

void masterCallBack(uint8_t *buf, uint16_t len)
{
	len = len;
}

// simulates checking for character from a serial buffer.

int16_t masterSerialRead(void)
{
	int16_t	retval = -1;
	static uint16_t	count = 0;

	if( bufferBufferedData(&s2mBuffer)) {
		retval = bufferGetFromFront( &s2mBuffer);
	}
	count++;
	if( count % 5 == 0 ) {
		ssp_ReceiveByte(&slave_port);
	}
	return retval;
}

void masterSerialWrite(uint8_t b)
{
	bufferAddToEnd( &m2sBuffer, b);
}

uint32_t	masterTime = 0;
uint32_t	slaveTime = 0;

uint32_t masterGetTime(void)
{
	masterTime++;
	return masterTime;
}

void slaveCallBack(uint8_t *buf, uint16_t len)
{
	len = len;
}

int16_t	slaveSerialRead(void)
{
	int16_t	retval = -1;
	if( bufferBufferedData(&m2sBuffer)) {
		retval = bufferGetFromFront( &m2sBuffer);
	}
	return retval;
}

void slaveSerialWrite(uint8_t b)
{
	bufferAddToEnd( &s2mBuffer, b);
}

uint32_t slaveGetTime(void)
{
	slaveTime++;
	return slaveTime;
}
