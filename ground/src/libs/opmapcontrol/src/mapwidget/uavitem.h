/**
******************************************************************************
*
* @file       uavitem.h
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
*             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
* @brief      A graphicsItem representing a WayPoint
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
#ifndef UAVITEM_H
#define UAVITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QLabel>
#include "../internals/pointlatlng.h"
#include "mapgraphicitem.h"
#include "waypointitem.h"
#include <QObject>
#include "uavmapfollowtype.h"
#include "uavtrailtype.h"
#include <QtSvg/QSvgRenderer>
#include "opmapwidget.h"
#include "trailitem.h"
namespace mapcontrol
{
    class WayPointItem;
    class OPMapWidget;
    /**
* @brief A QGraphicsItem representing the UAV
*
* @class UAVItem uavitem.h "mapwidget/uavitem.h"
*/
    class UAVItem:public QObject,public QGraphicsItem
    {
        Q_OBJECT
        Q_INTERFACES(QGraphicsItem)
    public:
                enum { Type = UserType + 2 };
        UAVItem(MapGraphicItem* map,OPMapWidget* parent);
        ~UAVItem();
        void SetUAVPos(internals::PointLatLng const& position,int const& altitude);
        void SetUAVHeading(qreal const& value);
        internals::PointLatLng UAVPos()const{return coord;}
        void SetMapFollowType(UAVMapFollowType::Types const& value){mapfollowtype=value;}
        void SetTrailType(UAVTrailType::Types const& value);
        UAVMapFollowType::Types GetMapFollowType()const{return mapfollowtype;}
        UAVTrailType::Types GetTrailType()const{return trailtype;}

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                    QWidget *widget);
        void RefreshPos();
        QRectF boundingRect() const;
        void SetTrailTime(int const& seconds){trailtime=seconds;}
        int TrailTime()const{return trailtime;}
        void SetTrailDistance(int const& distance){traildistance=distance;}
        int TrailDistance()const{return traildistance;}
        bool ShowTrail()const{return showtrail;}
        void SetShowTrail(bool const& value);
        void DeleteTrail()const;

        int type() const;
    private:
        MapGraphicItem* map;

        int altitude;
        UAVMapFollowType::Types mapfollowtype;
        UAVTrailType::Types trailtype;
        internals::PointLatLng coord;
        internals::PointLatLng lastcoord;
        QPixmap pic;
        core::Point localposition;
        OPMapWidget* mapwidget;
        QGraphicsItemGroup* trail;
        QTime timer;
        bool showtrail;
        int trailtime;
        int traildistance;
      //  QRectF rect;

    public slots:

    signals:
        void UAVReachedWayPoint(int const& waypointnumber,WayPointItem* waypoint);
        void UAVLeftSafetyBouble(internals::PointLatLng const& position);
    };
}
#endif // UAVITEM_H
