#ifndef ISIMULATOR_H
#define ISIMULATOR_H

#include "uavobjectmanager.h"
#include "actuatordesired.h"
#include "altitudestate.h"
#include "attitudestate.h"
#include "velocitystate.h"
#include "positionstate.h"
#include "gcstelemetrystats.h"

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <math.h>

class Simulator : public QObject {
    Q_OBJECT
public:
    // static ISimulator* Instance();
// protected:
    Simulator();
    ~ISimulator();

    bool isAutopilotConnected();
    bool isFGConnected();

signals:
    void myStart();
    void autopilotConnected();
    void autopilotDisconnected();
    void fgConnected();
    void fgDisconnected();

private slots:
    void onStart();
    void transmitUpdate();
    void receiveUpdate();
    void onAutopilotConnect();
    void onAutopilotDisconnect();
    void onFGConnectionTimeout();
    void telStatsUpdated(UAVObject *obj);

private:
    // static ISimulator* _instance;

    QUdpSocket *inSocket;
    QUdpSocket *outSocket;
    ActuatorDesired *actDesired;
    AltitudeState *altState;
    VelocityState *velState;
    AttitudeState *attState;
    PositionState *posState;
    GCSTelemetryStats *telStats;
    QHostAddress fgHost;
    int inPort;
    int outPort;
    int updatePeriod;
    QTimer *txTimer;
    QTimer *fgTimer;
    bool autopilotConnectionStatus;
    bool fgConnectionStatus;
    int fgTimeout;

    void processUpdate(QString & data);
    void setupOutputObject(UAVObject *obj, int updatePeriod);
    void setupInputObject(UAVObject *obj, int updatePeriod);
    void setupObjects();
};


#endif // ISIMULATOR_H
