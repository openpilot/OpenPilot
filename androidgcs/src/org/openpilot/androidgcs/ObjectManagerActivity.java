package org.openpilot.androidgcs;

import org.openpilot.androidgcs.OPTelemetryService.LocalBinder;
import org.openpilot.androidgcs.OPTelemetryService.TelemTask;
import org.openpilot.uavtalk.UAVObjectManager;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

public abstract class ObjectManagerActivity extends Activity {

	private final String TAG = "ObjectManagerActivity";
	private static int LOGLEVEL = 2;
//	private static boolean WARN = LOGLEVEL > 1;
	private static boolean DEBUG = LOGLEVEL > 0;

	UAVObjectManager objMngr;
    boolean mBound = false;
    boolean mConnected = false;
    LocalBinder binder;
    

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		BroadcastReceiver connectedReceiver = new BroadcastReceiver() {
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
	
	void onOPConnected() {
		
	}
	
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

	/** Defines callbacks for service binding, passed to bindService() */
	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName arg0, IBinder service) {
			// We've bound to LocalService, cast the IBinder and attempt to open a connection			
			if (DEBUG) Log.d(TAG,"Service bound");
			binder = (LocalBinder) service;			
		}

		public void onServiceDisconnected(ComponentName name) {
			mBound = false;
			mConnected = false;
			objMngr = null;
		}
	};
}
