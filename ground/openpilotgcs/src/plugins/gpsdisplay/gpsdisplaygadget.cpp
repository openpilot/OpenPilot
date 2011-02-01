/**
 ******************************************************************************
 *
 * @file       gpsdisplaygadget.cpp
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

#include "gpsdisplaygadget.h"
#include "gpsdisplaywidget.h"
#include "gpsdisplaygadgetconfiguration.h"

GpsDisplayGadget::GpsDisplayGadget(QString classId, GpsDisplayWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
        m_widget(widget),
        connected(FALSE)
{
    connect(m_widget->connectButton, SIGNAL(clicked(bool)), this,SLOT(onConnect()));
    connect(m_widget->disconnectButton, SIGNAL(clicked(bool)), this,SLOT(onDisconnect()));
}

GpsDisplayGadget::~GpsDisplayGadget()
{
    delete m_widget;
}

/*
  This is called when a configuration is loaded, and updates the plugin's settings.
  Careful: the plugin is already drawn before the loadConfiguration method is called the
  first time, so you have to be careful not to assume all the plugin values are initialized
  the first time you use them
 */
void GpsDisplayGadget::loadConfiguration(IUAVGadgetConfiguration* config)
{
    // Delete the (old)port, this also closes it.
    if(port) {
        delete port;
    }

    // Delete the (old)parser, this also disconnects all signals.
    if(parser) {
        delete parser;
    }

    GpsDisplayGadgetConfiguration *gpsDisplayConfig = qobject_cast< GpsDisplayGadgetConfiguration*>(config);

    if (gpsDisplayConfig->connectionMode() == "Serial") {
        PortSettings portsettings;
        portsettings.BaudRate=gpsDisplayConfig->speed();
        portsettings.DataBits=gpsDisplayConfig->dataBits();
        portsettings.FlowControl=gpsDisplayConfig->flow();
        portsettings.Parity=gpsDisplayConfig->parity();
        portsettings.StopBits=gpsDisplayConfig->stopBits();
        portsettings.Timeout_Millisec=gpsDisplayConfig->timeOut();

        // In case we find no port, buttons disabled
        m_widget->connectButton->setEnabled(false);
        m_widget->disconnectButton->setEnabled(false);

        QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
        foreach( QextPortInfo nport, ports ) {
            if(nport.friendName == gpsDisplayConfig->port())
            {
                qDebug() << "Using Serial parser";
                parser = new NMEAParser();

#ifdef Q_OS_WIN
                port=new QextSerialPort(nport.portName,portsettings,QextSerialPort::EventDriven);
#else
                port=new QextSerialPort(nport.physName,portsettings,QextSerialPort::EventDriven);
#endif
                m_widget->connectButton->setEnabled(true);
                m_widget->disconnectButton->setEnabled(false);
                m_widget->connectButton->setHidden(false);
                m_widget->disconnectButton->setHidden(false);

                connect(port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
            }
        }
        m_widget->dataStreamGroupBox->setHidden(false);
    } else if (gpsDisplayConfig->connectionMode() == "Telemetry") {
        qDebug() << "Using Telemetry parser";
        parser = new TelemetryParser();
        m_widget->disconnectButton->setHidden(true);
        m_widget->connectButton->setHidden(true);
        m_widget->dataStreamGroupBox->setHidden(true);
    } else if (gpsDisplayConfig->connectionMode() == "Network") {
       // Not implemented for now...
        m_widget->connectButton->setEnabled(false);
        m_widget->disconnectButton->setEnabled(false);
        m_widget->dataStreamGroupBox->setHidden(false);
    }

    connect(parser, SIGNAL(sv(int)), m_widget,SLOT(setSVs(int)));
    connect(parser, SIGNAL(position(double,double,double)), m_widget,SLOT(setPosition(double,double,double)));
    connect(parser, SIGNAL(speedheading(double,double)), m_widget,SLOT(setSpeedHeading(double,double)));
    connect(parser, SIGNAL(datetime(double,double)), m_widget,SLOT(setDateTime(double,double)));
    connect(parser, SIGNAL(packet(QString)), m_widget, SLOT(dumpPacket(QString)));
    connect(parser, SIGNAL(satellite(int,int,int,int,int)), m_widget->gpsSky, SLOT(updateSat(int,int,int,int,int)));
    connect(parser, SIGNAL(satellite(int,int,int,int,int)), m_widget->gpsSnrWidget, SLOT(updateSat(int,int,int,int,int)));
    connect(parser, SIGNAL(fixtype(QString)), m_widget, SLOT(setFixType(QString)));
    connect(parser, SIGNAL(dop(double,double,double)), m_widget, SLOT(setDOP(double,double,double)));
}

void GpsDisplayGadget::onConnect() {
    m_widget->textBrowser->append(QString("Connecting to GPS ...\n"));
    // TODO: Somehow mark that we're running, and disable connect button while so?

    if (port) {
        qDebug() <<  "Opening: " <<  port->portName() << ".";
        bool isOpen =  port->open(QIODevice::ReadWrite);
        qDebug() <<  "Open: " << isOpen;
        if(isOpen) {
            m_widget->connectButton->setEnabled(false);
            m_widget->disconnectButton->setEnabled(true);
        }
    } else {
        qDebug() << "Port undefined or invalid.";
    }

}

void GpsDisplayGadget::onDisconnect() {
    if (port) {
        qDebug() <<  "Closing: " <<  port->portName() << ".";
        port->close();
        m_widget->connectButton->setEnabled(true);
        m_widget->disconnectButton->setEnabled(false);
    } else {
        qDebug() << "Port undefined or invalid.";
    }
}

void GpsDisplayGadget::onDataAvailable() {
    int avail = port->bytesAvailable();
    if( avail > 0 ) {
        QByteArray serialData;
        serialData.resize(avail);
        int bytesRead = port->read(serialData.data(), serialData.size());
        if( bytesRead > 0 ) {
            processNewSerialData(serialData);
        }
    }
}

void GpsDisplayGadget::processNewSerialData(QByteArray serialData) {
    int dataLength = serialData.size();
    const char* data = serialData.constData();

    for(int pos = 0; pos < dataLength; pos++) {
        parser->processInputStream(data[pos]);
    }
}
