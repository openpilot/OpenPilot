#ifndef UAVOBJECTSTEST_H
#define UAVOBJECTSTEST_H

#include "..\UAVObjectManager.h"
#include "testobject1.h"
#include <QTimer>
#include <QTextStream>

class UAVObjectsTest: QObject
{
    Q_OBJECT


public:
    UAVObjectsTest();

private slots:
    void objectUpdated(UAVObject* obj);
    void objectUpdatedAuto(UAVObject* obj);
    void objectUpdatedManual(UAVObject* obj);
    void objectUnpacked(UAVObject* obj);
    void updateRequested(UAVObject* obj);
    void runTest();
    void newObject(UAVObject* obj);
    void newInstance(UAVObject* obj);

private:
    UAVObjectManager* objMngr;
    TestObject1* obj1;
    QTimer* timer;
    QTextStream sout;
    bool done;
};

#endif // UAVOBJECTSTEST_H
