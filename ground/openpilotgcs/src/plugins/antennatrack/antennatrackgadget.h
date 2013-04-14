/**
 ******************************************************************************
 *
 * @file       AntennaTrackgadget.h
 * @author     Sami Korhonen & the OpenPilot team Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup AntennaTrackGadgetPlugin Antenna Track Gadget Plugin
 * @{
 * @brief A gadget that communicates with antenna tracker and enables basic configuration
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

#ifndef ANTENNATRACKGADGET_H_
#define ANTENNATRACKGADGET_H_

#include <qextserialport/src/qextserialport.h>
#include <qextserialport/src/qextserialenumerator.h>
#include <coreplugin/iuavgadget.h>
#include "antennatrackwidget.h"
#include "telemetryparser.h"

class IUAVGadget;
class QWidget;
class QString;
class AntennaTrackWidget;

using namespace Core;

class AntennaTrackGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    AntennaTrackGadget(QString classId, AntennaTrackWidget *widget, QWidget *parent = 0);
    ~AntennaTrackGadget();

    QWidget *widget() { return m_widget; }

    //   void setMode(QString mode);  // Either UAVTalk or serial port

    void loadConfiguration(IUAVGadgetConfiguration* config);
public slots:
    void onConnect();
    void onDisconnect();

private slots:
    void onDataAvailable();

private:
    QPointer<AntennaTrackWidget> m_widget;
    QPointer<QextSerialPort> port;
    QPointer<GPSParser> parser;
    bool connected;
    void processNewSerialData(QByteArray serialData);
};


#endif // ANTENNATRACKGADGET_H_
