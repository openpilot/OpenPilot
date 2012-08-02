/**
******************************************************************************
*
* @file       uavitem.cpp
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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
#include "uavitem.h"

const qreal Pi = 3.14;

namespace mapcontrol
{
    UAVItem::UAVItem(MapGraphicItem* map,OPMapWidget* parent,QString uavPic):map(map),mapwidget(parent),showtrail(true),showtrailline(true),trailtime(5),traildistance(50),autosetreached(true)
    ,autosetdistance(100),altitude(0)
    {
        pic.load(uavPic);
        this->setFlag(QGraphicsItem::ItemIsMovable,true);
        this->setFlag(QGraphicsItem::ItemIsSelectable,true);
        localposition=map->FromLatLngToLocal(mapwidget->CurrentPosition());
        this->setPos(localposition.X(),localposition.Y());
        this->setZValue(4);
        trail=new QGraphicsItemGroup();
        trail->setParentItem(map);
        trailLine=new QGraphicsItemGroup();
        trailLine->setParentItem(map);
        this->setFlag(QGraphicsItem::ItemIgnoresTransformations,true);
        mapfollowtype=UAVMapFollowType::None;
        trailtype=UAVTrailType::ByDistance;
        timer.start();
    }
    UAVItem::~UAVItem()
    {
        delete trail;
    }

    void UAVItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);


        QPen myPen;

        //Turn on anti-aliasing so the fonts don't look terrible
        painter->setRenderHint(QPainter::Antialiasing, true);

        qreal arrowSize = 10;

        //Set pen attributes
        QColor myColor(Qt::red);
        myPen.setWidth(3);
        myPen.setColor(myColor);
        painter->setPen(myPen);

        //Set brush attributes
        painter->setBrush(myColor);

        //Create line from (0,0), to (1,1). Later, we'll scale and rotate it
        QLineF line(0,0,1.0,1.0);

        //Set the starting point to (0,0)
        line.setP1(QPointF(0,0));

        //Set angle and length
        line.setLength(60.0);
        line.setAngle(90.0);

        //Form arrowhead
        double angle = ::acos(line.dx() / line.length());
        if (line.dy() <= 0)
            angle = (Pi * 2) - angle;

        QPointF arrowP1 = line.pointAt(1) + QPointF(sin(angle + Pi / 3) * arrowSize,
                                                      cos(angle + Pi / 3) * arrowSize);
        QPointF arrowP2 = line.pointAt(1) + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                                      cos(angle + Pi - Pi / 3) * arrowSize);

        //Generate arrowhead
        arrowHead.clear();
        arrowHead << line.pointAt(1) << arrowP1 << arrowP2;
        painter->drawPolygon(arrowHead);
        painter->setPen(myPen);
        painter->drawLine(line);

        //*********** Create trend arc
        float radius;

        float spanAngle = yawRate_dps * 3; //Forecast 3 seconds into the future

        float meters2pixels=5; //This should be a function of the zoom level, and not a fixed constant
        radius=fabs((groundspeed_kph/3.6)/(yawRate_dps*Pi/180))*meters2pixels; //Calculate radius in [m], and then convert to Px.
//        qDebug()<< "Radius:" << radius;
//        qDebug()<< "Span angle:" << spanAngle;

        //Set trend arc's color
        myPen.setColor(Qt::magenta);
        painter->setPen(myPen);

        //Draw arc. Qt is incomprehensibly limited in that it does not let you set a radius and two points,
        //so instead it's a hackish approach requiring a rectangle and a span angle
        if (spanAngle > 0){
            QRectF rect(0, -radius, radius*2, radius*2);
            painter->drawArc(rect, 180*16, -spanAngle*16);
        }
        else{
            QRectF rect(-2*radius, -radius, radius*2, radius*2);
            painter->drawArc(rect, 0*16, -spanAngle*16);
        }

        //HUH? What does this do?
        painter->drawPixmap(-pic.width()/2,-pic.height()/2,pic);

        //***** Text info overlay. The font is a "glow" font, so that it's easier to use on the map

        //Rotate the text back to vertical
        qreal rot=this->rotation();
        painter->rotate(-1*rot);

        //Define font
        QFont borderfont( "Arial", 14, QFont::Normal, false );

        //Top left corner of text
        int textAnchorX = 20;
        int textAnchorY = 20;

        //Create text lines
        QString uavoInfoStrLine1, uavoInfoStrLine2;
        QString uavoInfoStrLine3;
        QString uavoInfoStrLine4;

        //For whatever reason, Qt does not let QPainterPath have text wrapping. So each line of
        //text has to be added to a different line.
        uavoInfoStrLine1.append(QString("CAS: %1 kph").arg(CAS_mps));
        uavoInfoStrLine2.append(QString("Groundspeed: %1 kph").arg(groundspeed_kph));
        uavoInfoStrLine3.append(QString("Lat-Lon: %1, %2").arg(coord.Lat()).arg(coord.Lng()));
        uavoInfoStrLine4.append(QString("Altitude: %1 m").arg(this->altitude));

        //Add the lines of text to the path
        //NOTE: We must use QPainterPath for the outlined text font. QPaint does not support this.
        QPainterPath path;
        path.addText(textAnchorX, textAnchorY+16*0, borderfont, uavoInfoStrLine1);
        path.addText(textAnchorX, textAnchorY+16*1, borderfont, uavoInfoStrLine2);
        path.addText(textAnchorX, textAnchorY+16*2, borderfont, uavoInfoStrLine3);
        path.addText(textAnchorX, textAnchorY+16*3, borderfont, uavoInfoStrLine4);

        //First pass is the outline...
        myPen.setWidth(4);
        myPen.setColor(Qt::black);
        painter->setPen(myPen);
        painter->setBrush(Qt::black);
        painter->drawPath(path);

        //...second pass is the inlay
        myPen.setWidth(1);
        myPen.setColor(Qt::white);
        painter->setBrush(Qt::white);
        painter->setPen(myPen);
        painter->drawPath(path);

    }

    QRectF UAVItem::boundingRect()const
    {
        return QRectF(-pic.width()/2,-pic.height()/2,pic.width(),pic.height());
    }

    void UAVItem::SetNED(double NED[3]){
        this->NED[0] = NED[0];
        this->NED[1] = NED[1];
        this->NED[2] = NED[2];
    }

    void UAVItem::SetYawRate(double yawRate_dps){
        this->yawRate_dps=yawRate_dps;

        //There is a minimum arc distance under which Qt no longer draws an arc. At this low a turning rate,
        //all curves look straight, so the user won't notice the difference. Moreover, our sensors aren't
        //accurate enough to reliably describe this low a yaw rate.
        if (fabs(this->yawRate_dps) < 5e-1){ //This number is really the smallest we can go. Any smaller, and it might have problems if we forecast a shorter distance into the future
            this->yawRate_dps=5e-1;
        }

    }

    void UAVItem::SetCAS(double CAS_mps){
        this->CAS_mps=CAS_mps;
    }

    void UAVItem::SetGroundspeed(double vNED[3]){
        this->vNED[0] = vNED[0];
        this->vNED[1] = vNED[1];
        this->vNED[2] = vNED[2];
        groundspeed_kph=sqrt(vNED[0]*vNED[0] + vNED[1]*vNED[1] + vNED[2]*vNED[2])*3.6;
    }


    void UAVItem::SetUAVPos(const internals::PointLatLng &position, const int &altitude)
    {
        if(coord.IsEmpty())
            lastcoord=coord;
        if(coord!=position)
        {

            if(trailtype==UAVTrailType::ByTimeElapsed)
            {
                if(timer.elapsed()>trailtime*1000)
                {
                    trail->addToGroup(new TrailItem(position,altitude,Qt::red,this));
                    if(!lasttrailline.IsEmpty())
                        trailLine->addToGroup((new TrailLineItem(lasttrailline,position,Qt::red,map)));
                    lasttrailline=position;
                    timer.restart();
                }

            }
            else if(trailtype==UAVTrailType::ByDistance)
            {
                if(qAbs(internals::PureProjection::DistanceBetweenLatLng(lastcoord,position)*1000)>traildistance)
                {
                    trail->addToGroup(new TrailItem(position,altitude,Qt::red,this));
                    if(!lasttrailline.IsEmpty())
                        trailLine->addToGroup((new TrailLineItem(lasttrailline,position,Qt::red,map)));
                    lasttrailline=position;
                    lastcoord=position;
                }
            }
            coord=position;
            this->altitude=altitude;
            RefreshPos();
            if(mapfollowtype==UAVMapFollowType::CenterAndRotateMap||mapfollowtype==UAVMapFollowType::CenterMap)
            {
                mapwidget->SetCurrentPosition(coord);
            }
            this->update();
            if(autosetreached)
            {
                foreach(QGraphicsItem* i,map->childItems())
                {
                    WayPointItem* wp=qgraphicsitem_cast<WayPointItem*>(i);
                    if(wp)
                    {
                        if(Distance3D(wp->Coord(),wp->Altitude())<autosetdistance)
                        {
                            wp->SetReached(true);
                            emit UAVReachedWayPoint(wp->Number(),wp);
                        }
                    }
                }
            }
            if(mapwidget->Home!=0)
            {
                //verify if the UAV is inside the safety bouble
                if(Distance3D(mapwidget->Home->Coord(),mapwidget->Home->Altitude())>mapwidget->Home->SafeArea())
                {
                    if(mapwidget->Home->safe!=false)
                    {
                        mapwidget->Home->safe=false;
                        mapwidget->Home->update();
                        emit UAVLeftSafetyBouble(this->coord);
                    }
                }
                else
                {
                    if(mapwidget->Home->safe!=true)
                    {
                        mapwidget->Home->safe=true;
                        mapwidget->Home->update();
                    }
                }

            }
        }
    }

    /**
      * Rotate the UAV Icon on the map, or rotate the map
      * depending on the display mode
      */
    void UAVItem::SetUAVHeading(const qreal &value)
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


    int UAVItem::type()const
    {
        return Type;
    }


    void UAVItem::RefreshPos()
    {
        localposition=map->FromLatLngToLocal(coord);
        this->setPos(localposition.X(),localposition.Y());
        foreach(QGraphicsItem* i,trail->childItems())
        {
            TrailItem* w=qgraphicsitem_cast<TrailItem*>(i);
            if(w)
                w->setPos(map->FromLatLngToLocal(w->coord).X(),map->FromLatLngToLocal(w->coord).Y());
        }
        foreach(QGraphicsItem* i,trailLine->childItems())
        {
            TrailLineItem* ww=qgraphicsitem_cast<TrailLineItem*>(i);
            if(ww)
                ww->setLine(map->FromLatLngToLocal(ww->coord1).X(),map->FromLatLngToLocal(ww->coord1).Y(),map->FromLatLngToLocal(ww->coord2).X(),map->FromLatLngToLocal(ww->coord2).Y());
        }

    }
    void UAVItem::SetTrailType(const UAVTrailType::Types &value)
    {
        trailtype=value;
        if(trailtype==UAVTrailType::ByTimeElapsed)
            timer.restart();
    }
    void UAVItem::SetShowTrail(const bool &value)
    {
        showtrail=value;
        trail->setVisible(value);
    }
    void UAVItem::SetShowTrailLine(const bool &value)
    {
        showtrailline=value;
        trailLine->setVisible(value);
    }

    void UAVItem::DeleteTrail()const
    {
        foreach(QGraphicsItem* i,trail->childItems())
            delete i;
        foreach(QGraphicsItem* i,trailLine->childItems())
            delete i;
    }
    double UAVItem::Distance3D(const internals::PointLatLng &coord, const int &altitude)
    {
       return sqrt(pow(internals::PureProjection::DistanceBetweenLatLng(this->coord,coord)*1000,2)+
       pow(this->altitude-altitude,2));
    }
    void UAVItem::SetUavPic(QString UAVPic)
    {
        pic.load(":/uavs/images/"+UAVPic);
    }
}
