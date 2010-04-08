
/**
@file

@brief Implementation of base class for YModemRx and YModemTx.
*/

#include "qymodem.h"

uint16_t QymodemBase::UpdateCRC16(uint16_t crcIn, uint8_t byte)
        {
        uint32_t crc = crcIn;
        uint32_t in = byte|0x100;
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


uint16_t QymodemBase::CRC16(const char* data, size_t size)
        {
        uint32_t crc = 0;
        const char* dataEnd = data+size;
        while(data<dataEnd)
                crc = UpdateCRC16(crc,*data++);
        crc = UpdateCRC16(crc,0);
        crc = UpdateCRC16(crc,0);
        return crc&0xffffu;
        }


uint8_t QymodemBase::Checksum(const char* data, size_t size)
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

