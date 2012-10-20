/**
 ******************************************************************************
 *
 * @file       opmapgadgetwidget.h
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

#ifndef OPMAP_GADGETWIDGET_H_
#define OPMAP_GADGETWIDGET_H_

// ******************************************************


#include "flightdatamodel.h"
#include "pathplanner.h"
#include "modelmapproxy.h"
#include "modeluavoproxy.h"

#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QStringList>
#include <QStandardItemModel>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QPointF>

#include "opmapcontrol/opmapcontrol.h"

#include "opmap_zoom_slider_widget.h"
#include "opmap_statusbar_widget.h"

#include "utils/coordinateconversions.h"

#include "extensionsystem/pluginmanager.h"
#include "uavobjectutilmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "objectpersistence.h"
#include <QItemSelectionModel>
#include "opmap_edit_waypoint_dialog.h"

#include "homeeditor.h"

// ******************************************************

namespace Ui
{
    class OPMap_Widget;
}

using namespace mapcontrol;

// ******************************************************

typedef struct t_home
{
    internals::PointLatLng coord;
    double altitude;
    bool locked;
} t_home;

// ******************************************************

enum opMapModeType { Normal_MapMode = 0,
                     MagicWaypoint_MapMode = 1};

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
    void setHome(QPointF pos);
    void setHome(internals::PointLatLng pos_lat_lon, double altitude);
    void goHome();
    void setZoom(int zoom);
    void setPosition(QPointF pos);
    void setMapProvider(QString provider);
    void setUseOpenGL(bool useOpenGL);
    void setShowTileGridLines(bool showTileGridLines);
    void setAccessMode(QString accessMode);
    void setUseMemoryCache(bool useMemoryCache);
    void setCacheLocation(QString cacheLocation);
    void setMapMode(opMapModeType mode);
	void SetUavPic(QString UAVPic);
    void setMaxUpdateRate(int update_rate);
    void setHomePosition(QPointF pos);
    void setOverlayOpacity(qreal value);
    bool getGPSPosition(double &latitude, double &longitude, double &altitude);
signals:
    void defaultLocationAndZoomChanged(double lng,double lat,double zoom);
    void overlayOpacityChanged(qreal);

public slots:
    void homePositionUpdated(UAVObject *);
    void onTelemetryConnect();
    void onTelemetryDisconnect();

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void closeEvent(QCloseEvent *);
private slots:
    void wpDoubleClickEvent(WayPointItem *wp);
    void updatePosition();

    void updateMousePos();

    void zoomIn();
    void zoomOut();

    /**
    * @brief signals received from the various map plug-in widget user controls
    *
    * Some are currently disabled for the v1.0 plugin version.
    */
    void on_toolButtonZoomM_clicked();
    void on_toolButtonZoomP_clicked();
    void on_toolButtonMapHome_clicked();
    void on_toolButtonMapUAV_clicked();
    void on_toolButtonMapUAVheading_clicked();
    void on_horizontalSliderZoom_sliderMoved(int position);
    void on_toolButtonMagicWaypointMapMode_clicked();
    void on_toolButtonNormalMapMode_clicked();
    void on_toolButtonHomeWaypoint_clicked();
    void on_toolButtonMoveToWP_clicked();

    /**
    * @brief signals received from the map object
    */
    void zoomChanged(double zoomt,double zoom, double zoomd);
    void OnCurrentPositionChanged(internals::PointLatLng point);
    void OnTileLoadComplete();
    void OnTileLoadStart();
    void OnTilesStillToLoad(int number);

    /**
    * @brief mouse right click context menu signals
    */
    void onReloadAct_triggered();
    void onRipAct_triggered();
    void onCopyMouseLatLonToClipAct_triggered();
    void onCopyMouseLatToClipAct_triggered();
    void onCopyMouseLonToClipAct_triggered();
    void onShowCompassAct_toggled(bool show);
    void onShowDiagnostics_toggled(bool show);
    void onShowUAVInfo_toggled(bool show);
    void onShowUAVAct_toggled(bool show);
    void onShowHomeAct_toggled(bool show);
    void onShowTrailLineAct_toggled(bool show);
    void onShowTrailAct_toggled(bool show);
    void onGoZoomInAct_triggered();
    void onGoZoomOutAct_triggered();
    void onGoMouseClickAct_triggered();
    void onSetHomeAct_triggered();
    void onGoHomeAct_triggered();
    void onGoUAVAct_triggered();
    void onFollowUAVpositionAct_toggled(bool checked);
    void onFollowUAVheadingAct_toggled(bool checked);

    void onOpenWayPointEditorAct_triggered();
    void onAddWayPointAct_triggeredFromContextMenu();
    void onAddWayPointAct_triggeredFromThis();
    void onAddWayPointAct_triggered(internals::PointLatLng coord);
    void onEditWayPointAct_triggered();
    void onLockWayPointAct_triggered();
    void onDeleteWayPointAct_triggered();
    void onClearWayPointsAct_triggered();

    void onMapModeActGroup_triggered(QAction *action);
    void onZoomActGroup_triggered(QAction *action);
    void onHomeMagicWaypointAct_triggered();
    void onShowSafeAreaAct_toggled(bool show);
    void onSafeAreaActGroup_triggered(QAction *action);
    void onUAVTrailTypeActGroup_triggered(QAction *action);
    void onClearUAVtrailAct_triggered();
    void onUAVTrailTimeActGroup_triggered(QAction *action);
    void onUAVTrailDistanceActGroup_triggered(QAction *action);
	void onMaxUpdateRateActGroup_triggered(QAction *action);
    void onChangeDefaultLocalAndZoom();
    void on_tbFind_clicked();
    void onHomeDoubleClick(HomeItem*);
    void onOverlayOpacityActGroup_triggered(QAction *action);
    void on_leFind_returnPressed();

private:
	int m_min_zoom;
	int m_max_zoom;
    double m_heading;	// uav heading
	internals::PointLatLng m_mouse_lat_lon;
	internals::PointLatLng m_context_menu_lat_lon;
	int m_prev_tile_number;
    opMapModeType m_map_mode;
	int m_maxUpdateRate;
	t_home m_home_position;
	QStringList findPlaceWordList;
    QCompleter *findPlaceCompleter;
    QTimer *m_updateTimer;
    QTimer *m_statusUpdateTimer;
    Ui::OPMap_Widget *m_widget;
    mapcontrol::OPMapWidget *m_map;
	ExtensionSystem::PluginManager *pm;
	UAVObjectManager *obm;
	UAVObjectUtilManager *obum;
    QPointer<opmap_edit_waypoint_dialog> waypoint_edit_dialog;
    QStandardItemModel wayPoint_treeView_model;
    mapcontrol::WayPointItem *m_mouse_waypoint;
    QPointer<modelUavoProxy> UAVProxy;
    QMutex m_map_mutex;
	bool m_telemetry_connected;
    QAction *closeAct1;
    QAction *closeAct2;
    QAction *reloadAct;
    QAction *ripAct;
	QAction *copyMouseLatLonToClipAct;
    QAction *copyMouseLatToClipAct;
    QAction *copyMouseLonToClipAct;
    QAction *showCompassAct;
    QAction *showDiagnostics;
    QAction *showUAVInfo;
    QAction *showHomeAct;
    QAction *showUAVAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *goMouseClickAct;
    QAction *setHomeAct;
    QAction *goHomeAct;
    QAction *goUAVAct;
    QAction *followUAVpositionAct;
    QAction *followUAVheadingAct;

    QAction *wayPointEditorAct;
    QAction *addWayPointActFromThis;
    QAction *addWayPointActFromContextMenu;
    QAction *editWayPointAct;
    QAction *lockWayPointAct;
    QAction *deleteWayPointAct;
    QAction *clearWayPointsAct;

    QAction *homeMagicWaypointAct;

    QAction *showSafeAreaAct;
    QAction *changeDefaultLocalAndZoom;
    QActionGroup *safeAreaActGroup;
    QList<QAction *> safeAreaAct;

    QActionGroup *uavTrailTypeActGroup;
    QList<QAction *> uavTrailTypeAct;
    QAction *clearUAVtrailAct;
    QActionGroup *uavTrailTimeActGroup;
    QAction *showTrailLineAct;
    QAction *showTrailAct;
    QList<QAction *> uavTrailTimeAct;
    QActionGroup *uavTrailDistanceActGroup;
    QList<QAction *> uavTrailDistanceAct;

    QActionGroup *mapModeActGroup;
    QList<QAction *> mapModeAct;

    QActionGroup *zoomActGroup;
    QActionGroup *overlayOpacityActGroup;
    QList<QAction *> zoomAct;
    QList<QAction *> overlayOpacityAct;

	QActionGroup *maxUpdateRateActGroup;
	QList<QAction *> maxUpdateRateAct;

    void createActions();
	void homeMagicWaypoint();
    void moveToMagicWaypointPosition();
    void hideMagicWaypointControls();
    void showMagicWaypointControls();
    void keepMagicWaypointWithInSafeArea();

    double distance(internals::PointLatLng from, internals::PointLatLng to);
    double bearing(internals::PointLatLng from, internals::PointLatLng to);
    internals::PointLatLng destPoint(internals::PointLatLng source, double bear, double dist);

	bool getUAVPosition(double &latitude, double &longitude, double &altitude);
    double getUAV_Yaw();

    void setMapFollowingMode();

	bool setHomeLocationObject();
    QMenu contextMenu;
    internals::PointLatLng lastLatLngMouse;
    WayPointItem * magicWayPoint;

    QPointer<flightDataModel> model;
    QPointer<pathPlanner> table;
    QPointer<modelMapProxy> mapProxy;
    QPointer<QItemSelectionModel> selectionModel;
};

#endif /* OPMAP_GADGETWIDGET_H_ */
