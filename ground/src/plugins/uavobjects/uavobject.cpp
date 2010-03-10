#include "uavobject.h"


UAVObject::UAVObject(quint32 objID, quint32 instID, bool isSingleInst, QString& name, quint32 numBytes): QObject()
{
    this->objID = objID;
    this->instID = instID;
    this->isSingleInst = isSingleInst;
    this->name = name;
    this->numBytes = numBytes;
    this->mutex = new QMutex(QMutex::Recursive);
}

quint32 UAVObject::getObjID()
{
    return objID;
}

quint32 UAVObject::getInstID()
{
    return instID;
}

bool UAVObject::isSingleInstance()
{
    return isSingleInst;
}

QString UAVObject::getName()
{
    return name;
}

quint32 UAVObject::getNumBytes()
{
    return numBytes;
}

void UAVObject::requestUpdate()
{
    emit updateRequested(this);
}

void UAVObject::updated()
{
    emit objectUpdated(this, false);
}

void UAVObject::lock()
{
    mutex->lock();
}

void UAVObject::lock(int timeoutMs)
{
    mutex->tryLock(timeoutMs);
}

void UAVObject::unlock()
{
    mutex->unlock();
}

QMutex* UAVObject::getMutex()
{
    return mutex;
}

