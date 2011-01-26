/**
 ******************************************************************************
 *
 * @file       qsspt.h
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
#ifndef QSSPT_H
#define QSSPT_H

#include "qssp.h"
#include <QThread>
#include <QQueue>
#include <QWaitCondition>
class qsspt:public qssp, public QThread
{
public:
    qsspt(port * info,bool debug);
    void run();
    int packets_Available();
    int read_Packet(void *);
    ~qsspt();
    bool sendData(uint8_t * buf,uint16_t size);
private:
    virtual void pfCallBack( uint8_t *, uint16_t);
    uint8_t * mbuf;
    uint16_t msize;
    QQueue<QByteArray> queue;
    QMutex mutex;
    QMutex sendbufmutex;
    bool datapending;
    bool endthread;
    uint16_t sendstatus;
    uint16_t receivestatus;
    QWaitCondition sendwait;
    QMutex msendwait;
    bool debug;
};

#endif // QSSPT_H
