/**
 ******************************************************************************
 *
 * @file       rawhid.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      QIODevice interface for USB RawHID
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   rawhid_plugin
 * @{
 *
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

#include "rawhid.h"

#include "rawhid_const.h"

//timeout value used when we want to return directly without waiting
static const int IMMEDIATE_READ_TIMEOUT = 10;
static const int IMMEDIATE_WRITE_TIMEOUT = 50;

#if 0
static const int MAX_RX_LENGTH  = 63;
static const int MAX_TX_LENGTH  = 63;

static const int READ_TIMEOUT   = 200;
static const int WRITE_TIMEOUT  = 100;
#endif

RawHID::RawHID()
    :QIODevice()
{
}

RawHID::RawHID(const QString &deviceName)
    :QIODevice(),
    serialNumber(deviceName),
    m_deviceNo(-1)
{
    //find the device the user want to open and close the other
    int opened = dev.open(MAX_DEVICES, VID, PID, USAGE_PAGE, USAGE);

    //for each devices found, get serial number and close
    for(int i=0; i<opened; i++)
    {
        char serial[256];
        dev.getserial(i, serial);
        if(deviceName == QString::fromAscii(serial, DEV_SERIAL_LEN))
            m_deviceNo = i;
        else
            dev.close(i);
    }

    //didn't find the device we are trying to open??
    if(m_deviceNo < 0)
        qDebug() << "Error: cannot open device " << deviceName;
}

RawHID::~RawHID()
{
}

bool RawHID::open(OpenMode mode)
{
    if(m_deviceNo)
        return false;

    QIODevice::open(mode);
    return true;
}

void RawHID::close()
{
    dev.close(m_deviceNo);

    QIODevice::close();
}

bool RawHID::isSequential() const
{
    return true;
}


qint64 RawHID::readData(char *data, qint64 maxSize)
{
    return dev.receive(m_deviceNo, data, maxSize, IMMEDIATE_READ_TIMEOUT);
}

qint64 RawHID::writeData(const char *data, qint64 maxSize)
{
    return dev.send(m_deviceNo, const_cast<char*>(data), maxSize, IMMEDIATE_WRITE_TIMEOUT);
}


