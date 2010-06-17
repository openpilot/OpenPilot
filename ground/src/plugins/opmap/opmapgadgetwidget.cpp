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
#include "extensionsystem/pluginmanager.h"

#include "ui_opmap_controlpanel.h"

// *************************************************************************************
// constructor

OPMapGadgetWidget::OPMapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    // **************

    map = NULL;
    controlpanel_ui = NULL;

    follow_uav = false;

    // **************
    // Get required UAVObjects

    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    m_positionActual = PositionActual::GetInstance(objManager);

    // **************
    // create the user control panel

    controlpanel_ui = new Ui::OPMapControlPanel();
    controlpanel_ui->setupUi(this);

    // **************
    // create the map display

    map = new mapcontrol::OPMapWidget();
    map->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    map->setMinimumSize(64, 64);

    // **************
    // set the user control options

    controlpanel_ui->comboBox->addItems(mapcontrol::Helper::MapTypes());
    controlpanel_ui->comboBox->setCurrentIndex(mapcontrol::Helper::MapTypes().indexOf("GoogleHybrid"));

    // **************

    // receive map zoom changes
    connect(map, SIGNAL(zoomChanged(double)), this, SLOT(zoomChanged(double)));

    map->SetShowTileGridLines(controlpanel_ui->checkBox->isChecked());

    // get current position data
    PositionActual::DataFields data = m_positionActual->getData();

    // set the default map position
    map->SetCurrentPosition(internals::PointLatLng(data.Latitude, data.Longitude));

    // **************

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(controlpanel_ui->layoutWidget);
    layout->addWidget(map);
    setLayout(layout);

    // **************

    m_updateTimer = new QTimer();
    m_updateTimer->setInterval(250);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));
    m_updateTimer->start();

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

void OPMapGadgetWidget::setZoom(int value)
{
    if (map)
    {
	map->SetZoom(value);
    }
}

void OPMapGadgetWidget::setPosition(QPointF pos)
{
    if (map)
    {
	map->SetCurrentPosition(internals::PointLatLng(pos.y(), pos.x()));
    }
}

void OPMapGadgetWidget::setMapProvider(QString provider)
{
//    if (map)
//	if (map->isStarted())
//	    map->SetMapType(mapcontrol::Helper::MapTypeFromString(provider));
}

// *************************************************************************************

void OPMapGadgetWidget::updatePosition()
{
    // get current position data
    PositionActual::DataFields data = m_positionActual->getData();

    if (map && follow_uav)
    {	// center the map onto the UAV
	map->SetCurrentPosition(internals::PointLatLng(data.Latitude, data.Longitude));
    }
}

// *************************************************************************************

void OPMapGadgetWidget::resizeEvent(QResizeEvent *event)
{
//    if (map) map->resize(QSize(width(), height()));
    update();
    QWidget::resizeEvent(event);
}

// *************************************************************************************

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

void OPMapGadgetWidget::zoomChanged(double zoom)
{
    controlpanel_ui->labelZoom->setText(" " + QString::number(zoom));
}

void OPMapGadgetWidget::on_checkBox_clicked(bool checked)
{
    if (map)
	map->SetShowTileGridLines(checked);
}

void OPMapGadgetWidget::on_comboBox_currentIndexChanged(QString value)
{
    if (map)
	if (map->isStarted())
	    map->SetMapType(mapcontrol::Helper::MapTypeFromString(value));
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
	map->SetRotate(map->Rotate() - 1);
}

void OPMapGadgetWidget::on_pushButtonRC_clicked()
{
    if (map)
	map->SetRotate(0);
}

void OPMapGadgetWidget::on_pushButtonRR_clicked()
{
    if (map)
	map->SetRotate(map->Rotate() + 1);
}

void OPMapGadgetWidget::on_pushButtonZoomP_clicked()
{
    if (map)
    {
	double zoom = map->Zoom();
//	double zoom_step = controlpanel_ui->doubleSpinBox->value();
	double zoom_step = 1;
	map->SetZoom(zoom + zoom_step);
    }
}

void OPMapGadgetWidget::on_pushButtonZoomM_clicked()
{
    if (map)
    {
	double zoom = map->Zoom();
//	double zoom_step = controlpanel_ui->doubleSpinBox->value();
	double zoom_step = 1;
	map->SetZoom(zoom - zoom_step);
    }
}

void OPMapGadgetWidget::on_checkBox_2_clicked(bool checked)
{
    if (map)
	map->SetUseOpenGL(checked);
}

void OPMapGadgetWidget::on_pushButtonGeoFenceM_clicked()
{
    int geo_fence_distance = controlpanel_ui->spinBoxGeoFenceDistance->value();
    int step = controlpanel_ui->spinBoxGeoFenceDistance->singleStep();
    controlpanel_ui->spinBoxGeoFenceDistance->setValue(geo_fence_distance - step);

    geo_fence_distance = controlpanel_ui->spinBoxGeoFenceDistance->value();
}

void OPMapGadgetWidget::on_pushButtonGeoFenceP_clicked()
{
    int geo_fence_distance = controlpanel_ui->spinBoxGeoFenceDistance->value();
    int step = controlpanel_ui->spinBoxGeoFenceDistance->singleStep();
    controlpanel_ui->spinBoxGeoFenceDistance->setValue(geo_fence_distance + step);

    geo_fence_distance = controlpanel_ui->spinBoxGeoFenceDistance->value();
}


// *************************************************************************************
