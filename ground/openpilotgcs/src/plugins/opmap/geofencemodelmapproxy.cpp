/**
 ******************************************************************************
 *
 * @file       geofencemodelmapproxy.cpp
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

#include "geofencemodelmapproxy.h"

const int GeofenceModelMapProxy::DEFAULT_LOWER_ALTITUDE = 100;
const int GeofenceModelMapProxy::DEFAULT_UPPER_ALTITUDE = 400;


GeofenceModelMapProxy::GeofenceModelMapProxy(QObject *parent, OPMapWidget *map, GeofenceDataModel *model, QItemSelectionModel *selectionModel) :
    QObject(parent),
    myMap(map),
    myModel(model),
    selection(selectionModel),
    currentPolygonId(-1)

{
    connect(myModel,SIGNAL(rowsInserted(const QModelIndex&,int,int)),this,SLOT(rowsInserted(const QModelIndex&,int,int)));
    connect(myModel,SIGNAL(rowsRemoved(const QModelIndex&,int,int)),this,SLOT(rowsRemoved(const QModelIndex&,int,int)));
    connect(selection,SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(currentRowChanged(QModelIndex,QModelIndex)));
    connect(myModel,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(dataChanged(QModelIndex,QModelIndex)));
    connect(myMap,SIGNAL(SelectedVertexChanged(QList<WayPointItem*>)),this,SLOT(selectedVertexChanged(QList<WayPointItem*>)));
    connect(myMap,SIGNAL(VertexValuesChanged(WayPointItem*)),this,SLOT(vertexValuesChanged(WayPointItem*)));
}

WayPointItem *GeofenceModelMapProxy::findVertexNumber(int number)
{
    if(number<0)
        return NULL;
    return myMap->WPFind(number);
}

void GeofenceModelMapProxy::createVertexPoint(internals::PointLatLng coord)
{
    myModel->insertRow(myModel->rowCount(),QModelIndex());
    QModelIndex index=myModel->index(myModel->rowCount()-1,GeofenceDataModel::LATITUDE);
    myModel->setData(index,coord.Lat(),Qt::EditRole);
    index=myModel->index(myModel->rowCount()-1,GeofenceDataModel::LONGITUDE);
    myModel->setData(index,coord.Lng(),Qt::EditRole);
    index=myModel->index(myModel->rowCount()-1,GeofenceDataModel::POLYGON_ID);
    myModel->setData(index,currentPolygonId,Qt::EditRole);
}

void GeofenceModelMapProxy::deleteVertexPoint(int number)
{
    myModel->removeRow(number,QModelIndex());
}

void GeofenceModelMapProxy::deleteAll()
{
    myModel->removeRows(0,myModel->rowCount(),QModelIndex());
}

void GeofenceModelMapProxy::beginGeofencePolygon()
{
    myMap->setGeofencePolyMode(true);
    currentPolygonId++;
}

void GeofenceModelMapProxy::endGeofencePolygon(QMouseEvent* event)
{
    Q_UNUSED(event);

    // Complete polygon, copy to create a top and bottom for
    // extruded polyhedron, convert to triangle mesh and open
    // vertex editor.
    int curPairId = 0;
    int startingRowCount = myModel->rowCount();
    for(int count = 0; count < startingRowCount; ++count){
        // Get latitude and longitude for current point, this will be used for the
        // new point.
        internals::PointLatLng point;
        QModelIndex index = myModel->index(count, GeofenceDataModel::LATITUDE);
        point.SetLat(index.data().toDouble());
        index = myModel->index(count, GeofenceDataModel::LONGITUDE);
        point.SetLng(index.data().toDouble());

        // Set the point's altitude
        index = myModel->index(count, GeofenceDataModel::ALTITUDE);
        myModel->setData(index, DEFAULT_UPPER_ALTITUDE);

        // Set the point's pair id and polygon id
        index = myModel->index(count, GeofenceDataModel::VERTEX_PAIR_ID);
        myModel->setData(index, curPairId);
        index = myModel->index(count, GeofenceDataModel::POLYGON_ID);
        myModel->setData(index, currentPolygonId);

        // Add a new vertex
        createVertexPoint(point);

        // Set the new point's altitude, polygon id and pair id
        index = myModel->index(myModel->rowCount() - 1, GeofenceDataModel::ALTITUDE);
        myModel->setData(index, DEFAULT_LOWER_ALTITUDE);
        index = myModel->index(myModel->rowCount() - 1, GeofenceDataModel::VERTEX_PAIR_ID);
        myModel->setData(index, curPairId);
        index = myModel->index(myModel->rowCount() - 1, GeofenceDataModel::POLYGON_ID);
        myModel->setData(index, currentPolygonId);

        ++curPairId;
    }

    // Display the editor dialog to allow desired altitudes to be set
    emit requestGeofenceEditDialog();
}

void GeofenceModelMapProxy::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(bottomRight);

    WayPointItem * item=findVertexNumber(topLeft.row());
    if(!item)
        return;
    internals::PointLatLng latlng;
    int x=topLeft.row();
    double altitude;
    QModelIndex index;
    switch(topLeft.column())
    {
    case GeofenceDataModel::LATITUDE:
        latlng=item->Coord();
        index=myModel->index(x,GeofenceDataModel::LATITUDE);
        latlng.SetLat(index.data(Qt::DisplayRole).toDouble());
        item->SetCoord(latlng);
        break;
    case GeofenceDataModel::LONGITUDE:
        latlng=item->Coord();
        index=myModel->index(x,GeofenceDataModel::LONGITUDE);
        latlng.SetLng(index.data(Qt::DisplayRole).toDouble());
        item->SetCoord(latlng);
        break;
    case GeofenceDataModel::ALTITUDE:
        index=myModel->index(x,GeofenceDataModel::ALTITUDE);
        altitude=index.data(Qt::DisplayRole).toDouble();
        item->SetAltitude(altitude);
        break;
    }
}

void GeofenceModelMapProxy::rowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    for(int x=first;x<last+1;x++)
    {
        QModelIndex index;
        WayPointItem * item;
        internals::PointLatLng latlng;
        double altitude;
        index=myModel->index(x,GeofenceDataModel::LATITUDE);
        latlng.SetLat(index.data(Qt::DisplayRole).toDouble());
        index=myModel->index(x,GeofenceDataModel::LONGITUDE);
        latlng.SetLng(index.data(Qt::DisplayRole).toDouble());
        index=myModel->index(x,GeofenceDataModel::ALTITUDE);
        altitude=index.data(Qt::DisplayRole).toDouble();
        item=myMap->VertexInsert(latlng, altitude, x);
    }
    refreshOverlays();
}

void GeofenceModelMapProxy::rowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);

    for(int x=last;x>first-1;x--)
    {
        myMap->WPDelete(x);
    }
    refreshOverlays();
}

void GeofenceModelMapProxy::vertexValuesChanged(WayPointItem *wp)
{
    QModelIndex index;
    index=myModel->index(wp->Number(),GeofenceDataModel::LATITUDE);
    myModel->setData(index,wp->Coord().Lat(),Qt::EditRole);
    index=myModel->index(wp->Number(),GeofenceDataModel::LONGITUDE);
    myModel->setData(index,wp->Coord().Lng(),Qt::EditRole);

    index=myModel->index(wp->Number(),GeofenceDataModel::ALTITUDE);
    myModel->setData(index,wp->Altitude(),Qt::EditRole);
}

void GeofenceModelMapProxy::currentRowChanged(QModelIndex current, QModelIndex previous)
{
    Q_UNUSED(previous);

    QList<WayPointItem*> list;
    WayPointItem * wp=findVertexNumber(current.row());
    if(!wp)
        return;
    list.append(wp);
    myMap->setSelectedVertex(list);
}

void GeofenceModelMapProxy::selectedVertexChanged(QList<WayPointItem *> list)
{
    selection->clearSelection();
    foreach(WayPointItem * wp,list)
    {
        QModelIndex index=myModel->index(wp->Number(),0);
        selection->setCurrentIndex(index,QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

GeofenceModelMapProxy::overlayType GeofenceModelMapProxy::overlayTranslate(int type)
{
}

void GeofenceModelMapProxy::createOverlay(WayPointItem *from, WayPointItem *to, GeofenceModelMapProxy::overlayType type, QColor color)
{
}

void GeofenceModelMapProxy::refreshOverlays()
{
}
