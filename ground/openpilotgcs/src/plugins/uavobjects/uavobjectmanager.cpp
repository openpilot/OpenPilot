/**
 ******************************************************************************
 *
 * @file       uavobjectmanager.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 * @brief      The UAVUObjects GCS plugin
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
#include "uavobjectmanager.h"

/**
 * Constructor
 */
UAVObjectManager::UAVObjectManager()
{
    mutex = new QMutex(QMutex::Recursive);
}

UAVObjectManager::~UAVObjectManager()
{
    delete mutex;
}

/**
 * Register an object with the manager. This function must be called for all newly created instances.
 * A new instance can be created directly by instantiating a new object or by calling clone() of
 * an existing object. The object will be registered and will be properly initialized so that it can accept
 * updates.
 */
bool UAVObjectManager::registerObject(UAVDataObject* obj)
{
    QMutexLocker locker(mutex);
    // Check if this object type is already in the list
    for (int objidx = 0; objidx < objects.length(); ++objidx)
    {
        // Check if the object ID is in the list
        if (objects[objidx].length() > 0 && objects[objidx][0]->getObjID() == obj->getObjID())
        {
            // Check if this is a single instance object, if yes we can not add a new instance
            if (obj->isSingleInstance())
            {
                return false;
            }
            // The object type has alredy been added, so now we need to initialize the new instance with the appropriate id
            // There is a single metaobject for all object instances of this type, so no need to create a new one
            // Get object type metaobject from existing instance
            UAVDataObject* refObj = dynamic_cast<UAVDataObject*>(objects[objidx][0]);
            if (refObj == NULL)
            {
                return false;
            }
            UAVMetaObject* mobj = refObj->getMetaObject();
            // If the instance ID is specified and not at the default value (0) then we need to make sure
            // that there are no gaps in the instance list. If gaps are found then then additional instances
            // will be created.
            if ( (obj->getInstID() > 0) && (obj->getInstID() < MAX_INSTANCES) )
            {
                for (int instidx = 0; instidx < objects[objidx].length(); ++instidx)
                {
                    if ( objects[objidx][instidx]->getInstID() == obj->getInstID() )
                    {
                        // Instance conflict, do not add
                        return false;
                    }
                }
                // Check if there are any gaps between the requested instance ID and the ones in the list,
                // if any then create the missing instances.
                for (quint32 instidx = objects[objidx].length(); instidx < obj->getInstID(); ++instidx)
                {
                    UAVDataObject* cobj = obj->clone(instidx);
                    cobj->initialize(mobj);
                    objects[objidx].append(cobj);
                    getObject(cobj->getObjID())->emitNewInstance(cobj);
                    emit newInstance(cobj);
                }
                // Finally, initialize the actual object instance
                obj->initialize(mobj);
            }
            else if (obj->getInstID() == 0)
            {
                // Assign the next available ID and initialize the object instance
                obj->initialize(objects[objidx].length(), mobj);
            }
            else
            {
                return false;
            }
            // Add the actual object instance in the list
            objects[objidx].append(obj);
            getObject(obj->getObjID())->emitNewInstance(obj);
            emit newInstance(obj);
            return true;
        }
    }
    // If this point is reached then this is the first time this object type (ID) is added in the list
    // create a new list of the instances, add in the object collection and create the object's metaobject
    // Create metaobject
    QString mname = obj->getName();
    mname.append("Meta");
    UAVMetaObject* mobj = new UAVMetaObject(obj->getObjID()+1, mname, obj);
    // Initialize object
    obj->initialize(0, mobj);
    // Add to list
    addObject(obj);
    addObject(mobj);
    return true;
}

void UAVObjectManager::addObject(UAVObject* obj)
{
    // Add to list
    QList<UAVObject*> list;
    list.append(obj);
    objects.append(list);
    emit newObject(obj);
}

/**
 * Get all objects. A two dimentional QList is returned. Objects are grouped by
 * instances of the same object type.
 */
QList< QList<UAVObject*> > UAVObjectManager::getObjects()
{
    QMutexLocker locker(mutex);
    return objects;
}

/**
 * Same as getObjects() but will only return DataObjects.
 */
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

/**
 * Same as getObjects() but will only return MetaObjects.
 */
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

/**
 * Get a specific object given its name and instance ID
 * @returns The object is found or NULL if not
 */
UAVObject* UAVObjectManager::getObject(const QString& name, quint32 instId)
{
    return getObject(&name, 0, instId);
}

/**
 * Get a specific object given its object and instance ID
 * @returns The object is found or NULL if not
 */
UAVObject* UAVObjectManager::getObject(quint32 objId, quint32 instId)
{
    return getObject(NULL, objId, instId);
}

/**
 * Helper function for the public getObject() functions.
 */
UAVObject* UAVObjectManager::getObject(const QString* name, quint32 objId, quint32 instId)
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
    //qWarning("UAVObjectManager::getObject: Object not found.  Probably a bug or mismatched GCS/flight versions.");
    // If this point is reached then the requested object could not be found
    return NULL;
}

/**
 * Get all the instances of the object specified by name
 */
QList<UAVObject*> UAVObjectManager::getObjectInstances(const QString& name)
{
    return getObjectInstances(&name, 0);
}

/**
 * Get all the instances of the object specified by its ID
 */
QList<UAVObject*> UAVObjectManager::getObjectInstances(quint32 objId)
{
    return getObjectInstances(NULL, objId);
}

/**
 * Helper function for the public getObjectInstances()
 */
QList<UAVObject*> UAVObjectManager::getObjectInstances(const QString* name, quint32 objId)
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

/**
 * Get the number of instances for an object given its name
 */
qint32 UAVObjectManager::getNumInstances(const QString& name)
{
    return getNumInstances(&name, 0);
}

/**
 * Get the number of instances for an object given its ID
 */
qint32 UAVObjectManager::getNumInstances(quint32 objId)
{
    return getNumInstances(NULL, objId);
}

/**
 * Helper function for public getNumInstances
 */
qint32 UAVObjectManager::getNumInstances(const QString* name, quint32 objId)
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
