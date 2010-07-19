/**
 ******************************************************************************
 *
 * @file       flightgearbridge.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup HITLPlugin HITL Plugin
 * @{
 * @brief The Hardware In The Loop plugin 
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

#ifndef FLIGHTGEARBRIDGE_H
#define FLIGHTGEARBRIDGE_H

#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QTimer>
#include <math.h>
#include "uavtalk/telemetrymanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/actuatordesired.h"
#include "uavobjects/altitudeactual.h"
#include "uavobjects/attitudeactual.h"
#include "uavobjects/positionactual.h"
#include "uavobjects/gcstelemetrystats.h"

class FlightGearBridge: public QThread
{
    Q_OBJECT

public:
    FlightGearBridge();
    ~FlightGearBridge();

    bool isAutopilotConnected();
    bool isFGConnected();
    void run();

signals:
    void autopilotConnected();
    void autopilotDisconnected();
    void fgConnected();
    void fgDisconnected();

private slots:
    void transmitUpdate();
    void receiveUpdate();
    void onAutopilotConnect();
    void onAutopilotDisconnect();
    void onFGConnectionTimeout();
    void telStatsUpdated(UAVObject* obj);

private:
    static const float FT2M = 0.3048;
    static const float KT2MPS = 0.514444444;
    static const float INHG2KPA = 3.386;

    QUdpSocket* inSocket;
    QUdpSocket* outSocket;
    ActuatorDesired* actDesired;
    AltitudeActual* altActual;
    AttitudeActual* attActual;
    PositionActual* posActual;
    GCSTelemetryStats* telStats;
    QHostAddress fgHost;
    int inPort;
    int outPort;
    int updatePeriod;
    QTimer* txTimer;
    QTimer* fgTimer;
    bool autopilotConnectionStatus;
    bool fgConnectionStatus;
    int fgTimeout;

    void processUpdate(QString& data);
    void setupOutputObject(UAVObject* obj, int updatePeriod);
    void setupInputObject(UAVObject* obj, int updatePeriod);
    void setupObjects();
};

#endif // FLIGHTGEARBRIDGE_H
