/**
 ******************************************************************************
 *
 * @file       qymodem_tx.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   ymodem_lib
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

/*!
*   \section Credits
*   This implementation is based on J.D.Medhurst (a.k.a. Tixy) work from
*   <a href="http://yxit.co.uk">Tixy's source code</a>.
*/

#ifndef YMODEM_TX_H
#define YMODEM_TX_H
#include "qymodem.h"

/**
Base Class for QymodemSend.
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
             int percent;
                /**
                Read data from the stream.

                @param[out] data	Pointer to buffer to hold data read from stream.
                @param size			Maximum size of data to read.

                @return Number of bytes successfully read, or a negative error value if failed.
                */
                virtual int In(quint8* data, size_t size) =0;

                /**
                Empty destructor to avoid compiler warnings.
                */
                inline virtual ~InStream() {}
                };


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
                ErrorFileNotFound                       = -303,/**< Error file not found */
                ErrorCoulNotOpenPort                    = -304,/**< Error Port could not be open */
                ErrorFileTransmissionInProgress         = -305/**< Error, user tried to transmit a File while other File transmisssion is in progress */

            };
        /**
        Enumeration of possible information values.
        */
        enum Info
        {
            InfoSending                                 = -100,/**< Info, the transmission started and the file is being sent*/
            InfoSent                                    =-101,/**< Info, transmission finished,the file as been sent */
            InfoWaitingforReceiver                      =-102/**< Info, the transmission is waiting for the receiver to signal begin of transfer */
        };

private:
        int SendInitialise(unsigned timeout);
        int SendBlock(const quint8* data, size_t size);
        int SendData(const quint8* data, size_t size);
        int SendAll(InStream& in);
        int MakeBlock0(quint8* buffer, const char* fileName, size_t fileSize);
        int ProcessResponse(int c);
private:
        size_t BlockNumber;
        bool SendCRC;
        bool WaitForBlockACK;
        bool Use1KBlocks;
        quint8 ModeChar;
        int CancelCount;    
        };


#endif // YMODEM_TX_H
