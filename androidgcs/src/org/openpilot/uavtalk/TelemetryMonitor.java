/**
 ******************************************************************************
 * @file       TelemetryMonitor.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      High level monitoring of telemetry to handle connection and
 *             disconnection and then signal the rest of the application.
 *             This also makes sure to fetch all objects on initial connection.
 * @see        The GNU Public License (GPL) Version 3
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
package org.openpilot.uavtalk;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
import java.util.Observable;
import java.util.Observer;
import java.util.Timer;
import java.util.TimerTask;

import org.openpilot.androidgcs.telemetry.OPTelemetryService;

import android.util.Log;

public class TelemetryMonitor extends Observable {

	private static final String TAG = "TelemetryMonitor";
	public static final int LOGLEVEL = 0;
	public static boolean DEBUG = LOGLEVEL > 2;
	public static final boolean WARN = LOGLEVEL > 1;
	public static final boolean ERROR = LOGLEVEL > 0;

	static final int STATS_UPDATE_PERIOD_MS = 4000;
	static final int STATS_CONNECT_PERIOD_MS = 1000;
	static final int CONNECTION_TIMEOUT_MS = 8000;

	private final boolean HANDSHAKE_IS_CONNECTED = false;

	private final UAVObjectManager objMngr;
	private final Telemetry tel;

	private boolean objectsRegistered;
	// private UAVObject objPending;
	private UAVObject gcsStatsObj;
	private UAVObject flightStatsObj;
	private final UAVObject firmwareIapObj;
	private Timer periodicTask;
	private int currentPeriod;
	private long lastUpdateTime;
	private final List<UAVObject> queue;

	private OPTelemetryService telemService;
	private boolean connected = false;
	private boolean objects_updated = false;

	public boolean getConnected() {
		return connected;
	};

	public boolean getObjectsUpdated() {
		return objects_updated;
	};

	public TelemetryMonitor(UAVObjectManager objMngr, Telemetry tel,
			OPTelemetryService s) {
		this(objMngr, tel);
		telemService = s;
	}

	public TelemetryMonitor(UAVObjectManager objMngr, Telemetry tel) {
		this.objMngr = objMngr;
		this.tel = tel;
		// this.objPending = null;
		queue = new ArrayList<UAVObject>();

		objectsRegistered = false;

		// Get stats objects
		gcsStatsObj = objMngr.getObject("GCSTelemetryStats");
		flightStatsObj = objMngr.getObject("FlightTelemetryStats");
		firmwareIapObj = objMngr.getObject("FirmwareIAPObj");

		// The first update of the firmwareIapObj will trigger registering the objects
		firmwareIapObj.addUpdatedObserver(firmwareIapUpdated);

		flightStatsObj.addUpdatedObserver(new Observer() {
			@Override
			public void update(Observable observable, Object data) {
				try {
					flightStatsUpdated((UAVObject) data);
				} catch (IOException e) {
					// The UAVTalk stream was broken, disconnect this signal
					// TODO: Should this actually be disconnected. Do we create
					// a new TelemetryMonitor for this
					// or fix the stream?
					flightStatsObj.removeUpdatedObserver(this);
				}
			}
		});

		// Start update timer
		setPeriod(STATS_CONNECT_PERIOD_MS);
	}

	/**
	 * Initiate object retrieval, initialize queue with objects to be retrieved.
	 *
	 * @throws IOException
	 */
	public synchronized void startRetrievingObjects() throws IOException {
		if (DEBUG)
			Log.d(TAG, "Start retrieving objects");

		// Clear object queue
		queue.clear();
		// Get all objects, add metaobjects, settings and data objects with
		// OnChange update mode to the queue
		List<List<UAVObject>> objs = objMngr.getObjects();

		ListIterator<List<UAVObject>> objListIterator = objs.listIterator();
		while (objListIterator.hasNext()) {
			List<UAVObject> instList = objListIterator.next();
			UAVObject obj = instList.get(0);
			UAVObject.Metadata mdata = obj.getMetadata();
			if (obj.isMetadata()) {
				queue.add(obj);
			} else /* Data object */
			{
				UAVDataObject dobj = (UAVDataObject) obj;
				if (dobj.isSettings()) {
					queue.add(obj);
				} else {
					if (mdata.GetFlightTelemetryUpdateMode() == UAVObject.UpdateMode.UPDATEMODE_ONCHANGE) {
						queue.add(obj);
					}
				}
			}
		}
		// Start retrieving
		Log.d(TAG,
				"Starting to retrieve meta and settings objects from the autopilot ("
						+ queue.size() + " objects)");
		retrieveNextObject();
	}

	/**
	 * Cancel the object retrieval
	 */
	public void stopRetrievingObjects() {
		Log.d(TAG, "Stop retrieving objects");
		queue.clear();
	}

	final Observer transactionObserver = new Observer() {
		@Override
		public void update(Observable observable, Object data) {
			try {
				UAVObject.TransactionResult result = (UAVObject.TransactionResult) data;
				transactionCompleted(result.obj, result.success);
			} catch (IOException e) {
				// When the telemetry stream is broken disconnect these
				// updates
				observable.deleteObserver(this);
			}
		}
	};

	/**
	 * Retrieve the next object in the queue
	 *
	 * @throws IOException
	 */
	public synchronized void retrieveNextObject() throws IOException {
		// If queue is empty return
		if (queue.isEmpty()) {
			if (telemService != null)
				telemService.toastMessage("Connected");
			if (DEBUG) Log.d(TAG, "All objects retrieved: Connected Successfully");
			objects_updated = true;
			if (!HANDSHAKE_IS_CONNECTED) {
				setChanged();
				notifyObservers();
			}
			return;
		}
		// Get next object from the queue
		UAVObject obj = queue.remove(0);

		if (obj == null) {
			throw new Error("Got null object forom transaction queue");
		}

		if (DEBUG)
			Log.d(TAG, "Retrieving object: " + obj.getName());

		// TODO: Does this need to stay here permanently? This appears to be
		// used for setup mainly
		obj.addTransactionCompleted(transactionObserver);

		// Request update
		obj.updateRequested();
	}

	/**
	 * Called by the retrieved object when a transaction is completed.
	 *
	 * @throws IOException
	 */
	public synchronized void transactionCompleted(UAVObject obj, boolean success)
			throws IOException {
		if (DEBUG)
			Log.d(TAG, "transactionCompleted.  Status: " + success);

		// Remove the listener for the event that just finished
		obj.removeTransactionCompleted(transactionObserver);

		if (!success) {
			// Right now success = false means received a NAK so don't
			// re-attempt
			if (ERROR) Log.e(TAG, "Transaction failed.");
		}

		// Process next object if telemetry is still available
		if (((String) gcsStatsObj.getField("Status").getValue()).compareTo("Connected") == 0) {
			retrieveNextObject();
		} else {
			stopRetrievingObjects();
		}
	}

	/**
	 * Called each time the flight stats object is updated by the autopilot
	 *
	 * @throws IOException
	 */
	public synchronized void flightStatsUpdated(UAVObject obj)
			throws IOException {
		// Force update if not yet connected
		gcsStatsObj = objMngr.getObject("GCSTelemetryStats");
		flightStatsObj = objMngr.getObject("FlightTelemetryStats");
		if (DEBUG)
			Log.d(TAG, "GCS Status: "
					+ gcsStatsObj.getField("Status").getValue());
		if (DEBUG)
			Log.d(TAG, "Flight Status: "
					+ flightStatsObj.getField("Status").getValue());
		if (((String) gcsStatsObj.getField("Status").getValue())
				.compareTo("Connected") != 0
				|| ((String) flightStatsObj.getField("Status").getValue())
						.compareTo("Connected") == 0) {
			processStatsUpdates();
		}
	}

	private long lastStatsTime;

	/**
	 * Called periodically to update the statistics and connection status.
	 *
	 * @throws IOException
	 */
	public synchronized void processStatsUpdates() throws IOException {
		// Get telemetry stats
		if (DEBUG)
			Log.d(TAG, "processStatsUpdates()");
		Telemetry.TelemetryStats telStats = tel.getStats();

		if (DEBUG)
			Log.d(TAG, "processStatsUpdates() - stats reset");

		// Need to compute time because this update is not regular enough
		float dT = (System.currentTimeMillis() - lastStatsTime) / 1000.0f;
		lastStatsTime = System.currentTimeMillis();

		// Update stats object
		gcsStatsObj.getField("RxDataRate").setDouble(telStats.rxBytes / dT);
		gcsStatsObj.getField("TxDataRate").setDouble(telStats.txBytes / dT);
		UAVObjectField field = gcsStatsObj.getField("RxFailures");
		field.setDouble(field.getDouble() + telStats.rxErrors);
		field = gcsStatsObj.getField("TxFailures");
		field.setDouble(field.getDouble() + telStats.txErrors);
		field = gcsStatsObj.getField("TxRetries");
		field.setDouble(field.getDouble() + telStats.txRetries);

		tel.resetStats();

		if (DEBUG)
			Log.d(TAG, "processStatsUpdates() - stats updated");

		// Check for a connection timeout
		boolean connectionTimeout;
		if (telStats.rxObjects > 0) {
			lastUpdateTime = System.currentTimeMillis();

		}
		if ((System.currentTimeMillis() - lastUpdateTime) > CONNECTION_TIMEOUT_MS) {
			connectionTimeout = true;
		} else {
			connectionTimeout = false;
		}

		// Update connection state
		gcsStatsObj = objMngr.getObject("GCSTelemetryStats");
		flightStatsObj = objMngr.getObject("FlightTelemetryStats");
		if (gcsStatsObj == null) {
			Log.d(TAG, "No GCS stats yet");
			return;
		}
		UAVObjectField statusField = gcsStatsObj.getField("Status");
		String oldStatus = new String((String) statusField.getValue());

		if (DEBUG)
			Log.d(TAG, "GCS: " + statusField.getValue() + " Flight: "
					+ flightStatsObj.getField("Status").getValue());

		if (oldStatus.compareTo("Disconnected") == 0) {
			// Request connection
			statusField.setValue("HandshakeReq");
		} else if (oldStatus.compareTo("HandshakeReq") == 0) {
			// Check for connection acknowledge
			if (((String) flightStatsObj.getField("Status").getValue())
					.compareTo("HandshakeAck") == 0) {
				statusField.setValue("Connected");
				if (DEBUG)
					Log.d(TAG, "Connected" + statusField.toString());
			}
		} else if (oldStatus.compareTo("Connected") == 0) {
			// Check if the connection is still active and the the autopilot is
			// still connected
			if (((String) flightStatsObj.getField("Status").getValue())
					.compareTo("Disconnected") == 0 || connectionTimeout) {
				statusField.setValue("Disconnected");
			}
		}

		// Force telemetry update if not yet connected
		boolean gcsStatusChanged = !oldStatus.equals(statusField.getValue());

		boolean gcsConnected = statusField.getValue().equals("Connected");
		boolean gcsDisconnected = statusField.getValue().equals("Disconnected");
		boolean flightConnected = flightStatsObj.getField("Status").equals(
				"Connected");

		if (!gcsConnected || !flightConnected) {
			if (DEBUG)
				Log.d(TAG, "Sending gcs status");
			gcsStatsObj.updated();
		}

		// Act on new connections or disconnections
		if (gcsConnected && gcsStatusChanged) {
			if (DEBUG)
				Log.d(TAG, "Connection with the autopilot established");
			setPeriod(STATS_UPDATE_PERIOD_MS);
			connected = true;
			objects_updated = false;
			if (objectsRegistered)
				startRetrievingObjects();
			else
				firmwareIapObj.updateRequested();
			if (HANDSHAKE_IS_CONNECTED)
				setChanged(); // Enabling this line makes the opConnected signal
								// occur whenever we get a handshake
		}
		if (gcsDisconnected && gcsStatusChanged) {
			if (DEBUG)
				Log.d(TAG, "Trying to connect to the autopilot");
			setPeriod(STATS_CONNECT_PERIOD_MS);
			connected = false;
			objects_updated = false;
			setChanged();
		}

		if (DEBUG)
			Log.d(TAG, "processStatsUpdates() - before notify");
		notifyObservers();
		if (DEBUG)
			Log.d(TAG, "processStatsUpdates() - after notify");
	}

	private void setPeriod(int ms) {
		if (periodicTask == null)
			periodicTask = new Timer();

		periodicTask.cancel();
		currentPeriod = ms;
		periodicTask = new Timer();
		periodicTask.scheduleAtFixedRate(new TimerTask() {
			@Override
			public void run() {
				try {
					processStatsUpdates();
				} catch (IOException e) {
					// Once the stream has died stop trying to process these
					// updates
					periodicTask.cancel();
				}
			}
		}, currentPeriod, currentPeriod);
	}

	public void stopMonitor() {
		periodicTask.cancel();
		periodicTask = null;
	}

	private final Observer firmwareIapUpdated = new Observer() {
		@Override
		public void update(Observable observable, Object data) {
			if (DEBUG) Log.d(TAG, "Received firmware IAP Updated message");

			UAVObjectField description = firmwareIapObj.getField("Description");
			if (description == null || description.getNumElements() < 100) {
				telemService.toastMessage("Failed to determine UAVO set");
			} else {
				final int HASH_SIZE_USED = 8;
				String jarName = new String();
				for (int i = 0; i < HASH_SIZE_USED; i++) {
					jarName += String.format("%02x", (int) description.getDouble(i + 60));
				}
				jarName += ".jar";
				if (DEBUG) Log.d(TAG, "Attempting to load: " + jarName);
				if (telemService.loadUavobjects(jarName, objMngr)) {
					telemService.toastMessage("Loaded appropriate UAVO set");
					objectsRegistered = true;
					try {
						startRetrievingObjects();
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				} else
					telemService.toastMessage("Failed to load UAVO set: " + jarName);
			}

			firmwareIapObj.removeUpdatedObserver(this);
		}
	};

}
