/**
 ******************************************************************************
 *
 * @file       gpsdisplaythread.h
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

#ifndef GPSDISPLAYTHREAD_H
#define GPSDISPLAYTHREAD_H

#include <QThread>
#include <qextserialport/src/qextserialport.h>
#include "nmeaparser.h"

class GpsDisplayThread : public QThread
{
public:
    QextSerialPort *port;
    NMEAParser *parser;
    void setPort(QextSerialPort* port);
    void setParser(NMEAParser* parser);
    void processInputStream();
    void run();
};

#endif // GPSDISPLAYTHREAD_H
