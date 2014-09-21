/**
 ******************************************************************************
 *
 * @file       telemetry.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVTalkPlugin UAVTalk Plugin
 * @{
 * @brief The UAVTalk protocol plugin
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
#include "oplinksettings.h"
#include "objectpersistence.h"
#include <QTime>
#include <QtGlobal>
#include <stdlib.h>
#include <QDebug>

/**
 * Constructor
 */
Telemetry::Telemetry(UAVTalk *utalk, UAVObjectManager *objMngr) : objMngr(objMngr), utalk(utalk)
{
    mutex = new QMutex(QMutex::Recursive);

    // Register all objects in the list
    QList< QList<UAVObject *> > objs = objMngr->getObjects();
    for (int objidx = 0; objidx < objs.length(); ++objidx) {
        // we only need to register one instance per object type
        registerObject(objs[objidx][0]);
    }

    // Listen to new object creations
    // connection must be direct, if not, it is not possible to create and send (or request) an object in one go
    connect(objMngr, SIGNAL(newObject(UAVObject *)), this, SLOT(newObject(UAVObject *)), Qt::DirectConnection);
    connect(objMngr, SIGNAL(newInstance(UAVObject *)), this, SLOT(newInstance(UAVObject *)), Qt::DirectConnection);

    // Listen to transaction completions
    // these slots will be executed in the telemetry thread
    // TODO should send a status (SUCCESS, FAILED, TIMEOUT)
    connect(utalk, SIGNAL(transactionCompleted(UAVObject *, bool)), this, SLOT(transactionCompleted(UAVObject *, bool)));

    // Get GCS stats object
    gcsStatsObj = GCSTelemetryStats::GetInstance(objMngr);

    // Setup and start the periodic timer
    timeToNextUpdateMs = 0;
    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(processPeriodicUpdates()));
    updateTimer->start(1000);

    // Setup and start the stats timer
    txErrors  = 0;
    txRetries = 0;
}

Telemetry::~Telemetry()
{
    closeAllTransactions();
}

/**
 * Register a new object for periodic updates (if enabled)
 */
void Telemetry::registerObject(UAVObject *obj)
{
    // Setup object for periodic updates
    addObject(obj);

    // Setup object for telemetry updates
    updateObject(obj, EV_NONE);
}

/**
 * Add an object in the list used for periodic updates
 */
void Telemetry::addObject(UAVObject *obj)
{
    // Check if object type is already in the list
    for (int n = 0; n < objList.length(); ++n) {
        if (objList[n].obj->getObjID() == obj->getObjID()) {
            // Object type (not instance!) is already in the list, do nothing
            return;
        }
    }

    // If this point is reached, then the object type is new, let's add it
    ObjectTimeInfo timeInfo;
    timeInfo.obj = obj;
    timeInfo.timeToNextUpdateMs = 0;
    timeInfo.updatePeriodMs     = 0;
    objList.append(timeInfo);
}

/**
 * Update the object's timers
 */
void Telemetry::setUpdatePeriod(UAVObject *obj, qint32 periodMs)
{
    // Find object type (not instance!) and update its period
    for (int n = 0; n < objList.length(); ++n) {
        if (objList[n].obj->getObjID() == obj->getObjID()) {
            objList[n].updatePeriodMs     = periodMs;
            objList[n].timeToNextUpdateMs = quint32((float)periodMs * (float)qrand() / (float)RAND_MAX); // avoid bunching of updates
        }
    }
}

/**
 * Connect to all instances of an object depending on the event mask specified
 */
void Telemetry::connectToObjectInstances(UAVObject *obj, quint32 eventMask)
{
    // TODO why connect systematically to all instances?
    // It is probably not needed to connect to always connect to all instances.
    QList<UAVObject *> objs = objMngr->getObjectInstances(obj->getObjID());
    for (int n = 0; n < objs.length(); ++n) {
        connectToObject(objs[n], eventMask);
    }
}

/**
 * Connect to all instances of an object depending on the event mask specified
 */
void Telemetry::connectToObject(UAVObject *obj, quint32 eventMask)
{
    // Disconnect all
    obj->disconnect(this);
    // Connect only the selected events
    if ((eventMask & EV_UNPACKED) != 0) {
        connect(obj, SIGNAL(objectUnpacked(UAVObject *)), this, SLOT(objectUnpacked(UAVObject *)));
    }
    if ((eventMask & EV_UPDATED) != 0) {
        connect(obj, SIGNAL(objectUpdatedAuto(UAVObject *)), this, SLOT(objectUpdatedAuto(UAVObject *)));
    }
    if ((eventMask & EV_UPDATED_MANUAL) != 0) {
        connect(obj, SIGNAL(objectUpdatedManual(UAVObject *, bool)), this, SLOT(objectUpdatedManual(UAVObject *, bool)));
    }
    if ((eventMask & EV_UPDATED_PERIODIC) != 0) {
        connect(obj, SIGNAL(objectUpdatedPeriodic(UAVObject *)), this, SLOT(objectUpdatedPeriodic(UAVObject *)));
    }
    if ((eventMask & EV_UPDATE_REQ) != 0) {
        connect(obj, SIGNAL(updateRequested(UAVObject *, bool)), this, SLOT(updateRequested(UAVObject *, bool)));
    }
}

/**
 * Update an object based on its metadata properties
 */
void Telemetry::updateObject(UAVObject *obj, quint32 eventType)
{
    // Get metadata
    UAVObject::Metadata metadata     = obj->getMetadata();
    UAVObject::UpdateMode updateMode = UAVObject::GetGcsTelemetryUpdateMode(metadata);

    // Setup object depending on update mode
    qint32 eventMask;

    if (updateMode == UAVObject::UPDATEMODE_PERIODIC) {
        // Set update period
        setUpdatePeriod(obj, metadata.gcsTelemetryUpdatePeriod);
        // Connect signals
        eventMask = EV_UPDATED_MANUAL | EV_UPDATE_REQ | EV_UPDATED_PERIODIC;
        if (dynamic_cast<UAVMetaObject *>(obj) != NULL) {
            // we also need to act on remote updates (unpack events)
            eventMask |= EV_UNPACKED;
        }
        connectToObjectInstances(obj, eventMask);
    } else if (updateMode == UAVObject::UPDATEMODE_ONCHANGE) {
        // Set update period
        setUpdatePeriod(obj, 0);
        // Connect signals
        eventMask = EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ;
        if (dynamic_cast<UAVMetaObject *>(obj) != NULL) {
            // we also need to act on remote updates (unpack events)
            eventMask |= EV_UNPACKED;
        }
        connectToObjectInstances(obj, eventMask);
    } else if (updateMode == UAVObject::UPDATEMODE_THROTTLED) {
        // If we received a periodic update, we can change back to update on change
        if ((eventType == EV_UPDATED_PERIODIC) || (eventType == EV_NONE)) {
            // Set update period
            if (eventType == EV_NONE) {
                setUpdatePeriod(obj, metadata.gcsTelemetryUpdatePeriod);
            }
            // Connect signals
            eventMask = EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ | EV_UPDATED_PERIODIC;
        } else {
            // Otherwise, we just received an object update, so switch to periodic for the timeout period to prevent more updates
            // Connect signals
            eventMask = EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ;
        }
        if (dynamic_cast<UAVMetaObject *>(obj) != NULL) {
            // we also need to act on remote updates (unpack events)
            eventMask |= EV_UNPACKED;
        }
        connectToObjectInstances(obj, eventMask);
    } else if (updateMode == UAVObject::UPDATEMODE_MANUAL) {
        // Set update period
        setUpdatePeriod(obj, 0);
        // Connect signals
        eventMask = EV_UPDATED_MANUAL | EV_UPDATE_REQ;
        if (dynamic_cast<UAVMetaObject *>(obj) != NULL) {
            // we also need to act on remote updates (unpack events)
            eventMask |= EV_UNPACKED;
        }
        connectToObjectInstances(obj, eventMask);
    }
}

/**
 * Called when a transaction is successfully completed (uavtalk event)
 */
void Telemetry::transactionCompleted(UAVObject *obj, bool success)
{
    // Lookup the transaction in the transaction map.
    ObjectTransactionInfo *transInfo = findTransaction(obj);

    if (transInfo) {
        if (success) {
#ifdef VERBOSE_TELEMETRY
            qDebug() << "Telemetry - transaction successful for object" << obj->toStringBrief();
#endif
        } else {
            qWarning() << "Telemetry - !!! transaction failed for object" << obj->toStringBrief();
        }

        // Remove this transaction as it's complete.
        closeTransaction(transInfo);

        // Send signal
        obj->emitTransactionCompleted(success);

        // Process new object updates from queue
        processObjectQueue();
    } else {
        qWarning() << "Telemetry - Error: received a transaction completed when did not expect it for" << obj->toStringBrief();
    }
}

/**
 * Called when a transaction is not completed within the timeout period (timer event)
 */
void Telemetry::transactionTimeout(ObjectTransactionInfo *transInfo)
{
    // Check if more retries are pending
    if (transInfo->retriesRemaining > 0) {
#ifdef VERBOSE_TELEMETRY
        qDebug().nospace() << "Telemetry - transaction timed out for object " << transInfo->obj->toStringBrief() << ", retrying...";
#endif
        ++txRetries;
        --transInfo->retriesRemaining;

        // Retry the transaction
        processObjectTransaction(transInfo);
    } else {
        qWarning().nospace() << "Telemetry - !!! transaction timed out for object " << transInfo->obj->toStringBrief();

        ++txErrors;

        // Terminate transaction
        utalk->cancelTransaction(transInfo->obj);

        // Remove this transaction as it's complete.
        UAVObject *obj = transInfo->obj;
        closeTransaction(transInfo);

        // Send signal
        obj->emitTransactionCompleted(false);

        // Process new object updates from queue
        processObjectQueue();
    }
}

/**
 * Start an object transaction with UAVTalk, all information is stored in transInfo
 */
void Telemetry::processObjectTransaction(ObjectTransactionInfo *transInfo)
{
    // Initiate transaction
    bool sent = false;

    if (transInfo->objRequest) {
#ifdef VERBOSE_TELEMETRY
        qDebug().nospace() << "Telemetry - sending request for object " << transInfo->obj->toStringBrief() << ", " << (transInfo->allInstances ? "all" : "single") << " " << (transInfo->acked ? "acked" : "");
#endif
        sent = utalk->sendObjectRequest(transInfo->obj, transInfo->allInstances);
    } else {
#ifdef VERBOSE_TELEMETRY
        qDebug().nospace() << "Telemetry - sending object " << transInfo->obj->toStringBrief() << ", " << (transInfo->allInstances ? "all" : "single") << " " << (transInfo->acked ? "acked" : "");
#endif
        sent = utalk->sendObject(transInfo->obj, transInfo->acked, transInfo->allInstances);
    }
    // Check if a response is needed now or will arrive asynchronously
    if (transInfo->objRequest || transInfo->acked) {
        if (sent) {
            // Start timer if a response is expected
            transInfo->timer->start(REQ_TIMEOUT_MS);
        } else {
            // message was not sent, the transaction will not complete and will timeout
            // there is no need to wait to close the transaction and notify of completion failure
            // transactionCompleted(transInfo->obj, false);
        }
    } else {
        // not transacted, so just close the transaction with no notification of completion
        closeTransaction(transInfo);
    }
}

/**
 * Process the event received from an object
 */
void Telemetry::processObjectUpdates(UAVObject *obj, EventMask event, bool allInstances, bool priority)
{
    // Push event into queue
    ObjectQueueInfo objInfo;

    objInfo.obj   = obj;
    objInfo.event = event;
    objInfo.allInstances = allInstances;
    if (priority) {
        if (objPriorityQueue.length() < MAX_QUEUE_SIZE) {
            objPriorityQueue.enqueue(objInfo);
        } else {
            ++txErrors;
            qWarning().nospace() << "Telemetry - !!! priority event queue is full, event lost " << obj->toStringBrief();
            obj->emitTransactionCompleted(false);
        }
    } else {
        if (objQueue.length() < MAX_QUEUE_SIZE) {
            objQueue.enqueue(objInfo);
        } else {
            ++txErrors;
            qWarning().nospace() << "Telemetry - !!! event queue is full, event lost " << obj->toStringBrief();
            obj->emitTransactionCompleted(false);
        }
    }

    // Process the transaction
    processObjectQueue();
}

/**
 * Process events from the object queue
 */
void Telemetry::processObjectQueue()
{
    // Get object information from queue (first the priority and then the regular queue)
    ObjectQueueInfo objInfo;

    if (!objPriorityQueue.isEmpty()) {
        objInfo = objPriorityQueue.dequeue();
    } else if (!objQueue.isEmpty()) {
        objInfo = objQueue.dequeue();
    } else {
        return;
    }

    // Check if a connection has been established, only process GCSTelemetryStats updates
    // (used to establish the connection)
    GCSTelemetryStats::DataFields gcsStats = gcsStatsObj->getData();
    if (gcsStats.Status != GCSTelemetryStats::STATUS_CONNECTED) {
        objQueue.clear();
        if ((objInfo.obj->getObjID() != GCSTelemetryStats::OBJID) &&
            (objInfo.obj->getObjID() != OPLinkSettings::OBJID) &&
            (objInfo.obj->getObjID() != ObjectPersistence::OBJID)) {
            objInfo.obj->emitTransactionCompleted(false);
            return;
        }
    }

    // Setup transaction (skip if unpack event)
    UAVObject::Metadata metadata     = objInfo.obj->getMetadata();
    UAVObject::UpdateMode updateMode = UAVObject::GetGcsTelemetryUpdateMode(metadata);
    if ((objInfo.event != EV_UNPACKED) && ((objInfo.event != EV_UPDATED_PERIODIC) || (updateMode != UAVObject::UPDATEMODE_THROTTLED))) {
        // Check if a transaction for that object already exists
        // It is allowed to have multiple transaction on the same object ID provided that the instance IDs are different
        // If an "all instances" transaction is running, then it is not allowed to start another transaction with same object ID
        // If a single instance transaction is running, then starting an "all instance" transaction is not allowed
        // TODO make the above logic a reality...
        if (findTransaction(objInfo.obj)) {
            qWarning().nospace() << "Telemetry - !!! Making request for an object " << objInfo.obj->toStringBrief() << " for which a request is already in progress";
            // objInfo.obj->emitTransactionCompleted(false);
            return;
        }
        UAVObject::Metadata metadata     = objInfo.obj->getMetadata();
        ObjectTransactionInfo *transInfo = new ObjectTransactionInfo(this);
        transInfo->obj   = objInfo.obj;
        transInfo->allInstances = objInfo.allInstances;
        transInfo->retriesRemaining = MAX_RETRIES;
        transInfo->acked = UAVObject::GetGcsTelemetryAcked(metadata);
        if (objInfo.event == EV_UPDATED || objInfo.event == EV_UPDATED_MANUAL || objInfo.event == EV_UPDATED_PERIODIC) {
            transInfo->objRequest = false;
        } else if (objInfo.event == EV_UPDATE_REQ) {
            transInfo->objRequest = true;
        }
        transInfo->telem = this;
        // Insert the transaction into the transaction map.
        openTransaction(transInfo);
        processObjectTransaction(transInfo);
    }

    // If this is a metaobject then make necessary telemetry updates
    UAVMetaObject *metaobj = dynamic_cast<UAVMetaObject *>(objInfo.obj);
    if (metaobj != NULL) {
        updateObject(metaobj->getParentObject(), EV_NONE);
    } else if (updateMode != UAVObject::UPDATEMODE_THROTTLED) {
        updateObject(objInfo.obj, objInfo.event);
    }

    // The fact we received an unpacked event does not mean that
    // we do not have additional objects still in the queue,
    // so we have to reschedule queue processing to make sure they are not
    // stuck:
    if (objInfo.event == EV_UNPACKED) {
        processObjectQueue();
    }
}

/**
 * Check is any objects are pending for periodic updates
 * TODO: Clean-up
 */
void Telemetry::processPeriodicUpdates()
{
    QMutexLocker locker(mutex);

    // Stop timer
    updateTimer->stop();

    // Iterate through each object and update its timer, if zero then transmit object.
    // Also calculate smallest delay to next update (will be used for setting timeToNextUpdateMs)
    qint32 minDelay  = MAX_UPDATE_PERIOD_MS;
    ObjectTimeInfo *objinfo;
    qint32 elapsedMs = 0;
    QTime time;
    qint32 offset;
    bool allInstances;
    for (int n = 0; n < objList.length(); ++n) {
        objinfo = &objList[n];
        // If object is configured for periodic updates
        if (objinfo->updatePeriodMs > 0) {
            objinfo->timeToNextUpdateMs -= timeToNextUpdateMs;
            // Check if time for the next update
            if (objinfo->timeToNextUpdateMs <= 0) {
                // Reset timer
                offset = (-objinfo->timeToNextUpdateMs) % objinfo->updatePeriodMs;
                objinfo->timeToNextUpdateMs = objinfo->updatePeriodMs - offset;
                // Send object
                time.start();
                allInstances = !objinfo->obj->isSingleInstance();
                processObjectUpdates(objinfo->obj, EV_UPDATED_PERIODIC, allInstances, false);
                elapsedMs    = time.elapsed();
                // Update timeToNextUpdateMs with the elapsed delay of sending the object;
                timeToNextUpdateMs += elapsedMs;
            }
            // Update minimum delay
            if (objinfo->timeToNextUpdateMs < minDelay) {
                minDelay = objinfo->timeToNextUpdateMs;
            }
        }
    }

    // Check if delay for the next update is too short
    if (minDelay < MIN_UPDATE_PERIOD_MS) {
        minDelay = MIN_UPDATE_PERIOD_MS;
    }

    // Done
    timeToNextUpdateMs = minDelay;

    // Restart timer
    updateTimer->start(timeToNextUpdateMs);
}

Telemetry::TelemetryStats Telemetry::getStats()
{
    QMutexLocker locker(mutex);

    // Get UAVTalk stats
    UAVTalk::ComStats utalkStats = utalk->getStats();

    // Update stats
    TelemetryStats stats;

    stats.txBytes       = utalkStats.txBytes;
    stats.txObjectBytes = utalkStats.txObjectBytes;
    stats.txObjects     = utalkStats.txObjects;
    stats.txErrors      = utalkStats.txErrors + txErrors;
    stats.txRetries     = txRetries;

    stats.rxBytes       = utalkStats.rxBytes;
    stats.rxObjectBytes = utalkStats.rxObjectBytes;
    stats.rxObjects     = utalkStats.rxObjects;
    stats.rxErrors      = utalkStats.rxErrors;
    stats.rxSyncErrors  = utalkStats.rxSyncErrors;
    stats.rxCrcErrors   = utalkStats.rxCrcErrors;

    // Done
    return stats;
}

void Telemetry::resetStats()
{
    QMutexLocker locker(mutex);

    utalk->resetStats();
    txErrors  = 0;
    txRetries = 0;
}

void Telemetry::objectUpdatedAuto(UAVObject *obj)
{
    QMutexLocker locker(mutex);

    processObjectUpdates(obj, EV_UPDATED, false, true);
}

void Telemetry::objectUpdatedManual(UAVObject *obj, bool all)
{
    QMutexLocker locker(mutex);

    bool allInstances = obj->isSingleInstance() ? false : all;

    processObjectUpdates(obj, EV_UPDATED_MANUAL, allInstances, true);
}

void Telemetry::objectUpdatedPeriodic(UAVObject *obj)
{
    QMutexLocker locker(mutex);

    processObjectUpdates(obj, EV_UPDATED_PERIODIC, false, true);
}

void Telemetry::objectUnpacked(UAVObject *obj)
{
    QMutexLocker locker(mutex);

    processObjectUpdates(obj, EV_UNPACKED, false, true);
}

void Telemetry::updateRequested(UAVObject *obj, bool all)
{
    QMutexLocker locker(mutex);

    bool allInstances = obj->isSingleInstance() ? false : all;

    processObjectUpdates(obj, EV_UPDATE_REQ, allInstances, true);
}

void Telemetry::newObject(UAVObject *obj)
{
    QMutexLocker locker(mutex);

#ifdef VERBOSE_TELEMETRY
    qDebug() << "Telemetry - new object" << obj->toStringBrief();
#endif

    registerObject(obj);
}

void Telemetry::newInstance(UAVObject *obj)
{
    QMutexLocker locker(mutex);

#ifdef VERBOSE_TELEMETRY
    qDebug() << "Telemetry - new object instance" << obj->toStringBrief();
#endif

    registerObject(obj);
}

ObjectTransactionInfo *Telemetry::findTransaction(UAVObject *obj)
{
    quint32 objId  = obj->getObjID();
    quint16 instId = obj->getInstID();

    // Lookup the transaction in the transaction map
    QMap<quint32, ObjectTransactionInfo *> *objTransactions = transMap.value(objId);
    if (objTransactions != NULL) {
        ObjectTransactionInfo *trans = objTransactions->value(instId);
        if (trans == NULL) {
            // see if there is an ALL_INSTANCES transaction
            trans = objTransactions->value(UAVTalk::ALL_INSTANCES);
        }
        return trans;
    }
    return NULL;
}

void Telemetry::openTransaction(ObjectTransactionInfo *trans)
{
    quint32 objId  = trans->obj->getObjID();
    quint16 instId = trans->allInstances ? UAVTalk::ALL_INSTANCES : trans->obj->getInstID();

    QMap<quint32, ObjectTransactionInfo *> *objTransactions = transMap.value(objId);
    if (objTransactions == NULL) {
        objTransactions = new QMap<quint32, ObjectTransactionInfo *>();
        transMap.insert(objId, objTransactions);
    }
    objTransactions->insert(instId, trans);
}

void Telemetry::closeTransaction(ObjectTransactionInfo *trans)
{
    quint32 objId  = trans->obj->getObjID();
    quint16 instId = trans->allInstances ? UAVTalk::ALL_INSTANCES : trans->obj->getInstID();

    QMap<quint32, ObjectTransactionInfo *> *objTransactions = transMap.value(objId);
    if (objTransactions != NULL) {
        objTransactions->remove(instId);
        // Keep the map even if it is empty
        // There are at most 100 different object IDs...
    }
    delete trans;
}

void Telemetry::closeAllTransactions()
{
    foreach(quint32 objId, transMap.keys()) {
        QMap<quint32, ObjectTransactionInfo *> *objTransactions = transMap.value(objId);
        foreach(quint32 instId, objTransactions->keys()) {
            ObjectTransactionInfo *trans = objTransactions->value(instId);

            qWarning() << "Telemetry - closing active transaction for object" << trans->obj->toStringBrief();
            objTransactions->remove(instId);
            delete trans;
        }
        transMap.remove(objId);
        delete objTransactions;
    }
}

ObjectTransactionInfo::ObjectTransactionInfo(QObject *parent) : QObject(parent)
{
    obj = 0;
    allInstances     = false;
    objRequest       = false;
    retriesRemaining = 0;
    acked = false;
    telem = 0;
    // Setup transaction timer
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

ObjectTransactionInfo::~ObjectTransactionInfo()
{
    telem = 0;
    timer->stop();
    delete timer;
}

void ObjectTransactionInfo::timeout()
{
    if (!telem.isNull()) {
        telem->transactionTimeout(this);
    }
}
