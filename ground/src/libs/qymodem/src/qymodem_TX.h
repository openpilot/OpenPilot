
/**
@file

@brief Y-Modem transmit protocol.
*/
#ifndef YMODEM_TX_H
#define YMODEM_TX_H
#include "qymodem.h"

/**
@brief Y-Modem transmiter object.
@ingroup ymodem
*/
class QymodemTx : public QymodemBase
        {
public:
        /**
        Construct a Y-Modem object which will transmit data over the given port.

        @param port		The port.
        */
        QymodemTx(QextSerialPort& port);

        /**
        Abstract class representing a stream of data being read.
        */

        class InStream
                {
        public:
                /**
                Read data from the stream.

                @param[out] data	Pointer to buffer to hold data read from stream.
                @param size			Maximum size of data to read.

                @return Number of bytes successfully read, or a negative error value if failed.
                */
                virtual int In(char* data, size_t size, int * percent) =0;

                /**
                Empty destructor to avoid compiler warnings.
                */
                inline virtual ~InStream() {}
                };

        /**
        Send data using X-Modem.

        @param in		The stream of data to send.
        @param timeout	Time in milliseconds to wait receiver to become ready.
        @param kMode	False to use 128 byte blocks, true to use 1kB blocks
        @return Zero if transfer was successful, or a negative error value if failed.
        */
        int SendX(InStream& in, unsigned timeout, bool kMode);

        /**
        Send data using Y-Modem.

        @param fileName	The name of the file being transferred.
        @param size		The size of the data being transferred.
        @param in		The stream of data to send.
        @param timeout	Time in milliseconds to wait receiver to become ready.

        @return Zero if transfer was successful, or a negative error value if failed.
        */
        int SendY(const char* fileName, size_t size, InStream& in, unsigned timeout);

        /**
        Enumeration of possible error values.
        */
        enum TxError
                {
                ErrorInputStreamError			= -300,	/**< Error with input stream */
                ErrorReceiverNotBehaving		= -301,	/**< Unexpected data received */
                ErrorTranferTerminatedByReceiver= -302,	/**< Transfer was terminated by receiver */
                ErrorFileNameTooLong			= -303,	/**< File name was too long to be transmitted */
                ErrorFileNotFound= -303,
                ErrorCoulNotOpenPort= -304,
                ErrorFileTransmissionInProgress = -305

            };
        enum Info
        {
            InfoSending= -100,
            InfoSent=-101,
            InfoWaitingforReceiver=-102
        };

private:
        int SendInitialise(unsigned timeout);
        int SendBlock(const char* data, size_t size);
        int SendData(const char* data, size_t size);
        int SendAll(InStream& in);
        int MakeBlock0(char* buffer, const char* fileName, size_t fileSize);
        int ProcessResponse(int c);
private:
        size_t BlockNumber;
        bool SendCRC;
        bool WaitForBlockACK;
        bool Use1KBlocks;
        uint8_t ModeChar;
        int CancelCount;    
        };


#endif // YMODEM_TX_H
