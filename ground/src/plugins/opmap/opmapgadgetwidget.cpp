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
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include "extensionsystem/pluginmanager.h"

// *************************************************************************************
// constructor

OPMapGadgetWidget::OPMapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    int size = 256;

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    m_positionActual = PositionActual::GetInstance(objManager);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    map = NULL;
//    map = new mapcontrol::OPMapWidget();
//    map->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
//    map->setMinimumSize(64, 64);

    PositionActual::DataFields data = m_positionActual->getData();  // get current position data

//   QVBoxLayout *layout = new QVBoxLayout;
//    layout->setSpacing(0);
//    layout->setContentsMargins(0, 0, 0, 0);
//    layout->addWidget(map);
//    setLayout(layout);

//    m_updateTimer = new QTimer();
//    m_updateTimer->setInterval(250);
//    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));
//    m_updateTimer->start();
}

// *************************************************************************************
// destructor

OPMapGadgetWidget::~OPMapGadgetWidget()
{
    if (map) delete map;
   // Do nothing
}

// *************************************************************************************

void OPMapGadgetWidget::setZoom(int value)
{
//    map->setZoom(value);
//    map->updateRequestNew();
}

void OPMapGadgetWidget::setPosition(QPointF pos)
{
//    map->setView(pos);
//    map->updateRequestNew();
}

void OPMapGadgetWidget::setMapProvider(QString provider)
{
//  map->
}

// *************************************************************************************

void OPMapGadgetWidget::updatePosition()
{
    PositionActual::DataFields data = m_positionActual->getData();	// get current position data

//    uavPoint->setCoordinate(QPointF(data.Longitude, data.Latitude));	// move the UAV icon
//
//    if (follow_uav)
//	setPosition(uavPoint->coordinate());				// center the map onto the UAV
}

// *************************************************************************************

void OPMapGadgetWidget::resizeEvent(QResizeEvent *event)
{
//    map->resize(QSize(width(), height()));
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
