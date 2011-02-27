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

#include <QMutexLocker>	// Pip

TelemetryManager::TelemetryManager()
{
	telemetryMon = NULL;	// Pip
	telemetry = NULL;		// Pip
	utalk = NULL;			// Pip

	moveToThread(Core::ICore::instance()->threadManager()->getRealTimeThread());

	mutex = new QMutex(QMutex::Recursive);	// Pip

	QMutexLocker locker(mutex);	// Pip

	// Get UAVObjectManager instance
	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	if (pm)	// Pip
	{
		objMngr = pm->getObject<UAVObjectManager>();
		if (objMngr)
			connect(objMngr, SIGNAL(destroyed(QObject *)), this, SLOT(onObjectDestroyed(QObject *)));	// Pip
	}

	// connect to start stop signals
	connect(this, SIGNAL(myStart()), this, SLOT(onStart()), Qt::QueuedConnection);
	connect(this, SIGNAL(myStop()), this, SLOT(onStop()), Qt::QueuedConnection);
}

TelemetryManager::~TelemetryManager()
{
	// Pip
	mutex->lock();
//		deleteObjects();
	mutex->unlock();
}

void TelemetryManager::onObjectDestroyed(QObject *obj)		// Pip
{
	QMutexLocker locker(mutex);
	deleteObjects();
}

void TelemetryManager::start(QIODevice *dev)
{
	device = dev;
    emit myStart();
}

void TelemetryManager::onStart()
{
	QMutexLocker locker(mutex);	// Pip

	deleteObjects();	// Pip

	// Get UAVObjectManager instance
	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	if (!pm) return;	// Pip
	objMngr = pm->getObject<UAVObjectManager>();
	if (!objMngr) return;	// Pip

	utalk = new UAVTalk(device, objMngr);
	if (utalk) telemetry = new Telemetry(utalk, objMngr);
	if (telemetry) telemetryMon = new TelemetryMonitor(objMngr, telemetry);

	if (!utalk || !telemetry || !telemetryMon)	// Pip
	{
		deleteObjects();
		return;
	}

	// Pip
	connect(objMngr, SIGNAL(destroyed(QObject *)), this, SLOT(onObjectDestroyed(QObject *)));
	connect(utalk, SIGNAL(destroyed(QObject *)), this, SLOT(onObjectDestroyed(QObject *)));
	connect(telemetry, SIGNAL(destroyed(QObject *)), this, SLOT(onObjectDestroyed(QObject *)));
	connect(telemetryMon, SIGNAL(destroyed(QObject *)), this, SLOT(onObjectDestroyed(QObject *)));

	connect(telemetryMon, SIGNAL(connected()), this, SLOT(onConnect()));
	connect(telemetryMon, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
}

void TelemetryManager::stop()
{
    emit myStop();
}

void TelemetryManager::onStop()
{
	mutex->lock();			// Pip
		if (telemetryMon) telemetryMon->disconnect(this);
		deleteObjects();
	mutex->unlock();

	onDisconnect();
}

void TelemetryManager::onConnect()
{
    emit connected();
}

void TelemetryManager::onDisconnect()
{
    emit disconnected();
}

void TelemetryManager::deleteObjects()	// Pip
{
	if (!objMngr) return;	// we've already destroyed everything

	objMngr = NULL;

	if (telemetryMon)
	{
		disconnect(telemetryMon, SIGNAL(destroyed(QObject *)), this, 0);
		delete telemetryMon;
		telemetryMon = NULL;
	}

	if (telemetry)
	{
		disconnect(telemetry, SIGNAL(destroyed(QObject *)), this, 0);
		delete telemetry;
		telemetry = NULL;
	}

	if (utalk)
	{
		disconnect(utalk, SIGNAL(destroyed(QObject *)), this, 0);
		delete utalk;
		utalk = NULL;
	}
}
