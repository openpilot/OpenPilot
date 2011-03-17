package org.openpilot.androidgcs;

import org.openpilot.uavtalk.Telemetry;
import org.openpilot.uavtalk.TelemetryMonitor;
import org.openpilot.uavtalk.UAVObjectManager;
import org.openpilot.uavtalk.UAVTalk;
import org.openpilot.uavtalk.uavobjects.UAVObjectsInitialize;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;

public class OPTelemetryService extends Service {
	private final String TAG = "OPTElemetryService";
	public static int LOGLEVEL = 2;
	public static boolean WARN = LOGLEVEL > 1;
	public static boolean DEBUG = LOGLEVEL > 0;

	private UAVObjectManager objMngr;
	private UAVTalk uavTalk;
	private Telemetry tel;
	private TelemetryMonitor mon;

	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

	@Override
	public void onCreate() {
		super.onCreate();
		
		if (DEBUG) Log.d(TAG, "Telemetry Service started");

		Thread telemetryThread = new Thread(telemetryRunnable);
		telemetryThread.start();
 	}

	@Override
	public void onDestroy() {
		super.onDestroy();
	}
	
	private final Runnable telemetryRunnable = new Runnable() {

		public void run() {			
			Looper.prepare();
			
			if (DEBUG) Log.d(TAG, "Telemetry Thread started");
			
	        objMngr = new UAVObjectManager();
			UAVObjectsInitialize.register(objMngr);

			BluetoothUAVTalk bt = new BluetoothUAVTalk(OPTelemetryService.this, BluetoothUAVTalk.DEVICE_NAME);
			while(true) {
				if (DEBUG) Log.d(TAG, "Attempting Bluetooth Connection");
				if(! bt.getConnected() )
					bt.connect(objMngr);
				else
					break;
				Looper.loop();
			}
			
			if (DEBUG) Log.d(TAG, "Connected via bluetooth");
			
			uavTalk = bt.getUavtalk();
			tel = new Telemetry(uavTalk, objMngr);
			mon = new TelemetryMonitor(objMngr,tel);

			if (DEBUG) Log.d(TAG, "Entering UAVTalk processing loop");
			while(true) {
				if( !uavTalk.processInputStream() )
					break;
				Looper.loop();
			}
			if (DEBUG) Log.d(TAG, "UAVTalk stream disconnected");
		}
	
	};
	
	public UAVObjectManager getObjMngr() { return objMngr; };
	public UAVTalk getUavTalk() { return uavTalk; };
	public Telemetry getTelemetry() { return tel; };
	public TelemetryMonitor getTelemetryMonitor() { return mon; };

}
