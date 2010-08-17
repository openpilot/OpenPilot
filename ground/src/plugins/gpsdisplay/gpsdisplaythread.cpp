/**
 ******************************************************************************
 *
 * @file       gpsdisplaythread.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GPSGadgetPlugin GPS Gadget Plugin
 * @{
 * @brief A gadget that displays GPS status and enables basic configuration
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

#include "gpsdisplaythread.h"

#include <QDebug>

void GpsDisplayThread::setPort(QextSerialPort* port)
{

    this->port=port;
}

void GpsDisplayThread::setParser(NMEAParser* parser)
{

    this->parser=parser;
}

void GpsDisplayThread::run()
{

    qDebug() <<  "Opening.";

    if (port) {
        qDebug() <<  port->portName();

        bool isOpen =  port->open(QIODevice::ReadWrite);
        qDebug() <<  "Open: " << isOpen;

        char buf[1024];
        char c;
        while(true) {
            qDebug() <<  "Reading.";
            /*qint64 bytesRead = port->readLine(buf, sizeof(buf));
            qDebug() << "bytesRead: " << bytesRead;
            if (bytesRead != -1) {
                qDebug() <<  "Result: '" << buf << "'";
            }*/
            while(port->bytesAvailable()>0)
            {
                    port->read(&c,1);
                    parser->processInputStream(c);
            }
            sleep(1);
        }
    } else {
        qDebug() << "Port undefined or invalid.";
    }
}
