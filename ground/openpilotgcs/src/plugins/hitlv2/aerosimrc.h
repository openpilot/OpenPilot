/**
 ******************************************************************************
 *
 * @file       aerosimrc.h
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

#ifndef AEROSIMRC_H
#define AEROSIMRC_H

#include <QObject>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>
#include "simulatorv2.h"

class AeroSimRCSimulator: public Simulator
{
    Q_OBJECT

public:
    AeroSimRCSimulator(const SimulatorSettings &params);
    ~AeroSimRCSimulator();

    bool setupProcess();
    void setupUdpPorts(const QString& host, int inPort, int outPort);

private slots:
    void transmitUpdate();

private:
    quint32 udpCounterASrecv;   //keeps track of udp packets received by ASim

    void processUpdate(const QByteArray &data);

    void asMatrix2Quat(const QMatrix4x4 &m, QQuaternion &q);
    void asMatrix2RPY(const QMatrix4x4 &m, QVector3D &rpy);
};

class AeroSimRCSimulatorCreator : public SimulatorCreator
{
public:
    AeroSimRCSimulatorCreator(const QString &classId, const QString &description)
        : SimulatorCreator (classId, description)
    {}

    Simulator* createSimulator(const SimulatorSettings &params)
    {
        return new AeroSimRCSimulator(params);
    }
};

#endif // AEROSIMRC_H
