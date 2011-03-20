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
import android.net.Uri;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
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
	static final int MSG_CONNECT_BT   = 1;
	static final int MSG_CONNECT_FAKE = 2;
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
				Toast.makeText(OPTelemetryService.this, "HERE", Toast.LENGTH_SHORT);
				System.out.println("HERE");
				stopSelf(msg.arg2);
			case MSG_CONNECT_BT:
				terminate = false;
				activeTelem = new BTTelemetryThread();
				activeTelem.start();
				break;
			case MSG_CONNECT_FAKE:
				terminate = false;
				activeTelem = new FakeTelemetryThread();
				activeTelem.start();
				break;
			case MSG_DISCONNECT:
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

	@Override
	public void onCreate() {
		// Low priority thread for message handling with service
		HandlerThread thread = new HandlerThread("TelemetryServiceHandler",
				Process.THREAD_PRIORITY_BACKGROUND);
		thread.start();

		// Get the HandlerThread's Looper and use it for our Handler 
		mServiceLooper = thread.getLooper();
		mServiceHandler = new ServiceHandler(mServiceLooper);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Toast.makeText(this, "Telemetry service starting", Toast.LENGTH_SHORT).show();

		System.out.println("Start");
		// For each start request, send a message to start a job and deliver the
		// start ID so we know which request we're stopping when we finish the job
		Message msg = mServiceHandler.obtainMessage();
		msg.arg1 = MSG_START;
		msg.arg2 = startId;
		mServiceHandler.sendMessage(msg);

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
		public void openFakeConnection() {
			Message msg = mServiceHandler.obtainMessage();
			msg.arg1 = MSG_CONNECT_FAKE;
			mServiceHandler.sendMessage(msg);
		}
		public void openBTConnection() {
			Message msg = mServiceHandler.obtainMessage();
			msg.arg1 = MSG_CONNECT_BT;
			mServiceHandler.sendMessage(msg);
		}
		public void stopConnection() {
			Message msg = mServiceHandler.obtainMessage();
			msg.arg1 = MSG_DISCONNECT;
			mServiceHandler.sendMessage(msg);
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
			while( !terminate ) {
				systemStats.updated();
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
