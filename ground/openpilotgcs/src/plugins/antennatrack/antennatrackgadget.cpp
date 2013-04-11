/**
 ******************************************************************************
 *
 * @file       AntennaTracgadget.cpp
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

#include "antennatrackgadget.h"
#include "antennatrackwidget.h"
#include "antennatrackgadgetconfiguration.h"

AntennaTrackGadget::AntennaTrackGadget(QString classId, AntennaTrackWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
        m_widget(widget),
        connected(FALSE)
{
    connect(m_widget->connectButton, SIGNAL(clicked(bool)), this,SLOT(onConnect()));
    connect(m_widget->disconnectButton, SIGNAL(clicked(bool)), this,SLOT(onDisconnect()));
}

AntennaTrackGadget::~AntennaTrackGadget()
{
}

/*
  This is called when a configuration is loaded, and updates the plugin's settings.
  Careful: the plugin is already drawn before the loadConfiguration method is called the
  first time, so you have to be careful not to assume all the plugin values are initialized
  the first time you use them
 */
void AntennaTrackGadget::loadConfiguration(IUAVGadgetConfiguration* config)
{
    // Delete the (old)port, this also closes it.
    if(port) {
        delete port;
    }

    // Delete the (old)parser, this also disconnects all signals.
    if(parser) {
        delete parser;
    }

    AntennaTrackGadgetConfiguration *AntennaTrackConfig = qobject_cast< AntennaTrackGadgetConfiguration*>(config);

    PortSettings portsettings;
    portsettings.BaudRate=AntennaTrackConfig->speed();
    portsettings.DataBits=AntennaTrackConfig->dataBits();
    portsettings.FlowControl=AntennaTrackConfig->flow();
    portsettings.Parity=AntennaTrackConfig->parity();
    portsettings.StopBits=AntennaTrackConfig->stopBits();
    portsettings.Timeout_Millisec=AntennaTrackConfig->timeOut();

    // In case we find no port, buttons disabled
    m_widget->connectButton->setEnabled(false);
    m_widget->disconnectButton->setEnabled(false);

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    foreach( QextPortInfo nport, ports ) {
        if(nport.friendName == AntennaTrackConfig->port())
        {
            qDebug() << "Using Serial port";
            //parser = new NMEAParser();

#ifdef Q_OS_WIN
            port=new QextSerialPort(nport.portName,portsettings,QextSerialPort::EventDriven);
#else
            port=new QextSerialPort(nport.physName,portsettings,QextSerialPort::EventDriven);
#endif
            m_widget->setPort(port);
            m_widget->connectButton->setEnabled(true);
            m_widget->disconnectButton->setEnabled(false);
            m_widget->connectButton->setHidden(false);
            m_widget->disconnectButton->setHidden(false);

            connect(port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
        }
    }
    m_widget->dataStreamGroupBox->setHidden(false);
    qDebug() << "Using Telemetry parser";
    parser = new TelemetryParser();

    connect(parser, SIGNAL(position(double,double,double)), m_widget,SLOT(setPosition(double,double,double)));
    connect(parser, SIGNAL(home(double,double,double)), m_widget,SLOT(setHomePosition(double,double,double)));
    connect(parser, SIGNAL(packet(QString)), m_widget, SLOT(dumpPacket(QString)));
}

void AntennaTrackGadget::onConnect() {
    m_widget->textBrowser->append(QString("Connecting to Tracker ...\n"));
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

void AntennaTrackGadget::onDisconnect() {
    if (port) {
        qDebug() <<  "Closing: " <<  port->portName() << ".";
        port->close();
        m_widget->connectButton->setEnabled(true);
        m_widget->disconnectButton->setEnabled(false);
    } else {
        qDebug() << "Port undefined or invalid.";
    }
}

void AntennaTrackGadget::onDataAvailable() {
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

void AntennaTrackGadget::processNewSerialData(QByteArray serialData) {
    int dataLength = serialData.size();
    const char* data = serialData.constData();
    m_widget->textBrowser->append(QString(serialData));
    for(int pos = 0; pos < dataLength; pos++) {
        //parser->processInputStream(data[pos]);
    }
}
