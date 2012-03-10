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

#include "simulatorv2.h"
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/threadmanager.h>

volatile bool Simulator::isStarted = false;

const float Simulator::GEE = 9.81;
const float Simulator::FT2M = 0.3048;
const float Simulator::KT2MPS = 0.514444444;
const float Simulator::INHG2KPA = 3.386;
const float Simulator::FPS2CMPS = 30.48;
const float Simulator::DEG2RAD = (M_PI/180.0);
const float Simulator::RAD2DEG = (180.0/M_PI);


Simulator::Simulator(const SimulatorSettings& params) :
    inSocket(NULL),
    outSocket(NULL),
    settings(params),
    updatePeriod(50),
    simTimeout(2000),
    autopilotConnectionStatus(false),
    simConnectionStatus(false),
    txTimer(NULL),
    simTimer(NULL),
    name("")
{
    // move to thread
    moveToThread(Core::ICore::instance()->threadManager()->getRealTimeThread());
    connect(this, SIGNAL(myStart()), this, SLOT(onStart()), Qt::QueuedConnection);
    emit myStart();
}

Simulator::~Simulator()
{
//    qDebug() << "Simulator::~Simulator";
    if (inSocket) {
        delete inSocket;
        inSocket = NULL;
    }
    if (outSocket) {
        delete outSocket;
        outSocket = NULL;
    }
    if (txTimer) {
        delete txTimer;
        txTimer = NULL;
    }
    if (simTimer) {
        delete simTimer;
        simTimer = NULL;
    }
}

void Simulator::onDeleteSimulator(void)
{
//    qDebug() << "Simulator::onDeleteSimulator";
    resetAllObjects();

    Simulator::setStarted(false);
    Simulator::Instances().removeOne(simulatorId);

    disconnect(this);
    delete this;
}

void Simulator::onStart()
{
//    qDebug() << "Simulator::onStart";
    QMutexLocker locker(&lock);

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();

//    actDesired = ActuatorDesired::GetInstance(objManager);
//    manCtrlCommand = ManualControlCommand::GetInstance(objManager);
//    velActual = VelocityActual::GetInstance(objManager);
//    posActual = PositionActual::GetInstance(objManager);
//    altActual = BaroAltitude::GetInstance(objManager);
//    camDesired = CameraDesired::GetInstance(objManager);
//    acsDesired = AccessoryDesired::GetInstance(objManager);
    posHome = HomeLocation::GetInstance(objManager);
    attRaw = AttitudeRaw::GetInstance(objManager);
    attActual = AttitudeActual::GetInstance(objManager);
    gpsPosition = GPSPosition::GetInstance(objManager);
    flightStatus = FlightStatus::GetInstance(objManager);
    gcsReceiver = GCSReceiver::GetInstance(objManager);
    actCommand = ActuatorCommand::GetInstance(objManager);
    attSettings = AttitudeSettings::GetInstance(objManager);

    telStats = GCSTelemetryStats::GetInstance(objManager);

    // Listen to autopilot connection events
    TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));

    // If already connect setup autopilot
    GCSTelemetryStats::DataFields stats = telStats->getData();
    if (stats.Status == GCSTelemetryStats::STATUS_CONNECTED)
        onAutopilotConnect();

    emit processOutput("Local interface: " + settings.hostAddress + ":" + \
                       QString::number(settings.inPort) + "\n" + \
                       "Remote interface: " + settings.remoteAddress + ":" + \
                       QString::number(settings.outPort) + "\n");

    inSocket = new QUdpSocket();
    outSocket = new QUdpSocket();
    setupUdpPorts(settings.hostAddress, settings.inPort, settings.outPort);

    connect(inSocket, SIGNAL(readyRead()), this, SLOT(receiveUpdate())/*, Qt::DirectConnection*/);

    // Setup transmit timer
    if (settings.manualOutput) {
        txTimer = new QTimer();
        connect(txTimer, SIGNAL(timeout()), this, SLOT(transmitUpdate())/*, Qt::DirectConnection*/);
        txTimer->setInterval(settings.outputRate);
        txTimer->start();
    }

    // Setup simulator connection timer
    simTimer = new QTimer();
    connect(simTimer, SIGNAL(timeout()), this, SLOT(onSimulatorConnectionTimeout())/*, Qt::DirectConnection*/);
    simTimer->setInterval(simTimeout);
    simTimer->start();

#ifdef DBG_TIMERS
    timeRX = QTime();
    timeRX.start();
    timeTX = QTime();
    timeTX.start();
#endif

    setupObjects();
}

void Simulator::receiveUpdate()
{
    // Update connection timer and status
    simTimer->start();
    if (!simConnectionStatus) {
        simConnectionStatus = true;
        emit simulatorConnected();
    }

    // Process data
    while (inSocket->hasPendingDatagrams()) {
        // Receive datagram
        QByteArray datagram;
        datagram.resize(inSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        inSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        // Process incomming data
        processUpdate(datagram);
    }
    if (!settings.manualOutput)
        transmitUpdate();
}

void Simulator::setupObjects()
{
    if (settings.gcsReciever) {
        setupInputObject(actCommand, settings.outputRate);
        setupOutputObject(gcsReceiver);
    } else if (settings.manualControl) {
//        setupInputObject(actDesired);
//        setupInputObject(camDesired);
//        setupInputObject(acsDesired);
//        setupOutputObject(manCtrlCommand);
        qDebug() << "ManualControlCommand not implemented yet";
    }

    if (settings.homeLocation)
        setupOutputObject(posHome);

    if (settings.gpsPosition)
        setupOutputObject(gpsPosition);

    if (settings.attRaw || settings.attActual)
        setupOutputObject(attRaw);

    if (settings.attActual && !settings.attActHW)
        setupOutputObject(attActual);
    else
        setupWatchedObject(attActual);
}

void Simulator::resetAllObjects()
{
    setupDefaultObject(posHome);
    setupDefaultObject(attRaw);
    setupDefaultObject(attActual);
    setupDefaultObject(gpsPosition);
    setupDefaultObject(gcsReceiver);
    setupDefaultObject(actCommand);
//    setupDefaultObject(manCtrlCommand);
//    setupDefaultObject(actDesired);
//    setupDefaultObject(camDesired);
//    setupDefaultObject(acsDesired);
//    setupDefaultObject(altActual);
//    setupDefaultObject(posActual);
//    setupDefaultObject(velActual);
}

void Simulator::setupInputObject(UAVObject* obj, quint32 updateRate)
{
    UAVObject::Metadata mdata;
    mdata = obj->getDefaultMetadata();

    mdata.gcsAccess = UAVObject::ACCESS_READONLY;
    mdata.gcsTelemetryAcked = false;
    mdata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_NEVER;
    mdata.gcsTelemetryUpdatePeriod = 0;
    mdata.flightAccess = UAVObject::ACCESS_READWRITE;
    mdata.flightTelemetryAcked = false;
    if (settings.manualOutput) {
        mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
        mdata.flightTelemetryUpdatePeriod = updateRate;
    } else {
        mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
        mdata.flightTelemetryUpdatePeriod = 0;
    }

    obj->setMetadata(mdata);
}

void Simulator::setupWatchedObject(UAVObject *obj)
{
    UAVObject::Metadata mdata;
    mdata = obj->getDefaultMetadata();

    mdata.gcsAccess = UAVObject::ACCESS_READONLY;
    mdata.gcsTelemetryAcked = false;
    mdata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    mdata.gcsTelemetryUpdatePeriod = 0;
    mdata.flightAccess = UAVObject::ACCESS_READWRITE;
    mdata.flightTelemetryAcked = false;
    mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    mdata.flightTelemetryUpdatePeriod = 100;

    obj->setMetadata(mdata);
}

void Simulator::setupOutputObject(UAVObject* obj)
{
    UAVObject::Metadata mdata;
    mdata = obj->getDefaultMetadata();

    mdata.flightAccess = UAVObject::ACCESS_READONLY;
    mdata.flightTelemetryAcked = false;
    mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_NEVER;
    mdata.flightTelemetryUpdatePeriod = 0;
    mdata.gcsAccess = UAVObject::ACCESS_READWRITE;
    mdata.gcsTelemetryAcked = false;
    mdata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
    mdata.gcsTelemetryUpdatePeriod = 0;

    obj->setMetadata(mdata);
}

void Simulator::setupDefaultObject(UAVObject *obj)
{
    UAVObject::Metadata mdata;
    mdata = obj->getDefaultMetadata();

    obj->setMetadata(mdata);
}

void Simulator::onAutopilotConnect()
{
    autopilotConnectionStatus = true;
    emit autopilotConnected();
}

void Simulator::onAutopilotDisconnect()
{
    autopilotConnectionStatus = false;
    emit autopilotDisconnected();
}

void Simulator::onSimulatorConnectionTimeout()
{
    if (simConnectionStatus) {
        simConnectionStatus = false;
        emit simulatorDisconnected();
    }
}

void Simulator::telStatsUpdated(UAVObject* obj)
{
    GCSTelemetryStats::DataFields stats = telStats->getData();
    if (!autopilotConnectionStatus && stats.Status == GCSTelemetryStats::STATUS_CONNECTED)
        onAutopilotConnect();
    else if (autopilotConnectionStatus && stats.Status != GCSTelemetryStats::STATUS_CONNECTED)
        onAutopilotDisconnect();
}

