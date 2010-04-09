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
/**
@file

@brief Implementation of base class for YModemRx and YModemTx.
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


quint16 QymodemBase::CRC16(const char* data, size_t size)
        {
        quint32 crc = 0;
        const char* dataEnd = data+size;
        while(data<dataEnd)
                crc = UpdateCRC16(crc,*data++);
        crc = UpdateCRC16(crc,0);
        crc = UpdateCRC16(crc,0);
        return crc&0xffffu;
        }


quint8 QymodemBase::Checksum(const char* data, size_t size)
        {
        int sum = 0;
        const char* dataEnd = data+size;
        while(data<dataEnd)
                sum += *data++;
        return sum&0xffu;
        }


int QymodemBase::InChar(long timeout)
        {
        char c;
        Port.setTimeout(timeout);
        int result =(int)Port.read(&c,1);
        if(result==1)
                return c;
        if(result==0)
                return ErrorTimeout;
        return result;
        }


void QymodemBase::Cancel()
        {
        const char CancelString[] = { CAN,CAN,CAN,CAN,CAN };
        Port.setTimeout(1000);
        Port.write(CancelString,sizeof(CancelString));
         }

