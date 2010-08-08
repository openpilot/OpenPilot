/**
 ******************************************************************************
 *
 * @file       qymodem.h
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

#ifndef YMODEM_H
#define YMODEM_H
#include <qextserialport/src/qextserialport.h>
#include <QThread>

/**
Base Class for QymodemTx.
*/
class QymodemBase: public QThread
{
    Q_OBJECT
signals:
    /*!
      An error has ocorred.

      \param errorString A string containing the error that has occurred.

      \param errorNumber The error code.
    */
    void Error(QString errorString,int errorNumber);
    /*!
      Information regarding a change of state in the transmission process.

      \param infoString A string containing the information of the new state.

      \param infoNumber The information number of the new state.
    */
    void Information(QString infoString,int infoNumber);
    /*!
      Percentage of the file already sent.

      \param percentSent The percentage of the file already sent.

    */
    void Percent(int percentSent);

protected:

    inline QymodemBase(QextSerialPort& port)
        : Port(port)
    {}

    /**
        Checksum a block of data.

        @param data		Start of data to checksum.
        @param size		Size of data.

        @return Sum of bytes in data, modulo 256.
        */
    quint8 Checksum(const quint8* data, size_t size);

    /**
        Calculate CRC for a block of data.

        @param data		Start of data to checksum.
        @param size		Size of data.

        @return CRC of data.
        */
    quint16 CRC16(const quint8* data, size_t size);

    /**
        Update CRC value by accumulating another byte of data.

        @param crcIn	Previous CRC value.
        @param byte		A byte of data.

        @return Updated CRC value.
        */
    quint16 UpdateCRC16(quint16 crcIn, quint8 byte);

    /**
        Receive a single character.
        If the timeout period is exceeded, ErrorTimeout is returned.

        @param timeout	Time in milliseconds to wait if no data available.

        @return The character received, or a negative error value if failed.
        */
    int InChar(long timeout);

    /**
        Send CANcel sequence.
        */
    void Cancel();

    /**
        Enumeration of control characted used in communications protocol.
        */
    enum ControlCharacters
    {
        SOH = 0x01,
        STX = 0x02,
        EOT = 0x04,
        ACK = 0x06,
        NAK = 0x15,
        CAN = 0x18
          };

    /**
        Enumeration of possible error values.
        */
    enum Error
    {
        ErrorTimeout					= -200,	/**< Timed out trying to communicate with other device */
        ErrorBlockRetriesExceded		= -201	/**< A block could not be sent */

                                              };

protected:
    /**
        The serial port to use for communications.
        */

    QextSerialPort& Port;
};


/** @} */ // End of group

#endif // YMODEM_H
