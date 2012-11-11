/**
 ******************************************************************************
 * @file       waypointtable.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup Waypoint Editor GCS Plugins
 * @{
 * @addtogroup WaypointEditorGadgetPlugin Waypoint Editor Gadget Plugin
 * @{
 * @brief Table model for listing waypoint
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

#include <QDebug>
#include <QBrush>
#include "waypointtable.h"
#include "extensionsystem/pluginmanager.h"

WaypointTable::WaypointTable(QObject *parent) :
    QAbstractTableModel(parent)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm != NULL);
    objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager != NULL);
    waypointObj = Waypoint::GetInstance(objManager);
    Q_ASSERT(waypointObj != NULL);
    waypointActiveObj = WaypointActive::GetInstance(objManager);
    Q_ASSERT(waypointActiveObj != NULL);

    elements = 0;

    // Unfortunately there is no per object new instance signal yet
    connect(objManager, SIGNAL(newInstance(UAVObject*)),
            this, SLOT(doNewInstance(UAVObject*)));
    connect(waypointActiveObj, SIGNAL(objectUpdated(UAVObject*)),
            this, SLOT(waypointsUpdated(UAVObject*)));

    int numWaypoints = objManager->getNumInstances(Waypoint::OBJID);
    for (int i = 0; i < numWaypoints; i++) {
        Waypoint *waypoint = Waypoint::GetInstance(objManager, i);
        Q_ASSERT(waypoint);
        if(waypoint)
            connect(waypoint, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(waypointsUpdated(UAVObject*)));
    }

    headers.clear();
    headers.append(QString("North"));
    headers.append(QString("East"));
    headers.append(QString("Down"));
}

int WaypointTable::rowCount(const QModelIndex & /*parent*/) const
{
    return elements;
}

int WaypointTable::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return headers.length();
}

QVariant WaypointTable::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole) {
        Waypoint *obj = Waypoint::GetInstance(objManager, index.row());
        Q_ASSERT(obj);
        Waypoint::DataFields waypoint = obj->getData();

        switch(index.column()) {
        case 0:
            return waypoint.Position[Waypoint::POSITION_NORTH];
        case 1:
            return waypoint.Position[Waypoint::POSITION_EAST];
        case 2:
            return waypoint.Position[Waypoint::POSITION_DOWN];
        default:
            Q_ASSERT(0);
            return 0;
        }
    } else if (role == Qt::BackgroundRole) {
        WaypointActive::DataFields waypointActive = waypointActiveObj->getData();

        if(index.row() == waypointActive.Index) {
            return QBrush(Qt::lightGray);
        } else
            return QVariant::Invalid;
    }
    else {
        return QVariant::Invalid;
    }

}

QVariant WaypointTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if(section < headers.length())
            return headers[section];
        return QVariant::Invalid;
    } else
        return QAbstractTableModel::headerData(section, orientation, role);
}

/**
  * Called for any new UAVO instance and when that is a Waypoint register
  * to update the table
  */
void WaypointTable::doNewInstance(UAVObject*obj)
{
    Q_ASSERT(obj);
    if (!obj)
        return;

    if (obj->getObjID() == Waypoint::OBJID)
        connect(obj, SIGNAL(objectUpdated(UAVObject*)),this,SLOT(waypointsUpdated(UAVObject*)));
}

/**
  * Called whenever the waypoints are updated to inform
  * the view
  */
void WaypointTable::waypointsUpdated(UAVObject *)
{
     int elementsNow = objManager->getNumInstances(waypointObj->getObjID());

    // Currently only support adding instances which is all the UAVO manager
    // supports
    if (elementsNow > elements) {
        beginInsertRows(QModelIndex(), elements, elementsNow-1);
        elements = elementsNow;
        endInsertRows();
    }

    QModelIndex i1 = index(0,0);
    QModelIndex i2 = index(elements-1, columnCount(QModelIndex()));
    emit dataChanged(i1, i2);
}

Qt::ItemFlags WaypointTable::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool WaypointTable::setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole )
{
    Q_UNUSED(role);

    double val = value.toDouble();
    qDebug() << "New value " << val << " for column " << index.column();

    Waypoint *obj = Waypoint::GetInstance(objManager, index.row());
    Q_ASSERT(obj);
    Waypoint::DataFields waypoint = obj->getData();

    switch(index.column()) {
    case 0:
        waypoint.Position[Waypoint::POSITION_NORTH] = val;
        break;
    case 1:
        waypoint.Position[Waypoint::POSITION_EAST] = val;
        break;
    case 2:
        waypoint.Position[Waypoint::POSITION_DOWN] = val;
        break;
    default:
        return false;
    }

    obj->setData(waypoint);
    obj->updated();
    qDebug() << "Set data for instance " << obj->getInstID();

    return true;
}
