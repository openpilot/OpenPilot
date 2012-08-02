package org.openpilot.uavtalk;

import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
import java.util.Observable;
import java.util.Observer;
import java.util.Timer;
import java.util.TimerTask;

import android.util.Log;

public class TelemetryMonitor extends Observable{

	private static final String TAG = "TelemetryMonitor";
	public static int LOGLEVEL = 0;
	public static boolean WARN = LOGLEVEL > 1;
	public static boolean DEBUG = LOGLEVEL > 0;

    static final int STATS_UPDATE_PERIOD_MS = 4000;
    static final int STATS_CONNECT_PERIOD_MS = 1000;
    static final int CONNECTION_TIMEOUT_MS = 8000;

	private UAVObjectManager objMngr;
	private Telemetry tel;
//	private UAVObject objPending;
	private UAVObject gcsStatsObj;
	private UAVObject flightStatsObj;
	private Timer periodicTask;
	private int currentPeriod;
	private long lastUpdateTime;
	private List<UAVObject> queue;
	
	private boolean connected = false;
	private boolean objects_updated = false;
	
	public boolean getConnected() { return connected; };
	public boolean getObjectsUpdated() { return objects_updated; };
	
	public TelemetryMonitor(UAVObjectManager objMngr, Telemetry tel)
	{
	    this.objMngr = objMngr;
	    this.tel = tel;
//	    this.objPending = null;
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
	public synchronized void startRetrievingObjects()
	{
		if (DEBUG) Log.d(TAG, "Start retrieving objects");
		
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
	        		if ( mdata.GetFlightTelemetryUpdateMode() == UAVObject.UpdateMode.UPDATEMODE_ONCHANGE )
	        		{
	        			queue.add(obj);
	        		}
	        	}
	        }
	    }
	    // Start retrieving
	    System.out.println(TAG + "Starting to retrieve meta and settings objects from the autopilot (" + queue.size() + " objects)");
	    retrieveNextObject();
	}

	/**
	 * Cancel the object retrieval
	 */
	public void stopRetrievingObjects()
	{
		Log.d(TAG, "Stop retrieving objects");
	    queue.clear();
	}

	/**
	 * Retrieve the next object in the queue
	 */
	public synchronized void retrieveNextObject()
	{
	    // If queue is empty return
	    if ( queue.isEmpty() )
	    {
	    	if (DEBUG) Log.d(TAG, "All objects retrieved: Connected Successfully");
	    	objects_updated = true;
	    	setChanged();
	    	notifyObservers();
	        return;
	    }
	    // Get next object from the queue
	    UAVObject obj = queue.remove(0);
	    
	    if(obj == null) {
	    	throw new Error("Got null object forom transaction queue");
	    }
	    	    
	    if (DEBUG) Log.d(TAG, "Retrieving object: " + obj.getName()) ;
	    // Connect to object
	    obj.addTransactionCompleted(new Observer() {
			public void update(Observable observable, Object data) {
				UAVObject.TransactionResult result = (UAVObject.TransactionResult) data;
				if (DEBUG) Log.d(TAG,"Got transaction completed event from " + result.obj.getName() + " status: " + result.success);
				transactionCompleted(result.obj, result.success);
			}	    	
	    });

	    // Request update
	    tel.updateRequested(obj);
//	    objPending = obj;
	}

	/**
	 * Called by the retrieved object when a transaction is completed.
	 */
	public synchronized void transactionCompleted(UAVObject obj, boolean success)
	{
	    //QMutexLocker locker(mutex);
	    // Disconnect from sending object
		if (DEBUG) Log.d(TAG,"transactionCompleted.  Status: " + success);
		// TODO: Need to be able to disconnect signals
	    //obj->disconnect(this);
//	    objPending = null;
	    
	    if(!success) {
	    	Log.e(TAG, "Transaction failed: " + obj.getName() + " sending again.");
	    	return;
	    }
	    
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
	    gcsStatsObj = objMngr.getObject("GCSTelemetryStats");
	    flightStatsObj = objMngr.getObject("FlightTelemetryStats");
	    if (DEBUG) Log.d(TAG,"GCS Status: " + gcsStatsObj.getField("Status").getValue());
	    if (DEBUG) Log.d(TAG,"Flight Status: " + flightStatsObj.getField("Status").getValue());
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
		if (DEBUG) Log.d(TAG, "processStatsUpdates()");
	    Telemetry.TelemetryStats telStats = tel.getStats();
	    tel.resetStats();

	    if (DEBUG) Log.d(TAG, "processStatsUpdates() - stats reset");
	    
	    // Update stats object 
	    gcsStatsObj.getField("RxDataRate").setDouble( (float)telStats.rxBytes / ((float)currentPeriod/1000.0) );
	    gcsStatsObj.getField("TxDataRate").setDouble( (float)telStats.txBytes / ((float)currentPeriod/1000.0) );
	    UAVObjectField field = gcsStatsObj.getField("RxFailures");
	    field.setDouble(field.getDouble() + telStats.rxErrors);
	    field = gcsStatsObj.getField("TxFailures");
	    field.setDouble(field.getDouble() + telStats.txErrors);
	    field = gcsStatsObj.getField("TxRetries");
	    field.setDouble(field.getDouble() + telStats.txRetries);

	    if (DEBUG) Log.d(TAG, "processStatsUpdates() - stats updated");
	    
	    // Check for a connection timeout
	    boolean connectionTimeout;
	    if ( telStats.rxObjects > 0 )
	    {
	    	lastUpdateTime = System.currentTimeMillis();

	    }
	    if ( (System.currentTimeMillis() - lastUpdateTime) > CONNECTION_TIMEOUT_MS  )
	    {
	        connectionTimeout = true;
	    }
	    else
	    {
	        connectionTimeout = false;
	    }

	    // Update connection state
	    gcsStatsObj = objMngr.getObject("GCSTelemetryStats");
	    flightStatsObj = objMngr.getObject("FlightTelemetryStats");
	    if(gcsStatsObj == null) {
	    	System.out.println("No GCS stats yet");
	    	return;
	    }
	    UAVObjectField statusField = gcsStatsObj.getField("Status");
	    String oldStatus = new String((String) statusField.getValue());
	    
	    if (DEBUG) Log.d(TAG,"GCS: " + statusField.getValue() + " Flight: " + flightStatsObj.getField("Status").getValue());

	    if ( oldStatus.compareTo("Disconnected") == 0 )
	    {
	        // Request connection
	    	statusField.setValue("HandshakeReq");
	    }
	    else if ( oldStatus.compareTo("HandshakeReq") == 0 )
	    {
	        // Check for connection acknowledge
	        if ( ((String) flightStatsObj.getField("Status").getValue()).compareTo("HandshakeAck") == 0 )
	        {
	        	statusField.setValue("Connected");
	        	if (DEBUG) Log.d(TAG,"Connected" + statusField.toString());
	        }
	    }
	    else if ( oldStatus.compareTo("Connected") == 0 )
	    {
	        // Check if the connection is still active and the the autopilot is still connected
	        if ( ((String) flightStatsObj.getField("Status").getValue()).compareTo("Disconnected") == 0 || connectionTimeout)
	        {
	        	statusField.setValue("Disconnected");
	        }
	    }

	    // Force telemetry update if not yet connected
	    boolean gcsStatusChanged = !oldStatus.equals(statusField.getValue());
	    
	    boolean gcsConnected = statusField.getValue().equals("Connected");
	    boolean gcsDisconnected = statusField.getValue().equals("Disconnected");
	    boolean flightConnected = flightStatsObj.getField("Status").equals("Connected");
	    
	    if (  !gcsConnected || !flightConnected )
	    {
	    	if (DEBUG) Log.d(TAG,"Sending gcs status");
	        gcsStatsObj.updated();
	    }

	    // Act on new connections or disconnections
	    if (gcsConnected && gcsStatusChanged)
	    {
	    	if (DEBUG) Log.d(TAG,"Connection with the autopilot established");
	    	setPeriod(STATS_UPDATE_PERIOD_MS);
	    	connected = true;
	    	objects_updated = false;
	        startRetrievingObjects();
	        setChanged();
	    }
	    if (gcsDisconnected && gcsStatusChanged)
	    {
	    	if (DEBUG) Log.d(TAG,"Trying to connect to the autopilot");
	    	setPeriod(STATS_CONNECT_PERIOD_MS);
	    	connected = false;
	    	objects_updated = false;
	        setChanged();
	    }
	    
	    if (DEBUG) Log.d(TAG, "processStatsUpdates() - before notify");
        notifyObservers();
        if (DEBUG) Log.d(TAG, "processStatsUpdates() - after notify");
	}

	private void setPeriod(int ms) {
		if(periodicTask == null)
		    periodicTask = new Timer();

		periodicTask.cancel();
		currentPeriod = ms;
		periodicTask = new Timer();
	    periodicTask.scheduleAtFixedRate(new TimerTask() {
			@Override
			public void run() {
				processStatsUpdates();				
			}	    	
	    }, currentPeriod, currentPeriod);
	}

}
