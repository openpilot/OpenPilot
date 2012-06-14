/**
 ******************************************************************************
 *
 * @file       pathplanmanager.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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
    connect(model,SIGNAL(rowsInserted(const QModelIndex&,int,int)),this,SLOT(on_rowsInserted(const QModelIndex&,int,int)));
    connect(model,SIGNAL(rowsRemoved(const QModelIndex&,int,int)),this,SLOT(on_rowsRemoved(const QModelIndex&,int,int)));
    connect(selection,SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(on_currentRowChanged(QModelIndex,QModelIndex)));
    connect(model,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(on_dataChanged(QModelIndex,QModelIndex)));
    connect(myMap,SIGNAL(selectedWPChanged(QList<WayPointItem*>)),this,SLOT(on_selectedWPChanged(QList<WayPointItem*>)));
    connect(myMap,SIGNAL(WPValuesChanged(WayPointItem*)),this,SLOT(on_WPValuesChanged(WayPointItem*)));
}

void modelMapProxy::on_WPDeleted(int wp_numberint,WayPointItem * wp)
{
}

void modelMapProxy::on_WPInserted(int wp_number, WayPointItem * wp)
{

}

void modelMapProxy::on_WPValuesChanged(WayPointItem * wp)
{
    QModelIndex index;
    index=model->index(wp->Number(),flightDataModel::LATPOSITION);
    if(!index.isValid())
        return;
    model->setData(index,wp->Coord().Lat(),Qt::EditRole);
    index=model->index(wp->Number(),flightDataModel::LNGPOSITION);
    model->setData(index,wp->Coord().Lng(),Qt::EditRole);
    index=model->index(wp->Number(),flightDataModel::DISRELATIVE);
    model->setData(index,wp->getRelativeCoord().distance,Qt::EditRole);
    index=model->index(wp->Number(),flightDataModel::BEARELATIVE);
    model->setData(index,wp->getRelativeCoord().bearingToDegrees(),Qt::EditRole);
}

void modelMapProxy::on_currentRowChanged(QModelIndex current, QModelIndex previous)
{
    QList<WayPointItem*> list;
    WayPointItem * wp=findWayPointNumber(current.row());
    if(!wp)
        return;
    list.append(wp);
    myMap->setSelectedWP(list);
}

void modelMapProxy::on_selectedWPChanged(QList<WayPointItem *> list)
{
    selection->clearSelection();
    foreach(WayPointItem * wp,list)
    {
        QModelIndex index=model->index(wp->Number(),0);
        selection->setCurrentIndex(index,QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}
/*
typedef enum { MODE_FLYENDPOINT=0, MODE_FLYVECTOR=1, MODE_FLYCIRCLERIGHT=2, MODE_FLYCIRCLELEFT=3,
               MODE_DRIVEENDPOINT=4, MODE_DRIVEVECTOR=5, MODE_DRIVECIRCLELEFT=6, MODE_DRIVECIRCLERIGHT=7,
               MODE_FIXEDATTITUDE=8, MODE_SETACCESSORY=9, MODE_DISARMALARM=10 } ModeOptions;
typedef enum { ENDCONDITION_NONE=0, ENDCONDITION_TIMEOUT=1, ENDCONDITION_DISTANCETOTARGET=2,
               ENDCONDITION_LEGREMAINING=3, ENDCONDITION_ABOVEALTITUDE=4, ENDCONDITION_POINTINGTOWARDSNEXT=5,
               ENDCONDITION_PYTHONSCRIPT=6, ENDCONDITION_IMMEDIATE=7 } EndConditionOptions;
typedef enum { COMMAND_ONCONDITIONNEXTWAYPOINT=0, COMMAND_ONNOTCONDITIONNEXTWAYPOINT=1,
               COMMAND_ONCONDITIONJUMPWAYPOINT=2, COMMAND_ONNOTCONDITIONJUMPWAYPOINT=3,
               COMMAND_IFCONDITIONJUMPWAYPOINTELSENEXTWAYPOINT=4 } CommandOptions;
*/
void modelMapProxy::refreshOverlays()
{
    /*
    QMutexLocker locker(&wplistmutex);
    myMap->deleteAllOverlays();
    foreach(WayPointItem * wp,*waypoints)
    {
        customData data=wp->data(0).value<customData>();

        switch(data.condition)
        {

        }

        switch(data.mode)
        {
        case PathAction::MODE_FLYENDPOINT:
        case PathAction::MODE_FLYVECTOR:
        case PathAction::MODE_DRIVEENDPOINT:
        case PathAction::MODE_DRIVEVECTOR:
            if(wp->Number()==0)
                myMap->WPLineCreate((HomeItem*)myMap->Home,wp);
            else
                myMap->WPLineCreate(findWayPointNumber(wp->Number()-1),wp);
            break;
        case PathAction::MODE_FLYCIRCLERIGHT:
        case PathAction::MODE_DRIVECIRCLERIGHT:
            if(wp->Number()==0)
                myMap->WPCircleCreate((HomeItem*)myMap->Home,wp,true);
            myMap->WPCircleCreate(findWayPointNumber(wp->Number()-1),wp,true);
            break;
        case PathAction::MODE_FLYCIRCLELEFT:
        case PathAction::MODE_DRIVECIRCLELEFT:
            if(wp->Number()==0)
                myMap->WPCircleCreate((HomeItem*)myMap->Home,wp,false);
            myMap->WPCircleCreate(findWayPointNumber(wp->Number()-1),wp,false);
            break;
        default:
            break;

        }
    }
    */
}

WayPointItem * modelMapProxy::findWayPointNumber(int number)
{
    if(number<0)
        return NULL;
    return myMap->WPFind(number);
}
/*
WPDESCRITPTION,LATPOSITION,LNGPOSITION,DISRELATIVE,BEARELATIVE,ISRELATIVE,ALTITUDE,
    VELOCITY,MODE,MODE_PARAMS0,MODE_PARAMS1,MODE_PARAMS2,MODE_PARAMS3,
    CONDITION,CONDITION_PARAMS0,CONDITION_PARAMS1,CONDITION_PARAMS2,CONDITION_PARAMS3,
    COMMAND,JUMPDESTINATION,ERRORDESTINATION
*/


void modelMapProxy::on_rowsRemoved(const QModelIndex &parent, int first, int last)
{
    qDebug()<<"modelMapProxy::on_rowsRemoved"<<"first"<<first<<"last"<<last;

    for(int x=last;x>first-1;x--)
    {
        myMap->WPDelete(x);
    }
}

void modelMapProxy::on_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    WayPointItem * item=findWayPointNumber(topLeft.row());
    if(!item)
        return;
    internals::PointLatLng latlng;
    int x=topLeft.row();
    distBearing distBearing;
    double altitude;
    bool relative;
    QModelIndex index;
    QString desc;
    switch(topLeft.column())
    {
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
        break;
    case flightDataModel::DISRELATIVE:
        distBearing=item->getRelativeCoord();
        index=model->index(x,flightDataModel::DISRELATIVE);
        distBearing.distance=index.data(Qt::DisplayRole).toDouble();
        break;

    case flightDataModel::ISRELATIVE:
        index=model->index(x,flightDataModel::ISRELATIVE);
        relative=index.data(Qt::DisplayRole).toBool();
        if(relative)
            item->setWPType(mapcontrol::WayPointItem::relative);
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

void modelMapProxy::on_rowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    for(int x=first;x<last+1;x++)
    {
        QModelIndex index;
        WayPointItem * item;
        internals::PointLatLng latlng;
        distBearing distBearing;
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
        index=model->index(x,flightDataModel::ISRELATIVE);
        relative=index.data(Qt::DisplayRole).toBool();
        index=model->index(x,flightDataModel::ALTITUDE);
        altitude=index.data(Qt::DisplayRole).toDouble();
        item=myMap->WPInsert(latlng,altitude,desc,x);
        item->setRelativeCoord(distBearing);
        if(relative)
            item->setWPType(mapcontrol::WayPointItem::relative);
    }
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
}
void modelMapProxy::deleteAll()
{
    model->removeRows(0,model->rowCount(),QModelIndex());
}
