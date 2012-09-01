/**
******************************************************************************
*
* @file       gpsitem.cpp
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
* @brief      A graphicsItem representing a UAV
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
#include "../internals/pureprojection.h"
#include "gpsitem.h"
namespace mapcontrol
{
    GPSItem::GPSItem(MapGraphicItem* map,OPMapWidget* parent,QString uavPic):map(map),mapwidget(parent),showtrail(true),showtrailline(true),trailtime(5),traildistance(50),autosetreached(true)
    ,autosetdistance(100)
    {
        pic.load(uavPic);
       // Don't scale but trust the image we are given
       // pic=pic.scaled(50,33,Qt::IgnoreAspectRatio);
        localposition=map->FromLatLngToLocal(mapwidget->CurrentPosition());
        this->setPos(localposition.X(),localposition.Y());
        this->setZValue(4);
        trail=new QGraphicsItemGroup(this);
        trail->setParentItem(map);
        trailLine=new QGraphicsItemGroup(this);
        trailLine->setParentItem(map);
        this->setFlag(QGraphicsItem::ItemIgnoresTransformations,true);
        mapfollowtype=UAVMapFollowType::None;
        trailtype=UAVTrailType::ByDistance;
        timer.start();
        connect(map,SIGNAL(childRefreshPosition()),this,SLOT(RefreshPos()));
        connect(map,SIGNAL(childSetOpacity(qreal)),this,SLOT(setOpacitySlot(qreal)));
    }
    GPSItem::~GPSItem()
    {

    }

    void GPSItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        painter->drawPixmap(-pic.width()/2,-pic.height()/2,pic);
    }
    QRectF GPSItem::boundingRect()const
    {
        return QRectF(-pic.width()/2,-pic.height()/2,pic.width(),pic.height());
    }
    void GPSItem::SetUAVPos(const internals::PointLatLng &position, const int &altitude)
    {
        if(coord.IsEmpty())
            lastcoord=coord;
        if(coord!=position)
        {

            if(trailtype==UAVTrailType::ByTimeElapsed)
            {
                if(timer.elapsed()>trailtime*1000)
                {
                    TrailItem * ob=new TrailItem(position,altitude,Qt::green,map);
                    trail->addToGroup(ob);
                    connect(this,SIGNAL(setChildPosition()),ob,SLOT(setPosSLOT()));
                    if(!lasttrailline.IsEmpty())
                    {
                        TrailLineItem * obj=new TrailLineItem(lasttrailline,position,Qt::red,map);
                        trailLine->addToGroup(obj);
                        connect(this,SIGNAL(setChildLine()),obj,SLOT(setLineSlot()));
                    }
                    lasttrailline=position;
                    timer.restart();
                }

            }
            else if(trailtype==UAVTrailType::ByDistance)
            {
                if(qAbs(internals::PureProjection::DistanceBetweenLatLng(lastcoord,position)*1000)>traildistance)
                {
                    TrailItem * ob=new TrailItem(position,altitude,Qt::green,map);
                    trail->addToGroup(ob);
                    connect(this,SIGNAL(setChildPosition()),ob,SLOT(setPosSLOT()));
                    if(!lasttrailline.IsEmpty())
                    {
                        TrailLineItem * obj=new TrailLineItem(lasttrailline,position,Qt::red,map);
                        trailLine->addToGroup(obj);
                        connect(this,SIGNAL(setChildLine()),obj,SLOT(setLineSlot()));
                    }
                    lasttrailline=position;
                    lastcoord=position;
                }
            }
            coord=position;
            this->altitude=altitude;
            RefreshPos();
        }
    }

    /**
      * Rotate the UAV Icon on the map, or rotate the map
      * depending on the display mode
      */
    void GPSItem::SetUAVHeading(const qreal &value)
    {
        if(mapfollowtype==UAVMapFollowType::CenterAndRotateMap)
        {
            mapwidget->SetRotate(-value);
        }
        else {
            if (this->rotation() != value)
                this->setRotation(value);
        }
    }


    int GPSItem::type()const
    {
        return Type;
    }


    void GPSItem::RefreshPos()
    {
        localposition=map->FromLatLngToLocal(coord);
        this->setPos(localposition.X(),localposition.Y());
        emit setChildPosition();
        emit setChildLine();

    }

    void GPSItem::setOpacitySlot(qreal opacity)
    {
        setOpacity(opacity);
    }
    void GPSItem::SetTrailType(const UAVTrailType::Types &value)
    {
        trailtype=value;
        if(trailtype==UAVTrailType::ByTimeElapsed)
            timer.restart();
    }
    void GPSItem::SetShowTrail(const bool &value)
    {
        showtrail=value;
        trail->setVisible(value);

    }
    void GPSItem::SetShowTrailLine(const bool &value)
    {
        showtrailline=value;
        trailLine->setVisible(value);
    }
    void GPSItem::DeleteTrail()const
    {
        foreach(QGraphicsItem* i,trail->childItems())
            delete i;
        foreach(QGraphicsItem* i,trailLine->childItems())
            delete i;
    }
    double GPSItem::Distance3D(const internals::PointLatLng &coord, const int &altitude)
    {
       return sqrt(pow(internals::PureProjection::DistanceBetweenLatLng(this->coord,coord)*1000,2)+
       pow(this->altitude-altitude,2));

    }
    void GPSItem::SetUavPic(QString UAVPic)
    {
        pic.load(":/uavs/images/"+UAVPic);
    }
}
