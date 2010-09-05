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
    marker = new QGraphicsSvgItem();
    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/gpsgadget/images/marker.svg"));
    marker->setSharedRenderer(renderer);
    fescene->addItem(marker);
    double scale = earthpix.width()/(marker->boundingRect().width()*20);
    marker->setScale(scale);
}

GpsDisplayWidget::~GpsDisplayWidget()
{
}

void GpsDisplayWidget::setSpeedHeading(double speed, double heading)
{
    QString str;
    str.sprintf("%.02f m/s",speed);
    speed_value->setText(str);
    speed_value->adjustSize();
    bear_value->setText(str.sprintf("%.02f deg",heading));
    bear_value->adjustSize();

}

void GpsDisplayWidget::setDateTime(double date, double time)
{
    QString dstring1 = QString::number(date,'g',10);
    dstring1.insert(6,".");
    dstring1.insert(4,".");
    QString dstring2 = QString::number(time,'g',10);
    dstring2.insert(dstring2.length()-2,":");
    dstring2.insert(dstring2.length()-5,":");
    time_value->setText(dstring1 + "    " + dstring2 + " GMT");

}

void GpsDisplayWidget::setFixType(QString fixtype)
{
  fix_value->setText(fixtype);
}

void GpsDisplayWidget::dumpPacket(char *packet)
{
    textBrowser->append(QString(packet));
}

void GpsDisplayWidget::setSVs(int sv)
{
    QString temp = "Sats: ";
    temp.append(QString::number(sv));
    status_value->setText(temp);
    status_value->adjustSize();
}

void GpsDisplayWidget::setDOP(double hdop, double vdop, double pdop)
{

    QString str;
    str.sprintf("%.2f / %.2f / %.2f", hdop, vdop, pdop);
    dop_value->setText(str);

}

void GpsDisplayWidget::setPosition(double lat, double lon, double alt)
{
    //lat *= 1E-7;
    //lon *= 1E-7;
    double deg = (lat>0) ? floor(lat):ceil(lat);
    double min = fabs(lat-deg)*60;
    QString str1;
    str1.sprintf("%.0f%c%.3f' ", deg,0x00b0, min);
    if (lat>0)
        str1.append("N");
    else
        str1.append("S");
    coord_value->setText(str1);
    deg = floor(fabs(lon));  // ABS takes an int.
    min = fabs(lon-deg)*60;
    QString str2;
    str2.sprintf("%.0f%c%.3f' ", deg,0x00b0, min);
    if (lon>0)
        str2.append("E");
    else
        str2.append("W");
    coord_value_2->setText(str2);
    QString str3;
    str3.sprintf("%.2f m", alt);
    coord_value_3->setText(str3);

    // Now place the marker:
    double wscale = flatEarth->sceneRect().width()/360;
    double hscale = flatEarth->sceneRect().height()/180;
    QPointF opd = QPointF((lon+180)*wscale-marker->boundingRect().width()*marker->scale()/2,
                          (90-lat)*hscale-marker->boundingRect().height()*marker->scale()/2);
    marker->setTransform(QTransform::fromTranslate( opd.x(), opd.y()) , false);

}
