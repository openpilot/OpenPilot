
/**
@file

@brief Implementation of Y-Modem transmit protocol.
*/


#include "qymodem_TX.h"


#include <string.h> // for memcpy and memset


/**
Minimum time in milli-seconds to wait for response from receiver.
*/
const unsigned SendTimeout = 11*1000;


/**
Process response from receiver.

@param c	Value received from InChar.

@return One if received an ACKnowledge,
                zero if received a Negative AcKnowledge,
                or a negative error value.
*/
int QymodemTx::ProcessResponse(int c)
        {
        if(c<0)
                return c;
        if(c==CAN)
                {
                if(CancelCount++)
                        return ErrorTranferTerminatedByReceiver;
                return 0;
                }
        CancelCount = 0;
        if(c==ACK)
                return 1;
        return 0;
        }


/**
Begin the Y-Modem transfer.

@param timeout	Time in milliseconds to wait receiver to become ready.

@return Zero if transfer initialisation was successful, or an error code on failure.
*/
int QymodemTx::SendInitialise(unsigned timeout)
        {
    emit Information("Waiting for Receiver",QymodemTx::InfoSending);
        if(timeout<SendTimeout)
                timeout = SendTimeout;

        while(InChar(-1)>=0) // flush input buffers
                {}

        CancelCount = 0;

        int c;
        for(;;)
                {
                const unsigned timeoutStep = 10;
                c = InChar(timeoutStep);
                if(c=='G')
                        {
                        SendCRC = true;
                        WaitForBlockACK = false;
                        break;
                        }
                else if(c=='C')
                        {
                        SendCRC = true;
                        WaitForBlockACK = true;
                        break;
                        }
                else if(c==NAK)
                        {
                        SendCRC = false;
                        WaitForBlockACK = true;
                        break;
                        }
                if(c<0 && c!=ErrorTimeout)
                        return c;
                if(timeout<timeoutStep)
                        return ErrorTimeout;
                timeout -= timeoutStep;
                }

        ModeChar = c;
        return 0;
        }


/**
Send a single block of data.
A zero sized block terminates the transfer.

@param data		The data to transfer.
@param size		Size of data.

@return The number of transfered, or an error code on failure.
                The number of bytes may be less than \a size.

@pre SendInitialise() must have been successful.
*/
int   QymodemTx::SendBlock(const char* data, size_t size)
        {
        char block[1+2+1024+2];	// buffer to hold data in the block
        int retryCount = 10;		// number of attempts to send the block
        bool waitForBlockACK = WaitForBlockACK;

change_mode:

        size_t blockSize = (Use1KBlocks && size>=1024) ? 1024 : 128;
        size_t dataSize = size<blockSize ? size : blockSize;	// size of data to send in block

        if(!dataSize)
                {
                // all bytes sent, so end transfer by sending a single EOT...
                block[0] = EOT;
                blockSize = 1;
                waitForBlockACK = true;
                }
        else
                {
                // make block header...
                block[0] = blockSize==1024 ? STX : SOH;
                block[1] = BlockNumber&0xffu;
                block[2] = (~BlockNumber)&0xffu;

                // copy data for block (padding with EOF)...
                memcpy(block+3,data,dataSize);
                memset(block+3+dataSize,26,blockSize-dataSize);

                // append checksum/crc...
                if(SendCRC)
                        {
                        uint16_t crc = CRC16(block+3,blockSize);
                        blockSize += 3;
                        block[blockSize++] = (uint8_t)(crc>>8);
                        block[blockSize++] = (uint8_t)crc;
                        }
                else
                        {
                        uint8_t sum = Checksum(block+3,blockSize);
                        blockSize += 3;
                        block[blockSize++] = sum;
                        }
                }

do_retry:
        // count attenpts...
        if(!retryCount--)
                return ErrorBlockRetriesExceded;

        char* out = block;
        size_t outSize = blockSize;
        for(;;)
                {
                // send some data...
                Port.setTimeout(1000);;
                int result = (int)Port.write(out,outSize);
                if(result<0)
                        return result; // return error
                if(result==0)
                        return ErrorTimeout;

                // adjust for data remaining...
                out += result;
                outSize -= result;

                // end if done...
                if(!outSize)
                        break;

                // poll for signal from receiver...
                result = ProcessResponse(InChar(10));
                if(result==ErrorTimeout)
                        continue; // nothing received
                if(result<0)
                        return result; // return error
                if(!result)
                        goto retry; // negative acknowledge received
                }

        if(waitForBlockACK)
                {
                // wait for up to one second for block to be acknowledged...
                int c = InChar(1000);
                int result = ProcessResponse(c);
                if(result<0)
                        return result; // return error
                if(!result)
                        {
                        // negagtive acknowledge received...
                        if(c=='C' && !SendCRC)
                                {
                                // change to CRC mode if receiver sent 'C', and retry...
                                SendCRC = true;
                                goto change_mode;
                                }
                        goto retry;
                        }
                }
        else
                {
                // check for receiver sending a cancel byte...
                int result = ProcessResponse(InChar(0));
                if(result<0)
                        {
                        if(result!=ErrorTimeout)
                                return result; // return error if it's not a timeout
                        }
                else
                        {
                        // ignore other responses
                        }
                }

        // block transferred OK...
        ++BlockNumber;
        return dataSize;

retry:
        while(InChar(500)>=0) // flush input buffers
                {}
        goto do_retry;
        }


/**
Send data.
A zero sized block terminates the transfer.

@param data		The data to transfer.
@param size		Size of data.

@return Zero if successful, or a negative error value if failed.

@pre SendInitialise() must have been successful.
*/
int QymodemTx::SendData(const char* data, size_t size)
        {
        do
                {
                int result = SendBlock(data, size);
                if(result<0)
                        return result;
                data += result;
                size -= result;
                }
        while(size);
        return 0;
        }


/**
Send an entire stread of data.

@param in	The stream of data to send.

@return Zero if successful, or a negative error value if failed.

@pre SendInitialise() must have been successful.
*/
int QymodemTx::SendAll(InStream& in)
        {
        BlockNumber = 1; // first block to send is number one
        size_t size;
        do
                {
                // get data from input stream...
                char data[1024];
                int result = in.In(data,sizeof(data),&percent);
                if(result<0)
                        return ErrorInputStreamError;

                // send data...

                size = result;
                result = SendData(data,size);
                if(result<0)
                        return result;
                }
        while(size); // end when no more data left
        return 0;
        }


/**
Construct the data for the first block of a Y-Modem transfer.

@param[out] buffer		The buffer to store the constructed block. Size must be >=128 bytes.
@param fileName			The name of the file being transferred.
@param fileSize			The size of the file being transferred.

@return Zero if successful, or a negative error value if failed.
*/
int QymodemTx::MakeBlock0(char* buffer, const char* fileName, size_t fileSize)
        {
        // setup buffer for block 0...
        char* out = buffer;
        char* outEnd = buffer+128-1;
        memset(buffer,0,128);

        // copy file name to block data...
        while(out<outEnd)
                {
                char c = *fileName++;
                if(c>='A' && c<='Z')
                        c += 'a'-'A';	// convert name to lower-case
                else if(c=='\\')
                        c = '/';		// convert back-slash to forward-slash
                *out++ = c;
                if(!c)
                        break;			// end of name
                }

        // convert fileSize to a decimal number...
        char length[sizeof(size_t)*3+1]; // buffer big enough to hold length as decimal number
        char* lenEnd = length+sizeof(length);
        char* len = lenEnd;
        do
                {
                *--len = '0'+fileSize%10;	// prepend digit to buffer
                fileSize /= 10;
                }
        while(fileSize);				// end when all digits done

        // append file length to block data...
        while(out<outEnd && len<lenEnd)
                *out++ = *len++;

        // check that buffer was big enough...
        if(out>=outEnd)
                return ErrorFileNameTooLong;

        return 0; // OK
        }


QymodemTx::QymodemTx(QextSerialPort& port)
        : QymodemBase(port)
        {
        }


int QymodemTx::SendX(InStream& in, unsigned timeout, bool kMode)
        {
        Use1KBlocks = kMode;
        int result = SendInitialise(timeout);
        if(result<0)
                return result;
        return SendAll(in);
        }


int QymodemTx::SendY(const char* fileName, size_t size, InStream& in, unsigned timeout)
        {
        Use1KBlocks = true;
        char buffer[128];
        int result = MakeBlock0(buffer,fileName,size);
        if(result<0)
                return result;

        result = SendInitialise(timeout);
        if(result<0 && result!=ErrorBlockRetriesExceded)
                return result;
        emit Information("Sending "+QString(fileName),QymodemTx::InfoSending);
        BlockNumber = 0;
        result = SendBlock(buffer,sizeof(buffer));
        if(result<0)
                return result;

        result = InChar(SendTimeout);
        if(result<0)
                return result;
        if(result!=ModeChar)
                return ErrorReceiverNotBehaving;

        result = SendAll(in);
        if(result<0)
                return result;

        result = InChar(SendTimeout);
        if(result<0)
                return result;
        if(result!=ModeChar)
                return ErrorReceiverNotBehaving;

        memset(buffer,0,sizeof(buffer));
        BlockNumber = 0;
        result = SendBlock(buffer,sizeof(buffer));
        if(result<0)
                return result;

        return 0;
        }


