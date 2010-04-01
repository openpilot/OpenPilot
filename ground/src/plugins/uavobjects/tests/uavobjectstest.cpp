#include "uavobjectstest.h"

UAVObjectsTest::UAVObjectsTest(): sout(stdout), done(false)
{
    // Create object manager
    objMngr = new UAVObjectManager();
    connect(objMngr, SIGNAL(newObject(UAVObject*)), this, SLOT(newObject(UAVObject*)));
    connect(objMngr, SIGNAL(newInstance(UAVObject*)), this, SLOT(newInstance(UAVObject*)));

    // Create test objects
    obj1 = new ExampleObject();
    connect(obj1, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(objectUpdated(UAVObject*)));
    connect(obj1, SIGNAL(objectUpdatedAuto(UAVObject*)), this, SLOT(objectUpdatedAuto(UAVObject*)));
    connect(obj1, SIGNAL(objectUpdatedManual(UAVObject*)), this, SLOT(objectUpdatedManual(UAVObject*)));
    connect(obj1, SIGNAL(objectUnpacked(UAVObject*)), this, SLOT(objectUnpacked(UAVObject*)));
    connect(obj1, SIGNAL(updateRequested(UAVObject*)), this, SLOT(updateRequested(UAVObject*)));
    objMngr->registerObject(obj1);

    // Setup timer
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(runTest()));
    timer->start(1000);
}

void UAVObjectsTest::objectUpdated(UAVObject* obj)
{
    sout << QString("[Object Updated]\n%1").arg(obj->toString());
}

void UAVObjectsTest::objectUpdatedAuto(UAVObject* obj)
{
    sout << QString("[Object Updated AUTO]\n%1").arg(obj->toString());
}

void UAVObjectsTest::objectUpdatedManual(UAVObject* obj)
{
    sout << QString("[Object Updated MANUAL]\n%1").arg(obj->toString());
}

void UAVObjectsTest::objectUnpacked(UAVObject* obj)
{
    sout << QString("[Object Updated UNPACKED]\n%1").arg(obj->toString());
}

void UAVObjectsTest::updateRequested(UAVObject* obj)
{
    sout << QString("[Object Update Requested]\n%1").arg(obj->toString());
}

void UAVObjectsTest::newObject(UAVObject* obj)
{
     sout << QString("[New object]\n%1").arg(obj->toString());
}

void UAVObjectsTest::newInstance(UAVObject* obj)
{
    sout << QString("[New instance]\n%1").arg(obj->toString());
}

void UAVObjectsTest::runTest()
{
    if (!done)
    {
        // Create a new instance
        ExampleObject* obj2 = new ExampleObject();
        objMngr->registerObject(obj2);    

        // Set data
        ExampleObject::DataFields data = obj1->getData();
        data.field1 = 1;
        data.field2 = 2;
        data.field3 = 3;
        data.field4[0] = 4.1;
        data.field4[1] = 4.2;
        data.field4[2] = 4.3;
        obj1->setData(data);

        // Set metadata
        UAVObject::Metadata mdata = obj1->getMetadata();
        mdata.gcsTelemetryUpdatePeriod = 123;
        obj1->setMetadata(mdata);
        // Print metadata of both instances
        sout << "[Meta of obj1]\n";
        sout << obj1->getMetaObject()->toString();
        sout << "[Meta of obj2]\n";
        sout << obj2->getMetaObject()->toString();

        // Create a new instance using clone() and an out of order instance ID
        UAVDataObject* obj3 = obj2->clone(5);
        objMngr->registerObject(obj3);

        // Pack, unpack testing
        quint8* buf = new quint8[obj1->getNumBytes()];
        obj1->pack(buf);
        data.field1 = 10;
        data.field2 = 20;
        data.field3 = 30;
        data.field4[0] = 40.1;
        data.field4[1] = 40.2;
        data.field4[2] = 40.3;
        obj1->setData(data);
        obj1->unpack(buf);

        // Save, load testing
        obj1->save();
        data.field1 = 10;
        data.field2 = 20;
        data.field3 = 30;
        data.field4[0] = 40.1;
        data.field4[1] = 40.2;
        data.field4[2] = 40.3;
        obj1->setData(data);
        obj1->load();

        // Get all instances
        QList<UAVObject*> objs = objMngr->getObjectInstances(ExampleObject::OBJID);
        for (int n = 0; n < objs.length(); ++n)
        {
            sout << "[Printing object instances]\n";
            sout << objs[n]->toString();
        }

        // Done
        done = true;
    }
    sout.flush();
}
