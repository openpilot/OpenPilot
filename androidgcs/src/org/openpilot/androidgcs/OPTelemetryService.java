package org.openpilot.androidgcs;

import java.util.Observable;
import java.util.Observer;

import org.openpilot.uavtalk.Telemetry;
import org.openpilot.uavtalk.TelemetryMonitor;
import org.openpilot.uavtalk.UAVDataObject;
import org.openpilot.uavtalk.UAVObjectManager;
import org.openpilot.uavtalk.UAVTalk;
import org.openpilot.uavtalk.uavobjects.UAVObjectsInitialize;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.Toast;

public class OPTelemetryService extends Service {

	// Logging settings
	private final String TAG = "OPTelemetryService";
	public static int LOGLEVEL = 2;
	public static boolean WARN = LOGLEVEL > 1;
	public static boolean DEBUG = LOGLEVEL > 0;

	// Intent category
	public final static String INTENT_CATEGORY_GCS        = "org.openpilot.intent.category.GCS";

	// Intent actions
	public final static String INTENT_ACTION_CONNECTED    = "org.openpilot.intent.action.CONNECTED";
	public final static String INTENT_ACTION_DISCONNECTED = "org.openpilot.intent.action.DISCONNECTED";

	// Variables for local message handler thread
	private Looper mServiceLooper;
	private ServiceHandler mServiceHandler;

	// Message ids
	static final int MSG_START        = 0;
	static final int MSG_CONNECT      = 1;
	static final int MSG_DISCONNECT   = 3;
	static final int MSG_TOAST        = 100;

	private boolean terminate = false;

	private Thread activeTelem;

	private final IBinder mBinder = new LocalBinder();

	private final class ServiceHandler extends Handler {
		public ServiceHandler(Looper looper) {
			super(looper);
		}
		@Override
		public void handleMessage(Message msg) {
			switch(msg.arg1) {
			case MSG_START:
				stopSelf(msg.arg2);
				break;
			case MSG_CONNECT:	
				Toast.makeText(getApplicationContext(), "Attempting connection", Toast.LENGTH_SHORT).show();
				terminate = false;
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(OPTelemetryService.this);
				int connection_type = Integer.decode(prefs.getString("connection_type", ""));
				switch(connection_type) {
				case 0: // No connection
					return;
				case 1:
					activeTelem = new FakeTelemetryThread();
					break;
				case 2:
					activeTelem = new BTTelemetryThread();
					break;
				case 3:
					throw new Error("Unsupported");
				}
				activeTelem.start();
				break;
			case MSG_DISCONNECT:
				Toast.makeText(getApplicationContext(), "Disconnct", Toast.LENGTH_SHORT).show();
				terminate = true;
				try {
					activeTelem.join();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				activeTelem = null;

				Intent intent = new Intent();
				intent.setAction(INTENT_ACTION_DISCONNECTED);
				sendBroadcast(intent,null);
				
				stopSelf();

				break;
			case MSG_TOAST:
				Toast.makeText(OPTelemetryService.this, (String) msg.obj, Toast.LENGTH_SHORT);
				break;
			default:
				System.out.println(msg.toString());
				throw new Error("Invalid message");
			}
		}
	};

	public void startup() {
		Toast.makeText(getApplicationContext(), "Telemetry service starting", Toast.LENGTH_SHORT).show();
		
		HandlerThread thread = new HandlerThread("TelemetryServiceHandler",
				Process.THREAD_PRIORITY_BACKGROUND);
		thread.start();

		// Get the HandlerThread's Looper and use it for our Handler 
		mServiceLooper = thread.getLooper();
		mServiceHandler = new ServiceHandler(mServiceLooper);
	
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(OPTelemetryService.this);
		if(prefs.getBoolean("autoconnect", false)) {
			Toast.makeText(getApplicationContext(), "Should auto connect", Toast.LENGTH_SHORT);
			Message msg = mServiceHandler.obtainMessage();
			msg.arg1 = MSG_CONNECT;
			msg.arg2 = 0;
			mServiceHandler.sendMessage(msg);
		}				

	}
	
	@Override
	public void onCreate() {
		startup();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		// Currently only using as bound service
		
		// If we get killed, after returning from here, restart
		return START_STICKY;
	}

	@Override
	public IBinder onBind(Intent intent) {		
		return mBinder;
	}

	@Override
	public void onDestroy() {
		Toast.makeText(this, "Telemetry service done", Toast.LENGTH_SHORT).show(); 
	}

	public class LocalBinder extends Binder {
		public TelemTask getTelemTask(int id) {
			return (TelemTask) activeTelem;
		}
		public void openConnection() {
			Toast.makeText(getApplicationContext(), "Requested open connection", Toast.LENGTH_SHORT);
			Message msg = mServiceHandler.obtainMessage();
			msg.arg1 = MSG_CONNECT;
			mServiceHandler.sendMessage(msg);
		}
		public void stopConnection() {
			Message msg = mServiceHandler.obtainMessage();
			msg.arg1 = MSG_DISCONNECT;
			mServiceHandler.sendMessage(msg);
		}
		public boolean isConnected() {
			return activeTelem != null;
		}
	};

	public void toastMessage(String msgText) {
		Message msg = mServiceHandler.obtainMessage();
		msg.arg1 = MSG_TOAST;
		msg.obj = msgText;
		mServiceHandler.sendMessage(msg);
	}

	public interface TelemTask {
		public UAVObjectManager getObjectManager();
	};

	// Fake class for testing, simply emits periodic updates on 
	private class FakeTelemetryThread extends Thread implements TelemTask  {
		private UAVObjectManager objMngr;
		public UAVObjectManager getObjectManager() { return objMngr; };

		FakeTelemetryThread() {
			objMngr = new UAVObjectManager();
			UAVObjectsInitialize.register(objMngr);
		}

		public void run() {
			System.out.println("Running fake thread");

			Intent intent = new Intent();
			intent.setAction(INTENT_ACTION_CONNECTED);
			sendBroadcast(intent,null);

			//toastMessage("Started fake telemetry thread");
			UAVDataObject systemStats = (UAVDataObject) objMngr.getObject("SystemStats");
			UAVDataObject attitudeActual = (UAVDataObject) objMngr.getObject("AttitudeActual");
			double roll = 0;
			double pitch = 0; 
			double yaw = 0;
			while( !terminate ) {
				attitudeActual.getField("Roll").setDouble(roll);
				attitudeActual.getField("Pitch").setDouble(pitch);
				attitudeActual.getField("Yaw").setDouble(yaw);
				roll = (roll + 10) % 180;
				pitch = (pitch + 10) % 180;
				yaw = (yaw + 10) % 360;
				systemStats.updated();
				attitudeActual.updated();
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					break;
				}
			}
		}
	}
	private class BTTelemetryThread extends Thread implements TelemTask { 

		private UAVObjectManager objMngr;
		private UAVTalk uavTalk;
		private Telemetry tel;
		private TelemetryMonitor mon;

		public UAVObjectManager getObjectManager() { return objMngr; };

		BTTelemetryThread() {
			objMngr = new UAVObjectManager();
			UAVObjectsInitialize.register(objMngr);
		}

		public void run() {			
			if (DEBUG) Log.d(TAG, "Telemetry Thread started");

			Looper.prepare();						

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
				return;
			}


			if (DEBUG) Log.d(TAG, "Connected via bluetooth");

			uavTalk = bt.getUavtalk();
			tel = new Telemetry(uavTalk, objMngr);
			mon = new TelemetryMonitor(objMngr,tel);
			mon.addObserver(new Observer() {
				public void update(Observable arg0, Object arg1) {
					System.out.println("Mon updated. Connected: " + mon.getConnected() + " objects updated: " + mon.getObjectsUpdated());
					if(mon.getConnected() /*&& mon.getObjectsUpdated()*/) {
						Intent intent = new Intent();
						intent.setAction(INTENT_ACTION_CONNECTED);
						sendBroadcast(intent,null);					
					}
				}				
			});


			if (DEBUG) Log.d(TAG, "Entering UAVTalk processing loop");
			while( !terminate ) {
				if( !uavTalk.processInputStream() )
					break;
			}
			if (DEBUG) Log.d(TAG, "UAVTalk stream disconnected");
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
}
