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

#include "flightgearbridge.h"
#include "extensionsystem/pluginmanager.h"

FlightGearBridge::FlightGearBridge()
{
    // Init fields
    fgHost = QHostAddress("127.0.0.1");
    inPort = 5500;
    outPort = 5501;
    updatePeriod = 50;
    fgTimeout = 2000;
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
    inSocket = new QUdpSocket(this);
    outSocket = new QUdpSocket(this);
    inSocket->bind(QHostAddress::Any, inPort);
    connect(inSocket, SIGNAL(readyRead()), this, SLOT(receiveUpdate()));

    // Setup transmit timer
    txTimer = new QTimer(this);
    connect(txTimer, SIGNAL(timeout()), this, SLOT(transmitUpdate()));
    txTimer->setInterval(updatePeriod);
    txTimer->start();

    // Setup FG connection timer
    fgTimer = new QTimer(this);
    connect(fgTimer, SIGNAL(timeout()), this, SLOT(onFGConnectionTimeout()));
    fgTimer->setInterval(fgTimeout);
    fgTimer->start();
}

FlightGearBridge::~FlightGearBridge()
{
    delete inSocket;
    delete outSocket;
    delete txTimer;
    delete fgTimer;
}

bool FlightGearBridge::isAutopilotConnected()
{
    return autopilotConnectionStatus;
}

bool FlightGearBridge::isFGConnected()
{
    return fgConnectionStatus;
}

void FlightGearBridge::transmitUpdate()
{
    // Read ActuatorDesired from autopilot
    ActuatorDesired::DataFields actData = actDesired->getData();
    float ailerons = actData.Roll;
    float elevator = -actData.Pitch;
    float rudder = actData.Yaw;
    float throttle = actData.Throttle;

    // Send update to FlightGear
    QString cmd;
    cmd = QString("%1,%2,%3,%4\n")
          .arg(ailerons)
          .arg(elevator)
          .arg(rudder)
          .arg(throttle);
    QByteArray data = cmd.toAscii();
    outSocket->writeDatagram(data, fgHost, outPort);
}

void FlightGearBridge::receiveUpdate()
{
    // Update connection timer and status
    fgTimer->setInterval(fgTimeout);
    fgTimer->stop();
    fgTimer->start();
    if ( !fgConnectionStatus )
    {
        fgConnectionStatus = true;
        emit fgConnected();
    }

    // Process data
    while ( inSocket->bytesAvailable() > 0 )
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

void FlightGearBridge::setupObjects()
{
    setupInputObject(actDesired, 75);
    setupOutputObject(altActual, 250);
    setupOutputObject(attActual, 75);
    setupOutputObject(posActual, 250);
}

void FlightGearBridge::setupInputObject(UAVObject* obj, int updatePeriod)
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

void FlightGearBridge::setupOutputObject(UAVObject* obj, int updatePeriod)
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

void FlightGearBridge::onAutopilotConnect()
{
    autopilotConnectionStatus = true;
    setupObjects();
    emit autopilotConnected();
}

void FlightGearBridge::onAutopilotDisconnect()
{
    autopilotConnectionStatus = false;
    emit autopilotDisconnected();
}

void FlightGearBridge::onFGConnectionTimeout()
{
    if ( fgConnectionStatus )
    {
        fgConnectionStatus = false;
        emit fgDisconnected();
    }
}

void FlightGearBridge::processUpdate(QString& data)
{
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

void FlightGearBridge::telStatsUpdated(UAVObject* obj)
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
