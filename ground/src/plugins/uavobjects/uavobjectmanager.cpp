#include "uavobjectmanager.h"

UAVObjectManager::UAVObjectManager()
{
    mutex = new QMutex(QMutex::Recursive);
}

bool UAVObjectManager::registerObject(UAVObject* obj)
{
    QMutexLocker locker(mutex);
    // Check if this object type is already in the list
    for (int objidx = 0; objidx < objects.length(); ++objidx)
    {
        // Check if the object ID is in the list
        if (objects[objidx].length() > 0 && objects[objidx][0]->getObjID() == obj->getObjID())
        {
            // Check if the instance is already in the list
            for (int instidx = 0; instidx < objects[objidx].length(); ++instidx)
            {
                if ( objects[objidx][instidx]->getInstID() == obj->getInstID() )
                {
                    // Object already registered, do not add
                    return false;
                }
            }
            // If this point is reached, the object is not in the list, so now we can add it in the existing list
            objects[objidx].append(obj);
            emit newObject(obj);
            return true;
        }
    }
    // If this point is reached then this is the first time this object ID is added in the list
    // create a new list of the instances and add in the object collection.
    QList<UAVObject*> list;
    list.append(obj);
    objects.append(list);
    emit newObject(obj);
    return true;
}

UAVDataObject* UAVObjectManager::newObjectInstance(QString& name, quint32 instId)
{
    return newObjectInstance(&name, 0, instId);
}

UAVDataObject* UAVObjectManager::newObjectInstance(quint32 objId, quint32 instId)
{
    return newObjectInstance(NULL, objId, instId);
}

UAVDataObject* UAVObjectManager::newObjectInstance(QString* name, quint32 objId, quint32 instId)
{
    QMutexLocker locker(mutex);
    // Get object of the same name from collection
    UAVObject* tmpObj;
    if (name != NULL)
    {
        tmpObj = getObject(*name);
    }
    else
    {
        tmpObj = getObject(objId);
    }
    if (tmpObj == NULL)
    {
        return NULL;
    }
    // Make sure this is a data object
    UAVDataObject* refObj = dynamic_cast<UAVDataObject*>(tmpObj);
    if (refObj == NULL)
    {
        return NULL;
    }
    // Check if this is single instance object
    if (refObj->isSingleInstance())
    {
        return NULL;
    }
    // Make a deep copy of the fields in the reference object
    QList<UAVObjectField*> fields;
    QList<UAVObjectField*> refFields = refObj->getFields();
    for (int n = 0; n < refFields.length(); ++n)
    {
        QString fname = refFields[n]->getName();
        QString funits = refFields[n]->getUnits();
        UAVObjectField* field = new UAVObjectField(fname, funits, refFields[n]->getType(),
                                                   refFields[n]->getNumElements());

        fields.append(field);
    }
    // Calculate instance ID, if the one specified is 0
    // (the first object registered always gets the instance ID of zero, so any new instances will be >1)
    if (instId == 0)
    {
        instId = getNumInstances(tmpObj->getObjID()) + 1;
    }
    // Create new instance, by using properties from reference object
    QString oname = refObj->getName();
    UAVDataObject* obj = new UAVDataObject(refObj->getObjID(), instId, refObj->isSingleInstance(),
                                           oname, refObj->getNumBytes());
    obj->initialize(fields, refObj->getMetaObject());
    // Register
    registerObject(obj);
    // Trigger update
    obj->updated();
    return obj;
}

QList< QList<UAVObject*> > UAVObjectManager::getObjects()
{
    QMutexLocker locker(mutex);
    return objects;
}

QList< QList<UAVDataObject*> > UAVObjectManager::getDataObjects()
{
    QMutexLocker locker(mutex);
    QList< QList<UAVDataObject*> > dObjects;

    // Go through objects and copy to new list when types match
    for (int objidx = 0; objidx < objects.length(); ++objidx)
    {
        if (objects[objidx].length() > 0)
        {
            // Check type
            UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objects[objidx][0]);
            if (obj != NULL)
            {
                // Create instance list
                QList<UAVDataObject*> list;
                // Go through instances and cast them to UAVDataObject, then add to list
                for (int instidx = 0; instidx < objects[objidx].length(); ++instidx)
                {
                   obj = dynamic_cast<UAVDataObject*>(objects[objidx][instidx]);
                   if (obj != NULL)
                   {
                       list.append(obj);
                   }
                }
                // Append to object list
                dObjects.append(list);
            }
        }
    }
    // Done
    return dObjects;
}

QList <QList<UAVMetaObject*> > UAVObjectManager::getMetaObjects()
{
    QMutexLocker locker(mutex);
    QList< QList<UAVMetaObject*> > mObjects;

    // Go through objects and copy to new list when types match
    for (int objidx = 0; objidx < objects.length(); ++objidx)
    {
        if (objects[objidx].length() > 0)
        {
            // Check type
            UAVMetaObject* obj = dynamic_cast<UAVMetaObject*>(objects[objidx][0]);
            if (obj != NULL)
            {
                // Create instance list
                QList<UAVMetaObject*> list;
                // Go through instances and cast them to UAVMetaObject, then add to list
                for (int instidx = 0; instidx < objects[objidx].length(); ++instidx)
                {
                   obj = dynamic_cast<UAVMetaObject*>(objects[objidx][instidx]);
                   if (obj != NULL)
                   {
                       list.append(obj);
                   }
                }
                // Append to object list
                mObjects.append(list);
            }
        }
    }
    // Done
    return mObjects;
}

UAVObject* UAVObjectManager::getObject(QString& name, quint32 instId)
{
    return getObject(&name, 0, instId);
}

UAVObject* UAVObjectManager::getObject(quint32 objId, quint32 instId)
{
    return getObject(NULL, objId, instId);
}

UAVObject* UAVObjectManager::getObject(QString* name, quint32 objId, quint32 instId)
{
    QMutexLocker locker(mutex);
    // Check if this object type is already in the list
    for (int objidx = 0; objidx < objects.length(); ++objidx)
    {
        // Check if the object ID is in the list
        if (objects[objidx].length() > 0)
        {
            if ( (name != NULL && objects[objidx][0]->getName().compare(name) == 0) || (name == NULL && objects[objidx][0]->getObjID() == objId) )
            {
                // Look for the requested instance ID
                for (int instidx = 0; instidx < objects[objidx].length(); ++instidx)
                {
                    if (objects[objidx][instidx]->getInstID() == instId)
                    {
                        return objects[objidx][instidx];
                    }
                }
            }
        }
    }
    // If this point is reached then the requested object could not be found
    return NULL;
}

QList<UAVObject*> UAVObjectManager::getObjectInstances(QString& name)
{
    return getObjectInstances(&name, 0);
}

QList<UAVObject*> UAVObjectManager::getObjectInstances(quint32 objId)
{
    return getObjectInstances(NULL, objId);
}

QList<UAVObject*> UAVObjectManager::getObjectInstances(QString* name, quint32 objId)
{
    QMutexLocker locker(mutex);
    // Check if this object type is already in the list
    for (int objidx = 0; objidx < objects.length(); ++objidx)
    {
        // Check if the object ID is in the list
        if (objects[objidx].length() > 0)
        {
            if ( (name != NULL && objects[objidx][0]->getName().compare(name) == 0) || (name == NULL && objects[objidx][0]->getObjID() == objId) )
            {
                return objects[objidx];
            }
        }
    }
    // If this point is reached then the requested object could not be found
    return QList<UAVObject*>();
}

qint32 UAVObjectManager::getNumInstances(QString& name)
{
    return getNumInstances(&name, 0);
}

qint32 UAVObjectManager::getNumInstances(quint32 objId)
{
    return getNumInstances(NULL, objId);
}

qint32 UAVObjectManager::getNumInstances(QString* name, quint32 objId)
{
    QMutexLocker locker(mutex);
    // Check if this object type is already in the list
    for (int objidx = 0; objidx < objects.length(); ++objidx)
    {
        // Check if the object ID is in the list
        if (objects[objidx].length() > 0)
        {
            if ( (name != NULL && objects[objidx][0]->getName().compare(name) == 0) || (name == NULL && objects[objidx][0]->getObjID() == objId) )
            {
                return objects[objidx].length();
            }
        }
    }
    // If this point is reached then the requested object could not be found
    return -1;
}
