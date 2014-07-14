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
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/threadmanager.h>

TelemetryManager::TelemetryManager() : autopilotConnected(false)
{
    moveToThread(Core::ICore::instance()->threadManager()->getRealTimeThread());
    // Get UAVObjectManager instance
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    objMngr = pm->getObject<UAVObjectManager>();

    // connect to start stop signals
    connect(this, SIGNAL(myStart()), this, SLOT(onStart()), Qt::QueuedConnection);
    connect(this, SIGNAL(myStop()), this, SLOT(onStop()), Qt::QueuedConnection);
}

TelemetryManager::~TelemetryManager()
{}

bool TelemetryManager::isConnected()
{
    return autopilotConnected;
}

void TelemetryManager::start(QIODevice *dev)
{
    device = dev;
    // take ownership of the device (the why is not clear?)
    device->moveToThread(Core::ICore::instance()->threadManager()->getRealTimeThread());
    emit myStart();
}

void TelemetryManager::onStart()
{
    utalk = new UAVTalk(device, objMngr);
    if (false) {
        // UAVTalk must be thread safe and for that:
        // 1- all public methods must lock a mutex
        // 2- the reader thread must lock that mutex too
        // The reader thread locks the mutex once a packet is read and decoded.
        // It is assumed that the UAVObjectManager is thread safe

        // Create the reader and move it to the reader thread
        IODeviceReader *reader = new IODeviceReader(utalk);
        reader->moveToThread(&readerThread);
        // The reader will be deleted (later) when the thread finishes
        connect(&readerThread, &QThread::finished, reader, &QObject::deleteLater);
        // Connect IO device to reader
        connect(device, SIGNAL(readyRead()), reader, SLOT(read()));
        // start the reader thread
        readerThread.start();
    } else {
        // Connect IO device to reader
        connect(device, SIGNAL(readyRead()), utalk, SLOT(processInputStream()));
    }

    telemetry    = new Telemetry(utalk, objMngr);
    telemetryMon = new TelemetryMonitor(objMngr, telemetry);

    connect(telemetryMon, SIGNAL(connected()), this, SLOT(onConnect()));
    connect(telemetryMon, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(telemetryMon, SIGNAL(telemetryUpdated(double, double)), this, SLOT(onTelemetryUpdate(double, double)));
}

void TelemetryManager::stop()
{
    emit myStop();

    if (false) {
        readerThread.quit();
        readerThread.wait();
    }
}

void TelemetryManager::onStop()
{
    telemetryMon->disconnect(this);
    delete telemetryMon;
    delete telemetry;
    delete utalk;
    onDisconnect();
}

void TelemetryManager::onConnect()
{
    autopilotConnected = true;
    emit connected();
}

void TelemetryManager::onDisconnect()
{
    autopilotConnected = false;
    emit disconnected();
}

void TelemetryManager::onTelemetryUpdate(double txRate, double rxRate)
{
    emit telemetryUpdated(txRate, rxRate);
}

IODeviceReader::IODeviceReader(UAVTalk *uavTalk) : uavTalk(uavTalk)
{}

void IODeviceReader::read()
{
    uavTalk->processInputStream();
}
