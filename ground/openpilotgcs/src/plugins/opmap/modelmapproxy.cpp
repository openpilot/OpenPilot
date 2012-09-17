/**
 ******************************************************************************
 *
 * @file       modelmapproxy.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin OpenPilot Map Plugin
 * @{
 * @brief The OpenPilot Map plugin
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

#include "modelmapproxy.h"

modelMapProxy::modelMapProxy(QObject *parent,OPMapWidget *map,flightDataModel * model,QItemSelectionModel * selectionModel):QObject(parent),myMap(map),model(model),selection(selectionModel)
{
    connect(model,SIGNAL(rowsInserted(const QModelIndex&,int,int)),this,SLOT(rowsInserted(const QModelIndex&,int,int)));
    connect(model,SIGNAL(rowsRemoved(const QModelIndex&,int,int)),this,SLOT(rowsRemoved(const QModelIndex&,int,int)));
    connect(selection,SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(currentRowChanged(QModelIndex,QModelIndex)));
    connect(model,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(dataChanged(QModelIndex,QModelIndex)));
    connect(myMap,SIGNAL(selectedWPChanged(QList<WayPointItem*>)),this,SLOT(selectedWPChanged(QList<WayPointItem*>)));
    connect(myMap,SIGNAL(WPValuesChanged(WayPointItem*)),this,SLOT(WPValuesChanged(WayPointItem*)));
}

void modelMapProxy::WPValuesChanged(WayPointItem * wp)
{
    QModelIndex index;
    index=model->index(wp->Number(),flightDataModel::LATPOSITION);
    if(!index.isValid())
        return;
    model->setData(index,wp->Coord().Lat(),Qt::EditRole);
    index=model->index(wp->Number(),flightDataModel::LNGPOSITION);
    model->setData(index,wp->Coord().Lng(),Qt::EditRole);

    index=model->index(wp->Number(),flightDataModel::ALTITUDE);
    model->setData(index,wp->Altitude(),Qt::EditRole);

    index=model->index(wp->Number(),flightDataModel::DISRELATIVE);
    model->setData(index,wp->getRelativeCoord().distance,Qt::EditRole);
    index=model->index(wp->Number(),flightDataModel::BEARELATIVE);
    model->setData(index,wp->getRelativeCoord().bearingToDegrees(),Qt::EditRole);
    index=model->index(wp->Number(),flightDataModel::ALTITUDERELATIVE);
    model->setData(index,wp->getRelativeCoord().altitudeRelative,Qt::EditRole);
}

void modelMapProxy::currentRowChanged(QModelIndex current, QModelIndex previous)
{
    Q_UNUSED(previous);

    QList<WayPointItem*> list;
    WayPointItem * wp=findWayPointNumber(current.row());
    if(!wp)
        return;
    list.append(wp);
    myMap->setSelectedWP(list);
}

void modelMapProxy::selectedWPChanged(QList<WayPointItem *> list)
{
    selection->clearSelection();
    foreach(WayPointItem * wp,list)
    {
        QModelIndex index=model->index(wp->Number(),0);
        selection->setCurrentIndex(index,QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

modelMapProxy::overlayType modelMapProxy::overlayTranslate(int type)
{
    switch(type)
    {
    case MapDataDelegate::MODE_FLYENDPOINT:
    case MapDataDelegate::MODE_FLYVECTOR:
    case MapDataDelegate::MODE_DRIVEENDPOINT:
    case MapDataDelegate::MODE_DRIVEVECTOR:
       return OVERLAY_LINE;
        break;
    case MapDataDelegate::MODE_FLYCIRCLERIGHT:
    case MapDataDelegate::MODE_DRIVECIRCLERIGHT:
        return OVERLAY_CIRCLE_RIGHT;
         break;
    case MapDataDelegate::MODE_FLYCIRCLELEFT:
    case MapDataDelegate::MODE_DRIVECIRCLELEFT:
        return OVERLAY_CIRCLE_LEFT;
        break;
    default:
        break;
    }
}

void modelMapProxy::createOverlay(WayPointItem *from, WayPointItem *to, modelMapProxy::overlayType type,QColor color)
{
    if(from==NULL || to==NULL || from==to)
        return;
    switch(type)
    {
    case OVERLAY_LINE:
        myMap->WPLineCreate(from,to,color);
        break;
    case OVERLAY_CIRCLE_RIGHT:
        myMap->WPCircleCreate(to,from,true,color);
        break;
    case OVERLAY_CIRCLE_LEFT:
        myMap->WPCircleCreate(to,from,false,color);
        break;
    default:
        break;

    }
}
void modelMapProxy::createOverlay(WayPointItem *from, HomeItem *to, modelMapProxy::overlayType type,QColor color)
{
    if(from==NULL || to==NULL)
        return;
    switch(type)
    {
    case OVERLAY_LINE:
        myMap->WPLineCreate(to,from,color);
        break;
    case OVERLAY_CIRCLE_RIGHT:
        myMap->WPCircleCreate(to,from,true,color);
        break;
    case OVERLAY_CIRCLE_LEFT:
        myMap->WPCircleCreate(to,from,false,color);
        break;
    default:
        break;

    }
}
void modelMapProxy::refreshOverlays()
{
    myMap->deleteAllOverlays();
    if(model->rowCount()<1)
        return;
    WayPointItem * wp_current=NULL;
    WayPointItem * wp_next=NULL;
    int wp_jump;
    int wp_error;
    overlayType wp_next_overlay;
    overlayType wp_jump_overlay;
    overlayType wp_error_overlay;
    wp_current=findWayPointNumber(0);
    overlayType wp_current_overlay=overlayTranslate(model->data(model->index(0,flightDataModel::MODE)).toInt());
    createOverlay(wp_current,myMap->Home,wp_current_overlay,Qt::green);
    for(int x=0;x<model->rowCount();++x)
    {
        wp_current=findWayPointNumber(x);
        wp_jump=model->data(model->index(x,flightDataModel::JUMPDESTINATION)).toInt()-1;
        wp_error=model->data(model->index(x,flightDataModel::ERRORDESTINATION)).toInt()-1;
        wp_next_overlay=overlayTranslate(model->data(model->index(x+1,flightDataModel::MODE)).toInt());
        wp_jump_overlay=overlayTranslate(model->data(model->index(wp_jump,flightDataModel::MODE)).toInt());
        wp_error_overlay=overlayTranslate(model->data(model->index(wp_error,flightDataModel::MODE)).toInt());
        createOverlay(wp_current,findWayPointNumber(wp_error),wp_error_overlay,Qt::red);
        switch(model->data(model->index(x,flightDataModel::COMMAND)).toInt())
        {
        case MapDataDelegate::COMMAND_ONCONDITIONNEXTWAYPOINT:
            wp_next=findWayPointNumber(x+1);
            createOverlay(wp_current,wp_next,wp_next_overlay,Qt::green);
            break;
        case MapDataDelegate::COMMAND_ONCONDITIONJUMPWAYPOINT:
            wp_next=findWayPointNumber(wp_jump);
            createOverlay(wp_current,wp_next,wp_jump_overlay,Qt::green);
            break;
        case MapDataDelegate::COMMAND_ONNOTCONDITIONJUMPWAYPOINT:
            wp_next=findWayPointNumber(wp_jump);
            createOverlay(wp_current,wp_next,wp_jump_overlay,Qt::yellow);
            break;
        case MapDataDelegate::COMMAND_ONNOTCONDITIONNEXTWAYPOINT:
            wp_next=findWayPointNumber(x+1);
            createOverlay(wp_current,wp_next,wp_next_overlay,Qt::yellow);
            break;
        case MapDataDelegate::COMMAND_IFCONDITIONJUMPWAYPOINTELSENEXTWAYPOINT:
            wp_next=findWayPointNumber(wp_jump);
            createOverlay(wp_current,wp_next,wp_jump_overlay,Qt::green);
            wp_next=findWayPointNumber(x+1);
            createOverlay(wp_current,wp_next,wp_next_overlay,Qt::green);
            break;
        }
    }
}

WayPointItem * modelMapProxy::findWayPointNumber(int number)
{
    if(number<0)
        return NULL;
    return myMap->WPFind(number);
}

void modelMapProxy::rowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);

    for(int x=last;x>first-1;x--)
    {
        myMap->WPDelete(x);
    }
    refreshOverlays();
}

void modelMapProxy::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(bottomRight);

    WayPointItem * item=findWayPointNumber(topLeft.row());
    if(!item)
        return;
    internals::PointLatLng latlng;
    int x=topLeft.row();
    distBearingAltitude distBearing;
    double altitude;
    bool relative;
    QModelIndex index;
    QString desc;
    switch(topLeft.column())
    {
    case flightDataModel::COMMAND:
    case flightDataModel::CONDITION:
    case flightDataModel::JUMPDESTINATION:
    case flightDataModel::ERRORDESTINATION:
    case flightDataModel::MODE:
        refreshOverlays();
    break;
    case flightDataModel::WPDESCRITPTION:
        index=model->index(x,flightDataModel::WPDESCRITPTION);
        desc=index.data(Qt::DisplayRole).toString();
        item->SetDescription(desc);
        break;
    case flightDataModel::LATPOSITION:
        latlng=item->Coord();
        index=model->index(x,flightDataModel::LATPOSITION);
        latlng.SetLat(index.data(Qt::DisplayRole).toDouble());
        item->SetCoord(latlng);
        break;
    case flightDataModel::LNGPOSITION:
        latlng=item->Coord();
        index=model->index(x,flightDataModel::LNGPOSITION);
        latlng.SetLng(index.data(Qt::DisplayRole).toDouble());
        item->SetCoord(latlng);
        break;
    case flightDataModel::BEARELATIVE:
        distBearing=item->getRelativeCoord();
        index=model->index(x,flightDataModel::BEARELATIVE);
        distBearing.setBearingFromDegrees(index.data(Qt::DisplayRole).toDouble());
        item->setRelativeCoord(distBearing);
        break;
    case flightDataModel::DISRELATIVE:
        distBearing=item->getRelativeCoord();
        index=model->index(x,flightDataModel::DISRELATIVE);
        distBearing.distance=index.data(Qt::DisplayRole).toDouble();
        item->setRelativeCoord(distBearing);
        break;
    case flightDataModel::ALTITUDERELATIVE:
        distBearing=item->getRelativeCoord();
        index=model->index(x,flightDataModel::ALTITUDERELATIVE);
        distBearing.altitudeRelative=index.data(Qt::DisplayRole).toFloat();
        item->setRelativeCoord(distBearing);
        break;
    case flightDataModel::ISRELATIVE:
        index=model->index(x,flightDataModel::ISRELATIVE);
        relative=index.data(Qt::DisplayRole).toBool();
        if(relative)
            item->setWPType(mapcontrol::WayPointItem::relative);
        else
            item->setWPType(mapcontrol::WayPointItem::absolute);
        break;
    case flightDataModel::ALTITUDE:
        index=model->index(x,flightDataModel::ALTITUDE);
        altitude=index.data(Qt::DisplayRole).toDouble();
        item->SetAltitude(altitude);
        break;
    case flightDataModel::LOCKED:
        index=model->index(x,flightDataModel::LOCKED);
        item->setFlag(QGraphicsItem::ItemIsMovable,!index.data(Qt::DisplayRole).toBool());
        break;
    }
}

void modelMapProxy::rowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    for(int x=first;x<last+1;x++)
    {
        QModelIndex index;
        WayPointItem * item;
        internals::PointLatLng latlng;
        distBearingAltitude distBearing;
        double altitude;
        bool relative;
        index=model->index(x,flightDataModel::WPDESCRITPTION);
        QString desc=index.data(Qt::DisplayRole).toString();
        index=model->index(x,flightDataModel::LATPOSITION);
        latlng.SetLat(index.data(Qt::DisplayRole).toDouble());
        index=model->index(x,flightDataModel::LNGPOSITION);
        latlng.SetLng(index.data(Qt::DisplayRole).toDouble());
        index=model->index(x,flightDataModel::DISRELATIVE);
        distBearing.distance=index.data(Qt::DisplayRole).toDouble();
        index=model->index(x,flightDataModel::BEARELATIVE);
        distBearing.setBearingFromDegrees(index.data(Qt::DisplayRole).toDouble());
        index=model->index(x,flightDataModel::ALTITUDERELATIVE);
        distBearing.altitudeRelative=index.data(Qt::DisplayRole).toFloat();
        index=model->index(x,flightDataModel::ISRELATIVE);
        relative=index.data(Qt::DisplayRole).toBool();
        index=model->index(x,flightDataModel::ALTITUDE);
        altitude=index.data(Qt::DisplayRole).toDouble();
        if(relative)
            item=myMap->WPInsert(distBearing,desc,x);
        else
            item=myMap->WPInsert(latlng,altitude,desc,x);
    }
    refreshOverlays();
}
void modelMapProxy::deleteWayPoint(int number)
{
    model->removeRow(number,QModelIndex());
}

void modelMapProxy::createWayPoint(internals::PointLatLng coord)
{
    model->insertRow(model->rowCount(),QModelIndex());
    QModelIndex index=model->index(model->rowCount()-1,flightDataModel::LATPOSITION,QModelIndex());
    model->setData(index,coord.Lat(),Qt::EditRole);
    index=model->index(model->rowCount()-1,flightDataModel::LNGPOSITION,QModelIndex());
    model->setData(index,coord.Lng(),Qt::EditRole);
    index=model->index(model->rowCount()-1,flightDataModel::JUMPDESTINATION,QModelIndex());
    model->setData(index,1,Qt::EditRole);
    index=model->index(model->rowCount()-1,flightDataModel::ERRORDESTINATION,QModelIndex());
    model->setData(index,1,Qt::EditRole);
}
void modelMapProxy::deleteAll()
{
    model->removeRows(0,model->rowCount(),QModelIndex());
}
