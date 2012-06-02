/**
******************************************************************************
*
* @file       waypointcircle.cpp
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
* @brief      A graphicsItem representing a circle connecting 2 waypoints
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
#include "waypointcircle.h"
#include <math.h>
#include "homeitem.h"
const qreal Pi = 3.14;

namespace mapcontrol
{
WayPointCircle::WayPointCircle(WayPointItem *center, WayPointItem *radius,bool clockwise, MapGraphicItem *map,QColor color):my_center(center),
    my_radius(radius),my_map(map),QGraphicsEllipseItem(map),myColor(color),myClockWise(clockwise)
{
    connect(center,SIGNAL(localPositionChanged(QPointF)),this,SLOT(refreshLocations()));
    connect(radius,SIGNAL(localPositionChanged(QPointF)),this,SLOT(refreshLocations()));
    connect(center,SIGNAL(aboutToBeDeleted(WayPointItem*)),this,SLOT(waypointdeleted()));
    connect(radius,SIGNAL(aboutToBeDeleted(WayPointItem*)),this,SLOT(waypointdeleted()));
}

WayPointCircle::WayPointCircle(HomeItem *center, WayPointItem *radius, bool clockwise, MapGraphicItem *map, QColor color):my_center(center),
    my_radius(radius),my_map(map),QGraphicsEllipseItem(map),myColor(color),myClockWise(clockwise)
{
    connect(center,SIGNAL(homePositionChanged(internals::PointLatLng)),this,SLOT(refreshLocations()));
    connect(radius,SIGNAL(localPositionChanged(QPointF)),this,SLOT(refreshLocations()));
    connect(radius,SIGNAL(aboutToBeDeleted(WayPointItem*)),this,SLOT(waypointdeleted()));
}

int WayPointCircle::type() const
{
    // Enable the use of qgraphicsitem_cast with this item.
    return Type;
}
QPainterPath WayPointCircle::shape() const
{
    QPainterPath path = QGraphicsEllipseItem::shape();
    path.addPolygon(arrowHead);
    return path;
}
void WayPointCircle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPointF p1;
    QPointF p2;
    p1=my_center->pos();
    p2=my_center->pos();
    QLineF line(my_radius->pos(),my_center->pos());
    p1.ry()=p1.ry()+line.length();
    p2.ry()=p2.ry()-line.length();
    QPen myPen = pen();
    myPen.setColor(myColor);
    qreal arrowSize = 10;
    painter->setPen(myPen);
    QBrush brush=painter->brush();
    painter->setBrush(myColor);
    double angle =0;
    if(myClockWise)
        angle+=Pi;
    if (line.dy() >= 0)
        angle = (Pi) - angle;

        QPointF arrowP1 = p1 + QPointF(sin(angle + Pi / 3) * arrowSize,
                                        cos(angle + Pi / 3) * arrowSize);
        QPointF arrowP2 = p1 + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                        cos(angle + Pi - Pi / 3) * arrowSize);

        QPointF arrowP21 = p2 + QPointF(sin(angle + Pi + Pi / 3) * arrowSize,
                                        cos(angle + Pi + Pi / 3) * arrowSize);
        QPointF arrowP22 = p2 + QPointF(sin(angle + Pi + Pi - Pi / 3) * arrowSize,
                                        cos(angle + Pi + Pi - Pi / 3) * arrowSize);

        arrowHead.clear();
        arrowHead << p1 << arrowP1 << arrowP2;
        painter->drawPolygon(arrowHead);
        arrowHead.clear();
        arrowHead << p2 << arrowP21 << arrowP22;
        painter->drawPolygon(arrowHead);
        painter->translate(-line.length(),-line.length());
        painter->setBrush(brush);
        painter->drawEllipse(this->rect());

}

void WayPointCircle::refreshLocations()
{
    QLineF line(my_center->pos(),my_radius->pos());
    this->setRect(my_center->pos().x(),my_center->pos().y(),2*line.length(),2*line.length());
}

void WayPointCircle::waypointdeleted()
{
    this->deleteLater();
}

}
