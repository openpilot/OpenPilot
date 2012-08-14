/**
 ******************************************************************************
 *
 * @file       telemetrymonitor.cpp
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

#include "telemetrymonitor.h"
#include "qxtlogger.h"
#include "coreplugin/connectionmanager.h"
#include "coreplugin/icore.h"

/**
 * Constructor
 */
TelemetryMonitor::TelemetryMonitor(UAVObjectManager* objMngr, Telemetry* tel)
{
    this->objMngr = objMngr;
    this->tel = tel;
    this->objPending = NULL;
    this->connectionTimer = new QTime();

    // Create mutex
    mutex = new QMutex(QMutex::Recursive);

    // Get stats objects
    gcsStatsObj = GCSTelemetryStats::GetInstance(objMngr);
    flightStatsObj = FlightTelemetryStats::GetInstance(objMngr);

    // Listen for flight stats updates
    connect(flightStatsObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(flightStatsUpdated(UAVObject*)));

    // Start update timer
    statsTimer = new QTimer(this);
    connect(statsTimer, SIGNAL(timeout()), this, SLOT(processStatsUpdates()));
    statsTimer->start(STATS_CONNECT_PERIOD_MS);

    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    connect(this,SIGNAL(connected()),cm,SLOT(telemetryConnected()));
    connect(this,SIGNAL(disconnected()),cm,SLOT(telemetryDisconnected()));
    connect(this,SIGNAL(telemetryUpdated(double,double)),cm,SLOT(telemetryUpdated(double,double)));
}

TelemetryMonitor::~TelemetryMonitor() {
    // Before saying goodbye, set the GCS connection status to disconnected too:
    GCSTelemetryStats::DataFields gcsStats = gcsStatsObj->getData();
    gcsStats.Status = GCSTelemetryStats::STATUS_DISCONNECTED;
    // Set data
    gcsStatsObj->setData(gcsStats);
}

/**
 * Initiate object retrieval, initialize queue with objects to be retrieved.
 */
void TelemetryMonitor::startRetrievingObjects()
{
    // Clear object queue
    queue.clear();
    // Get all objects, add metaobjects, settings and data objects with OnChange update mode to the queue
    QList< QList<UAVObject*> > objs = objMngr->getObjects();
    for (int n = 0; n < objs.length(); ++n)
    {
        UAVObject* obj = objs[n][0];
        UAVMetaObject* mobj = dynamic_cast<UAVMetaObject*>(obj);
        UAVDataObject* dobj = dynamic_cast<UAVDataObject*>(obj);
        UAVObject::Metadata mdata = obj->getMetadata();
        if ( mobj != NULL )
        {
            queue.enqueue(obj);
        }
        else if ( dobj != NULL )
        {
            if ( dobj->isSettings() )
            {
                queue.enqueue(obj);
            }
            else
            {
                if ( UAVObject::GetFlightTelemetryUpdateMode(mdata) == UAVObject::UPDATEMODE_ONCHANGE )
                {
                    queue.enqueue(obj);
                }
            }
        }
    }
    // Start retrieving
    qxtLog->debug(tr("Starting to retrieve meta and settings objects from the autopilot (%1 objects)")
                  .arg( queue.length()) );
    retrieveNextObject();
}

/**
 * Cancel the object retrieval
 */
void TelemetryMonitor::stopRetrievingObjects()
{
    qxtLog->debug("Object retrieval has been cancelled");
    queue.clear();
}

/**
 * Retrieve the next object in the queue
 */
void TelemetryMonitor::retrieveNextObject()
{
    // If queue is empty return
    if ( queue.isEmpty() )
    {
        qxtLog->debug("Object retrieval completed");
        emit connected();
        return;
    }
    // Get next object from the queue
    UAVObject* obj = queue.dequeue();
    //qxtLog->trace( tr("Retrieving object: %1").arg(obj->getName()) );
    // Connect to object
    connect(obj, SIGNAL(transactionCompleted(UAVObject*,bool)), this, SLOT(transactionCompleted(UAVObject*,bool)));
    // Request update
    obj->requestUpdate();
    objPending = obj;
}

/**
 * Called by the retrieved object when a transaction is completed.
 */
void TelemetryMonitor::transactionCompleted(UAVObject* obj, bool success)
{
    Q_UNUSED(success);
    QMutexLocker locker(mutex);
    // Disconnect from sending object
    obj->disconnect(this);
    objPending = NULL;
    // Process next object if telemetry is still available
    GCSTelemetryStats::DataFields gcsStats = gcsStatsObj->getData();
    if ( gcsStats.Status == GCSTelemetryStats::STATUS_CONNECTED )
    {
        retrieveNextObject();
    }
    else
    {
        stopRetrievingObjects();
    }
}

/**
 * Called each time the flight stats object is updated by the autopilot
 */
void TelemetryMonitor::flightStatsUpdated(UAVObject* obj)
{
    Q_UNUSED(obj);
    QMutexLocker locker(mutex);

    // Force update if not yet connected
    GCSTelemetryStats::DataFields gcsStats = gcsStatsObj->getData();
    FlightTelemetryStats::DataFields flightStats = flightStatsObj->getData();
    if ( gcsStats.Status != GCSTelemetryStats::STATUS_CONNECTED ||
         flightStats.Status != FlightTelemetryStats::STATUS_CONNECTED )
    {
        processStatsUpdates();
    }
}

/**
 * Called periodically to update the statistics and connection status.
 */
void TelemetryMonitor::processStatsUpdates()
{
    QMutexLocker locker(mutex);

    // Get telemetry stats
    GCSTelemetryStats::DataFields gcsStats = gcsStatsObj->getData();
    FlightTelemetryStats::DataFields flightStats = flightStatsObj->getData();
    Telemetry::TelemetryStats telStats = tel->getStats();
    tel->resetStats();

    // Update stats object 
    gcsStats.RxDataRate = (float)telStats.rxBytes / ((float)statsTimer->interval()/1000.0);
    gcsStats.TxDataRate = (float)telStats.txBytes / ((float)statsTimer->interval()/1000.0);
    gcsStats.RxFailures += telStats.rxErrors;
    gcsStats.TxFailures += telStats.txErrors;
    gcsStats.TxRetries += telStats.txRetries;

    // Check for a connection timeout
    bool connectionTimeout;
    if ( telStats.rxObjects > 0 )
    {
        connectionTimer->start();
    }
    if ( connectionTimer->elapsed() > CONNECTION_TIMEOUT_MS  )
    {
        connectionTimeout = true;
    }
    else
    {
        connectionTimeout = false;
    }

    // Update connection state
    int oldStatus = gcsStats.Status;
    if ( gcsStats.Status == GCSTelemetryStats::STATUS_DISCONNECTED )
    {
        // Request connection
        gcsStats.Status = GCSTelemetryStats::STATUS_HANDSHAKEREQ;
    }
    else if ( gcsStats.Status == GCSTelemetryStats::STATUS_HANDSHAKEREQ )
    {
        // Check for connection acknowledge
        if ( flightStats.Status == FlightTelemetryStats::STATUS_HANDSHAKEACK )
        {
            gcsStats.Status = GCSTelemetryStats::STATUS_CONNECTED;
        }
    }
    else if ( gcsStats.Status == GCSTelemetryStats::STATUS_CONNECTED )
    {
        // Check if the connection is still active and the the autopilot is still connected
        if (flightStats.Status == FlightTelemetryStats::STATUS_DISCONNECTED || connectionTimeout)
        {
            gcsStats.Status = GCSTelemetryStats::STATUS_DISCONNECTED;
        }
    }

    emit telemetryUpdated((double)gcsStats.TxDataRate, (double)gcsStats.RxDataRate);

    // Set data
    gcsStatsObj->setData(gcsStats);

    // Force telemetry update if not yet connected
    if ( gcsStats.Status != GCSTelemetryStats::STATUS_CONNECTED ||
         flightStats.Status != FlightTelemetryStats::STATUS_CONNECTED )
    {
        gcsStatsObj->updated();
    }

    // Act on new connections or disconnections
    if (gcsStats.Status == GCSTelemetryStats::STATUS_CONNECTED && gcsStats.Status != oldStatus)
    {
        statsTimer->setInterval(STATS_UPDATE_PERIOD_MS);
        qxtLog->info("Connection with the autopilot established");
        startRetrievingObjects();
    }
    if (gcsStats.Status == GCSTelemetryStats::STATUS_DISCONNECTED && gcsStats.Status != oldStatus)
    {
        statsTimer->setInterval(STATS_CONNECT_PERIOD_MS);
        qxtLog->info("Connection with the autopilot lost");
        qxtLog->info("Trying to connect to the autopilot");
        emit disconnected();
    }
}

