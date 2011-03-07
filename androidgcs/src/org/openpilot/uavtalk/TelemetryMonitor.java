package org.openpilot.uavtalk;

import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
import java.util.Observable;
import java.util.Observer;
import java.util.Timer;
import java.util.TimerTask;

import org.openpilot.uavtalk.uavobjects.FlightTelemetryStats;
import org.openpilot.uavtalk.uavobjects.GCSTelemetryStats;

import android.util.Log;

public class TelemetryMonitor {

	private static final String TAG = "TelemetryMonitor";
	
    static final int STATS_UPDATE_PERIOD_MS = 4000;
    static final int STATS_CONNECT_PERIOD_MS = 1000;
    static final int CONNECTION_TIMEOUT_MS = 8000;

	private UAVObjectManager objMngr;
	private Telemetry tel;
	private UAVObject objPending;
	private UAVObject gcsStatsObj;
	private UAVObject flightStatsObj;
	private Timer periodicTask;
	private int currentPeriod;
	private List<UAVObject> queue;
	
	public TelemetryMonitor(UAVObjectManager objMngr, Telemetry tel)
	{
	    this.objMngr = objMngr;
	    this.tel = tel;
	    this.objPending = null;
	    queue = new ArrayList<UAVObject>();

	    // Get stats objects
	    gcsStatsObj = objMngr.getObject("GCSTelemetryStats");
	    flightStatsObj = objMngr.getObject("FlightTelemetryStats");

	    flightStatsObj.addUpdatedObserver(new Observer() {
			public void update(Observable observable, Object data) {
				flightStatsUpdated((UAVObject) data);
			}	    	
	    });

	    // Start update timer
	    setPeriod(STATS_CONNECT_PERIOD_MS);
	}

	/**
	 * Initiate object retrieval, initialize queue with objects to be retrieved.
	 */
	public void startRetrievingObjects()
	{
	    // Clear object queue
	    queue.clear();
	    // Get all objects, add metaobjects, settings and data objects with OnChange update mode to the queue
	    List< List<UAVObject> > objs = objMngr.getObjects();
	    
	    ListIterator<List<UAVObject>> objListIterator = objs.listIterator();
	    while( objListIterator.hasNext() )
	    {
	    	List <UAVObject> instList = objListIterator.next();
	        UAVObject obj = instList.get(0);
	        UAVObject.Metadata mdata = obj.getMetadata();
	        if ( mdata.gcsTelemetryUpdateMode != UAVObject.UpdateMode.UPDATEMODE_NEVER )
	        {
	            if ( obj.isMetadata() )
	            {
	                queue.add(obj);
	            }
	            else /* Data object */
	            {
	            	UAVDataObject dobj = (UAVDataObject) obj;
	                if ( dobj.isSettings() )
	                {
	                    queue.add(obj);
	                }
	                else
	                {
	                    if ( mdata.flightTelemetryUpdateMode == UAVObject.UpdateMode.UPDATEMODE_ONCHANGE )
	                    {
	                        queue.add(obj);
	                    }
	                }
	            }
	        }
	    }
	    // Start retrieving
	    Log.d(TAG,"Starting to retrieve meta and settings objects from the autopilot (%1 objects)" + queue.size()) ;
	    retrieveNextObject();
	}

	/**
	 * Cancel the object retrieval
	 */
	public void stopRetrievingObjects()
	{
	    //qxtLog->debug("Object retrieval has been cancelled");
	    queue.clear();
	}

	/**
	 * Retrieve the next object in the queue
	 */
	public void retrieveNextObject()
	{
	    // If queue is empty return
	    if ( queue.isEmpty() )
	    {
	        //qxtLog->debug("Object retrieval completed");
	        //emit connected();
	        return;
	    }
	    // Get next object from the queue
	    UAVObject obj = queue.remove(0);
	    
	    Log.d(TAG, "Retrieving object: " + obj.getName()) ;
	    // Connect to object
	    //connect(obj, SIGNAL(transactionCompleted(UAVObject*,bool)), this, SLOT(transactionCompleted(UAVObject*,bool)));
	    // Request update
	    obj.requestUpdate();
	    objPending = obj;
	}

	/**
	 * Called by the retrieved object when a transaction is completed.
	 */
	public void transactionCompleted(UAVObject obj, boolean success)
	{
	    //QMutexLocker locker(mutex);
	    // Disconnect from sending object
	    //obj->disconnect(this);
	    objPending = null;
	    
	    // Process next object if telemetry is still available
	    if ( ((String) gcsStatsObj.getField("Status").getValue()).compareTo("Connected") == 0 )
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
	public synchronized void flightStatsUpdated(UAVObject obj)
	{
	    // Force update if not yet connected
	    if ( ((String) gcsStatsObj.getField("Status").getValue()).compareTo("Connected") != 0 ||
	    		((String) flightStatsObj.getField("Status").getValue()).compareTo("Connected") == 0 )
	    {
	        processStatsUpdates();
	    }
	}

	/**
	 * Called periodically to update the statistics and connection status.
	 */
	public synchronized void processStatsUpdates()
	{
	    // Get telemetry stats
	    Telemetry.TelemetryStats telStats = tel.getStats();
	    tel.resetStats();

	    // Update stats object 
	    gcsStatsObj.getField("RxDataRate").setDouble( (float)telStats.rxBytes / ((float)currentPeriod/1000.0) );
	    gcsStatsObj.getField("TxDataRate").setDouble( (float)telStats.txBytes / ((float)currentPeriod/1000.0) );
	    UAVObjectField field = gcsStatsObj.getField("RxFailures");
	    field.setDouble(field.getDouble() + telStats.rxErrors);
	    field = gcsStatsObj.getField("TxFailures");
	    field.setDouble(field.getDouble() + telStats.txErrors);
	    field = gcsStatsObj.getField("TxRetries");
	    field.setDouble(field.getDouble() + telStats.txRetries);

	    // Check for a connection timeout
	    boolean connectionTimeout;
	    if ( telStats.rxObjects > 0 )
	    {
	        //connectionTimer.start();
	    }
	    if ( connectionTimer.elapsed() > CONNECTION_TIMEOUT_MS  )
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
	    	setPeriod(STATS_UPDATE_PERIOD_MS);
	        statsTimer->setInterval(STATS_UPDATE_PERIOD_MS);
	        Log.d(TAG,"Connection with the autopilot established");
	        startRetrievingObjects();
	    }
	    if (gcsStats.Status == GCSTelemetryStats::STATUS_DISCONNECTED && gcsStats.Status != oldStatus)
	    {
	    	setPeriod(STATS_CONNECT_PERIOD_MS);
	        Log.d(TAG,"Connection with the autopilot lost");
	        Log.d(TAG,"Trying to connect to the autopilot");
	        emit disconnected();
	    }
	}

	private void setPeriod(int ms) {
		if(periodicTask == null)
		    periodicTask = new Timer();

		periodicTask.cancel();
		currentPeriod = ms;
	    periodicTask.scheduleAtFixedRate(new TimerTask() {
			@Override
			public void run() {
				processStatsUpdates();				
			}	    	
	    }, currentPeriod, currentPeriod);
	}

}
