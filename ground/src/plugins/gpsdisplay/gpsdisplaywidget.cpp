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
#include <QtGui/QFileDialog>
#include <QDebug>
#include <QThread>
#include "nmeaparser.h"



class MyThread : public QThread
{
public:
    QextSerialPort *port;
    NMEAParser *parser;
    void setPort(QextSerialPort* port);
    void processInputStream();
    void run();
};



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


    connect(widget->connectButton, SIGNAL(clicked(bool)),
            this,SLOT(connectButtonClicked()));
}

GpsDisplayWidget::~GpsDisplayWidget()
{
   delete widget;
}


void GpsDisplayWidget::setPort(QextSerialPort* port)
{

    this->port=port;
}

void GpsDisplayWidget::connectButtonClicked() {
    MyThread* myThread = new MyThread();
    myThread->setPort(port);
    myThread->start();
}


void MyThread::setPort(QextSerialPort* port)
{

    this->port=port;
}

void MyThread::run()
{
    qDebug() <<  "Opening.";

    qDebug() <<  port->portName();

    bool isOpen =  port->open(QIODevice::ReadWrite);
    qDebug() <<  "Open: " << isOpen;
    parser=new NMEAParser();

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
}


