#include "uavmetaobject.h"

UAVMetaObject::UAVMetaObject(quint32 objID, QString& name, Metadata& mdata, UAVObject* parent):
        UAVObject(objID, 0, true, name, sizeof(Metadata))
{
    this->parentMetadata = mdata;
    this->parent = parent;
    ownMetadata.ackRequired = 1;
    ownMetadata.flightTelemetryUpdateMode = UPDATEMODE_ONCHANGE;
    ownMetadata.flightTelemetryUpdatePeriod = 0;
    ownMetadata.gcsTelemetryUpdateMode = UPDATEMODE_ONCHANGE;
    ownMetadata.gcsTelemetryUpdatePeriod = 0;
    ownMetadata.loggingUpdateMode = UPDATEMODE_ONCHANGE;
    ownMetadata.loggingUpdatePeriod = 0;
}

UAVObject* UAVMetaObject::getParentObject()
{
    return parent;
}

qint32 UAVMetaObject::pack(quint8* dataOut)
{
    QMutexLocker locker(mutex);
    memcpy(dataOut, &parentMetadata, sizeof(Metadata));
    return sizeof(Metadata);
}

qint32 UAVMetaObject::unpack(const quint8* dataIn)
{
    QMutexLocker locker(mutex);
    memcpy(&parentMetadata, dataIn, sizeof(Metadata));
    emit objectUpdated(this, true); // trigger object updated event
    return sizeof(Metadata);
}

void UAVMetaObject::setMetadata(const Metadata& mdata)
{
    return; // can not update metaobject's metadata
}

UAVObject::Metadata UAVMetaObject::getMetadata()
{
    return ownMetadata;
}

void UAVMetaObject::setData(const Metadata& mdata)
{
    QMutexLocker locker(mutex);
    parentMetadata = mdata;
    emit objectUpdated(this, false); // trigger object updated event
}

UAVObject::Metadata UAVMetaObject::getData()
{
    QMutexLocker locker(mutex);
    return parentMetadata;
}
