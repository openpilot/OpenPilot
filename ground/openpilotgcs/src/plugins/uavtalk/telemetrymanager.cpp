/**
 ******************************************************************************
 *
 * @file       telemetrymanager.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVTalkPlugin UAVTalk Plugin
 * @{
 * @brief The UAVTalk protocol plugin
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

#include "telemetrymanager.h"
#include "telemetry.h"
#include "telemetrymonitor.h"
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/threadmanager.h>

TelemetryManager::TelemetryManager() : m_isAutopilotConnected(false)
{
    moveToThread(Core::ICore::instance()->threadManager()->getRealTimeThread());
    // Get UAVObjectManager instance
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_uavobjectManager = pm->getObject<UAVObjectManager>();

    // connect to start stop signals
    connect(this, SIGNAL(myStart()), this, SLOT(onStart()), Qt::QueuedConnection);
    connect(this, SIGNAL(myStop()), this, SLOT(onStop()), Qt::QueuedConnection);
}

TelemetryManager::~TelemetryManager()
{}

bool TelemetryManager::isConnected()
{
    return m_isAutopilotConnected;
}

bool TelemetryManager::isObjectKnown(UAVObject *object) const
{
    return m_knownObjects.contains(object);
}

void TelemetryManager::start(QIODevice *dev)
{
    m_telemetryDevice = dev;
    // OP-1383
    // take ownership of the device by moving it to the TelemetryManager thread (see TelemetryManager constructor)
    // this removes the following runtime Qt warning and incidentally fixes GCS crashes:
    // QObject: Cannot create children for a parent that is in a different thread.
    // (Parent is QSerialPort(0x56af73f8), parent's thread is QThread(0x23f69ae8), current thread is QThread(0x2649cfd8)
    m_telemetryDevice->moveToThread(thread());
    emit myStart();
}

void TelemetryManager::onStart()
{
    m_uavTalk = new UAVTalk(m_telemetryDevice, m_uavobjectManager);
    if (false) {
        // UAVTalk must be thread safe and for that:
        // 1- all public methods must lock a mutex
        // 2- the reader thread must lock that mutex too
        // The reader thread locks the mutex once a packet is read and decoded.
        // It is assumed that the UAVObjectManager is thread safe

        // Create the reader and move it to the reader thread
        IODeviceReader *reader = new IODeviceReader(m_uavTalk);
        reader->moveToThread(&m_telemetryReaderThread);
        // The reader will be deleted (later) when the thread finishes
        connect(&m_telemetryReaderThread, &QThread::finished, reader, &QObject::deleteLater);
        // Connect IO device to reader
        connect(m_telemetryDevice, SIGNAL(readyRead()), reader, SLOT(read()));
        // start the reader thread
        m_telemetryReaderThread.start();
    } else {
        // Connect IO device to reader
        connect(m_telemetryDevice, SIGNAL(readyRead()), m_uavTalk, SLOT(processInputStream()));
    }

    m_telemetry    = new Telemetry(this, m_uavTalk, m_uavobjectManager);
    m_telemetryMonitor = new TelemetryMonitor(m_uavobjectManager, m_telemetry);

    connect(m_telemetryMonitor, SIGNAL(connected()), this, SLOT(onConnect()));
    connect(m_telemetryMonitor, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(m_telemetryMonitor, SIGNAL(telemetryUpdated(double, double)), this, SLOT(onTelemetryUpdate(double, double)));
}

void TelemetryManager::stop()
{
    emit myStop();

    if (false) {
        m_telemetryReaderThread.quit();
        m_telemetryReaderThread.wait();
    }
}

void TelemetryManager::onStop()
{
    foreach (UAVObject *object, m_knownObjects) {
        onKnownObjectsChanged(object, false);
    }
    m_telemetryMonitor->disconnect(this);
    delete m_telemetryMonitor;
    delete m_telemetry;
    delete m_uavTalk;
    onDisconnect();
}

void TelemetryManager::onKnownObjectsChanged(UAVObject *object, bool known)
{
    bool contains = m_knownObjects.contains(object);
    if (known && !contains) {
        m_knownObjects.insert(object);
        emit knownObjectsChanged(object, known);
    } else if (contains) {
        m_knownObjects.remove(object);
        emit knownObjectsChanged(object, known);
    }
}

void TelemetryManager::onConnect()
{
    m_isAutopilotConnected = true;
    emit connected();
}

void TelemetryManager::onDisconnect()
{
    m_isAutopilotConnected = false;
    emit disconnected();
}

void TelemetryManager::onTelemetryUpdate(double txRate, double rxRate)
{
    emit telemetryUpdated(txRate, rxRate);
}

IODeviceReader::IODeviceReader(UAVTalk *uavTalk) : m_uavTalk(uavTalk)
{}

void IODeviceReader::read()
{
    m_uavTalk->processInputStream();
}
