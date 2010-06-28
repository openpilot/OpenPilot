/**
 ******************************************************************************
 *
 * @file       il2bridge.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlplugin
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

#include "il2bridge.h"
#include "extensionsystem/pluginmanager.h"

Il2Bridge::Il2Bridge(QString il2HostName, int il2Port)
{
    // Init fields
    il2Host = QHostAddress(il2HostName);
    outPort = il2Port;
    updatePeriod = 50;
    il2Timeout = 2000;
    autopilotConnectionStatus = false;
    fgConnectionStatus = false;

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    actDesired = ActuatorDesired::GetInstance(objManager);
    altActual = AltitudeActual::GetInstance(objManager);
    attActual = AttitudeActual::GetInstance(objManager);
    posActual = PositionActual::GetInstance(objManager);
    telStats = GCSTelemetryStats::GetInstance(objManager);

    // Listen to autopilot connection events
    TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));
    //connect(telStats, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(telStatsUpdated(UAVObject*)));

    // If already connect setup autopilot
    GCSTelemetryStats::DataFields stats = telStats->getData();
    if ( stats.Status == GCSTelemetryStats::STATUS_CONNECTED )
    {
        onAutopilotConnect();
    }

    // Setup local ports
    outSocket = new QUdpSocket(this);
    outSocket->connectToHost((const QString )il2HostName,il2Port);
    connect(outSocket, SIGNAL(readyRead()), this, SLOT(receiveUpdate()));

    // Setup transmit timer
    txTimer = new QTimer(this);
    connect(txTimer, SIGNAL(timeout()), this, SLOT(transmitUpdate()));
    txTimer->setInterval(updatePeriod);
    txTimer->start();

    // Setup FG connection timer
    il2Timer = new QTimer(this);
    connect(il2Timer, SIGNAL(timeout()), this, SLOT(onIl2ConnectionTimeout()));
    fgTimer->setInterval(fgTimeout);
    fgTimer->start();
}

Il2Bridge::~Il2Bridge()
{
    delete outSocket;
    delete txTimer;
    delete il2Timer;
}

bool Il2Bridge::isAutopilotConnected()
{
    return autopilotConnectionStatus;
}

bool Il2Bridge::isIl2Connected()
{
    return il2ConnectionStatus;
}

void Il2Bridge::transmitUpdate()
{
    // Read ActuatorDesired from autopilot
    ActuatorDesired::DataFields actData = actDesired->getData();
    float ailerons = actData.Roll;
    float elevator = -actData.Pitch;
    float rudder = actData.Yaw;
    float throttle = actData.Throttle;

    // Send update to Il2
//    QString cmd;
//    cmd = QString("%1,%2,%3,%4\n")
//          .arg(ailerons)
//          .arg(elevator)
//          .arg(rudder)
//          .arg(throttle);
    QString cmd;
    cmd=QString("R/30/32/34/40/42/46/48/64\\0/64\\1/74\\0/74\\1/164/");
    QByteArray data = cmd.toAscii();
    outSocket->write(data);
}

void Il2Bridge::receiveUpdate()
{
    // Update connection timer and status
    il2Timer->setInterval(il2Timeout);
    il2Timer->stop();
    il2Timer->start();
    if ( !il2ConnectionStatus )
    {
        il2ConnectionStatus = true;
        emit il2Connected();
    }

    // Process data
    while ( outSocket->bytesAvailable() > 0 )
    {
        // Receive datagram
        QByteArray datagram;
        datagram.resize(inSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        inSocket->readDatagram(datagram.data(), datagram.size(),
                               &sender, &senderPort);
        QString datastr(datagram);
        // Process incomming data
        processUpdate(datastr);
     }
}

void Il2Bridge::setupObjects()
{
    setupInputObject(actDesired, 75);
    setupOutputObject(altActual, 250);
    setupOutputObject(attActual, 75);
    setupOutputObject(posActual, 250);
}

void Il2Bridge::setupInputObject(UAVObject* obj, int updatePeriod)
{
    UAVObject::Metadata mdata;
    mdata = obj->getDefaultMetadata();
    mdata.flightAccess = UAVObject::ACCESS_READWRITE;
    mdata.gcsAccess = UAVObject::ACCESS_READWRITE;
    mdata.flightTelemetryAcked = false;
    mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    mdata.flightTelemetryUpdatePeriod = updatePeriod;
    mdata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    obj->setMetadata(mdata);
}

void Il2Bridge::setupOutputObject(UAVObject* obj, int updatePeriod)
{
    UAVObject::Metadata mdata;
    mdata = obj->getDefaultMetadata();
    mdata.flightAccess = UAVObject::ACCESS_READONLY;
    mdata.gcsAccess = UAVObject::ACCESS_READWRITE;
    mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_NEVER;
    mdata.gcsTelemetryAcked = false;
    mdata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    mdata.gcsTelemetryUpdatePeriod = updatePeriod;
    obj->setMetadata(mdata);
}

void Il2Bridge::onAutopilotConnect()
{
    autopilotConnectionStatus = true;
    setupObjects();
    emit autopilotConnected();
}

void Il2Bridge::onAutopilotDisconnect()
{
    autopilotConnectionStatus = false;
    emit autopilotDisconnected();
}

void Il2Bridge::onFGConnectionTimeout()
{
    if ( fgConnectionStatus )
    {
        fgConnectionStatus = false;
        emit fgDisconnected();
    }
}

void Il2Bridge::processUpdate(QString& data)
{
return;
    // Split
    QStringList fields = data.split(",");
    // Get xRate (deg/s)
    float xRate = fields[0].toFloat() * 180.0/M_PI;
    // Get yRate (deg/s)
    float yRate = fields[1].toFloat() * 180.0/M_PI;
    // Get zRate (deg/s)
    float zRate = fields[2].toFloat() * 180.0/M_PI;
    // Get xAccel (m/s^2)
    float xAccel = fields[3].toFloat() * FT2M;
    // Get yAccel (m/s^2)
    float yAccel = fields[4].toFloat() * FT2M;
    // Get xAccel (m/s^2)
    float zAccel = fields[5].toFloat() * FT2M;
    // Get pitch (deg)
    float pitch = fields[6].toFloat();
    // Get pitchRate (deg/s)
    float pitchRate = fields[7].toFloat();
    // Get roll (deg)
    float roll = fields[8].toFloat();
    // Get rollRate (deg/s)
    float rollRate = fields[9].toFloat();
    // Get yaw (deg)
    float yaw = fields[10].toFloat();
    // Get yawRate (deg/s)
    float yawRate = fields[11].toFloat();
    // Get latitude (deg)
    float latitude = fields[12].toFloat();
    // Get longitude (deg)
    float longitude = fields[13].toFloat();
    // Get heading (deg)
    float heading = fields[14].toFloat();
    // Get altitude (m)
    float altitude = fields[15].toFloat() * FT2M;
    // Get altitudeAGL (m)
    float altitudeAGL = fields[16].toFloat() * FT2M;
    // Get groundspeed (m/s)
    float groundspeed = fields[17].toFloat() * KT2MPS;
    // Get airspeed (m/s)
    float airspeed = fields[18].toFloat() * KT2MPS;
    // Get temperature (degC)
    float temperature = fields[19].toFloat();
    // Get pressure (kpa)
    float pressure = fields[20].toFloat() * INHG2KPA;

    // Update AltitudeActual object
    AltitudeActual::DataFields altActualData;
    altActualData.Altitude = altitudeAGL;
    altActualData.Temperature = temperature;
    altActualData.Pressure = pressure;
    altActual->setData(altActualData);

    // Update attActual object
    AttitudeActual::DataFields attActualData;
    attActualData.Roll = roll;
    attActualData.Pitch = pitch;
    attActualData.Yaw = yaw;
    attActualData.q1 = 0;
    attActualData.q2 = 0;
    attActualData.q3 = 0;
    attActualData.q4 = 0;
    attActualData.seq = 0;
    attActual->setData(attActualData);

    // Update gps objects
    PositionActual::DataFields gpsData;
    gpsData.Altitude = altitude;
    gpsData.Heading = heading;
    gpsData.Groundspeed = groundspeed;
    gpsData.Latitude = latitude;
    gpsData.Longitude = longitude;
    gpsData.Satellites = 10;
    gpsData.Status = PositionActual::STATUS_FIX3D;
    posActual->setData(gpsData);
}

void Il2Bridge::telStatsUpdated(UAVObject* obj)
{
    GCSTelemetryStats::DataFields stats = telStats->getData();
    if ( !autopilotConnectionStatus && stats.Status == GCSTelemetryStats::STATUS_CONNECTED )
    {
        onAutopilotConnect();
    }
    else if ( autopilotConnectionStatus && stats.Status != GCSTelemetryStats::STATUS_CONNECTED )
    {
        onAutopilotDisconnect();
    }
}
