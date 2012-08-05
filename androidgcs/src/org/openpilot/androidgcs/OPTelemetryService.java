package org.openpilot.androidgcs;

import java.util.Observable;
import java.util.Observer;

import org.openpilot.uavtalk.Telemetry;
import org.openpilot.uavtalk.TelemetryMonitor;
import org.openpilot.uavtalk.UAVDataObject;
import org.openpilot.uavtalk.UAVObjectManager;
import org.openpilot.uavtalk.UAVTalk;
import org.openpilot.uavtalk.uavobjects.UAVObjectsInitialize;

import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
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
	public static int LOGLEVEL = 0;
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
	private static HandlerThread thread;

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
				terminate = false;
				int connection_type;
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(OPTelemetryService.this);
				try {
					connection_type = Integer.decode(prefs.getString("connection_type", ""));
				} catch (NumberFormatException e) {
					connection_type = 0;
				}

				switch(connection_type) {
				case 0: // No connection
					return;
				case 1:
					Toast.makeText(getApplicationContext(), "Attempting fake connection", Toast.LENGTH_SHORT).show();
					activeTelem = new FakeTelemetryThread();
					break;
				case 2:
					Toast.makeText(getApplicationContext(), "Attempting BT connection", Toast.LENGTH_SHORT).show();
					activeTelem = new BTTelemetryThread();
					break;
				case 3:
					Toast.makeText(getApplicationContext(), "Attempting TCP connection", Toast.LENGTH_SHORT).show();
					activeTelem = new TcpTelemetryThread();
					break;
				default:
					throw new Error("Unsupported");
				}
				activeTelem.start();
				break;
			case MSG_DISCONNECT:
				Toast.makeText(getApplicationContext(), "Disconnect requested", Toast.LENGTH_SHORT).show();
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
            	Toast.makeText(OPTelemetryService.this, (String) msg.obj, Toast.LENGTH_SHORT).show();
            	break;
			default:
				System.out.println(msg.toString());
				throw new Error("Invalid message");
			}
		}
	};

	public void startup() {
		Toast.makeText(getApplicationContext(), "Telemetry service starting", Toast.LENGTH_SHORT).show();
		
		thread = new HandlerThread("TelemetryServiceHandler", Process.THREAD_PRIORITY_BACKGROUND);
		thread.start();

		// Get the HandlerThread's Looper and use it for our Handler 
		mServiceLooper = thread.getLooper();
		mServiceHandler = new ServiceHandler(mServiceLooper);
	
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(OPTelemetryService.this);
		if(prefs.getBoolean("autoconnect", false)) {
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
			Toast.makeText(getApplicationContext(), "Requested open connection", Toast.LENGTH_SHORT).show();
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
			UAVDataObject homeLocation = (UAVDataObject) objMngr.getObject("HomeLocation");
			UAVDataObject positionActual = (UAVDataObject) objMngr.getObject("PositionActual");

			homeLocation.getField("Latitude").setDouble(379420315);
			homeLocation.getField("Longitude").setDouble(-88330078);
			homeLocation.getField("Be").setDouble(26702.78710938,0);
			homeLocation.getField("Be").setDouble(-1468.33605957,1);
			homeLocation.getField("Be").setDouble(34181.78515625,2);
			
			
			double roll = 0;
			double pitch = 0; 
			double yaw = 0;
			double north = 0;
			double east = 0;
			while( !terminate ) {
				attitudeActual.getField("Roll").setDouble(roll);
				attitudeActual.getField("Pitch").setDouble(pitch);
				attitudeActual.getField("Yaw").setDouble(yaw);
				positionActual.getField("North").setDouble(north += 100);
				positionActual.getField("East").setDouble(east += 100);
				roll = (roll + 10) % 180;
				pitch = (pitch + 10) % 180;
				yaw = (yaw + 10) % 360;
				systemStats.updated();
				attitudeActual.updated();
				positionActual.updated();
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

			BluetoothUAVTalk bt = new BluetoothUAVTalk(OPTelemetryService.this);
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
				toastMessage("BT connection failed");
				return;
			}
			toastMessage("BT Connected");

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

	private class TcpTelemetryThread extends Thread implements TelemTask { 

		private UAVObjectManager objMngr;
		private UAVTalk uavTalk;
		private Telemetry tel;
		private TelemetryMonitor mon;

		public UAVObjectManager getObjectManager() { return objMngr; };

		TcpTelemetryThread() {
			objMngr = new UAVObjectManager();
			UAVObjectsInitialize.register(objMngr);
		}

		public void run() {			
			if (DEBUG) Log.d(TAG, "Telemetry Thread started");

			Looper.prepare();						

			TcpUAVTalk tcp = new TcpUAVTalk(OPTelemetryService.this);
			for( int i = 0; i < 10; i++ ) {
				if (DEBUG) Log.d(TAG, "Attempting network Connection");

				tcp.connect(objMngr);

				if( tcp.getConnected() )
					
					break;

				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					Log.e(TAG, "Thread interrupted while trying to connect");
					e.printStackTrace();
					return;
				}
			}
			if( ! tcp.getConnected() || terminate ) {
				toastMessage("TCP connection failed");
				return;
			}
			toastMessage("TCP Connected");

			if (DEBUG) Log.d(TAG, "Connected via network");

			uavTalk = tcp.getUavtalk();
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
}
