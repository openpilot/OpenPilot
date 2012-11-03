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

#include "aerosimrcsimulator.h"
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

#define AEROSIM_RCCHANNEL_NUMELEM 8
    float   delT,
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
            ch[AEROSIM_RCCHANNEL_NUMELEM];

    stream >> delT;
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

    Output2Hardware out;
    memset(&out, 0, sizeof(Output2Hardware));


    out.delT=delT;

    /*************************************************************************************/
    for (int i=0; i< AEROSIM_RCCHANNEL_NUMELEM; i++){
        out.rc_channel[i]=ch[i]; //Elements in rc_channel are between -1 and 1
    }

    /**********************************************************************************************/
    QMatrix4x4 mat;
    mat = QMatrix4x4( fy,  fx, -fz,  0.0,           // model matrix
                      ry,  rx, -rz,  0.0,           // (X,Y,Z) -> (+Y,+X,-Z)
                     -uy, -ux,  uz,  0.0,
                     0.0, 0.0, 0.0,  1.0);
    mat.optimize();

    QQuaternion quat;                               // model quat
    asMatrix2Quat(mat, quat);

    /*************************************************************************************/
    // rotate gravity
    QVector3D acc = QVector3D(accY, accX, -accZ);   // accel (X,Y,Z) -> (+Y,+X,-Z)
    QVector3D gee = QVector3D(0.0, 0.0, -GEE);
    QQuaternion qWorld = quat.conjugate();
    gee = qWorld.rotatedVector(gee);
    acc += gee;

    out.rollRate = angY * RAD2DEG;       // gyros (X,Y,Z) -> (+Y,+X,-Z)
    out.pitchRate = angX * RAD2DEG;
    out.yawRate = angZ * -RAD2DEG;

    out.accX = acc.x();
    out.accY = acc.y();
    out.accZ = acc.z();

    /*************************************************************************************/
    QVector3D rpy;          // model roll, pitch, yaw
    asMatrix2RPY(mat, rpy);

    out.roll  = rpy.x();
    out.pitch = rpy.y();
    out.heading = rpy.z();


    /**********************************************************************************************/
    out.altitude = posZ;
    out.agl = posZ;
    out.heading = yaw * RAD2DEG;
    out.latitude = lat * 10e6;
    out.longitude = lon * 10e6;
    out.groundspeed = qSqrt(velX * velX + velY * velY);

    /**********************************************************************************************/
    out.dstN = posY * 100;
    out.dstE = posX * 100;
    out.dstD = posZ * -100;

    out.velDown = velY * 100;
    out.velEast = velX * 100;
    out.velDown = velZ * 100; //WHY ISN'T THIS `-velZ`???

    updateUAVOs(out);


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
