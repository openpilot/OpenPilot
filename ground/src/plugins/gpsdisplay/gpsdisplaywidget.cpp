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
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"

#include <iostream>
#include <QtGui>
#include <QDebug>

/*
 * Initialize the widget
 */
GpsDisplayWidget::GpsDisplayWidget(QWidget *parent) : QWidget(parent)
{
    setupUi(this);

    //Not elegant, just load the image for now
    QGraphicsScene *fescene = new QGraphicsScene(this);
    QPixmap earthpix( ":/gpsgadget/images/flatEarth.png" );
    fescene->addPixmap( earthpix );
    flatEarth->setScene(fescene);
}

GpsDisplayWidget::~GpsDisplayWidget()
{
}

void GpsDisplayWidget::setSpeedHeading(double speed, double heading)
{
    QString temp = "Speed: ";
    temp.append(QString::number(speed,'g',10));
    temp.append(" Heaging: ");
    temp.append(QString::number(heading,'g',10));
    speed_value->setText(QString::number(speed,'g',10));
    speed_value->adjustSize();
    bear_value->setText(QString::number(heading,'g',10));
    bear_value->adjustSize();

    // widget->textBrowser->append(temp);
}

void GpsDisplayWidget::setDateTime(double date, double time)
{
    QString temp = "Date: ";
    temp.append(QString::number(date,'g',10));
    temp.append(" Time: ");
    temp.append(QString::number(time,'g',10));
    gdate_value->setText(QString::number(date,'g',10));
    gdate_value->adjustSize();
    gtime_value->setText(QString::number(time,'g',10));
    gdate_value->adjustSize();

    //textBrowser->append(temp);
}

void GpsDisplayWidget::dumpPacket(char *packet)
{
    textBrowser->append(QString(packet));
}

void GpsDisplayWidget::setSVs(int sv)
{
    QString temp = "Fix: Sats: ";
    temp.append(QString::number(sv));
    status_value->setText(temp);
    status_value->adjustSize();
}

void GpsDisplayWidget::setPosition(double lat, double lon, double alt)
{
    QString temp = "Position: ";
    temp.append(QString::number(lat,'g',10));
    temp.append(" ");
    temp.append(QString::number(lon,'g',10));
    temp.append(" ");
    temp.append(QString::number(alt,'g',10));
    lat_value->setText(QString::number(lat,'g',10));
    lat_value->adjustSize();
    long_value->setText(QString::number(lon,'g',10));
    long_value->adjustSize();
    //alt_value->setText(QString::number(alt,'g',10));
    //alt_value->adjustSize();

    //textBrowser->append(temp);
}
