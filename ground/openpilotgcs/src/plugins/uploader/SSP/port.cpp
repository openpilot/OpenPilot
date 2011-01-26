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

port::~port() {
    sport->close();
}

port::portstatus port::status()
{
    return mstatus;
}
int16_t port::pfSerialRead(void)
{

    char c[1];
    if( sport->bytesAvailable() )
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
