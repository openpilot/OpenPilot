#ifndef UAVOBJECTMANAGER_H
#define UAVOBJECTMANAGER_H

#include "uavobject.h"
#include "uavdataobject.h"
#include "uavmetaobject.h"
#include <QList>
#include <QMutex>

class UAVObjectManager: public QObject
{
    Q_OBJECT

public:
    UAVObjectManager();

    bool registerObject(UAVObject* obj);
    UAVDataObject* newObjectInstance(QString& name, quint32 instId = 0);
    UAVDataObject* newObjectInstance(quint32 objId, quint32 instId = 0);
    QList< QList<UAVObject*> > getObjects();
    QList< QList<UAVDataObject*> > getDataObjects();
    QList< QList<UAVMetaObject*> > getMetaObjects();
    UAVObject* getObject(QString& name, quint32 instId = 0);
    UAVObject* getObject(quint32 objId, quint32 instId = 0);
    QList<UAVObject*> getObjectInstances(QString& name);
    QList<UAVObject*> getObjectInstances(quint32 objId);
    qint32 getNumInstances(QString& name);
    qint32 getNumInstances(quint32 objId);

signals:
    void newObject(UAVObject* obj);

private:
    QList< QList<UAVObject*> > objects;
    QMutex* mutex;

    UAVDataObject* newObjectInstance(QString* name, quint32 objId, quint32 instId);
    UAVObject* getObject(QString* name, quint32 objId, quint32 instId);
    QList<UAVObject*> getObjectInstances(QString* name, quint32 objId);
    qint32 getNumInstances(QString* name, quint32 objId);
};


#endif // UAVOBJECTMANAGER_H
