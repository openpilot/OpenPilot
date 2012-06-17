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
