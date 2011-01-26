/**
 ******************************************************************************
 *
 * @file       qsspt.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Uploader Serial and USB Uploader Plugin
 * @{
 * @brief The USB and Serial protocol uploader plugin
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
        msleep(1);
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
    sendwait.wait(&msendwait,10000);
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
