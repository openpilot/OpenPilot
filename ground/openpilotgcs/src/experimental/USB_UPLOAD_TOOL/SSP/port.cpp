#include "port.h"
#include "delay.h"
port::port(PortSettings settings,QString name):mstatus(port::closed)
{
    timer.start();
    sport = new QextSerialPort(name,settings, QextSerialPort::Polling);
    if(sport->open(QIODevice::ReadWrite|QIODevice::Unbuffered))
    {
        mstatus=port::open;
      //  sport->setDtr();
    }
    else
        mstatus=port::error;
}
port::portstatus port::status()
{
    return mstatus;
}
int16_t port::pfSerialRead(void)
{

    char c[1];
    if(sport->bytesAvailable())
    {
        sport->read(c,1);
    }
    else return -1;
    return (uint8_t)c[0];
}

void port::pfSerialWrite(uint8_t c)
{
    char cc[1];
    cc[0]=c;
    sport->write(cc,1);
}

uint32_t port::pfGetTime(void)
{
    return timer.elapsed();
}
