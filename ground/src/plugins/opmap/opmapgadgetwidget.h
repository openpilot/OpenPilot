/**
 ******************************************************************************
 *
 * @file       opmapgadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   opmap
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

#ifndef OPMAP_GADGETWIDGET_H_
#define OPMAP_GADGETWIDGET_H_

#include "opmapcontrol/opmapcontrol.h"

#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QStringList>
#include <QStandardItemModel>
#include <QList>
#include <QMutex>

#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/positionactual.h"

#include "opmap_mapoverlaywidget.h"
#include "opmap_waypointeditor_dialog.h"
#include "opmap_edit_waypoint_dialog.h"

#include "extensionsystem/pluginmanager.h"

namespace Ui
{
    class OPMap_Widget;
}

using namespace mapcontrol;

// ******************************************************

// local waypoint list item structure
typedef struct t_waypoint
{
    mapcontrol::WayPointItem *item;
    int time_seconds;
    int hold_time_seconds;
} t_waypoint;

// ******************************************************

class OPMapGadgetWidget : public QWidget
{
    Q_OBJECT

public:
    OPMapGadgetWidget(QWidget *parent = 0);
   ~OPMapGadgetWidget();

    /**
    * @brief public functions
    *
    * @param
    */
    void setZoom(int value);
    void setPosition(QPointF pos);
    void setMapProvider(QString provider);
    void setUseOpenGL(bool useOpenGL);
    void setShowTileGridLines(bool showTileGridLines);
    void setAccessMode(QString accessMode);
    void setUseMemoryCache(bool useMemoryCache);
    void setCacheLocation(QString cacheLocation);

public slots:

protected:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void keyPressEvent(QKeyEvent* event);

private slots:
    void updatePosition();

    void updateMousePos();

    void zoomIn();
    void zoomOut();

    /**
    * @brief signals received from the various map plug-in widget user controls
    *
    * @param
    */
    void on_comboBoxFindPlace_returnPressed();
    void on_toolButtonFindPlace_clicked();
    void on_toolButtonZoomM_clicked();
    void on_toolButtonZoomP_clicked();
    void on_toolButtonWaypointsTreeViewShowHide_clicked();
    void on_toolButtonFlightControlsShowHide_clicked();
    void on_toolButtonMapHome_clicked();
    void on_toolButtonMapUAV_clicked();
    void on_horizontalSliderZoom_sliderMoved(int position);
    void on_toolButtonAddWaypoint_clicked();
    void on_toolButtonWaypointEditor_clicked();
    void on_treeViewWaypoints_clicked(QModelIndex index);
    void on_toolButtonHome_clicked();
    void on_toolButtonNextWaypoint_clicked();
    void on_toolButtonPrevWaypoint_clicked();
    void on_toolButtonHoldPosition_clicked();
    void on_toolButtonGo_clicked();

    /**
    * @brief signals received from the map object
    *
    * @param
    */
    void zoomChanged(double zoom);
    void OnCurrentPositionChanged(internals::PointLatLng point);
    void OnTileLoadComplete();
    void OnTileLoadStart();
    void OnMapDrag();
    void OnMapZoomChanged();
    void OnMapTypeChanged(MapType::Types type);
    void OnEmptyTileError(int zoom, core::Point pos);
    void OnTilesStillToLoad(int number);
    void WPNumberChanged(int const& oldnumber,int const& newnumber, WayPointItem* waypoint);
    void WPValuesChanged(WayPointItem* waypoint);
    void WPInserted(int const& number, WayPointItem* waypoint);
    void WPDeleted(int const& number);

    /**
    * @brief mouse right click context menu signals
    *
    * @param
    */
    void on_reloadAct_triggered();
    void on_findPlaceAct_triggered();
    void on_showCompassAct_toggled(bool show_compass);
    void on_goZoomInAct_triggered();
    void on_goZoomOutAct_triggered();
    void on_goMouseClickAct_triggered();
    void on_goHomeAct_triggered();
    void on_goUAVAct_triggered();
    void on_followUAVpositionAct_toggled(bool checked);
    void on_followUAVheadingAct_toggled(bool checked);
    void on_openWayPointEditorAct_triggered();
    void on_addWayPointAct_triggered();
    void on_editWayPointAct_triggered();
    void on_lockWayPointAct_triggered();
    void on_deleteWayPointAct_triggered();
    void on_clearWayPointsAct_triggered();
    void on_zoomActGroup_triggered(QAction *action);

private:
    double m_heading;	// uav heading

    internals::PointLatLng mouse_lat_lon;

    int prev_tile_number;

    QStringList findPlaceWordList;
    QCompleter *findPlaceCompleter;

    QTimer *m_updateTimer;
    QTimer *m_statusUpdateTimer;

    ExtensionSystem::PluginManager *pm;
    UAVObjectManager *objManager;
    PositionActual *m_positionActual;

    Ui::OPMap_Widget *m_widget;

    mapcontrol::OPMapWidget *m_map;

    opmap_waypointeditor_dialog waypoint_editor_dialog;

    opmap_edit_waypoint_dialog waypoint_edit_dialog;

    QGraphicsScene *m_map_graphics_scene;
    QGraphicsProxyWidget *m_map_scene_proxy;
    OPMap_MapOverlayWidget *m_map_overlay_widget;

    QStandardItemModel *wayPoint_treeView_model;

    mapcontrol::WayPointItem *m_mouse_waypoint;

    QList<t_waypoint> m_waypoint_list;
    QMutex m_waypoint_list_mutex;

    void createActions();

    QAction *closeAct;
    QAction *reloadAct;
    QAction *findPlaceAct;
    QAction *showCompassAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *goMouseClickAct;
    QAction *goHomeAct;
    QAction *goUAVAct;
    QAction *followUAVpositionAct;
    QAction *followUAVheadingAct;
    QAction *wayPointEditorAct;
    QAction *addWayPointAct;
    QAction *editWayPointAct;
    QAction *lockWayPointAct;
    QAction *deleteWayPointAct;
    QAction *clearWayPointsAct;

    QActionGroup *zoomActGroup;
    QList<QAction *> zoomAct;

    void loadComboBoxLines(QComboBox *comboBox, QString filename);
    void saveComboBoxLines(QComboBox *comboBox, QString filename);
};

#endif /* OPMAP_GADGETWIDGET_H_ */
