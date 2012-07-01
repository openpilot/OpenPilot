/**
 ******************************************************************************
 *
 * @file       simulator.cpp
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


#include "simulator.h"
#include "qxtlogger.h"
#include "extensionsystem/pluginmanager.h"
#include "coreplugin/icore.h"
#include "coreplugin/threadmanager.h"
#include "hitlnoisegeneration.h"
#include <QDebug>

volatile bool Simulator::isStarted = false;

const float Simulator::GEE = 9.81;
const float Simulator::FT2M = 0.3048;
const float Simulator::KT2MPS = 0.514444444;
const float Simulator::INHG2KPA = 3.386;
const float Simulator::FPS2CMPS = 30.48;
const float Simulator::DEG2RAD = (M_PI/180.0);


Simulator::Simulator(const SimulatorSettings& params) :
    simProcess(NULL),
    time(NULL),
    inSocket(NULL),
    outSocket(NULL),
    settings(params),
        updatePeriod(50),
        simTimeout(8000),
    autopilotConnectionStatus(false),
    simConnectionStatus(false),
    txTimer(NULL),
    simTimer(NULL),
    name(""),
    once(false)
{
    // move to thread
    moveToThread(Core::ICore::instance()->threadManager()->getRealTimeThread());
        connect(this, SIGNAL(myStart()), this, SLOT(onStart()),Qt::QueuedConnection);
    emit myStart();
}

Simulator::~Simulator()
{
    if(inSocket)
    {
        delete inSocket;
        inSocket = NULL;
    }

    if(outSocket)
    {
        delete outSocket;
        outSocket = NULL;
    }

    if(txTimer)
    {
        delete txTimer;
        txTimer = NULL;
    }

    if(simTimer)
    {
        delete simTimer;
        simTimer = NULL;
    }
    // NOTE: Does not currently work, may need to send control+c to through the terminal
    if (simProcess != NULL)
    {
        //connect(simProcess,SIGNAL(finished(int, QProcess::ExitStatus)),this,SLOT(onFinished(int, QProcess::ExitStatus)));

        simProcess->disconnect();
        if(simProcess->state() == QProcess::Running)
            simProcess->kill();
        //if(simProcess->waitForFinished())
            //emit deleteSimProcess();
        delete simProcess;
        simProcess = NULL;
    }
}

void Simulator::onDeleteSimulator(void)
{
    // [1]
    Simulator::setStarted(false);
    // [2]
    Simulator::Instances().removeOne(simulatorId);

    disconnect(this);
    delete this;
}

void Simulator::onStart()
{
    QMutexLocker locker(&lock);

        QThread* mainThread = QThread::currentThread();
    qDebug() << "Simulator Thread: "<< mainThread;

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    actDesired = ActuatorDesired::GetInstance(objManager);
    manCtrlCommand = ManualControlCommand::GetInstance(objManager);
    flightStatus = FlightStatus::GetInstance(objManager);
    posHome = HomeLocation::GetInstance(objManager);
    velActual = VelocityActual::GetInstance(objManager);
    posActual = PositionActual::GetInstance(objManager);
    baroAlt = BaroAltitude::GetInstance(objManager);
    baroAirspeed = BaroAirspeed::GetInstance(objManager);
    attActual = AttitudeActual::GetInstance(objManager);
    accels = Accels::GetInstance(objManager);
    gyros = Gyros::GetInstance(objManager);
    gpsPos = GPSPosition::GetInstance(objManager);
    gpsVel = GPSVelocity::GetInstance(objManager);
    telStats = GCSTelemetryStats::GetInstance(objManager);

    // Listen to autopilot connection events
    TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));
    //connect(telStats, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(telStatsUpdated(UAVObject*)));

    // If already connect setup autopilot
    GCSTelemetryStats::DataFields stats = telStats->getData();
    if ( stats.Status == GCSTelemetryStats::STATUS_CONNECTED )
        onAutopilotConnect();

    inSocket = new QUdpSocket();
    outSocket = new QUdpSocket();
    setupUdpPorts(settings.hostAddress,settings.inPort,settings.outPort);

        emit processOutput("\nLocal interface: " + settings.hostAddress + "\n" + \
                           "Remote interface: " + settings.remoteHostAddress + "\n" + \
                           "inputPort: " + QString::number(settings.inPort) + "\n" + \
                           "outputPort: " + QString::number(settings.outPort) + "\n");

        qxtLog->info("\nLocal interface: " + settings.hostAddress + "\n" + \
                     "Remote interface: " + settings.remoteHostAddress + "\n" + \
                     "inputPort: " + QString::number(settings.inPort) + "\n" + \
                     "outputPort: " + QString::number(settings.outPort) + "\n");

//        if(!inSocket->waitForConnected(5000))
//                emit processOutput(QString("Can't connect to %1 on %2 port!").arg(settings.hostAddress).arg(settings.inPort));
//        outSocket->connectToHost(settings.hostAddress,settings.outPort); // FG
//        if(!outSocket->waitForConnected(5000))
//                emit processOutput(QString("Can't connect to %1 on %2 port!").arg(settings.hostAddress).arg(settings.outPort));


    connect(inSocket, SIGNAL(readyRead()), this, SLOT(receiveUpdate()),Qt::DirectConnection);

    // Setup transmit timer
    txTimer = new QTimer();
    connect(txTimer, SIGNAL(timeout()), this, SLOT(transmitUpdate()),Qt::DirectConnection);
    txTimer->setInterval(updatePeriod);
    txTimer->start();
    // Setup simulator connection timer
    simTimer = new QTimer();
    connect(simTimer, SIGNAL(timeout()), this, SLOT(onSimulatorConnectionTimeout()),Qt::DirectConnection);
    simTimer->setInterval(simTimeout);
    simTimer->start();

    // setup time
    time = new QTime();
    time->start();
    current.T=0;
    current.i=0;

}

void Simulator::receiveUpdate()
{
    // Update connection timer and status
    simTimer->setInterval(simTimeout);
    simTimer->stop();
    simTimer->start();
    if ( !simConnectionStatus )
    {
        simConnectionStatus = true;
        emit simulatorConnected();
    }

    // Process data
        while(inSocket->hasPendingDatagrams()) {
        // Receive datagram
        QByteArray datagram;
        datagram.resize(inSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        inSocket->readDatagram(datagram.data(), datagram.size(),
                               &sender, &senderPort);
        //QString datastr(datagram);
        // Process incomming data
        processUpdate(datagram);
     }
}

void Simulator::setupObjects()
{


    setupInputObject(actDesired, 100); //100ms update period
    setupOutputObject(baroAlt, 100); //250ms update period
    setupOutputObject(baroAirspeed, 100); //250ms update period
    setupOutputObject(attActual, 10); //etc...
    setupOutputObject(gpsPos, 100);
    setupOutputObject(gpsVel, 100);
    setupOutputObject(posActual, 250);
    setupOutputObject(velActual, 250);
//    setupOutputObject(posHome, 1000); //WE DON'T WANT TO UPDATE HOME
    setupOutputObject(accels, 10);
    setupOutputObject(gyros, 10);
}

void Simulator::setupInputObject(UAVObject* obj, int updatePeriod)
{
    UAVObject::Metadata mdata;
    mdata = obj->getDefaultMetadata();
    UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READWRITE);
    UAVObject::SetGcsAccess(mdata, UAVObject::ACCESS_READWRITE);
    UAVObject::SetFlightTelemetryAcked(mdata, false);
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = updatePeriod;
    UAVObject::SetGcsTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_MANUAL);
    obj->setMetadata(mdata);
}

void Simulator::setupOutputObject(UAVObject* obj, int updatePeriod)
{
    UAVObject::Metadata mdata;
    mdata = obj->getDefaultMetadata();
    UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READONLY);
    UAVObject::SetGcsAccess(mdata, UAVObject::ACCESS_READWRITE);
    UAVObject::SetFlightTelemetryUpdateMode(mdata,UAVObject::UPDATEMODE_MANUAL);
    UAVObject::SetGcsTelemetryAcked(mdata, false);
    UAVObject::SetGcsTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.gcsTelemetryUpdatePeriod = updatePeriod;
    obj->setMetadata(mdata);
}

void Simulator::onAutopilotConnect()
{
    autopilotConnectionStatus = true;
    setupObjects();
    emit autopilotConnected();
}

void Simulator::onAutopilotDisconnect()
{
    autopilotConnectionStatus = false;
    emit autopilotDisconnected();
}

void Simulator::onSimulatorConnectionTimeout()
{
    if ( simConnectionStatus )
    {
        simConnectionStatus = false;
        emit simulatorDisconnected();
    }
}


void Simulator::telStatsUpdated(UAVObject* obj)
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

void Simulator::resetInitialHomePosition(){
    once=false;
}

void Simulator::updateUAVOs(Output2OP out){

    Noise noise;
    HitlNoiseGeneration noiseSource;
    if(settings.addNoise){
        noise = noiseSource.generateNoise();
    }
    else{
        memset(&noise, 0, sizeof(Noise));
    }

    HomeLocation::DataFields homeData = posHome->getData();
    if(!once)
    {
        // Upon startup, we reset the HomeLocation object to
        // the plane's location:
        memset(&homeData, 0, sizeof(HomeLocation::DataFields));
        // Update homelocation
        homeData.Latitude = out.latitude;   //Already in *10^7 integer format
        homeData.Longitude = out.longitude; //Already in *10^7 integer format
        homeData.Altitude = out.altitude;
        double LLA[3];
        LLA[0]=out.latitude;
        LLA[1]=out.longitude;
        LLA[2]=out.altitude;
        double ECEF[3];
        double RNE[9];
        Utils::CoordinateConversions().RneFromLLA(LLA,(double (*)[3])RNE);
        Utils::CoordinateConversions().LLA2ECEF(LLA,ECEF);
        homeData.Be[0]=0;
        homeData.Be[1]=0;
        homeData.Be[2]=0;
        posHome->setData(homeData);
        posHome->updated();

        // Compute initial distance
        initN = out.dstN;
        initE = out.dstE;
        initD = out.dstD;

        once=true;
    }

    // Update attActual object
    AttitudeActual::DataFields attActualData;
    memset(&attActualData, 0, sizeof(AttitudeActual::DataFields));
    attActualData.Roll = out.roll + noise.attActualData.Roll;   //roll;
    attActualData.Pitch = out.pitch + noise.attActualData.Pitch;  // pitch
    attActualData.Yaw = out.heading + noise.attActualData.Yaw; // Yaw
    float rpy[3];
    float quat[4];
    rpy[0] = attActualData.Roll;
    rpy[1] = attActualData.Pitch;
    rpy[2] = attActualData.Yaw;
    Utils::CoordinateConversions().RPY2Quaternion(rpy,quat);
    attActualData.q1 = quat[0];
    attActualData.q2 = quat[1];
    attActualData.q3 = quat[2];
    attActualData.q4 = quat[3];
    attActual->setData(attActualData);

    // Update GPS Position objects
    GPSPosition::DataFields gpsPosData;
    memset(&gpsPosData, 0, sizeof(GPSPosition::DataFields));
    gpsPosData.Altitude = out.altitude + noise.gpsPosData.Altitude;
    gpsPosData.Heading = out.heading + noise.gpsPosData.Heading;
    gpsPosData.Groundspeed = out.groundspeed + noise.gpsPosData.Groundspeed;
    gpsPosData.Latitude = out.latitude + noise.gpsPosData.Latitude;    //Already in *10^7 integer format
    gpsPosData.Longitude = out.longitude + noise.gpsPosData.Longitude; //Already in *10^7 integer format
    gpsPosData.Satellites = 10;
    gpsPosData.Status = GPSPosition::STATUS_FIX3D;
    gpsPos->setData(gpsPosData);

    // Update GPS Velocity.{North,East,Down}
    GPSVelocity::DataFields gpsVelData;
    memset(&gpsVelData, 0, sizeof(GPSVelocity::DataFields));
    gpsVelData.North = out.velNorth + noise.gpsVelData.North;
    gpsVelData.East = out.velEast + noise.gpsVelData.East;
    gpsVelData.Down = out.velDown + noise.gpsVelData.Down;
    gpsVel->setData(gpsVelData);


    // Update VelocityActual.{North,East,Down}
    VelocityActual::DataFields velocityActualData;
    memset(&velocityActualData, 0, sizeof(VelocityActual::DataFields));
    velocityActualData.North = out.velNorth + noise.velocityActualData.North;
    velocityActualData.East = out.velEast + noise.velocityActualData.East;
    velocityActualData.Down = out.velDown + noise.velocityActualData.Down;
    velActual->setData(velocityActualData);

    // Update PositionActual.{Nort,East,Down}
    PositionActual::DataFields positionActualData;
    memset(&positionActualData, 0, sizeof(PositionActual::DataFields));
    positionActualData.North = (out.dstN-initN) + noise.positionActualData.North;
    positionActualData.East = (out.dstE-initE) + noise.positionActualData.East;
    positionActualData.Down = (out.dstD/*-initD*/) + noise.positionActualData.Down;
    posActual->setData(positionActualData);


    // Update BaroAltitude object
    BaroAltitude::DataFields baroAltData;
    memset(&baroAltData, 0, sizeof(BaroAltitude::DataFields));
    baroAltData.Altitude = out.altitude + noise.baroAltData.Altitude;
    baroAltData.Temperature = out.temperature + noise.baroAltData.Temperature;
    baroAltData.Pressure = out.pressure + noise.baroAltData.Pressure;
    baroAlt->setData(baroAltData);

    // Update BaroAirspeed object
    BaroAirspeed::DataFields baroAirspeedData;
    memset(&baroAirspeedData, 0, sizeof(BaroAirspeed::DataFields));
    baroAirspeedData.Airspeed = out.airspeed + noise.baroAirspeed.Airspeed;
    baroAirspeed->setData(baroAirspeedData);

    //Update gyroscope sensor data
    Gyros::DataFields gyroData;
    memset(&gyroData, 0, sizeof(Gyros::DataFields));
    gyroData.x = out.rollRate + noise.gyroData.x;
    gyroData.y = out.pitchRate + noise.gyroData.y;
    gyroData.z = out.yawRate + noise.gyroData.z;
    gyros->setData(gyroData);

    //Update accelerometer sensor data
    Accels::DataFields accelData;
    memset(&accelData, 0, sizeof(Accels::DataFields));
    accelData.x = out.accX + noise.accelData.x;
    accelData.y = out.accY + noise.accelData.y;
    accelData.z = out.accZ + noise.accelData.z;
    accels->setData(accelData);
}
