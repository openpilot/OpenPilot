package org.openpilot.androidgcs;

import java.util.List;

import org.openpilot.uavtalk.UAVObject;

import android.os.Bundle;
import android.util.Log;


public class Logger extends ObjectManagerActivity {
	
	final String TAG = "Logger";

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.logger);
	}
	
	@Override
	void onOPConnected() {
		Log.d(TAG, "onOPConnected()");
		
		List<List<UAVObject>> allObjects = objMngr.getObjects();
		registerObjectUpdates(allObjects);
	}		
	
	/**
	 * Called whenever any objects subscribed to via registerObjects 
	 */
	@Override
	protected void objectUpdated(UAVObject obj) {
		Log.d(TAG,"Updated: " + obj.toString());
	}
}
