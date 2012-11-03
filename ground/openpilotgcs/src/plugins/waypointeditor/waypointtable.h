/**
 ******************************************************************************
 * @file       waypointtable.h
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

#ifndef WAYPOINTTABLE_H
#define WAYPOINTTABLE_H

#include <QAbstractTableModel>
#include <QList>
#include <QString>
#include <waypoint.h>
#include <waypointactive.h>

class WaypointTable : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit WaypointTable(QObject *parent = 0);

    // Get dimensionality of the data
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    // Access data
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    // Functions to make the data editable
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData ( const QModelIndex & index, const QVariant & value, int role);
signals:

protected slots:
    void waypointsUpdated(UAVObject *);
    void doNewInstance(UAVObject*);
public slots:

private:
    UAVObjectManager *objManager;
    Waypoint *waypointObj;
    WaypointActive *waypointActiveObj;
    QList <QString> headers;
    int elements;
};

#endif // WAYPOINTTABLE_H
