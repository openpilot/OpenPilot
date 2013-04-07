#include "qsspt.h"

qsspt::qsspt(port * info,bool debug):qssp(info,debug),endthread(false),datapending(false),debug(debug)
{
}

void qsspt::run()
{
    while(!endthread)
    {
        receivestatus=this->ssp_ReceiveProcess();
        sendstatus=this->ssp_SendProcess();
        sendbufmutex.lock();
        if(datapending && receivestatus==SSP_TX_IDLE)
        {
            this->ssp_SendData(mbuf,msize);
            datapending=false;
        }
        sendbufmutex.unlock();
        if(sendstatus==SSP_TX_ACKED)
            sendwait.wakeAll();
    }

}
bool qsspt::sendData(uint8_t * buf,uint16_t size)
{
    if(datapending)
        return false;
    sendbufmutex.lock();
    datapending=true;
    mbuf=buf;
    msize=size;
    sendbufmutex.unlock();
    msendwait.lock();
    sendwait.wait(&msendwait,100000);
    msendwait.unlock();
    return true;
}

void qsspt::pfCallBack( uint8_t * buf, uint16_t size)
{
     if (debug)
         qDebug()<<"receive callback"<<buf[0]<<buf[1]<<buf[2]<<buf[3]<<buf[4]<<"array size="<<queue.count();
    QByteArray array;
    for(int x=0;x<size;x++)
    {
        array.append(buf[x]);
    }
    mutex.lock();
    queue.enqueue(array);
    // queue.enqueue((const char *)&buf);
    mutex.unlock();
}
int qsspt::packets_Available()
{
    return queue.count();
}
int qsspt::read_Packet(void * data)
{
    mutex.lock();
    if(queue.size()==0)
    {
        mutex.unlock();
        return -1;
    }
    QByteArray arr=queue.dequeue();
    memcpy(data,(uint8_t*)arr.data(),arr.length());
    mutex.unlock();
    return arr.length();
}
qsspt::~qsspt()
{
    endthread=true;
    wait(1000);
}
