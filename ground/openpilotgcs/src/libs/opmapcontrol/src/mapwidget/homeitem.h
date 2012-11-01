/**
******************************************************************************
*
* @file       homeitem.h
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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
#ifndef HOMEITEM_H
#define HOMEITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QLabel>
#include "../internals/pointlatlng.h"
#include <QObject>
#include "opmapwidget.h"
namespace mapcontrol
{

    class HomeItem:public QObject,public QGraphicsItem
    {
        Q_OBJECT
        Q_INTERFACES(QGraphicsItem)
    public:
                enum { Type = UserType + 4 };
        HomeItem(MapGraphicItem* map,OPMapWidget* parent);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                    QWidget *widget);
        QRectF boundingRect() const;
        int type() const;
        bool ShowSafeArea()const{return showsafearea;}
        void SetShowSafeArea(bool const& value){showsafearea=value;}
        void SetToggleRefresh(bool const& value){toggleRefresh=value;}
        int SafeArea()const{return safearea;}
        void SetSafeArea(int const& value){safearea=value;}
        bool safe;
        void SetCoord(internals::PointLatLng const& value){coord=value;emit homePositionChanged(value,Altitude());}
        internals::PointLatLng Coord()const{return coord;}
        void SetAltitude(float const& value){altitude=value;emit homePositionChanged(Coord(),Altitude());}
        float Altitude()const{return altitude;}
        void RefreshToolTip();
    private:

        MapGraphicItem* map;
        OPMapWidget* mapwidget;
        QPixmap pic;
        core::Point localposition;
        internals::PointLatLng coord;
        bool showsafearea;
        bool toggleRefresh;
        int safearea;
        int localsafearea;
        float altitude;
        bool isDragging;
    protected:
        void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
        void mousePressEvent ( QGraphicsSceneMouseEvent * event );
        void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
        QPainterPath shape() const;
    public slots:
        void RefreshPos();
        void setOpacitySlot(qreal opacity);
    signals:
        void homePositionChanged(internals::PointLatLng coord,float);
        void homedoubleclick(HomeItem* waypoint);
    };
}
#endif // HOMEITEM_H
