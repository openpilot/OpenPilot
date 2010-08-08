/**
 ******************************************************************************
 *
 * @file       il2bridge.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlil2plugin
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

#ifndef IL2BRIDGE_H
#define IL2BRIDGE_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QTime>
#include <QMessageBox>
#include <math.h>
#include "uavtalk/telemetrymanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/actuatordesired.h"
#include "uavobjects/altitudeactual.h"
#include "uavobjects/attitudeactual.h"
#include "uavobjects/positionactual.h"
#include "uavobjects/gcstelemetrystats.h"


/**
 * just imagine this was a class without methods and all public properties
 */
struct flightParams {

	// time
	float T;
	float dT;

	// speed (relative)
	float ias;
	float tas;
	float groundspeed;

	// position (absolute)
	float X;
	float Y;
	float Z;

	// speed (absolute)
	float dX;
	float dY;
	float dZ;

	//angle
	float azimuth;
	float pitch;
	float roll;

};


class Il2Bridge: public QObject
{
    Q_OBJECT

public:
    Il2Bridge(QString il2HostName, int il2Port, QString il2Latitude, QString il2Longitude);
    ~Il2Bridge();

    bool isAutopilotConnected();
    bool isIl2Connected();

signals:
    void myStart();
    void autopilotConnected();
    void autopilotDisconnected();
    void il2Connected();
    void il2Disconnected();

private slots:
    void onStart();
    void transmitUpdate();
    void receiveUpdate();
    void onAutopilotConnect();
    void onAutopilotDisconnect();
    void onIl2ConnectionTimeout();
    void telStatsUpdated(UAVObject* obj);

private:
    static const float FT2M;
    static const float KT2MPS;
    static const float MPS2KMH;
    static const float KMH2MPS;
    static const float INHG2KPA;
    static const float RAD2DEG;
    static const float DEG2RAD;
    static const float M2DEG;
    static const float DEG2M;
    static const float AIR_CONST;
    static const float GROUNDDENSITY;
    static const float TEMP_GROUND;
    static const float TEMP_LAPSE_RATE;
    static const float AIR_CONST_FACTOR;

    struct flightParams current;
    struct flightParams old;
    QUdpSocket* outSocket;
    ActuatorDesired* actDesired;
    AltitudeActual* altActual;
    AttitudeActual* attActual;
    PositionActual* posActual;
    GCSTelemetryStats* telStats;
    QHostAddress il2Host;
    float latitude;
    float longitude;
    int outPort;
    int updatePeriod;
    QTimer* txTimer;
    QTimer* il2Timer;
    QTime* time;
    bool autopilotConnectionStatus;
    bool il2ConnectionStatus;
    int il2Timeout;

    float DENSITY(float pressure);
    float PRESSURE(float alt);
    float TAS(float ias,float alt);
    void processUpdate(QString& data);
    void setupOutputObject(UAVObject* obj, int updatePeriod);
    void setupInputObject(UAVObject* obj, int updatePeriod);
    void setupObjects();
};

#endif // IL2BRIDGE_H
