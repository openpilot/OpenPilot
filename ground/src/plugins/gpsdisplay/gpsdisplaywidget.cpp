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
#include "gpsdisplaythread.h"
#include "ui_gpsdisplaywidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"

#include <iostream>
#include <QtGui>
#include <QDebug>
#include "nmeaparser.h"



/*
 * Initialize the widget
 */
GpsDisplayWidget::GpsDisplayWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(128,128);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
 
    widget = new Ui_GpsDisplayWidget();
    widget->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(this);
    QSvgRenderer *renderer = new QSvgRenderer();
    QGraphicsSvgItem *world = new QGraphicsSvgItem();
    renderer->load(QString(":/gpsgadget/images/gpsEarth.svg"));
    world->setSharedRenderer(renderer);
    world->setElementId("map");
    scene->addItem(world);
    scene->setSceneRect(world->boundingRect());
    widget->gpsWorld->setScene(scene);
    // Somehow fitInView does not work there at all? Makes
    // the 'world' element tiny tiny tiny. anyone knows why??
    //widget->gpsWorld->fitInView(world,Qt::KeepAspectRatio);
    qreal factor = widget->gpsWorld->size().height()/world->boundingRect().height();
    widget->gpsWorld->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    world->setScale(factor);

    //Not elegant, just load the image for now
    QGraphicsScene *fescene = new QGraphicsScene(this);
    QPixmap earthpix( ":/gpsgadget/images/flatEarth.png" );
    fescene->addPixmap( earthpix );
    widget->flatEarth->setScene(fescene);

    connect(widget->connectButton, SIGNAL(clicked(bool)),
            this,SLOT(connectButtonClicked()));
    parser=new NMEAParser();
    connect(parser,SIGNAL(sv(int)),this,SLOT(setSVs(int)));
    connect(parser,SIGNAL(position(double,double,double)),this,SLOT(setPosition(double,double,double)));
    connect(parser,SIGNAL(speedheading(double,double)),this,SLOT(setSpeedHeading(double,double)));
    connect(parser,SIGNAL(datetime(double,double)),this,SLOT(setDateTime(double,double)));
    connect(parser,SIGNAL(packet(char*)), this, SLOT(dumpPacket(char*)));

    port = NULL;
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
    widget->speed_label->setText(QString::number(speed,'g',10));
    widget->speed_label->adjustSize();
    widget->bear_label->setText(QString::number(heading,'g',10));
    widget->bear_label->adjustSize();

    // widget->textBrowser->append(temp);
}

void GpsDisplayWidget::setDateTime(double date, double time)
{
    QString temp = "Date: ";
    temp.append(QString::number(date,'g',10));
    temp.append(" Time: ");
    temp.append(QString::number(time,'g',10));
    widget->gdate_label->setText(QString::number(date,'g',10));
    widget->gdate_label->adjustSize();
    widget->gtime_label->setText(QString::number(time,'g',10));
    widget->gdate_label->adjustSize();

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
    widget->label_2->setText(temp);
    widget->label_2->adjustSize();
}

void GpsDisplayWidget::setPosition(double lat, double lon, double alt)
{
    QString temp = "Position: ";
    temp.append(QString::number(lat,'g',10));
    temp.append(" ");
    temp.append(QString::number(lon,'g',10));
    temp.append(" ");
    temp.append(QString::number(alt,'g',10));
    widget->lat_label->setText(QString::number(lat,'g',10));
    widget->lat_label->adjustSize();
    widget->long_label->setText(QString::number(lon,'g',10));
    widget->long_label->adjustSize();
    //widget->alt_label->setText(QString::number(alt,'g',10));
    //widget->alt_label->adjustSize();

    //widget->textBrowser->append(temp);
}

void GpsDisplayWidget::setPort(QextSerialPort* port)
{

    this->port=port;
}

void GpsDisplayWidget::setParser(NMEAParser* parser)
{

    this->parser=parser;
}

void GpsDisplayWidget::connectButtonClicked() {
    GpsDisplayThread* gpsThread = new GpsDisplayThread();
    widget->textBrowser->append(QString("Connecting to GPS ...\n"));
    gpsThread->setPort(port);
    gpsThread->setParser(parser);
    gpsThread->start();
}
