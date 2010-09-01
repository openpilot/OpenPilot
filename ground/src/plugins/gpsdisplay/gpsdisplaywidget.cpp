/**
 ******************************************************************************
 *
 * @file       gpsdisplaywidget.cpp
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

#include "gpsdisplaywidget.h"
#include "ui_gpsdisplaywidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"

#include <iostream>
#include <QtGui>
#include <QDebug>
#include "nmeaparser.h"
#include "telemetryparser.h"



/*
 * Initialize the widget
 */
GpsDisplayWidget::GpsDisplayWidget(QWidget *parent) : QWidget(parent)
{
    widget = new Ui_GpsDisplayWidget();
    widget->setupUi(this);

    //Not elegant, just load the image for now
    QGraphicsScene *fescene = new QGraphicsScene(this);
    QPixmap earthpix( ":/gpsgadget/images/flatEarth.png" );
    fescene->addPixmap( earthpix );
    widget->flatEarth->setScene(fescene);

    connect(widget->connectButton, SIGNAL(clicked(bool)),
            this,SLOT(connectButtonClicked()));
    connect(widget->disconnectButton, SIGNAL(clicked(bool)),
            this,SLOT(disconnectButtonClicked()));
}

GpsDisplayWidget::~GpsDisplayWidget()
{
   delete widget;
}

void GpsDisplayWidget::setSpeedHeading(double speed, double heading)
{
    QString temp = "Speed: ";
    temp.append(QString::number(speed,'g',10));
    temp.append(" Heaging: ");
    temp.append(QString::number(heading,'g',10));
    widget->speed_value->setText(QString::number(speed,'g',10));
    widget->speed_value->adjustSize();
    widget->bear_value->setText(QString::number(heading,'g',10));
    widget->bear_value->adjustSize();

    // widget->textBrowser->append(temp);
}

void GpsDisplayWidget::setDateTime(double date, double time)
{
    QString temp = "Date: ";
    temp.append(QString::number(date,'g',10));
    temp.append(" Time: ");
    temp.append(QString::number(time,'g',10));
    widget->gdate_value->setText(QString::number(date,'g',10));
    widget->gdate_value->adjustSize();
    widget->gtime_value->setText(QString::number(time,'g',10));
    widget->gdate_value->adjustSize();

    //widget->textBrowser->append(temp);
}

void GpsDisplayWidget::dumpPacket(char *packet)
{
    widget->textBrowser->append(QString(packet));
}

void GpsDisplayWidget::setSVs(int sv)
{
    QString temp = "Fix: Sats: ";
    temp.append(QString::number(sv));
    widget->status_value->setText(temp);
    widget->status_value->adjustSize();
}

void GpsDisplayWidget::setPosition(double lat, double lon, double alt)
{
    QString temp = "Position: ";
    temp.append(QString::number(lat,'g',10));
    temp.append(" ");
    temp.append(QString::number(lon,'g',10));
    temp.append(" ");
    temp.append(QString::number(alt,'g',10));
    widget->lat_value->setText(QString::number(lat,'g',10));
    widget->lat_value->adjustSize();
    widget->long_value->setText(QString::number(lon,'g',10));
    widget->long_value->adjustSize();
    //widget->alt_value->setText(QString::number(alt,'g',10));
    //widget->alt_value->adjustSize();

    //widget->textBrowser->append(temp);
}


void GpsDisplayWidget::setPort(QextSerialPort* port)
{

    this->port=port;
}

void GpsDisplayWidget::setParser(QString connectionMode)
{

    if (connectionMode == "Serial") {
        parser = new NMEAParser();
        widget->connectButton->setEnabled(true);
        widget->disconnectButton->setEnabled(false);

    } else if (connectionMode == "Telemetry") {
        parser = new TelemetryParser();
        widget->connectButton->setEnabled(false);
        widget->disconnectButton->setEnabled(false);
    } else {
        parser = new NMEAParser(); // for the time being...
    }

    connect(parser,SIGNAL(sv(int)),this,SLOT(setSVs(int)));
    connect(parser,SIGNAL(position(double,double,double)),this,SLOT(setPosition(double,double,double)));
    connect(parser,SIGNAL(speedheading(double,double)),this,SLOT(setSpeedHeading(double,double)));
    connect(parser,SIGNAL(datetime(double,double)),this,SLOT(setDateTime(double,double)));
    connect(parser,SIGNAL(packet(char*)), this, SLOT(dumpPacket(char*)));
    connect(parser, SIGNAL(satellite(int,int,int,int,int)), widget->gpsSky, SLOT(updateSat(int,int,int,int,int)))
;
    port = NULL;

}

void GpsDisplayWidget::connectButtonClicked() {
    widget->textBrowser->append(QString("Connecting to GPS ...\n"));
    // TODO: Somehow mark that we're running, and disable connect button while so?

    if (port) {
        connect(port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));

        qDebug() <<  "Opening: " <<  port->portName() << ".";
        bool isOpen =  port->open(QIODevice::ReadWrite);
        qDebug() <<  "Open: " << isOpen;
    } else {
        qDebug() << "Port undefined or invalid.";
    }

}

void GpsDisplayWidget::disconnectButtonClicked() {
}

void GpsDisplayWidget::onDataAvailable() {
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

void GpsDisplayWidget::processNewSerialData(QByteArray serialData) {

    int dataLength = serialData.size();
    const char* data = serialData.constData();

    for(int pos = 0; pos < dataLength; pos++) {
        parser->processInputStream(data[pos]);
    }
}
