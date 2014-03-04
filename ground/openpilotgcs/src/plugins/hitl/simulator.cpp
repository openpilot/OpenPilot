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
#include "hitlnoisegeneration.h"

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/threadmanager.h>
#include <uavtalk/telemetrymanager.h>

volatile bool Simulator::isStarted = false;

const float Simulator::GEE      = 9.81;
const float Simulator::FT2M     = 0.3048;
const float Simulator::KT2MPS   = 0.514444444;
const float Simulator::INHG2KPA = 3.386;
const float Simulator::FPS2CMPS = 30.48;
const float Simulator::DEG2RAD  = (M_PI / 180.0);
const float Simulator::RAD2DEG  = (180.0 / M_PI);


Simulator::Simulator(const SimulatorSettings & params) :
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
    name("")
{
    // move to thread
    moveToThread(Core::ICore::instance()->threadManager()->getRealTimeThread());
    connect(this, SIGNAL(myStart()), this, SLOT(onStart()), Qt::QueuedConnection);
    emit myStart();

    QTime currentTime = QTime::currentTime();
    gpsPosTime        = currentTime;
    groundTruthTime   = currentTime;
    gcsRcvrTime       = currentTime;
    attRawTime        = currentTime;
    baroAltTime       = currentTime;
    battTime = currentTime;
    airspeedStateTime = currentTime;

    // Define standard atmospheric constants
    airParameters.univGasConstant  = 8.31447; // [J/(mol·K)]
    airParameters.dryAirConstant   = 287.058;  // [J/(kg*K)]
    airParameters.groundDensity    = 1.225;     // [kg/m^3]
    airParameters.groundTemp = 15 + 273.15; // [K]
    airParameters.tempLapseRate    = 0.0065;    // [deg/m]
    airParameters.M = 0.0289644; // [kg/mol]
    airParameters.relativeHumidity = 20; // [%]
    airParameters.seaLevelPress    = 101.325;   // [kPa]
}

Simulator::~Simulator()
{
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
    // NOTE: Does not currently work, may need to send control+c to through the terminal
    if (simProcess != NULL) {
        // connect(simProcess,SIGNAL(finished(int, QProcess::ExitStatus)),this,SLOT(onFinished(int, QProcess::ExitStatus)));

        simProcess->disconnect();
        if (simProcess->state() == QProcess::Running) {
            simProcess->kill();
        }
        // if(simProcess->waitForFinished())
        // emit deleteSimProcess();
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

    QThread *mainThread = QThread::currentThread();

    qDebug() << "Simulator Thread: " << mainThread;

    // Get required UAVObjects
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    actDesired     = ActuatorDesired::GetInstance(objManager);
    actCommand     = ActuatorCommand::GetInstance(objManager);
    manCtrlCommand = ManualControlCommand::GetInstance(objManager);
    gcsReceiver    = GCSReceiver::GetInstance(objManager);
    flightStatus   = FlightStatus::GetInstance(objManager);
    posHome       = HomeLocation::GetInstance(objManager);
    velState      = VelocityState::GetInstance(objManager);
    posState      = PositionState::GetInstance(objManager);
    baroAlt       = BaroSensor::GetInstance(objManager);
    flightBatt    = FlightBatteryState::GetInstance(objManager);
    airspeedState = AirspeedState::GetInstance(objManager);
    attState      = AttitudeState::GetInstance(objManager);
    attSettings   = AttitudeSettings::GetInstance(objManager);
    accelState    = AccelState::GetInstance(objManager);
    gyroState     = GyroState::GetInstance(objManager);
    gpsPos = GPSPositionSensor::GetInstance(objManager);
    gpsVel = GPSVelocitySensor::GetInstance(objManager);
    telStats      = GCSTelemetryStats::GetInstance(objManager);
    groundTruth   = GroundTruth::GetInstance(objManager);

    // Listen to autopilot connection events
    TelemetryManager *telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));
    // connect(telStats, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(telStatsUpdated(UAVObject*)));

    // If already connect setup autopilot
    GCSTelemetryStats::DataFields stats = telStats->getData();
    if (stats.Status == GCSTelemetryStats::STATUS_CONNECTED) {
        onAutopilotConnect();
    }

    inSocket  = new QUdpSocket();
    outSocket = new QUdpSocket();
    setupUdpPorts(settings.hostAddress, settings.inPort, settings.outPort);

    emit processOutput("\nLocal interface: " + settings.hostAddress + "\n" + \
                       "Remote interface: " + settings.remoteAddress + "\n" + \
                       "inputPort: " + QString::number(settings.inPort) + "\n" + \
                       "outputPort: " + QString::number(settings.outPort) + "\n");

    qDebug() << ("\nLocal interface: " + settings.hostAddress + "\n" + \
                 "Remote interface: " + settings.remoteAddress + "\n" + \
                 "inputPort: " + QString::number(settings.inPort) + "\n" + \
                 "outputPort: " + QString::number(settings.outPort) + "\n");

// if(!inSocket->waitForConnected(5000))
// emit processOutput(QString("Can't connect to %1 on %2 port!").arg(settings.hostAddress).arg(settings.inPort));
// outSocket->connectToHost(settings.hostAddress,settings.outPort); // FG
// if(!outSocket->waitForConnected(5000))
// emit processOutput(QString("Can't connect to %1 on %2 port!").arg(settings.hostAddress).arg(settings.outPort));


    connect(inSocket, SIGNAL(readyRead()), this, SLOT(receiveUpdate()), Qt::DirectConnection);

    // Setup transmit timer
    txTimer = new QTimer();
    connect(txTimer, SIGNAL(timeout()), this, SLOT(transmitUpdate()), Qt::DirectConnection);
    txTimer->setInterval(updatePeriod);
    txTimer->start();
    // Setup simulator connection timer
    simTimer = new QTimer();
    connect(simTimer, SIGNAL(timeout()), this, SLOT(onSimulatorConnectionTimeout()), Qt::DirectConnection);
    simTimer->setInterval(simTimeout);
    simTimer->start();

    // setup time
    time = new QTime();
    time->start();
    current.T = 0;
    current.i = 0;
}

void Simulator::receiveUpdate()
{
    // Update connection timer and status
    simTimer->setInterval(simTimeout);
    simTimer->stop();
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
        inSocket->readDatagram(datagram.data(), datagram.size(),
                               &sender, &senderPort);
        // QString datastr(datagram);
        // Process incomming data
        processUpdate(datagram);
    }
}

void Simulator::setupObjects()
{
    if (settings.gcsReceiverEnabled) {
        setupInputObject(actCommand, settings.minOutputPeriod); // Input to the simulator
        setupOutputObject(gcsReceiver, settings.minOutputPeriod);
    } else if (settings.manualControlEnabled) {
        setupInputObject(actDesired, settings.minOutputPeriod); // Input to the simulator
    }

    setupOutputObject(posHome, 10000); // Hardcoded? Bleh.

    if (settings.gpsPositionEnabled) {
        setupOutputObject(gpsPos, settings.gpsPosRate);
        setupOutputObject(gpsVel, settings.gpsPosRate);
    }

    if (settings.groundTruthEnabled) {
        setupOutputObject(posState, settings.groundTruthRate);
        setupOutputObject(velState, settings.groundTruthRate);
    }

    if (settings.attRawEnabled) {
        setupOutputObject(accelState, settings.attRawRate);
        setupOutputObject(gyroState, settings.attRawRate);
    }

    if (settings.attStateEnabled && settings.attActHW) {
        setupOutputObject(accelState, settings.attRawRate);
        setupOutputObject(gyroState, settings.attRawRate);
    }

    if (settings.attStateEnabled && !settings.attActHW) {
        setupOutputObject(attState, 20); // Hardcoded? Bleh.
    } else {
        setupWatchedObject(attState, 100); // Hardcoded? Bleh.
    }
    if (settings.airspeedStateEnabled) {
        setupOutputObject(airspeedState, settings.airspeedStateRate);
    }

    if (settings.baroSensorEnabled) {
        setupOutputObject(baroAlt, settings.baroAltRate);
        setupOutputObject(flightBatt, settings.baroAltRate);
    }
}


void Simulator::setupInputObject(UAVObject *obj, quint32 updatePeriod)
{
    UAVObject::Metadata mdata;

    mdata = obj->getDefaultMetadata();

    UAVObject::SetGcsAccess(mdata, UAVObject::ACCESS_READONLY);
    UAVObject::SetGcsTelemetryAcked(mdata, false);
    UAVObject::SetGcsTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_MANUAL);
    mdata.gcsTelemetryUpdatePeriod = 0;

    UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READWRITE);
    UAVObject::SetFlightTelemetryAcked(mdata, false);

    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = updatePeriod;

    obj->setMetadata(mdata);
}


void Simulator::setupWatchedObject(UAVObject *obj, quint32 updatePeriod)
{
    UAVObject::Metadata mdata;

    mdata = obj->getDefaultMetadata();

    UAVObject::SetGcsAccess(mdata, UAVObject::ACCESS_READONLY);
    UAVObject::SetGcsTelemetryAcked(mdata, false);
    UAVObject::SetGcsTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_MANUAL);
    mdata.gcsTelemetryUpdatePeriod = 0;

    UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READWRITE);
    UAVObject::SetFlightTelemetryAcked(mdata, false);
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = updatePeriod;

    obj->setMetadata(mdata);
}


void Simulator::setupOutputObject(UAVObject *obj, quint32 updatePeriod)
{
    UAVObject::Metadata mdata;

    mdata = obj->getDefaultMetadata();

    UAVObject::SetGcsAccess(mdata, UAVObject::ACCESS_READWRITE);
    UAVObject::SetGcsTelemetryAcked(mdata, false);
    UAVObject::SetGcsTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.gcsTelemetryUpdatePeriod = updatePeriod;

    UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READONLY);
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_MANUAL);

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
    if (simConnectionStatus) {
        simConnectionStatus = false;
        emit simulatorDisconnected();
    }
}


void Simulator::telStatsUpdated(UAVObject *obj)
{
    Q_UNUSED(obj);

    GCSTelemetryStats::DataFields stats = telStats->getData();
    if (!autopilotConnectionStatus && stats.Status == GCSTelemetryStats::STATUS_CONNECTED) {
        onAutopilotConnect();
    } else if (autopilotConnectionStatus && stats.Status != GCSTelemetryStats::STATUS_CONNECTED) {
        onAutopilotDisconnect();
    }
}


void Simulator::resetInitialHomePosition()
{
    once = false;
}


void Simulator::updateUAVOs(Output2Hardware out)
{
    QTime currentTime = QTime::currentTime();

    Noise noise;
    HitlNoiseGeneration noiseSource;

    if (settings.addNoise) {
        noise = noiseSource.generateNoise();
    } else {
        memset(&noise, 0, sizeof(Noise));
    }

    /*******************************/
    HomeLocation::DataFields homeData = posHome->getData();
    if (!once) {
        // Upon startup, we reset the HomeLocation object to
        // the plane's location:
        memset(&homeData, 0, sizeof(HomeLocation::DataFields));
        // Update homelocation
        homeData.Latitude  = out.latitude;   // Already in *10^7 integer format
        homeData.Longitude = out.longitude; // Already in *10^7 integer format
        homeData.Altitude  = out.agl;

        homeData.Be[0]     = 0;
        homeData.Be[1]     = 0;
        homeData.Be[2]     = 0;
        posHome->setData(homeData);
        posHome->updated();

        // Compute initial distance
        initN = out.dstN;
        initE = out.dstE;
        initD = out.dstD;

        once  = true;
    }

    /*******************************/
    // Copy everything to the ground truth object. GroundTruth is Noise-free.
    GroundTruth::DataFields groundTruthData;
    groundTruthData = groundTruth->getData();

    groundTruthData.AccelerationXYZ[0] = out.accX;
    groundTruthData.AccelerationXYZ[1] = out.accY;
    groundTruthData.AccelerationXYZ[2] = out.accZ;

    groundTruthData.AngularRates[0]    = out.rollRate;
    groundTruthData.AngularRates[1]    = out.pitchRate;
    groundTruthData.AngularRates[2]    = out.yawRate;

    groundTruthData.CalibratedAirspeed = out.calibratedAirspeed;
    groundTruthData.TrueAirspeed   = out.trueAirspeed;
    groundTruthData.AngleOfAttack  = out.angleOfAttack;
    groundTruthData.AngleOfSlip    = out.angleOfSlip;

    groundTruthData.PositionNED[0] = out.dstN - initN;
    groundTruthData.PositionNED[1] = out.dstE - initD;
    groundTruthData.PositionNED[2] = out.dstD - initD;

    groundTruthData.VelocityNED[0] = out.velNorth;
    groundTruthData.VelocityNED[1] = out.velEast;
    groundTruthData.VelocityNED[2] = out.velDown;

    groundTruthData.RPY[0] = out.roll;
    groundTruthData.RPY[0] = out.pitch;
    groundTruthData.RPY[0] = out.heading;

    // Set UAVO
    groundTruth->setData(groundTruthData);

/*******************************/
    // Update attState object
    AttitudeState::DataFields attStateData;
    attStateData = attState->getData();

    if (settings.attActHW) {
        // do nothing
        /*****************************************/
    } else if (settings.attActSim) {
        // take all data from simulator
        attStateData.Roll  = out.roll + noise.attStateData.Roll;   // roll;
        attStateData.Pitch = out.pitch + noise.attStateData.Pitch; // pitch
        attStateData.Yaw   = out.heading + noise.attStateData.Yaw; // Yaw
        float rpy[3];
        float quat[4];
        rpy[0] = attStateData.Roll;
        rpy[1] = attStateData.Pitch;
        rpy[2] = attStateData.Yaw;
        Utils::CoordinateConversions().RPY2Quaternion(rpy, quat);
        attStateData.q1 = quat[0];
        attStateData.q2 = quat[1];
        attStateData.q3 = quat[2];
        attStateData.q4 = quat[3];

        // Set UAVO
        attState->setData(attStateData);
        /*****************************************/
    } else if (settings.attActCalc) {
        // calculate RPY with code from Attitude module
        static float q[4] = { 1, 0, 0, 0 };
        static float gyro_correct_int2 = 0;

        float dT = out.delT;

        AttitudeSettings::DataFields attSettData = attSettings->getData();
        float accelKp     = attSettData.AccelKp * 0.1666666666666667;
        // float accelKi     = attSettData.AccelKp * 0.1666666666666667;
        float yawBiasRate = attSettData.YawBiasRate;

        // calibrate sensors on arming
        if (flightStatus->getData().Armed == FlightStatus::ARMED_ARMING) {
            accelKp = 2.0;
            // accelKi = 0.9;
        }

        float gyro[3] = { out.rollRate, out.pitchRate, out.yawRate };
        float attRawAcc[3] = { out.accX, out.accY, out.accZ };

        // code from Attitude module begin ///////////////////////////////
        float *accels = attRawAcc;
        float grot[3];
        float accel_err[3];

        // Rotate gravity to body frame and cross with accels
        grot[0] = -(2 * (q[1] * q[3] - q[0] * q[2]));
        grot[1] = -(2 * (q[2] * q[3] + q[0] * q[1]));
        grot[2] = -(q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]);

        // CrossProduct
        {
            accel_err[0] = accels[1] * grot[2] - grot[1] * accels[2];
            accel_err[1] = grot[0] * accels[2] - accels[0] * grot[2];
            accel_err[2] = accels[0] * grot[1] - grot[0] * accels[1];
        }

        // Account for accel magnitude
        float accel_mag = sqrt(accels[0] * accels[0] + accels[1] * accels[1] + accels[2] * accels[2]);
        accel_err[0] /= accel_mag;
        accel_err[1] /= accel_mag;
        accel_err[2] /= accel_mag;

        // Accumulate integral of error.  Scale here so that units are (deg/s) but Ki has units of s
        gyro_correct_int2 += -gyro[2] * yawBiasRate;

        // Correct rates based on error, integral component dealt with in updateSensors
        gyro[0] += accel_err[0] * accelKp / dT;
        gyro[1] += accel_err[1] * accelKp / dT;
        gyro[2] += accel_err[2] * accelKp / dT + gyro_correct_int2;

        // Work out time derivative from INSAlgo writeup
        // Also accounts for the fact that gyros are in deg/s
        float qdot[4];
        qdot[0] = (-q[1] * gyro[0] - q[2] * gyro[1] - q[3] * gyro[2]) * dT * M_PI / 180 / 2;
        qdot[1] = (+q[0] * gyro[0] - q[3] * gyro[1] + q[2] * gyro[2]) * dT * M_PI / 180 / 2;
        qdot[2] = (+q[3] * gyro[0] + q[0] * gyro[1] - q[1] * gyro[2]) * dT * M_PI / 180 / 2;
        qdot[3] = (-q[2] * gyro[0] + q[1] * gyro[1] + q[0] * gyro[2]) * dT * M_PI / 180 / 2;

        // Take a time step
        q[0]   += qdot[0];
        q[1]   += qdot[1];
        q[2]   += qdot[2];
        q[3]   += qdot[3];

        if (q[0] < 0) {
            q[0] = -q[0];
            q[1] = -q[1];
            q[2] = -q[2];
            q[3] = -q[3];
        }

        // Renomalize
        float qmag = sqrt((q[0] * q[0]) + (q[1] * q[1]) + (q[2] * q[2]) + (q[3] * q[3]));
        q[0] /= qmag;
        q[1] /= qmag;
        q[2] /= qmag;
        q[3] /= qmag;

        // If quaternion has become inappropriately short or is nan reinit.
        // THIS SHOULD NEVER ACTUALLY HAPPEN
        if ((fabs(qmag) < 1e-3) || (qmag != qmag)) {
            q[0] = 1;
            q[1] = 0;
            q[2] = 0;
            q[3] = 0;
        }

        float rpy2[3];
        // Quaternion2RPY
        {
            float q0s, q1s, q2s, q3s;
            q0s     = q[0] * q[0];
            q1s     = q[1] * q[1];
            q2s     = q[2] * q[2];
            q3s     = q[3] * q[3];

            float R13, R11, R12, R23, R33;
            R13     = 2 * (q[1] * q[3] - q[0] * q[2]);
            R11     = q0s + q1s - q2s - q3s;
            R12     = 2 * (q[1] * q[2] + q[0] * q[3]);
            R23     = 2 * (q[2] * q[3] + q[0] * q[1]);
            R33     = q0s - q1s - q2s + q3s;

            rpy2[1] = RAD2DEG * asinf(-R13); // pitch always between -pi/2 to pi/2
            rpy2[2] = RAD2DEG * atan2f(R12, R11);
            rpy2[0] = RAD2DEG * atan2f(R23, R33);
        }

        attStateData.Roll  = rpy2[0];
        attStateData.Pitch = rpy2[1];
        attStateData.Yaw   = rpy2[2];
        attStateData.q1    = q[0];
        attStateData.q2    = q[1];
        attStateData.q3    = q[2];
        attStateData.q4    = q[3];

        // Set UAVO
        attState->setData(attStateData);
        /*****************************************/
    }

    /*******************************/
    if (settings.gcsReceiverEnabled) {
        if (gcsRcvrTime.msecsTo(currentTime) >= settings.minOutputPeriod) {
            GCSReceiver::DataFields gcsRcvrData;
            memset(&gcsRcvrData, 0, sizeof(GCSReceiver::DataFields));

            for (quint16 i = 0; i < GCSReceiver::CHANNEL_NUMELEM; i++) {
                gcsRcvrData.Channel[i] = 1500 + (out.rc_channel[i] * 500); // Elements in rc_channel are between -1 and 1
            }

            gcsReceiver->setData(gcsRcvrData);

            gcsRcvrTime = gcsRcvrTime.addMSecs(settings.minOutputPeriod);
        }
    }


    /*******************************/
    if (settings.gpsPositionEnabled) {
        if (gpsPosTime.msecsTo(currentTime) >= settings.gpsPosRate) {
            qDebug() << " GPS time:" << gpsPosTime << ", currentTime: " << currentTime << ", difference: " << gpsPosTime.msecsTo(currentTime);
            // Update GPS Position objects
            GPSPositionSensor::DataFields gpsPosData;
            memset(&gpsPosData, 0, sizeof(GPSPositionSensor::DataFields));
            gpsPosData.Altitude        = out.altitude + noise.gpsPosData.Altitude;
            gpsPosData.Heading         = out.heading + noise.gpsPosData.Heading;
            gpsPosData.Groundspeed     = out.groundspeed + noise.gpsPosData.Groundspeed;
            gpsPosData.Latitude        = out.latitude + noise.gpsPosData.Latitude;    // Already in *10^7 integer format
            gpsPosData.Longitude       = out.longitude + noise.gpsPosData.Longitude; // Already in *10^7 integer format
            gpsPosData.GeoidSeparation = 0.0;
            gpsPosData.PDOP = 3.0;
            gpsPosData.VDOP = gpsPosData.PDOP * 1.5;
            gpsPosData.Satellites      = 10;
            gpsPosData.Status = GPSPositionSensor::STATUS_FIX3D;

            gpsPos->setData(gpsPosData);

            // Update GPS Velocity.{North,East,Down}
            GPSVelocitySensor::DataFields gpsVelData;
            memset(&gpsVelData, 0, sizeof(GPSVelocitySensor::DataFields));
            gpsVelData.North = out.velNorth + noise.gpsVelData.North;
            gpsVelData.East  = out.velEast + noise.gpsVelData.East;
            gpsVelData.Down  = out.velDown + noise.gpsVelData.Down;

            gpsVel->setData(gpsVelData);

            gpsPosTime = gpsPosTime.addMSecs(settings.gpsPosRate);
        }
    }

    /*******************************/
    // Update VelocityState.{North,East,Down}
    if (settings.groundTruthEnabled) {
        if (groundTruthTime.msecsTo(currentTime) >= settings.groundTruthRate) {
            VelocityState::DataFields velocityStateData;
            memset(&velocityStateData, 0, sizeof(VelocityState::DataFields));
            velocityStateData.North = out.velNorth + noise.velocityStateData.North;
            velocityStateData.East  = out.velEast + noise.velocityStateData.East;
            velocityStateData.Down  = out.velDown + noise.velocityStateData.Down;
            velState->setData(velocityStateData);

            // Update PositionState.{Nort,East,Down}
            PositionState::DataFields positionStateData;
            memset(&positionStateData, 0, sizeof(PositionState::DataFields));
            positionStateData.North = (out.dstN - initN) + noise.positionStateData.North;
            positionStateData.East  = (out.dstE - initE) + noise.positionStateData.East;
            positionStateData.Down  = (out.dstD /*-initD*/) + noise.positionStateData.Down;
            posState->setData(positionStateData);

            groundTruthTime = groundTruthTime.addMSecs(settings.groundTruthRate);
        }
    }

///*******************************/
// if (settings.sonarAltitude) {
// static QTime sonarAltTime = currentTime;
// if (sonarAltTime.msecsTo(currentTime) >= settings.sonarAltRate) {
// SonarAltitude::DataFields sonarAltData;
// sonarAltData = sonarAlt->getData();

// float sAlt = settings.sonarMaxAlt;
//// 0.35 rad ~= 20 degree
// if ((agl < (sAlt * 2.0)) && (roll < 0.35) && (pitch < 0.35)) {
// float x = agl * qTan(roll);
// float y = agl * qTan(pitch);
// float h = qSqrt(x*x + y*y + agl*agl);
// sAlt = qMin(h, sAlt);
// }

// sonarAltData.Altitude = sAlt;
// sonarAlt->setData(sonarAltData);
// sonarAltTime = currentTime;
// }
// }

    /*******************************/
    // Update BaroSensor object
    if (settings.baroSensorEnabled) {
        if (baroAltTime.msecsTo(currentTime) >= settings.baroAltRate) {
            BaroSensor::DataFields baroAltData;
            memset(&baroAltData, 0, sizeof(BaroSensor::DataFields));
            baroAltData.Altitude    = out.altitude + noise.baroAltData.Altitude;
            baroAltData.Temperature = out.temperature + noise.baroAltData.Temperature;
            baroAltData.Pressure    = out.pressure + noise.baroAltData.Pressure;
            baroAlt->setData(baroAltData);

            baroAltTime = baroAltTime.addMSecs(settings.baroAltRate);
        }
    }

    /*******************************/
    // Update FlightBatteryState object
    if (settings.baroSensorEnabled) {
        if (battTime.msecsTo(currentTime) >= settings.baroAltRate) {
            FlightBatteryState::DataFields batteryData;
            memset(&batteryData, 0, sizeof(FlightBatteryState::DataFields));
            batteryData.Voltage = out.voltage;
            batteryData.Current = out.current;
            batteryData.ConsumedEnergy = out.consumption;
            flightBatt->setData(batteryData);

            battTime = battTime.addMSecs(settings.baroAltRate);
        }
    }

    /*******************************/
    // Update AirspeedState object
    if (settings.airspeedStateEnabled) {
        if (airspeedStateTime.msecsTo(currentTime) >= settings.airspeedStateRate) {
            AirspeedState::DataFields airspeedStateData;
            memset(&airspeedStateData, 0, sizeof(AirspeedState::DataFields));
            airspeedStateData.CalibratedAirspeed = out.calibratedAirspeed + noise.airspeedState.CalibratedAirspeed;
            airspeedStateData.TrueAirspeed = out.trueAirspeed + noise.airspeedState.TrueAirspeed;
            // airspeedStateData.alpha=out.angleOfAttack; // to be implemented
            // airspeedStateData.beta=out.angleOfSlip;
            airspeedState->setData(airspeedStateData);

            airspeedStateTime = airspeedStateTime.addMSecs(settings.airspeedStateRate);
        }
    }

    /*******************************/
    // Update raw attitude sensors
    if (settings.attRawEnabled) {
        if (attRawTime.msecsTo(currentTime) >= settings.attRawRate) {
            // Update gyroscope sensor data
            GyroState::DataFields gyroStateData;
            memset(&gyroStateData, 0, sizeof(GyroState::DataFields));
            gyroStateData.x = out.rollRate + noise.gyroStateData.x;
            gyroStateData.y = out.pitchRate + noise.gyroStateData.y;
            gyroStateData.z = out.yawRate + noise.gyroStateData.z;
            gyroState->setData(gyroStateData);

            // Update accelerometer sensor data
            AccelState::DataFields accelStateData;
            memset(&accelStateData, 0, sizeof(AccelState::DataFields));
            accelStateData.x = out.accX + noise.accelStateData.x;
            accelStateData.y = out.accY + noise.accelStateData.y;
            accelStateData.z = out.accZ + noise.accelStateData.z;
            accelState->setData(accelStateData);

            attRawTime = attRawTime.addMSecs(settings.attRawRate);
        }
    }
}

/**
 * calculate air density from altitude. http://en.wikipedia.org/wiki/Density_of_air
 */
float Simulator::airDensityFromAltitude(float alt, AirParameters air, float gravity)
{
    float p   = airPressureFromAltitude(alt, air, gravity);
    float rho = p * air.M / (air.univGasConstant * (air.groundTemp - air.tempLapseRate * alt));

    return rho;
}

/**
 * @brief Simulator::airPressureFromAltitude Get air pressure from altitude and atmospheric conditions.
 * @param alt altitude
 * @param air atmospheric conditions
 * @param gravity
 * @return
 */
float Simulator::airPressureFromAltitude(float alt, AirParameters air, float gravity)
{
    return air.seaLevelPress * pow(1 - air.tempLapseRate * alt / air.groundTemp, gravity * air.M / (air.univGasConstant * air.tempLapseRate));
}

/**
 * @brief Simulator::cas2tas calculate TAS from CAS and altitude. http://en.wikipedia.org/wiki/Airspeed
 * @param CAS Calibrated airspeed
 * @param alt altitude
 * @param air atmospheric conditions
 * @param gravity
 * @return True airspeed
 */
float Simulator::cas2tas(float CAS, float alt, AirParameters air, float gravity)
{
    float rho = airDensityFromAltitude(alt, air, gravity);

    return CAS * sqrt(air.groundDensity / rho);
}

/**
 * @brief Simulator::tas2cas calculate CAS from TAS and altitude. http://en.wikipedia.org/wiki/Airspeed
 * @param TAS True airspeed
 * @param alt altitude
 * @param air atmospheric conditions
 * @param gravity
 * @return Calibrated airspeed
 */
float Simulator::tas2cas(float TAS, float alt, AirParameters air, float gravity)
{
    float rho = airDensityFromAltitude(alt, air, gravity);

    return TAS / sqrt(air.groundDensity / rho);
}

/**
 * @brief Simulator::getAirParameters get air parameters
 * @return airParameters
 */
AirParameters Simulator::getAirParameters()
{
    return airParameters;
}

/**
 * @brief Simulator::setAirParameters set air parameters
 * @param airParameters
 */
void Simulator::setAirParameters(AirParameters airParameters)
{
    this->airParameters = airParameters;
}
