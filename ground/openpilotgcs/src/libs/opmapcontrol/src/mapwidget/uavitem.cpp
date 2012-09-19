/**
******************************************************************************
*
* @file       uavitem.cpp
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
#include "uavitem.h"
#include <math.h>

namespace mapcontrol
{

    double UAVItem::groundspeed_mps_filt = 0;

    UAVItem::UAVItem(MapGraphicItem* map,OPMapWidget* parent,QString uavPic):map(map),mapwidget(parent),showtrail(true),showtrailline(true),trailtime(5),traildistance(50),autosetreached(true)
      ,autosetdistance(100),altitude(0),showUAVInfo(false)
    {
        pic.load(uavPic);
        this->setFlag(QGraphicsItem::ItemIsMovable,false);
        this->setFlag(QGraphicsItem::ItemIsSelectable,false);
        localposition=map->FromLatLngToLocal(mapwidget->CurrentPosition());
        this->setPos(localposition.X(),localposition.Y());
        this->setZValue(4);
        trail=new QGraphicsItemGroup(this);
        trail->setParentItem(map);
        trailLine=new QGraphicsItemGroup(this);
        trailLine->setParentItem(map);
        this->setFlag(QGraphicsItem::ItemIgnoresTransformations,true);
        setCacheMode(QGraphicsItem::ItemCoordinateCache);
        mapfollowtype=UAVMapFollowType::None;
        trailtype=UAVTrailType::ByDistance;
        timer.start();
        generateArrowhead();
        double pixels2meters = map->Projection()->GetGroundResolution(map->ZoomTotal(),coord.Lat());
        meters2pixels=1.0 / pixels2meters;
        setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        connect(map,SIGNAL(childRefreshPosition()),this,SLOT(RefreshPos()));
        connect(map,SIGNAL(childSetOpacity(qreal)),this,SLOT(setOpacitySlot(qreal)));
        connect(map,SIGNAL(zoomChanged(double,double,double)),this,SLOT(zoomChangedSlot()));
    }
    UAVItem::~UAVItem()
    {

    }

    void UAVItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        //Draw plane
        painter->drawPixmap(-pic.width()/2,-pic.height()/2,pic);

        //Return if UAV Info context menu is turned off
        if(!showUAVInfo){
            return;
        }

        QPen myPen;

        //Turn on anti-aliasing so the fonts don't look terrible
        painter->setRenderHint(QPainter::Antialiasing, true);

        //Set pen attributes
        QColor myColor(Qt::red);
        myPen.setWidth(3);
        myPen.setColor(myColor);
        painter->setPen(myPen);
        painter->drawPolygon(arrowHead);
        painter->setPen(myPen);
        painter->drawLine(arrowShaft);

        //Set trend arc's color
        myPen.setColor(Qt::magenta);
        painter->setPen(myPen);

        if (trendSpanAngle > 0){
            QRectF rect(0, -trendRadius, trendRadius*2, trendRadius*2);
            painter->drawArc(rect, 180*16, -trendSpanAngle*16);
        }
        else{
            QRectF rect(-2*trendRadius, -trendRadius, trendRadius*2, trendRadius*2);
            painter->drawArc(rect, 0*16, -trendSpanAngle*16);
        }

        //*********** Create time rings
        if(groundspeed_mps_filt > 0){ //Don't clutter the display with rings that are only one pixel wide
            myPen.setWidth(2);

            myPen.setColor(QColor(0, 0, 0, 100));
            painter->setPen(myPen);
            painter->drawEllipse(QPointF(0,0),precalcRings,precalcRings);

            myPen.setColor(QColor(0, 0, 0, 110));
            painter->setPen(myPen);
            painter->drawEllipse(QPointF(0,0),precalcRings*2,precalcRings*2);

            myPen.setColor(QColor(0, 0, 0, 120));
            painter->setPen(myPen);
            painter->drawEllipse(QPointF(0,0),precalcRings*4,precalcRings*4);
        }
        //Rotate the text back to vertical
        qreal rot=this->rotation();
        painter->rotate(-1*rot);

        myPen.setWidth(1);
        myPen.setColor(Qt::white);
        painter->setBrush(Qt::white);
        painter->setPen(myPen);
        painter->drawPath(textPath);
    }

    void UAVItem::updateTextOverlay()
    {
        QPainterPath temp;
        if(!showUAVInfo)
        {
            temp.swap(textPath);
            return;
        }

        QFont borderfont( "Arial", 14, QFont::Normal, false );

        //Top left corner of text
        int textAnchorX = 20;
        int textAnchorY = 20;

        QString uavoInfoStrLine1, uavoInfoStrLine2;
        QString uavoInfoStrLine3, uavoInfoStrLine4;
        QString uavoInfoStrLine5;

        uavoInfoStrLine1.append(QString("CAS: %1 kph").arg(CAS_mps));
        uavoInfoStrLine2.append(QString("Groundspeed: %1 kph").arg(groundspeed_kph, 0, 'f',1));
        uavoInfoStrLine3.append(QString("Lat-Lon: %1, %2").arg(coord.Lat(), 0, 'f',7).arg(coord.Lng(), 0, 'f',7));
        uavoInfoStrLine4.append(QString("North-East: %1 m, %2 m").arg(NED[0], 0, 'f',1).arg(NED[1], 0, 'f',1));
        uavoInfoStrLine5.append(QString("Altitude: %1 m").arg(-NED[2], 0, 'f',1));
        temp.addText(textAnchorX, textAnchorY+16*0, borderfont, uavoInfoStrLine1);
        temp.addText(textAnchorX, textAnchorY+16*1, borderfont, uavoInfoStrLine2);
        temp.addText(textAnchorX, textAnchorY+16*2, borderfont, uavoInfoStrLine3);
        temp.addText(textAnchorX, textAnchorY+16*3, borderfont, uavoInfoStrLine4);
        temp.addText(textAnchorX, textAnchorY+16*4, borderfont, uavoInfoStrLine5);

        //Add text for time rings.
        if(groundspeed_mps > 0){
            //Always add the left side...
            temp.addText(-(groundspeed_mps_filt*ringTime*1*meters2pixels+10), 0, borderfont, QString("%1 s").arg(ringTime,0,'f',0));
            temp.addText(-(groundspeed_mps_filt*ringTime*2*meters2pixels+10), 0, borderfont, QString("%1 s").arg(ringTime*2,0,'f',0));
            temp.addText(-(groundspeed_mps_filt*ringTime*4*meters2pixels+10), 0, borderfont, QString("%1 s").arg(ringTime*4,0,'f',0));
            //... and add the right side, only if it doesn't interfere with the uav info text
            if(groundspeed_mps_filt*ringTime*4*meters2pixels > 200){
                if(groundspeed_mps_filt*ringTime*2*meters2pixels > 200){
                    if(groundspeed_mps_filt*ringTime*1*meters2pixels > 200){
                        temp.addText(groundspeed_mps_filt*ringTime*1*meters2pixels-8, 0, borderfont, QString("%1 s").arg(ringTime,0,'f',0));
                    }
                    temp.addText(groundspeed_mps_filt*ringTime*2*meters2pixels-8, 0, borderfont, QString("%1 s").arg(ringTime*2,0,'f',0));
                }
                temp.addText(groundspeed_mps_filt*ringTime*4*meters2pixels-8, 0, borderfont, QString("%1 s").arg(ringTime*4,0,'f',0));
            }
        }
        temp.swap(textPath);
    }

    QRectF UAVItem::boundingRect()const
    {
        if(showUAVInfo){
            if (boundingRectSize < 220){
                //In case the bounding rectangle isn't big enough to get the whole of the UAV Info graphic
                return QRectF(-boundingRectSize,-80,boundingRectSize+220,180);
            }
            else{
                return QRectF(-boundingRectSize,-boundingRectSize,2*boundingRectSize,2*boundingRectSize);
            }
        }
        else{
            return QRectF(-pic.width()/2,-pic.height()/2,pic.width(),pic.height());
        }
    }

    void UAVItem::SetNED(double NED[3]){
        this->NED[0] = NED[0];
        this->NED[1] = NED[1];
        this->NED[2] = NED[2];
    }

    void UAVItem::SetYawRate(double yawRate_dps){
        this->yawRate_dps=yawRate_dps;

        if (fabs(this->yawRate_dps) < 5e-1){ //This number is really the smallest we can go. Any smaller, and it might have problems if we forecast a shorter distance into the future
            this->yawRate_dps=5e-1;
        }

        //*********** Create trend arc
        trendSpanAngle = this->yawRate_dps * 5; //Forecast 5 seconds into the future

        //Calculate radius in [m], and then convert to pixels in local frame (not the same frame as is displayed on the map widget)
        trendRadius=fabs(groundspeed_mps/(this->yawRate_dps*M_PI/180))*meters2pixels;
    }

    void UAVItem::SetCAS(double CAS_mps){
        this->CAS_mps=CAS_mps;
    }

    void UAVItem::SetGroundspeed(double vNED[3], int m_maxUpdateRate_ms){
        this->vNED[0] = vNED[0];
        this->vNED[1] = vNED[1];
        this->vNED[2] = vNED[2];
        groundspeed_kph=sqrt(vNED[0]*vNED[0] + vNED[1]*vNED[1] + vNED[2]*vNED[2])*3.6;
        groundspeed_mps=groundspeed_kph/3.6;
        //On the first pass, set the filtered speed to the reported speed.
        static bool firstGroundspeed=true;
        if (firstGroundspeed){
            groundspeed_mps_filt=groundspeed_kph/3.6;
            firstGroundspeed=false;
        }
        else{
            int riseTime_ms=1000;
            double alpha= m_maxUpdateRate_ms/(double)(m_maxUpdateRate_ms+riseTime_ms);
            groundspeed_mps_filt= alpha*groundspeed_mps_filt + (1-alpha)*(groundspeed_kph/3.6);
         }
        ringTime=10*pow(2,17-map->ZoomTotal()); //Basic ring is 10 seconds wide at zoom level 17
        precalcRings=groundspeed_mps_filt*ringTime*meters2pixels;
        boundingRectSize=groundspeed_mps_filt*ringTime*4*meters2pixels+20;
        prepareGeometryChange();
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
            if(mapfollowtype==UAVMapFollowType::CenterAndRotateMap||mapfollowtype==UAVMapFollowType::CenterMap)
            {
                mapwidget->SetCurrentPosition(coord);
            }
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
        emit setChildPosition();
        emit setChildLine();
        updateTextOverlay();
    }

    void UAVItem::setOpacitySlot(qreal opacity)
    {
        this->setOpacity(opacity);
    }

    void UAVItem::zoomChangedSlot()
    {
        double pixels2meters = map->Projection()->GetGroundResolution(map->ZoomTotal(),coord.Lat());
        meters2pixels=1.0 / pixels2meters;
        boundingRectSize=groundspeed_mps_filt*ringTime*4*meters2pixels+20;
        prepareGeometryChange();
        updateTextOverlay();
        update();
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

    void UAVItem::SetShowUAVInfo(bool const& value)
    {
        showUAVInfo=value;
        showJustChanged=true;
        update();
    }

    void UAVItem::generateArrowhead(){
        qreal arrowSize = 10;

        //Create line from (0,0), to (1,1). Later, we'll scale and rotate it
        arrowShaft=QLineF(0,0,1.0,1.0);

        //Set the starting point to (0,0)
        arrowShaft.setP1(QPointF(0,0));

        //Set angle and length
        arrowShaft.setLength(60.0);
        arrowShaft.setAngle(90.0);

        //Form arrowhead
        double angle = ::acos(arrowShaft.dx() / arrowShaft.length());
        if (arrowShaft.dy() <= 0)
            angle = (M_PI * 2) - angle;

        QPointF arrowP1 = arrowShaft.pointAt(1) + QPointF(sin(angle + M_PI / 3) * arrowSize,
                                                      cos(angle + M_PI / 3) * arrowSize);
        QPointF arrowP2 = arrowShaft.pointAt(1) + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                                      cos(angle + M_PI - M_PI / 3) * arrowSize);

        //Assemble arrowhead
        arrowHead.clear();
        arrowHead << arrowShaft.pointAt(1) << arrowP1 << arrowP2;

    }
}
