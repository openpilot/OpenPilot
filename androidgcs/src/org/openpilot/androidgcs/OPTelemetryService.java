package org.openpilot.androidgcs;

import org.openpilot.uavtalk.Telemetry;
import org.openpilot.uavtalk.TelemetryMonitor;
import org.openpilot.uavtalk.UAVObjectManager;
import org.openpilot.uavtalk.UAVTalk;
import org.openpilot.uavtalk.uavobjects.UAVObjectsInitialize;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;

public class OPTelemetryService extends Service {
	private final String TAG = "OPTElemetryService";
	public static int LOGLEVEL = 2;
	public static boolean WARN = LOGLEVEL > 1;
	public static boolean DEBUG = LOGLEVEL > 0;

	final int DISCONNECT_MESSAGE = 0;
	final int CONNECT_MESSAGE = 1;
	final int CONNECT_FAILED_MESSAGE = 2;
	
	private UAVObjectManager objMngr;
	private UAVTalk uavTalk;
	private Telemetry tel;
	private TelemetryMonitor mon;
	
	private Handler handler;

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
			if (DEBUG) Log.d(TAG, "Telemetry Thread started");

			Looper.prepare();						
			
	        objMngr = new UAVObjectManager();
			UAVObjectsInitialize.register(objMngr);
			
			postNotification(CONNECT_FAILED_MESSAGE, "Connecting");
			BluetoothUAVTalk bt = new BluetoothUAVTalk(OPTelemetryService.this, BluetoothUAVTalk.DEVICE_NAME);
			for( int i = 0; i < 10; i++ ) {
				if (DEBUG) Log.d(TAG, "Attempting Bluetooth Connection");
					
				bt.connect(objMngr);
				
				if (DEBUG) Log.d(TAG, "Done attempting connection");
				if( bt.getConnected() )
					break;
				
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					Log.e(TAG, "Thread interrupted while trying to connect");
					e.printStackTrace();
					return;
				}
			}
			if( ! bt.getConnected() ) {
				postNotification(CONNECT_FAILED_MESSAGE, "Could not connect to UAV");
				return;
			}
			
			postNotification(CONNECT_MESSAGE, "Connected to UAV port");
			
			if (DEBUG) Log.d(TAG, "Connected via bluetooth");
			
			uavTalk = bt.getUavtalk();
			tel = new Telemetry(uavTalk, objMngr);
			mon = new TelemetryMonitor(objMngr,tel);

			if (DEBUG) Log.d(TAG, "Entering UAVTalk processing loop");
			while(true) {
				if( !uavTalk.processInputStream() )
					break;
			}
			if (DEBUG) Log.d(TAG, "UAVTalk stream disconnected");
			postNotification(DISCONNECT_MESSAGE,"UAVTalk stream disconnected");
		}
	
	};
	
	void postNotification(int id, String message) {
		String ns = Context.NOTIFICATION_SERVICE;
		NotificationManager mNManager = (NotificationManager) getSystemService(ns);
		final Notification msg = new Notification(R.drawable.icon, message, System.currentTimeMillis());
		
		Context context = getApplicationContext(); 
		CharSequence contentTitle = "OpenPilot";
		CharSequence contentText = message; 
		Intent msgIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("http://forums.openpilot.org"));
		PendingIntent intent = PendingIntent.getActivity(this, 0, msgIntent, Intent.FLAG_ACTIVITY_NEW_TASK);		
		
		msg.setLatestEventInfo(context, contentTitle, contentText, intent);
		
		mNManager.notify(id, msg);
	}
	
	public UAVObjectManager getObjMngr() { return objMngr; };
	public UAVTalk getUavTalk() { return uavTalk; };
	public Telemetry getTelemetry() { return tel; };
	public TelemetryMonitor getTelemetryMonitor() { return mon; };

}
