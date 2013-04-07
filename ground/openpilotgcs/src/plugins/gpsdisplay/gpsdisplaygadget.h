/**
 ******************************************************************************
 *
 * @file       gpsdisplaygadget.h
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

#ifndef GPSDISPLAYGADGET_H_
#define GPSDISPLAYGADGET_H_

#include <qextserialport/src/qextserialport.h>
#include <qextserialport/src/qextserialenumerator.h>
#include <coreplugin/iuavgadget.h>
#include "gpsdisplaywidget.h"
#include "nmeaparser.h"
#include "telemetryparser.h"

class IUAVGadget;
class QWidget;
class QString;
class GpsDisplayWidget;

using namespace Core;

class GpsDisplayGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    GpsDisplayGadget(QString classId, GpsDisplayWidget *widget, QWidget *parent = 0);
    ~GpsDisplayGadget();

    QWidget *widget() { return m_widget; }

    //   void setMode(QString mode);  // Either UAVTalk or serial port

    void loadConfiguration(IUAVGadgetConfiguration* config);
public slots:
    void onConnect();
    void onDisconnect();

private slots:
    void onDataAvailable();

private:
    QPointer<GpsDisplayWidget> m_widget;
    QPointer<QextSerialPort> port;
    QPointer<GPSParser> parser;
    bool connected;
    void processNewSerialData(QByteArray serialData);
};


#endif // GPSDISPLAYGADGET_H_
