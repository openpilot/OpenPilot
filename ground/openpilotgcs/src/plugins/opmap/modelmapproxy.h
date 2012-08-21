/**
 ******************************************************************************
 *
 * @file       modelmapproxy.h
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
#ifndef MODELMAPPROXY_H
#define MODELMAPPROXY_H
#include <QWidget>
#include "opmapcontrol/opmapcontrol.h"
#include "pathaction.h"
#include "waypoint.h"
#include "QMutexLocker"
#include "QPointer"
#include "flightdatamodel.h"
#include <QItemSelectionModel>
#include <widgetdelegates.h>


using namespace mapcontrol;
class modelMapProxy:public QObject
{
    typedef enum {OVERLAY_LINE,OVERLAY_CIRCLE_RIGHT,OVERLAY_CIRCLE_LEFT} overlayType;
    Q_OBJECT
public:
    explicit modelMapProxy(QObject *parent,OPMapWidget * map,flightDataModel * model,QItemSelectionModel * selectionModel);
    WayPointItem *findWayPointNumber(int number);
    void createWayPoint(internals::PointLatLng coord);
    void deleteWayPoint(int number);
    void deleteAll();
private slots:
    void dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight );
    void rowsInserted ( const QModelIndex & parent, int first, int last );
    void rowsRemoved ( const QModelIndex & parent, int first, int last );
    void WPValuesChanged(WayPointItem *wp);
    void currentRowChanged(QModelIndex,QModelIndex);
    void selectedWPChanged(QList<WayPointItem*>);
private:
    overlayType overlayTranslate(int type);
    void createOverlay(WayPointItem * from,WayPointItem * to,overlayType type,QColor color);
    void createOverlay(WayPointItem *from, HomeItem *to, modelMapProxy::overlayType type, QColor color);
    OPMapWidget * myMap;
    flightDataModel * model;
    void refreshOverlays();
    QItemSelectionModel * selection;
};

#endif // MODELMAPPROXY_H
