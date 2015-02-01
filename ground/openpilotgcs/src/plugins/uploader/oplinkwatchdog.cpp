/**
 ******************************************************************************
 *
 * @file       oplinkwatchdog.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup [Group]
 * @{
 * @addtogroup OPLinkWatchdog
 * @{
 * @brief [Brief]
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

#include "oplinkwatchdog.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "oplinkstatus.h"
#include <QDebug>

OPLinkWatchdog::OPLinkWatchdog() : QObject(),
    m_isConnected(false)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager);
    m_oplinkStatus = OPLinkStatus::GetInstance(objManager);
    Q_ASSERT(m_oplinkStatus);
    connect(m_oplinkStatus, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(onOPLinkStatusUpdate()));
    m_watchdog = new QTimer(this);
    connect(m_watchdog, SIGNAL(timeout()), this, SLOT(onTimeout()));
    onOPLinkStatusUpdate();
}

OPLinkWatchdog::~OPLinkWatchdog()
{
}

void OPLinkWatchdog::onOPLinkStatusUpdate()
{
    m_watchdog->stop();
    quint8 type = m_oplinkStatus->getBoardType();
    if (!m_isConnected) {
        switch (type) {
        case 3:
            m_oplinkType = OPLINK_STANDALONE;
            m_isConnected = true;
            emit connected();
            emit standaloneConnected();
            break;
        case 9:
            m_oplinkType = OPLINK_REVOLUTION;
            m_isConnected = true;
            emit connected();
            emit revolutionConnected();
            break;
        default:
            m_isConnected = false;
            m_oplinkType = OPLINK_UNKNOWN;
            return;
        }

        qDebug() << "OPLink connected";
    }
    m_watchdog->start(m_oplinkStatus->getMetadata().flightTelemetryUpdatePeriod * 3);
}

void OPLinkWatchdog::onTimeout()
{
    if (m_isConnected) {
        m_isConnected = false;
        m_oplinkType = OPLINK_UNKNOWN;
        qDebug() << "OPLink disconnected";
        emit disconnected();
    }
}
