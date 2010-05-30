/**
 ******************************************************************************
 *
 * @file       mapgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   map
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
#include "mapgadgetwidget.h"
#include <QStringList>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include "extensionsystem/pluginmanager.h"

MapGadgetWidget::MapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    int size = 256;
    m_mc = new MapControl(QSize(size, size));
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_mc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_mc->setMinimumSize(64, 64);
    m_mc->showScale(true);
    m_osmAdapter = new OSMMapAdapter();
    m_googleSatAdapter = new GoogleSatMapAdapter();
    m_googleAdapter = new GoogleMapAdapter();
    m_yahooAdapter = new YahooMapAdapter();
    m_osmLayer = new MapLayer("OpenStreetMap", m_osmAdapter);
    m_googleLayer = new MapLayer("Google", m_googleAdapter);
    m_googleSatLayer = new MapLayer("Google Sat", m_googleSatAdapter);
    m_yahooLayer = new MapLayer("Yahoo", m_yahooAdapter);
    m_osmLayer->setVisible(true);
    m_googleLayer->setVisible(false);
    m_googleSatLayer->setVisible(false);
    m_yahooLayer->setVisible(false);
    m_mc->addLayer(m_osmLayer);
    m_mc->addLayer(m_googleLayer);
    m_mc->addLayer(m_googleSatLayer);
    m_mc->addLayer(m_yahooLayer);

    addZoomButtons();
    m_mc->setView(QPointF(5.718888888888, 58.963333333333));
    m_mc->setZoom(10);
    m_mc->updateRequestNew();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mc);
    setLayout(layout);

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    m_gpsObj = GpsObject::GetInstance(objManager);

    m_updateTimer = new QTimer();
    m_updateTimer->setInterval(250);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));
    m_updateTimer->start();
}

MapGadgetWidget::~MapGadgetWidget()
{
   // Do nothing
}
void MapGadgetWidget::setZoom(int value)
{
    m_mc->setZoom(value);
    m_mc->updateRequestNew();
}

void MapGadgetWidget::setPosition(QPointF pos)
{
    m_mc->setView(pos);
    m_mc->updateRequestNew();
}

void MapGadgetWidget::updatePosition()
{
    GpsObject::DataFields data = m_gpsObj->getData();
    setPosition(QPointF(data.Longitude, data.Latitude));
}

void MapGadgetWidget::setMapProvider(QString provider)
{
    foreach(QString layerName, m_mc->layers())
        m_mc->layer(layerName)->setVisible(layerName == provider);
}


void MapGadgetWidget::resizeEvent(QResizeEvent *event)
{
    m_mc->resize(QSize(width(), height()));
    update();
    QWidget::resizeEvent(event);
}

void MapGadgetWidget::addZoomButtons()
{
	// create buttons as controls for zoom
	QPushButton* zoomin = new QPushButton("+");
	QPushButton* zoomout = new QPushButton("-");
	zoomin->setMaximumWidth(50);
	zoomout->setMaximumWidth(50);

	connect(zoomin, SIGNAL(clicked(bool)),
                        m_mc, SLOT(zoomIn()));
	connect(zoomout, SIGNAL(clicked(bool)),
                        m_mc, SLOT(zoomOut()));

	// add zoom buttons to the layout of the MapControl
	QVBoxLayout* innerlayout = new QVBoxLayout;
	innerlayout->addWidget(zoomin);
	innerlayout->addWidget(zoomout);
        m_mc->setLayout(innerlayout);
}

