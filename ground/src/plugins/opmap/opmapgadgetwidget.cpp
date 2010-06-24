/**
 ******************************************************************************
 *
 * @file       opmapgadgetwidget.cpp
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

#include "opmapgadgetwidget.h"
#include "ui_opmap_widget.h"

#include <QStringList>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QDir>
#include "extensionsystem/pluginmanager.h"

// *************************************************************************************
// constructor

OPMapGadgetWidget::OPMapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    // **************

    m_widget = NULL;
    m_map = NULL;

    setMouseTracking(true);

    // **************
    // Get required UAVObjects

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    m_positionActual = PositionActual::GetInstance(objManager);

    // **************
    // create the widget that holds the user controls and the map

    m_widget = new Ui::OPMap_Widget();
//    m_widget = new Ui_OPMap_Widget();
    m_widget->setupUi(this);

    // **************
    // create the map display

    m_map = new mapcontrol::OPMapWidget();

    if (m_map)
    {
	m_map->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	m_map->setMinimumSize(64, 64);

	m_map->configuration->DragButton = Qt::LeftButton;  // use the left mouse button for map dragging
    }

    // **************

    createActions();

    // **************
    // set the user control options

//  m_widget->labelNumTilesToLoad->setText(" 0");
    m_widget->labelMapPos->setText("");
    m_widget->labelMousePos->setText("");
    m_widget->progressBarMap->setMaximum(1);

    m_widget->widgetFlightControls->setVisible(false);
    m_widget->toolButtonFlightControlsShowHide->setIcon(QIcon(QString::fromUtf8(":/core/images/next.png")));

    // **************
    // map stuff

    // get current UAV data
    PositionActual::DataFields data = m_positionActual->getData();

    if (m_map)
    {
	connect(m_map, SIGNAL(zoomChanged(double)), this, SLOT(zoomChanged(double)));					// map zoom change signals
	connect(m_map, SIGNAL(OnCurrentPositionChanged(internals::PointLatLng)), this, SLOT(OnCurrentPositionChanged(internals::PointLatLng)));    // map poisition change signals
	connect(m_map, SIGNAL(OnTileLoadComplete()), this, SLOT(OnTileLoadComplete()));					// tile loading stop signals
	connect(m_map, SIGNAL(OnTileLoadStart()), this, SLOT(OnTileLoadStart()));					// tile loading start signals
	connect(m_map, SIGNAL(OnMapDrag()), this, SLOT(OnMapDrag()));							// map drag signals
	connect(m_map, SIGNAL(OnMapZoomChanged()), this, SLOT(OnMapZoomChanged()));					// map zoom changed
	connect(m_map, SIGNAL(OnMapTypeChanged(MapType::Types)), this, SLOT(OnMapTypeChanged(MapType::Types)));		// map type changed
	connect(m_map, SIGNAL(OnEmptyTileError(int, core::Point)), this, SLOT(OnEmptyTileError(int, core::Point)));	// tile error
	connect(m_map, SIGNAL(OnTilesStillToLoad(int)), this, SLOT(OnTilesStillToLoad(int)));				// tile loading signals

	m_map->SetMaxZoom(19);									    // increase the maximum zoom level
	m_map->SetMouseWheelZoomType(internals::MouseWheelZoomType::MousePositionWithoutCenter);    // set how the mouse wheel zoom functions
	m_map->SetFollowMouse(true);								    // we want a contiuous mouse position reading
	m_map->SetUseOpenGL(openGLAct->isChecked());						    // enable/disable openGL
	m_map->SetShowTileGridLines(gridLinesAct->isChecked());					    // map grid lines on/off
	m_map->SetCurrentPosition(internals::PointLatLng(data.Latitude, data.Longitude));	    // set the default map position
    }

    // **************

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_map);
    m_widget->mapWidget->setLayout(layout);

    // **************
    // create the user controls overlayed onto the map

//    createMapOverlayUserControls();

    // **************
    // create the desired timers

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

// *************************************************************************************
// destructor

OPMapGadgetWidget::~OPMapGadgetWidget()
{
    if (m_map) delete m_map;
    if (m_widget) delete m_widget;
}

// *************************************************************************************
// widget signals

void OPMapGadgetWidget::resizeEvent(QResizeEvent *event)
{
    update();
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
//	    QString coord_str = " " + QString::number(mouse_lat_lon.Lat(), 'f', 6) + "   " + QString::number(mouse_lat_lon.Lng(), 'f', 6) + " ";
//
//	    statusLabel.setText(coord_str);
//	    widget->labelStatus->setText(coord_str);
//	}
    }

    if (event->buttons() & Qt::LeftButton)
    {
	 QPoint pos = event->pos();
    }
}

void OPMapGadgetWidget::contextMenuEvent(QContextMenuEvent *event)
{
    // ****************

    QMenu zoomMenu(tr("&Zoom ") + "(" + QString::number(m_map->Zoom()) + ")", this);
    zoomMenu.addAction(zoom2Act);
    zoomMenu.addAction(zoom3Act);
    zoomMenu.addAction(zoom4Act);
    zoomMenu.addAction(zoom5Act);
    zoomMenu.addAction(zoom6Act);
    zoomMenu.addAction(zoom7Act);
    zoomMenu.addAction(zoom8Act);
    zoomMenu.addAction(zoom9Act);
    zoomMenu.addAction(zoom10Act);
    zoomMenu.addAction(zoom11Act);
    zoomMenu.addAction(zoom12Act);
    zoomMenu.addAction(zoom13Act);
    zoomMenu.addAction(zoom14Act);
    zoomMenu.addAction(zoom15Act);
    zoomMenu.addAction(zoom16Act);
    zoomMenu.addAction(zoom17Act);
    zoomMenu.addAction(zoom18Act);
    zoomMenu.addAction(zoom19Act);

    // ****************

    QMenu menu(this);

    menu.addAction(closeAct);

    menu.addSeparator();

    menu.addAction(reloadAct);

    menu.addSeparator();

    menu.addAction(findPlaceAct);

    menu.addSeparator()->setText(tr("Zoom"));

    menu.addAction(zoomInAct);
    menu.addAction(zoomOutAct);
    menu.addMenu(&zoomMenu);

    menu.addSeparator()->setText(tr("Position"));

    menu.addAction(goMouseClickAct);
    menu.addAction(goHomeAct);
    menu.addAction(goUAVAct);

    menu.addSeparator()->setText(tr("Follow"));

    menu.addAction(followUAVpositionAct);
    menu.addAction(followUAVheadingAct);

    menu.addSeparator()->setText(tr("Waypoints"));

    menu.addAction(wayPointEditorAct);
    menu.addAction(addWayPointAct);
    menu.addAction(deleteWayPointAct);
    menu.addAction(clearWayPointsAct);

    menu.addSeparator();

//    menu.addAction(gridLinesAct);
    menu.addAction(openGLAct);

    menu.exec(event->globalPos());

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
    PositionActual::DataFields data = m_positionActual->getData();				// get current UAV data
    internals::PointLatLng uav_pos = internals::PointLatLng(data.Latitude, data.Longitude);	// current UAV position
    double uav_heading = data.Heading;								// current UAV heading
    double uav_height_feet = data.Altitude * 3.2808399;						// current UAV height
    double uav_ground_speed = data.Groundspeed;							// current UAV ground speed

    // display the UAV lat/lon position
    QString str =   " lat:" + QString::number(uav_pos.Lat(), 'f', 6) +
		    "   lon:" + QString::number(uav_pos.Lng(), 'f', 6) +
		    "   " + QString::number(uav_heading, 'f', 1) + "deg" +
		    "   " + QString::number(uav_height_feet, 'f', 1) + "feet" +
		    "   " + QString::number(uav_ground_speed, 'f', 1) + "mph";
    if (m_widget) m_widget->labelMapPos->setText(str);

    if (m_map && followUAVpositionAct && followUAVheadingAct)
    {
	if (followUAVpositionAct->isChecked())
	{
	    internals::PointLatLng map_pos = m_map->CurrentPosition();					// current MAP position
	    double map_heading = m_map->Rotate();

	    if (map_pos != uav_pos || map_heading != uav_heading)
	    {
		m_map->SetCurrentPosition(uav_pos);							// keep the map centered on the UAV

		if (followUAVheadingAct->isChecked())
		    m_map->SetRotate(-uav_heading);							// rotate the map to match the uav heading
		else
		if (m_map->Rotate() != 0)
		    m_map->SetRotate(0);								// reset the rotation to '0'
	    }
	}
    }
}

void OPMapGadgetWidget::updateMousePos()
{
    internals::PointLatLng lat_lon = m_map->currentMousePosition();				// fetch the current lat/lon mouse position

    if (mouse_lat_lon != lat_lon)
    {	// the mouse has moved
	mouse_lat_lon = lat_lon;

	QString str = " " + QString::number(mouse_lat_lon.Lat(), 'f', 6) + "   " + QString::number(mouse_lat_lon.Lng(), 'f', 6);
	if (m_widget) m_widget->labelMousePos->setText(str);
    }
}

// *************************************************************************************
// map signals

void OPMapGadgetWidget::zoomChanged(double zoom)
{
    int i_zoom = (int)(zoom + 0.5);

    if (m_widget)
    {
//	m_widget->labelZoom->setText(" " + QString::number(zoom));
	m_widget->horizontalSliderZoom->setValue(i_zoom);
    }

    switch (i_zoom)
    {
	case 2: if (zoom2Act) zoom2Act->setChecked(true); break;
	case 3: if (zoom3Act) zoom3Act->setChecked(true); break;
	case 4: if (zoom4Act) zoom4Act->setChecked(true); break;
	case 5: if (zoom5Act) zoom5Act->setChecked(true); break;
	case 6: if (zoom6Act) zoom6Act->setChecked(true); break;
	case 7: if (zoom7Act) zoom7Act->setChecked(true); break;
	case 8: if (zoom8Act) zoom8Act->setChecked(true); break;
	case 9: if (zoom9Act) zoom9Act->setChecked(true); break;
	case 10: if (zoom10Act) zoom10Act->setChecked(true); break;
	case 11: if (zoom11Act) zoom11Act->setChecked(true); break;
	case 12: if (zoom12Act) zoom12Act->setChecked(true); break;
	case 13: if (zoom13Act) zoom13Act->setChecked(true); break;
	case 14: if (zoom14Act) zoom14Act->setChecked(true); break;
	case 15: if (zoom15Act) zoom15Act->setChecked(true); break;
	case 16: if (zoom16Act) zoom16Act->setChecked(true); break;
	case 17: if (zoom17Act) zoom17Act->setChecked(true); break;
	case 18: if (zoom18Act) zoom18Act->setChecked(true); break;
	case 19: if (zoom19Act) zoom19Act->setChecked(true); break;
	default: break;
    }
}

void OPMapGadgetWidget::OnMapDrag()
{
}

void OPMapGadgetWidget::OnCurrentPositionChanged(internals::PointLatLng point)
{
    if (m_widget)
    {
	QString coord_str = " " + QString::number(point.Lat(), 'f', 6) + "   " + QString::number(point.Lng(), 'f', 6) + " ";
//	m_widget->labelMapPos->setText(coord_str);
    }
}

void OPMapGadgetWidget::OnTilesStillToLoad(int number)
{
    if (m_widget)
    {
	if (m_widget->progressBarMap->maximum() < number)
	    m_widget->progressBarMap->setMaximum(number);					// update the maximum number of tiles used
	m_widget->progressBarMap->setValue(m_widget->progressBarMap->maximum() - number);	// update the progress bar

//	m_widget->labelNumTilesToLoad->setText(" " + QString::number(number));
    }
}

void OPMapGadgetWidget::OnTileLoadStart()
{
    if (m_widget) m_widget->progressBarMap->setVisible(true);
}

void OPMapGadgetWidget::OnTileLoadComplete()
{
    if (m_widget) m_widget->progressBarMap->setVisible(false);
}

void OPMapGadgetWidget::OnMapZoomChanged()
{
}

void OPMapGadgetWidget::OnMapTypeChanged(MapType::Types type)
{
}

void OPMapGadgetWidget::OnEmptyTileError(int zoom, core::Point pos)
{
}

// *************************************************************************************
// user control signals

void OPMapGadgetWidget::on_toolButtonReload_clicked()
{
    if (m_map)
	m_map->ReloadMap();
}

void OPMapGadgetWidget::on_toolButtonFindPlace_clicked()
{
    findPlace();
}

void OPMapGadgetWidget::on_toolButtonZoomP_clicked()
{
    zoomIn();
}

void OPMapGadgetWidget::on_toolButtonZoomM_clicked()
{
    zoomOut();
}

void OPMapGadgetWidget::on_pushButtonGeoFenceM_clicked()
{
    if (m_widget)
    {
//	int geo_fence_distance = m_widget->spinBoxGeoFenceDistance->value();
//	int step = m_widget->spinBoxGeoFenceDistance->singleStep();
//	m_widget->spinBoxGeoFenceDistance->setValue(geo_fence_distance - step);

//	geo_fence_distance = m_widget->spinBoxGeoFenceDistance->value();



	// to do



    }
}

void OPMapGadgetWidget::on_pushButtonGeoFenceP_clicked()
{
    if (m_widget)
    {
//	int geo_fence_distance = m_widget->spinBoxGeoFenceDistance->value();
//	int step = m_widget->spinBoxGeoFenceDistance->singleStep();
//	m_widget->spinBoxGeoFenceDistance->setValue(geo_fence_distance + step);

//	geo_fence_distance = m_widget->spinBoxGeoFenceDistance->value();


	// to do



    }
}

void OPMapGadgetWidget::on_toolButtonFlightControlsShowHide_clicked()
{
    if (m_widget)
    {
	m_widget->widgetFlightControls->setVisible(!m_widget->widgetFlightControls->isVisible());

	if (m_widget->widgetFlightControls->isVisible())
	    m_widget->toolButtonFlightControlsShowHide->setIcon(QIcon(QString::fromUtf8(":/core/images/prev.png")));
	else
	    m_widget->toolButtonFlightControlsShowHide->setIcon(QIcon(QString::fromUtf8(":/core/images/next.png")));
    }
}

void OPMapGadgetWidget::on_toolButtonMapHome_clicked()
{
    followUAVpositionAct->setChecked(false);
}

void OPMapGadgetWidget::on_toolButtonMapUAV_clicked()
{
    followUAVpositionAct->toggle();
}

void OPMapGadgetWidget::on_horizontalSliderZoom_sliderMoved(int position)
{
    setZoom(position);
}

void OPMapGadgetWidget::on_toolButtonHome_clicked()
{
}

void OPMapGadgetWidget::on_toolButtonHoldPosition_clicked()
{
}

void OPMapGadgetWidget::on_toolButtonGo_clicked()
{
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
	if (m_map->isStarted())
	    m_map->SetMapType(mapcontrol::Helper::MapTypeFromString(provider));
}

void OPMapGadgetWidget::setUseMemoryCache(bool useMemoryCache)
{
    if (m_map)
	m_map->configuration->SetUseMemoryCache(useMemoryCache);
}

void OPMapGadgetWidget::setCacheLocation(QString cacheLocation)
{
    cacheLocation = cacheLocation.trimmed();	// remove any surrounding spaces

    if (cacheLocation.isEmpty()) return;

    #if defined(Q_WS_WIN)
	if (!cacheLocation.endsWith('/')) cacheLocation += '/';
    #elif defined(Q_WS_X11)
	if (!cacheLocation.endsWith(QDir::separator())) cacheLocation += QDir::separator();
    #elif defined(Q_WS_MAC)
	if (!cacheLocation.endsWith(QDir::separator())) cacheLocation += QDir::separator();
    #endif

    QDir dir;
    if (!dir.exists(cacheLocation))
	if (!dir.mkpath(cacheLocation))
	    return;

    qDebug() << "map cache dir: " << cacheLocation;

    if (m_map)
	m_map->configuration->SetCacheLocation(cacheLocation);
}

// *************************************************************************************
// create some user controls overlayed onto the map area

QPushButton * OPMapGadgetWidget::createTransparentButton(QWidget *parent, QString text, QString icon)
{
    QPixmap pix;
    pix.load(icon);

    QPushButton *but = new QPushButton(parent);

    QColor transparent_color(0,0,0,0);
    QPalette but_pal(but->palette());

    but_pal.setColor(QPalette::Button, transparent_color);
    but->setPalette(but_pal);

    but->setIcon(pix);
    but->setText(text);
    but->setIconSize(pix.size());

    return but;
}

void OPMapGadgetWidget::createMapOverlayUserControls()
{
    QPushButton *zoomout = new QPushButton("");
    zoomout->setToolTip(tr("Zoom out"));
    zoomout->setCursor(Qt::OpenHandCursor);
    zoomout->setFlat(true);
    zoomout->setIcon(QIcon(QString::fromUtf8(":/opmap/images/minus.png")));
    zoomout->setIconSize(QSize(32, 32));
    zoomout->setFixedSize(28, 28);
    connect(zoomout, SIGNAL(clicked(bool)), this, SLOT(zoomOut()));

//    QPushButton *zoomin = createTransparentButton(map, "", QString::fromUtf8(":/core/images/plus.png"));
//    zoomin->setStyleSheet("");
    QPushButton *zoomin = new QPushButton("");
    zoomin->setFlat(true);
    zoomin->setToolTip(tr("Zoom in"));
    zoomin->setCursor(Qt::OpenHandCursor);
    zoomin->setIcon(QIcon(QString::fromUtf8(":/opmap/images/plus.png")));
//    zoomin->setIconSize(QSize(12, 12));
    zoomin->setIconSize(QSize(28, 28));
    zoomin->setFixedSize(32, 32);
//    zoomin->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(zoomin, SIGNAL(clicked(bool)), this, SLOT(zoomIn()));

//    statusLabel.font().setPointSize(20);

    // add zoom buttons to the layout of the MapControl
    QVBoxLayout* overlay_layout_v1 = new QVBoxLayout;
    overlay_layout_v1->setMargin(4);
    overlay_layout_v1->setSpacing(4);

    QHBoxLayout* overlay_layout_h1 = new QHBoxLayout;
    overlay_layout_h1->setMargin(0);
    overlay_layout_h1->setSpacing(4);
//  overlay_layout_h1->addWidget(gcsButton);
//  overlay_layout_h1->addWidget(uavButton);
//  overlay_layout_h1->addSpacing(10);
    overlay_layout_h1->addWidget(zoomout);
    overlay_layout_h1->addWidget(zoomin);
    overlay_layout_h1->addStretch(0);

    QHBoxLayout* overlay_layout_h2 = new QHBoxLayout;
    overlay_layout_h2->setMargin(0);
    overlay_layout_h2->setSpacing(4);
    overlay_layout_h2->addStretch(0);
//    overlay_layout_h2->addWidget(&statusLabel);
    overlay_layout_h2->addStretch(0);

    overlay_layout_v1->addSpacing(10);
    overlay_layout_v1->addLayout(overlay_layout_h1);
    overlay_layout_v1->addStretch(0);
//    overlay_layout_v1->addLayout(overlay_layout_h2);
    overlay_layout_v1->addSpacing(10);

    m_map->setLayout(overlay_layout_v1);
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
    connect(reloadAct, SIGNAL(triggered()), this, SLOT(reload()));

    findPlaceAct = new QAction(tr("&Find place"), this);
    findPlaceAct->setShortcut(tr("Ctrl+F"));
    findPlaceAct->setStatusTip(tr("Find a location"));
    connect(findPlaceAct, SIGNAL(triggered()), this, SLOT(findPlace()));

    zoomInAct = new QAction(tr("Zoom &In"), this);
    zoomInAct->setShortcut(Qt::Key_PageUp);
    zoomInAct->setStatusTip(tr("Zoom the map in"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(goZoomIn()));

    zoomOutAct = new QAction(tr("Zoom &Out"), this);
    zoomOutAct->setShortcut(Qt::Key_PageDown);
    zoomOutAct->setStatusTip(tr("Zoom the map out"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(goZoomOut()));

    goMouseClickAct = new QAction(tr("Go too where you right clicked the mouse"), this);
    goMouseClickAct->setStatusTip(tr("Center the map onto where you right clicked the mouse"));
    connect(goMouseClickAct, SIGNAL(triggered()), this, SLOT(goMouseClick()));

    goHomeAct = new QAction(tr("Go too &Home location"), this);
    goHomeAct->setShortcut(tr("Ctrl+H"));
    goHomeAct->setStatusTip(tr("Center the map onto the home location"));
    connect(goHomeAct, SIGNAL(triggered()), this, SLOT(goHome()));

    goUAVAct = new QAction(tr("Go too &UAV location"), this);
    goUAVAct->setShortcut(tr("Ctrl+U"));
    goUAVAct->setStatusTip(tr("Center the map onto the UAV location"));
    connect(goUAVAct, SIGNAL(triggered()), this, SLOT(goUAV()));

    followUAVpositionAct = new QAction(tr("Follow UAV position"), this);
    followUAVpositionAct->setShortcut(tr("Ctrl+F"));
    followUAVpositionAct->setStatusTip(tr("Keep the map centered onto the UAV"));
    followUAVpositionAct->setCheckable(true);
    followUAVpositionAct->setChecked(false);
    connect(followUAVpositionAct, SIGNAL(toggled(bool)), this, SLOT(on_followUAVpositionAct_toggled(bool)));

    followUAVheadingAct = new QAction(tr("Follow UAV heading"), this);
    followUAVheadingAct->setShortcut(tr("Ctrl+F"));
    followUAVheadingAct->setStatusTip(tr("Keep the map rotation to the UAV heading"));
    followUAVheadingAct->setCheckable(true);
    followUAVheadingAct->setChecked(true);
    connect(followUAVheadingAct, SIGNAL(toggled(bool)), this, SLOT(on_followUAVheadingAct_toggled(bool)));

    wayPointEditorAct = new QAction(tr("&Way point editor"), this);
    wayPointEditorAct->setShortcut(tr("Ctrl+W"));
    wayPointEditorAct->setStatusTip(tr("Open the way-point editor"));
    connect(wayPointEditorAct, SIGNAL(triggered()), this, SLOT(openWayPointEditor()));

    addWayPointAct = new QAction(tr("&Add waypoint"), this);
    addWayPointAct->setShortcut(tr("Ctrl+A"));
    addWayPointAct->setStatusTip(tr("Add waypoint"));
    connect(addWayPointAct, SIGNAL(triggered()), this, SLOT(addWayPoint()));

    deleteWayPointAct = new QAction(tr("&Delete waypoint"), this);
    deleteWayPointAct->setShortcut(tr("Ctrl+D"));
    deleteWayPointAct->setStatusTip(tr("Delete waypoint"));
    connect(deleteWayPointAct, SIGNAL(triggered()), this, SLOT(deleteWayPoint()));

    clearWayPointsAct = new QAction(tr("&Clear waypoints"), this);
    clearWayPointsAct->setShortcut(tr("Ctrl+C"));
    clearWayPointsAct->setStatusTip(tr("Clear waypoints"));
    connect(clearWayPointsAct, SIGNAL(triggered()), this, SLOT(clearWayPoints()));

    gridLinesAct = new QAction(tr("Grid lines"), this);
    gridLinesAct->setShortcut(tr("Ctrl+G"));
    gridLinesAct->setStatusTip(tr("Show/Hide grid lines"));
    gridLinesAct->setCheckable(true);
    gridLinesAct->setChecked(false);
    connect(gridLinesAct, SIGNAL(triggered()), this, SLOT(gridLines()));

    openGLAct = new QAction(tr("Use OpenGL"), this);
    openGLAct->setShortcut(tr("Ctrl+O"));
    openGLAct->setStatusTip(tr("Enable/Disable OpenGL"));
    openGLAct->setCheckable(true);
    openGLAct->setChecked(false);
    connect(openGLAct, SIGNAL(triggered()), this, SLOT(openGL()));

    zoom2Act = new QAction(tr("2"), this);
    zoom2Act->setCheckable(true);
    connect(zoom2Act, SIGNAL(triggered()), this, SLOT(zoom2()));
    zoom3Act = new QAction(tr("3"), this);
    zoom3Act->setCheckable(true);
    connect(zoom3Act, SIGNAL(triggered()), this, SLOT(zoom3()));
    zoom4Act = new QAction(tr("4"), this);
    zoom4Act->setCheckable(true);
    connect(zoom4Act, SIGNAL(triggered()), this, SLOT(zoom4()));
    zoom5Act = new QAction(tr("5"), this);
    zoom5Act->setCheckable(true);
    connect(zoom5Act, SIGNAL(triggered()), this, SLOT(zoom5()));
    zoom6Act = new QAction(tr("6"), this);
    zoom6Act->setCheckable(true);
    connect(zoom6Act, SIGNAL(triggered()), this, SLOT(zoom6()));
    zoom7Act = new QAction(tr("7"), this);
    zoom7Act->setCheckable(true);
    connect(zoom7Act, SIGNAL(triggered()), this, SLOT(zoom7()));
    zoom8Act = new QAction(tr("8"), this);
    zoom8Act->setCheckable(true);
    connect(zoom8Act, SIGNAL(triggered()), this, SLOT(zoom8()));
    zoom9Act = new QAction(tr("9"), this);
    zoom9Act->setCheckable(true);
    connect(zoom9Act, SIGNAL(triggered()), this, SLOT(zoom9()));
    zoom10Act = new QAction(tr("10"), this);
    zoom10Act->setCheckable(true);
    connect(zoom10Act, SIGNAL(triggered()), this, SLOT(zoom10()));
    zoom11Act = new QAction(tr("11"), this);
    zoom11Act->setCheckable(true);
    connect(zoom11Act, SIGNAL(triggered()), this, SLOT(zoom11()));
    zoom12Act = new QAction(tr("12"), this);
    zoom12Act->setCheckable(true);
    connect(zoom12Act, SIGNAL(triggered()), this, SLOT(zoom12()));
    zoom13Act = new QAction(tr("13"), this);
    zoom13Act->setCheckable(true);
    connect(zoom13Act, SIGNAL(triggered()), this, SLOT(zoom13()));
    zoom14Act = new QAction(tr("14"), this);
    zoom14Act->setCheckable(true);
    connect(zoom14Act, SIGNAL(triggered()), this, SLOT(zoom14()));
    zoom15Act = new QAction(tr("15"), this);
    zoom15Act->setCheckable(true);
    connect(zoom15Act, SIGNAL(triggered()), this, SLOT(zoom15()));
    zoom16Act = new QAction(tr("16"), this);
    zoom16Act->setCheckable(true);
    connect(zoom16Act, SIGNAL(triggered()), this, SLOT(zoom16()));
    zoom17Act = new QAction(tr("17"), this);
    zoom17Act->setCheckable(true);
    connect(zoom17Act, SIGNAL(triggered()), this, SLOT(zoom17()));
    zoom18Act = new QAction(tr("18"), this);
    zoom18Act->setCheckable(true);
    connect(zoom18Act, SIGNAL(triggered()), this, SLOT(zoom18()));
    zoom19Act = new QAction(tr("19"), this);
    zoom19Act->setCheckable(true);
    connect(zoom19Act, SIGNAL(triggered()), this, SLOT(zoom19()));
    zoomActGroup = new QActionGroup(this);
    zoomActGroup->addAction(zoom2Act);
    zoomActGroup->addAction(zoom3Act);
    zoomActGroup->addAction(zoom4Act);
    zoomActGroup->addAction(zoom5Act);
    zoomActGroup->addAction(zoom6Act);
    zoomActGroup->addAction(zoom7Act);
    zoomActGroup->addAction(zoom8Act);
    zoomActGroup->addAction(zoom9Act);
    zoomActGroup->addAction(zoom10Act);
    zoomActGroup->addAction(zoom11Act);
    zoomActGroup->addAction(zoom12Act);
    zoomActGroup->addAction(zoom13Act);
    zoomActGroup->addAction(zoom14Act);
    zoomActGroup->addAction(zoom15Act);
    zoomActGroup->addAction(zoom16Act);
    zoomActGroup->addAction(zoom17Act);
    zoomActGroup->addAction(zoom18Act);
    zoomActGroup->addAction(zoom19Act);

    // ***********************
}

void OPMapGadgetWidget::reload()
{
    if (m_map)
	m_map->ReloadMap();
}

void OPMapGadgetWidget::findPlace()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("OpenPilot GCS"), tr("Find place"), QLineEdit::Normal, QString::null, &ok);
    if (ok && !text.isEmpty())
    {
	if (m_map)
	{
	    core::GeoCoderStatusCode::Types x = m_map->SetCurrentPositionByKeywords(text);
	    QString returned_text = mapcontrol::Helper::StrFromGeoCoderStatusCode(x);

	    int ret = QMessageBox::information(this, tr("OpenPilot GCS"), returned_text, QMessageBox::Ok);
	}
    }
}

void OPMapGadgetWidget::goZoomIn()
{
    if (m_map)
	setZoom(m_map->Zoom() + 1);
}

void OPMapGadgetWidget::goZoomOut()
{
    if (m_map)
	setZoom(m_map->Zoom() - 1);
}

void OPMapGadgetWidget::goMouseClick()
{
    if (m_map)
	m_map->SetCurrentPosition(m_map->currentMousePosition());   // center the map onto the mouse position
}

void OPMapGadgetWidget::goHome()
{
    followUAVpositionAct->setChecked(false);
}

void OPMapGadgetWidget::goUAV()
{
    PositionActual::DataFields data = m_positionActual->getData();				// get current UAV data

    if (m_map)
    {
	internals::PointLatLng uav_pos = internals::PointLatLng(data.Latitude, data.Longitude);	// current UAV position
	internals::PointLatLng map_pos = m_map->CurrentPosition();				// current MAP position
	if (map_pos != uav_pos) m_map->SetCurrentPosition(uav_pos);				// center the map onto the UAV
    }
}

void OPMapGadgetWidget::on_followUAVpositionAct_toggled(bool checked)
{
    if (m_widget)
    {
	if (m_widget->toolButtonMapUAV->isChecked() != followUAVpositionAct->isChecked())
	    m_widget->toolButtonMapUAV->setChecked(followUAVpositionAct->isChecked());

	if (m_map)
	    m_map->SetRotate(0);				    				// reset the rotation to '0'
    }
}

void OPMapGadgetWidget::on_followUAVheadingAct_toggled(bool checked)
{
    if (!checked && m_map)
	m_map->SetRotate(0);									// reset the rotation to '0'
}

void OPMapGadgetWidget::openWayPointEditor()
{
    // to do
}

void OPMapGadgetWidget::addWayPoint()
{
    // to do
}

void OPMapGadgetWidget::deleteWayPoint()
{
    // to do
}

void OPMapGadgetWidget::clearWayPoints()
{
    // to do
}

void OPMapGadgetWidget::gridLines()
{
    if (m_map)
	m_map->SetShowTileGridLines(gridLinesAct->isChecked());
}

void OPMapGadgetWidget::openGL()
{
    if (m_map)
	m_map->SetUseOpenGL(openGLAct->isChecked());
}

// *************************************************************************************
