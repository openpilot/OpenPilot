/**
 ******************************************************************************
 *
 * @file       geofencemodelmapproxy.h
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
#ifndef GEOFENCEMODELMAPPROXY_H
#define GEOFENCEMODELMAPPROXY_H

#include <QObject>
#include "opmapcontrol/opmapcontrol.h"
#include "geofencedatamodel.h"

using namespace mapcontrol;

class GeofenceModelMapProxy : public QObject
{
    typedef enum {OVERLAY_LINE,OVERLAY_CIRCLE_RIGHT,OVERLAY_CIRCLE_LEFT} overlayType;
    Q_OBJECT
public:
    explicit GeofenceModelMapProxy(QObject *parent, OPMapWidget * map, GeofenceDataModel * model, QItemSelectionModel * selectionModel);

    WayPointItem *findVertexNumber(int number);
    void createVertexPoint(internals::PointLatLng coord);
    void deleteVertexPoint(int number);
    void deleteAll();
    
signals:
    void requestGeofenceEditDialog();

public slots:
    void beginGeofencePolygon();
    void endGeofencePolygon(QMouseEvent *event);
    
private slots:
    void dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight );
    void rowsInserted ( const QModelIndex & parent, int first, int last );
    void rowsRemoved ( const QModelIndex & parent, int first, int last );
    void vertexValuesChanged(WayPointItem *wp);
    void currentRowChanged(QModelIndex,QModelIndex);
    void selectedVertexChanged(QList<WayPointItem*>);
    
private:
    overlayType overlayTranslate(int type);
    void createOverlay(WayPointItem * from,WayPointItem * to,overlayType type,QColor color);
    void refreshOverlays();

    static const int DEFAULT_UPPER_ALTITUDE;
    static const int DEFAULT_LOWER_ALTITUDE;

    OPMapWidget * myMap;
    GeofenceDataModel * myModel;
    QItemSelectionModel * selection;
    int currentPolygonId;
};

#endif // GEOFENCEMODELMAPPROXY_H
