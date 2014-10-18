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

#ifndef TELEMETRYMANAGER_H
#define TELEMETRYMANAGER_H

#include "uavtalk_global.h"
#include "uavtalk.h"
#include "uavobjectmanager.h"
#include <QIODevice>
#include <QObject>

class Telemetry;
class TelemetryMonitor;

class UAVTALK_EXPORT TelemetryManager : public QObject {
    Q_OBJECT

public:
    TelemetryManager();
    ~TelemetryManager();

    void start(QIODevice *dev);
    void stop();
    bool isConnected();

signals:
    void connected();
    void disconnected();
    void telemetryUpdated(double txRate, double rxRate);
    void myStart();
    void myStop();

private slots:
    void onConnect();
    void onDisconnect();
    void onTelemetryUpdate(double txRate, double rxRate);
    void onStart();
    void onStop();

private:
    UAVObjectManager *m_uavobjectManager;
    UAVTalk *m_uavTalk;
    Telemetry *m_telemetry;
    TelemetryMonitor *m_telemetryMonitor;
    QIODevice *m_telemetryDevice;
    bool m_isAutopilotConnected;
    QThread m_telemetryReaderThread;
};


class IODeviceReader : public QObject {
    Q_OBJECT
public:
    IODeviceReader(UAVTalk *uavTalk);

    UAVTalk *m_uavTalk;

public slots:
    void read();
};

#endif // TELEMETRYMANAGER_H
