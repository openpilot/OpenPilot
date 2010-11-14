#include "port.h"

port::port(PortSettings settings,QString name)
{
    timer.start();
    sport = new QextSerialPort(name,settings, QextSerialPort::Polling);
    sport->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
}

int16_t port::pfSerialRead(void)
{
    char c[1];
    if(sport->bytesAvailable()>0)
    {
        sport->read(c,1);
    }
    else return -1;
    //qDebug()<<"read="<<c[0];
    return c[0];
}

void port::pfSerialWrite(uint8_t c)
{
    char cc[1];
    cc[0]=c;
    while (sport->write(cc,1)==-1){};
}

uint32_t port::pfGetTime(void)
{
    return timer.elapsed();
}
