/**
 ******************************************************************************
 *
 * @file       port.cpp
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
#include "port.h"
#include "delay.h"
#include <QDebug>
port::port(QString name, bool debug) : mstatus(port::closed), debug(debug)
{
    timer.start();
    sport = new QSerialPort();
    sport->setPortName(name);
    if (sport->open(QIODevice::ReadWrite)) {
        sport->setReadBufferSize(0);
        if (sport->setBaudRate(QSerialPort::Baud57600)
            && sport->setDataBits(QSerialPort::Data8)
            && sport->setParity(QSerialPort::NoParity)
            && sport->setStopBits(QSerialPort::OneStop)
            && sport->setFlowControl(QSerialPort::NoFlowControl)) {
            mstatus = port::open;
        }
        // sport->setDtr();
    } else {
        qDebug() << sport->error();
        mstatus = port::error;
    }
}

port::~port()
{
    sport->close();
}

port::portstatus port::status()
{
    return mstatus;
}

int16_t port::pfSerialRead(void)
{
    char c[1];
    sport->waitForBytesWritten(5);
    if (sport->bytesAvailable() || sport->waitForReadyRead(1)) {
        sport->read(c, 1);
        if (debug) {
            if (((uint8_t)c[0]) == 0xe1 || rxDebugBuff.count() > 50) {
                qDebug() << "PORT R " << rxDebugBuff.toHex();
                rxDebugBuff.clear();
            }
            rxDebugBuff.append(c[0]);
        }
    } else { return -1; }
    return (uint8_t)c[0];
}

void port::pfSerialWrite(uint8_t c)
{
    char cc[1];

    cc[0] = c;
    sport->write(cc, 1);
    if (debug) {
        if (((uint8_t)cc[0]) == 0xe1 || rxDebugBuff.count() > 50) {
            qDebug() << "PORT T " << txDebugBuff.toHex();
            txDebugBuff.clear();
        }
        txDebugBuff.append(cc[0]);
    }
    sport->waitForBytesWritten(1);
}

uint32_t port::pfGetTime(void)
{
    return timer.elapsed();
}
