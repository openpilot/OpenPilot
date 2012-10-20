/**
 ******************************************************************************
 *
 * @file       telemetrymonitor.cpp
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

#ifndef TELEMETRYMONITOR_H
#define TELEMETRYMONITOR_H

#include <QObject>
#include <QQueue>
#include <QTimer>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>
#include "uavobjectmanager.h"
#include "gcstelemetrystats.h"
#include "flighttelemetrystats.h"
#include "systemstats.h"
#include "telemetry.h"

class TelemetryMonitor : public QObject
{
    Q_OBJECT

public:
    TelemetryMonitor(UAVObjectManager* objMngr, Telemetry* tel);
    ~TelemetryMonitor();

signals:
    void connected();
    void disconnected();
    void telemetryUpdated(double txRate, double rxRate);

public slots:
    void transactionCompleted(UAVObject* obj, bool success);
    void processStatsUpdates();
    void flightStatsUpdated(UAVObject* obj);

private:
    static const int STATS_UPDATE_PERIOD_MS = 4000;
    static const int STATS_CONNECT_PERIOD_MS = 2000;
    static const int CONNECTION_TIMEOUT_MS = 8000;

    UAVObjectManager* objMngr;
    Telemetry* tel;
    QQueue<UAVObject*> queue;
    GCSTelemetryStats* gcsStatsObj;
    FlightTelemetryStats* flightStatsObj;
    QTimer* statsTimer;
    UAVObject* objPending;
    QMutex* mutex;
    QTime* connectionTimer;

    void startRetrievingObjects();
    void retrieveNextObject();
    void stopRetrievingObjects();
};

#endif // TELEMETRYMONITOR_H
