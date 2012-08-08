/**
 ******************************************************************************
 * @file       ObjectManagerActivity.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Base object for all activies that use the UAVObjectManager.
 *             This class takes care of binding to the service and getting the
 *             object manager as well as setting up callbacks to the objects of
 *             interest that run on the UI thread.
 *             Implements a new Android lifecycle: onOPConnected() / onOPDisconnected()
 *             which indicates when a valid telemetry is established as well as a
 *             valid object manager handle.
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
package org.openpilot.androidgcs;

import java.util.List;
import java.util.ListIterator;
import java.util.Observable;
import java.util.Observer;

import org.openpilot.androidgcs.telemetry.OPTelemetryService;
import org.openpilot.androidgcs.telemetry.OPTelemetryService.LocalBinder;
import org.openpilot.androidgcs.telemetry.OPTelemetryService.TelemTask;
import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectManager;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.TextView;

public abstract class ObjectManagerActivity extends Activity {

	private final String TAG = "ObjectManagerActivity";
	private static int LOGLEVEL = 1;
//	private static boolean WARN = LOGLEVEL > 1;
	private static boolean DEBUG = LOGLEVEL > 0;

	//! Object manager, populated by parent for the children to use
	UAVObjectManager objMngr;
	//! Indicates if the activity is bound to the service
	boolean mBound = false;
	//! Indicates if telemetry is connected
	boolean mConnected = false;
	//! The binder to access the telemetry task, and thus the object manager
	LocalBinder binder;
	//! Store the broadcast receiver to unregister it
	BroadcastReceiver connectedReceiver;
	//! Indicate if this activity has already connected it's telemetry callbacks
	private boolean telemetryStatsConnected = false;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		connectedReceiver = new BroadcastReceiver() {
			@Override
			public void onReceive(Context context, Intent intent) {
				Log.d(TAG, "Received intent");
				TelemTask task;
				if(intent.getAction().compareTo(OPTelemetryService.INTENT_ACTION_CONNECTED) == 0) {
					if(binder  == null)
						return;
					if((task = binder.getTelemTask(0)) == null)
						return;
					objMngr = task.getObjectManager();
					mConnected = true;
					onOPConnected();
					Log.d(TAG, "Connected()");
				} else if (intent.getAction().compareTo(OPTelemetryService.INTENT_ACTION_DISCONNECTED) == 0) {
					objMngr = null;
					mConnected = false;
					onOPDisconnected();
					Log.d(TAG, "Disonnected()");
				}
			}
		};

		IntentFilter filter = new IntentFilter();
		filter.addCategory(OPTelemetryService.INTENT_CATEGORY_GCS);
		filter.addAction(OPTelemetryService.INTENT_ACTION_CONNECTED);
		filter.addAction(OPTelemetryService.INTENT_ACTION_DISCONNECTED);
		registerReceiver(connectedReceiver, filter);
	}

	/**
	 * Called whenever any objects subscribed to via registerObjects
	 */
	protected void objectUpdated(UAVObject obj) {

	}

	/**
	 * A message handler and a custom Observer to use it which calls
	 * objectUpdated with the right object type
	 */
	final Handler uavobjHandler = new Handler();
	private class ActivityUpdatedObserver implements Observer  {
		UAVObject obj;
		ActivityUpdatedObserver(UAVObject obj) { this.obj = obj; };
		@Override
		public void update(Observable observable, Object data) {
			uavobjHandler.post(new Runnable() {
				@Override
				public void run() { objectUpdated(obj); }
			});
		}
	};
	private class FragmentUpdatedObserver implements Observer  {
		UAVObject obj;
		ObjectManagerFragment frag;
		FragmentUpdatedObserver(UAVObject obj, ObjectManagerFragment frag) {
			this.obj = obj;
			this.frag = frag;
		};
		@Override
		public void update(Observable observable, Object data) {
			uavobjHandler.post(new Runnable() {
				@Override
				public void run() { frag.objectUpdated(obj); }
			});
		}
	};


	/**
	 * Register an activity to receive updates from this object
	 *
	 * the objectUpdated() method will be called in the original UI thread
	 */
	protected void registerObjectUpdates(UAVObject object) {
		object.addUpdatedObserver(new ActivityUpdatedObserver(object));
	}
	protected void registerObjectUpdates(UAVObject object, ObjectManagerFragment frag) {
		object.addUpdatedObserver(new FragmentUpdatedObserver(object, frag));
	}
	protected void registerObjectUpdates(List<List<UAVObject>> objects) {
		ListIterator<List<UAVObject>> li = objects.listIterator();
		while(li.hasNext()) {
			ListIterator<UAVObject> li2 = li.next().listIterator();
			while(li2.hasNext())
				registerObjectUpdates(li2.next());
		}
	}

	private void updateStats() {
		UAVObject stats = objMngr.getObject("GCSTelemetryStats");
		TextView rxRate = (TextView) findViewById(R.id.telemetry_stats_rx_rate);
		TextView txRate = (TextView) findViewById(R.id.telemetry_stats_tx_rate);
		if (rxRate != null)
			rxRate.setText(Integer.valueOf((int) stats.getField("RxDataRate").getDouble()).toString());
		if (rxRate != null)
			txRate.setText(Integer.valueOf((int) stats.getField("TxDataRate").getDouble()).toString());

	}

	final Observer telemetryObserver = new Observer() {
		@Override
		public void update(Observable observable, Object data) {
			uavobjHandler.post(new Runnable() {
				@Override
				public void run() {
					updateStats();
				}
			});
		}
	};

	/**
	 * Called when either the telemetry establishes a connection or
	 * if it already has on creation of this activity
	 *
	 * This should be called by all inherited classes if they want the telemetry bar etc
	 */
	void onOPConnected() {
		if ( telemetryStatsConnected )
			return;

		// We are not using the objectUpdated mechanism in place so that all the children
		// don't have to sort through the messages.
		UAVObject stats = objMngr.getObject("GCSTelemetryStats");
		if (stats == null)
			return;

		stats.addUpdatedObserver(telemetryObserver);
		telemetryStatsConnected = true;
		updateStats();

		if (DEBUG) Log.d(TAG, "Notifying listeners about connection.  There are " + connectionListeners.countObservers());
		connectionListeners.connected();
	}

	/**
	 * Called when telemetry drops the connection
	 *
	 * This should be called by all inherited classes if they want the telemetry bar etc
	 */
	void onOPDisconnected() {
		// Indicate no connection
		TextView rxRate = (TextView) findViewById(R.id.telemetry_stats_rx_rate);
		TextView txRate = (TextView) findViewById(R.id.telemetry_stats_tx_rate);
		rxRate.setText("");
		txRate.setText("");

		// Providing a null update triggers a disconnect on fragments
		connectionListeners.disconnected();
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
		case R.id.menu_connect:
			binder.openConnection();
			return true;
		case R.id.menu_disconnect:
			binder.stopConnection();
			return true;
		case R.id.menu_settings:
			startActivity(new Intent(this, Preferences.class));
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.status_menu, menu);
		inflater.inflate(R.menu.options_menu, menu);
		return true;
	}

	@Override
	public void onStart() {
		super.onStart();
		Intent intent = new Intent(this, OPTelemetryService.class);
		bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
	}

	/**
	 * When stopping disconnect form the service and the broadcast receiver
	 */
	@Override
	public void onStop() {
		super.onStop();
		unbindService(mConnection);
		//unregisterReceiver(connectedReceiver);
	}

	public void onBind() {

	}


	/**
	 * Callbacks so ObjectManagerFragments get the onOPConnected and onOPDisconnected signals
	 */
	class ConnectionObserver extends Observable  {
		public void disconnected() {
			synchronized(this) {
				setChanged();
				notifyObservers();
			}
		}
		public void connected() {
			synchronized(this) {
				setChanged();
				notifyObservers(objMngr);
			}
		}
	};
	private final ConnectionObserver connectionListeners = new ConnectionObserver();
	public class OnConnectionListener implements Observer {

		// Local reference of the fragment to notify, store in constructor
		ObjectManagerFragment fragment;
		OnConnectionListener(ObjectManagerFragment fragment) { this.fragment = fragment; };

		// Whenever the observer is updated either conenct or disconnect based on the data
		@Override
		public void update(Observable observable, Object data) {
			Log.d(TAG, "onConnectionListener called");
			if (data == null)
				fragment.onOPDisconnected();
			else
				fragment.onOPConnected(objMngr);
		}

	} ;
	void addOnConnectionListenerFragment(ObjectManagerFragment frag) {
		connectionListeners.addObserver(new OnConnectionListener(frag));
		if (DEBUG) Log.d(TAG, "Connecting " + frag + " there are now " + connectionListeners.countObservers());
		if (mConnected)
			frag.onOPConnected(objMngr);
	}


	/** Defines callbacks for service binding, passed to bindService() */
	private final ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName arg0, IBinder service) {
			// We've bound to LocalService, cast the IBinder and attempt to open a connection
			if (DEBUG) Log.d(TAG,"Service bound");
			mBound = true;
			binder = (LocalBinder) service;

			if(binder.isConnected()) {
				TelemTask task;
				if((task = binder.getTelemTask(0)) != null) {
					objMngr = task.getObjectManager();
					mConnected = true;
					onOPConnected();
				}

			}
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			mBound = false;
			binder = null;
			mConnected = false;
			objMngr = null;
			objMngr = null;
			mConnected = false;
			onOPDisconnected();
		}
	};
}
