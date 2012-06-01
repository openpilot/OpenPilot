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

const qreal Pi = 3.14;

namespace mapcontrol
{
WayPointCircle::WayPointCircle(WayPointItem *from, WayPointItem *to,bool clockwise, MapGraphicItem *map,QColor color):source(from),
    destination(to),my_map(map),QGraphicsEllipseItem(map),myColor(color),myClockWise(clockwise)
{
    QLineF line(from->pos(),to->pos());
    this->setRect(from->pos().x(),from->pos().y(),2*line.length(),2*line.length());
    connect(source,SIGNAL(localPositionChanged(QPointF)),this,SLOT(refreshLocations()));
    connect(destination,SIGNAL(localPositionChanged(QPointF)),this,SLOT(refreshLocations()));
    connect(source,SIGNAL(aboutToBeDeleted(WayPointItem*)),this,SLOT(waypointdeleted()));
    connect(destination,SIGNAL(aboutToBeDeleted(WayPointItem*)),this,SLOT(waypointdeleted()));
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
    QLineF line(destination->pos(),source->pos());
    QPen myPen = pen();
    myPen.setColor(myColor);
    qreal arrowSize = 10;
    painter->setPen(myPen);
    QBrush brush=painter->brush();
    painter->setBrush(myColor);

    double angle = ::acos(line.dx() / line.length());
    angle=angle+90*2*Pi/360;
    if(myClockWise)
        angle+=Pi;
    if (line.dy() >= 0)
        angle = (Pi) - angle;

        QPointF arrowP1 = line.p1() + QPointF(sin(angle + Pi / 3) * arrowSize,
                                        cos(angle + Pi / 3) * arrowSize);
        QPointF arrowP2 = line.p1() + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                        cos(angle + Pi - Pi / 3) * arrowSize);

        arrowHead.clear();
        arrowHead << line.p1() << arrowP1 << arrowP2;
        painter->drawPolygon(arrowHead);
        painter->translate(-line.length(),-line.length());
        painter->setBrush(brush);
        painter->drawEllipse(this->rect());

}

void WayPointCircle::refreshLocations()
{
    QLineF line(source->pos(),destination->pos());
    this->setRect(source->pos().x(),source->pos().y(),2*line.length(),2*line.length());
}

void WayPointCircle::waypointdeleted()
{
    this->deleteLater();
}

}
