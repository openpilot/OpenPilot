/**
 ******************************************************************************
 *
 * @file       qymodem.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Implementation of base class for QymodemTx.
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

#include "qymodem.h"

quint16 QymodemBase::UpdateCRC16(quint16 crcIn, quint8 byte)
        {
        quint32 crc = crcIn;
        quint32 in = byte|0x100;
        do
                {
                crc <<= 1;
                in <<= 1;
                if(in&0x100)
                        ++crc;
                if(crc&0x10000)
                        crc ^= 0x1021;
                }
        while(!(in&0x10000));
        return crc&0xffffu;
        }


quint16 QymodemBase::CRC16(const quint8* data, size_t size)
        {
        quint32 crc = 0;
        const quint8* dataEnd = data+size;
        while(data<dataEnd)
                crc = UpdateCRC16(crc,*data++);
        crc = UpdateCRC16(crc,0);
        crc = UpdateCRC16(crc,0);
        return crc&0xffffu;
        }


quint8 QymodemBase::Checksum(const quint8* data, size_t size)
        {
        int sum = 0;
        const quint8* dataEnd = data+size;
        while(data<dataEnd)
                sum += *data++;
        return sum&0xffu;
        }


int QymodemBase::InChar(long timeout)
        {
        quint8 c;
        char cc;
        Port.setTimeout(timeout);
        int result =(int)Port.read(&cc,1);
        c=(quint8)cc;
        if(result==1)
                return c;
        if(result==0)
                return ErrorTimeout;
        return result;
        }


void QymodemBase::Cancel()
        {
        const quint8 CancelString[] = { CAN,CAN,CAN,CAN,CAN };
        Port.setTimeout(1000);
        Port.write((char*)CancelString,sizeof(CancelString));
         }

