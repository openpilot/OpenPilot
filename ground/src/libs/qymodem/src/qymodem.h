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

#ifndef YMODEM_H
#define YMODEM_H
#include <qextserialport/src/qextserialport.h>
#include <QThread>


class QymodemBase: public QThread
{
    Q_OBJECT
signals:
    void Error(QString,int);
    void Information(QString,int);

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
    uint8_t Checksum(const char* data, size_t size);

    /**
        Calculate CRC for a block of data.

        @param data		Start of data to checksum.
        @param size		Size of data.

        @return CRC of data.
        */
    uint16_t CRC16(const char* data, size_t size);

    /**
        Update CRC value by accumulating another byte of data.

        @param crcIn	Previous CRC value.
        @param byte		A byte of data.

        @return Updated CRC value.
        */
    uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte);

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
        ErrorBlockRetriesExceded		= -201,	/**< A block could not be sent */

                                              };

protected:
    /**
        The serial port to use for communications.
        */

    QextSerialPort& Port;
    int percent;
};


/** @} */ // End of group

#endif // YMODEM_H
