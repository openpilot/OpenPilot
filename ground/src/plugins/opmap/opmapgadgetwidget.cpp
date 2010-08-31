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

// *************************************************************************************

// constructor
OPMapGadgetWidget::OPMapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    // **************

    m_widget = NULL;
    m_map = NULL;
    findPlaceCompleter = NULL;
    m_overlay_widget = NULL;

    m_map_graphics_scene = NULL;
    m_map_scene_proxy = NULL;
    m_zoom_slider_widget = NULL;
    m_statusbar_widget = NULL;


    m_plugin_manager = NULL;
    m_objManager = NULL;

    m_mouse_waypoint = NULL;

    prev_tile_number = 0;

    // **************
    // fetch required UAVObjects

    m_plugin_manager = ExtensionSystem::PluginManager::instance();
    m_objManager = m_plugin_manager->getObject<UAVObjectManager>();

    // **************
    // create the widget that holds the user controls and the map

    m_widget = new Ui::OPMap_Widget();
    m_widget->setupUi(this);

    // **************
    // create the central map widget

    m_map = new mapcontrol::OPMapWidget();

    m_map->setFrameStyle(QFrame::NoFrame);							    // no border frame
    m_map->setBackgroundBrush(Qt::black);							    // black background

    m_map->configuration->DragButton = Qt::LeftButton;						    // use the left mouse button for map dragging
    m_map->SetMinZoom(2);
   // m_map->SetMaxZoom(19);
    m_map->SetMouseWheelZoomType(internals::MouseWheelZoomType::MousePositionWithoutCenter);	    // set how the mouse wheel zoom functions
    m_map->SetFollowMouse(true);								    // we want a contiuous mouse position reading
    m_map->SetShowHome(true);									    // display the HOME position on the map
    m_map->SetShowUAV(true);									    // display the UAV position on the map

//  m_map->UAV->SetTrailTime(1);								    // seconds
//  m_map->UAV->SetTrailDistance(0.1);								    // kilometers

    m_map->UAV->SetTrailType(UAVTrailType::ByTimeElapsed);
//  m_map->UAV->SetTrailType(UAVTrailType::ByDistance);

    // **************

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_map);
    m_widget->mapWidget->setLayout(layout);

    // **************
    // create the user controls overlayed onto the map

    // doing this makes the map VERY slow :(

/*
    m_zoom_slider_widget = new opmap_zoom_slider_widget();
    m_statusbar_widget = new opmap_statusbar_widget();

    m_map_graphics_scene = m_map->scene();

    m_map_scene_proxy = m_map_graphics_scene->addWidget(m_zoom_slider_widget);
    m_map_scene_proxy = m_map_graphics_scene->addWidget(m_statusbar_widget);

    m_zoom_slider_widget->move(m_map->width() - 20 - m_zoom_slider_widget->width(), 20);
    m_statusbar_widget->move(0, m_map->height() - m_statusbar_widget->height());
*/
/*
    m_overlay_widget = new opmap_overlay_widget(m_map);
    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->setSpacing(0);
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->addWidget(m_overlay_widget);
    m_map->setLayout(layout2);
*/
    // **************
    // set the user control options

    m_widget->labelUAVPos->setText("---");
    m_widget->labelMapPos->setText("---");
    m_widget->labelMousePos->setText("---");

    m_widget->splitter->setCollapsible(1, false);

    // set the size of the collapsable widgets
    QList<int> m_SizeList;
//    m_SizeList << m_widget->splitter->sizes();
    m_SizeList << 0 << 0 << 0;
    m_widget->splitter->setSizes(m_SizeList);

    m_widget->progressBarMap->setMaximum(1);

    m_widget->toolButtonShowUAVtrail->setChecked(true);

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
    // add an auto-completer to the find-place line edit box
/*
    findPlaceWordList << "england" << "london" << "birmingham" << "shropshire";
    QCompleter *findPlaceCompleter = new QCompleter(findPlaceWordList, this);
    findPlaceCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    findPlaceCompleter->setCompletionMode(QCompleter::PopupCompletion);
    findPlaceCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    m_widget->comboBoxFindPlace->setCompleter(findPlaceCompleter);
*/
    m_widget->comboBoxFindPlace->setAutoCompletion(true);

    connect( m_widget->comboBoxFindPlace->lineEdit(), SIGNAL(returnPressed()), this, SLOT(comboBoxFindPlace_returnPressed()));

    // **************
    // init the waypoint tree (shown on the left on the map plugin GUI)

    m_widget->treeViewWaypoints->setModel(&wayPoint_treeView_model);

/*
    // test
//    wayPoint_treeView_model = new QStandardItemModel(5, 2);
    for (int r = 0; r < 5; r++)
    {
	for (int c = 0; c < 2; c++)
	{
	    QStandardItem *item = new QStandardItem(QString("Row:%0, Column:%1").arg(r).arg(c));

	    if (c == 0)
    	    {
		for (int i = 0; i < 3; i++)
		{
		    QStandardItem *child = new QStandardItem(QString("Item %0").arg(i));
		    child->setEditable(false);
		    item->appendRow(child);
		}
	    }

	    wayPoint_treeView_model->setItem(r, c, item);
	}
    }
    wayPoint_treeView_model->setHorizontalHeaderItem(0, new QStandardItem("Foo"));
    wayPoint_treeView_model->setHorizontalHeaderItem(1, new QStandardItem("Bar-Baz"));

//    m_widget->treeViewWaypoints->setModel(wayPoint_treeView_model);
*/





    // test only
/*
    // create a waypoint group
    QStandardItem *item = new QStandardItem(tr("Camera shoot at the town hall"));
    // add some waypoints
    {
	QStandardItem *child = new QStandardItem(QIcon(QString::fromUtf8(":/opmap/images/waypoint.png")), "North side window view");
	child->setEditable(true);
	item->appendRow(child);
    }
    {
	QStandardItem *child = new QStandardItem(QIcon(QString::fromUtf8(":/opmap/images/waypoint.png")), "East side window view");
	child->setEditable(true);
	item->appendRow(child);
    }
    {
	QStandardItem *child = new QStandardItem(QIcon(QString::fromUtf8(":/opmap/images/waypoint.png")), "South side window view");
	child->setEditable(true);
	item->appendRow(child);
    }
    {
	QStandardItem *child = new QStandardItem(QIcon(QString::fromUtf8(":/opmap/images/waypoint.png")), "West side window view");
	child->setEditable(true);
	item->appendRow(child);
    }
    wayPoint_treeView_model.appendRow(item);
*/
    // create another waypoint group
    QStandardItem *item = new QStandardItem(tr("Flight path 62"));
    for (int i = 1; i < 8; i++)
    {   // add some waypoints
	QStandardItem *child = new QStandardItem(QIcon(QString::fromUtf8(":/opmap/images/waypoint.png")), QString("Waypoint %0").arg(i));
	child->setEditable(true);
	item->appendRow(child);
    }
    wayPoint_treeView_model.appendRow(item);










    // **************
    // map stuff

    connect(m_map, SIGNAL(zoomChanged(double)), this, SLOT(zoomChanged(double)));					// map zoom change signals
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

    QPointF pos = getLatLon();
    m_map->SetCurrentPosition(internals::PointLatLng(pos.x(), pos.y()));	    // set the default map position

    // **************
    // create various context menu (mouse right click menu) actions

    createActions();

    // **************
    // create the desired timers

    // Pip .. I don't like polling, I prefer an efficient event driven system (signal/slot) but this will do for now

    m_updateTimer = new QTimer();
    m_updateTimer->setInterval(200);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));
    m_updateTimer->start();

    m_statusUpdateTimer = new QTimer();
    m_statusUpdateTimer->setInterval(200);
    connect(m_statusUpdateTimer, SIGNAL(timeout()), this, SLOT(updateMousePos()));
    m_statusUpdateTimer->start();

    // **************
}

// destructor
OPMapGadgetWidget::~OPMapGadgetWidget()
{


    // this destructor doesn't appear to be called at shutdown???




//    #if defined(Q_OS_MAC)
//    #elif defined(Q_OS_WIN)
//	saveComboBoxLines(m_widget->comboBoxFindPlace, QCoreApplication::applicationDirPath() + "/opmap_find_place_history.txt");
//    #else
//    #endif

    onClearWayPointsAct_triggered();

    if (m_zoom_slider_widget) delete m_zoom_slider_widget;
    if (m_statusbar_widget) delete m_statusbar_widget;
    if (m_map) delete m_map;
    if (m_widget) delete m_widget;
}

// *************************************************************************************
// widget signals

void OPMapGadgetWidget::resizeEvent(QResizeEvent *event)
{
//    update();
    QWidget::resizeEvent(event);
}

void OPMapGadgetWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_map)
    {
//	mouse_lat_lon = m_map->currentMousePosition();			    // fetch the current mouse lat/longitude position
//	if (mouse_lat_lon != lat_lon)
//	{	// the mouse has moved
//	    mouse_lat_lon = lat_lon;
//
//	    QString coord_str = " " + QString::number(mouse_lat_lon.Lat(), 'f', 7) + "   " + QString::number(mouse_lat_lon.Lng(), 'f', 7) + " ";
//
//	    statusLabel.setText(coord_str);
//	    widget->labelStatus->setText(coord_str);
//	}
    }

    if (event->buttons() & Qt::LeftButton)
    {
	 QPoint pos = event->pos();
    }

    QWidget::mouseMoveEvent(event);
}

void OPMapGadgetWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (event->reason() != QContextMenuEvent::Mouse)
	return;	// not a mouse click event

    if (!m_map)
	return;	// we don't appear to have a map to work with!

    // current mouse position
    QPoint p = m_map->mapFromGlobal(QCursor::pos());

    // save the current lat/lon mouse position
    mouse_lat_lon = m_map->currentMousePosition();

    if (!m_map->contentsRect().contains(p))
	return;					    // the mouse click was not on the map

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

    menu.addAction(closeAct);

    menu.addSeparator();

    menu.addAction(reloadAct);

    menu.addSeparator();

    QMenu copySubMenu(tr("Copy"), this);
    copySubMenu.addAction(copyMouseLatLonToClipAct);
    copySubMenu.addAction(copyMouseLatToClipAct);
    copySubMenu.addAction(copyMouseLonToClipAct);
    menu.addMenu(&copySubMenu);

    menu.addSeparator();

    menu.addAction(findPlaceAct);

    menu.addSeparator();

    menu.addAction(showCompassAct);

    menu.addSeparator()->setText(tr("Zoom"));

    menu.addAction(zoomInAct);
    menu.addAction(zoomOutAct);

    QMenu zoomSubMenu(tr("&Zoom ") + "(" + QString::number(m_map->Zoom()) + ")", this);
    for (int i = 0; i < zoomAct.count(); i++)
	zoomSubMenu.addAction(zoomAct.at(i));
    menu.addMenu(&zoomSubMenu);

    menu.addSeparator();

    menu.addAction(goMouseClickAct);

    menu.addSeparator()->setText(tr("HOME"));

    menu.addAction(setHomePosAct);
    menu.addAction(showHomeAct);
    menu.addAction(goHomeAct);

    menu.addSeparator()->setText(tr("UAV"));

    menu.addAction(showUAVAct);
    menu.addAction(showUAVtrailAct);
    menu.addAction(followUAVpositionAct);
    menu.addAction(followUAVheadingAct);
    menu.addAction(goUAVAct);

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

//    menu.addSeparator();

    menu.exec(event->globalPos());  // popup the menu

    // ****************
}

void OPMapGadgetWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) // ESC
    {
    }
    else
    if (event->key() == Qt::Key_F1) // F1
    {
    }
    else
    if (event->key() == Qt::Key_F2) // F2
    {
    }
    else
    if (event->key() == Qt::Key_Up)
    {
    }
    else
    if (event->key() == Qt::Key_Down)
    {
    }
    else
    if (event->key() == Qt::Key_Left)
    {
    }
    else
    if (event->key() == Qt::Key_Right)
    {
    }
    else
    if (event->key() == Qt::Key_PageUp)
    {
    }
    else
    if (event->key() == Qt::Key_PageDown)
    {
    }
    else
    {
	qDebug() << event->key() << endl;
    }
}

// *************************************************************************************
// timer signals

void OPMapGadgetWidget::updatePosition()
{
    QPointF pos = getLatLon();

    internals::PointLatLng uav_pos = internals::PointLatLng(pos.x(), pos.y());	// current UAV position
    float uav_heading = 0; //data.Heading;								// current UAV heading
    float uav_altitude_meters = 0; //data.Altitude;							// current UAV height
    float uav_ground_speed_meters_per_second = 0; //data.Groundspeed;				// current UAV ground speed

    // display the UAV lat/lon position
    QString str =   " lat: " + QString::number(uav_pos.Lat(), 'f', 7) +
		    "   lon: " + QString::number(uav_pos.Lng(), 'f', 7) +
		    "   " + QString::number(uav_heading, 'f', 1) + "deg" +
		    "   " + QString::number(uav_altitude_meters, 'f', 1) + "m" +
		    "   " + QString::number(uav_ground_speed_meters_per_second, 'f', 1) + "m/s";
    if (m_widget) m_widget->labelUAVPos->setText(str);

    if (m_map)
    {
	m_map->UAV->SetUAVPos(uav_pos, uav_altitude_meters);					// set the maps UAV position
	m_map->UAV->SetUAVHeading(uav_heading);							// set the maps UAV heading
    }
}

void OPMapGadgetWidget::updateMousePos()
{
    internals::PointLatLng lat_lon = m_map->currentMousePosition();				// fetch the current lat/lon mouse position

    if (mouse_lat_lon != lat_lon)
    {	// the mouse has moved
	mouse_lat_lon = lat_lon;

	QString str = " " + QString::number(mouse_lat_lon.Lat(), 'f', 7) + "   " + QString::number(mouse_lat_lon.Lng(), 'f', 7);
	if (m_widget) m_widget->labelMousePos->setText(str);
    }
}

// *************************************************************************************
// map signals

void OPMapGadgetWidget::zoomChanged(double zoom)
{
    int i_zoom = (int)(zoom + 0.5);
    if (i_zoom < 2 || i_zoom > 19) return;

    if (m_widget)
    {
//	m_widget->labelZoom->setText(" " + QString::number(zoom));
	if (m_widget->horizontalSliderZoom->value() != i_zoom)
	    m_widget->horizontalSliderZoom->setValue(i_zoom);
    }

    if (i_zoom - 2 < zoomAct.count())
	zoomAct.at(i_zoom - 2)->setChecked(true);
}

void OPMapGadgetWidget::OnMapDrag()
{
}

void OPMapGadgetWidget::OnCurrentPositionChanged(internals::PointLatLng point)
{
    if (m_widget)
    {
	QString coord_str = " " + QString::number(point.Lat(), 'f', 7) + "   " + QString::number(point.Lng(), 'f', 7) + " ";
	m_widget->labelMapPos->setText(coord_str);
    }
}

void OPMapGadgetWidget::OnTilesStillToLoad(int number)
{
    if (m_widget)
    {
//	if (prev_tile_number < number || m_widget->progressBarMap->maximum() < number)
//	    m_widget->progressBarMap->setMaximum(number);

	if (m_widget->progressBarMap->maximum() < number)
	    m_widget->progressBarMap->setMaximum(number);

	m_widget->progressBarMap->setValue(m_widget->progressBarMap->maximum() - number);	// update the progress bar

//	m_widget->labelNumTilesToLoad->setText(" " + QString::number(number));

	prev_tile_number = number;
    }
}

void OPMapGadgetWidget::OnTileLoadStart()
{
    if (m_widget)
	m_widget->progressBarMap->setVisible(true);
}

void OPMapGadgetWidget::OnTileLoadComplete()
{
    if (m_widget)
	m_widget->progressBarMap->setVisible(false);
}

void OPMapGadgetWidget::OnMapZoomChanged()
{
    // to do
}

void OPMapGadgetWidget::OnMapTypeChanged(MapType::Types type)
{
    // to do
}

void OPMapGadgetWidget::OnEmptyTileError(int zoom, core::Point pos)
{
    // to do
}

void OPMapGadgetWidget::WPNumberChanged(int const &oldnumber, int const &newnumber, WayPointItem *waypoint)
{
    // to do
}

void OPMapGadgetWidget::WPValuesChanged(WayPointItem *waypoint)
{
    // to do
}

void OPMapGadgetWidget::WPInserted(int const &number, WayPointItem *waypoint)
{
    // to do
}

void OPMapGadgetWidget::WPDeleted(int const &number)
{
    // to do
}

// *************************************************************************************
// user control signals

void OPMapGadgetWidget::comboBoxFindPlace_returnPressed()
{
    QString place = m_widget->comboBoxFindPlace->currentText().simplified();
    if (place.isNull() || place.isEmpty()) return;

    if (!findPlaceWordList.contains(place, Qt::CaseInsensitive))
    {
	findPlaceWordList << place;	// add the new word into the history list
/*
	m_widget->comboBoxFindPlace->setCompleter(NULL);
	delete findPlaceCompleter;
	findPlaceCompleter = new QCompleter(findPlaceWordList, this);
	findPlaceCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	findPlaceCompleter->setCompletionMode(QCompleter::PopupCompletion);
	findPlaceCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	m_widget->comboBoxFindPlace->setCompleter(findPlaceCompleter);
*/
/*
	#if defined(Q_OS_MAC)
	#elif defined(Q_OS_WIN)
	    saveComboBoxLines(m_widget->comboBoxFindPlace, QCoreApplication::applicationDirPath() + "/opmap_find_place_history.txt");
	#else
	#endif
*/
    }

    if (!m_map) return;

    core::GeoCoderStatusCode::Types x = m_map->SetCurrentPositionByKeywords(place);
    QString returned_text = mapcontrol::Helper::StrFromGeoCoderStatusCode(x);

    QMessageBox::information(this, tr("OpenPilot GCS"), returned_text, QMessageBox::Ok);
}

void OPMapGadgetWidget::on_toolButtonFindPlace_clicked()
{
    m_widget->comboBoxFindPlace->setFocus();
    comboBoxFindPlace_returnPressed();
}

void OPMapGadgetWidget::on_toolButtonZoomP_clicked()
{
    zoomIn();
}

void OPMapGadgetWidget::on_toolButtonZoomM_clicked()
{
    zoomOut();
}

void OPMapGadgetWidget::on_toolButtonMapHome_clicked()
{
    followUAVpositionAct->setChecked(false);

    if (m_map)
    {
	internals::PointLatLng home_pos = m_map->Home->Coord();	// get the home location
	m_map->SetCurrentPosition(home_pos);			// center the map onto the home location
    }
}

void OPMapGadgetWidget::on_toolButtonMapUAV_clicked()
{
    followUAVpositionAct->toggle();
}

void OPMapGadgetWidget::on_toolButtonMapUAVheading_clicked()
{
    followUAVheadingAct->toggle();
}

void OPMapGadgetWidget::on_toolButtonShowUAVtrail_clicked()
{
    showUAVtrailAct->toggle();
}

void OPMapGadgetWidget::on_toolButtonClearUAVtrail_clicked()
{
    clearUAVtrailAct->trigger();
}

void OPMapGadgetWidget::on_horizontalSliderZoom_sliderMoved(int position)
{
    setZoom(position);
}

void OPMapGadgetWidget::on_toolButtonHome_clicked()
{
    // to do
}


void OPMapGadgetWidget::on_toolButtonPrevWaypoint_clicked()
{
    // to do
}

void OPMapGadgetWidget::on_toolButtonNextWaypoint_clicked()
{
    // to do
}

void OPMapGadgetWidget::on_toolButtonHoldPosition_clicked()
{
    // to do
}

void OPMapGadgetWidget::on_toolButtonGo_clicked()
{
    // to do
}

void OPMapGadgetWidget::on_toolButtonAddWaypoint_clicked()
{
    if (!m_map) return;

    m_waypoint_list_mutex.lock();

	// create a waypoint at the center of the map
	t_waypoint waypoint;
	waypoint.item = m_map->WPCreate(m_map->CurrentPosition(), 0);
	waypoint.time_seconds = 0;
	waypoint.hold_time_seconds = 0;

	// and remember it in our own local waypoint list
	m_waypoint_list.append(waypoint);

    m_waypoint_list_mutex.unlock();
}

void OPMapGadgetWidget::on_treeViewWaypoints_clicked(QModelIndex index)
{
    QStandardItem *item = wayPoint_treeView_model.itemFromIndex(index);
    if (!item) return;

    // to do
}

// *************************************************************************************
// public functions

void OPMapGadgetWidget::zoomIn()
{
    if (m_map)
	m_map->SetZoom(m_map->Zoom() + 1);
}

void OPMapGadgetWidget::zoomOut()
{
    if (m_map)
	m_map->SetZoom(m_map->Zoom() - 1);
}

void OPMapGadgetWidget::setZoom(int value)
{
    if (m_map)
    {
	internals::MouseWheelZoomType::Types zoom_type = m_map->GetMouseWheelZoomType();
	m_map->SetMouseWheelZoomType(internals::MouseWheelZoomType::ViewCenter);

	m_map->SetZoom(value);

	m_map->SetMouseWheelZoomType(zoom_type);
    }
}

void OPMapGadgetWidget::setPosition(QPointF pos)
{
    if (m_map)
	m_map->SetCurrentPosition(internals::PointLatLng(pos.y(), pos.x()));
}

void OPMapGadgetWidget::setMapProvider(QString provider)
{
    if (m_map)
	m_map->SetMapType(mapcontrol::Helper::MapTypeFromString(provider));
}

void OPMapGadgetWidget::setAccessMode(QString accessMode)
{
    if (m_map)
	m_map->configuration->SetAccessMode(mapcontrol::Helper::AccessModeFromString(accessMode));
}

void OPMapGadgetWidget::setUseOpenGL(bool useOpenGL)
{
    if (m_map)
	m_map->SetUseOpenGL(useOpenGL);
}

void OPMapGadgetWidget::setShowTileGridLines(bool showTileGridLines)
{
    if (m_map)
	m_map->SetShowTileGridLines(showTileGridLines);
}

void OPMapGadgetWidget::setUseMemoryCache(bool useMemoryCache)
{
    if (m_map)
	m_map->configuration->SetUseMemoryCache(useMemoryCache);
}

void OPMapGadgetWidget::setCacheLocation(QString cacheLocation)
{
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

//    qDebug() << "map cache dir: " << cacheLocation;

    if (m_map)
	m_map->configuration->SetCacheLocation(cacheLocation);
}

// *************************************************************************************
// Context menu stuff

void OPMapGadgetWidget::createActions()
{
    // ***********************
    // create menu actions

    closeAct = new QAction(tr("&Close menu"), this);
//    closeAct->setShortcuts(QKeySequence::New);
    closeAct->setStatusTip(tr("Close the context menu"));

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

    findPlaceAct = new QAction(tr("&Find place"), this);
    findPlaceAct->setShortcut(tr("Ctrl+F"));
    findPlaceAct->setStatusTip(tr("Find a location"));
    connect(findPlaceAct, SIGNAL(triggered()), this, SLOT(onFindPlaceAct_triggered()));

    showCompassAct = new QAction(tr("Show compass"), this);
    showCompassAct->setStatusTip(tr("Show/Hide the compass"));
    showCompassAct->setCheckable(true);
    showCompassAct->setChecked(true);
    connect(showCompassAct, SIGNAL(toggled(bool)), this, SLOT(onShowCompassAct_toggled(bool)));

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

    goHomeAct = new QAction(tr("Go to &Home location"), this);
    goHomeAct->setShortcut(tr("Ctrl+H"));
    goHomeAct->setStatusTip(tr("Center the map onto the home location"));
    connect(goHomeAct, SIGNAL(triggered()), this, SLOT(onGoHomeAct_triggered()));

    goUAVAct = new QAction(tr("Go to &UAV location"), this);
    goUAVAct->setShortcut(tr("Ctrl+U"));
    goUAVAct->setStatusTip(tr("Center the map onto the UAV location"));
    connect(goUAVAct, SIGNAL(triggered()), this, SLOT(onGoUAVAct_triggered()));

    setHomePosAct = new QAction(tr("Set Home location"), this);
    setHomePosAct->setStatusTip(tr("Set the current UAV location as the HOME location"));
    connect(setHomePosAct, SIGNAL(triggered()), this, SLOT(onSetHomePosAct_triggered()));

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

    showUAVtrailAct = new QAction(tr("Show UAV trail"), this);
    showUAVtrailAct->setStatusTip(tr("Show/Hide the UAV trail"));
    showUAVtrailAct->setCheckable(true);
    showUAVtrailAct->setChecked(true);
    connect(showUAVtrailAct, SIGNAL(toggled(bool)), this, SLOT(onShowUAVtrailAct_toggled(bool)));

    clearUAVtrailAct = new QAction(tr("Clear UAV trail"), this);
    clearUAVtrailAct->setStatusTip(tr("Clear the UAV trail"));
    connect(clearUAVtrailAct, SIGNAL(triggered()), this, SLOT(onClearUAVtrailAct_triggered()));

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

    zoomActGroup = new QActionGroup(this);
    connect(zoomActGroup, SIGNAL(triggered(QAction *)), this, SLOT(onZoomActGroup_triggered(QAction *)));
    zoomAct.clear();
    for (int i = 2; i <= 19; i++)
    {
	QAction *zoom_act = new QAction(QString::number(i), zoomActGroup);
	zoom_act->setCheckable(true);
	zoom_act->setData(i);
	zoomAct.append(zoom_act);
    }

    // ***********************
}

QPointF OPMapGadgetWidget::getLatLon()
{
    double BaseECEF[3];
    double NED[3];
    double LLA[3];
    UAVObject *obj;

    obj = dynamic_cast<UAVDataObject*>(m_objManager->getObject(QString("HomeLocation")));
    BaseECEF[0] = obj->getField(QString("ECEF"))->getDouble(0);
    BaseECEF[1] = obj->getField(QString("ECEF"))->getDouble(1);
    BaseECEF[2] = obj->getField(QString("ECEF"))->getDouble(2);

    obj = dynamic_cast<UAVDataObject*>(m_objManager->getObject(QString("PositionActual")));
    NED[0] = obj->getField(QString("NED"))->getDouble(0);
    NED[1] = obj->getField(QString("NED"))->getDouble(1);
    NED[2] = obj->getField(QString("NED"))->getDouble(2);

    Utils::CoordinateConversions().GetLLA(BaseECEF, NED, LLA);
    return QPointF(LLA[0],LLA[1]);
}

void OPMapGadgetWidget::onReloadAct_triggered()
{
    if (m_map)
	m_map->ReloadMap();
}

void OPMapGadgetWidget::onCopyMouseLatLonToClipAct_triggered()
{
//    QClipboard *clipboard = qApp->clipboard();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(mouse_lat_lon.Lat(), 'f', 7) + ", " + QString::number(mouse_lat_lon.Lng(), 'f', 7), QClipboard::Clipboard);
}

void OPMapGadgetWidget::onCopyMouseLatToClipAct_triggered()
{
//    QClipboard *clipboard = qApp->clipboard();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(mouse_lat_lon.Lat(), 'f', 7), QClipboard::Clipboard);
}

void OPMapGadgetWidget::onCopyMouseLonToClipAct_triggered()
{
//    QClipboard *clipboard = qApp->clipboard();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(mouse_lat_lon.Lng(), 'f', 7), QClipboard::Clipboard);
}

void OPMapGadgetWidget::onFindPlaceAct_triggered()
{
    m_widget->comboBoxFindPlace->setFocus();	// move focus to the 'find place' text box

/*
    bool ok;
    QString place = QInputDialog::getText(this, tr("OpenPilot GCS"), tr("Find place"), QLineEdit::Normal, QString::null, &ok);
    place = place.simplified();
    if (!ok || place.isNull() || place.isEmpty()) return;

    if (!findPlaceWordList.contains(place, Qt::CaseInsensitive))
    {
	findPlaceWordList += place;	// add the new word into the history list
    }

    if (!m_map) return;

    core::GeoCoderStatusCode::Types x = m_map->SetCurrentPositionByKeywords(place);
    QString returned_text = mapcontrol::Helper::StrFromGeoCoderStatusCode(x);

    QMessageBox::information(this, tr("OpenPilot GCS"), returned_text, QMessageBox::Ok);
*/
}

void OPMapGadgetWidget::onShowCompassAct_toggled(bool show)
{
    if (m_map)
	m_map->SetShowCompass(show);
}

void OPMapGadgetWidget::onShowHomeAct_toggled(bool show)
{
    if (m_map)
//	m_map->SetShowHome(show);    // this can cause a rather big crash
	m_map->Home->setVisible(show);
}

void OPMapGadgetWidget::onShowUAVAct_toggled(bool show)
{
    if (m_map)
//	m_map->SetShowUAV(show);    // this can cause a rather big crash
	m_map->UAV->setVisible(show);
}

void OPMapGadgetWidget::onGoZoomInAct_triggered()
{
    if (m_map)
	setZoom(m_map->Zoom() + 1);
}

void OPMapGadgetWidget::onGoZoomOutAct_triggered()
{
    if (m_map)
	setZoom(m_map->Zoom() - 1);
}

void OPMapGadgetWidget::onZoomActGroup_triggered(QAction *action)
{
    if (!action) return;

    int zoom = action->data().toInt();
    if (zoom < 2 || zoom > 19) return;

    setZoom(zoom);
}

void OPMapGadgetWidget::onGoMouseClickAct_triggered()
{
    if (m_map)
	m_map->SetCurrentPosition(m_map->currentMousePosition());   // center the map onto the mouse position
}

void OPMapGadgetWidget::onGoHomeAct_triggered()
{
    followUAVpositionAct->setChecked(false);
}

void OPMapGadgetWidget::onGoUAVAct_triggered()
{
    QPointF pos = getLatLon();

    if (m_map)
    {
        internals::PointLatLng uav_pos = internals::PointLatLng(pos.x(), pos.y());	// current UAV position
	internals::PointLatLng map_pos = m_map->CurrentPosition();				// current MAP position
	if (map_pos != uav_pos) m_map->SetCurrentPosition(uav_pos);				// center the map onto the UAV
    }
}

void OPMapGadgetWidget::onSetHomePosAct_triggered()
{
    QPointF pos = getLatLon();

    if (m_map)
    {
        internals::PointLatLng uav_pos = internals::PointLatLng(pos.x(), pos.y());	// current UAV position
	m_map->Home->SetCoord(uav_pos);								// move the HOME location to match the current UAV location
        m_map->Home->SetAltitude(0);  // TODO: Update
    }
}

void OPMapGadgetWidget::onFollowUAVpositionAct_toggled(bool checked)
{
    if (m_widget)
    {
	if (m_widget->toolButtonMapUAV->isChecked() != checked)
	    m_widget->toolButtonMapUAV->setChecked(checked);

	setMapFollowingmode();
    }
}

void OPMapGadgetWidget::onFollowUAVheadingAct_toggled(bool checked)
{
    if (m_widget)
    {
	if (m_widget->toolButtonMapUAVheading->isChecked() != checked)
	    m_widget->toolButtonMapUAVheading->setChecked(checked);

	setMapFollowingmode();
    }
}

void OPMapGadgetWidget::onShowUAVtrailAct_toggled(bool checked)
{
    if (m_widget)
    {
	if (m_widget->toolButtonShowUAVtrail->isChecked() != checked)
	    m_widget->toolButtonShowUAVtrail->setChecked(checked);

	if (m_map)
	    m_map->UAV->SetShowTrail(checked);
    }
}

void OPMapGadgetWidget::onClearUAVtrailAct_triggered()
{
    if (m_map)
	m_map->UAV->DeleteTrail();
}

void OPMapGadgetWidget::onOpenWayPointEditorAct_triggered()
{
    waypoint_editor_dialog.show();
}

void OPMapGadgetWidget::onAddWayPointAct_triggered()
{
    if (!m_map) return;

    m_waypoint_list_mutex.lock();

	// create a waypoint on the map at the last known mouse position
	t_waypoint waypoint;
	waypoint.item = m_map->WPCreate(mouse_lat_lon, 0);
	waypoint.time_seconds = 0;
	waypoint.hold_time_seconds = 0;

	// and remember it in our own local waypoint list
	m_waypoint_list.append(waypoint);

    m_waypoint_list_mutex.unlock();
}

void OPMapGadgetWidget::onEditWayPointAct_triggered()
{
    if (!m_mouse_waypoint) return;

    waypoint_edit_dialog.editWaypoint(m_mouse_waypoint);

    m_mouse_waypoint = NULL;
}

void OPMapGadgetWidget::onLockWayPointAct_triggered()
{
    if (!m_mouse_waypoint) return;

    bool locked = (m_mouse_waypoint->flags() & QGraphicsItem::ItemIsMovable) == 0;
    m_mouse_waypoint->setFlag(QGraphicsItem::ItemIsMovable, locked);

    m_mouse_waypoint = NULL;
}

void OPMapGadgetWidget::onDeleteWayPointAct_triggered()
{
    if (!m_mouse_waypoint) return;

    bool locked = (m_mouse_waypoint->flags() & QGraphicsItem::ItemIsMovable) == 0;

    if (locked) return;	// waypoint is locked

    m_waypoint_list_mutex.lock();

	for (int i = 0; i < m_waypoint_list.count(); i++)
	{
	    t_waypoint waypoint = m_waypoint_list.at(i);
	    if (waypoint.item != m_mouse_waypoint) continue;

	    // delete the waypoint from the map
	    if (m_map) m_map->WPDelete(waypoint.item);

	    // delete the waypoint from our local waypoint list
	    m_waypoint_list.removeAt(i);

	    break;
	}

    m_waypoint_list_mutex.unlock();

    m_mouse_waypoint = NULL;
}

void OPMapGadgetWidget::onClearWayPointsAct_triggered()
{
    m_waypoint_list_mutex.lock();
	if (m_map) m_map->WPDeleteAll();
	m_waypoint_list.clear();
    m_waypoint_list_mutex.unlock();
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

void OPMapGadgetWidget::setMapFollowingmode()
{
    if (!m_map) return;

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
	m_map->UAV->SetMapFollowType(UAVMapFollowType::CenterMap);			// the map library won't let you reset the uav rotation if it's already in rotate mode

	m_map->UAV->SetUAVHeading(0);							// reset the UAV heading to 0deg
	m_map->UAV->SetMapFollowType(UAVMapFollowType::CenterAndRotateMap);
    }
}

// *************************************************************************************
