/**
******************************************************************************
*
* @file       homeitem.cpp
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
* @brief      A graphicsItem representing a trail point
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
#include "homeitem.h"
namespace mapcontrol
{
    HomeItem::HomeItem(MapGraphicItem* map,OPMapWidget* parent):safe(true),map(map),mapwidget(parent),
        showsafearea(true),toggleRefresh(true),safearea(1000),altitude(0),isDragging(false)
    {
        pic.load(QString::fromUtf8(":/markers/images/home2.svg"));
        pic=pic.scaled(30,30,Qt::IgnoreAspectRatio);
        this->setFlag(QGraphicsItem::ItemIgnoresTransformations,true);
        this->setFlag(QGraphicsItem::ItemIsMovable,false);
        this->setFlag(QGraphicsItem::ItemIsSelectable,true);
        localposition=map->FromLatLngToLocal(mapwidget->CurrentPosition());
        this->setPos(localposition.X(),localposition.Y());
        this->setZValue(4);
        coord=internals::PointLatLng(50,50);
        RefreshToolTip();
        setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        connect(map,SIGNAL(childRefreshPosition()),this,SLOT(RefreshPos()));
        connect(map,SIGNAL(childSetOpacity(qreal)),this,SLOT(setOpacitySlot(qreal)));
    }

    void HomeItem::RefreshToolTip()
    {
        QString coord_str = " " + QString::number(coord.Lat(), 'f', 6) + "   " + QString::number(coord.Lng(), 'f', 6);

        setToolTip(QString("Waypoint: Home\nCoordinate:%1\nAltitude:%2\n").arg(coord_str).arg(QString::number(altitude)));
    }


    void HomeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        painter->drawPixmap(-pic.width()/2,-pic.height()/2,pic);
        if(showsafearea)
        {
            if(safe)
                painter->setPen(Qt::green);
            else
                painter->setPen(Qt::red);
            painter->drawEllipse(QPointF(0,0),localsafearea,localsafearea);
            //   painter->drawRect(QRectF(-localsafearea,-localsafearea,localsafearea*2,localsafearea*2));
        }

    }
    QRectF HomeItem::boundingRect()const
    {
        if(pic.width()>localsafearea*2 && !toggleRefresh)
            return QRectF(-pic.width()/2,-pic.height()/2,pic.width(),pic.height());
        else
            return QRectF(-localsafearea,-localsafearea,localsafearea*2,localsafearea*2);
    }


    int HomeItem::type()const
    {
        return Type;
    }

    void HomeItem::RefreshPos()
    {
        prepareGeometryChange();
        localposition=map->FromLatLngToLocal(coord);
        this->setPos(localposition.X(),localposition.Y());
        if(showsafearea)
            localsafearea=safearea/map->Projection()->GetGroundResolution(map->ZoomTotal(),coord.Lat());

        RefreshToolTip();

        this->update();
        toggleRefresh=false;

    }

    void HomeItem::setOpacitySlot(qreal opacity)
    {
        setOpacity(opacity);
    }

    void HomeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        if(event->button()==Qt::LeftButton)
        {
            isDragging=true;
        }
        QGraphicsItem::mousePressEvent(event);
    }

    void HomeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        if(event->button()==Qt::LeftButton)
        {
            coord=map->FromLocalToLatLng(this->pos().x(),this->pos().y());
            isDragging=false;

            emit homePositionChanged(coord,Altitude());
        }
        QGraphicsItem::mouseReleaseEvent(event);

        RefreshToolTip();
    }
    void HomeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
    {
        if(event->button()==Qt::LeftButton)
        {
            emit homedoubleclick(this);
        }
    }
    void HomeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {

        if(isDragging)
        {
            coord=map->FromLocalToLatLng(this->pos().x(),this->pos().y());
            emit homePositionChanged(coord,Altitude());
        }
            QGraphicsItem::mouseMoveEvent(event);
    }

    //Set clickable area as smaller than the bounding rect.
    QPainterPath HomeItem::shape() const
     {
         QPainterPath path;
         path.addEllipse(QRectF(-12, -25, 24, 50));
         return path;
     }

}

