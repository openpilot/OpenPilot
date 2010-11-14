#include "qsspt.h"

qsspt::qsspt(port * info):qssp(info),endthread(false)
{
    this->start();
}
void qsspt::run()
{
    while(!endthread)
    {
    this->ssp_ReceiveProcess();
    this->ssp_SendProcess();
}
}
void qsspt::pfCallBack( uint8_t * buf, uint16_t size)
{
    //qDebug()<<"receive callback"<<buf[0]<<buf[1]<<buf[2]<<buf[3]<<buf[4]<<"array size="<<queue.count();
    QByteArray array;
    for(int x=0;x<size;x++)
    {
        array.append((char)buf[x]);
    }
    mutex.lock();
    queue.enqueue(array);
    //queue.enqueue((const char *)&buf);
    mutex.unlock();
}
int qsspt::packets_Available()
{
   return queue.count();
}
QByteArray qsspt::read_Packet()
{
    mutex.lock();
    QByteArray arr=queue.dequeue();
    mutex.unlock();
    return arr;
}
qsspt::~qsspt()
{
    endthread=true;
    wait(1000);
}
