/**
 ******************************************************************************
 *
 * @file       opmapgadgetwidget.cpp
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

#include "opmapgadgetwidget.h"
#include "ui_opmap_widget.h"

#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QClipboard>
#include <QtGui/QMenu>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QDateTime>

#include <math.h>

#include "utils/stylehelper.h"
#include "utils/homelocationutil.h"
#include "utils/worldmagmodel.h"

#include "uavtalk/telemetrymanager.h"
#include "uavobject.h"

#include "positionactual.h"
#include "homelocation.h"
#include "gpsposition.h"
#include "gyros.h"
#include "attitudeactual.h"
#include "positionactual.h"
#include "velocityactual.h"

#define allow_manual_home_location_move

// *************************************************************************************

#define deg_to_rad          ((double)M_PI / 180.0)
#define rad_to_deg          (180.0 / (double)M_PI)

#define earth_mean_radius   6371    // kilometers

#define	max_digital_zoom    3       // maximum allowed digital zoom level

const int safe_area_radius_list[] = {5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000};   // meters

const int uav_trail_time_list[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};                      // seconds

const int uav_trail_distance_list[] = {1, 2, 5, 10, 20, 50, 100, 200, 500};             // meters

const int max_update_rate_list[] = {100, 200, 500, 1000, 2000, 5000};                   // milliseconds

// *************************************************************************************


// *************************************************************************************
//  NOTE: go back to SVN REV 2137 and earlier to get back to experimental waypoint support.
// *************************************************************************************


// constructor
OPMapGadgetWidget::OPMapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    // **************

    m_widget = NULL;
    m_map = NULL;
    findPlaceCompleter = NULL;

    m_mouse_waypoint = NULL;

	pm = NULL;
	obm = NULL;
	obum = NULL;

	m_prev_tile_number = 0;

	m_min_zoom = m_max_zoom = 0;

    m_map_mode = Normal_MapMode;

    m_maxUpdateRate = max_update_rate_list[4];	// 2 seconds //SHOULDN'T THIS BE LOADED FROM THE USER PREFERENCES?

	m_telemetry_connected = false;

	m_context_menu_lat_lon = m_mouse_lat_lon = internals::PointLatLng(0, 0);

    setMouseTracking(true);

	pm = ExtensionSystem::PluginManager::instance();
	if (pm)
	{
		obm = pm->getObject<UAVObjectManager>();
		obum = pm->getObject<UAVObjectUtilManager>();
	}

	// **************
    // get current location

    double latitude = 0;
    double longitude = 0;
    double altitude = 0;

	// current position
	getUAVPosition(latitude, longitude, altitude);

    internals::PointLatLng pos_lat_lon = internals::PointLatLng(latitude, longitude);

    // **************
    // default home position

	m_home_position.coord = pos_lat_lon;
	m_home_position.altitude = altitude;
	m_home_position.locked = false;

    // **************
    // create the widget that holds the user controls and the map

    m_widget = new Ui::OPMap_Widget();
    m_widget->setupUi(this);

    // **************
    // create the central map widget

    m_map = new mapcontrol::OPMapWidget();	// create the map object

    m_map->setFrameStyle(QFrame::NoFrame);      // no border frame
    m_map->setBackgroundBrush(QBrush(Utils::StyleHelper::baseColor())); // tile background

    m_map->configuration->DragButton = Qt::LeftButton;  // use the left mouse button for map dragging

    m_widget->horizontalSliderZoom->setMinimum(m_map->MinZoom());			//
    m_widget->horizontalSliderZoom->setMaximum(m_map->MaxZoom() + max_digital_zoom);	//

	m_min_zoom = m_widget->horizontalSliderZoom->minimum();	// minimum zoom we can accept
	m_max_zoom = m_widget->horizontalSliderZoom->maximum();	// maximum zoom we can accept

    m_map->SetMouseWheelZoomType(internals::MouseWheelZoomType::MousePositionWithoutCenter);	    // set how the mouse wheel zoom functions
    m_map->SetFollowMouse(true);				    // we want a contiuous mouse position reading

    m_map->SetShowHome(true);					    // display the HOME position on the map
    m_map->SetShowUAV(true);					    // display the UAV position on the map

    m_map->Home->SetSafeArea(safe_area_radius_list[0]);                         // set radius (meters) //SHOULDN'T THE DEFAULT BE USER DEFINED?
    m_map->Home->SetShowSafeArea(true);                                         // show the safe area  //SHOULDN'T THE DEFAULT BE USER DEFINED?
    m_map->Home->SetToggleRefresh(true);

    if(m_map->Home)
        connect(m_map->Home,SIGNAL(homedoubleclick(HomeItem*)),this,SLOT(onHomeDoubleClick(HomeItem*)));
    m_map->UAV->SetTrailTime(uav_trail_time_list[0]);                           // seconds
    m_map->UAV->SetTrailDistance(uav_trail_distance_list[1]);                   // meters

    m_map->UAV->SetTrailType(UAVTrailType::ByTimeElapsed);
    if(m_map->GPS)
    {
        m_map->GPS->SetTrailTime(uav_trail_time_list[0]);                           // seconds
        m_map->GPS->SetTrailDistance(uav_trail_distance_list[1]);                   // meters

        m_map->GPS->SetTrailType(UAVTrailType::ByTimeElapsed);
    }
    // **************

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_map);
    m_widget->mapWidget->setLayout(layout);

            m_widget->toolButtonMagicWaypointMapMode->setChecked(false);
            m_widget->toolButtonNormalMapMode->setChecked(true);
            hideMagicWaypointControls();

    m_widget->labelUAVPos->setText("---");
    m_widget->labelMapPos->setText("---");
    m_widget->labelMousePos->setText("---");
    m_widget->labelMapZoom->setText("---");

    m_widget->progressBarMap->setMaximum(1);

    connect(m_map, SIGNAL(zoomChanged(double, double, double)), this, SLOT(zoomChanged(double, double, double)));					// map zoom change signals
    connect(m_map, SIGNAL(OnCurrentPositionChanged(internals::PointLatLng)), this, SLOT(OnCurrentPositionChanged(internals::PointLatLng)));    // map poisition change signals
    connect(m_map, SIGNAL(OnTileLoadComplete()), this, SLOT(OnTileLoadComplete()));					// tile loading stop signals
    connect(m_map, SIGNAL(OnTileLoadStart()), this, SLOT(OnTileLoadStart()));					// tile loading start signals
    connect(m_map, SIGNAL(OnTilesStillToLoad(int)), this, SLOT(OnTilesStillToLoad(int)));				// tile loading signals
    connect(m_map,SIGNAL(OnWayPointDoubleClicked(WayPointItem*)),this,SLOT(wpDoubleClickEvent(WayPointItem*)));
	m_map->SetCurrentPosition(m_home_position.coord);         // set the map position
	m_map->Home->SetCoord(m_home_position.coord);             // set the HOME position
	m_map->UAV->SetUAVPos(m_home_position.coord, 0.0);        // set the UAV position
    m_map->UAV->update();
    if(m_map->GPS)
        m_map->GPS->SetUAVPos(m_home_position.coord, 0.0);        // set the GPS position
#ifdef USE_PATHPLANNER
    model=new flightDataModel(this);
    table=new pathPlanner();
    selectionModel=new QItemSelectionModel(model);
    mapProxy=new modelMapProxy(this,m_map,model,selectionModel);
    table->setModel(model,selectionModel);
    waypoint_edit_dialog=new opmap_edit_waypoint_dialog(this,model,selectionModel);
    UAVProxy=new modelUavoProxy(this,model);
    connect(table,SIGNAL(sendPathPlanToUAV()),UAVProxy,SLOT(modelToObjects()));
    connect(table,SIGNAL(receivePathPlanFromUAV()),UAVProxy,SLOT(objectsToModel()));
#endif
    magicWayPoint=m_map->magicWPCreate();
    magicWayPoint->setVisible(false);

    m_map->setOverlayOpacity(0.5);

    // **************
    // create various context menu (mouse right click menu) actions

    createActions();

    // **************
    // connect to the UAVObject updates we require to become a bit aware of our environment:

    if (pm)
    {
        // Register for Home Location state changes
        if (obm)
        {
			UAVDataObject *obj = dynamic_cast<UAVDataObject *>(obm->getObject(QString("HomeLocation")));
            if (obj)
            {
				connect(obj, SIGNAL(objectUpdated(UAVObject *)), this , SLOT(homePositionUpdated(UAVObject *)));
            }
        }

        // Listen to telemetry connection events
        TelemetryManager *telMngr = pm->getObject<TelemetryManager>();
        if (telMngr)
        {
            connect(telMngr, SIGNAL(connected()), this, SLOT(onTelemetryConnect()));
            connect(telMngr, SIGNAL(disconnected()), this, SLOT(onTelemetryDisconnect()));
        }
    }

    // **************
    // create the desired timers

    m_updateTimer = new QTimer();
	m_updateTimer->setInterval(m_maxUpdateRate);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));
    m_updateTimer->start();

    m_statusUpdateTimer = new QTimer();
	m_statusUpdateTimer->setInterval(200);
	connect(m_statusUpdateTimer, SIGNAL(timeout()), this, SLOT(updateMousePos()));
    m_statusUpdateTimer->start();
    // **************

    m_map->setFocus();
}

// destructor
OPMapGadgetWidget::~OPMapGadgetWidget()
{
	if (m_map)
	{
		disconnect(m_map, 0, 0, 0);
		m_map->SetShowHome(false);	// doing this appears to stop the map lib crashing on exit
		m_map->SetShowUAV(false);	//   "          "
	}

	if (m_map)
	{
		delete m_map;
		m_map = NULL;
	}
    if(!model.isNull())
        delete model;
    if(!table.isNull())
        delete table;
    if(!selectionModel.isNull())
        delete selectionModel;
    if(!mapProxy.isNull())
        delete mapProxy;
    if(!waypoint_edit_dialog.isNull())
        delete waypoint_edit_dialog;
    if(!UAVProxy.isNull())
        delete UAVProxy;
}

// *************************************************************************************
// widget signals .. the mouseMoveEvent does not get called - don't yet know why

void OPMapGadgetWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void OPMapGadgetWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_widget && m_map)
    {
    }

    if (event->buttons() & Qt::LeftButton)
    {
    }
    QWidget::mouseMoveEvent(event);
}
void OPMapGadgetWidget::wpDoubleClickEvent(WayPointItem  *wp)
{
        m_mouse_waypoint = wp;
        onEditWayPointAct_triggered();
}

void OPMapGadgetWidget::contextMenuEvent(QContextMenuEvent *event)
{   // the user has right clicked on the map - create the pop-up context menu and display it

    QString s;

    if (!m_widget || !m_map)
        return;

    if (event->reason() != QContextMenuEvent::Mouse)
        return;	// not a mouse click event

    // current mouse position
    QPoint p = m_map->mapFromGlobal(event->globalPos());
    m_context_menu_lat_lon = m_map->GetFromLocalToLatLng(p);

    if (!m_map->contentsRect().contains(p))
        return;					    // the mouse click was not on the map

    // show the mouse position
    s = QString::number(m_context_menu_lat_lon.Lat(), 'f', 7) + "  " + QString::number(m_context_menu_lat_lon.Lng(), 'f', 7);
    m_widget->labelMousePos->setText(s);

    // find out if we have a waypoint under the mouse cursor
    QGraphicsItem *item = m_map->itemAt(p);
    m_mouse_waypoint = qgraphicsitem_cast<mapcontrol::WayPointItem *>(item);

    // find out if the waypoint is locked (or not)
    bool waypoint_locked = false;
    if (m_mouse_waypoint)
        waypoint_locked = (m_mouse_waypoint->flags() & QGraphicsItem::ItemIsMovable) == 0;

    // ****************
    // Dynamically create the popup menu

    contextMenu.addAction(closeAct1);
    contextMenu.addSeparator();
    contextMenu.addAction(reloadAct);
    contextMenu.addSeparator();
    contextMenu.addAction(ripAct);
    contextMenu.addSeparator();

    QMenu maxUpdateRateSubMenu(tr("&Max Update Rate ") + "(" + QString::number(m_maxUpdateRate) + " ms)", this);
    for (int i = 0; i < maxUpdateRateAct.count(); i++)
        maxUpdateRateSubMenu.addAction(maxUpdateRateAct.at(i));
    contextMenu.addMenu(&maxUpdateRateSubMenu);

    contextMenu.addSeparator();

    switch (m_map_mode)
    {
        case Normal_MapMode: s = tr(" (Normal)"); break;
        case MagicWaypoint_MapMode: s = tr(" (Magic Waypoint)"); break;
        default: s = tr(" (Unknown)"); break;
    }
    for (int i = 0; i < mapModeAct.count(); i++)
    {   // set the menu to checked (or not)
        QAction *act = mapModeAct.at(i);
        if (!act) continue;
        if (act->data().toInt() == (int)m_map_mode)
            act->setChecked(true);
    }
    QMenu mapModeSubMenu(tr("Map mode") + s, this);
    for (int i = 0; i < mapModeAct.count(); i++)
        mapModeSubMenu.addAction(mapModeAct.at(i));
    contextMenu.addMenu(&mapModeSubMenu);

    contextMenu.addSeparator();

    QMenu copySubMenu(tr("Copy"), this);
    copySubMenu.addAction(copyMouseLatLonToClipAct);
    copySubMenu.addAction(copyMouseLatToClipAct);
    copySubMenu.addAction(copyMouseLonToClipAct);
    contextMenu.addMenu(&copySubMenu);

    contextMenu.addSeparator();
    contextMenu.addAction(changeDefaultLocalAndZoom);
    contextMenu.addSeparator();

    QMenu safeArea("Safety Area definitions");
   // menu.addAction(showSafeAreaAct);
    QMenu safeAreaSubMenu(tr("Safe Area Radius") + " (" + QString::number(m_map->Home->SafeArea()) + "m)", this);
    for (int i = 0; i < safeAreaAct.count(); i++)
        safeAreaSubMenu.addAction(safeAreaAct.at(i));
    safeArea.addMenu(&safeAreaSubMenu);
    safeArea.addAction(showSafeAreaAct);
    contextMenu.addMenu(&safeArea);

    contextMenu.addSeparator();

    contextMenu.addAction(showCompassAct);

    contextMenu.addAction(showDiagnostics);

    contextMenu.addAction(showUAVInfo);

    contextMenu.addSeparator()->setText(tr("Zoom"));

    contextMenu.addAction(zoomInAct);
    contextMenu.addAction(zoomOutAct);

    QMenu zoomSubMenu(tr("&Zoom ") + "(" + QString::number(m_map->ZoomTotal()) + ")", this);
    for (int i = 0; i < zoomAct.count(); i++)
        zoomSubMenu.addAction(zoomAct.at(i));
    contextMenu.addMenu(&zoomSubMenu);

    contextMenu.addSeparator();

    contextMenu.addAction(goMouseClickAct);

    contextMenu.addSeparator()->setText(tr("HOME"));

    contextMenu.addAction(setHomeAct);
    contextMenu.addAction(showHomeAct);
    contextMenu.addAction(goHomeAct);

    // ****
    // uav trails
    QMenu uav_menu(tr("UAV"));
    uav_menu.addSeparator()->setText(tr("UAV Trail"));
    contextMenu.addMenu(&uav_menu);
    QMenu uavTrailTypeSubMenu(tr("UAV trail type") + " (" + mapcontrol::Helper::StrFromUAVTrailType(m_map->UAV->GetTrailType()) + ")", this);
    for (int i = 0; i < uavTrailTypeAct.count(); i++)
        uavTrailTypeSubMenu.addAction(uavTrailTypeAct.at(i));
    uav_menu.addMenu(&uavTrailTypeSubMenu);

    QMenu uavTrailTimeSubMenu(tr("UAV trail time") + " (" + QString::number(m_map->UAV->TrailTime()) + " sec)", this);
    for (int i = 0; i < uavTrailTimeAct.count(); i++)
        uavTrailTimeSubMenu.addAction(uavTrailTimeAct.at(i));
    uav_menu.addMenu(&uavTrailTimeSubMenu);

    QMenu uavTrailDistanceSubMenu(tr("UAV trail distance") + " (" + QString::number(m_map->UAV->TrailDistance()) + " meters)", this);
    for (int i = 0; i < uavTrailDistanceAct.count(); i++)
        uavTrailDistanceSubMenu.addAction(uavTrailDistanceAct.at(i));
    uav_menu.addMenu(&uavTrailDistanceSubMenu);

    uav_menu.addAction(showTrailAct);

    uav_menu.addAction(showTrailLineAct);

    uav_menu.addAction(clearUAVtrailAct);

    // ****

    uav_menu.addSeparator()->setText(tr("UAV"));

    uav_menu.addAction(showUAVAct);
    uav_menu.addAction(followUAVpositionAct);
    uav_menu.addAction(followUAVheadingAct);
    uav_menu.addAction(goUAVAct);

    // *********
#ifdef USE_PATHPLANNER
    switch (m_map_mode)
    {
        case Normal_MapMode:
        // only show the waypoint stuff if not in 'magic waypoint' mode

            contextMenu.addSeparator()->setText(tr("Waypoints"));

            contextMenu.addAction(wayPointEditorAct);
            contextMenu.addAction(addWayPointActFromContextMenu);

            if (m_mouse_waypoint)
            {	// we have a waypoint under the mouse
                contextMenu.addAction(editWayPointAct);

                lockWayPointAct->setChecked(waypoint_locked);
                contextMenu.addAction(lockWayPointAct);

                if (!waypoint_locked)
                    contextMenu.addAction(deleteWayPointAct);
            }

            if (m_map->WPPresent())
                contextMenu.addAction(clearWayPointsAct);	// we have waypoints

            break;

        case MagicWaypoint_MapMode:
            contextMenu.addSeparator()->setText(tr("Waypoints"));
            contextMenu.addAction(homeMagicWaypointAct);
            break;
    }
#endif
    // *********

    QMenu overlaySubMenu(tr("&Overlay Opacity "),this);
    for (int i = 0; i < overlayOpacityAct.count(); i++)
        overlaySubMenu.addAction(overlayOpacityAct.at(i));
    contextMenu.addMenu(&overlaySubMenu);
    contextMenu.addSeparator();

    contextMenu.addAction(closeAct2);

    contextMenu.exec(event->globalPos());  // popup the menu

    // ****************
}

void OPMapGadgetWidget::closeEvent(QCloseEvent *event)
{
    table->close();
    QWidget::closeEvent(event);
}

// *************************************************************************************
// timer signals

/**
  Updates the UAV position on the map. It is called at a user-defined frequency,
  as set inside the map widget.
*/
void OPMapGadgetWidget::updatePosition()
{
	double uav_latitude, uav_longitude, uav_altitude, uav_yaw;
	double gps_latitude, gps_longitude, gps_altitude, gps_heading;

	internals::PointLatLng uav_pos;
	internals::PointLatLng gps_pos;

	if (!m_widget || !m_map)
		return;

    QMutexLocker locker(&m_map_mutex);

	// *************
	// get the current UAV details

    // get current UAV position
	if (!getUAVPosition(uav_latitude, uav_longitude, uav_altitude))
        return;

    // get current UAV heading
	uav_yaw = getUAV_Yaw();

	uav_pos = internals::PointLatLng(uav_latitude, uav_longitude);

	// *************
    // get the current GPS position and heading
    GPSPosition *gpsPositionObj = GPSPosition::GetInstance(obm);
    Q_ASSERT(gpsPositionObj);

    GPSPosition::DataFields gpsPositionData = gpsPositionObj->getData();

    gps_heading = gpsPositionData.Heading;
    gps_latitude = gpsPositionData.Latitude;
    gps_longitude = gpsPositionData.Longitude;
    gps_altitude = gpsPositionData.Altitude;

	gps_pos = internals::PointLatLng(gps_latitude, gps_longitude);

    //**********************
    // get the current position and heading estimates
    AttitudeActual *attitudeActualObj = AttitudeActual::GetInstance(obm);
    PositionActual *positionActualObj = PositionActual::GetInstance(obm);
    VelocityActual *velocityActualObj = VelocityActual::GetInstance(obm);
    Gyros *gyrosObj = Gyros::GetInstance(obm);

    Q_ASSERT(attitudeActualObj);
    Q_ASSERT(positionActualObj);
    Q_ASSERT(velocityActualObj);
    Q_ASSERT(gyrosObj);

    AttitudeActual::DataFields attitudeActualData = attitudeActualObj->getData();
    PositionActual::DataFields positionActualData = positionActualObj->getData();
    VelocityActual::DataFields velocityActualData = velocityActualObj->getData();
    Gyros::DataFields gyrosData = gyrosObj->getData();

    double NED[3]={positionActualData.North, positionActualData.East, positionActualData.Down};
    double vNED[3]={velocityActualData.North, velocityActualData.East, velocityActualData.Down};

    //Set the position and heading estimates in the painter module
    m_map->UAV->SetNED(NED);
    m_map->UAV->SetCAS(-1); //THIS NEEDS TO BECOME AIRSPEED, ONCE WE SETTLE ON A UAVO
    m_map->UAV->SetGroundspeed(vNED, m_maxUpdateRate);

    //Convert angular velocities into a rotationg rate around the world-frame yaw axis. This is found by simply taking the dot product of the angular Euler-rate matrix with the angular rates.
    float psiRate_dps=0*gyrosData.z + sin(attitudeActualData.Roll*deg_to_rad)/cos(attitudeActualData.Pitch*deg_to_rad)*gyrosData.y + cos(attitudeActualData.Roll*deg_to_rad)/cos(attitudeActualData.Pitch*deg_to_rad)*gyrosData.z;

    //Set the angular rate in the painter module
    m_map->UAV->SetYawRate(psiRate_dps); //Not correct, but I'm being lazy right now.

    // *************
	// display the UAV position

    QString str =
            "lat: " + QString::number(uav_pos.Lat(), 'f', 7) +
            " lon: " + QString::number(uav_pos.Lng(), 'f', 7) +
			" " + QString::number(uav_yaw, 'f', 1) + "deg" +
			" " + QString::number(uav_altitude, 'f', 1) + "m";
//            " " + QString::number(uav_ground_speed_meters_per_second, 'f', 1) + "m/s";
    m_widget->labelUAVPos->setText(str);

	// *************
	// set the UAV icon position on the map

	m_map->UAV->SetUAVPos(uav_pos, uav_altitude);        // set the maps UAV position
//	qDebug()<<"UAVPOSITION"<<uav_pos.ToString();
	m_map->UAV->SetUAVHeading(uav_yaw);                  // set the maps UAV heading

	// *************
	// set the GPS icon position on the map
    if(m_map->GPS)
    {
        m_map->GPS->SetUAVPos(gps_pos, gps_altitude); // set the maps GPS position
        m_map->GPS->SetUAVHeading(gps_heading);       // set the maps GPS heading
    }
    m_map->UAV->updateTextOverlay();
    m_map->UAV->update();
	// *************
}

/**
  Update plugin behaviour based on mouse position; Called every few ms by a
  timer.
 */
void OPMapGadgetWidget::updateMousePos()
{
	if (!m_widget || !m_map)
		return;

    QMutexLocker locker(&m_map_mutex);

    QPoint p = m_map->mapFromGlobal(QCursor::pos());
    internals::PointLatLng lat_lon = m_map->GetFromLocalToLatLng(p);    // fetch the current lat/lon mouse position
    lastLatLngMouse=lat_lon;
    if (!m_map->contentsRect().contains(p))
        return;					    // the mouse is not on the map

//    internals::PointLatLng lat_lon = m_map->currentMousePosition();     // fetch the current lat/lon mouse position

    QGraphicsItem *item = m_map->itemAt(p);

    // find out if we are over the home position
    mapcontrol::HomeItem *home = qgraphicsitem_cast<mapcontrol::HomeItem *>(item);

    // find out if we have a waypoint under the mouse cursor
    mapcontrol::WayPointItem *wp = qgraphicsitem_cast<mapcontrol::WayPointItem *>(item);

	if (m_mouse_lat_lon == lat_lon)
        return;                 // the mouse has not moved

	m_mouse_lat_lon = lat_lon;    // yes it has!

    internals::PointLatLng home_lat_lon = m_map->Home->Coord();

	QString s = QString::number(m_mouse_lat_lon.Lat(), 'f', 7) + "  " + QString::number(m_mouse_lat_lon.Lng(), 'f', 7);
    if (wp)
    {
        s += "  wp[" + QString::number(wp->numberAdjusted()) + "]";

        double dist = distance(home_lat_lon, wp->Coord());
        double bear = bearing(home_lat_lon, wp->Coord());
        s += "  " + QString::number(dist * 1000, 'f', 1) + "m";
        s += "  " + QString::number(bear, 'f', 1) + "deg";
    }
    else
    if (home)
    {
        s += "  home";

		double dist = distance(home_lat_lon, m_mouse_lat_lon);
		double bear = bearing(home_lat_lon, m_mouse_lat_lon);
        s += "  " + QString::number(dist * 1000, 'f', 1) + "m";
        s += "  " + QString::number(bear, 'f', 1) + "deg";
    }
    m_widget->labelMousePos->setText(s);
}

// *************************************************************************************
// map signals


/**
  Update the Plugin UI to reflect a change in zoom level
  */
void OPMapGadgetWidget::zoomChanged(double zoomt, double zoom, double zoomd)
{
	if (!m_widget || !m_map)
		return;

    QString s = "tot:" + QString::number(zoomt, 'f', 1) + " rea:" + QString::number(zoom, 'f', 1) + " dig:" + QString::number(zoomd, 'f', 1);
    m_widget->labelMapZoom->setText(s);

    int i_zoom = (int)(zoomt + 0.5);

	if (i_zoom < m_min_zoom) i_zoom = m_min_zoom;
    else
	if (i_zoom > m_max_zoom) i_zoom = m_max_zoom;

    if (m_widget->horizontalSliderZoom->value() != i_zoom)
	m_widget->horizontalSliderZoom->setValue(i_zoom);	// set the GUI zoom slider position

	int index0_zoom = i_zoom - m_min_zoom;			// zoom level starting at index level '0'
    if (index0_zoom < zoomAct.count())
    zoomAct.at(index0_zoom)->setChecked(true);		// set the right-click context menu zoom level
}

void OPMapGadgetWidget::OnCurrentPositionChanged(internals::PointLatLng point)
{
	if (!m_widget || !m_map)
		return;

    QString coord_str = QString::number(point.Lat(), 'f', 7) + "   " + QString::number(point.Lng(), 'f', 7) + " ";
    m_widget->labelMapPos->setText(coord_str);
}

/**
  Update the progress bar while there are still tiles to load
  */
void OPMapGadgetWidget::OnTilesStillToLoad(int number)
{
	if (!m_widget || !m_map)
		return;

	if (m_widget->progressBarMap->maximum() < number)
	    m_widget->progressBarMap->setMaximum(number);

	m_widget->progressBarMap->setValue(m_widget->progressBarMap->maximum() - number);	// update the progress bar

	m_prev_tile_number = number;
}

/**
  Show the progress bar as soon as the map lib starts downloading
  */
void OPMapGadgetWidget::OnTileLoadStart()
{
	if (!m_widget || !m_map)
		return;
    m_widget->progressBarMap->setVisible(true);
}

/**
  Hide the progress bar once the map lib has finished downloading

  TODO: somehow this gets called before tile load is actually complete?
  */

void OPMapGadgetWidget::OnTileLoadComplete()
{
	if (!m_widget || !m_map)
		return;

    m_widget->progressBarMap->setVisible(false);
}

void OPMapGadgetWidget::on_toolButtonZoomP_clicked()
{
    QMutexLocker locker(&m_map_mutex);
    zoomIn();
}

void OPMapGadgetWidget::on_toolButtonZoomM_clicked()
{
    QMutexLocker locker(&m_map_mutex);
    zoomOut();
}

void OPMapGadgetWidget::on_toolButtonMapHome_clicked()
{
    QMutexLocker locker(&m_map_mutex);
    goHome();
}

void OPMapGadgetWidget::on_toolButtonMapUAV_clicked()
{
	if (!m_widget || !m_map)
		return;

    QMutexLocker locker(&m_map_mutex);

    followUAVpositionAct->toggle();
}

void OPMapGadgetWidget::on_toolButtonMapUAVheading_clicked()
{
	if (!m_widget || !m_map)
		return;

    followUAVheadingAct->toggle();
}

void OPMapGadgetWidget::on_horizontalSliderZoom_sliderMoved(int position)
{
	if (!m_widget || !m_map)
		return;

    QMutexLocker locker(&m_map_mutex);

    setZoom(position);
}


void OPMapGadgetWidget::on_toolButtonNormalMapMode_clicked()
{
    setMapMode(Normal_MapMode);
}

void OPMapGadgetWidget::on_toolButtonMagicWaypointMapMode_clicked()
{
    setMapMode(MagicWaypoint_MapMode);
}

void OPMapGadgetWidget::on_toolButtonHomeWaypoint_clicked()
{
    homeMagicWaypoint();
}

void OPMapGadgetWidget::on_toolButtonMoveToWP_clicked()
{
    moveToMagicWaypointPosition();
}

// *************************************************************************************
// public slots

void OPMapGadgetWidget::onTelemetryConnect()
{
	m_telemetry_connected = true;

	if (!obum) return;

	bool set;
	double LLA[3];

	// ***********************
	// fetch the home location

	if (obum->getHomeLocation(set, LLA) < 0)
		return;	// error

    setHome(internals::PointLatLng(LLA[0], LLA[1]),LLA[2]);

    if (m_map)
    {
        if(m_map->UAV->GetMapFollowType()!=UAVMapFollowType::None)
            m_map->SetCurrentPosition(m_home_position.coord);         // set the map position
    }
    // ***********************
}

void OPMapGadgetWidget::onTelemetryDisconnect()
{
	m_telemetry_connected = false;
}

// Updates the Home position icon whenever the HomePosition object is updated
void OPMapGadgetWidget::homePositionUpdated(UAVObject *hp)
{
    Q_UNUSED(hp);
    if (!obum) return;
    bool set;
    double LLA[3];
    if (obum->getHomeLocation(set, LLA) < 0)
        return;	// error
    setHome(internals::PointLatLng(LLA[0], LLA[1]),LLA[2]);

}

// *************************************************************************************
// public functions

/**
  Sets the home position on the map widget
  */
void OPMapGadgetWidget::setHome(QPointF pos)
{
	if (!m_widget || !m_map)
		return;

    double latitude = pos.x();
    double longitude = pos.y();

    if (latitude >  90) latitude =  90;
    else
    if (latitude < -90) latitude = -90;

    if (longitude != longitude) longitude = 0; // nan detection
    else
    if (longitude >  180) longitude =  180;
    else
    if (longitude < -180) longitude = -180;

    setHome(internals::PointLatLng(latitude, longitude),0);
}

/**
  Sets the home position on the map widget
  */
void OPMapGadgetWidget::setHome(internals::PointLatLng pos_lat_lon, double altitude)
{
	if (!m_widget || !m_map)
		return;

    if (pos_lat_lon.Lat() != pos_lat_lon.Lat() || pos_lat_lon.Lng() != pos_lat_lon.Lng())
        return;; // nan prevention

    double latitude = pos_lat_lon.Lat();
    double longitude = pos_lat_lon.Lng();

    if (latitude != latitude) latitude = 0; // nan detection
    else
    if (latitude >  90) latitude =  90;
    else
    if (latitude < -90) latitude = -90;

    if (longitude != longitude) longitude = 0; // nan detection
    else
    if (longitude >  180) longitude =  180;
    else
    if (longitude < -180) longitude = -180;

    // *********

    m_home_position.coord = internals::PointLatLng(latitude, longitude);
    m_home_position.altitude = altitude;

    m_map->Home->SetCoord(m_home_position.coord);
    m_map->Home->SetAltitude(altitude);
    m_map->Home->RefreshPos();

    // move the magic waypoint to keep it within the safe area boundry
    keepMagicWaypointWithInSafeArea();
}


/**
  Centers the map over the home position
  */
void OPMapGadgetWidget::goHome()
{
	if (!m_widget || !m_map)
		return;

    followUAVpositionAct->setChecked(false);

	internals::PointLatLng home_pos = m_home_position.coord;	// get the home location
    m_map->SetCurrentPosition(home_pos);                    // center the map onto the home location
}


void OPMapGadgetWidget::zoomIn()
{
	if (!m_widget || !m_map)
		return;

    int zoom = m_map->ZoomTotal() + 1;

	if (zoom < m_min_zoom) zoom = m_min_zoom;
    else
	if (zoom > m_max_zoom) zoom = m_max_zoom;

    m_map->SetZoom(zoom);
}

void OPMapGadgetWidget::zoomOut()
{
	if (!m_widget || !m_map)
		return;

    int zoom = m_map->ZoomTotal() - 1;

	if (zoom < m_min_zoom) zoom = m_min_zoom;
    else
	if (zoom > m_max_zoom) zoom = m_max_zoom;

    m_map->SetZoom(zoom);
}

void OPMapGadgetWidget::setMaxUpdateRate(int update_rate)
{
	if (!m_widget || !m_map)
		return;

	int list_size = sizeof(max_update_rate_list) / sizeof(max_update_rate_list[0]);
	int min_rate = max_update_rate_list[0];
	int max_rate = max_update_rate_list[list_size - 1];

	if (update_rate < min_rate) update_rate = min_rate;
	else
	if (update_rate > max_rate) update_rate = max_rate;

	m_maxUpdateRate = update_rate;

	if (m_updateTimer)
		m_updateTimer->setInterval(m_maxUpdateRate);

//	if (m_statusUpdateTimer)
//		m_statusUpdateTimer->setInterval(m_maxUpdateRate);
}

void OPMapGadgetWidget::setZoom(int zoom)
{
	if (!m_widget || !m_map)
		return;

	if (zoom < m_min_zoom) zoom = m_min_zoom;
    else
	if (zoom > m_max_zoom) zoom = m_max_zoom;

    internals::MouseWheelZoomType::Types zoom_type = m_map->GetMouseWheelZoomType();
    m_map->SetMouseWheelZoomType(internals::MouseWheelZoomType::ViewCenter);

    m_map->SetZoom(zoom);

    m_map->SetMouseWheelZoomType(zoom_type);
}
void OPMapGadgetWidget::setOverlayOpacity(qreal value)
{
    if (!m_widget || !m_map)
        return;
    m_map->setOverlayOpacity(value);
    overlayOpacityAct.at(value*10)->setChecked(true);
}

void OPMapGadgetWidget::setHomePosition(QPointF pos)
{
    if (!m_widget || !m_map)
        return;

    double latitude = pos.y();
    double longitude = pos.x();

    if (latitude != latitude || longitude != longitude)
        return; // nan prevention

    if (latitude >  90) latitude =  90;
    else
    if (latitude < -90) latitude = -90;

    if (longitude >  180) longitude =  180;
    else
    if (longitude < -180) longitude = -180;

    m_map->Home->SetCoord(internals::PointLatLng(latitude, longitude));
}

void OPMapGadgetWidget::setPosition(QPointF pos)
{
	if (!m_widget || !m_map)
		return;

    double latitude = pos.y();
    double longitude = pos.x();

    if (latitude != latitude || longitude != longitude)
        return; // nan prevention

    if (latitude >  90) latitude =  90;
    else
    if (latitude < -90) latitude = -90;

    if (longitude >  180) longitude =  180;
    else
    if (longitude < -180) longitude = -180;

    m_map->SetCurrentPosition(internals::PointLatLng(latitude, longitude));
}

void OPMapGadgetWidget::setMapProvider(QString provider)
{
	if (!m_widget || !m_map)
		return;

    m_map->SetMapType(mapcontrol::Helper::MapTypeFromString(provider));
}

void OPMapGadgetWidget::setAccessMode(QString accessMode)
{
	if (!m_widget || !m_map)
		return;

    m_map->configuration->SetAccessMode(mapcontrol::Helper::AccessModeFromString(accessMode));
}

void OPMapGadgetWidget::setUseOpenGL(bool useOpenGL)
{
	if (!m_widget || !m_map)
		return;

    m_map->SetUseOpenGL(useOpenGL);
}

void OPMapGadgetWidget::setShowTileGridLines(bool showTileGridLines)
{
	if (!m_widget || !m_map)
		return;

    m_map->SetShowTileGridLines(showTileGridLines);
}

void OPMapGadgetWidget::setUseMemoryCache(bool useMemoryCache)
{
	if (!m_widget || !m_map)
		return;

    m_map->configuration->SetUseMemoryCache(useMemoryCache);
}

void OPMapGadgetWidget::setCacheLocation(QString cacheLocation)
{
	if (!m_widget || !m_map)
		return;

    cacheLocation = cacheLocation.simplified();	// remove any surrounding spaces

    if (cacheLocation.isEmpty()) return;

    if (!cacheLocation.endsWith(QDir::separator())) cacheLocation += QDir::separator();

    QDir dir;
    if (!dir.exists(cacheLocation))
        if (!dir.mkpath(cacheLocation))
            return;
    m_map->configuration->SetCacheLocation(cacheLocation);
}

void OPMapGadgetWidget::setMapMode(opMapModeType mode)
{
	if (!m_widget || !m_map)
		return;

    if (mode != Normal_MapMode && mode != MagicWaypoint_MapMode)
        mode = Normal_MapMode;  // fix error

    if (m_map_mode == mode)
    {   // no change in map mode
        switch (mode)
        {   // make sure the UI buttons are set correctly
            case Normal_MapMode:
                m_widget->toolButtonMagicWaypointMapMode->setChecked(false);
                m_widget->toolButtonNormalMapMode->setChecked(true);
                break;
            case MagicWaypoint_MapMode:
                m_widget->toolButtonNormalMapMode->setChecked(false);
                m_widget->toolButtonMagicWaypointMapMode->setChecked(true);
                break;
        }
        return;
    }

    switch (mode)
    {
        case Normal_MapMode:
            m_map_mode = Normal_MapMode;

            m_widget->toolButtonMagicWaypointMapMode->setChecked(false);
            m_widget->toolButtonNormalMapMode->setChecked(true);

            hideMagicWaypointControls();

            magicWayPoint->setVisible(false);
            m_map->WPSetVisibleAll(true);

            break;

        case MagicWaypoint_MapMode:
            m_map_mode = MagicWaypoint_MapMode;

            m_widget->toolButtonNormalMapMode->setChecked(false);
            m_widget->toolButtonMagicWaypointMapMode->setChecked(true);

            showMagicWaypointControls();

            // delete the normal waypoints from the map

            m_map->WPSetVisibleAll(false);
            magicWayPoint->setVisible(true);

            break;
    }
}

// *************************************************************************************
// Context menu stuff

void OPMapGadgetWidget::createActions()
{
	int list_size;

	if (!m_widget || !m_map)
		return;

    // ***********************
    // create menu actions

    closeAct1 = new QAction(tr("Close menu"), this);
    closeAct1->setStatusTip(tr("Close the context menu"));

    closeAct2 = new QAction(tr("Close menu"), this);
    closeAct2->setStatusTip(tr("Close the context menu"));

    reloadAct = new QAction(tr("&Reload map"), this);
    reloadAct->setShortcut(tr("F5"));
    reloadAct->setStatusTip(tr("Reload the map tiles"));
    connect(reloadAct, SIGNAL(triggered()), this, SLOT(onReloadAct_triggered()));
    this->addAction(reloadAct);
    ripAct = new QAction(tr("&Rip map"), this);
    ripAct->setStatusTip(tr("Rip the map tiles"));
    connect(ripAct, SIGNAL(triggered()), this, SLOT(onRipAct_triggered()));

    copyMouseLatLonToClipAct = new QAction(tr("Mouse latitude and longitude"), this);
    copyMouseLatLonToClipAct->setStatusTip(tr("Copy the mouse latitude and longitude to the clipboard"));
    connect(copyMouseLatLonToClipAct, SIGNAL(triggered()), this, SLOT(onCopyMouseLatLonToClipAct_triggered()));

    copyMouseLatToClipAct = new QAction(tr("Mouse latitude"), this);
    copyMouseLatToClipAct->setStatusTip(tr("Copy the mouse latitude to the clipboard"));
    connect(copyMouseLatToClipAct, SIGNAL(triggered()), this, SLOT(onCopyMouseLatToClipAct_triggered()));

    copyMouseLonToClipAct = new QAction(tr("Mouse longitude"), this);
    copyMouseLonToClipAct->setStatusTip(tr("Copy the mouse longitude to the clipboard"));
    connect(copyMouseLonToClipAct, SIGNAL(triggered()), this, SLOT(onCopyMouseLonToClipAct_triggered()));

    showCompassAct = new QAction(tr("Show compass"), this);
    showCompassAct->setStatusTip(tr("Show/Hide the compass"));
    showCompassAct->setCheckable(true);
    showCompassAct->setChecked(true);
    connect(showCompassAct, SIGNAL(toggled(bool)), this, SLOT(onShowCompassAct_toggled(bool)));

    showDiagnostics = new QAction(tr("Show Diagnostics"), this);
    showDiagnostics->setStatusTip(tr("Show/Hide the diagnostics"));
    showDiagnostics->setCheckable(true);
    showDiagnostics->setChecked(false);
    connect(showDiagnostics, SIGNAL(toggled(bool)), this, SLOT(onShowDiagnostics_toggled(bool)));

    showUAVInfo = new QAction(tr("Show UAV Info"), this);
    showUAVInfo->setStatusTip(tr("Show/Hide the UAV info"));
    showUAVInfo->setCheckable(true);
    showUAVInfo->setChecked(false);
    connect(showUAVInfo, SIGNAL(toggled(bool)), this, SLOT(onShowUAVInfo_toggled(bool)));

    showHomeAct = new QAction(tr("Show Home"), this);
    showHomeAct->setStatusTip(tr("Show/Hide the Home location"));
    showHomeAct->setCheckable(true);
    showHomeAct->setChecked(true);
    connect(showHomeAct, SIGNAL(toggled(bool)), this, SLOT(onShowHomeAct_toggled(bool)));

    showUAVAct = new QAction(tr("Show UAV"), this);
    showUAVAct->setStatusTip(tr("Show/Hide the UAV"));
    showUAVAct->setCheckable(true);
    showUAVAct->setChecked(true);
    connect(showUAVAct, SIGNAL(toggled(bool)), this, SLOT(onShowUAVAct_toggled(bool)));

    changeDefaultLocalAndZoom = new QAction(tr("Set default zoom and location"), this);
    changeDefaultLocalAndZoom->setStatusTip(tr("Changes the map default zoom and location to the current values"));
    connect(changeDefaultLocalAndZoom, SIGNAL(triggered()), this, SLOT(onChangeDefaultLocalAndZoom()));

    zoomInAct = new QAction(tr("Zoom &In"), this);
    zoomInAct->setShortcut(Qt::Key_PageUp);
    zoomInAct->setStatusTip(tr("Zoom the map in"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(onGoZoomInAct_triggered()));
    this->addAction(zoomInAct);

    zoomOutAct = new QAction(tr("Zoom &Out"), this);
    zoomOutAct->setShortcut(Qt::Key_PageDown);
    zoomOutAct->setStatusTip(tr("Zoom the map out"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(onGoZoomOutAct_triggered()));
    this->addAction(zoomOutAct);

    goMouseClickAct = new QAction(tr("Go to where you right clicked the mouse"), this);
    goMouseClickAct->setStatusTip(tr("Center the map onto where you right clicked the mouse"));
    connect(goMouseClickAct, SIGNAL(triggered()), this, SLOT(onGoMouseClickAct_triggered()));

    setHomeAct = new QAction(tr("Set the home location"), this);
    setHomeAct->setStatusTip(tr("Set the home location to where you clicked"));
    #if !defined(allow_manual_home_location_move)
        setHomeAct->setEnabled(false);
    #endif
    connect(setHomeAct, SIGNAL(triggered()), this, SLOT(onSetHomeAct_triggered()));

    goHomeAct = new QAction(tr("Go to &Home location"), this);
    goHomeAct->setShortcut(tr("Ctrl+H"));
    goHomeAct->setStatusTip(tr("Center the map onto the home location"));
    connect(goHomeAct, SIGNAL(triggered()), this, SLOT(onGoHomeAct_triggered()));

    goUAVAct = new QAction(tr("Go to &UAV location"), this);
    goUAVAct->setShortcut(tr("Ctrl+U"));
    goUAVAct->setStatusTip(tr("Center the map onto the UAV location"));
    connect(goUAVAct, SIGNAL(triggered()), this, SLOT(onGoUAVAct_triggered()));

    followUAVpositionAct = new QAction(tr("Follow UAV position"), this);
    followUAVpositionAct->setShortcut(tr("Ctrl+F"));
    followUAVpositionAct->setStatusTip(tr("Keep the map centered onto the UAV"));
    followUAVpositionAct->setCheckable(true);
    followUAVpositionAct->setChecked(false);
    connect(followUAVpositionAct, SIGNAL(toggled(bool)), this, SLOT(onFollowUAVpositionAct_toggled(bool)));

    followUAVheadingAct = new QAction(tr("Follow UAV heading"), this);
    followUAVheadingAct->setShortcut(tr("Ctrl+F"));
    followUAVheadingAct->setStatusTip(tr("Keep the map rotation to the UAV heading"));
    followUAVheadingAct->setCheckable(true);
    followUAVheadingAct->setChecked(false);
    connect(followUAVheadingAct, SIGNAL(toggled(bool)), this, SLOT(onFollowUAVheadingAct_toggled(bool)));

    /*
      TODO: Waypoint support is disabled for v1.0
      */

#ifdef USE_PATHPLANNER
    wayPointEditorAct = new QAction(tr("&Waypoint editor"), this);
    wayPointEditorAct->setShortcut(tr("Ctrl+W"));
    wayPointEditorAct->setStatusTip(tr("Open the waypoint editor"));
    connect(wayPointEditorAct, SIGNAL(triggered()), this, SLOT(onOpenWayPointEditorAct_triggered()));

    addWayPointActFromContextMenu = new QAction(tr("&Add waypoint"), this);
    addWayPointActFromContextMenu->setShortcut(tr("Ctrl+A"));
    addWayPointActFromContextMenu->setStatusTip(tr("Add waypoint"));
    connect(addWayPointActFromContextMenu, SIGNAL(triggered()), this, SLOT(onAddWayPointAct_triggeredFromContextMenu()));

    addWayPointActFromThis = new QAction(tr("&Add waypoint"), this);
    addWayPointActFromThis->setShortcut(tr("Ctrl+A"));
    addWayPointActFromThis->setStatusTip(tr("Add waypoint"));
    connect(addWayPointActFromThis, SIGNAL(triggered()), this, SLOT(onAddWayPointAct_triggeredFromThis()));
    this->addAction(addWayPointActFromThis);

    editWayPointAct = new QAction(tr("&Edit waypoint"), this);
    editWayPointAct->setShortcut(tr("Ctrl+E"));
    editWayPointAct->setStatusTip(tr("Edit waypoint"));
    connect(editWayPointAct, SIGNAL(triggered()), this, SLOT(onEditWayPointAct_triggered()));

    lockWayPointAct = new QAction(tr("&Lock waypoint"), this);
    lockWayPointAct->setStatusTip(tr("Lock/Unlock a waypoint"));
    lockWayPointAct->setCheckable(true);
    lockWayPointAct->setChecked(false);
    connect(lockWayPointAct, SIGNAL(triggered()), this, SLOT(onLockWayPointAct_triggered()));

    deleteWayPointAct = new QAction(tr("&Delete waypoint"), this);
    deleteWayPointAct->setShortcut(tr("Ctrl+D"));
    deleteWayPointAct->setStatusTip(tr("Delete waypoint"));
    connect(deleteWayPointAct, SIGNAL(triggered()), this, SLOT(onDeleteWayPointAct_triggered()));

    clearWayPointsAct = new QAction(tr("&Clear waypoints"), this);
    clearWayPointsAct->setShortcut(tr("Ctrl+C"));
    clearWayPointsAct->setStatusTip(tr("Clear waypoints"));
    connect(clearWayPointsAct, SIGNAL(triggered()), this, SLOT(onClearWayPointsAct_triggered()));
#endif
    overlayOpacityActGroup = new QActionGroup(this);
    connect(overlayOpacityActGroup, SIGNAL(triggered(QAction *)), this, SLOT(onOverlayOpacityActGroup_triggered(QAction *)));
    overlayOpacityAct.clear();
    for (int i = 0; i <= 10; i++)
    {
        QAction *overlayAct = new QAction(QString::number(i*10), overlayOpacityActGroup);
        overlayAct->setCheckable(true);
        overlayAct->setData(i*10);
        overlayOpacityAct.append(overlayAct);
    }

    homeMagicWaypointAct = new QAction(tr("Home magic waypoint"), this);
    homeMagicWaypointAct->setStatusTip(tr("Move the magic waypoint to the home position"));
    connect(homeMagicWaypointAct, SIGNAL(triggered()), this, SLOT(onHomeMagicWaypointAct_triggered()));

    mapModeActGroup = new QActionGroup(this);
    connect(mapModeActGroup, SIGNAL(triggered(QAction *)), this, SLOT(onMapModeActGroup_triggered(QAction *)));
    mapModeAct.clear();
    {
        QAction *map_mode_act;

        map_mode_act = new QAction(tr("Normal"), mapModeActGroup);
        map_mode_act->setCheckable(true);
        map_mode_act->setChecked(m_map_mode == Normal_MapMode);
        map_mode_act->setData((int)Normal_MapMode);
        mapModeAct.append(map_mode_act);

        map_mode_act = new QAction(tr("Magic Waypoint"), mapModeActGroup);
        map_mode_act->setCheckable(true);
        map_mode_act->setChecked(m_map_mode == MagicWaypoint_MapMode);
        map_mode_act->setData((int)MagicWaypoint_MapMode);
        mapModeAct.append(map_mode_act);
    }

    zoomActGroup = new QActionGroup(this);
    connect(zoomActGroup, SIGNAL(triggered(QAction *)), this, SLOT(onZoomActGroup_triggered(QAction *)));
    zoomAct.clear();
	for (int i = m_min_zoom; i <= m_max_zoom; i++)
    {
        QAction *zoom_act = new QAction(QString::number(i), zoomActGroup);
        zoom_act->setCheckable(true);
        zoom_act->setData(i);
        zoomAct.append(zoom_act);
    }

	maxUpdateRateActGroup = new QActionGroup(this);
	connect(maxUpdateRateActGroup, SIGNAL(triggered(QAction *)), this, SLOT(onMaxUpdateRateActGroup_triggered(QAction *)));
	maxUpdateRateAct.clear();
	list_size = sizeof(max_update_rate_list) / sizeof(max_update_rate_list[0]);
	for (int i = 0; i < list_size; i++)
	{
		QAction *maxUpdateRate_act;
		int j = max_update_rate_list[i];
		maxUpdateRate_act = new QAction(QString::number(j), maxUpdateRateActGroup);
		maxUpdateRate_act->setCheckable(true);
		maxUpdateRate_act->setData(j);
		maxUpdateRate_act->setChecked(j == m_maxUpdateRate);
		maxUpdateRateAct.append(maxUpdateRate_act);
	}

	// *****
    // safe area

    showSafeAreaAct = new QAction(tr("Show Safe Area"), this);
    showSafeAreaAct->setStatusTip(tr("Show/Hide the Safe Area around the home location"));
    showSafeAreaAct->setCheckable(true);
    showSafeAreaAct->setChecked(m_map->Home->ShowSafeArea());
    connect(showSafeAreaAct, SIGNAL(toggled(bool)), this, SLOT(onShowSafeAreaAct_toggled(bool)));

    safeAreaActGroup = new QActionGroup(this);
    connect(safeAreaActGroup, SIGNAL(triggered(QAction *)), this, SLOT(onSafeAreaActGroup_triggered(QAction *)));
    safeAreaAct.clear();
	list_size = sizeof(safe_area_radius_list) / sizeof(safe_area_radius_list[0]);
	for (int i = 0; i < list_size; i++)
    {
        int safeArea = safe_area_radius_list[i];
        QAction *safeArea_act = new QAction(QString::number(safeArea) + "m", safeAreaActGroup);
        safeArea_act->setCheckable(true);
        safeArea_act->setChecked(safeArea == m_map->Home->SafeArea());
        safeArea_act->setData(safeArea);
        safeAreaAct.append(safeArea_act);
    }

    // *****
    // UAV trail

    uavTrailTypeActGroup = new QActionGroup(this);
    connect(uavTrailTypeActGroup, SIGNAL(triggered(QAction *)), this, SLOT(onUAVTrailTypeActGroup_triggered(QAction *)));
    uavTrailTypeAct.clear();
    QStringList uav_trail_type_list = mapcontrol::Helper::UAVTrailTypes();
    for (int i = 0; i < uav_trail_type_list.count(); i++)
    {
        mapcontrol::UAVTrailType::Types uav_trail_type = mapcontrol::Helper::UAVTrailTypeFromString(uav_trail_type_list[i]);
        QAction *uavTrailType_act = new QAction(mapcontrol::Helper::StrFromUAVTrailType(uav_trail_type), uavTrailTypeActGroup);
        uavTrailType_act->setCheckable(true);
        uavTrailType_act->setChecked(uav_trail_type == m_map->UAV->GetTrailType());
        uavTrailType_act->setData(i);
        uavTrailTypeAct.append(uavTrailType_act);
    }

    showTrailAct = new QAction(tr("Show Trail dots"), this);
    showTrailAct->setStatusTip(tr("Show/Hide the Trail dots"));
    showTrailAct->setCheckable(true);
    showTrailAct->setChecked(true);
    connect(showTrailAct, SIGNAL(toggled(bool)), this, SLOT(onShowTrailAct_toggled(bool)));

    showTrailLineAct = new QAction(tr("Show Trail lines"), this);
    showTrailLineAct->setStatusTip(tr("Show/Hide the Trail lines"));
    showTrailLineAct->setCheckable(true);
    showTrailLineAct->setChecked(true);
    connect(showTrailLineAct, SIGNAL(toggled(bool)), this, SLOT(onShowTrailLineAct_toggled(bool)));

    clearUAVtrailAct = new QAction(tr("Clear UAV trail"), this);
    clearUAVtrailAct->setStatusTip(tr("Clear the UAV trail"));
    connect(clearUAVtrailAct, SIGNAL(triggered()), this, SLOT(onClearUAVtrailAct_triggered()));

    uavTrailTimeActGroup = new QActionGroup(this);
    connect(uavTrailTimeActGroup, SIGNAL(triggered(QAction *)), this, SLOT(onUAVTrailTimeActGroup_triggered(QAction *)));
    uavTrailTimeAct.clear();
	list_size = sizeof(uav_trail_time_list) / sizeof(uav_trail_time_list[0]);
	for (int i = 0; i < list_size; i++)
    {
        int uav_trail_time = uav_trail_time_list[i];
        QAction *uavTrailTime_act = new QAction(QString::number(uav_trail_time) + " sec", uavTrailTimeActGroup);
        uavTrailTime_act->setCheckable(true);
        uavTrailTime_act->setChecked(uav_trail_time == m_map->UAV->TrailTime());
        uavTrailTime_act->setData(uav_trail_time);
        uavTrailTimeAct.append(uavTrailTime_act);
    }

    uavTrailDistanceActGroup = new QActionGroup(this);
    connect(uavTrailDistanceActGroup, SIGNAL(triggered(QAction *)), this, SLOT(onUAVTrailDistanceActGroup_triggered(QAction *)));
    uavTrailDistanceAct.clear();
	list_size = sizeof(uav_trail_distance_list) / sizeof(uav_trail_distance_list[0]);
	for (int i = 0; i < list_size; i++)
    {
        int uav_trail_distance = uav_trail_distance_list[i];
        QAction *uavTrailDistance_act = new QAction(QString::number(uav_trail_distance) + " meters", uavTrailDistanceActGroup);
        uavTrailDistance_act->setCheckable(true);
        uavTrailDistance_act->setChecked(uav_trail_distance == m_map->UAV->TrailDistance());
        uavTrailDistance_act->setData(uav_trail_distance);
        uavTrailDistanceAct.append(uavTrailDistance_act);
    }
}

void OPMapGadgetWidget::onReloadAct_triggered()
{
	if (!m_widget || !m_map)
		return;

    m_map->ReloadMap();
}

void OPMapGadgetWidget::onRipAct_triggered()
{
    m_map->RipMap();
}

void OPMapGadgetWidget::onCopyMouseLatLonToClipAct_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(QString::number(m_context_menu_lat_lon.Lat(), 'f', 7) + ", " + QString::number(m_context_menu_lat_lon.Lng(), 'f', 7), QClipboard::Clipboard);
}

void OPMapGadgetWidget::onCopyMouseLatToClipAct_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(QString::number(m_context_menu_lat_lon.Lat(), 'f', 7), QClipboard::Clipboard);
}

void OPMapGadgetWidget::onCopyMouseLonToClipAct_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(QString::number(m_context_menu_lat_lon.Lng(), 'f', 7), QClipboard::Clipboard);
}


void OPMapGadgetWidget::onShowCompassAct_toggled(bool show)
{
	if (!m_widget || !m_map)
		return;

    m_map->SetShowCompass(show);
}

void OPMapGadgetWidget::onShowDiagnostics_toggled(bool show)
{
    if (!m_widget || !m_map)
		return;

    m_map->SetShowDiagnostics(show);
}

void OPMapGadgetWidget::onShowUAVInfo_toggled(bool show)
{
    if (!m_widget || !m_map)
        return;

    m_map->UAV->SetShowUAVInfo(show);
}

void OPMapGadgetWidget::onShowHomeAct_toggled(bool show)
{
	if (!m_widget || !m_map)
		return;

    m_map->Home->setVisible(show);
}

void OPMapGadgetWidget::onShowUAVAct_toggled(bool show)
{
	if (!m_widget || !m_map)
		return;

    m_map->UAV->setVisible(show);
    if(m_map->GPS)
        m_map->GPS->setVisible(show);
}

void OPMapGadgetWidget::onShowTrailAct_toggled(bool show)
{
	if (!m_widget || !m_map)
		return;

    m_map->UAV->SetShowTrail(show);
    if(m_map->GPS)
        m_map->GPS->SetShowTrail(show);
}

void OPMapGadgetWidget::onShowTrailLineAct_toggled(bool show)
{
	if (!m_widget || !m_map)
		return;

    m_map->UAV->SetShowTrailLine(show);
    if(m_map->GPS)
        m_map->GPS->SetShowTrailLine(show);
}

void OPMapGadgetWidget::onMapModeActGroup_triggered(QAction *action)
{
    if (!m_widget || !m_map || !action)
        return;

    opMapModeType mode = (opMapModeType)action->data().toInt();

    setMapMode(mode);
}

void OPMapGadgetWidget::onGoZoomInAct_triggered()
{
    zoomIn();
}

void OPMapGadgetWidget::onGoZoomOutAct_triggered()
{
    zoomOut();
}

void OPMapGadgetWidget::onZoomActGroup_triggered(QAction *action)
{
	if (!m_widget || !m_map || !action)
		return;

    setZoom(action->data().toInt());
}

void OPMapGadgetWidget::onMaxUpdateRateActGroup_triggered(QAction *action)
{
	if (!m_widget || !m_map || !action)
		return;

    setMaxUpdateRate(action->data().toInt());
}

void OPMapGadgetWidget::onChangeDefaultLocalAndZoom()
{
    emit defaultLocationAndZoomChanged(m_map->CurrentPosition().Lng(),m_map->CurrentPosition().Lat(),m_map->ZoomTotal());
}

void OPMapGadgetWidget::onGoMouseClickAct_triggered()
{
	if (!m_widget || !m_map)
		return;

    m_map->SetCurrentPosition(m_map->currentMousePosition());   // center the map onto the mouse position
}

void OPMapGadgetWidget::onSetHomeAct_triggered()
{
	if (!m_widget || !m_map)
		return;

    float altitude=0;
    bool ok;

    //Get desired HomeLocation altitude from dialog box.
    //TODO: Populate box with altitude already in HomeLocation UAVO
    altitude = QInputDialog::getDouble(this, tr("Set home altitude"),
                                      tr("In [m], referenced to WGS84:"), altitude, -100, 100000, 2, &ok);

    setHome(m_context_menu_lat_lon, altitude);

    setHomeLocationObject();  // update the HomeLocation UAVObject
}

void OPMapGadgetWidget::onGoHomeAct_triggered()
{
	if (!m_widget || !m_map)
		return;

    goHome();
}

void OPMapGadgetWidget::onGoUAVAct_triggered()
{
	if (!m_widget || !m_map)
		return;

    double latitude;
    double longitude;
    double altitude;
	if (getUAVPosition(latitude, longitude, altitude))  // get current UAV position
    {
        internals::PointLatLng uav_pos = internals::PointLatLng(latitude, longitude);	// current UAV position
        internals::PointLatLng map_pos = m_map->CurrentPosition();                      // current MAP position
        if (map_pos != uav_pos) m_map->SetCurrentPosition(uav_pos);                     // center the map onto the UAV
    }
}

void OPMapGadgetWidget::onFollowUAVpositionAct_toggled(bool checked)
{
	if (!m_widget || !m_map)
		return;

    if (m_widget->toolButtonMapUAV->isChecked() != checked)
        m_widget->toolButtonMapUAV->setChecked(checked);

    setMapFollowingMode();
}

void OPMapGadgetWidget::onFollowUAVheadingAct_toggled(bool checked)
{
	if (!m_widget || !m_map)
		return;

    if (m_widget->toolButtonMapUAVheading->isChecked() != checked)
        m_widget->toolButtonMapUAVheading->setChecked(checked);

    setMapFollowingMode();
}

void OPMapGadgetWidget::onUAVTrailTypeActGroup_triggered(QAction *action)
{
	if (!m_widget || !m_map || !action)
		return;

    int trail_type_idx = action->data().toInt();

    QStringList uav_trail_type_list = mapcontrol::Helper::UAVTrailTypes();
    mapcontrol::UAVTrailType::Types uav_trail_type = mapcontrol::Helper::UAVTrailTypeFromString(uav_trail_type_list[trail_type_idx]);

    m_map->UAV->SetTrailType(uav_trail_type);
}

void OPMapGadgetWidget::onClearUAVtrailAct_triggered()
{
	if (!m_widget || !m_map)
		return;

    m_map->UAV->DeleteTrail();
    if(m_map->GPS)
        m_map->GPS->DeleteTrail();
}

void OPMapGadgetWidget::onUAVTrailTimeActGroup_triggered(QAction *action)
{
	if (!m_widget || !m_map || !action)
		return;

    int trail_time = (double)action->data().toInt();

    m_map->UAV->SetTrailTime(trail_time);
}

void OPMapGadgetWidget::onUAVTrailDistanceActGroup_triggered(QAction *action)
{
	if (!m_widget || !m_map || !action)
		return;

    int trail_distance = action->data().toInt();

    m_map->UAV->SetTrailDistance(trail_distance);
}

void OPMapGadgetWidget::onOpenWayPointEditorAct_triggered()
{
    table->show();
}
void OPMapGadgetWidget::onAddWayPointAct_triggeredFromContextMenu()
{
    onAddWayPointAct_triggered(m_context_menu_lat_lon);
}
void OPMapGadgetWidget::onAddWayPointAct_triggeredFromThis()
{
    onAddWayPointAct_triggered(lastLatLngMouse);
}

void OPMapGadgetWidget::onAddWayPointAct_triggered(internals::PointLatLng coord)
{
	if (!m_widget || !m_map)
		return;

    if (m_map_mode != Normal_MapMode)
        return;

    mapProxy->createWayPoint(coord);
}


/**
  * Called when the user asks to edit a waypoint from the map
  *
  * TODO: should open an interface to edit waypoint properties, or
  *       propagate the signal to a specific WP plugin (tbd).
  **/

void OPMapGadgetWidget::onEditWayPointAct_triggered()
{
	if (!m_widget || !m_map)
		return;

    if (m_map_mode != Normal_MapMode)
        return;

    if (!m_mouse_waypoint)
        return;

    waypoint_edit_dialog->editWaypoint(m_mouse_waypoint);
    m_mouse_waypoint = NULL;
}


/**
  * TODO: unused for v1.0
  */

void OPMapGadgetWidget::onLockWayPointAct_triggered()
{
    if (!m_widget || !m_map || !m_mouse_waypoint)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    bool locked = (m_mouse_waypoint->flags() & QGraphicsItem::ItemIsMovable) == 0;
    m_mouse_waypoint->setFlag(QGraphicsItem::ItemIsMovable, locked);

    if (!locked)
        m_mouse_waypoint->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker2.png"));
    else
        m_mouse_waypoint->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker1.png"));
    m_mouse_waypoint->update();

    m_mouse_waypoint = NULL;
}

void OPMapGadgetWidget::onDeleteWayPointAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    if (!m_mouse_waypoint)
        return;

    mapProxy->deleteWayPoint(m_mouse_waypoint->Number());
}

void OPMapGadgetWidget::onClearWayPointsAct_triggered()
{

    //First, ask to ensure this is what the user wants to do
    QMessageBox msgBox;
    msgBox.setText(tr("Are you sure you want to clear waypoints?"));
    msgBox.setInformativeText(tr("All associated data will be lost."));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int ret = msgBox.exec();

    if (ret == QMessageBox::No)
    {
        return;
    }

    if (!m_widget || !m_map)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    mapProxy->deleteAll();

 }


void OPMapGadgetWidget::onHomeMagicWaypointAct_triggered()
{
    // center the magic waypoint on the home position
    homeMagicWaypoint();
}

void OPMapGadgetWidget::onShowSafeAreaAct_toggled(bool show)
{
    if (!m_widget || !m_map)
        return;

    m_map->Home->SetShowSafeArea(show);             // show the safe area
    m_map->Home->SetToggleRefresh(true);
    m_map->Home->RefreshPos();
}

void OPMapGadgetWidget::onSafeAreaActGroup_triggered(QAction *action)
{
	if (!m_widget || !m_map || !action)
        return;

    int radius = action->data().toInt();

    m_map->Home->SetSafeArea(radius);               // set the radius (meters)
    m_map->Home->RefreshPos();

    // move the magic waypoint if need be to keep it within the safe area around the home position
    keepMagicWaypointWithInSafeArea();
}

/**
* move the magic waypoint to the home position
**/
void OPMapGadgetWidget::homeMagicWaypoint()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != MagicWaypoint_MapMode)
        return;

    magicWayPoint->SetCoord(m_home_position.coord);
}

// *************************************************************************************
// move the UAV to the magic waypoint position

void OPMapGadgetWidget::moveToMagicWaypointPosition()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != MagicWaypoint_MapMode)
        return;
}

// *************************************************************************************
// show/hide the magic waypoint controls

void OPMapGadgetWidget::hideMagicWaypointControls()
{
    m_widget->lineWaypoint->setVisible(false);
    m_widget->toolButtonHomeWaypoint->setVisible(false);
    m_widget->toolButtonMoveToWP->setVisible(false);
}

void OPMapGadgetWidget::showMagicWaypointControls()
{
    m_widget->lineWaypoint->setVisible(true);
    m_widget->toolButtonHomeWaypoint->setVisible(true);

    #if defined(allow_manual_home_location_move)
        m_widget->toolButtonMoveToWP->setVisible(true);
    #else
        m_widget->toolButtonMoveToWP->setVisible(false);
    #endif
}

// *************************************************************************************
// move the magic waypoint to keep it within the safe area boundry

void OPMapGadgetWidget::keepMagicWaypointWithInSafeArea()
{

    // calcute the bearing and distance from the home position to the magic waypoint
    double dist = distance(m_home_position.coord, magicWayPoint->Coord());
    double bear = bearing(m_home_position.coord, magicWayPoint->Coord());

    // get the maximum safe distance - in kilometers
    double boundry_dist = (double)m_map->Home->SafeArea() / 1000;

    if (dist > boundry_dist) dist = boundry_dist;

    // move the magic waypoint;

    if (m_map_mode == MagicWaypoint_MapMode)
    {   // move the on-screen waypoint
        if (magicWayPoint)
            magicWayPoint->SetCoord(destPoint(m_home_position.coord, bear, dist));
    }
}

// *************************************************************************************
// return the distance between two points .. in kilometers

double OPMapGadgetWidget::distance(internals::PointLatLng from, internals::PointLatLng to)
{
    double lat1 = from.Lat() * deg_to_rad;
    double lon1 = from.Lng() * deg_to_rad;

    double lat2 = to.Lat() * deg_to_rad;
    double lon2 = to.Lng() * deg_to_rad;

    return (acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon2 - lon1)) * earth_mean_radius);

    // ***********************
}

// *************************************************************************************
// return the bearing from one point to another .. in degrees

double OPMapGadgetWidget::bearing(internals::PointLatLng from, internals::PointLatLng to)
{
    double lat1 = from.Lat() * deg_to_rad;
    double lon1 = from.Lng() * deg_to_rad;

    double lat2 = to.Lat() * deg_to_rad;
    double lon2 = to.Lng() * deg_to_rad;

//    double delta_lat = lat2 - lat1;
    double delta_lon = lon2 - lon1;

    double y = sin(delta_lon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(delta_lon);
    double bear = atan2(y, x) * rad_to_deg;

    bear += 360;
    while (bear < 0) bear += 360;
    while (bear >= 360) bear -= 360;

    return bear;
}

// *************************************************************************************
// return a destination lat/lon point given a source lat/lon point and the bearing and distance from the source point

internals::PointLatLng OPMapGadgetWidget::destPoint(internals::PointLatLng source, double bear, double dist)
{
    double lat1 = source.Lat() * deg_to_rad;
    double lon1 = source.Lng() * deg_to_rad;

    bear *= deg_to_rad;

    double ad = dist / earth_mean_radius;

    double lat2 = asin(sin(lat1) * cos(ad) + cos(lat1) * sin(ad) * cos(bear));
    double lon2 = lon1 + atan2(sin(bear) * sin(ad) * cos(lat1), cos(ad) - sin(lat1) * sin(lat2));

    return internals::PointLatLng(lat2 * rad_to_deg, lon2 * rad_to_deg);
}

// *************************************************************************************

bool OPMapGadgetWidget::getUAVPosition(double &latitude, double &longitude, double &altitude)
{
    double NED[3];
    double LLA[3];
    double homeLLA[3];

    Q_ASSERT(obm != NULL);

    HomeLocation *homeLocation = HomeLocation::GetInstance(obm);
    Q_ASSERT(homeLocation != NULL);
    HomeLocation::DataFields homeLocationData = homeLocation->getData();

    homeLLA[0] = homeLocationData.Latitude / 1e7;
    homeLLA[1] = homeLocationData.Longitude / 1e7;
    homeLLA[2] = homeLocationData.Altitude;

    PositionActual *positionActual = PositionActual::GetInstance(obm);
    Q_ASSERT(positionActual != NULL);
    PositionActual::DataFields positionActualData = positionActual->getData();

    NED[0] = positionActualData.North;
    NED[1] = positionActualData.East;
    NED[2] = positionActualData.Down;

    Utils::CoordinateConversions().NED2LLA_HomeLLA(homeLLA, NED, LLA);

    latitude = LLA[0];
    longitude = LLA[1];
    altitude = LLA[2];

    if (latitude != latitude) latitude = 0; // nan detection
    else if (latitude >  90) latitude =  90;
    else if (latitude < -90) latitude = -90;

    if (longitude != longitude) longitude = 0; // nan detection
    else if (longitude >  180) longitude =  180;
    else if (longitude < -180) longitude = -180;

    if (altitude != altitude) altitude = 0; // nan detection

    return true;
}

double OPMapGadgetWidget::getUAV_Yaw()
{
	if (!obm)
		return 0;

	UAVObject *obj = dynamic_cast<UAVDataObject*>(obm->getObject(QString("AttitudeActual")));
	double yaw = obj->getField(QString("Yaw"))->getDouble();

	if (yaw != yaw) yaw = 0; // nan detection

	while (yaw < 0) yaw += 360;
	while (yaw >= 360) yaw -= 360;

	return yaw;
}

bool OPMapGadgetWidget::getGPSPosition(double &latitude, double &longitude, double &altitude)
{
	double LLA[3];

	if (!obum)
		return false;

	if (obum->getGPSPosition(LLA) < 0)
		return false;	// error

	latitude = LLA[0];
	longitude = LLA[1];
	altitude = LLA[2];

    return true;
}

// *************************************************************************************

void OPMapGadgetWidget::setMapFollowingMode()
{
    if (!m_widget || !m_map)
        return;

    if (!followUAVpositionAct->isChecked())
    {
        m_map->UAV->SetMapFollowType(UAVMapFollowType::None);
        m_map->SetRotate(0);								// reset map rotation to 0deg
    }
    else
    if (!followUAVheadingAct->isChecked())
    {
        m_map->UAV->SetMapFollowType(UAVMapFollowType::CenterMap);
        m_map->SetRotate(0);								// reset map rotation to 0deg
    }
    else
    {
        m_map->UAV->SetMapFollowType(UAVMapFollowType::CenterMap);      // the map library won't let you reset the uav rotation if it's already in rotate mode

        m_map->UAV->SetUAVHeading(0);                                   // reset the UAV heading to 0deg
        m_map->UAV->SetMapFollowType(UAVMapFollowType::CenterAndRotateMap);
    }
}

// *************************************************************************************
// update the HomeLocation UAV Object

bool OPMapGadgetWidget::setHomeLocationObject()
{
	if (!obum)
		return false;

	double LLA[3] = {m_home_position.coord.Lat(), m_home_position.coord.Lng(), m_home_position.altitude};
	return (obum->setHomeLocation(LLA, true) >= 0);
}

// *************************************************************************************

void OPMapGadgetWidget::SetUavPic(QString UAVPic)
{
    m_map->SetUavPic(UAVPic);
}

void OPMapGadgetWidget::on_tbFind_clicked()
{
    QPalette pal = m_widget->leFind->palette();

    int result=m_map->SetCurrentPositionByKeywords(m_widget->leFind->text());
    if(result==core::GeoCoderStatusCode::G_GEO_SUCCESS)
    {
        pal.setColor( m_widget->leFind->backgroundRole(), Qt::green);
        m_widget->leFind->setPalette(pal);
        m_map->SetZoom(12);
    }
    else
    {
        pal.setColor( m_widget->leFind->backgroundRole(), Qt::red);
        m_widget->leFind->setPalette(pal);
    }
}

void OPMapGadgetWidget::onHomeDoubleClick(HomeItem *)
{
    new homeEditor(m_map->Home,this);
}

void OPMapGadgetWidget::onOverlayOpacityActGroup_triggered(QAction *action)
{
    if (!m_widget || !m_map || !action)
        return;

    m_map->setOverlayOpacity(action->data().toReal()/100);
    emit overlayOpacityChanged(action->data().toReal()/100);
}

void OPMapGadgetWidget::on_leFind_returnPressed()
{
    on_tbFind_clicked();
}
