/**
 ******************************************************************************
 *
 * @file       telemetry.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   
 * @{
 * 
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

#include "telemetry.h"
#include <QTime>

/**
 * Constructor
 */
Telemetry::Telemetry(UAVTalk* utalk, UAVObjectManager* objMngr)
{
    this->utalk = utalk;
    this->objMngr = objMngr;
    mutex = new QMutex(QMutex::Recursive);
    // Process all objects in the list
    QList< QList<UAVObject*> > objs = objMngr->getObjects();
    for (int objidx = 0; objidx < objs.length(); ++objidx)
    {
        registerObject(objs[objidx][0]); // we only need to register one instance per object type
    }
    // Listen to new object creations
    connect(objMngr, SIGNAL(newObject(UAVObject*)), this, SLOT(newObject(UAVObject*)), Qt::QueuedConnection);
    connect(objMngr, SIGNAL(newInstance(UAVObject*)), this, SLOT(newInstance(UAVObject*)), Qt::QueuedConnection);
    // Setup and start the timer
    timeToNextUpdateMs = 0;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(processPeriodicUpdates()));
    timer->start(1000);
    // Start thread
    start();
}

/**
 * Event loop
 */
void Telemetry::run()
{
    // Start main event loop
    exec();
}

/**
 * Register a new object for periodic updates (if enabled)
 */
void Telemetry::registerObject(UAVObject* obj)
{
    // Setup object for periodic updates
    addObject(obj);

    // Setup object for telemetry updates
    updateObject(obj);
}

/**
 * Add an object in the list used for periodic updates
 */
void Telemetry::addObject(UAVObject* obj)
{
    // Check if object type is already in the list
    for (int n = 0; n < objList.length(); ++n)
    {
        if ( objList[n].obj->getObjID() == obj->getObjID() )
        {
            // Object type (not instance!) is already in the list, do nothing
            return;
        }
    }

    // If this point is reached, then the object type is new, let's add it
    ObjectTimeInfo timeInfo;
    timeInfo.obj = obj;
    timeInfo.timeToNextUpdateMs = 0;
    timeInfo.updatePeriodMs = 0;
    objList.append(timeInfo);
}

/**
 * Update the object's timers
 */
void Telemetry::setUpdatePeriod(UAVObject* obj, qint32 periodMs)
{
    // Find object type (not instance!) and update its period
    for (int n = 0; n < objList.length(); ++n)
    {
        if ( objList[n].obj->getObjID() == obj->getObjID() )
        {
            objList[n].updatePeriodMs = periodMs;
            objList[n].timeToNextUpdateMs = 0;
        }
    }
}

/**
 * Connect to all instances of an object depending on the event mask specified
 */
void Telemetry::connectToObjectInstances(UAVObject* obj, quint32 eventMask)
{
    QList<UAVObject*> objs = objMngr->getObjectInstances(obj->getObjID());
    for (int n = 0; n < objs.length(); ++n)
    {
        // Disconnect all
        objs[n]->disconnect(this);
        // Connect only the selected events
        if ( (eventMask&EV_UNPACKED) != 0)
        {
            connect(objs[n], SIGNAL(objectUnpacked(UAVObject*)), this, SLOT(objectUnpacked(UAVObject*)), Qt::QueuedConnection);
        }
        if ( (eventMask&EV_UPDATED) != 0)
        {
            connect(objs[n], SIGNAL(objectUpdatedAuto(UAVObject*)), this, SLOT(objectUpdatedAuto(UAVObject*)), Qt::QueuedConnection);
        }
        if ( (eventMask&EV_UPDATED_MANUAL) != 0)
        {
            connect(objs[n], SIGNAL(objectUpdatedManual(UAVObject*)), this, SLOT(objectUpdatedManual(UAVObject*)), Qt::QueuedConnection);
        }
        if ( (eventMask&EV_UPDATE_REQ) != 0)
        {
            connect(objs[n], SIGNAL(updateRequested(UAVObject*)), this, SLOT(updateRequested(UAVObject*)), Qt::QueuedConnection);
        }
    }
}

/**
 * Update an object based on its metadata properties
 */
void Telemetry::updateObject(UAVObject* obj)
{
    // Get metadata
    UAVObject::Metadata metadata = obj->getMetadata();

    // Setup object depending on update mode
    qint32 eventMask;
    if( metadata.gcsTelemetryUpdateMode == UAVObject::UPDATEMODE_PERIODIC )
    {
        // Set update period
        setUpdatePeriod(obj, metadata.gcsTelemetryUpdatePeriod);
        // Connect signals for all instances
        eventMask = EV_UPDATED_MANUAL | EV_UPDATE_REQ;
        if( dynamic_cast<UAVMetaObject*>(obj) != NULL )
        {
            eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
        }
        connectToObjectInstances(obj, eventMask);
    }
    else if(metadata.gcsTelemetryUpdateMode == UAVObject::UPDATEMODE_ONCHANGE)
    {
        // Set update period
        setUpdatePeriod(obj, 0);
        // Connect signals for all instances
        eventMask = EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ;
        if( dynamic_cast<UAVMetaObject*>(obj) != NULL )
        {
            eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
        }
        connectToObjectInstances(obj, eventMask);
    }
    else if(metadata.gcsTelemetryUpdateMode == UAVObject::UPDATEMODE_MANUAL)
    {
        // Set update period
        setUpdatePeriod(obj, 0);
        // Connect signals for all instances
        eventMask = EV_UPDATED_MANUAL | EV_UPDATE_REQ;
        if( dynamic_cast<UAVMetaObject*>(obj) != NULL )
        {
            eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
        }
        connectToObjectInstances(obj, eventMask);
    }
    else if(metadata.gcsTelemetryUpdateMode == UAVObject::UPDATEMODE_NEVER)
    {
        // Set update period
        setUpdatePeriod(obj, 0);
        // Disconnect from object
        connectToObjectInstances(obj, 0);
    }
}

/**
 * Process the event received from an object
 */
void Telemetry::processObjectUpdates(UAVObject* obj, EventMask event, bool allInstances)
{
    qint32 retries = 0;
    bool success = false;
    // Get object metadata
    UAVObject::Metadata metadata = obj->getMetadata();
    // Act on event
    if(event == EV_UPDATED || event == EV_UPDATED_MANUAL)
    {
        // Send update to autopilot (with retries)
        retries = 0;
        while(retries < MAX_RETRIES && !success)
        {
            success = utalk->sendObject(obj, metadata.gcsTelemetryAcked, REQ_TIMEOUT_MS, allInstances); // call blocks until ack is received or timeout
            ++retries;
        }
    }
    else if(event == EV_UPDATE_REQ)
    {
        // Request object update from autopilot (with retries)
        retries = 0;
        while(retries < MAX_RETRIES && !success)
        {
            success = utalk->sendObjectRequest(obj, REQ_TIMEOUT_MS, allInstances); // call blocks until update is received or timeout
            ++retries;
        }
    }
    // If this is a metaobject then make necessary telemetry updates
    UAVMetaObject* metaobj = dynamic_cast<UAVMetaObject*>(obj);
    if ( metaobj != NULL )
    {
        updateObject( metaobj->getParentObject() );
    }
}

/**
 * Check is any objects are pending for periodic updates
 */
void Telemetry::processPeriodicUpdates()
{
    QMutexLocker locker(mutex);

    // Stop timer
    timer->stop();

    // Iterate through each object and update its timer, if zero then transmit object.
    // Also calculate smallest delay to next update (will be used for setting timeToNextUpdateMs)
    qint32 minDelay = MAX_UPDATE_PERIOD_MS;
    ObjectTimeInfo objinfo;
    qint32 elapsedMs = 0;
    QTime time;
    for (int n = 0; n < objList.length(); ++n)
    {
        objinfo = objList[n];
        // If object is configured for periodic updates
        if (objinfo.updatePeriodMs > 0)
        {
            objinfo.timeToNextUpdateMs -= timeToNextUpdateMs;
            // Check if time for the next update
            if (objinfo.timeToNextUpdateMs <= 0)
            {
                // Reset timer
                objinfo.timeToNextUpdateMs = objinfo.updatePeriodMs;
                // Send object
                time.start();
                processObjectUpdates(objinfo.obj, EV_UPDATED_MANUAL, true);
                elapsedMs = time.elapsed();
                // Update timeToNextUpdateMs with the elapsed delay of sending the object;
                timeToNextUpdateMs += elapsedMs;
            }
            // Update minimum delay
            if (objinfo.timeToNextUpdateMs < minDelay)
            {
                minDelay = objinfo.timeToNextUpdateMs;
            }
        }
    }

    // Check if delay for the next update is too short
    if (minDelay < MIN_UPDATE_PERIOD_MS)
    {
        minDelay = MIN_UPDATE_PERIOD_MS;
    }

    // Done
    timeToNextUpdateMs = minDelay;

    // Restart timer
    timer->start(timeToNextUpdateMs);
}

void Telemetry::objectUpdatedAuto(UAVObject* obj)
{
    QMutexLocker locker(mutex);
    processObjectUpdates(obj, EV_UPDATED, false);
}

void Telemetry::objectUpdatedManual(UAVObject* obj)
{
    QMutexLocker locker(mutex);
    processObjectUpdates(obj, EV_UPDATED_MANUAL, false);
}

void Telemetry::objectUnpacked(UAVObject* obj)
{
    QMutexLocker locker(mutex);
    processObjectUpdates(obj, EV_UNPACKED, false);
}

void Telemetry::updateRequested(UAVObject* obj)
{
    QMutexLocker locker(mutex);
    processObjectUpdates(obj, EV_UPDATE_REQ, false);
}

void Telemetry::newObject(UAVObject* obj)
{
    QMutexLocker locker(mutex);
    registerObject(obj);
}

void Telemetry::newInstance(UAVObject* obj)
{
    QMutexLocker locker(mutex);
    registerObject(obj);
}




