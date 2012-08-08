/**
 ******************************************************************************
 *
 * @file       simulatorv2.h
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

#ifndef ISIMULATORV2_H
#define ISIMULATORV2_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QProcess>
#include <QScopedPointer>
#include <qmath.h>
#include "uavtalk/telemetrymanager.h"
#include "uavobjectmanager.h"
#include "homelocation.h"
#include "accels.h"
#include "gyros.h"
#include "attitudeactual.h"
#include "gpsposition.h"
#include "flightstatus.h"
#include "gcsreceiver.h"
#include "actuatorcommand.h"
#include "gcstelemetrystats.h"
#include "attitudesettings.h"
#include "sonaraltitude.h"

//#define DBG_TIMERS
#undef DBG_TIMERS

/**
 * just imagine this was a class without methods and all public properties
 */

typedef struct _CONNECTION
{
    QString simulatorId;
    QString hostAddress;
    int inPort;
    QString remoteAddress;
    int outPort;
    QString binPath;
    QString dataPath;

    bool homeLocation;
    quint16 homeLocRate;

    bool attRaw;
    quint8 attRawRate;

    bool attActual;
    bool attActHW;
    bool attActSim;
    bool attActCalc;

    bool sonarAltitude;
    float sonarMaxAlt;
    quint16 sonarAltRate;

    bool gpsPosition;
    quint16 gpsPosRate;

    bool inputCommand;
    bool gcsReciever;
    bool manualControl;
    bool manualOutput;
    quint8 outputRate;

} SimulatorSettings;

class Simulator : public QObject
{
    Q_OBJECT

public:
    Simulator(const SimulatorSettings& params);
    virtual ~Simulator();

    bool isAutopilotConnected() const { return autopilotConnectionStatus; }
    bool isSimulatorConnected() const { return simConnectionStatus; }

    QString Name() const { return name; }
    void setName(QString str) { name = str; }

    QString SimulatorId() const { return simulatorId; }
    void setSimulatorId(QString str) { simulatorId = str; }

    static bool IsStarted() { return isStarted; }
    static void setStarted(bool val) { isStarted = val; }

    static QStringList& Instances() { return Simulator::instances; }
    static void setInstance(const QString& str) { Simulator::instances.append(str); }

    virtual void stopProcess() {}
    virtual void setupUdpPorts(const QString& host, int inPort, int outPort) { Q_UNUSED(host) Q_UNUSED(inPort) Q_UNUSED(outPort)}

signals:
    void processOutput(QString str);
    void autopilotConnected();
    void autopilotDisconnected();
    void simulatorConnected();
    void simulatorDisconnected();
    void myStart();

public slots:
    Q_INVOKABLE virtual bool setupProcess() { return true; }

private slots:
    void onStart();
    void receiveUpdate();
    void onAutopilotConnect();
    void onAutopilotDisconnect();
    void onSimulatorConnectionTimeout();
    void telStatsUpdated(UAVObject* obj);
    Q_INVOKABLE void onDeleteSimulator(void);

    virtual void transmitUpdate() = 0;
    virtual void processUpdate(const QByteArray& data) = 0;

protected:
    static const float GEE;
    static const float FT2M;
    static const float KT2MPS;
    static const float INHG2KPA;
    static const float FPS2CMPS;
    static const float DEG2RAD;
    static const float RAD2DEG;

#ifdef DBG_TIMERS
    QTime timeRX;
    QTime timeTX;
#endif

    QUdpSocket* inSocket;
    QUdpSocket* outSocket;

//    ActuatorDesired* actDesired;
//    ManualControlCommand* manCtrlCommand;
//    VelocityActual* velActual;
//    PositionActual* posActual;
//    BaroAltitude* altActual;
//    CameraDesired *camDesired;
//    AccessoryDesired *acsDesired;
    Accels *accels;
    Gyros *gyros;
    AttitudeActual *attActual;
    HomeLocation *posHome;
    FlightStatus *flightStatus;
    GPSPosition *gpsPosition;
    GCSReceiver *gcsReceiver;
    ActuatorCommand *actCommand;
    AttitudeSettings *attSettings;
    SonarAltitude *sonarAlt;

    GCSTelemetryStats* telStats;
    SimulatorSettings settings;

    QMutex lock;

private:
    int updatePeriod;
    int simTimeout;
    volatile bool autopilotConnectionStatus;
    volatile bool simConnectionStatus;
    QTimer* txTimer;
    QTimer* simTimer;
    QString name;
    QString simulatorId;
    volatile static bool isStarted;
    static QStringList instances;

    void setupObjects();
    void resetAllObjects();
    void setupInputObject(UAVObject* obj, quint32 updateRate);
    void setupOutputObject(UAVObject* obj);
    void setupWatchedObject(UAVObject *obj);
    void setupDefaultObject(UAVObject *obj);
};

class SimulatorCreator
{
public:
    SimulatorCreator(QString id, QString descr) :
        classId(id),
        description(descr)
    {}
    virtual ~SimulatorCreator() {}

    QString ClassId() const { return classId; }
    QString Description() const { return description; }

    virtual Simulator* createSimulator(const SimulatorSettings& params) = 0;

private:
    QString classId;
    QString description;
};

#endif // ISIMULATORV2_H
