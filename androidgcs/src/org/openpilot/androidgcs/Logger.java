package org.openpilot.androidgcs;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.openpilot.uavtalk.UAVObject;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;


public class Logger extends ObjectManagerActivity {
	
	final String TAG = "Logger";

	final boolean VERBOSE = false;
	final boolean DEBUG = true;
	
	private File file;
	private boolean logging;
	private BufferedWriter out;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.logger);
	}
	
	private void onStartLogging() {
		
		File root = Environment.getExternalStorageDirectory();
		
		Date d = new Date();
		String date = (new SimpleDateFormat("yyyyMMdd_hhmmss")).format(d);
		String fileName = "/logs/logs_" + date + ".uav";
		
		file = new File(root, fileName);
		if (DEBUG) Log.d(TAG, "Trying for file: " + file.getAbsolutePath());
		try {
			if (root.canWrite()){
				FileWriter filewriter = new FileWriter(file);
				out = new BufferedWriter(filewriter);
				logging = true;
			} else {
				Log.e(TAG, "Unwriteable address");
			}
		} catch (IOException e) {
			Log.e(TAG, "Could not write file " + e.getMessage());
		}
		
		// TODO: if logging succeeded then retrieve all settings
	}
	
	private void onStopLogging() {
		if (DEBUG) Log.d(TAG, "Stop logging");
		logging = false;
		try {
			out.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	@Override
	void onOPConnected() {
		if (DEBUG) Log.d(TAG, "onOPConnected()");
		onStartLogging();
		registerObjectUpdates(objMngr.getObjects());
	}

	@Override
	void onOPDisconnected() {
		if (DEBUG) Log.d(TAG, "onOPDisconnected()");
		onStopLogging();
	}

	@Override
	public void onPause()
	{
	    super.onPause();
	    onStopLogging();
	}

	@Override
	public void onResume()
	{
	    super.onResume();
	    onStartLogging();
	}
	/**
	 * Called whenever any objects subscribed to via registerObjects 
	 */
	@Override
	protected void objectUpdated(UAVObject obj) {
		if (logging) {
			if (VERBOSE) Log.v(TAG,"Updated: " + obj.toString());
			try {
				out.write(obj + "\n");
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
}
