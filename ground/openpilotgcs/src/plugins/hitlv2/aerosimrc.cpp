/**
 ******************************************************************************
 *
 * @file       aerosimrc.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup HITLPlugin HITLv2 Plugin
 * @{
 * @brief The Hardware In The Loop plugin version 2
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

#include "aerosimrc.h"
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/threadmanager.h>

AeroSimRCSimulator::AeroSimRCSimulator(const SimulatorSettings &params)
    : Simulator(params)
{
    udpCounterASrecv = 0;
}

AeroSimRCSimulator::~AeroSimRCSimulator()
{
}

bool AeroSimRCSimulator::setupProcess()
{
    QMutexLocker locker(&lock);
    return true;
}

void AeroSimRCSimulator::setupUdpPorts(const QString &host, int inPort, int outPort)
{
    Q_UNUSED(outPort)
    if (inSocket->bind(QHostAddress(host), inPort))
        emit processOutput("Successfully bound to address " + host + ", port " + QString::number(inPort) + "\n");
    else
        emit processOutput("Cannot bind to address " + host + ", port " + QString::number(inPort) + "\n");
}

void AeroSimRCSimulator::transmitUpdate()
{
    // read actuator output
    ActuatorCommand::DataFields actCmdData;
    actCmdData = actCommand->getData();
    float channels[10];
    for (int i = 0; i < 10; ++i) {
        qint16 ch = actCmdData.Channel[i];
        float out = -1.0;
        if (ch >= 1000 && ch <= 2000) {
            ch -= 1000;
            out = ((float)ch / 500.0) - 1.0;
        }
        channels[i] = out;
    }

    // read flight status
    FlightStatus::DataFields flightData;
    flightData = flightStatus->getData();
    quint8 armed;
    quint8 mode;
    armed = flightData.Armed;
    mode = flightData.FlightMode;

    QByteArray data;
    // 50 - current size of values, 4(quint32) + 10*4(float) + 2(quint8) + 4(quint32)
    data.resize(50);
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << quint32(0x52434D44);      // magic header, "RCMD"
    for (int i = 0; i < 10; ++i)
        stream << channels[i];          // channels
    stream << armed << mode;            // flight status
    stream << udpCounterASrecv;         // packet counter

    if (outSocket->writeDatagram(data, QHostAddress(settings.remoteAddress), settings.outPort) == -1) {
        qDebug() << "write failed: " << outSocket->errorString();
    }

#ifdef DBG_TIMERS
    static int cntTX = 0;
    if (cntTX >= 100) {
        qDebug() << "TX=" << 1000.0 * 100 / timeTX.restart();
        cntTX = 0;
    } else {
        ++cntTX;
    }
#endif
}

void AeroSimRCSimulator::processUpdate(const QByteArray &data)
{
    // check size
    if (data.size() > 188) {
        qDebug() << "!!! big datagram: " << data.size();
        return;
    }

    QByteArray buf = data;
    QDataStream stream(&buf, QIODevice::ReadOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // check magic header
    quint32 magic;
    stream >> magic;
    if (magic != 0x4153494D) {   // "AERO"
        qDebug() << "wrong magic: " << magic << ", correct: " << quint32(0x4153494D);
        return;
    }

    float   timeStep,
            homeX, homeY, homeZ,
            WpHX, WpHY, WpLat, WpLon,
            posX, posY, posZ,   // world
            velX, velY, velZ,   // world
            angX, angY, angZ,   // model
            accX, accY, accZ,   // model
            lat, lon, agl,      // world
            yaw, pitch, roll,   // model
            volt, curr,
            rx, ry, rz, fx, fy, fz, ux, uy, uz, // matrix
            ch[8];

    stream >> timeStep;
    stream >> homeX >> homeY >> homeZ;
    stream >> WpHX >> WpHY >> WpLat >> WpLon;
    stream >> posX >> posY >> posZ;
    stream >> velX >> velY >> velZ;
    stream >> angX >> angY >> angZ;
    stream >> accX >> accY >> accZ;
    stream >> lat >> lon >> agl;
    stream >> yaw >> pitch >> roll;
    stream >> volt >> curr;
    stream >> rx >> ry >> rz >> fx >> fy >> fz >> ux >> uy >> uz;
    stream >> ch[0] >> ch[1] >> ch[2] >> ch[3] >> ch[4] >> ch[5] >> ch[6] >> ch[7];
    stream >> udpCounterASrecv;

    /**********************************************************************************************/
    QTime currentTime = QTime::currentTime();
    /**********************************************************************************************/
    static bool firstRun = true;
    if (settings.homeLocation) {
        if (firstRun) {
            HomeLocation::DataFields homeData;
            homeData = posHome->getData();

            homeData.Latitude = WpLat * 10e6;
            homeData.Longitude = WpLon * 10e6;
            homeData.Altitude = homeZ;
            homeData.Set = HomeLocation::SET_TRUE;

            posHome->setData(homeData);

            firstRun = false;
        }
        if (settings.homeLocRate > 0) {
            static QTime homeLocTime = currentTime;
            if (homeLocTime.secsTo(currentTime) >= settings.homeLocRate) {
                firstRun = true;
                homeLocTime = currentTime;
            }
        }
    }
    /**********************************************************************************************/
    if (settings.attRaw || settings.attActual) {
        QMatrix4x4 mat;
        mat = QMatrix4x4( fy,  fx, -fz,  0.0,           // model matrix
                          ry,  rx, -rz,  0.0,           // (X,Y,Z) -> (+Y,+X,-Z)
                         -uy, -ux,  uz,  0.0,
                         0.0, 0.0, 0.0,  1.0);
        mat.optimize();

        QQuaternion quat;                               // model quat
        asMatrix2Quat(mat, quat);

        // rotate gravity
        QVector3D acc = QVector3D(accY, accX, -accZ);   // accel (X,Y,Z) -> (+Y,+X,-Z)
        QVector3D gee = QVector3D(0.0, 0.0, -GEE);
        QQuaternion qWorld = quat.conjugate();
        gee = qWorld.rotatedVector(gee);
        acc += gee;

        /*************************************************************************************/
        if (settings.attRaw) {
            Accels::DataFields accelsData;
            accelsData = accels->getData();
            Gyros::DataFields gyrosData;
            gyrosData = gyros->getData();

            gyrosData.x = angY * RAD2DEG;       // gyros (X,Y,Z) -> (+Y,+X,-Z)
            gyrosData.y = angX * RAD2DEG;
            gyrosData.z = angZ * -RAD2DEG;
            accelsData.x = acc.x();
            accelsData.y = acc.y();
            accelsData.z = acc.z();

            accels->setData(accelsData);
            gyros->setData(gyrosData);
        }
        /*************************************************************************************/
        if (settings.attActHW) {
            // do nothing
            /*****************************************/
        } else if (settings.attActSim) {
            // take all data from simulator
            AttitudeActual::DataFields attActData;
            attActData = attActual->getData();

            QVector3D rpy;          // model roll, pitch, yaw
            asMatrix2RPY(mat, rpy);

            attActData.Roll  = rpy.x();
            attActData.Pitch = rpy.y();
            attActData.Yaw   = rpy.z();
            attActData.q1 = quat.scalar();
            attActData.q2 = quat.x();
            attActData.q3 = quat.y();
            attActData.q4 = quat.z();

            attActual->setData(attActData);
            /*****************************************/
        } else if (settings.attActCalc) {
            // calculate RPY with code from Attitude module
            AttitudeActual::DataFields attActData;
            attActData = attActual->getData();

            static float q[4] = {1, 0, 0, 0};
            static float gyro_correct_int2 = 0;

            float dT = timeStep;

            AttitudeSettings::DataFields attSettData = attSettings->getData();
            float accelKp = attSettData.AccelKp * 0.1666666666666667;
            float accelKi = attSettData.AccelKp * 0.1666666666666667;
            float yawBiasRate = attSettData.YawBiasRate;

            // calibrate sensors on arming
            if (flightStatus->getData().Armed == FlightStatus::ARMED_ARMING) {
                accelKp = 2.0;
                accelKi = 0.9;
            }

            float gyro[3] = {angY * RAD2DEG, angX * RAD2DEG, angZ * -RAD2DEG};
            float attRawAcc[3] = {acc.x(), acc.y(), acc.z()};

            // code from Attitude module begin ///////////////////////////////
            float *accels = attRawAcc;
            float grot[3];
            float accel_err[3];

            // Rotate gravity to body frame and cross with accels
            grot[0] = -(2 * (q[1] * q[3] - q[0] * q[2]));
            grot[1] = -(2 * (q[2] * q[3] + q[0] * q[1]));
            grot[2] = -(q[0] * q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3]);

            // CrossProduct
            {
                accel_err[0] = accels[1]*grot[2] - grot[1]*accels[2];
                accel_err[1] = grot[0]*accels[2] - accels[0]*grot[2];
                accel_err[2] = accels[0]*grot[1] - grot[0]*accels[1];
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
            q[0] += qdot[0];
            q[1] += qdot[1];
            q[2] += qdot[2];
            q[3] += qdot[3];

            if(q[0] < 0) {
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
            if((fabs(qmag) < 1e-3) || (qmag != qmag)) {
                q[0] = 1;
                q[1] = 0;
                q[2] = 0;
                q[3] = 0;
            }

            float rpy2[3];
            // Quaternion2RPY
            {
                float q0s, q1s, q2s, q3s;
                q0s = q[0] * q[0];
                q1s = q[1] * q[1];
                q2s = q[2] * q[2];
                q3s = q[3] * q[3];

                float R13, R11, R12, R23, R33;
                R13 = 2 * (q[1] * q[3] - q[0] * q[2]);
                R11 = q0s + q1s - q2s - q3s;
                R12 = 2 * (q[1] * q[2] + q[0] * q[3]);
                R23 = 2 * (q[2] * q[3] + q[0] * q[1]);
                R33 = q0s - q1s - q2s + q3s;

                rpy2[1] = RAD2DEG * asinf(-R13);    // pitch always between -pi/2 to pi/2
                rpy2[2] = RAD2DEG * atan2f(R12, R11);
                rpy2[0] = RAD2DEG * atan2f(R23, R33);
            }

            attActData.Roll  = rpy2[0];
            attActData.Pitch = rpy2[1];
            attActData.Yaw   = rpy2[2];
            attActData.q1 = q[0];
            attActData.q2 = q[1];
            attActData.q3 = q[2];
            attActData.q4 = q[3];
            attActual->setData(attActData);
            /*****************************************/
        }
    }
    /**********************************************************************************************/
    if (settings.gcsReciever) {
        static QTime gcsRcvrTime = currentTime;
        if (!settings.manualOutput || gcsRcvrTime.msecsTo(currentTime) >= settings.outputRate) {
            GCSReceiver::DataFields gcsRcvrData;
            gcsRcvrData = gcsReceiver->getData();

            for (int i = 0; i < 8; ++i)
                gcsRcvrData.Channel[i] = 1500 + (ch[i] * 500);

            gcsReceiver->setData(gcsRcvrData);
            if (settings.manualOutput)
                gcsRcvrTime = currentTime;
        }
    } else if (settings.manualControl) {
        // not implemented yet
    }
    /**********************************************************************************************/
    if (settings.gpsPosition) {
        static QTime gpsPosTime = currentTime;
        if (gpsPosTime.msecsTo(currentTime) >= settings.gpsPosRate) {
            GPSPosition::DataFields gpsPosData;
            gpsPosData = gpsPosition->getData();

            gpsPosData.Altitude = posZ;
            gpsPosData.Heading = yaw * RAD2DEG;
            gpsPosData.Latitude = lat * 10e6;
            gpsPosData.Longitude = lon * 10e6;
            gpsPosData.Groundspeed = qSqrt(velX * velX + velY * velY);
            gpsPosData.GeoidSeparation = 0.0;
            gpsPosData.Satellites = 8;
            gpsPosData.PDOP = 3.0;
            gpsPosData.Status = GPSPosition::STATUS_FIX3D;

            gpsPosition->setData(gpsPosData);
            gpsPosTime = currentTime;
        }
    }
    /**********************************************************************************************/
    if (settings.sonarAltitude) {
        static QTime sonarAltTime = currentTime;
        if (sonarAltTime.msecsTo(currentTime) >= settings.sonarAltRate) {
            SonarAltitude::DataFields sonarAltData;
            sonarAltData = sonarAlt->getData();

            float sAlt = settings.sonarMaxAlt;
            // 0.35 rad ~= 20 degree
            if ((agl < (sAlt * 2.0)) && (roll < 0.35) && (pitch < 0.35)) {
                float x = agl * qTan(roll);
                float y = agl * qTan(pitch);
                float h = qSqrt(x*x + y*y + agl*agl);
                sAlt = qMin(h, sAlt);
            }

            sonarAltData.Altitude = sAlt;
            sonarAlt->setData(sonarAltData);
            sonarAltTime = currentTime;
        }
    }
    /**********************************************************************************************/
/*
    BaroAltitude::DataFields altActData;
    altActData = altActual->getData();
    altActData.Altitude = posZ;
    altActual->setData(altActData);

    PositionActual::DataFields posActData;
    posActData = posActual->getData();
    posActData.North = posY * 100;
    posActData.East = posX * 100;
    posActData.Down = posZ * -100;
    posActual->setData(posActData);

    VelocityActual::DataFields velActData;
    velActData = velActual->getData();
    velActData.North = velY * 100;
    velActData.East = velX * 100;
    velActData.Down = velZ * 100;
    velActual->setData(velActData);
*/

#ifdef DBG_TIMERS
    static int cntRX = 0;
    if (cntRX >= 100) {
        qDebug() << "RX=" << 1000.0 * 100 / timeRX.restart();
        cntRX = 0;
    } else {
        ++cntRX;
    }
#endif
}

// transfomations

void AeroSimRCSimulator::asMatrix2Quat(const QMatrix4x4 &m, QQuaternion &q)
{
    qreal w, x, y, z;

    // w always >= 0
    w = qSqrt(qMax(0.0, 1.0 + m(0, 0) + m(1, 1) + m(2, 2))) / 2.0;
    x = qSqrt(qMax(0.0, 1.0 + m(0, 0) - m(1, 1) - m(2, 2))) / 2.0;
    y = qSqrt(qMax(0.0, 1.0 - m(0, 0) + m(1, 1) - m(2, 2))) / 2.0;
    z = qSqrt(qMax(0.0, 1.0 - m(0, 0) - m(1, 1) + m(2, 2))) / 2.0;

    x = copysign(x, (m(1, 2) - m(2, 1)));
    y = copysign(y, (m(2, 0) - m(0, 2)));
    z = copysign(z, (m(0, 1) - m(1, 0)));

    q.setScalar(w);
    q.setX(x);
    q.setY(y);
    q.setZ(z);
}

void AeroSimRCSimulator::asMatrix2RPY(const QMatrix4x4 &m, QVector3D &rpy)
{
    qreal roll, pitch, yaw;

    if (qFabs(m(0, 2)) > 0.998) {
        // ~86.3°, gimbal lock
        roll  = 0.0;
        pitch = copysign(M_PI_2, -m(0, 2));
        yaw   = qAtan2(-m(1, 0), m(1, 1));
    } else {
        roll  = qAtan2(m(1, 2), m(2, 2));
        pitch = qAsin(-m(0, 2));
        yaw   = qAtan2(m(0, 1), m(0, 0));
    }

    rpy.setX(roll  * RAD2DEG);
    rpy.setY(pitch * RAD2DEG);
    rpy.setZ(yaw   * RAD2DEG);
}
