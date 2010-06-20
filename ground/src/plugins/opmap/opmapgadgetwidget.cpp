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
#include <QStringList>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QDir>
#include "extensionsystem/pluginmanager.h"

#include "ui_opmap_controlpanel.h"

// *************************************************************************************
// constructor

OPMapGadgetWidget::OPMapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    // **************

    controlpanel_ui = NULL;
    map = NULL;

    m_follow_uav = false;

    setMouseTracking(true);

    // **************
    // Get required UAVObjects

    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    m_positionActual = PositionActual::GetInstance(objManager);

    // **************
    // create the user control panel

    controlpanel_ui = new Ui::OPMapControlPanel();
    controlpanel_ui->setupUi(this);






/*
    QWidget *dialog = new QWidget(this, Qt::Dialog);

    controlpanel_ui = new Ui::OPMapControlPanel();
    controlpanel_ui->setupUi(dialog);

    QHBoxLayout *d_layout = new QHBoxLayout(dialog);
    d_layout->setSpacing(0);
    d_layout->setContentsMargins(0, 0, 0, 0);
    d_layout->addWidget(controlpanel_ui->layoutWidget);
    dialog->setLayout(d_layout);

    dialog->show();
*/
    // **************
    // create the map display

    map = new mapcontrol::OPMapWidget();
    map->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    map->setMinimumSize(64, 64);

    // **************
    // set the user control options

    controlpanel_ui->labelZoom->setText(" " + QString::number(map->Zoom()));
    controlpanel_ui->labelRotate->setText(" " + QString::number(map->Rotate()));
//    controlpanel_ui->labelNumTilesToLoad->setText(" 0");
    controlpanel_ui->labelNumTilesToLoad->setText("");
    controlpanel_ui->labelStatus->setText("");
    controlpanel_ui->progressBarMap->setMaximum(1);

    // **************

    // get current UAV data
    PositionActual::DataFields data = m_positionActual->getData();

    connect(map, SIGNAL(zoomChanged(double)), this, SLOT(zoomChanged(double)));					// map zoom change signals
    connect(map, SIGNAL(OnCurrentPositionChanged(internals::PointLatLng)), this, SLOT(OnCurrentPositionChanged(internals::PointLatLng)));    // map poisition change signals
    connect(map, SIGNAL(OnTileLoadComplete()), this, SLOT(OnTileLoadComplete()));				// tile loading stop signals
    connect(map, SIGNAL(OnTileLoadStart()), this, SLOT(OnTileLoadStart()));					// tile loading start signals
    connect(map, SIGNAL(OnMapDrag()), this, SLOT(OnMapDrag()));							// map drag signals
    connect(map, SIGNAL(OnMapZoomChanged()), this, SLOT(OnMapZoomChanged()));					// map zoom changed
    connect(map, SIGNAL(OnMapTypeChanged(MapType::Types)), this, SLOT(OnMapTypeChanged(MapType::Types)));	// map type changed
    connect(map, SIGNAL(OnEmptyTileError(int, core::Point)), this, SLOT(OnEmptyTileError(int, core::Point)));	// tile error
    connect(map, SIGNAL(OnTilesStillToLoad(int)), this, SLOT(OnTilesStillToLoad(int)));				// tile loading signals

    map->SetMaxZoom(20);								    // increase the maximum zoom level
    map->SetMouseWheelZoomType(internals::MouseWheelZoomType::MousePositionWithoutCenter);  // set how the mouse wheel zoom functions
    map->SetFollowMouse(true);								    // we want a contiuous mouse position reading
    map->SetUseOpenGL(controlpanel_ui->checkBox_2->isChecked());			    // enable/disable openGL
    map->SetShowTileGridLines(controlpanel_ui->checkBox->isChecked());			    // map grid lines on/off
    map->SetCurrentPosition(internals::PointLatLng(data.Latitude, data.Longitude));	    // set the default map position

    // **************

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(controlpanel_ui->layoutWidget);
//    layout->addWidget(dialog);
    layout->addWidget(map);
    setLayout(layout);

    // **************
    // create the user controls overlayed onto the map

    createMapOverlayUserControls();

    // **************
    // create the desired timers

    m_updateTimer = new QTimer();
    m_updateTimer->setInterval(250);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));
    m_updateTimer->start();

    m_statusUpdateTimer = new QTimer();
    m_statusUpdateTimer->setInterval(100);
    connect(m_statusUpdateTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
    m_statusUpdateTimer->start();

    // **************
}

// *************************************************************************************
// destructor

OPMapGadgetWidget::~OPMapGadgetWidget()
{
    if (map) delete map;
    if (controlpanel_ui) delete controlpanel_ui;
}

// *************************************************************************************
// widget signals

void OPMapGadgetWidget::resizeEvent(QResizeEvent *event)
{
//    if (map) map->resize(QSize(width(), height()));
    update();
    QWidget::resizeEvent(event);
}

void OPMapGadgetWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (map)
    {
//	mouse_lat_lon = map->currentMousePosition();			    // fetch the current mouse lat/longitude position
//	if (mouse_lat_lon != lat_lon)
//	{	// the mouse has moved
//	    mouse_lat_lon = lat_lon;
//
//	    QString coord_str = " " + QString::number(mouse_lat_lon.Lat(), 'f', 6) + "   " + QString::number(mouse_lat_lon.Lng(), 'f', 6);
//
//	    statusLabel.setText(coord_str);
//	    controlpanel_ui->labelStatus->setText(coord_str);
//	}
    }

    if (event->buttons() & Qt::LeftButton)
    {
	 QPoint pos = event->pos();
    }
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

    if (map && m_follow_uav)
    {
	internals::PointLatLng uav_pos = internals::PointLatLng(data.Latitude, data.Longitude);	// current UAV position
	internals::PointLatLng map_pos = map->CurrentPosition();				// current MAP position
	if (map_pos != uav_pos) map->SetCurrentPosition(uav_pos);				// center the map onto the UAV
    }
}

void OPMapGadgetWidget::statusUpdate()
{
    internals::PointLatLng lat_lon = map->currentMousePosition();				// fetch the current lat/lon mouse position

    if (mouse_lat_lon != lat_lon)
    {	// the mouse has moved
	mouse_lat_lon = lat_lon;

	QString coord_str = " " + QString::number(mouse_lat_lon.Lat(), 'f', 6) + "   " + QString::number(mouse_lat_lon.Lng(), 'f', 6);

	statusLabel.setText(coord_str);
	controlpanel_ui->labelStatus->setText(coord_str);
    }
}

// *************************************************************************************
// map signals

void OPMapGadgetWidget::zoomChanged(double zoom)
{
    controlpanel_ui->labelZoom->setText(" " + QString::number(zoom));
}

void OPMapGadgetWidget::OnMapDrag()
{
}

void OPMapGadgetWidget::OnCurrentPositionChanged(internals::PointLatLng point)
{
}

void OPMapGadgetWidget::OnTilesStillToLoad(int number)
{
    if (controlpanel_ui->progressBarMap->maximum() < number)
	controlpanel_ui->progressBarMap->setMaximum(number);						// update the maximum number of tiles used
    controlpanel_ui->progressBarMap->setValue(controlpanel_ui->progressBarMap->maximum() - number);	// update the progress bar

//    controlpanel_ui->labelNumTilesToLoad->setText(" " + QString::number(number));
}

void OPMapGadgetWidget::OnTileLoadStart()
{
    controlpanel_ui->progressBarMap->setVisible(true);
}

void OPMapGadgetWidget::OnTileLoadComplete()
{
    controlpanel_ui->progressBarMap->setVisible(false);
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
// user control panel signals

void OPMapGadgetWidget::on_checkBox_clicked(bool checked)
{
    if (map)
	map->SetShowTileGridLines(checked);
}

void OPMapGadgetWidget::on_checkBox_2_clicked(bool checked)
{
    if (map)
	map->SetUseOpenGL(checked);
}

void OPMapGadgetWidget::on_pushButtonGO_clicked()
{
    if (map)
    {
	core::GeoCoderStatusCode::Types x = map->SetCurrentPositionByKeywords(controlpanel_ui->lineEdit->text());
	controlpanel_ui->label->setText(mapcontrol::Helper::StrFromGeoCoderStatusCode(x));
    }
}

void OPMapGadgetWidget::on_pushButtonReload_clicked()
{
    if (map)
	map->ReloadMap();
}

void OPMapGadgetWidget::on_pushButtonRL_clicked()
{
    if (map)
    {
	map->SetRotate(map->Rotate() - 1);
	controlpanel_ui->labelRotate->setText(" " + QString::number(map->Rotate()));
    }
}

void OPMapGadgetWidget::on_pushButtonRC_clicked()
{
    if (map)
    {
	map->SetRotate(0);
	controlpanel_ui->labelRotate->setText(" " + QString::number(map->Rotate()));
    }
}

void OPMapGadgetWidget::on_pushButtonRR_clicked()
{
    if (map)
    {
	map->SetRotate(map->Rotate() + 1);
	controlpanel_ui->labelRotate->setText(" " + QString::number(map->Rotate()));
    }
}

void OPMapGadgetWidget::on_pushButtonZoomP_clicked()
{
    zoomIn();
}

void OPMapGadgetWidget::on_pushButtonZoomM_clicked()
{
    zoomOut();
}

void OPMapGadgetWidget::on_pushButtonGeoFenceM_clicked()
{
    int geo_fence_distance = controlpanel_ui->spinBoxGeoFenceDistance->value();
    int step = controlpanel_ui->spinBoxGeoFenceDistance->singleStep();
    controlpanel_ui->spinBoxGeoFenceDistance->setValue(geo_fence_distance - step);

    geo_fence_distance = controlpanel_ui->spinBoxGeoFenceDistance->value();


    // to do



}

void OPMapGadgetWidget::on_pushButtonGeoFenceP_clicked()
{
    int geo_fence_distance = controlpanel_ui->spinBoxGeoFenceDistance->value();
    int step = controlpanel_ui->spinBoxGeoFenceDistance->singleStep();
    controlpanel_ui->spinBoxGeoFenceDistance->setValue(geo_fence_distance + step);

    geo_fence_distance = controlpanel_ui->spinBoxGeoFenceDistance->value();


    // to do



}

// *************************************************************************************
// public functions

void OPMapGadgetWidget::zoomIn()
{
    if (map)
	map->SetZoom(map->Zoom() + 1);
}

void OPMapGadgetWidget::zoomOut()
{
    if (map)
	map->SetZoom(map->Zoom() - 1);
}

void OPMapGadgetWidget::setZoom(int value)
{
    if (map)
	map->SetZoom(value);
}

void OPMapGadgetWidget::setPosition(QPointF pos)
{
    if (map)
	map->SetCurrentPosition(internals::PointLatLng(pos.y(), pos.x()));
}

void OPMapGadgetWidget::setMapProvider(QString provider)
{
    if (map)
	if (map->isStarted())
	    map->SetMapType(mapcontrol::Helper::MapTypeFromString(provider));
}

void OPMapGadgetWidget::setUseMemoryCache(bool useMemoryCache)
{
    if (map)
	map->configuration->SetUseMemoryCache(useMemoryCache);
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

    if (map)
	map->configuration->SetCacheLocation(cacheLocation);
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
    overlay_layout_h2->addWidget(&statusLabel);
    overlay_layout_h2->addStretch(0);

    overlay_layout_v1->addSpacing(10);
    overlay_layout_v1->addLayout(overlay_layout_h1);
    overlay_layout_v1->addStretch(0);
//    overlay_layout_v1->addLayout(overlay_layout_h2);
    overlay_layout_v1->addSpacing(10);

    map->setLayout(overlay_layout_v1);
}

// *************************************************************************************
