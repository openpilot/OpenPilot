#include "testobject1.h"

const QString TestObject1::NAME = QString("TestObject1");

TestObject1::TestObject1(): UAVDataObject(OBJID, SINGLEINST, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    fields.append(new UAVObjectField(QString("Field1"), QString("unit1"), UAVObjectField::FIELDTYPE_INT16, 1));
    fields.append(new UAVObjectField(QString("Field2"), QString("unit2"), UAVObjectField::FIELDTYPE_FLOAT32, 3));
    fields.append(new UAVObjectField(QString("Field3"), QString("unit3"), UAVObjectField::FIELDTYPE_INT8, 1));
    fields.append(new UAVObjectField(QString("Field4"), QString("unit4"), UAVObjectField::FIELDTYPE_INT32, 1));

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
}

UAVObject::Metadata TestObject1::getDefaultMetadata()
{
    // Create metadata
    UAVObject::Metadata metadata;
    metadata.ackRequired = true;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.gcsTelemetryUpdatePeriod = 200;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_NEVER;
    metadata.flightTelemetryUpdatePeriod = 0;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_NEVER;
    metadata.loggingUpdatePeriod = 0;
    return metadata;
}

TestObject1::DataFields TestObject1::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

void TestObject1::setData(DataFields& data)
{
    QMutexLocker locker(mutex);
    this->data = data;
    emit objectUpdatedAuto(this); // trigger object updated event
    emit objectUpdated(this);
}

UAVDataObject* TestObject1::clone(quint32 instID)
{
    TestObject1* obj = new TestObject1();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}
