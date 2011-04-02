/**
 ******************************************************************************
 *
 * @file       opmapgadgetwidget.cpp
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

#include "positionactual.h"
#include "homelocation.h"

#define allow_manual_home_location_move

// *************************************************************************************

#define deg_to_rad          ((double)M_PI / 180.0)
#define rad_to_deg          (180.0 / (double)M_PI)

#define earth_mean_radius   6371    // kilometers

#define	max_digital_zoom    3       // maximum allowed digital zoom level

const int safe_area_radius_list[] = {5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000};   // meters

const int uav_trail_time_list[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};                      // seconds

const int uav_trail_distance_list[] = {1, 2, 5, 10, 20, 50, 100, 200, 500};             // meters

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

    prev_tile_number = 0;

    min_zoom = max_zoom = 0;

    m_map_mode = Normal_MapMode;

    telemetry_connected = false;

    context_menu_lat_lon = mouse_lat_lon = internals::PointLatLng(0, 0);

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

    home_position.coord = pos_lat_lon;
    home_position.altitude = altitude;
    home_position.locked = false;

    // **************
    // default magic waypoint params

    magic_waypoint.map_wp_item = NULL;
    magic_waypoint.coord = home_position.coord;
    magic_waypoint.altitude = altitude;
    magic_waypoint.description = "Magic waypoint";
    magic_waypoint.locked = false;
    magic_waypoint.time_seconds = 0;
    magic_waypoint.hold_time_seconds = 0;

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

    min_zoom = m_widget->horizontalSliderZoom->minimum();	// minimum zoom we can accept
    max_zoom = m_widget->horizontalSliderZoom->maximum();	// maximum zoom we can accept

    m_map->SetMouseWheelZoomType(internals::MouseWheelZoomType::MousePositionWithoutCenter);	    // set how the mouse wheel zoom functions
    m_map->SetFollowMouse(true);				    // we want a contiuous mouse position reading

    m_map->SetShowHome(true);					    // display the HOME position on the map
    m_map->SetShowUAV(true);					    // display the UAV position on the map

	m_map->Home->SetSafeArea(safe_area_radius_list[0]);                         // set radius (meters)
    m_map->Home->SetShowSafeArea(true);                                         // show the safe area

    m_map->UAV->SetTrailTime(uav_trail_time_list[0]);                           // seconds
    m_map->UAV->SetTrailDistance(uav_trail_distance_list[1]);                   // meters

    m_map->UAV->SetTrailType(UAVTrailType::ByTimeElapsed);
//  m_map->UAV->SetTrailType(UAVTrailType::ByDistance);

    m_map->GPS->SetTrailTime(uav_trail_time_list[0]);                           // seconds
    m_map->GPS->SetTrailDistance(uav_trail_distance_list[1]);                   // meters

    m_map->GPS->SetTrailType(UAVTrailType::ByTimeElapsed);
//  m_map->GPS->SetTrailType(UAVTrailType::ByDistance);

    // **************

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_map);
    m_widget->mapWidget->setLayout(layout);

    // **************
    // set the user control options

    // TODO: this switch does not make sense, does it??

    switch (m_map_mode)
    {
        case Normal_MapMode:
            m_widget->toolButtonMagicWaypointMapMode->setChecked(false);
            m_widget->toolButtonNormalMapMode->setChecked(true);
            hideMagicWaypointControls();
            break;

        case MagicWaypoint_MapMode:
            m_widget->toolButtonNormalMapMode->setChecked(false);
            m_widget->toolButtonMagicWaypointMapMode->setChecked(true);
            showMagicWaypointControls();
            break;

        default:
            m_map_mode = Normal_MapMode;
            m_widget->toolButtonMagicWaypointMapMode->setChecked(false);
            m_widget->toolButtonNormalMapMode->setChecked(true);
            hideMagicWaypointControls();
            break;
    }

    m_widget->labelUAVPos->setText("---");
    m_widget->labelMapPos->setText("---");
    m_widget->labelMousePos->setText("---");
    m_widget->labelMapZoom->setText("---");


    // Splitter is not used at the moment:
    // m_widget->splitter->setCollapsible(1, false);

    // set the size of the collapsable widgets
    //QList<int> m_SizeList;
    //m_SizeList << 0 << 0 << 0;
    //m_widget->splitter->setSizes(m_SizeList);

    m_widget->progressBarMap->setMaximum(1);

/*
    #if defined(Q_OS_MAC)
    #elif defined(Q_OS_WIN)
	m_widget->comboBoxFindPlace->clear();
	loadComboBoxLines(m_widget->comboBoxFindPlace, QCoreApplication::applicationDirPath() + "/opmap_find_place_history.txt");
	m_widget->comboBoxFindPlace->setCurrentIndex(-1);
    #else
    #endif
*/


    // **************
    // map stuff

    connect(m_map, SIGNAL(zoomChanged(double, double, double)), this, SLOT(zoomChanged(double, double, double)));					// map zoom change signals
    connect(m_map, SIGNAL(OnCurrentPositionChanged(internals::PointLatLng)), this, SLOT(OnCurrentPositionChanged(internals::PointLatLng)));    // map poisition change signals
    connect(m_map, SIGNAL(OnTileLoadComplete()), this, SLOT(OnTileLoadComplete()));					// tile loading stop signals
    connect(m_map, SIGNAL(OnTileLoadStart()), this, SLOT(OnTileLoadStart()));					// tile loading start signals
    connect(m_map, SIGNAL(OnMapDrag()), this, SLOT(OnMapDrag()));							// map drag signals
    connect(m_map, SIGNAL(OnMapZoomChanged()), this, SLOT(OnMapZoomChanged()));					// map zoom changed
    connect(m_map, SIGNAL(OnMapTypeChanged(MapType::Types)), this, SLOT(OnMapTypeChanged(MapType::Types)));		// map type changed
    connect(m_map, SIGNAL(OnEmptyTileError(int, core::Point)), this, SLOT(OnEmptyTileError(int, core::Point)));	// tile error
    connect(m_map, SIGNAL(OnTilesStillToLoad(int)), this, SLOT(OnTilesStillToLoad(int)));				// tile loading signals
    connect(m_map, SIGNAL(WPNumberChanged(int const&,int const&,WayPointItem*)), this, SLOT(WPNumberChanged(int const&,int const&,WayPointItem*)));
    connect(m_map, SIGNAL(WPValuesChanged(WayPointItem*)), this, SLOT(WPValuesChanged(WayPointItem*)));
    connect(m_map, SIGNAL(WPInserted(int const&, WayPointItem*)), this, SLOT(WPInserted(int const&, WayPointItem*)));
    connect(m_map, SIGNAL(WPDeleted(int const&)), this, SLOT(WPDeleted(int const&)));

    m_map->SetCurrentPosition(home_position.coord);         // set the map position
    m_map->Home->SetCoord(home_position.coord);             // set the HOME position
    m_map->UAV->SetUAVPos(home_position.coord, 0.0);        // set the UAV position
    m_map->GPS->SetUAVPos(home_position.coord, 0.0);        // set the UAV position

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
    m_updateTimer->setInterval(200);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));
    m_updateTimer->start();

    m_statusUpdateTimer = new QTimer();
    m_statusUpdateTimer->setInterval(100);
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


    // this destructor doesn't appear to be called at shutdown???

//    #if defined(Q_OS_MAC)
//    #elif defined(Q_OS_WIN)
//	saveComboBoxLines(m_widget->comboBoxFindPlace, QCoreApplication::applicationDirPath() + "/opmap_find_place_history.txt");
//    #else
//    #endif

    m_waypoint_list_mutex.lock();
    foreach (t_waypoint *wp, m_waypoint_list)
    {
        if (!wp) continue;


        // todo:


        delete wp->map_wp_item;
    }
    m_waypoint_list_mutex.unlock();
    m_waypoint_list.clear();

	if (m_map)
	{
		delete m_map;
		m_map = NULL;
	}
}

// *************************************************************************************
// widget signals .. the mouseMoveEvent does not get called - don't yet know why

void OPMapGadgetWidget::resizeEvent(QResizeEvent *event)
{
    qDebug("opmap: resizeEvent");

    QWidget::resizeEvent(event);
}

void OPMapGadgetWidget::mouseMoveEvent(QMouseEvent *event)
{
    qDebug("opmap: mouseMoveEvent");

    if (m_widget && m_map)
    {
    }

    if (event->buttons() & Qt::LeftButton)
    {
//        QPoint pos = event->pos();
    }

    QWidget::mouseMoveEvent(event);
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
    context_menu_lat_lon = m_map->GetFromLocalToLatLng(p);
//    context_menu_lat_lon = m_map->currentMousePosition();

    if (!m_map->contentsRect().contains(p))
        return;					    // the mouse click was not on the map

    // show the mouse position
    s = QString::number(context_menu_lat_lon.Lat(), 'f', 7) + "  " + QString::number(context_menu_lat_lon.Lng(), 'f', 7);
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

    QMenu menu(this);

    menu.addAction(closeAct1);

    menu.addSeparator();

    menu.addAction(reloadAct);

    menu.addSeparator();

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
    menu.addMenu(&mapModeSubMenu);

    menu.addSeparator();

    QMenu copySubMenu(tr("Copy"), this);
    copySubMenu.addAction(copyMouseLatLonToClipAct);
    copySubMenu.addAction(copyMouseLatToClipAct);
    copySubMenu.addAction(copyMouseLonToClipAct);
    menu.addMenu(&copySubMenu);

    menu.addSeparator();

    /*
    menu.addAction(findPlaceAct);

    menu.addSeparator();
    */

    menu.addAction(showSafeAreaAct);
    QMenu safeAreaSubMenu(tr("Safe Area Radius") + " (" + QString::number(m_map->Home->SafeArea()) + "m)", this);
    for (int i = 0; i < safeAreaAct.count(); i++)
        safeAreaSubMenu.addAction(safeAreaAct.at(i));
    menu.addMenu(&safeAreaSubMenu);

    menu.addSeparator();

    menu.addAction(showCompassAct);

    menu.addAction(showDiagnostics);

    menu.addSeparator()->setText(tr("Zoom"));

    menu.addAction(zoomInAct);
    menu.addAction(zoomOutAct);

    QMenu zoomSubMenu(tr("&Zoom ") + "(" + QString::number(m_map->ZoomTotal()) + ")", this);
    for (int i = 0; i < zoomAct.count(); i++)
        zoomSubMenu.addAction(zoomAct.at(i));
    menu.addMenu(&zoomSubMenu);

    menu.addSeparator();

    menu.addAction(goMouseClickAct);

    menu.addSeparator()->setText(tr("HOME"));

    menu.addAction(setHomeAct);
    menu.addAction(showHomeAct);
    menu.addAction(goHomeAct);

    // ****
    // uav trails

    menu.addSeparator()->setText(tr("UAV Trail"));

    QMenu uavTrailTypeSubMenu(tr("UAV trail type") + " (" + mapcontrol::Helper::StrFromUAVTrailType(m_map->UAV->GetTrailType()) + ")", this);
    for (int i = 0; i < uavTrailTypeAct.count(); i++)
        uavTrailTypeSubMenu.addAction(uavTrailTypeAct.at(i));
    menu.addMenu(&uavTrailTypeSubMenu);

    QMenu uavTrailTimeSubMenu(tr("UAV trail time") + " (" + QString::number(m_map->UAV->TrailTime()) + " sec)", this);
    for (int i = 0; i < uavTrailTimeAct.count(); i++)
        uavTrailTimeSubMenu.addAction(uavTrailTimeAct.at(i));
    menu.addMenu(&uavTrailTimeSubMenu);

    QMenu uavTrailDistanceSubMenu(tr("UAV trail distance") + " (" + QString::number(m_map->UAV->TrailDistance()) + " meters)", this);
    for (int i = 0; i < uavTrailDistanceAct.count(); i++)
        uavTrailDistanceSubMenu.addAction(uavTrailDistanceAct.at(i));
    menu.addMenu(&uavTrailDistanceSubMenu);

    menu.addAction(showTrailAct);

    menu.addAction(showTrailLineAct);

    menu.addAction(clearUAVtrailAct);

    // ****

    menu.addSeparator()->setText(tr("UAV"));

    menu.addAction(showUAVAct);
    menu.addAction(followUAVpositionAct);
    menu.addAction(followUAVheadingAct);
    menu.addAction(goUAVAct);

    // *********

    switch (m_map_mode)
    {
        case Normal_MapMode:
        // only show the waypoint stuff if not in 'magic waypoint' mode
            /*
            menu.addSeparator()->setText(tr("Waypoints"));

            menu.addAction(wayPointEditorAct);
            menu.addAction(addWayPointAct);

            if (m_mouse_waypoint)
            {	// we have a waypoint under the mouse
                menu.addAction(editWayPointAct);

                lockWayPointAct->setChecked(waypoint_locked);
                menu.addAction(lockWayPointAct);

                if (!waypoint_locked)
                    menu.addAction(deleteWayPointAct);
            }

            m_waypoint_list_mutex.lock();
            if (m_waypoint_list.count() > 0)
                menu.addAction(clearWayPointsAct);	// we have waypoints
            m_waypoint_list_mutex.unlock();
            */

            break;

        case MagicWaypoint_MapMode:
            menu.addSeparator()->setText(tr("Waypoints"));
            menu.addAction(homeMagicWaypointAct);
            break;
    }

    // *********

    menu.addSeparator();

    menu.addAction(closeAct2);

    menu.exec(event->globalPos());  // popup the menu

    // ****************
}

void OPMapGadgetWidget::keyPressEvent(QKeyEvent* event)
{
    qDebug() << "opmap: keyPressEvent, key =" << event->key() << endl;

    switch (event->key())
    {
        case Qt::Key_Escape:
            break;

        case Qt::Key_F1:
            break;

        case Qt::Key_F2:
            break;

        case Qt::Key_Up:
            break;

        case Qt::Key_Down:
            break;

        case Qt::Key_Left:
            break;

        case Qt::Key_Right:
            break;

        case Qt::Key_PageUp:
            break;

        case Qt::Key_PageDown:
            break;
    }
}

// *************************************************************************************
// timer signals


/**
  Updates the UAV position on the map. It is called every 200ms
  by a timer.

  TODO: consider updating upon object update, not timer.
  */
void OPMapGadgetWidget::updatePosition()
{
    if (!m_widget || !m_map)
        return;

    QMutexLocker locker(&m_map_mutex);
//Pip I'm sorry, I know this was here with a purpose vvv
    //if (!telemetry_connected)
      //  return;

    double latitude;
    double longitude;
    double altitude;

    // get current UAV position
	if (!getUAVPosition(latitude, longitude, altitude))
        return;

    // get current UAV heading
    float yaw = getUAV_Yaw();

    internals::PointLatLng uav_pos = internals::PointLatLng(latitude, longitude);	// current UAV position
    float uav_heading_degrees = yaw;                                                // current UAV heading
    float uav_altitude_meters = altitude;                                           // current UAV height
    float uav_ground_speed_meters_per_second = 0; //data.Groundspeed;               // current UAV ground speed

    // display the UAV lat/lon position
    QString str =
            "lat: " + QString::number(uav_pos.Lat(), 'f', 7) +
            " lon: " + QString::number(uav_pos.Lng(), 'f', 7) +
            " " + QString::number(uav_heading_degrees, 'f', 1) + "deg" +
            " " + QString::number(uav_altitude_meters, 'f', 1) + "m" +
            " " + QString::number(uav_ground_speed_meters_per_second, 'f', 1) + "m/s";
    m_widget->labelUAVPos->setText(str);

    m_map->UAV->SetUAVPos(uav_pos, uav_altitude_meters); // set the maps UAV position
   // qDebug()<<"UAVPOSITION"<<uav_pos.ToString();
    m_map->UAV->SetUAVHeading(uav_heading_degrees);      // set the maps UAV heading

	if (!getGPSPosition(latitude, longitude, altitude))
        return;

    uav_pos = internals::PointLatLng(latitude, longitude);	// current UAV position
    m_map->GPS->SetUAVPos(uav_pos, uav_altitude_meters); // set the maps UAV position
    m_map->GPS->SetUAVHeading(uav_heading_degrees);      // set the maps UAV heading
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

    if (!m_map->contentsRect().contains(p))
        return;					    // the mouse is not on the map

//    internals::PointLatLng lat_lon = m_map->currentMousePosition();     // fetch the current lat/lon mouse position

    QGraphicsItem *item = m_map->itemAt(p);

    // find out if we are over the home position
    mapcontrol::HomeItem *home = qgraphicsitem_cast<mapcontrol::HomeItem *>(item);

    // find out if we are over the UAV
    mapcontrol::UAVItem *uav = qgraphicsitem_cast<mapcontrol::UAVItem *>(item);

    // find out if we have a waypoint under the mouse cursor
    mapcontrol::WayPointItem *wp = qgraphicsitem_cast<mapcontrol::WayPointItem *>(item);

    if (mouse_lat_lon == lat_lon)
        return;                 // the mouse has not moved

    mouse_lat_lon = lat_lon;    // yes it has!

    internals::PointLatLng home_lat_lon = m_map->Home->Coord();

    QString s = QString::number(mouse_lat_lon.Lat(), 'f', 7) + "  " + QString::number(mouse_lat_lon.Lng(), 'f', 7);
    if (wp)
    {
        s += "  wp[" + QString::number(wp->Number()) + "]";

        double dist = distance(home_lat_lon, wp->Coord());
        double bear = bearing(home_lat_lon, wp->Coord());
        s += "  " + QString::number(dist * 1000, 'f', 1) + "m";
        s += "  " + QString::number(bear, 'f', 1) + "deg";
    }
    else
    if (home)
    {
        s += "  home";

        double dist = distance(home_lat_lon, mouse_lat_lon);
        double bear = bearing(home_lat_lon, mouse_lat_lon);
        s += "  " + QString::number(dist * 1000, 'f', 1) + "m";
        s += "  " + QString::number(bear, 'f', 1) + "deg";
    }
    else
    if (uav)
    {
        s += "  uav";

        double latitude;
        double longitude;
        double altitude;
		if (getUAVPosition(latitude, longitude, altitude))  // get current UAV position
        {
            internals::PointLatLng uav_pos = internals::PointLatLng(latitude, longitude);

//          double dist = distance(home_lat_lon, uav_pos);
//          double bear = bearing(home_lat_lon, uav_pos);
//          s += "  " + QString::number(dist * 1000, 'f', 1) + "m";
//          s += "  " + QString::number(bear, 'f', 1) + "deg";
        }
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

    if (i_zoom < min_zoom) i_zoom = min_zoom;
    else
    if (i_zoom > max_zoom) i_zoom = max_zoom;

    if (m_widget->horizontalSliderZoom->value() != i_zoom)
	m_widget->horizontalSliderZoom->setValue(i_zoom);	// set the GUI zoom slider position

    int index0_zoom = i_zoom - min_zoom;			// zoom level starting at index level '0'
    if (index0_zoom < zoomAct.count())
	zoomAct.at(index0_zoom)->setChecked(true);		// set the right-click context menu zoom level
}

void OPMapGadgetWidget::OnMapDrag()
{
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

//	if (prev_tile_number < number || m_widget->progressBarMap->maximum() < number)
//	    m_widget->progressBarMap->setMaximum(number);

	if (m_widget->progressBarMap->maximum() < number)
	    m_widget->progressBarMap->setMaximum(number);

	m_widget->progressBarMap->setValue(m_widget->progressBarMap->maximum() - number);	// update the progress bar

//	m_widget->labelNumTilesToLoad->setText(QString::number(number));

	prev_tile_number = number;
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

void OPMapGadgetWidget::OnMapZoomChanged()
{
}

void OPMapGadgetWidget::OnMapTypeChanged(MapType::Types type)
{
    Q_UNUSED(type);
}

void OPMapGadgetWidget::OnEmptyTileError(int zoom, core::Point pos)
{
    Q_UNUSED(zoom);
    Q_UNUSED(pos);
}

void OPMapGadgetWidget::WPNumberChanged(int const &oldnumber, int const &newnumber, WayPointItem *waypoint)
{
    Q_UNUSED(oldnumber);
    Q_UNUSED(newnumber);
    Q_UNUSED(waypoint);
}

void OPMapGadgetWidget::WPValuesChanged(WayPointItem *waypoint)
{
//    qDebug("opmap: WPValuesChanged");

    switch (m_map_mode)
    {
        case Normal_MapMode:
            m_waypoint_list_mutex.lock();
            foreach (t_waypoint *wp, m_waypoint_list)
            {   // search for the waypoint in our own waypoint list and update it
                if (!wp) continue;
                if (!wp->map_wp_item) continue;
                if (wp->map_wp_item != waypoint) continue;
                // found the waypoint in our list
                wp->coord = waypoint->Coord();
                wp->altitude = waypoint->Altitude();
                wp->description = waypoint->Description();
                break;
            }
            m_waypoint_list_mutex.unlock();
            break;

        case MagicWaypoint_MapMode:
            // update our copy of the magic waypoint
            if (magic_waypoint.map_wp_item && magic_waypoint.map_wp_item == waypoint)
            {
                magic_waypoint.coord = waypoint->Coord();
                magic_waypoint.altitude = waypoint->Altitude();
                magic_waypoint.description = waypoint->Description();

                // move the UAV to the magic waypoint position
                // moveToMagicWaypointPosition();
            }
            break;
    }

}

/**
  TODO: slot to do something upon Waypoint insertion
  */
void OPMapGadgetWidget::WPInserted(int const &number, WayPointItem *waypoint)
{
    Q_UNUSED(number);
    Q_UNUSED(waypoint);
}

/**
  TODO: slot to do something upon Waypoint deletion
  */
void OPMapGadgetWidget::WPDeleted(int const &number)
{
    Q_UNUSED(number);
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
    telemetry_connected = true;

	if (!obum) return;

	bool set;
	double LLA[3];

	// ***********************
	// fetch the home location

	if (obum->getHomeLocation(set, LLA) < 0)
		return;	// error

	setHome(internals::PointLatLng(LLA[0], LLA[1]));

    if (m_map)
        m_map->SetCurrentPosition(home_position.coord);         // set the map position

    // ***********************
}

void OPMapGadgetWidget::onTelemetryDisconnect()
{
    telemetry_connected = false;
}

// Updates the Home position icon whenever the HomePosition object is updated
void OPMapGadgetWidget::homePositionUpdated(UAVObject *hp)
{
    if (!hp)
        return;

    double lat = hp->getField("Latitude")->getDouble() * 1e-7;
    double lon = hp->getField("Longitude")->getDouble() * 1e-7;
    setHome(internals::PointLatLng(lat, lon));
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

    setHome(internals::PointLatLng(latitude, longitude));
}

/**
  Sets the home position on the map widget
  */
void OPMapGadgetWidget::setHome(internals::PointLatLng pos_lat_lon)
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

    home_position.coord = internals::PointLatLng(latitude, longitude);

    m_map->Home->SetCoord(home_position.coord);
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

    internals::PointLatLng home_pos = home_position.coord;	// get the home location
    m_map->SetCurrentPosition(home_pos);                    // center the map onto the home location
}


void OPMapGadgetWidget::zoomIn()
{
    if (!m_widget || !m_map)
        return;

    int zoom = m_map->ZoomTotal() + 1;

    if (zoom < min_zoom) zoom = min_zoom;
    else
    if (zoom > max_zoom) zoom = max_zoom;

    m_map->SetZoom(zoom);
}

void OPMapGadgetWidget::zoomOut()
{
    if (!m_widget || !m_map)
        return;

    int zoom = m_map->ZoomTotal() - 1;

    if (zoom < min_zoom) zoom = min_zoom;
    else
    if (zoom > max_zoom) zoom = max_zoom;

    m_map->SetZoom(zoom);
}

void OPMapGadgetWidget::setZoom(int zoom)
{
    if (!m_widget || !m_map)
        return;

    if (zoom < min_zoom) zoom = min_zoom;
    else
    if (zoom > max_zoom) zoom = max_zoom;

    internals::MouseWheelZoomType::Types zoom_type = m_map->GetMouseWheelZoomType();
    m_map->SetMouseWheelZoomType(internals::MouseWheelZoomType::ViewCenter);

    m_map->SetZoom(zoom);

    m_map->SetMouseWheelZoomType(zoom_type);
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

//    #if defined(Q_WS_WIN)
//	if (!cacheLocation.endsWith('\\')) cacheLocation += '\\';
//    #elif defined(Q_WS_X11)
	if (!cacheLocation.endsWith(QDir::separator())) cacheLocation += QDir::separator();
//    #elif defined(Q_WS_MAC)
//	if (!cacheLocation.endsWith(QDir::separator())) cacheLocation += QDir::separator();
//    #endif

    QDir dir;
    if (!dir.exists(cacheLocation))
        if (!dir.mkpath(cacheLocation))
            return;

//    qDebug() << "opmap: map cache dir: " << cacheLocation;

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

            // delete the magic waypoint from the map
            if (magic_waypoint.map_wp_item)
            {
                magic_waypoint.coord = magic_waypoint.map_wp_item->Coord();
                magic_waypoint.altitude = magic_waypoint.map_wp_item->Altitude();
                magic_waypoint.description = magic_waypoint.map_wp_item->Description();
                magic_waypoint.map_wp_item = NULL;
            }
            m_map->WPDeleteAll();

            // restore the normal waypoints on the map
            m_waypoint_list_mutex.lock();
            foreach (t_waypoint *wp, m_waypoint_list)
            {
                if (!wp) continue;
                wp->map_wp_item = m_map->WPCreate(wp->coord, wp->altitude, wp->description);
                if (!wp->map_wp_item) continue;
                wp->map_wp_item->setZValue(10 + wp->map_wp_item->Number());
                wp->map_wp_item->setFlag(QGraphicsItem::ItemIsMovable, !wp->locked);
                if (!wp->locked)
                    wp->map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker1.png"));
                else
                    wp->map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker2.png"));
                wp->map_wp_item->update();
            }
            m_waypoint_list_mutex.unlock();

            break;

        case MagicWaypoint_MapMode:
            m_map_mode = MagicWaypoint_MapMode;

            m_widget->toolButtonNormalMapMode->setChecked(false);
            m_widget->toolButtonMagicWaypointMapMode->setChecked(true);

            showMagicWaypointControls();

            // delete the normal waypoints from the map
            m_waypoint_list_mutex.lock();
            foreach (t_waypoint *wp, m_waypoint_list)
            {
                if (!wp) continue;
                if (!wp->map_wp_item) continue;
                wp->coord = wp->map_wp_item->Coord();
                wp->altitude = wp->map_wp_item->Altitude();
                wp->description = wp->map_wp_item->Description();
                wp->locked = (wp->map_wp_item->flags() & QGraphicsItem::ItemIsMovable) == 0;
                wp->map_wp_item = NULL;
            }
            m_map->WPDeleteAll();
            m_waypoint_list_mutex.unlock();

            // restore the magic waypoint on the map
            magic_waypoint.map_wp_item = m_map->WPCreate(magic_waypoint.coord, magic_waypoint.altitude, magic_waypoint.description);
            magic_waypoint.map_wp_item->setZValue(10 + magic_waypoint.map_wp_item->Number());
            magic_waypoint.map_wp_item->SetShowNumber(false);
            magic_waypoint.map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker3.png"));

            break;
    }
}

// *************************************************************************************
// Context menu stuff

void OPMapGadgetWidget::createActions()
{
    if (!m_widget)
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

    copyMouseLatLonToClipAct = new QAction(tr("Mouse latitude and longitude"), this);
    copyMouseLatLonToClipAct->setStatusTip(tr("Copy the mouse latitude and longitude to the clipboard"));
    connect(copyMouseLatLonToClipAct, SIGNAL(triggered()), this, SLOT(onCopyMouseLatLonToClipAct_triggered()));

    copyMouseLatToClipAct = new QAction(tr("Mouse latitude"), this);
    copyMouseLatToClipAct->setStatusTip(tr("Copy the mouse latitude to the clipboard"));
    connect(copyMouseLatToClipAct, SIGNAL(triggered()), this, SLOT(onCopyMouseLatToClipAct_triggered()));

    copyMouseLonToClipAct = new QAction(tr("Mouse longitude"), this);
    copyMouseLonToClipAct->setStatusTip(tr("Copy the mouse longitude to the clipboard"));
    connect(copyMouseLonToClipAct, SIGNAL(triggered()), this, SLOT(onCopyMouseLonToClipAct_triggered()));

    /*
    findPlaceAct = new QAction(tr("&Find place"), this);
    findPlaceAct->setShortcut(tr("Ctrl+F"));
    findPlaceAct->setStatusTip(tr("Find a location"));
    connect(findPlaceAct, SIGNAL(triggered()), this, SLOT(onFindPlaceAct_triggered()));
    */

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

    zoomInAct = new QAction(tr("Zoom &In"), this);
    zoomInAct->setShortcut(Qt::Key_PageUp);
    zoomInAct->setStatusTip(tr("Zoom the map in"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(onGoZoomInAct_triggered()));

    zoomOutAct = new QAction(tr("Zoom &Out"), this);
    zoomOutAct->setShortcut(Qt::Key_PageDown);
    zoomOutAct->setStatusTip(tr("Zoom the map out"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(onGoZoomOutAct_triggered()));

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

    /*
    wayPointEditorAct = new QAction(tr("&Waypoint editor"), this);
    wayPointEditorAct->setShortcut(tr("Ctrl+W"));
    wayPointEditorAct->setStatusTip(tr("Open the waypoint editor"));
    wayPointEditorAct->setEnabled(false);   // temporary
    connect(wayPointEditorAct, SIGNAL(triggered()), this, SLOT(onOpenWayPointEditorAct_triggered()));

    addWayPointAct = new QAction(tr("&Add waypoint"), this);
    addWayPointAct->setShortcut(tr("Ctrl+A"));
    addWayPointAct->setStatusTip(tr("Add waypoint"));
    connect(addWayPointAct, SIGNAL(triggered()), this, SLOT(onAddWayPointAct_triggered()));

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
    */

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
    for (int i = min_zoom; i <= max_zoom; i++)
    {
        QAction *zoom_act = new QAction(QString::number(i), zoomActGroup);
        zoom_act->setCheckable(true);
        zoom_act->setData(i);
        zoomAct.append(zoom_act);
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
    for (int i = 0; i < (int)(sizeof(safe_area_radius_list) / sizeof(safe_area_radius_list[0])); i++)
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
    for (int i = 0; i < (int)(sizeof(uav_trail_time_list) / sizeof(uav_trail_time_list[0])); i++)
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
    for (int i = 0; i < (int)(sizeof(uav_trail_distance_list) / sizeof(uav_trail_distance_list[0])); i++)
    {
        int uav_trail_distance = uav_trail_distance_list[i];
        QAction *uavTrailDistance_act = new QAction(QString::number(uav_trail_distance) + " meters", uavTrailDistanceActGroup);
        uavTrailDistance_act->setCheckable(true);
        uavTrailDistance_act->setChecked(uav_trail_distance == m_map->UAV->TrailDistance());
        uavTrailDistance_act->setData(uav_trail_distance);
        uavTrailDistanceAct.append(uavTrailDistance_act);
    }

    // *****

    // ***********************
}

void OPMapGadgetWidget::onReloadAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    m_map->ReloadMap();
}

void OPMapGadgetWidget::onCopyMouseLatLonToClipAct_triggered()
{
//    QClipboard *clipboard = qApp->clipboard();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(context_menu_lat_lon.Lat(), 'f', 7) + ", " + QString::number(context_menu_lat_lon.Lng(), 'f', 7), QClipboard::Clipboard);
}

void OPMapGadgetWidget::onCopyMouseLatToClipAct_triggered()
{
//    QClipboard *clipboard = qApp->clipboard();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(context_menu_lat_lon.Lat(), 'f', 7), QClipboard::Clipboard);
}

void OPMapGadgetWidget::onCopyMouseLonToClipAct_triggered()
{
//    QClipboard *clipboard = qApp->clipboard();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(context_menu_lat_lon.Lng(), 'f', 7), QClipboard::Clipboard);
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
    m_map->GPS->setVisible(show);
}

void OPMapGadgetWidget::onShowTrailAct_toggled(bool show)
{
    if (!m_widget || !m_map)
        return;

    m_map->UAV->SetShowTrail(show);
    m_map->GPS->SetShowTrail(show);
}

void OPMapGadgetWidget::onShowTrailLineAct_toggled(bool show)
{
    if (!m_widget || !m_map)
        return;

    m_map->UAV->SetShowTrailLine(show);
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
    if (!m_widget || !action)
        return;

    setZoom(action->data().toInt());
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

    setHome(context_menu_lat_lon);

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
    if (!m_widget || !m_map)
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
    m_map->GPS->DeleteTrail();
}

void OPMapGadgetWidget::onUAVTrailTimeActGroup_triggered(QAction *action)
{
    if (!m_widget || !m_map)
        return;

    int trail_time = (double)action->data().toInt();

    m_map->UAV->SetTrailTime(trail_time);
}

void OPMapGadgetWidget::onUAVTrailDistanceActGroup_triggered(QAction *action)
{
    if (!m_widget || !m_map)
        return;

    int trail_distance = action->data().toInt();

    m_map->UAV->SetTrailDistance(trail_distance);
}

/**
  * TODO: unused for v1.0
  **/
/*
void OPMapGadgetWidget::onAddWayPointAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    m_waypoint_list_mutex.lock();

	// create a waypoint on the map at the last known mouse position
    t_waypoint *wp = new t_waypoint;
    wp->map_wp_item = NULL;
    wp->coord = context_menu_lat_lon;
    wp->altitude = 0;
    wp->description = "";
    wp->locked = false;
    wp->time_seconds = 0;
    wp->hold_time_seconds = 0;
    wp->map_wp_item = m_map->WPCreate(wp->coord, wp->altitude, wp->description);

    wp->map_wp_item->setZValue(10 + wp->map_wp_item->Number());

    wp->map_wp_item->setFlag(QGraphicsItem::ItemIsMovable, !wp->locked);

    if (wp->map_wp_item)
    {
        if (!wp->locked)
            wp->map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker1.png"));
        else
            wp->map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker2.png"));
        wp->map_wp_item->update();
    }

    // and remember it in our own local waypoint list
    m_waypoint_list.append(wp);

    m_waypoint_list_mutex.unlock();
}
*/

/**
  * Called when the user asks to edit a waypoint from the map
  *
  * TODO: should open an interface to edit waypoint properties, or
  *       propagate the signal to a specific WP plugin (tbd).
  **/
/*
void OPMapGadgetWidget::onEditWayPointAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    if (!m_mouse_waypoint)
        return;

    //waypoint_edit_dialog.editWaypoint(m_mouse_waypoint);

    m_mouse_waypoint = NULL;
}
*/

/**
  * TODO: unused for v1.0
  */
/*
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
*/

/**
  * TODO: unused for v1.0
  */
/*
void OPMapGadgetWidget::onDeleteWayPointAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    if (!m_mouse_waypoint)
        return;

    bool locked = (m_mouse_waypoint->flags() & QGraphicsItem::ItemIsMovable) == 0;

    if (locked) return;	// waypoint is locked

    QMutexLocker locker(&m_waypoint_list_mutex);

    for (int i = 0; i < m_waypoint_list.count(); i++)
    {
        t_waypoint *wp = m_waypoint_list.at(i);
        if (!wp) continue;
        if (!wp->map_wp_item || wp->map_wp_item != m_mouse_waypoint) continue;

        // delete the waypoint from the map
        m_map->WPDelete(wp->map_wp_item);

        // delete the waypoint from our local waypoint list
        m_waypoint_list.removeAt(i);

        delete wp;

        break;
    }
//
//    foreach (t_waypoint *wp, m_waypoint_list)
//    {
//        if (!wp) continue;
//        if (!wp->map_wp_item || wp->map_wp_item != m_mouse_waypoint) continue;
//
//	    // delete the waypoint from the map
//      m_map->WPDelete(wp->map_wp_item);
//
//	    // delete the waypoint from our local waypoint list
//        m_waypoint_list.removeOne(wp);
//
//        delete wp;
//
//	    break;
//	}

    m_mouse_waypoint = NULL;
}
*/

/**
  * TODO: No Waypoint support in v1.0
  */
/*
void OPMapGadgetWidget::onClearWayPointsAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    QMutexLocker locker(&m_waypoint_list_mutex);

	m_map->WPDeleteAll();

    foreach (t_waypoint *wp, m_waypoint_list)
    {
        if (wp)
        {
            delete wp;
            wp = NULL;
        }
    }

    m_waypoint_list.clear();
}
*/

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
    m_map->Home->RefreshPos();
}

void OPMapGadgetWidget::onSafeAreaActGroup_triggered(QAction *action)
{
    if (!m_widget || !m_map)
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

    magic_waypoint.coord = home_position.coord;

    if (magic_waypoint.map_wp_item)
        magic_waypoint.map_wp_item->SetCoord(magic_waypoint.coord);
}

// *************************************************************************************
// move the UAV to the magic waypoint position

void OPMapGadgetWidget::moveToMagicWaypointPosition()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != MagicWaypoint_MapMode)
        return;

//    internals::PointLatLng coord = magic_waypoint.coord;
//    double altitude = magic_waypoint.altitude;


    // ToDo:

}

// *************************************************************************************
// temporary until an object is created for managing the save/restore

// load the contents of a simple text file into a combobox
void OPMapGadgetWidget::loadComboBoxLines(QComboBox *comboBox, QString filename)
{
    if (!comboBox) return;
    if (filename.isNull() || filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine().simplified();
        if (line.isNull() || line.isEmpty()) continue;
        comboBox->addItem(line);
    }

    file.close();
}

// save a combobox text contents to a simple text file
void OPMapGadgetWidget::saveComboBoxLines(QComboBox *comboBox, QString filename)
{
    if (!comboBox) return;
    if (filename.isNull() || filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    for (int i = 0; i < comboBox->count(); i++)
    {
        QString line = comboBox->itemText(i).simplified();
        if (line.isNull() || line.isEmpty()) continue;
        out << line << "\n";
    }

    file.close();
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
    double dist = distance(home_position.coord, magic_waypoint.coord);
    double bear = bearing(home_position.coord, magic_waypoint.coord);

    // get the maximum safe distance - in kilometers
    double boundry_dist = (double)m_map->Home->SafeArea() / 1000;

//    if (dist <= boundry_dist)
//        return; // the magic waypoint is still within the safe area, don't move it

    if (dist > boundry_dist) dist = boundry_dist;

    // move the magic waypoint

    magic_waypoint.coord = destPoint(home_position.coord, bear, dist);

    if (m_map_mode == MagicWaypoint_MapMode)
    {   // move the on-screen waypoint
        if (magic_waypoint.map_wp_item)
            magic_waypoint.map_wp_item->SetCoord(magic_waypoint.coord);
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

    // ***********************
    // Haversine formula
/*
    double delta_lat = lat2 - lat1;
    double delta_lon = lon2 - lon1;

    double t1 = sin(delta_lat / 2);
    double t2 = sin(delta_lon / 2);
    double a = (t1 * t1) + cos(lat1) * cos(lat2) * (t2 * t2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return (earth_mean_radius * c);
*/
    // ***********************
    // Spherical Law of Cosines

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
    double BaseECEF[3];
    double NED[3];
    double LLA[3];
    UAVObject *obj;

	if (!obm)
		return false;

	obj = dynamic_cast<UAVDataObject*>(obm->getObject(QString("HomeLocation")));
    if (!obj) return false;
    BaseECEF[0] = obj->getField(QString("ECEF"))->getDouble(0) / 100;
    BaseECEF[1] = obj->getField(QString("ECEF"))->getDouble(1) / 100;
    BaseECEF[2] = obj->getField(QString("ECEF"))->getDouble(2) / 100;

	obj = dynamic_cast<UAVDataObject*>(obm->getObject(QString("PositionActual")));
    if (!obj) return false;
    NED[0] = obj->getField(QString("North"))->getDouble() / 100;
    NED[1] = obj->getField(QString("East"))->getDouble() / 100;
    NED[2] = obj->getField(QString("Down"))->getDouble() / 100;

//    obj = dynamic_cast<UAVDataObject*>(om->getObject(QString("PositionDesired")));

//    obj = dynamic_cast<UAVDataObject*>(objManager->getObject("VelocityActual"));      // air speed

    Utils::CoordinateConversions().GetLLA(BaseECEF, NED, LLA);

    latitude = LLA[0];
    longitude = LLA[1];
    altitude = LLA[2];

    if (latitude != latitude) latitude = 0; // nan detection
//    if (isNan(latitude)) latitude = 0; // nan detection
    else
//    if (!isFinite(latitude)) latitude = 0;
//    else
    if (latitude >  90) latitude =  90;
    else
    if (latitude < -90) latitude = -90;

    if (longitude != longitude) longitude = 0; // nan detection
    else
//    if (longitude > std::numeric_limits<double>::max()) longitude = 0;  // +infinite
//    else
//    if (longitude < -std::numeric_limits<double>::max()) longitude = 0;  // -infinite
//    else
    if (longitude >  180) longitude =  180;
    else
    if (longitude < -180) longitude = -180;

    if (altitude != altitude) altitude = 0; // nan detection

    return true;
}

// *************************************************************************************

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

	double LLA[3] = {home_position.coord.Lat(), home_position.coord.Lng(), home_position.altitude};
	return (obum->setHomeLocation(LLA, true) >= 0);
}

// *************************************************************************************

void OPMapGadgetWidget::SetUavPic(QString UAVPic)
{
    m_map->SetUavPic(UAVPic);
}
