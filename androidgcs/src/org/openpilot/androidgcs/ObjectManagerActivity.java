package org.openpilot.androidgcs;

import java.util.List;
import java.util.ListIterator;
import java.util.Observable;
import java.util.Observer;

import org.openpilot.androidgcs.OPTelemetryService.LocalBinder;
import org.openpilot.androidgcs.OPTelemetryService.TelemTask;
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

public abstract class ObjectManagerActivity extends Activity {

	private final String TAG = "ObjectManagerActivity";
	private static int LOGLEVEL = 0;
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
	private class UpdatedObserver implements Observer  {
		UAVObject obj;
		UpdatedObserver(UAVObject obj) { this.obj = obj; };
		public void update(Observable observable, Object data) {
			uavobjHandler.post(new Runnable() {
				@Override
				public void run() { objectUpdated(obj); }
			});
		}
	};

	/**
	 * Register an activity to receive updates from this object
	 * 
	 * the objectUpdated() method will be called in the original UI thread
	 */
	protected void registerObjectUpdates(UAVObject object) {
		object.addUpdatedObserver(new UpdatedObserver(object));
	}
	protected void registerObjectUpdates(List<List<UAVObject>> objects) {
		ListIterator<List<UAVObject>> li = objects.listIterator();
		while(li.hasNext()) {
			ListIterator<UAVObject> li2 = li.next().listIterator();
			while(li2.hasNext())
				registerObjectUpdates(li2.next());
		}
	}
	
	/**
	 * Called when either the telemetry establishes a connection or
	 * if it already has on creation of this activity
	 */
	void onOPConnected() {
		
	}
	
	/**
	 * Called when telemetry drops the connection
	 */
	void onOPDisconnected() {
		
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
		inflater.inflate(R.menu.options_menu, menu);
		return true;
	}

	@Override
	public void onStart() {
		super.onStart();
		Intent intent = new Intent(this, OPTelemetryService.class);
		bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
	}
	
	@Override
	public void onStop() {
		super.onStop();
		unbindService(mConnection);
		unregisterReceiver(connectedReceiver);
	}
	public void onBind() {
		
	}

	/** Defines callbacks for service binding, passed to bindService() */
	private ServiceConnection mConnection = new ServiceConnection() {
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
