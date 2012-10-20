/**
******************************************************************************
*
* @file       waypointline.cpp
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
* @brief      A graphicsItem representing a line connecting 2 waypoints
* @see        The GNU Public License (GPL) Version 3
* @defgroup   OPMapWidget
* @{
*
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
#include "waypointline.h"
#include <math.h>
#include "homeitem.h"

namespace mapcontrol
{
WayPointLine::WayPointLine(WayPointItem *from, WayPointItem *to, MapGraphicItem *map,QColor color):source(from),
    destination(to),my_map(map),QGraphicsLineItem(map),myColor(color)
{
    this->setLine(to->pos().x(),to->pos().y(),from->pos().x(),from->pos().y());
    connect(from,SIGNAL(localPositionChanged(QPointF,WayPointItem*)),this,SLOT(refreshLocations()));
    connect(to,SIGNAL(localPositionChanged(QPointF,WayPointItem*)),this,SLOT(refreshLocations()));
    connect(from,SIGNAL(aboutToBeDeleted(WayPointItem*)),this,SLOT(waypointdeleted()));
    connect(to,SIGNAL(aboutToBeDeleted(WayPointItem*)),this,SLOT(waypointdeleted()));
    if(myColor==Qt::green)
        this->setZValue(10);
    else if(myColor==Qt::yellow)
        this->setZValue(9);
    else if(myColor==Qt::red)
        this->setZValue(8);
    connect(map,SIGNAL(childSetOpacity(qreal)),this,SLOT(setOpacitySlot(qreal)));
}

WayPointLine::WayPointLine(HomeItem *from, WayPointItem *to, MapGraphicItem *map, QColor color):source(from),
    destination(to),my_map(map),QGraphicsLineItem(map),myColor(color)
{
    this->setLine(to->pos().x(),to->pos().y(),from->pos().x(),from->pos().y());
    connect(from,SIGNAL(homePositionChanged(internals::PointLatLng,float)),this,SLOT(refreshLocations()));
    connect(to,SIGNAL(localPositionChanged(QPointF,WayPointItem*)),this,SLOT(refreshLocations()));
    connect(to,SIGNAL(aboutToBeDeleted(WayPointItem*)),this,SLOT(waypointdeleted()));
    if(myColor==Qt::green)
        this->setZValue(10);
    else if(myColor==Qt::yellow)
        this->setZValue(9);
    else if(myColor==Qt::red)
        this->setZValue(8);
    connect(map,SIGNAL(childSetOpacity(qreal)),this,SLOT(setOpacitySlot(qreal)));
}
int WayPointLine::type() const
{
    // Enable the use of qgraphicsitem_cast with this item.
    return Type;
}
QPainterPath WayPointLine::shape() const
{
    QPainterPath path = QGraphicsLineItem::shape();
    path.addPolygon(arrowHead);
    return path;
}
void WayPointLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen myPen = pen();
    myPen.setColor(myColor);
    qreal arrowSize = 10;
    painter->setPen(myPen);
    painter->setBrush(myColor);

    double angle = ::acos(line().dx() / line().length());
    if (line().dy() >= 0)
        angle = (M_PI * 2) - angle;

        QPointF arrowP1 = line().pointAt(0.5) + QPointF(sin(angle + M_PI / 3) * arrowSize,
                                        cos(angle + M_PI / 3) * arrowSize);
        QPointF arrowP2 = line().pointAt(0.5) + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                        cos(angle + M_PI - M_PI / 3) * arrowSize);
        arrowHead.clear();
        arrowHead << line().pointAt(0.5) << arrowP1 << arrowP2;
        painter->drawPolygon(arrowHead);
        if(myColor==Qt::red)
            myPen.setWidth(3);
        else if(myColor==Qt::yellow)
            myPen.setWidth(2);
        else if(myColor==Qt::green)
            myPen.setWidth(1);
        painter->setPen(myPen);
        painter->drawLine(line());

}

void WayPointLine::refreshLocations()
{
    this->setLine(destination->pos().x(),destination->pos().y(),source->pos().x(),source->pos().y());
}

void WayPointLine::waypointdeleted()
{
    this->deleteLater();
}

void WayPointLine::setOpacitySlot(qreal opacity)
{
    setOpacity(opacity);
}

}
